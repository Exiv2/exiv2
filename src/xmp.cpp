// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "error.hpp"
#include "properties.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "xmp_exiv2.hpp"

// + standard includes
#include <algorithm>
#include <iostream>

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
#include <expat.h>
#define TXMP_STRING_TYPE std::string
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
Exiv2::XmpValue::XmpStruct xmpStruct(const XMP_OptionBits& opt);

//! Convert Value::XmpStruct to XMP Toolkit array option bits
XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpStruct xs);

//! Convert XMP Toolkit array option bits to array TypeId
Exiv2::TypeId arrayValueTypeId(const XMP_OptionBits& opt);

//! Convert XMP Toolkit array option bits to Value::XmpArrayType
Exiv2::XmpValue::XmpArrayType xmpArrayType(const XMP_OptionBits& opt);

//! Convert Value::XmpArrayType to XMP Toolkit array option bits
XMP_OptionBits xmpArrayOptionBits(Exiv2::XmpValue::XmpArrayType xat);

//! Convert XmpFormatFlags to XMP Toolkit format option bits
XMP_OptionBits xmpFormatOptionBits(Exiv2::XmpParser::XmpFormatFlags flags);

//! Print information about a parsed XMP node
void printNode(const std::string& schemaNs, const std::string& propPath, const std::string& propValue,
               const XMP_OptionBits& opt);

//! Make an XMP key from a schema namespace and property path
Exiv2::XmpKey::UniquePtr makeXmpKey(const std::string& schemaNs, const std::string& propPath);

//! Helper class used to serialize critical sections
class AutoLock {
 public:
  AutoLock(Exiv2::XmpParser::XmpLockFct xmpLockFct, void* pLockData) : xmpLockFct_(xmpLockFct), pLockData_(pLockData) {
    if (xmpLockFct_)
      xmpLockFct_(pLockData_, true);
  }
  ~AutoLock() {
    if (xmpLockFct_)
      xmpLockFct_(pLockData_, false);
  }

  AutoLock(const AutoLock&) = delete;
  AutoLock& operator=(const AutoLock&) = delete;

 private:
  Exiv2::XmpParser::XmpLockFct xmpLockFct_;
  void* pLockData_;
};
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

Xmpdatum::Xmpdatum(const Xmpdatum& rhs) : Metadatum(rhs), p_(std::make_unique<Impl>(*rhs.p_)) {
}

Xmpdatum& Xmpdatum::operator=(const Xmpdatum& rhs) {
  if (this == &rhs)
    return *this;
  Metadatum::operator=(rhs);
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

Xmpdatum& Xmpdatum::operator=(const std::string& value) {
  setValue(value);
  return *this;
}

Xmpdatum& Xmpdatum::operator=(const Value& value) {
  setValue(&value);
  return *this;
}

void Xmpdatum::setValue(const Value* pValue) {
  p_->value_.reset();
  if (pValue)
    p_->value_ = pValue->clone();
}

int Xmpdatum::setValue(const std::string& value) {
  if (!p_->value_) {
    TypeId type = xmpText;
    if (p_->key_) {
      type = XmpProperties::propertyType(*p_->key_.get());
    }
    p_->value_ = Value::create(type);
  }
  return p_->value_->read(value);
}

Xmpdatum& XmpData::operator[](const std::string& key) {
  XmpKey xmpKey(key);
  auto pos = findKey(xmpKey);
  if (pos == end()) {
    xmpMetadata_.emplace_back(xmpKey);
    return xmpMetadata_.back();
  }
  return *pos;
}

int XmpData::add(const XmpKey& key, const Value* value) {
  return add(Xmpdatum(key, value));
}

int XmpData::add(const Xmpdatum& xmpDatum) {
  xmpMetadata_.push_back(xmpDatum);
  return 0;
}

XmpData::const_iterator XmpData::findKey(const XmpKey& key) const {
  return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(), FindXmpdatum(key));
}

XmpData::iterator XmpData::findKey(const XmpKey& key) {
  return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(), FindXmpdatum(key));
}

void XmpData::clear() {
  xmpMetadata_.clear();
}

void XmpData::sortByKey() {
  std::sort(xmpMetadata_.begin(), xmpMetadata_.end(), cmpMetadataByKey);
}

XmpData::const_iterator XmpData::begin() const {
  return xmpMetadata_.begin();
}

XmpData::const_iterator XmpData::end() const {
  return xmpMetadata_.end();
}

bool XmpData::empty() const {
  return xmpMetadata_.empty();
}

long XmpData::count() const {
  return static_cast<long>(xmpMetadata_.size());
}

XmpData::iterator XmpData::begin() {
  return xmpMetadata_.begin();
}

XmpData::iterator XmpData::end() {
  return xmpMetadata_.end();
}

XmpData::iterator XmpData::erase(XmpData::iterator pos) {
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
    if (!Exiv2::Internal::startsWith(pos->key(), key))
      break;
    keys.push_back(pos->key());
    pos++;
  }
  // now erase the family!
  for (const auto& k : keys) {
    erase(findKey(Exiv2::XmpKey(k)));
  }
}

bool XmpParser::initialized_ = false;
XmpParser::XmpLockFct XmpParser::xmpLockFct_ = nullptr;
void* XmpParser::pLockData_ = nullptr;

#ifdef EXV_HAVE_XMP_TOOLKIT
bool XmpParser::initialize(XmpParser::XmpLockFct xmpLockFct, void* pLockData) {
  if (!initialized_) {
    xmpLockFct_ = xmpLockFct;
    pLockData_ = pLockData;
    initialized_ = SXMPMeta::Initialize();
#ifdef EXV_ADOBE_XMPSDK
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/lightroom/1.0/", "lr", nullptr);
    SXMPMeta::RegisterNamespace("http://rs.tdwg.org/dwc/index.htm", "dwc", nullptr);
    SXMPMeta::RegisterNamespace("http://purl.org/dc/terms/", "dcterms", nullptr);
    SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/1.0/", "digiKam", nullptr);
    SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/kipi/1.0/", "kipi", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.0/", "MicrosoftPhoto", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.acdsee.com/iptc/1.0/", "acdsee", nullptr);
    SXMPMeta::RegisterNamespace("http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "iptcExt", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.useplus.org/ldf/xmp/1.0/", "plus", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/", "MP", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/Region#", "MPReg", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.google.com/photos/1.0/panorama/", "GPano", nullptr);
    SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs", nullptr);
    SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/keywords/", "mwg-kw", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/xmp/sType/Area#", "stArea", nullptr);
    SXMPMeta::RegisterNamespace("http://cipa.jp/exif/1.0/", "exifEX", nullptr);
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/camera-raw-saved-settings/1.0/", "crss", nullptr);
    SXMPMeta::RegisterNamespace("http://www.audio/", "audio", nullptr);
    SXMPMeta::RegisterNamespace("http://www.video/", "video", nullptr);
#else
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/lightroom/1.0/", "lr");
    SXMPMeta::RegisterNamespace("http://rs.tdwg.org/dwc/index.htm", "dwc");
    SXMPMeta::RegisterNamespace("http://purl.org/dc/terms/", "dcterms");
    SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/1.0/", "digiKam");
    SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/kipi/1.0/", "kipi");
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.0/", "MicrosoftPhoto");
    SXMPMeta::RegisterNamespace("http://ns.acdsee.com/iptc/1.0/", "acdsee");
    SXMPMeta::RegisterNamespace("http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "iptcExt");
    SXMPMeta::RegisterNamespace("http://ns.useplus.org/ldf/xmp/1.0/", "plus");
    SXMPMeta::RegisterNamespace("http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro");
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia");
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/", "MP");
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI");
    SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/Region#", "MPReg");
    SXMPMeta::RegisterNamespace("http://ns.google.com/photos/1.0/panorama/", "GPano");
    SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs");
    SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/keywords/", "mwg-kw");
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/xmp/sType/Area#", "stArea");
    SXMPMeta::RegisterNamespace("http://cipa.jp/exif/1.0/", "exifEX");
    SXMPMeta::RegisterNamespace("http://ns.adobe.com/camera-raw-saved-settings/1.0/", "crss");
    SXMPMeta::RegisterNamespace("http://www.audio/", "audio");
    SXMPMeta::RegisterNamespace("http://www.video/", "video");
#endif
  }
  return initialized_;
}
#else
bool XmpParser::initialize(XmpParser::XmpLockFct, void*) {
  initialized_ = true;
  return initialized_;
}
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
static XMP_Status nsDumper(void* refCon, XMP_StringPtr buffer, XMP_StringLen bufferSize) {
  XMP_Status result = 0;
  std::string out(buffer, bufferSize);

  // remove blanks: http://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
  out.erase(std::remove_if(out.begin(), out.end(), isspace), out.end());

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
      m[b] = out;
    } else if (m.find(b) != m.end()) {  // store dict[uri] = dict[""]
      m[m[b]] = out;
      m.erase(b);
    }
  }
  return result;
}
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
void XmpParser::registeredNamespaces(Exiv2::Dictionary& dict) {
  bool bInit = !initialized_;
  try {
    if (bInit)
      initialize();
    SXMPMeta::DumpNamespaces(nsDumper, &dict);
    if (bInit)
      terminate();
  } catch (const XMP_Error& e) {
    throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
}
#else
void XmpParser::registeredNamespaces(Exiv2::Dictionary&) {
}
#endif

void XmpParser::terminate() {
  XmpProperties::unregisterNs();
#ifdef EXV_HAVE_XMP_TOOLKIT
  if (initialized_)
    SXMPMeta::Terminate();
#else
  initialized_ = false;
#endif
}

#ifdef EXV_HAVE_XMP_TOOLKIT
void XmpParser::registerNs(const std::string& ns, const std::string& prefix) {
  try {
    initialize();
    AutoLock autoLock(xmpLockFct_, pLockData_);
    SXMPMeta::DeleteNamespace(ns.c_str());
#ifdef EXV_ADOBE_XMPSDK
    SXMPMeta::RegisterNamespace(ns.c_str(), prefix.c_str(), nullptr);
#else
    SXMPMeta::RegisterNamespace(ns.c_str(), prefix.c_str());
#endif
  } catch (const XMP_Error& /* e */) {
    // throw Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg());
  }
}  // XmpParser::registerNs
#else
void XmpParser::registerNs(const std::string& /*ns*/, const std::string& /*prefix*/) {
  initialize();
}  // XmpParser::registerNs
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

#ifdef EXV_HAVE_XMP_TOOLKIT
int XmpParser::decode(XmpData& xmpData, const std::string& xmpPacket) {
  try {
    xmpData.clear();
    xmpData.setPacket(xmpPacket);
    if (xmpPacket.empty())
      return 0;

    if (!initialize()) {
#ifndef SUPPRESS_WARNINGS
      EXV_ERROR << "XMP toolkit initialization failed.\n";
#endif
      return 2;
    }

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
        if (XmpProperties::prefix(schemaNs).empty()) {
          std::string prefix;
          if (!SXMPMeta::GetNamespacePrefix(schemaNs.c_str(), &prefix))
            throw Error(ErrorCode::kerSchemaNamespaceNotRegistered, schemaNs);
          prefix.pop_back();
          XmpProperties::registerNs(schemaNs, prefix);
        }
        continue;
      }
      auto key = makeXmpKey(schemaNs, propPath);
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
          const std::string text = propValue;
          // Get the language qualifier
          haveNext = iter.Next(&schemaNs, &propPath, &propValue, &opt);
          printNode(schemaNs, propPath, propValue, opt);
          if (!haveNext || !XMP_PropIsSimple(opt) || !XMP_PropIsQualifier(opt) ||
              propPath.substr(propPath.size() - 8, 8) != "xml:lang") {
            throw Error(ErrorCode::kerDecodeLangAltQualifierFailed, propPath, opt);
          }
          val->value_[propValue] = text;
        }
        xmpData.add(*key, val.get());
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
          xmpData.add(*key, val.get());
          continue;
        }
      }

      auto val = std::make_unique<XmpTextValue>();
      if (XMP_PropIsStruct(opt) || XMP_PropIsArray(opt)) {
        // Create a metadatum with only XMP options
        val->setXmpArrayType(xmpArrayType(opt));
        val->setXmpStruct(xmpStruct(opt));
        xmpData.add(*key, val.get());
        continue;
      }
      if (XMP_PropIsSimple(opt) || XMP_PropIsQualifier(opt)) {
        val->read(propValue);
        xmpData.add(*key, val.get());
        continue;
      }
      // Don't let any node go by unnoticed
      throw Error(ErrorCode::kerUnhandledXmpNode, key->key(), opt);
    }  // iterate through all XMP nodes

    return 0;
  }
#ifndef SUPPRESS_WARNINGS
  catch (const XMP_Error& e) {
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
    if (xmpData.empty()) {
      xmpPacket.clear();
      return 0;
    }

    if (!initialize()) {
#ifndef SUPPRESS_WARNINGS
      EXV_ERROR << "XMP toolkit initialization failed.\n";
#endif
      return 2;
    }
    // Register custom namespaces with XMP-SDK
    for (const auto& [xmp, uri] : XmpProperties::nsRegistry_) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Registering " << uri.prefix_ << " : " << xmp << "\n";
#endif
      registerNs(xmp, uri.prefix_);
    }
    SXMPMeta meta;
    for (const auto& xmp : xmpData) {
      const std::string ns = XmpProperties::ns(xmp.groupName());
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
        if (xmp.count() == 0) {
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
    xmpPacket = tmpPacket;

    return 0;
  }
#ifndef SUPPRESS_WARNINGS
  catch (const XMP_Error& e) {
    EXV_ERROR << Error(ErrorCode::kerXMPToolkitError, e.GetID(), e.GetErrMsg()) << "\n";
    return 3;
  }
#else
  catch (const XMP_Error&) {
    return 3;
  }
#endif  // SUPPRESS_WARNINGS
}  // XmpParser::decode
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
namespace {
#ifdef EXV_HAVE_XMP_TOOLKIT
Exiv2::XmpValue::XmpStruct xmpStruct(const XMP_OptionBits& opt) {
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

Exiv2::TypeId arrayValueTypeId(const XMP_OptionBits& opt) {
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

Exiv2::XmpValue::XmpArrayType xmpArrayType(const XMP_OptionBits& opt) {
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
               const XMP_OptionBits& opt) {
  static bool first = true;
  if (first) {
    first = false;
    std::cout << "ashisabsals\n"
              << "lcqqtrgqlai\n";
  }
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
void printNode(const std::string&, const std::string&, const std::string&, const XMP_OptionBits&) {
}
#endif  // EXIV2_DEBUG_MESSAGES

Exiv2::XmpKey::UniquePtr makeXmpKey(const std::string& schemaNs, const std::string& propPath) {
  std::string property;
  std::string::size_type idx = propPath.find(':');
  if (idx == std::string::npos) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerPropertyNameIdentificationFailed, propPath, schemaNs);
  }
  // Don't worry about out_of_range, XMP parser takes care of this
  property = propPath.substr(idx + 1);
  std::string prefix = Exiv2::XmpProperties::prefix(schemaNs);
  if (prefix.empty()) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerNoPrefixForNamespace, propPath, schemaNs);
  }
  return std::make_unique<Exiv2::XmpKey>(prefix, property);
}  // makeXmpKey
#endif  // EXV_HAVE_XMP_TOOLKIT

}  // namespace
