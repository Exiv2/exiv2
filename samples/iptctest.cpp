// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <iostream>

using namespace Exiv2;

bool processLine(const std::string& line, int num, IptcData& iptcData);
void processAdd(const std::string& line, int num, IptcData& iptcData);
void processRemove(const std::string& line, int num, IptcData& iptcData);
void processModify(const std::string& line, int num, IptcData& iptcData);

// *****************************************************************************
// Main
int main(int argc, char* const argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
  Exiv2::enableBMFF();
#endif

  try {
    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " image\n";
      std::cout << "Commands read from stdin.\n";
      return EXIT_FAILURE;
    }

    auto image = ImageFactory::open(argv[1]);
    image->readMetadata();

    // Process commands
    std::string line;
    int num = 0;
    std::getline(std::cin, line);
    while (line.length() && processLine(line, ++num, image->iptcData())) {
      std::getline(std::cin, line);
    }

    // Save any changes
    image->writeMetadata();

    return EXIT_SUCCESS;
  } catch (Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}

bool processLine(const std::string& line, int num, IptcData& iptcData) {
  switch (line.at(0)) {
    case 'a':
    case 'A':
      processAdd(line, num, iptcData);
      break;
    case 'r':
    case 'R':
      processRemove(line, num, iptcData);
      break;
    case 'm':
    case 'M':
      processModify(line, num, iptcData);
      break;
    case 'q':
    case 'Q':
      return false;
    default:
      std::ostringstream os;
      os << "Unknown command (" << line.at(0) << ") at line " << num;
      throw Error(ErrorCode::kerErrorMessage, os.str());
  }
  return true;
}

void processAdd(const std::string& line, int num, IptcData& iptcData) {
  std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
  std::string::size_type keyEnd = line.find_first_of(" \t", keyStart + 1);
  std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd + 1);

  if (keyStart == std::string::npos || keyEnd == std::string::npos || dataStart == std::string::npos) {
    std::ostringstream os;
    os << "Invalid \'a\' command at line " << num;
    throw Error(ErrorCode::kerErrorMessage, os.str());
  }

  std::string key(line.substr(keyStart, keyEnd - keyStart));
  IptcKey iptcKey(key);

  std::string data(line.substr(dataStart));
  // if data starts and ends with quotes, remove them
  if (data.at(0) == '\"' && data.at(data.size() - 1) == '\"') {
    data = data.substr(1, data.size() - 2);
  }
  TypeId type = IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
  Value::UniquePtr value = Value::create(type);
  value->read(data);

  int rc = iptcData.add(iptcKey, value.get());
  if (rc) {
    throw Error(ErrorCode::kerErrorMessage, "Iptc dataset already exists and is not repeatable");
  }
}

void processRemove(const std::string& line, int num, IptcData& iptcData) {
  std::string::size_type keyStart = line.find_first_not_of(" \t", 1);

  if (keyStart == std::string::npos) {
    std::ostringstream os;
    os << "Invalid \'r\' command at line " << num;
    throw Error(ErrorCode::kerErrorMessage, os.str());
  }

  const std::string key(line.substr(keyStart));
  IptcKey iptcKey(key);

  auto iter = iptcData.findKey(iptcKey);
  if (iter != iptcData.end()) {
    iptcData.erase(iter);
  }
}

void processModify(const std::string& line, int num, IptcData& iptcData) {
  std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
  std::string::size_type keyEnd = line.find_first_of(" \t", keyStart + 1);
  std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd + 1);

  if (keyStart == std::string::npos || keyEnd == std::string::npos || dataStart == std::string::npos) {
    std::ostringstream os;
    os << "Invalid \'m\' command at line " << num;
    throw Error(ErrorCode::kerErrorMessage, os.str());
  }

  std::string key(line.substr(keyStart, keyEnd - keyStart));
  IptcKey iptcKey(key);

  std::string data(line.substr(dataStart));
  // if data starts and ends with quotes, remove them
  if (data.at(0) == '\"' && data.at(data.size() - 1) == '\"') {
    data = data.substr(1, data.size() - 2);
  }
  TypeId type = IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
  Value::UniquePtr value = Value::create(type);
  value->read(data);

  auto iter = iptcData.findKey(iptcKey);
  if (iter != iptcData.end()) {
    iter->setValue(value.get());
  } else {
    int rc = iptcData.add(iptcKey, value.get());
    if (rc) {
      throw Error(ErrorCode::kerErrorMessage, "Iptc dataset already exists and is not repeatable");
    }
  }
}
