// SPDX-License-Identifier: GPL-2.0-or-later

// Sample program to test getopt()
// Command:
// $ getopt-test -v pr -P EIXxgklnycsvth file1 file2
//
// Output:
// standard getopt()
// 118 = v optind = 2 opterr = 1 optopt = 118 optarg = unknown
// -1 optind = 2 opterr = 1 optopt = 118 optarg = unknown
//
// homemade getopt()
// 118 = v optind = 2 opterr = 1 optopt = 118 optarg = unknown
// -1 optind = 2 opterr = 1 optopt = 118 optarg = unknown
//
// Params::option() opt = 118 optarg =  optopt = 118
// Params::nonoption() pr
// Params::nonoption() -P
// Params::nonoption() EIXxgklnycsvth
// Params::nonoption() file1
// Params::nonoption() file2
// Params::getopt() rc = 0

#include <exiv2/exiv2.hpp>

// getopt.{cpp|hpp} is not part of libexiv2
#include "getopt.hpp"

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#include <iostream>

#define Safe(x) ((x) ? (x) : "unknown")
const auto optstring = ":hVvqfbuktTFa:Y:O:D:r:p:P:d:e:i:c:m:M:l:S:g:K:n:Q:";

// *****************************************************************************
// class Params
class Params : public Util::Getopt {
 public:
  /*!
    @brief Call Getopt::getopt() with optstring, to initiate command line
           argument parsing, perform consistency checks after all command line
           arguments are parsed.

    @param argc Argument count as passed to main() on program invocation.
    @param argv Argument array as passed to main() on program invocation.

    @return 0 if successful, >0 in case of errors.
   */
  int getopt(int argc, char** const argv) {
    int rc = Util::Getopt::getopt(argc, argv, ::optstring);
    std::cout << "Params::getopt()"
              << " rc = " << rc << '\n';
    return rc;
  }

  //! Handle options and their arguments.
  int option(int opt, const std::string& optarg, int optopt) override {
    std::cout << "Params::option()"
              << " opt = " << opt << " optarg = " << optarg << " optopt = " << optopt << '\n';
    return 0;
  }

  //! Handle non-option parameters.
  int nonoption(const std::string& argv) override {
    std::cout << "Params::nonoption()"
              << " " << argv << '\n';
    return 0;
  }
};  // class Params

int main(int argc, char** const argv) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  int n;

#if __has_include(<unistd.h>)
  std::cout << "standard getopt()" << '\n';
  do {
    n = ::getopt(argc, argv, ::optstring);
    if (n >= 0) {
      auto N = static_cast<char>(n);
      std::cout << n << " = " << N;
    } else {
      std::cout << n;
    }
    std::cout << " optind = " << ::optind << " opterr = " << ::opterr << " optopt = " << ::optopt
              << " optarg = " << Safe(::optarg) << '\n';
  } while (n >= 0);
  std::cout << '\n';
#endif

  std::cout << "homemade getopt()" << '\n';
  do {
    n = Util::getopt(argc, argv, ::optstring);
    if (n >= 0) {
      auto N = static_cast<char>(n);
      std::cout << n << " = " << N;
    } else {
      std::cout << n;
    }
    std::cout << " optind = " << Util::optind << " opterr = " << Util::opterr << " optopt = " << Util::optopt
              << " optarg = " << Safe(Util::optarg) << '\n';

  } while (n >= 0);
  std::cout << '\n';

  // Handle command line arguments
  Params params;
  params.getopt(argc, argv);

  return EXIT_SUCCESS;
}
