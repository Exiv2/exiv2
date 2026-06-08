// Regression tests for the XMP namespace-leak bug.
//
// Symptom (as originally reported): once exiv2 reads an image whose XMP packet
// binds a namespace prefix to a particular URI, that URI starts appearing in
// every subsequently written file -- even unrelated ones. The trigger in the
// wild is a *corrupted* URI: a packet that binds a standard prefix (e.g. xmp,
// xmpMM) to a typo'd URI poisons exiv2's process-global namespace registry, and
// because the URI is re-derived from that registry at encode() time, the
// corruption leaks into later writes.
//
// Reporter's reproduction sketch (downloads a NASA image, flips bytes in its
// XMP to corrupt the xmp/xmpMM namespace URIs, then writes a *different* clean
// image and observes the corruption in its output):
//   https://gist.github.com/mrled/4c9e0ac820fd47d697138689f7474d90
//
// Root cause and fix:
//   - decode() captures each prefix->URI binding from the packet onto the
//     XmpData itself (XmpData::nsBindings), and refuses to let a read rebind a
//     prefix that is already globally bound (built-in or explicitly registered)
//     to a different URI.
//   - encode() resolves each property's URI from its own XmpData's bindings
//     first, falling back to the global registry only for keys added without a
//     source packet.
//
// gtest runs every TEST in one process, so the cross-call global-state effects
// these tests exercise are real. Where ordering matters the whole sequence is
// packed into one TEST.

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <regex>
#include <string>

namespace {

// Return the URI bound to a given xmlns prefix in a serialized packet.
std::string uriForPrefix(const std::string& packet, const std::string& prefix) {
  std::regex re("xmlns:" + prefix + R"RE(\s*=\s*"([^"]*)")RE");
  std::smatch m;
  if (std::regex_search(packet, m, re))
    return m[1].str();
  return "<none>";
}

}  // namespace

// ---------------------------------------------------------------------------
// The reporter's exact pipeline, distilled: read a poisoned file that binds the
// standard xmp/xmpMM prefixes to corrupted URIs, then read a CLEAN victim file,
// set two properties, and re-encode. The clean file's output must NOT contain
// the corrupted URIs from the poisoned file.
// ---------------------------------------------------------------------------
TEST(XmpNsLeak, ReporterPoisonedFileDoesNotLeakIntoCleanFile) {
  const std::string poisoned = R"(<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>)"
                               R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
                               R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
                               R"(<rdf:Description rdf:about="")"
                               R"( xmlns:xmp="http://ns.abobe.com/xap/1.0/")"       // adobe -> abobe
                               R"( xmlns:xmpMM="(ttp://ns.adobe.com/xap/1.0/mm/")"  // http  -> (ttp
                               R"( xmp:Rating="3")"
                               R"( xmpMM:DocumentID="uuid:original"/>)"
                               R"(</rdf:RDF></x:xmpmeta><?xpacket end="w"?>)";

  // Faithful victim: like the real NASA image, it carries xmp AND xmpMM with
  // their correct URIs (the DocumentID below is overwritten by the reporter).
  const std::string victim = R"(<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>)"
                             R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
                             R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
                             R"(<rdf:Description rdf:about="")"
                             R"( xmlns:xmp="http://ns.adobe.com/xap/1.0/")"       // correct
                             R"( xmlns:xmpMM="http://ns.adobe.com/xap/1.0/mm/")"  // correct
                             R"( xmlns:dc="http://purl.org/dc/elements/1.1/")"
                             R"( xmp:CreatorTool="cleantool")"
                             R"( xmpMM:InstanceID="uuid:clean-instance")"
                             R"( dc:format="image/jpeg"/>)"
                             R"(</rdf:RDF></x:xmpmeta><?xpacket end="w"?>)";

  // Read the poisoned file first (discard its data; keep global side-effects).
  {
    Exiv2::XmpData poisonedXmp;
    Exiv2::XmpParser::decode(poisonedXmp, poisoned);
  }

  // Read the clean victim file second, set the two properties the reporter sets.
  Exiv2::XmpData xmp;
  ASSERT_EQ(0, Exiv2::XmpParser::decode(xmp, victim));
  xmp["Xmp.xmp.Rating"] = "1";
  xmp["Xmp.xmpMM.DocumentID"] = "xmp.did:exiv2-namespace-poison-repro";

  std::string packet;
  ASSERT_EQ(0, Exiv2::XmpParser::encode(packet, xmp, Exiv2::XmpParser::omitPacketWrapper));

  EXPECT_EQ("http://ns.adobe.com/xap/1.0/", uriForPrefix(packet, "xmp"))
      << "xmp prefix bound to corrupted URI in the clean file's output";
  EXPECT_EQ("http://ns.adobe.com/xap/1.0/mm/", uriForPrefix(packet, "xmpMM"))
      << "xmpMM prefix bound to corrupted URI in the clean file's output";
  EXPECT_EQ(std::string::npos, packet.find("abobe")) << "corrupted 'abobe' leaked: " << packet;
  EXPECT_EQ(std::string::npos, packet.find("(ttp")) << "corrupted '(ttp' leaked: " << packet;
}

// ---------------------------------------------------------------------------
// A read must not rebind a prefix that is already globally bound (here, an
// explicitly registered one) to a different URI. After decoding a packet that
// rebinds the prefix to a corrupted URI, XmpProperties::ns() must still return
// the original, canonical URI.
// ---------------------------------------------------------------------------
TEST(XmpNsLeak, PoisonedReadDoesNotRebindRegisteredPrefix) {
  using Exiv2::XmpProperties;

  // Unique prefix so this test is independent of other tests' global state.
  const std::string good = "http://ns.adobe.com/xleak/1.0/";
  XmpProperties::registerNs(good, "xleak");
  ASSERT_EQ(good, XmpProperties::ns("xleak"));

  // Decode a packet that binds the SAME prefix to a corrupted URI.
  const std::string poison = R"(<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>)"
                             R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
                             R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
                             R"(<rdf:Description rdf:about="")"
                             R"( xmlns:xleak="http://ns.abobe.com/xleak/1.0/")"
                             R"( xleak:Foo="bar"/>)"
                             R"(</rdf:RDF></x:xmpmeta><?xpacket end="w"?>)";
  Exiv2::XmpData d;
  Exiv2::XmpParser::decode(d, poison);

  EXPECT_EQ(good, XmpProperties::ns("xleak"))
      << "decode() of a poisoned packet rebound prefix 'xleak' in the global registry";
}

// ---------------------------------------------------------------------------
// The legitimate case behind the bug: two DIFFERENT images each bind the SAME
// custom prefix to a DIFFERENT (valid) URI. Exiv2 keys are prefix-based, so the
// global registry can only hold one binding at a time -- but with per-instance
// bindings each image must still serialize its own URI, even when both are read
// in the same process before either is written.
// ---------------------------------------------------------------------------
TEST(XmpNsLeak, PerImageNamespaceBindingsAreIndependent) {
  auto packetFor = [](const std::string& uri) {
    return std::string(R"(<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>)") +
           R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
           R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
           R"(<rdf:Description rdf:about="")"
           R"( xmlns:shared=")" +
           uri +
           R"(")"
           R"( shared:Tag="v"/>)"
           R"(</rdf:RDF></x:xmpmeta><?xpacket end="w"?>)";
  };
  const std::string uriA = "http://example.com/A/1.0/";
  const std::string uriB = "http://example.com/B/1.0/";

  // Read both images (same prefix, different valid URIs) before writing either.
  Exiv2::XmpData a;
  ASSERT_EQ(0, Exiv2::XmpParser::decode(a, packetFor(uriA)));
  Exiv2::XmpData b;
  ASSERT_EQ(0, Exiv2::XmpParser::decode(b, packetFor(uriB)));

  std::string pa, pb;
  ASSERT_EQ(0, Exiv2::XmpParser::encode(pa, a, Exiv2::XmpParser::omitPacketWrapper));
  ASSERT_EQ(0, Exiv2::XmpParser::encode(pb, b, Exiv2::XmpParser::omitPacketWrapper));

  EXPECT_EQ(uriA, uriForPrefix(pa, "shared")) << "image A serialized the wrong URI for 'shared'";
  EXPECT_EQ(uriB, uriForPrefix(pb, "shared")) << "image B serialized the wrong URI for 'shared'";
}

// ---------------------------------------------------------------------------
// A property added PROGRAMMATICALLY in a namespace the source packet never
// declared has no per-instance binding, so encode() falls back to the global
// registry. After a poisoned read, that fallback must still resolve the
// canonical URI for a standard prefix (the global hardening in decode()).
// ---------------------------------------------------------------------------
TEST(XmpNsLeak, ProgrammaticAddUsesCanonicalUriAfterPoison) {
  const std::string poison = R"(<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>)"
                             R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
                             R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
                             R"(<rdf:Description rdf:about="")"
                             R"( xmlns:xmpMM="(ttp://ns.adobe.com/xap/1.0/mm/")"
                             R"( xmpMM:DocumentID="uuid:original"/>)"
                             R"(</rdf:RDF></x:xmpmeta><?xpacket end="w"?>)";
  {
    Exiv2::XmpData p;
    Exiv2::XmpParser::decode(p, poison);
  }

  // Victim never declared xmpMM; we add it programmatically.
  Exiv2::XmpData d;
  d["Xmp.dc.creator"] = "Alice";
  d["Xmp.xmpMM.DocumentID"] = "xmp.did:fresh";
  std::string packet;
  ASSERT_EQ(0, Exiv2::XmpParser::encode(packet, d, Exiv2::XmpParser::omitPacketWrapper));

  EXPECT_EQ("http://ns.adobe.com/xap/1.0/mm/", uriForPrefix(packet, "xmpMM"))
      << "programmatic add fell back to the poisoned global binding";
}
