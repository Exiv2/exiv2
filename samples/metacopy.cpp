// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

// include local header files which are not part of libexiv2
#include "metacopy.hpp"

// *****************************************************************************
// Main
int main(int argc, char* const argv[]) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    // Handle command line arguments
    Params params;
    if (params.getopt(argc, argv)) {
      params.usage();
      return 1;
    }
    if (params.help_) {
      params.help();
      return 2;
    }

    // Use MemIo to increase test coverage.
    Exiv2::FileIo fileIo(params.read_);
    auto memIo = std::make_unique<Exiv2::MemIo>();
    memIo->transfer(fileIo);

    auto readImg = Exiv2::ImageFactory::open(std::move(memIo));
    readImg->readMetadata();

    auto writeImg = Exiv2::ImageFactory::open(params.write_);
    if (params.preserve_) {
      writeImg->readMetadata();
    }
    if (params.iptc_) {
      writeImg->setIptcData(readImg->iptcData());
    }
    if (params.exif_) {
      writeImg->setExifData(readImg->exifData());
    }
    if (params.comment_) {
      writeImg->setComment(readImg->comment());
    }
    if (params.xmp_) {
      writeImg->setXmpData(readImg->xmpData());
    }

    try {
      writeImg->writeMetadata();
    } catch (const Exiv2::Error&) {
      std::cerr << params.progname() << ": Could not write metadata to (" << params.write_ << ")\n";
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cerr << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}

int Params::option(int opt, const std::string& /*optarg*/, int optopt) {
  int rc = 0;
  switch (opt) {
    case 'h': {
      help_ = true;
      break;
    }
    case 'i': {
      iptc_ = true;
      break;
    }
    case 'e': {
      exif_ = true;
      break;
    }
    case 'c': {
      comment_ = true;
      break;
    }
    case 'x': {
      xmp_ = true;
      break;
    }
    case 'p': {
      preserve_ = true;
      break;
    }
    case 'a': {
      iptc_ = true;
      exif_ = true;
      comment_ = true;
      xmp_ = true;
      break;
    }
    case ':': {
      std::cerr << progname() << ": Option -" << static_cast<char>(optopt) << " requires an argument\n";
      rc = 1;
      break;
    }
    case '?': {
      std::cerr << progname() << ": Unrecognized option -" << static_cast<char>(optopt) << "\n";
      rc = 1;
      break;
    }
    default: {
      std::cerr << progname() << ": getopt returned unexpected character code " << std::hex << opt << "\n";
      rc = 1;
      break;
    }
  }

  return rc;
}

int Params::nonoption(const std::string& argv) {
  if (!write_.empty()) {
    std::cerr << progname() << ": Unexpected extra argument (" << argv << ")\n";
    return 1;
  }
  if (first_)
    read_ = argv;
  else
    write_ = argv;
  first_ = false;
  return 0;
}

int Params::getopt(int argc, char* const argv[]) {
  int rc = Util::Getopt::getopt(argc, argv, optstring_);
  // Further consistency checks
  if (!help_) {
    if (rc == 0 && read_.empty()) {
      std::cerr << progname() << ": Read and write files must be specified\n";
      rc = 1;
    }
    if (rc == 0 && write_.empty()) {
      std::cerr << progname() << ": Write file must be specified\n";
      rc = 1;
    }
    if (preserve_ && iptc_ && exif_ && comment_ && xmp_) {
      std::cerr << progname() << ": Option -p has no effect when all metadata types are specified.\n";
      rc = 1;
    }
  }
  return rc;
}  // Params::getopt

void Params::usage(std::ostream& os) const {
  os << "\nReads and writes raw metadata. Use -h option for help.\n"
     << "Usage: " << progname() << " [-iecxaph] readfile writefile\n";
}

void Params::help(std::ostream& os) const {
  usage(os);
  os << "\nOptions:\n"
     << "   -i      Read Iptc data from readfile and write to writefile.\n"
     << "   -e      Read Exif data from readfile and write to writefile.\n"
     << "   -c      Read Jpeg comment from readfile and write to writefile.\n"
     << "   -x      Read XMP data from readfile and write to writefile.\n"
     << "   -a      Read all metadata from readfile and write to writefile.\n"
     << "   -p      Preserve existing metadata in writefile if not replaced.\n"
     << "   -h      Display this help and exit.\n\n";
}  // Params::help
