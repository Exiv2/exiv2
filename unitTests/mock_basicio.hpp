// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_MOCK_BASICIO_HPP
#define EXIV2_MOCK_BASICIO_HPP

#include <exiv2/basicio.hpp>

#include <gmock/gmock.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace Exiv2 {
class MockBasicIo : public BasicIo {
 public:
  MockBasicIo() {
    // Sane defaults so the mock is usable without per-method setup. Tests
    // override these with ON_CALL as needed.
    ON_CALL(*this, path()).WillByDefault(::testing::ReturnRef(defaultPath_));
    ON_CALL(*this, open()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, close()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, error()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, eof()).WillByDefault(::testing::Return(false));
    ON_CALL(*this, seek(::testing::_, ::testing::_)).WillByDefault(::testing::Return(0));
    ON_CALL(*this, tell()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, size()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, isopen()).WillByDefault(::testing::Return(false));
    ON_CALL(*this, getb()).WillByDefault(::testing::Return(-1));
    ON_CALL(*this, putb(::testing::_)).WillByDefault(::testing::Return(0));
    ON_CALL(*this, write(::testing::_, ::testing::_)).WillByDefault(::testing::Return(0));
    ON_CALL(*this, write(::testing::_)).WillByDefault(::testing::Return(0));
    ON_CALL(*this, munmap()).WillByDefault(::testing::Return(0));
    ON_CALL(*this, populateFakeData()).WillByDefault(::testing::Return());
    // Default both read() overloads: byte* read returns no data, DataBuf
    // read returns an empty buffer, so the mock is usable as-is.
    ON_CALL(*this, read(::testing::_, ::testing::_)).WillByDefault(::testing::Return(0));
    ON_CALL(*this, read(::testing::_)).WillByDefault(::testing::Return(DataBuf()));
  }

  MOCK_METHOD(int, open, (), (override));
  MOCK_METHOD(int, close, (), (override));

  MOCK_METHOD(size_t, write, (const byte* data, size_t wcount), (override));
  MOCK_METHOD(size_t, write, (BasicIo & src), (override));
  MOCK_METHOD(int, putb, (byte data), (override));

  MOCK_METHOD(DataBuf, read, (size_t rcount), (override));
  MOCK_METHOD(size_t, read, (byte * buf, size_t rcount), (override));
  MOCK_METHOD(int, getb, (), (override));

  MOCK_METHOD(void, transfer, (BasicIo & src), (override));
  MOCK_METHOD(int, seek, (int64_t offset, Position pos), (override));

  MOCK_METHOD(byte*, mmap, (bool isWriteable), (override));
  MOCK_METHOD(int, munmap, (), (override));

  MOCK_METHOD(size_t, tell, (), (const, override));
  MOCK_METHOD(size_t, size, (), (const, override));
  MOCK_METHOD(bool, isopen, (), (const, override));
  MOCK_METHOD(int, error, (), (const, override));
  MOCK_METHOD(bool, eof, (), (const, override));
  MOCK_METHOD(const std::string&, path, (), (const, noexcept, override));

  MOCK_METHOD(void, populateFakeData, (), (override));

  // ── Helper: change the path returned by path() ────────────────────
  void setDefaultPath(const std::string& p) {
    defaultPath_ = p;
  }

 private:
  std::string defaultPath_ = "mock";
};

/// Create a NiceMock<MockBasicIo> in a known-good default state.
inline std::unique_ptr<::testing::NiceMock<MockBasicIo>> makeMockIo(const std::string& path = "mock") {
  auto mockIo = std::make_unique<::testing::NiceMock<MockBasicIo>>();
  mockIo->setDefaultPath(path);
  return mockIo;
}

/// Make open() fail, as if the underlying file/stream could not be opened.
inline void setupOpenFailure(MockBasicIo& mockIo) {
  ON_CALL(mockIo, open()).WillByDefault(::testing::Return(1));
}

/// Make every read() return 0 bytes, simulating an I/O error / empty source.
/// Overrides any prior setupRead() on both read() overloads.
inline void setupReadFailure(MockBasicIo& mockIo) {
  ON_CALL(mockIo, read(::testing::_, ::testing::_)).WillByDefault(::testing::Return(0));
  ON_CALL(mockIo, read(::testing::_)).WillByDefault(::testing::Return(DataBuf()));
  ON_CALL(mockIo, eof()).WillByDefault(::testing::Return(true));
}

namespace detail {
/// Set up read() to return `bytes`; short reads report the actual byte count,
/// mirroring BasicIo::read. The buffer is zero-filled beyond the returned data.
///
/// Note: the stub is position-agnostic. It ignores seek()/tell() and always
/// returns from the start of `bytes`, so it suits one-shot signature checks
/// (isXxxType) rather than tests that perform sequential reads.
inline void setupReadImpl(MockBasicIo& mockIo, std::vector<byte> bytes) {
  ON_CALL(mockIo, read(::testing::_, ::testing::_))
      .WillByDefault([bytes = std::move(bytes)](byte* buf, size_t count) mutable {
        auto n = std::min(count, bytes.size());
        std::memcpy(buf, bytes.data(), n);
        if (count > n)
          std::memset(buf + n, 0, count - n);
        return n;
      });
}
}  // namespace detail

inline void setupRead(MockBasicIo& mockIo, std::initializer_list<byte> bytes) {
  detail::setupReadImpl(mockIo, std::vector<byte>(bytes.begin(), bytes.end()));
}

/// Set up read() from any contiguous byte range (e.g. std::array / std::vector).
template <typename ByteRange>
inline void setupRead(MockBasicIo& mockIo, const ByteRange& bytes) {
  detail::setupReadImpl(mockIo, std::vector<byte>(std::begin(bytes), std::end(bytes)));
}
}  // namespace Exiv2

#endif  // EXIV2_MOCK_BASICIO_HPP
