#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <exiv2/exiv2.hpp>
#include <properties.hpp>
#include <string>
#include <thread>
#include <vector>

// Test for Concurrent Registered Namespaces and Terminate Race Condition:
// verifiable thread safety issue where registeredNamespaces() calls DumpNamespaces()
// without holding the lifecycle lock (xmpLifecycleMutex).
// If another thread calls terminate() concurrently, the SDK may be destroyed while
// DumpNamespaces() is iterating, leading to undefined behavior or crashes.
TEST(XmpConcurrentRegistry, ConcurrentRegisteredNamespacesAndTerminate) {
  constexpr auto ITERATIONS = 100;
  auto stop = std::atomic<bool>{false};
  auto success_count = std::atomic<int>{0};

  // Thread 1: Repeatedly calls registeredNamespaces()
  auto dumper_worker = [&]() {
    for (int i = 0; i < ITERATIONS && !stop; ++i) {
      try {
        auto dict = Exiv2::Dictionary{};
        Exiv2::XmpProperties::registeredNamespaces(dict);
        success_count++;
      } catch (...) {
        // Ignore errors, we are looking for crashes/races
      }
    }
  };

  // Thread 2: Repeatedly terminates the SDK
  // This mimics an application shutting down or resetting the parser
  // while another thread is inspecting namespaces.
  auto terminator_worker = [&]() {
    for (int i = 0; i < ITERATIONS && !stop; ++i) {
      try {
        Exiv2::XmpParser::terminate();
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        // Re-init to allow dumper to continue
        Exiv2::XmpParser::initialize();
        std::this_thread::sleep_for(std::chrono::microseconds(50));
      } catch (...) {
      }
    }
  };

  auto threads = std::vector<std::thread>{};
  threads.emplace_back(dumper_worker);
  threads.emplace_back(terminator_worker);

  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }

  // If we don't crash, the test passes (but DRD/Helgrind should flag it)
  SUCCEED();
}

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
