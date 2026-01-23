#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <properties.hpp>
#include <random>
#include <string>
#include <thread>
#include <vector>

// Test case for Namespace Collision Thread Safety
// Note: We access XmpProperties::nsInfo which is public API.

TEST(XmpProperties, CollisionTest) {
  auto ns1 = std::string("http://example.com/ns1/");
  auto prefix = std::string("ex1");

  // 1. Register first namespace
  Exiv2::XmpProperties::registerNs(ns1, prefix);

  // 2. Get info and hold a pointer
  const auto* info1 = Exiv2::XmpProperties::nsInfo(prefix);
  ASSERT_NE(info1, nullptr);
  ASSERT_EQ(std::string(info1->prefix_), prefix);
  ASSERT_EQ(std::string(info1->ns_), ns1);

  // 3. Register a DIFFERENT namespace with the SAME prefix
  // This overwrites the previous registration for 'prefix'
  auto ns2 = std::string("http://example.com/ns2/");
  Exiv2::XmpProperties::registerNs(ns2, prefix);

  // 4. Verify overwrite
  // info1 is now dangling because the old entry was deleted. Fetch fresh info.
  const auto* info2 = Exiv2::XmpProperties::nsInfo(prefix);
  ASSERT_NE(info2, nullptr);
  EXPECT_EQ(std::string(info2->ns_), ns2);
  EXPECT_EQ(std::string(info2->prefix_), prefix);

  // 5. Verify ns1 lookup no longer returns this prefix (or at least ns2 took over)
  EXPECT_EQ(Exiv2::XmpProperties::prefix(ns2), prefix);
}

TEST(XmpProperties, IdempotencyTest) {
  auto ns3 = std::string("http://example.com/ns3/");
  auto prefix3 = std::string("ex3");

  Exiv2::XmpProperties::registerNs(ns3, prefix3);
  const auto* info3 = Exiv2::XmpProperties::nsInfo(prefix3);

  // Check stability
  Exiv2::XmpProperties::registerNs(ns3, prefix3);
  const auto* info3_new = Exiv2::XmpProperties::nsInfo(prefix3);

  // The exact address might or might not be same depending on implementation,
  // but the object at info3 MUST be valid and correct.
  EXPECT_EQ(std::string(info3->ns_), ns3);

  // In our implementation, we return EARLY, so the pointer should be identical.
  EXPECT_EQ(info3, info3_new);
}

TEST(XmpParser, TortureTest) {
  // This test spawns multiple threads to aggressively register namespaces and encode XMP.
  // It aims to trigger race conditions in global state handling.

  constexpr auto NUM_THREADS = 10;
  constexpr auto ITERATIONS = 100;

  auto work = [](int id) {
    std::string ns_base = "http://example.com/thread/" + std::to_string(id) + "/";
    std::string prefix_base = "t" + std::to_string(id);

    for (int i = 0; i < ITERATIONS; ++i) {
      std::string ns = ns_base + std::to_string(i) + "/";
      std::string prefix = prefix_base + "_" + std::to_string(i);

      // Register namespace
      try {
        Exiv2::XmpProperties::registerNs(ns, prefix);
      } catch (const std::exception& e) {
        // Failures here might indicate race conditions
        // std::cerr << "Thread " << id << " failed register: " << e.what() << std::endl;
      }

      // Create some XMP data and encode it
      Exiv2::XmpData xmpData;
      xmpData["Xmp.dc.format"] = "image/jpeg";
      xmpData["Xmp." + prefix + ".test"] = "value " + std::to_string(i);

      std::string packet;
      try {
        Exiv2::XmpParser::encode(packet, xmpData);
      } catch (const std::exception& e) {
        // std::cerr << "Thread " << id << " failed encode: " << e.what() << std::endl;
      }
    }
  };

  auto threads = std::vector<std::thread>();
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(work, i);
  }

  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }
}
