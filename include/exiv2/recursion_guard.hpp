// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_RECURSION_GUARD_HPP
#define EXIV2_RECURSION_GUARD_HPP

// *****************************************************************************
#include <cstddef>
#include "error.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

/*!
  @brief RecursionLimit is used in combination with RecursionGuard
  (below) to prevent excessively deep recursion (which could cause
  stack exhaustion if Exiv2 is run on a very deeply nested file.
  The Image class has a field of type RecursionLimit which tracks
  the remaining number of recursions allowed. It is decremented at
  the beginning of every recursive function and incremented on exit.
  An exception is thrown if the zero is reached. The incrementing
  and decrementing is done by RecursionGuard.
 */
class EXIV2API RecursionLimit final {
  friend class RecursionGuard;

 public:
  explicit RecursionLimit(size_t limit) : remaining_(limit) {
  }

  size_t remaining() const {
    return remaining_;
  }

 private:
  //! Remaining quota of recursive calls. Gets decremented at the
  //! beginning of a recursive function and incremented at the end. An
  //! exception will be thrown if this number hits zero.
  size_t remaining_;
};  // class RecursionLimit

/*!
  @brief Used to prevent excessively deep recursion (which could
  cause stack exhaustion if Exiv2 is run on a very deeply nested file.

  This class is intended to be declared as a local variable at the
  beginning of a recursive method. Its constructor is given a
  reference to a variable of type RecursionLimit. The RecursionLimit
  reference is decremented by RecursionGuard's constructor and
  incremented back to its original value by RecursionGuard's destructor.
  An exception is thrown if the limit drops to zero.

  Usage: add this statement to the beginning of every recursive
  method of an Image class:

  RECURSION_GUARD(recursion_limit_);
*/
class EXIV2API RecursionGuard final {
 public:
  explicit RecursionGuard(RecursionLimit& limit) : limit_(limit) {
    if (limit_.remaining_ == 0) {
      throw Error(ErrorCode::kerMaxRecursionDepth);
    }
    limit_.remaining_--;
  }

  ~RecursionGuard() {
    limit_.remaining_++;
  }

  // Prevent copying
  RecursionGuard() = delete;
  RecursionGuard(const RecursionGuard&) = delete;
  RecursionGuard& operator=(const RecursionGuard&) = delete;
  RecursionGuard(RecursionGuard&&) = delete;
  RecursionGuard& operator=(RecursionGuard&&) = delete;

 private:
  RecursionLimit& limit_;
};  // class RecursionGuard

}  // namespace Exiv2

#define RECURSION_GUARD(limit) Exiv2::RecursionGuard _recursion_guard(limit)

#endif  // EXIV2_RECURSION_GUARD_HPP
