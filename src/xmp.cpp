// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

// included header files
#include "error.hpp"
#include "properties.hpp"
#include "types.hpp"
#include "value.hpp"
#include "xmp_exiv2.hpp"

// + standard includes
#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
#include <expat.h>
#include "utils.hpp"
#define TXMP_STRING_TYPE std::string
#include "xmp_lifecycle.hpp"
#ifdef EXV_ADOBE_XMPSDK
#include <XMP.hpp>
#else
#include <XMPSDK.hpp>
#endif
#include <XMP.incl_cpp>
#endif  // EXV_HAVE_XMP_TOOLKIT

#ifdef EXV_HAVE_XMP_TOOLKIT
// This anonymous namespace contains a class named XMLValidator, which uses
// libexpat to do a basic validation check on an XML document. This is to
// reduce the chance of hitting a bug in the (third-party) xmpsdk
// library. For example, it is easy to a trigger a stack overflow in xmpsdk
// with a deeply nested tree.
namespace {
using namespace Exiv2;

class XMLValidator {
  size_t element_depth_ = 0;
  size_t namespace_depth_ = 0;

  // These fields are used to record whether an error occurred during
  // parsing. Why do we need to store the error for later, rather
  // than throw an exception immediately? Because expat is a C
  // library, so it isn't designed to be able to handle exceptions
  // thrown by the callback functions. Throwing exceptions during
  // parsing is an example of one of the things that xmpsdk does
  // wrong, leading to problems like https://github.com/Exiv2/exiv2/issues/1821.
  bool haserror_ = false;
  std::string errmsg_;
  XML_Size errlinenum_ = 0;
  XML_Size errcolnum_ = 0;

  // Very deeply nested XML trees can cause a stack overflow in
  // xmpsdk.  They are also very unlikely to be valid XMP, so we
  // error out if the depth exceeds this limit.
  static const size_t max_recursion_limit_ = 1000;

  XML_Parser parser_;

 public:
  // Runs an XML parser on `buf`. Throws an exception if the XML is invalid.
  static void check(const char* buf, size_t buflen) {
    XMLValidator validator;
    validator.check_internal(buf, buflen);
  }

  XMLValidator(const XMLValidator&) = delete;
  XMLValidator& operator=(const XMLValidator&) = delete;

 private:
  // Private constructor, because this class is only constructed by
  // the (static) check method.
  XMLValidator() : parser_(XML_ParserCreateNS(nullptr, '@')) {
    if (!parser_) {
      throw Error(ErrorCode::kerXMPToolkitError, "Could not create expat parser");
    }
  }

  ~XMLValidator() {
    XML_ParserFree(parser_);
  }

  void setError(const char* msg) {
    const XML_Size errlinenum = XML_GetCurrentLineNumber(parser_);
    const XML_Size errcolnum = XML_GetCurrentColumnNumber(parser_);
#ifndef SUPPRESS_WARNINGS
    EXV_INFO << "Invalid XML at line " << errlinenum << ", column " << errcolnum << ": " << msg << "\n";
#endif
    // If this is the first error, then save it.
    if (!haserror_) {
      haserror_ = true;
      errmsg_ = msg;
      errlinenum_ = errlinenum;
      errcolnum_ = errcolnum;
    }
  }

  void check_internal(const char* buf, size_t buflen) {
    if (buflen > static_cast<size_t>(std::numeric_limits<int>::max())) {
      throw Error(ErrorCode::kerXMPToolkitError, "Buffer length is greater than INT_MAX");
    }

    XML_SetUserData(parser_, this);
    XML_SetElementHandler(parser_, startElement_cb, endElement_cb);
    XML_SetNamespaceDeclHandler(parser_, startNamespace_cb, endNamespace_cb);
    XML_SetStartDoctypeDeclHandler(parser_, startDTD_cb);

    if (XML_Parse(parser_, buf, static_cast<int>(buflen), true) == XML_STATUS_ERROR) {
      setError(XML_ErrorString(XML_GetErrorCode(parser_)));
    }

    if (haserror_) {
      throw XMP_Error(kXMPErr_BadXML, "Error in XMLValidator");
    }
  }

  void startElement(const XML_Char*, const XML_Char**) noexcept {
    if (element_depth_ > max_recursion_limit_) {
      setError("Too deeply nested");
    }
    ++element_depth_;
  }

  void endElement(const XML_Char*) noexcept {
    if (element_depth_ > 0) {
      --element_depth_;
    } else {
      setError("Negative depth");
    }
  }

  void startNamespace(const XML_Char*, const XML_Char*) noexcept {
    if (namespace_depth_ > max_recursion_limit_) {
      setError("Too deeply nested");
    }
    ++namespace_depth_;
  }

  void endNamespace(const XML_Char*) noexcept {
    if (namespace_depth_ > 0) {
      --namespace_depth_;
    } else {
      setError("Negative depth");
    }
  }

  void startDTD(const XML_Char*, const XML_Char*, const XML_Char*, int) noexcept {
    // DOCTYPE is used for XXE attacks.
    setError("DOCTYPE not supported");
  }

  // This callback function is called by libexpat. It's a static wrapper
  // around startElement().
  static void XMLCALL startElement_cb(void* userData, const XML_Char* name, const XML_Char** attrs) noexcept {
    static_cast<XMLValidator*>(userData)->startElement(name, attrs);
  }

  // This callback function is called by libexpat. It's a static wrapper
  // around endElement().
  static void XMLCALL endElement_cb(void* userData, const XML_Char* name) noexcept {
    static_cast<XMLValidator*>(userData)->endElement(name);
  }

  // This callback function is called by libexpat. It's a static wrapper
  // around startNamespace().
  static void XMLCALL startNamespace_cb(void* userData, const XML_Char* prefix, const XML_Char* uri) noexcept {
    static_cast<XMLValidator*>(userData)->startNamespace(prefix, uri);
  }

  // This callback function is called by libexpat. It's a static wrapper
  // around endNamespace().
  static void XMLCALL endNamespace_cb(void* userData, const XML_Char* prefix) noexcept {
    static_cast<XMLValidator*>(userData)->endNamespace(prefix);
  }

  static void XMLCALL startDTD_cb(void* userData, const XML_Char* doctypeName, const XML_Char* sysid,
                                  const XML_Char* pubid, int has_internal_subset) noexcept {
    static_cast<XMLValidator*>(userData)->startDTD(doctypeName, sysid, pubid, has_internal_subset);
  }
};
}  // namespace
#endif  // EXV_HAVE_XMP_TOOLKIT

// *****************************************************************************
// local declarations
namespace {
//! Unary predicate that matches an Xmpdatum by key
class FindXmpdatum {
 public:
  //! Constructor, initializes the object with key
  explicit FindXmpdatum(const Exiv2::XmpKey& key) : key_(key.key()) {
  }
  /*!
    @brief Returns true if prefix and property of the argument
           Xmpdatum are equal to that of the object.
  */
  bool operator()(const Exiv2::Xmpdatum& xmpdatum) const {
    return key_ == xmpdatum.key();
  }

 private:
  std::string key_;

};  // class FindXmpdatum

#ifdef EXV_HAVE_XMP_TOOLKIT
//! Convert XMP Toolkit struct option bit to Value::XmpStruct
Exiv2::XmpValue::XmpStruct xmpStruct(XMP_OptionBits opt);

//! Convert Value::XmpStruct to XMP Toolkit array option bits
XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpStruct xs);

//! Convert XMP Toolkit array option bits to array TypeId
Exiv2::TypeId arrayValueTypeId(XMP_OptionBits opt);

//! Convert XMP Toolkit array option bits to Value::XmpArrayType
Exiv2::XmpValue::XmpArrayType xmpArrayType(XMP_OptionBits opt);

//! Convert Value::XmpArrayType to XMP Toolkit array option bits
XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpArrayType xat);

//! Convert XmpFormatFlags to XMP Toolkit format option bits
XMP_OptionBits xmpFormatOptionBits(Exiv2::XmpParser::XmpFormatFlags flags);

//! Print information about a parsed XMP node
void printNode(const std::string& schemaNs, const std::string& propPath, const std::string& propValue,
               XMP_OptionBits opt);

#endif  // EXV_HAVE_XMP_TOOLKIT
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
//! Internal Pimpl structure of class Xmpdatum.
struct Xmpdatum::Impl {
  Impl(const XmpKey& key, const Value* pValue);  //!< Constructor
  Impl(const Impl& rhs);                         //!< Copy constructor
  Impl& operator=(const Impl& rhs);              //!< Assignment
  ~Impl() = default;

  // DATA
  XmpKey::UniquePtr key_;   //!< Key
  Value::UniquePtr value_;  //!< Value
};

Xmpdatum::Impl::Impl(const XmpKey& key, const Value* pValue) : key_(key.clone()) {
  if (pValue)
    value_ = pValue->clone();
}

Xmpdatum::Impl::Impl(const Impl& rhs) {
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy
}

Xmpdatum::Impl& Xmpdatum::Impl::operator=(const Impl& rhs) {
  if (this == &rhs)
    return *this;
  key_.reset();
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy
  value_.reset();
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy
  return *this;
}

Xmpdatum::Xmpdatum(const XmpKey& key, const Value* pValue) : p_(std::make_unique<Impl>(key, pValue)) {
}

Xmpdatum::Xmpdatum(const Xmpdatum& rhs) : p_(std::make_unique<Impl>(*rhs.p_)) {
}

Xmpdatum& Xmpdatum::operator=(const Xmpdatum& rhs) {
  if (this == &rhs)
    return *this;
  *p_ = *rhs.p_;
  return *this;
}

Xmpdatum::~Xmpdatum() = default;

std::string Xmpdatum::key() const {
  return p_->key_ ? p_->key_->key() : "";
}

const char* Xmpdatum::familyName() const {
  return p_->key_ ? p_->key_->familyName() : "";
}

std::string Xmpdatum::groupName() const {
  return p_->key_ ? p_->key_->groupName() : "";
}

std::string Xmpdatum::tagName() const {
  return p_->key_ ? p_->key_->tagName() : "";
}

std::string Xmpdatum::tagLabel() const {
  return p_->key_ ? p_->key_->tagLabel() : "";
}

std::string Xmpdatum::tagDesc() const {
  return p_->key_ ? p_->key_->tagDesc() : "";
}

uint16_t Xmpdatum::tag() const {
  return p_->key_ ? p_->key_->tag() : 0;
}

TypeId Xmpdatum::typeId() const {
  return p_->value_ ? p_->value_->typeId() : invalidTypeId;
}

const char* Xmpdatum::typeName() const {
  return TypeInfo::typeName(typeId());
}

size_t Xmpdatum::typeSize() const {
  return 0;
}

size_t Xmpdatum::count() const {
  return p_->value_ ? p_->value_->count() : 0;
}

size_t Xmpdatum::size() const {
  return p_->value_ ? p_->value_->size() : 0;
}

std::string Xmpdatum::toString() const {
  return p_->value_ ? p_->value_->toString() : "";
}

std::string Xmpdatum::toString(size_t n) const {
  return p_->value_ ? p_->value_->toString(n) : "";
}

int64_t Xmpdatum::toInt64(size_t n) const {
  return p_->value_ ? p_->value_->toInt64(n) : -1;
}

float Xmpdatum::toFloat(size_t n) const {
  return p_->value_ ? p_->value_->toFloat(n) : -1;
}

Rational Xmpdatum::toRational(size_t n) const {
  return p_->value_ ? p_->value_->toRational(n) : Rational(-1, 1);
}

Value::UniquePtr Xmpdatum::getValue() const {
  return p_->value_ ? p_->value_->clone() : nullptr;
}

const Value& Xmpdatum::value() const {
  if (!p_->value_)
    throw Error(ErrorCode::kerValueNotSet, key());
  return *p_->value_;
}

size_t Xmpdatum::copy(byte* /*buf*/, ByteOrder /*byteOrder*/) const {
  throw Error(ErrorCode::kerFunctionNotSupported, "Xmpdatum::copy");
}

std::ostream& Xmpdatum::write(std::ostream& os, const ExifData*) const {
  return XmpProperties::printProperty(os, key(), value());
}

void Xmpdatum::setValue(const Value* pValue) {
  p_->value_.reset();
  if (pValue)
    p_->value_ = pValue->clone();
}

int Xmpdatum::setValue(const std::string& value) {
  XmpProperties::XmpLock lock;
  if (!p_->value_) {
    TypeId type = xmpText;
    if (p_->key_) {
      type = XmpProperties::propertyTypeUnlocked(*p_->key_.get(), lock);
    }
    p_->value_ = Value::create(type);
  }
  return p_->value_->read(value);
}

Xmpdatum& XmpData::operator[](const std::string& key) {
  XmpProperties::XmpLock lock;
  XmpKey xmpKey(key, lock);
  auto pos = std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(), FindXmpdatum(xmpKey));
  if (pos == xmpMetadata_.end()) {
    return xmpMetadata_.emplace_back(xmpKey);
  }
  return *pos;
}

int XmpData::add(const XmpKey& key, const Value* value) {
  XmpProperties::XmpLock lock;
  return addUnlocked(key, value, lock);
}

int XmpData::addUnlocked(const XmpKey& key, const Value* value, const XmpProperties::XmpLock&) {
  xmpMetadata_.emplace_back(key, value);
  return 0;
}

int XmpData::add(const Xmpdatum& xmpDatum) {
  XmpProperties::XmpLock lock;
  return addUnlocked(xmpDatum, lock);
}

int XmpData::addUnlocked(const Xmpdatum& xmpDatum, const XmpProperties::XmpLock&) {
  xmpMetadata_.push_back(xmpDatum);
  return 0;
}

XmpData::const_iterator XmpData::findKey(const XmpKey& key) const {
  XmpProperties::XmpLock lock;
  return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(), FindXmpdatum(key));
}

XmpData::iterator XmpData::findKey(const XmpKey& key) {
  XmpProperties::XmpLock lock;
  return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(), FindXmpdatum(key));
}

void XmpData::clear() {
  XmpProperties::XmpLock lock;
  clearUnlocked(lock);
}

void XmpData::clearUnlocked(const XmpProperties::XmpLock&) {
  xmpMetadata_.clear();
}

void XmpData::sortByKey() {
  XmpProperties::XmpLock lock;
  sortByKeyUnlocked(lock);
}

void XmpData::sortByKeyUnlocked(const XmpProperties::XmpLock&) {
  std::sort(xmpMetadata_.begin(), xmpMetadata_.end(), cmpMetadataByKey);
}

XmpData::const_iterator XmpData::begin() const {
  return xmpMetadata_.begin();
}

XmpData::const_iterator XmpData::end() const {
  return xmpMetadata_.end();
}

bool XmpData::empty() const {
  XmpProperties::XmpLock lock;
  return emptyUnlocked(lock);
}

bool XmpData::emptyUnlocked(const XmpProperties::XmpLock&) const {
  return xmpMetadata_.empty();
}

long XmpData::count() const {
  XmpProperties::XmpLock lock;
  return countUnlocked(lock);
}

long XmpData::countUnlocked(const XmpProperties::XmpLock&) const {
  return static_cast<long>(xmpMetadata_.size());
}

XmpData::iterator XmpData::begin() {
  return xmpMetadata_.begin();
}

XmpData::iterator XmpData::end() {
  return xmpMetadata_.end();
}

XmpData::iterator XmpData::erase(XmpData::iterator pos) {
  XmpProperties::XmpLock lock;
  return xmpMetadata_.erase(pos);
}

void XmpData::eraseFamily(XmpData::iterator& pos) {
  // https://github.com/Exiv2/exiv2/issues/521
  // delete 'children' of XMP composites (XmpSeq and XmpBag)

  // I build a StringVector of keys to remove
  // Then I remove them with erase(....)
  // erase() has nasty side effects on its argument
  // The side effects are avoided by the two-step approach
  // https://github.com/Exiv2/exiv2/issues/560
  std::string key(pos->key());
  std::vector<std::string> keys;
  while (pos != xmpMetadata_.end()) {
    if (!pos->key().starts_with(key))
      break;
    keys.push_back(pos->key());
    pos++;
  }
  // now erase the family!
  for (const auto& k : keys) {
    erase(findKey(Exiv2::XmpKey(k)));
  }
}

// We use XmpProperties::getMutex() as the single "Giant Lock" for the entire XMP subsystem.
// See src/properties.cpp for the definition.

// Lock Hierarchy:
// 1. XmpProperties::getMutex()
//    - Protects XMP Toolkit lifecycle (initialize/terminate)
//    - Protects XMP Toolkit usage (encode/decode) via serialization
//    - Protects XMP Namespace Registry (XmpProperties::nsRegistry_)
//    - Protects XMP SDK internal state (via exclusive access)
//
// Facade Pattern:
// - Public methods (encode, decode, registerNs, etc...) acquire the lock
//   and call corresponding private static *Impl* methods.
// - *Impl* methods assert/assume lock is held and perform the work.
// - *Impl* methods can call other *Impl* methods or *Unsafe* methods in XmpProperties
//   without fear of deadlock or recursive locking issues.

// Default locking implementation removed as we use Giant Lock

#ifdef EXV_HAVE_XMP_TOOLKIT

void xmpToolkitEnsureInitialized() {
  static XmpToolkitLifetimeManager instance;
  (void)instance;
}

void XmpParser::registerNsImpl(const std::string& ns, const std::string& prefix) {
  xmpToolkitEnsureInitialized();
  try {
    std::string existingPrefix;
    if (SXMPMeta::GetNamespacePrefix(ns.c_str(), &existingPrefix)) {
      if (!existingPrefix.empty() && existingPrefix.back() == ':') {
        existingPrefix.pop_back();
      }
      if (existingPrefix == prefix) {
        // Already registered correctly, skip overhead
        return;
      }
    }

    SXMPMeta::DeleteNamespace(ns.c_str());
#ifdef EXV_ADOBE_XMPSDK
    SXMPMeta::RegisterNamespace(ns.c_str(), prefix.c_str(), nullptr);
#else
    SXMPMeta::RegisterNamespace(ns.c_str(), prefix.c_str());
#endif
  } catch (const XMP_Error& /* e */) {
    // throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
}
#else
void XmpParser::registerNsImpl(const std::string& /*ns*/, const std::string& /*prefix*/) {
}
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
static XMP_Status nsDumper(void* refCon, XMP_StringPtr buffer, XMP_StringLen bufferSize) {
  XMP_Status result = 0;
  std::string out(buffer, bufferSize);

  // remove blanks: http://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
  std::erase_if(out, [](unsigned char c) { return c < 32 || c > 126; });

  bool bURI = Internal::contains(out, "http://");
  bool bNS = Internal::contains(out, ':') && !bURI;

  // pop trailing ':' on a namespace
  if (bNS && !out.empty() && out.back() == ':')
    out.pop_back();

  if (bURI || bNS) {
    auto p = static_cast<std::map<std::string, std::string>*>(refCon);
    auto& m = *p;

    std::string b;
    if (bNS) {  // store the NS in dict[""]
      m[b] = std::move(out);
    } else if (m.contains(b)) {  // store dict[uri] = dict[""]
      m[m[b]] = std::move(out);
      m.erase(b);
    }
  }
  return result;
}
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
void XmpParser::registeredNamespaces(Exiv2::Dictionary& dict) {
  try {
    XmpProperties::XmpLock lock;
    registeredNamespacesUnlocked(dict, lock);
  } catch (const XMP_Error& e) {
    throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
}

void XmpParser::registeredNamespacesUnlocked(Exiv2::Dictionary& dict, const XmpProperties::XmpLock&) {
  xmpToolkitEnsureInitialized();
  SXMPMeta::DumpNamespaces(nsDumper, &dict);
}
#else
void XmpParser::registeredNamespaces(Exiv2::Dictionary&) {
}

void XmpParser::registeredNamespacesUnlocked(Exiv2::Dictionary&, const XmpProperties::XmpLock&) {
}
#endif

void XmpParser::clearCustomNamespaces() {
  XmpProperties::XmpLock lock;
  XmpProperties::unregisterNsUnlocked(lock);
}

#ifdef EXV_HAVE_XMP_TOOLKIT
void XmpParser::registerNs(const std::string& ns, const std::string& prefix) {
  try {
    XmpProperties::XmpLock lock;
    registerNsImpl(ns, prefix);
  } catch (const XMP_Error& /* e */) {
    // throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
}
#else
void XmpParser::registerNs(const std::string& /*ns*/, const std::string& /*prefix*/) {
}
#endif

void XmpParser::unregisterNs(const std::string& /*ns*/) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  try {
    // Throws XMP Toolkit error 8: Unimplemented method XMPMeta::DeleteNamespace
    //          SXMPMeta::DeleteNamespace(ns.c_str());
  } catch (const XMP_Error& e) {
    throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
#endif
}  // XmpParser::unregisterNs

bool XmpParser::initialize(void (*)(void*, bool), void*) {
  return true;
}

void XmpParser::terminate() {
}

#ifdef EXV_HAVE_XMP_TOOLKIT
int XmpParser::decode(XmpData& xmpData, const std::string& xmpPacket) {
  try {
    xmpData.setPacket(xmpPacket);
    if (xmpPacket.empty()) {
      xmpData.clear();
      return 0;
    }

    // Acquire Giant Lock
    XmpProperties::XmpLock lock;
    try {
      xmpToolkitEnsureInitialized();
    } catch (const Error&) {
#ifndef SUPPRESS_WARNINGS
      EXV_ERROR << "XMP toolkit initialization failed.\n";
#endif
      return 2;
    }

    xmpData.clearUnlocked(lock);

    // Make sure the unterminated substring is used
    size_t len = xmpPacket.size();
    while (len > 0 && 0 == xmpPacket[len - 1])
      --len;

    XMLValidator::check(xmpPacket.data(), len);
    SXMPMeta meta(xmpPacket.data(), static_cast<XMP_StringLen>(len));
    SXMPIterator iter(meta);
    std::string schemaNs;
    std::string propPath;
    std::string propValue;
    XMP_OptionBits opt = 0;
    while (iter.Next(&schemaNs, &propPath, &propValue, &opt)) {
      printNode(schemaNs, propPath, propValue, opt);
      if (XMP_PropIsAlias(opt)) {
        throw Error(ErrorCode::kerAliasesNotSupported, schemaNs, propPath, propValue);
      }
      if (XMP_NodeIsSchema(opt)) {
        // Register unknown namespaces with Exiv2
        // (Namespaces are automatically registered with the XMP Toolkit)
        if (XmpProperties::prefixUnlocked(schemaNs, lock).empty()) {
          std::string prefix;
          if (!SXMPMeta::GetNamespacePrefix(schemaNs.c_str(), &prefix))
            throw Error(ErrorCode::kerSchemaNamespaceNotRegistered, schemaNs);
          prefix.pop_back();
          XmpProperties::registerNsUnlocked(schemaNs, prefix, lock);
        }
        continue;
      }
      auto key = makeXmpKey(schemaNs, propPath, lock);
      if (XMP_ArrayIsAltText(opt)) {
        // Read Lang Alt property
        auto val = std::make_unique<LangAltValue>();
        XMP_Index count = meta.CountArrayItems(schemaNs.c_str(), propPath.c_str());
        while (count-- > 0) {
          // Get the text
          bool haveNext = iter.Next(&schemaNs, &propPath, &propValue, &opt);
          printNode(schemaNs, propPath, propValue, opt);
          if (!haveNext || !XMP_PropIsSimple(opt) || !XMP_PropHasLang(opt)) {
            throw Error(ErrorCode::kerDecodeLangAltPropertyFailed, propPath, opt);
          }
          std::string text = propValue;
          // Get the language qualifier
          haveNext = iter.Next(&schemaNs, &propPath, &propValue, &opt);
          printNode(schemaNs, propPath, propValue, opt);
          if (!haveNext || !XMP_PropIsSimple(opt) || !XMP_PropIsQualifier(opt) ||
              propPath.substr(propPath.size() - 8, 8) != "xml:lang") {
            throw Error(ErrorCode::kerDecodeLangAltQualifierFailed, propPath, opt);
          }
          val->value_[propValue] = std::move(text);
        }
        xmpData.addUnlocked(*key, val.get(), lock);
        continue;
      }
      if (XMP_PropIsArray(opt) && !XMP_PropHasQualifiers(opt) && !XMP_ArrayIsAltText(opt)) {
        // Check if all elements are simple
        bool simpleArray = true;
        SXMPIterator aIter(meta, schemaNs.c_str(), propPath.c_str());
        std::string aSchemaNs;
        std::string aPropPath;
        std::string aPropValue;
        XMP_OptionBits aOpt = 0;
        while (aIter.Next(&aSchemaNs, &aPropPath, &aPropValue, &aOpt)) {
          if (propPath == aPropPath)
            continue;
          if (!XMP_PropIsSimple(aOpt) || XMP_PropHasQualifiers(aOpt) || XMP_PropIsQualifier(aOpt) ||
              XMP_NodeIsSchema(aOpt) || XMP_PropIsAlias(aOpt)) {
            simpleArray = false;
            break;
          }
        }
        if (simpleArray) {
          // Read the array into an XmpArrayValue
          auto val = std::make_unique<XmpArrayValue>(arrayValueTypeId(opt));
          XMP_Index count = meta.CountArrayItems(schemaNs.c_str(), propPath.c_str());
          while (count-- > 0) {
            iter.Next(&schemaNs, &propPath, &propValue, &opt);
            printNode(schemaNs, propPath, propValue, opt);
            val->read(propValue);
          }
          xmpData.addUnlocked(*key, val.get(), lock);
          continue;
        }
      }

      auto val = std::make_unique<XmpTextValue>();
      if (XMP_PropIsStruct(opt) || XMP_PropIsArray(opt)) {
        // Create a metadatum with only XMP options
        val->setXmpArrayType(xmpArrayType(opt));
        val->setXmpStruct(xmpStruct(opt));
        xmpData.addUnlocked(*key, val.get(), lock);
        continue;
      }
      if (XMP_PropIsSimple(opt) || XMP_PropIsQualifier(opt)) {
        val->read(propValue);
        xmpData.addUnlocked(*key, val.get(), lock);
        continue;
      }
      // Don't let any node go by unnoticed
      throw Error(ErrorCode::kerUnhandledXmpNode, key->key(), opt);
    }  // iterate through all XMP nodes

    return 0;
  }
#ifndef SUPPRESS_WARNINGS
  catch (const Error& e) {
    if (e.code() == ErrorCode::kerXMPToolkitError) {
      // Initialization failures caught above, this handles other XMP errors if any wrapped in Error
    }
    throw;
  } catch (const XMP_Error& e) {
    EXV_ERROR << Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg()) << "\n";
    xmpData.clear();
    return 3;
  }
#else
  catch (const XMP_Error&) {
    xmpData.clear();
    return 3;
  }
#endif  // SUPPRESS_WARNINGS
}  // XmpParser::decode
#else
int XmpParser::decode(XmpData& xmpData, const std::string& xmpPacket) {
  xmpData.clear();
  if (!xmpPacket.empty()) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "XMP toolkit support not compiled in.\n";
#endif
  }
  return 1;
}  // XmpParser::decode
#endif  // !EXV_HAVE_XMP_TOOLKIT

#ifdef EXV_HAVE_XMP_TOOLKIT
int XmpParser::encode(std::string& xmpPacket, const XmpData& xmpData, uint16_t formatFlags, uint32_t padding) {
  try {
    // Acquire Giant Lock
    XmpProperties::XmpLock lock;
    try {
      xmpToolkitEnsureInitialized();
    } catch (const Error&) {
#ifndef SUPPRESS_WARNINGS
      EXV_ERROR << "XMP toolkit initialization failed.\n";
#endif
      return 2;
    }

    if (xmpData.emptyUnlocked(lock)) {
      xmpPacket.clear();
      return 0;
    }  // We are holding the Giant Lock, so we can iterate nsRegistry_ safely.
    // XmpProperties interactions must go through Unlocked/impl methods to avoid deadlocks.
    for (const auto& [xmp, uri] : XmpProperties::nsRegistry_) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Registering " << uri.prefix_ << " : " << xmp << "\n";
#endif
      // registerNsImpl is safe since we hold the lock
      registerNsImpl(xmp, uri.prefix_);
    }

    SXMPMeta meta;
    for (const auto& xmp : xmpData) {
      // Must use Unlocked version of ns() because we hold the lock!
      const std::string ns = XmpProperties::nsUnlocked(xmp.groupName(), lock);
      XMP_OptionBits options = 0;

      if (xmp.typeId() == langAlt) {
        // Encode Lang Alt property
        const auto la = dynamic_cast<const LangAltValue*>(&xmp.value());
        if (!la)
          throw Error(ErrorCode::kerEncodeLangAltPropertyFailed, xmp.key());

        int idx = 1;
        for (const auto& [lang, specs] : la->value_) {
          if (!specs.empty()) {  // remove lang specs with no value
            printNode(ns, xmp.tagName(), specs, 0);
            meta.AppendArrayItem(ns.c_str(), xmp.tagName().c_str(), kXMP_PropArrayIsAlternate, specs.c_str());
            const std::string item = xmp.tagName() + "[" + toString(idx++) + "]";
            meta.SetQualifier(ns.c_str(), item.c_str(), kXMP_NS_XML, "lang", lang.c_str());
          }
        }
        continue;
      }

      // Todo: Xmpdatum should have an XmpValue, not a Value
      const auto val = dynamic_cast<const XmpValue*>(&xmp.value());
      if (!val)
        throw Error(ErrorCode::kerInvalidKeyXmpValue, xmp.key(), xmp.typeName());
      options = xmpArrayOptionBits(val->xmpArrayType()) | xmpArrayOptionBits(val->xmpStruct());
      if (xmp.typeId() == xmpBag || xmp.typeId() == xmpSeq || xmp.typeId() == xmpAlt) {
        printNode(ns, xmp.tagName(), "", options);
        meta.SetProperty(ns.c_str(), xmp.tagName().c_str(), nullptr, options);
        for (size_t idx = 0; idx < xmp.count(); ++idx) {
          const std::string item = xmp.tagName() + "[" + toString(idx + 1) + "]";
          printNode(ns, item, xmp.toString(static_cast<long>(idx)), 0);
          meta.SetProperty(ns.c_str(), item.c_str(), xmp.toString(static_cast<long>(idx)).c_str());
        }
        continue;
      }
      if (xmp.typeId() == xmpText) {
        const auto xt = dynamic_cast<const XmpTextValue*>(&xmp.value());
        if (xmp.count() == 0 || xt->xmpStruct() != XmpValue::xsNone || xt->xmpArrayType() != XmpValue::xaNone) {
          printNode(ns, xmp.tagName(), "", options);
          meta.SetProperty(ns.c_str(), xmp.tagName().c_str(), nullptr, options);
        } else {
          printNode(ns, xmp.tagName(), xmp.toString(0), options);
          meta.SetProperty(ns.c_str(), xmp.tagName().c_str(), xmp.toString(0).c_str(), options);
        }
        continue;
      }
      // Don't let any Xmpdatum go by unnoticed
      throw Error(ErrorCode::kerUnhandledXmpdatum, xmp.tagName(), xmp.typeName());
    }
    std::string tmpPacket;
    meta.SerializeToBuffer(&tmpPacket, xmpFormatOptionBits(static_cast<XmpFormatFlags>(formatFlags)),
                           padding);  // throws
    xmpPacket = std::move(tmpPacket);

    return 0;
  }
#ifndef SUPPRESS_WARNINGS
  catch (const Error& /* e */) {
    throw;
  } catch (const XMP_Error& e) {
    EXV_ERROR << Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg()) << "\n";
    return 3;
  }
#else
  catch (const XMP_Error&) {
    return 3;
  }
#endif  // SUPPRESS_WARNINGS
}  // XmpParser::encode
#else
int XmpParser::encode(std::string& /*xmpPacket*/, const XmpData& xmpData, uint16_t /*formatFlags*/,
                      uint32_t /*padding*/) {
  if (!xmpData.empty()) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "XMP toolkit support not compiled in.\n";
#endif
  }
  return 1;
}  // XmpParser::encode
#endif  // !EXV_HAVE_XMP_TOOLKIT

}  // namespace Exiv2

// *****************************************************************************
// local definitions
#ifdef EXV_HAVE_XMP_TOOLKIT
namespace {
Exiv2::XmpValue::XmpStruct xmpStruct(XMP_OptionBits opt) {
  Exiv2::XmpValue::XmpStruct var(Exiv2::XmpValue::xsNone);
  if (XMP_PropIsStruct(opt)) {
    var = Exiv2::XmpValue::xsStruct;
  }
  return var;
}

XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpStruct xs) {
  XMP_OptionBits var(0);
  switch (xs) {
    case Exiv2::XmpValue::xsNone:
      break;
    case Exiv2::XmpValue::xsStruct:
      XMP_SetOption(var, kXMP_PropValueIsStruct);
      break;
  }
  return var;
}

Exiv2::TypeId arrayValueTypeId(XMP_OptionBits opt) {
  Exiv2::TypeId typeId(Exiv2::invalidTypeId);
  if (XMP_PropIsArray(opt)) {
    if (XMP_ArrayIsAlternate(opt))
      typeId = Exiv2::xmpAlt;
    else if (XMP_ArrayIsOrdered(opt))
      typeId = Exiv2::xmpSeq;
    else if (XMP_ArrayIsUnordered(opt))
      typeId = Exiv2::xmpBag;
  }
  return typeId;
}

Exiv2::XmpValue::XmpArrayType xmpArrayType(XMP_OptionBits opt) {
  return Exiv2::XmpValue::xmpArrayType(arrayValueTypeId(opt));
}

XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpArrayType xat) {
  XMP_OptionBits var(0);
  switch (xat) {
    case Exiv2::XmpValue::xaNone:
      break;
    case Exiv2::XmpValue::xaAlt:
      XMP_SetOption(var, kXMP_PropValueIsArray);
      XMP_SetOption(var, kXMP_PropArrayIsAlternate);
      break;
    case Exiv2::XmpValue::xaSeq:
      XMP_SetOption(var, kXMP_PropValueIsArray);
      XMP_SetOption(var, kXMP_PropArrayIsOrdered);
      break;
    case Exiv2::XmpValue::xaBag:
      XMP_SetOption(var, kXMP_PropValueIsArray);
      break;
  }
  return var;
}

#ifdef EXV_ADOBE_XMPSDK
#define kXMP_WriteAliasComments 0x0400UL
#endif

XMP_OptionBits xmpFormatOptionBits(Exiv2::XmpParser::XmpFormatFlags flags) {
  XMP_OptionBits var(0);
  if (flags & Exiv2::XmpParser::omitPacketWrapper)
    var |= kXMP_OmitPacketWrapper;
  if (flags & Exiv2::XmpParser::readOnlyPacket)
    var |= kXMP_ReadOnlyPacket;
  if (flags & Exiv2::XmpParser::useCompactFormat)
    var |= kXMP_UseCompactFormat;
  if (flags & Exiv2::XmpParser::includeThumbnailPad)
    var |= kXMP_IncludeThumbnailPad;
  if (flags & Exiv2::XmpParser::exactPacketLength)
    var |= kXMP_ExactPacketLength;
  if (flags & Exiv2::XmpParser::writeAliasComments)
    var |= kXMP_WriteAliasComments;
  if (flags & Exiv2::XmpParser::omitAllFormatting)
    var |= kXMP_OmitAllFormatting;
  return var;
}

#ifdef EXIV2_DEBUG_MESSAGES
void printNode(const std::string& schemaNs, const std::string& propPath, const std::string& propValue,
               XMP_OptionBits opt) {
  static std::once_flag flag;
  std::call_once(flag, []() {
    std::cout << "ashisabsals\n"
              << "lcqqtrgqlai\n";
  });
  enum {
    alia = 0,
    sche,
    hasq,
    isqu,
    stru,
    arra,
    abag,
    aseq,
    aalt,
    lang,
    simp,
    len,
  };

  std::string opts(len, '.');
  if (XMP_PropIsAlias(opt))
    opts[alia] = 'X';
  if (XMP_NodeIsSchema(opt))
    opts[sche] = 'X';
  if (XMP_PropHasQualifiers(opt))
    opts[hasq] = 'X';
  if (XMP_PropIsQualifier(opt))
    opts[isqu] = 'X';
  if (XMP_PropIsStruct(opt))
    opts[stru] = 'X';
  if (XMP_PropIsArray(opt))
    opts[arra] = 'X';
  if (XMP_ArrayIsUnordered(opt))
    opts[abag] = 'X';
  if (XMP_ArrayIsOrdered(opt))
    opts[aseq] = 'X';
  if (XMP_ArrayIsAlternate(opt))
    opts[aalt] = 'X';
  if (XMP_ArrayIsAltText(opt))
    opts[lang] = 'X';
  if (XMP_PropIsSimple(opt))
    opts[simp] = 'X';

  std::cout << opts << " ";
  if (opts[sche] == 'X') {
    std::cout << "ns=" << schemaNs;
  } else {
    std::cout << propPath << " = " << propValue;
  }
  std::cout << '\n';
}
#else
void printNode(const std::string&, const std::string&, const std::string&, XMP_OptionBits) {
}
#endif  // EXIV2_DEBUG_MESSAGES

}  // namespace

Exiv2::XmpKey::UniquePtr Exiv2::XmpParser::makeXmpKey(const std::string& schemaNs, const std::string& propPath,
                                                      const XmpProperties::XmpLock& lock) {
  std::string property;
  std::string::size_type idx = propPath.find(':');
  if (idx == std::string::npos) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerPropertyNameIdentificationFailed, propPath, schemaNs);
  }
  // Don't worry about out_of_range, XMP parser takes care of this
  property = propPath.substr(idx + 1);
  std::string prefix = Exiv2::XmpProperties::prefixUnlocked(schemaNs, lock);
  if (prefix.empty()) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerNoPrefixForNamespace, propPath, schemaNs);
  }
  return Exiv2::XmpKey::UniquePtr(new Exiv2::XmpKey(prefix, property, lock));
}  // makeXmpKey
#endif  // EXV_HAVE_XMP_TOOLKIT
