#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <exiv2/exiv2.hpp>
#include <exiv2/properties.hpp>
#include <string>
#include <thread>
#include <vector>

// Test concurrent encode() and decode() operations to trigger race condition #1:
// encode() iterates nsRegistry_ without lock while decode() can modify it

TEST(XmpRace, ConcurrentEncodeDecode) {
  constexpr int ITERATIONS = 50;
  std::atomic<bool> stop{false};
  std::atomic<int> encode_count{0};
  std::atomic<int> decode_count{0};

  // Thread 1: Continuously encode XMP data with custom namespaces
  auto encode_worker = [&](int thread_id) {
    for (int i = 0; i < ITERATIONS && !stop; ++i) {
      try {
        Exiv2::XmpData xmpData;
        auto ns = std::string("http://encode.test/" + std::to_string(thread_id) + "/" + std::to_string(i) + "/");
        auto prefix = std::string("enc" + std::to_string(thread_id) + "_" + std::to_string(i));

        // Register a custom namespace - this populates nsRegistry_
        Exiv2::XmpProperties::registerNs(ns, prefix);

        // Add data using the custom namespace
        xmpData["Xmp." + prefix + ".value"] = "test_value_" + std::to_string(i);
        xmpData["Xmp.dc.format"] = "image/jpeg";

        // encode() will iterate nsRegistry_ WITHOUT a lock
        auto packet = std::string();
        if (Exiv2::XmpParser::encode(packet, xmpData) == 0) {
          encode_count++;
        }
      } catch (const std::exception& e) {
        // Catch exceptions but continue to maximize race detection
      }
    }
  };

  // Thread 2: Continuously decode XMP packets with unknown namespaces
  auto decode_worker = [&](int thread_id) {
    for (int i = 0; i < ITERATIONS && !stop; ++i) {
      try {
        // Create XMP packet with unknown namespace
        // decode() will call XmpProperties::registerNs() when it encounters unknown namespace
        auto unknown_ns =
            std::string("http://decode.test/" + std::to_string(thread_id) + "/" + std::to_string(i) + "/");
        auto unknown_prefix = std::string("dec" + std::to_string(thread_id) + "_" + std::to_string(i));

        auto xmpPacket = std::string(
            "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 4.4.0\">\n"
            " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "  <rdf:Description rdf:about=\"\"\n"
            "    xmlns:" +
            unknown_prefix + "=\"" + unknown_ns +
            "\"\n"
            "    " +
            unknown_prefix + ":test=\"value" + std::to_string(i) +
            "\"/>\n"
            " </rdf:RDF>\n"
            "</x:xmpmeta>\n"
            "<?xpacket end=\"w\"?>");

        Exiv2::XmpData xmpData;
        // decode() will modify nsRegistry_ when it sees the unknown namespace
        if (Exiv2::XmpParser::decode(xmpData, xmpPacket) == 0) {
          decode_count++;
        }
      } catch (const std::exception& e) {
        // Catch exceptions but continue
      }
    }
  };

  // Run with multiple threads to increase race probability
  constexpr int NUM_ENCODE_THREADS = 3;
  constexpr int NUM_DECODE_THREADS = 3;

  std::vector<std::thread> threads;

  for (int i = 0; i < NUM_ENCODE_THREADS; ++i) {
    threads.emplace_back(encode_worker, i);
  }

  for (int i = 0; i < NUM_DECODE_THREADS; ++i) {
    threads.emplace_back(decode_worker, i);
  }

  // Wait for all threads
  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }

  // If we got here without crashing, the test technically passes
  // But Valgrind/ThreadSanitizer should detect the races
  EXPECT_GT(encode_count.load(), 0);
  EXPECT_GT(decode_count.load(), 0);
}

// Test concurrent initialization - race condition #2:
// initialized_ is a plain bool with no synchronization

// Test concurrent encode operations - race condition #3:
// AutoLock doesn't protect XMP SDK operations in encode()

TEST(XmpRace, ConcurrentEncodeEncode) {
  constexpr int ITERATIONS = 300;
  constexpr int NUM_THREADS = 5;

  auto encode_worker = [&](int thread_id) {
    for (int i = 0; i < ITERATIONS; ++i) {
      try {
        Exiv2::XmpData xmpData;
        xmpData["Xmp.dc.format"] = "image/jpeg";
        xmpData["Xmp.dc.title"] = "Thread " + std::to_string(thread_id) + " iteration " + std::to_string(i);

        std::string packet;
        Exiv2::XmpParser::encode(packet, xmpData);
      } catch (...) {
      }
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(encode_worker, i);
  }

  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }

  SUCCEED();
}

// Test concurrent decode operations

TEST(XmpRace, ConcurrentDecodeDecode) {
  constexpr int ITERATIONS = 300;
  constexpr int NUM_THREADS = 5;

  const auto xmpPacket = std::string(
      "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
      "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 4.4.0\">\n"
      " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
      "  <rdf:Description rdf:about=\"\"\n"
      "    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
      "    dc:format=\"image/jpeg\"/>\n"
      " </rdf:RDF>\n"
      "</x:xmpmeta>\n"
      "<?xpacket end=\"w\"?>");

  auto decode_worker = [&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      try {
        Exiv2::XmpData xmpData;
        Exiv2::XmpParser::decode(xmpData, xmpPacket);
      } catch (...) {
      }
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(decode_worker);
  }

  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }

  SUCCEED();
}
