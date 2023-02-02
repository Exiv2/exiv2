// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "getopt.hpp"

#include <cstdio>
#include <cstring>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace Util {
// https://raw.githubusercontent.com/skeeto/getopt/master/getopt.h
int optind = 0;
int opterr = 1;
int optopt;
int optpos = 1;
const char* optarg;

/* A minimal POSIX getopt() implementation in ANSI C
 *
 * This is free and unencumbered software released into the public domain.
 *
 * This implementation supports the convention of resetting the option
 * parser by assigning optind to 0. This resets the internal state
 * appropriately.
 *
 * Ref: http://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html
 */

int getopt(int argc, char* const argv[], const char* optstring) {
  const char* arg;
  (void)argc;

  /* Reset? */
  if (optind == 0) {
    optind = 1;
    optpos = 1;
  }

  arg = argv[optind];
  if (arg && strcmp(arg, "--") == 0) {
    optind++;
    return -1;
  }
  if (!arg || arg[0] != '-' || !isalnum(arg[1])) {
    return -1;
  }
  const char* opt = strchr(optstring, arg[optpos]);
  optopt = arg[optpos];
  if (!opt) {
    if (opterr && *optstring != ':')
      fprintf(stderr, "%s: illegal option: %c\n", argv[0], optopt);
    return '?';
  }
  if (opt[1] == ':') {
    if (arg[optpos + 1]) {
      optarg = const_cast<char*>(arg) + optpos + 1;
      optind++;
      optpos = 1;
      return optopt;
    }
    if (argv[optind + 1]) {
      optarg = argv[optind + 1];
      optind += 2;
      optpos = 1;
      return optopt;
    }
    if (opterr && *optstring != ':')
      fprintf(stderr, "%s: option requires an argument: %c\n", argv[0], optopt);
    return *optstring == ':' ? ':' : '?';
  }
  if (!arg[++optpos]) {
    optind++;
    optpos = 1;
  }
  return optopt;
}

// *****************************************************************************
// class Getopt
int Getopt::getopt(int argc, char* const argv[], const std::string& optstring) {
  progname_ = fs::path(argv[0]).filename().string();
  Util::optind = 0;  // reset the Util::Getopt scanner

  while (!errcnt_) {
    int c = Util::getopt(argc, argv, optstring.c_str());
    if (c == -1) {
      break;
    }
    errcnt_ += option(c, Util::optarg ? Util::optarg : "", Util::optopt);
    if (c == '?') {
      break;
    }
  }
  for (int i = Util::optind; i < argc; i++) {
    errcnt_ += nonoption(argv[i]);
  }

  return errcnt_;
}

int Getopt::nonoption(const std::string& /*argv*/) {
  return 0;
}

}  // namespace Util
