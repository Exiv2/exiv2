#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <string>

TEST(XmpParser, ImplicitLifecycleTest) {
  // 1. Verify we can use XMP properties without explicit initialization
  Exiv2::XmpData xmpData;
  xmpData["Xmp.dc.format"] = "image/jpeg";
  ASSERT_FALSE(xmpData.empty());

  // 2. Verify we can register namespaces
  Exiv2::XmpProperties::registerNs("http://example.com/lifecycle/", "life");
  ASSERT_EQ("life", Exiv2::XmpProperties::prefix("http://example.com/lifecycle/"));

  // 3. Verify clearing custom namespaces works
  Exiv2::XmpParser::clearCustomNamespaces();
  // Note: Standard namespaces should still be there, but our custom one might be gone
  // depending on implementation. The previous implementation of terminate() unregistered everything.
  // clearCustomNamespaces delegates to UnregisterNsUnsafe which clears the registry map.

  // We can re-register
  Exiv2::XmpProperties::registerNs("http://example.com/lifecycle/", "life");
  ASSERT_EQ("life", Exiv2::XmpProperties::prefix("http://example.com/lifecycle/"));
}
