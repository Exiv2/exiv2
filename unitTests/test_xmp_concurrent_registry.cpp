#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <exiv2/exiv2.hpp>
#include <exiv2/properties.hpp>
#include <string>
#include <thread>
#include <vector>

// Test for Concurrent Registered Namespaces and Terminate Race Condition:
// verifiable thread safety issue where registeredNamespaces() calls DumpNamespaces()
// without holding the lifecycle lock (xmpLifecycleMutex).
// If another thread calls terminate() concurrently, the SDK may be destroyed while
// DumpNamespaces() is iterating, leading to undefined behavior or crashes.

// Test for Concurrent Decode and PrintNode Race Condition:
// verifiable data race on a static variable inside printNode() function
// which is used when EXIV2_DEBUG_MESSAGES is defined.
// This function is typically called deep inside encode/decode operations.
// We trigger it by running concurrent decodes of data that produces output to ensuring
// that the static variable access is thread-safe.
TEST(XmpConcurrentRegistry, ConcurrentDecodePrintNode) {
  constexpr auto ITERATIONS = 200;
  constexpr auto NUM_THREADS = 4;

  const auto xmpPacket = std::string(
      "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
      "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 4.4.0\">\n"
      " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
      "  <rdf:Description rdf:about=\"\"\n"
      "    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
      "    dc:format=\"image/jpeg\"\n"
      "    dc:description=\"Test Description\"\n"
      "    dc:title=\"Test Title\"/>\n"
      " </rdf:RDF>\n"
      "</x:xmpmeta>\n"
      "<?xpacket end=\"w\"?>");

  auto worker = [&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      try {
        auto xmpData = Exiv2::XmpData{};
        Exiv2::XmpParser::decode(xmpData, xmpPacket);
      } catch (...) {
      }
    }
  };

  auto threads = std::vector<std::thread>{};
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(worker);
  }

  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }

  SUCCEED();
}

// Test for Deadlock during Encode with Concurrent Registration
// Scenario:
// Thread 1: Calls encode(), which iterates XmpProperties::nsRegistry_ under lock
// Thread 2: Calls registerNs(), which acquires lock.
// With Giant Lock, this should just serialize, no deadlock.
TEST(XmpConcurrentRegistry, EncodeDeadlockReproduction) {
  auto stop = std::atomic<bool>{false};
  Exiv2::XmpData xmpData;
  xmpData["Xmp.dc.title"] = "Test";

  auto encoder_worker = [&]() {
    std::string packet;
    while (!stop) {
      try {
        Exiv2::XmpParser::encode(packet, xmpData);
      } catch (...) {
      }
      std::this_thread::yield();
    }
  };

  auto register_worker = [&]() {
    int i = 0;
    while (!stop) {
      std::string prefix = "test" + std::to_string(i++);
      try {
        Exiv2::XmpProperties::registerNs("http://test.com/" + prefix, prefix);
      } catch (...) {
      }
      std::this_thread::yield();
    }
  };

  std::thread t1(encoder_worker);
  std::thread t2(register_worker);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  stop = true;
  if (t1.joinable())
    t1.join();
  if (t2.joinable())
    t2.join();
  SUCCEED();
}
