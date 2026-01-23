#include <gtest/gtest.h>
#include <chrono>
#include <exiv2/exiv2.hpp>
#include <string>
#include <thread>

TEST(XmpParser, LifecycleTest) {
  // 1. Initialize
  ASSERT_TRUE(Exiv2::XmpParser::initialize());

  // 2. Terminate
  Exiv2::XmpParser::terminate();

  // 3. Initialize again
  // This previously failed (returned true but didn't actually re-init the SDK because of call_once)
  // or failed because initialized_ was never reset.
  ASSERT_TRUE(Exiv2::XmpParser::initialize());

  // 4. Verify SDK is alive by registering a namespace
  // If SDK is dead, this might crash or throw
  try {
    Exiv2::XmpProperties::registerNs("http://example.com/lifecycle/", "life");
  } catch (const Exiv2::Error& e) {
    FAIL() << "Failed to register namespace after re-initialization: " << e.what();
  } catch (...) {
    FAIL() << "Unknown exception after re-initialization";
  }

  // Cleanup
  Exiv2::XmpParser::terminate();
}
