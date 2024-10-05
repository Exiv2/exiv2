// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program to format exif data in various external formats

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <string>

using format_t = std::map<std::string, int>;
enum format_e { wolf, csv, json, xml };

void syntax(const char* argv[], format_t& formats) {
  std::cout << "Usage: " << argv[0] << " file format" << '\n';
  int count = 0;
  std::cout << "formats: ";
  for (auto&& format : formats) {
    std::cout << (count++ ? " | " : "") << format.first;
  }
  std::cout << '\n';
}

size_t formatInit(Exiv2::ExifData& exifData) {
  return std::distance(exifData.begin(), exifData.end());
}

///////////////////////////////////////////////////////////////////////
std::string escapeCSV(Exiv2::ExifData::const_iterator it, bool bValue) {
  std::string result;

  std::ostringstream os;
  if (bValue)
    os << it->value();
  else
    os << it->key();

  std::string s = os.str();
  for (auto&& c : s) {
    if (c == ',')
      result += '\\';
    result += c;
  }

  return result;
}

std::string formatCSV(Exiv2::ExifData& exifData) {
  size_t count = 0;
  size_t length = formatInit(exifData);
  std::ostringstream result;

  for (auto i = exifData.begin(); count++ < length; ++i) {
    result << escapeCSV(i, false) << (count != length ? ", " : "");
  }
  result << '\n';

  count = 0;
  for (auto i = exifData.begin(); count++ < length; ++i) {
    result << escapeCSV(i, true) << (count != length ? ", " : "");
  }
  return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string formatWolf(Exiv2::ExifData& exifData) {
  size_t count = 0;
  size_t length = formatInit(exifData);
  std::ostringstream result;

  result << "{ " << '\n';
  for (auto i = exifData.begin(); count++ < length; ++i) {
    result << "  " << i->key() << " -> " << i->value() << (count != length ? "," : "") << '\n';
  }
  result << "}";
  return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string escapeJSON(Exiv2::ExifData::const_iterator it, bool bValue = true) {
  std::string result;

  std::ostringstream os;
  if (bValue)
    os << it->value();
  else
    os << it->key();

  std::string s = os.str();
  for (auto&& c : s) {
    if (c == '"')
      result += "\\\"";
    result += c;
  }

  std::string q = "\"";
  return q + result + q;
}

std::string formatJSON(Exiv2::ExifData& exifData) {
  size_t count = 0;
  size_t length = formatInit(exifData);
  std::ostringstream result;

  result << "{" << '\n';
  for (auto i = exifData.begin(); count++ < length; ++i) {
    result << "  " << escapeJSON(i, false) << ":" << escapeJSON(i, true) << (count != length ? "," : "") << '\n';
  }
  result << "}";
  return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string escapeXML(Exiv2::ExifData::const_iterator it, bool bValue = true) {
  std::string result;

  std::ostringstream os;
  if (bValue)
    os << it->value();
  else
    os << it->key();

  std::string s = os.str();
  for (auto&& c : s) {
    if (c == '<')
      result += "&lg;";
    if (c == '>')
      result += "&gt;";
    result += c;
  }

  return result;
}

std::string formatXML(Exiv2::ExifData& exifData) {
  size_t count = 0;
  size_t length = formatInit(exifData);
  std::ostringstream result;

  result << "<exif>" << '\n';
  for (auto i = exifData.begin(); count++ < length; ++i) {
    std::string key = escapeXML(i, false);
    std::string value = escapeXML(i, true);
    result << "  <" << key << ">" << value << "<" << key << "/>" << '\n';
  }
  result << "</exif>" << '\n';
  return result.str();
}

///////////////////////////////////////////////////////////////////////
int main(int argc, const char* argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  format_t formats;
  formats["wolf"] = wolf;
  formats["csv"] = csv;
  formats["json"] = json;
  formats["xml"] = xml;

  int result = 0;
  if (argc != 3) {
    syntax(argv, formats);
    result = 1;
  }

  const char* file = argv[1];
  const char* format = argv[2];

  if (!result && formats.find(format) == formats.end()) {
    std::cout << "Unrecognised format " << format << '\n';
    syntax(argv, formats);
    result = 2;
  }

  if (!result)
    try {
      auto image = Exiv2::ImageFactory::open(file);
      image->readMetadata();
      Exiv2::ExifData& exifData = image->exifData();

      switch (formats.find(format)->second) {
        case wolf:
          std::cout << formatWolf(exifData) << '\n';
          break;
        case csv:
          std::cout << formatCSV(exifData) << '\n';
          break;
        case json:
          std::cout << formatJSON(exifData) << '\n';
          break;
        case xml:
          std::cout << formatXML(exifData) << '\n';
          break;

        default:
          std::cout << "*** error: format not implemented yet: " << format << " ***" << '\n';
          result = 3;
          break;
      }
    } catch (Exiv2::Error& e) {
      std::cerr << "*** error exiv2 exception '" << e << "' ***" << '\n';
      result = 4;
    } catch (...) {
      std::cerr << "*** error exception" << '\n';
      result = 5;
    }

  return result;
}
