// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program to read gpx files and update images with GPS tags

#include <exiv2/exiv2.hpp>

#include <expat.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

namespace {
// prototypes
class Options;
int getFileType(const fs::path& path, Options& options);

std::string getExifTime(time_t t);
time_t parseTime(const char*, bool bAdjust = false);
int timeZoneAdjust();

// Command-line parser
class Options {
 public:
  bool verbose{false};
  bool help{false};
  bool version{false};
  bool dst{false};
  bool dryrun{false};
  bool ascii{false};

  Options() = default;
  virtual ~Options() = default;
};

enum {
  resultOK = 0,
  resultSyntaxError,
  resultSelectFailed,
};

// keyword indices
enum {
  kwHELP = 0,
  kwVERSION,
  kwDST,
  kwDRYRUN,
  kwASCII,
  kwVERBOSE,
  kwADJUST,
  kwTZ,
  kwDELTA,
  kwMAX,                   // manages keyword array
  kwNEEDVALUE,             // bogus keywords for error reporting
  kwSYNTAX,                // -- ditto --
  kwNOVALUE = -kwVERBOSE,  // keywords <= kwNOVALUE are flags (no value needed)
};

// file types supported
enum {
  typeUnknown = 0,
  typeDirectory = 1,
  typeImage = 2,
  typeXML = 3,
  typeFile = 4,
  typeDoc = 5,
  typeCode = 6,
  typeMax = 7
};

// forward declaration
class Position;

// globals
using TimeDict_t = std::map<time_t, Position>;
using paths_t = std::vector<fs::path>;
const char* gDeg = nullptr;  // string "°" or "deg"
TimeDict_t gTimeDict;
paths_t gFiles;

// Position (from gpx file)
class Position final {
 public:
  Position(time_t time, double lat, double lon, double ele) : time_(time), lon_(lon), lat_(lat), ele_(ele) {
  }

  Position() = default;
  ~Position() = default;
  Position(const Position&) = default;
  Position& operator=(const Position&) = default;

  //  instance methods
  [[nodiscard]] bool good() const {
    return time_ || lon_ || lat_ || ele_;
  }
  std::string getTimeString() {
    if (times_.empty())
      times_ = getExifTime(time_);
    return times_;
  }
  [[nodiscard]] time_t getTime() const {
    return time_;
  }
  [[nodiscard]] std::string toString() const;

  //  getters/setters
  [[nodiscard]] double lat() const {
    return lat_;
  }
  [[nodiscard]] double lon() const {
    return lon_;
  }
  [[nodiscard]] double ele() const {
    return ele_;
  }
  [[nodiscard]] int delta() const {
    return delta_;
  }
  void delta(int delta) {
    delta_ = delta;
  }

  //  data
 private:
  time_t time_{0};
  double lon_{0.0};
  double lat_{0.0};
  double ele_{0.0};
  std::string times_;
  int delta_{0};

  // public static data
 public:
  static int adjust_;
  static int tz_;
  static int dst_;
  static time_t deltaMax_;

  // public static member functions

  static int Adjust() {
    return Position::adjust_ + Position::tz_ + Position::dst_;
  }
  static int tz() {
    return tz_;
  }
  static int dst() {
    return dst_;
  }
  static int adjust() {
    return adjust_;
  }

  static std::string toExifString(double d, bool bRational, bool bLat);
  static std::string toExifString(double d);
  static std::string toExifTimeStamp(std::string& t);
};

std::string Position::toExifTimeStamp(std::string& t) {
  char result[200];
  const char* arg = t.c_str();
  int HH = 0;
  int mm = 0;
  int SS1 = 0;
  if (strstr(arg, ":") || strstr(arg, "-")) {
    int YY = 0, MM = 0, DD = 0;
    char a = 0, b = 0, c = 0, d = 0, e = 0;
    sscanf(arg, "%d%c%d%c%d%c%d%c%d%c%d", &YY, &a, &MM, &b, &DD, &c, &HH, &d, &mm, &e, &SS1);
  }
  snprintf(result, sizeof(result), "%d/1 %d/1 %d/1", HH, mm, SS1);
  return result;
}

std::string Position::toExifString(double d) {
  char result[200];
  d *= 100;
  snprintf(result, sizeof(result), "%d/100", abs(static_cast<int>(d)));
  return result;
}

std::string Position::toExifString(double d, bool bRational, bool bLat) {
  const char* NS = d >= 0.0 ? "N" : "S";
  const char* EW = d >= 0.0 ? "E" : "W";
  const char* NSEW = bLat ? NS : EW;
  if (d < 0)
    d = -d;
  auto deg = static_cast<int>(d);
  d -= deg;
  d *= 60;
  auto min = static_cast<int>(d);
  d -= min;
  d *= 60;
  auto sec = static_cast<int>(d);
  char result[200];
  if (bRational)
    snprintf(result, sizeof(result), "%d/1 %d/1 %d/1", deg, min, sec);
  else
    snprintf(result, sizeof(result), "%03d%s%02d'%02d\"%s", deg, gDeg, min, sec, NSEW);
  return result;
}

std::string Position::toString() const {
  char result[200];
  std::string sLat = Position::toExifString(lat_, false, true);
  std::string sLon = Position::toExifString(lon_, false, false);
  snprintf(result, sizeof(result), "%s %s %-8.3f", sLon.c_str(), sLat.c_str(), ele_);
  return result;
}

// defaults
int Position::adjust_ = 0;
int Position::tz_ = timeZoneAdjust();
int Position::dst_ = 0;
time_t Position::deltaMax_ = 60;

///////////////////////////////////////////////////////////
// UserData - used by XML Parser
struct UserData final {
  explicit UserData(Options& options) : options_(options) {
  }

  //  public data members
  int indent{0};
  size_t count{0};
  Position now;
  Position prev;
  int nTrkpt{0};
  bool bTime{false};
  bool bEle{false};
  double ele{0.};
  double lat{0.};
  double lon{0.};
  std::string xmlt;
  std::string exift;
  time_t time{0};
  Options& options_;
  // static public data members
};

// XML Parser Callbacks
void startElement(void* userData, const char* name, const char** atts) {
  auto me = static_cast<UserData*>(userData);
  // for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
  // printf("begin %s\n",name);
  me->bTime = strcmp(name, "time") == 0;
  me->bEle = strcmp(name, "ele") == 0;

  if (strcmp(name, "trkpt") == 0) {
    me->nTrkpt++;
    while (*atts) {
      const char* a = atts[0];
      const char* v = atts[1];
      if (!strcmp(a, "lat"))
        me->lat = atof(v);
      if (!strcmp(a, "lon"))
        me->lon = atof(v);
      atts += 2;
    }
  }
  me->count++;
  me->indent++;
}

void endElement(void* userData, const char* name) {
  auto me = static_cast<UserData*>(userData);
  me->indent--;
  if (strcmp(name, "trkpt") == 0) {
    me->nTrkpt--;
    me->now = Position(me->time, me->lat, me->lon, me->ele);

    if (!me->prev.good() && me->options_.verbose) {
      printf("trkseg %s begin ", me->now.getTimeString().c_str());
    }

    // remember our location and put it in gTimeDict
    gTimeDict[me->time] = me->now;
    me->prev = me->now;
  }
  if (strcmp(name, "trkseg") == 0 && me->options_.verbose) {
    printf("%s end\n", me->now.getTimeString().c_str());
  }
}

void charHandler(void* userData, const char* s, int len) {
  auto me = static_cast<UserData*>(userData);

  if (me->nTrkpt == 1) {
    char buffer[100];
    int l_max = 98;  // lengthof(buffer) -2 ;

    if (me->bTime && len > 5) {
      if (len < l_max) {
        memcpy(buffer, s, len);
        buffer[len] = 0;
        char* b = buffer;
        while (*b == ' ' && b < buffer + len)
          b++;
        me->xmlt = b;
        me->time = parseTime(me->xmlt.c_str());
        me->exift = getExifTime(me->time);
      }
      me->bTime = false;
    }
    if (me->bEle && len > 2) {
      if (len < l_max) {
        memcpy(buffer, s, len);
        buffer[len] = 0;
        char* b = buffer;
        while (*b == ' ' && b < buffer + len)
          b++;
        me->ele = atof(b);
      }
      me->bEle = false;
    }
  }
}

///////////////////////////////////////////////////////////
// Time Functions
time_t parseTime(const char* arg, bool bAdjust) {
  time_t result = 0;
  try {
    // 559 rmills@rmills-imac:~/bin $ exiv2 -pa ~/R.jpg | grep -i date
    // Exif.Image.DateTime                          Ascii      20  2009:08:03 08:58:57
    // Exif.Photo.DateTimeOriginal                  Ascii      20  2009:08:03 08:58:57
    // Exif.Photo.DateTimeDigitized                 Ascii      20  2009:08:03 08:58:57
    // Exif.GPSInfo.GPSDateStamp                    Ascii      21  2009-08-03T15:58:57Z

    // <time>2012-07-14T17:33:16Z</time>

    if (strstr(arg, ":") || strstr(arg, "-")) {
      int YY = 0, MM = 0, DD = 0, HH = 0, mm = 0, SS1 = 0;
      char a = 0, b = 0, c = 0, d = 0, e = 0;
      sscanf(arg, "%d%c%d%c%d%c%d%c%d%c%d", &YY, &a, &MM, &b, &DD, &c, &HH, &d, &mm, &e, &SS1);

      struct tm T = {};
      T.tm_min = mm;
      T.tm_hour = HH;
      T.tm_sec = SS1;
      if (bAdjust)
        T.tm_sec -= Position::Adjust();
      T.tm_year = YY - 1900;
      T.tm_mon = MM - 1;
      T.tm_mday = DD;
      T.tm_isdst = -1;  // determine value automatically (otherwise hour may shift)
      result = mktime(&T);
    }
  } catch (...) {
  }
  return result;
}

// West of GMT is negative (PDT = Pacific Daylight = -07:00 == -25200 seconds
int timeZoneAdjust() {
  std::tm gmt;
  std::tm local;

  auto now = std::time(nullptr);
#if defined(_WIN32)
  if (localtime_s(&local, &now) || gmtime_s(&gmt, &now))
    return 0;
#else
  localtime_r(&now, &local);
  gmtime_r(&now, &gmt);
#endif
  return std::mktime(&gmt) - std::mktime(&local);
}

std::string getExifTime(const time_t t) {
  static char result[100];
  strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", localtime(&t));
  return result;
}

// file utilities
bool readDir(const fs::path& path, Options& options) {
  try {
    for (const auto& entry : fs::directory_iterator(path)) {
      auto pathName = entry.path();
      if (entry.is_regular_file() && getFileType(pathName, options) == typeImage) {
        gFiles.push_back(std::move(pathName));
      }
    }
    return true;
  } catch (const fs::filesystem_error&) {
    return false;
  }
}

inline size_t sip(std::ifstream& f, std::vector<char>& buffer, size_t len) {
  while (f && len < buffer.size() && buffer[len - 1] != '>') {
    char c;
    f.get(c);
    buffer[len++] = c;
  }
  return len;
}

bool readXML(const fs::path& path, Options& options) {
  std::ifstream file(path);
  if (!file) {
    return false;
  }

  XML_Parser parser = XML_ParserCreate(nullptr);
  if (!parser) {
    return false;
  }

  bool bResult = true;
  std::vector<char> buffer(8 * 1024);
  UserData me(options);

  XML_SetUserData(parser, &me);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charHandler);

  // A little sip at the data
  file.read(buffer.data(), buffer.size() - 100);
  std::streamsize len = file.gcount();
  const char* lead = "<?xml";
  bResult = len > 0 && strncmp(lead, buffer.data(), strlen(lead)) == 0;

  // Swallow it
  if (bResult) {
    len = sip(file, buffer, len);
    bResult = XML_Parse(parser, buffer.data(), static_cast<int>(len), len == 0) == XML_STATUS_OK;
  }

  // Drink the rest of the file
  while (bResult && len > 0) {
    file.read(buffer.data(), buffer.size() - 100);
    len = file.gcount();
    len = sip(file, buffer, len);
    bResult = XML_Parse(parser, buffer.data(), static_cast<int>(len), len == 0) == XML_STATUS_OK;
  }

  XML_ParserFree(parser);
  return bResult;
}

bool readImage(const fs::path& path, Options& /* options */) {
  using namespace Exiv2;
  bool bResult = false;

  try {
    if (auto image = ImageFactory::open(path)) {
      image->readMetadata();
      ExifData& exifData = image->exifData();
      bResult = !exifData.empty();
    }
  } catch (...) {
  }
  return bResult;
}

time_t readImageTime(const fs::path& path, std::string* pS = nullptr) {
  using namespace Exiv2;

  time_t result = 0;

  const char* dateStrings[] = {"Exif.Photo.DateTimeOriginal", "Exif.Photo.DateTimeDigitized", "Exif.Image.DateTime",
                               nullptr};

  for (size_t i = 0; !result && dateStrings[i]; i++) {
    const char* dateString = dateStrings[i];
    try {
      if (auto image = ImageFactory::open(path)) {
        image->readMetadata();
        ExifData& exifData = image->exifData();
        //  printf("%s => %s\n",dateString, exifData[dateString].toString().c_str());
        result = parseTime(exifData[dateString].toString().c_str(), true);
        if (result && pS)
          *pS = exifData[dateString].toString();
      }
    } catch (...) {
    }
  }

  return result;
}

bool sina(const char* s, const char** a) {
  bool bResult = false;
  int i = 0;
  while (*s == '-')
    s++;
  while (!bResult && a[i]) {
    const char* A = a[i];
    while (*A == '-')
      A++;
    bResult = _stricmp(s, A) == 0;
    i++;
  }
  return bResult;
}

int readFile(const fs::path& path, const Options& /* options */) {
  if (!fs::exists(path)) {
    return typeUnknown;
  }

  for (auto doc : {".doc", ".txt"})
    if (path.extension() == doc)
      return typeDoc;

  for (auto code : {".cpp", ".h", ".pl", ".py", ".pyc"})
    if (path.extension() == code)
      return typeCode;

  return typeFile;
}

Position* searchTimeDict(TimeDict_t& td, const time_t& time, long long delta) {
  Position* result = nullptr;
  for (int t = 0; !result && t < delta; t++) {
    for (int x = 0; !result && x < 2; x++) {
      int T = t * ((x == 0) ? -1 : 1);
      if (td.contains(time + T)) {
        result = &td[time + T];
        result->delta(T);
      }
    }
  }
  return result;
}

int getFileType(const fs::path& path, Options& options) {
  if (readXML(path, options))
    return typeXML;
  if (readDir(path, options))
    return typeDirectory;
  if (readImage(path, options))
    return typeImage;
  return readFile(path, options);
}

int version(const char* program) {
  printf("%s: %s %s\n", program, __DATE__, __TIME__);
  return 0;
}

int help(const char* program, char const* words[], int nWords, bool /*bVerbose*/) {
  printf("usage: %s ", program);
  for (int i = 0; i < nWords; i++) {
    if (words[i])
      printf("%c-%s%s", i ? '|' : '{', words[i], i > (-kwNOVALUE) ? " value" : "");
  }
  printf("}+ path+\n");
  return 0;
}

int compare(const char* a, const char* b) {
  int result = *a && *b;
  while (result && *a && *b) {
    char A = *a++;
    char B = *b++;
    result = tolower(A) == tolower(B);
  }
  return result;
}

int find(const char* arg, char const* words[], int nWords) {
  if (arg[0] != '-')
    return kwSYNTAX;

  int result = 0;
  int count = 0;

  for (int i = 0; i < nWords; i++) {
    int j = 0;
    while (arg[j] == '-')
      j++;
    if (::compare(arg + j, words[i])) {
      result = i;
      count++;
    }
  }

  return count == 1 ? result : kwSYNTAX;
}

int parseTZ(const char* adjust) {
  int h = 0;
  int m = 0;
  char c = 0;
  try {
    sscanf(adjust, "%d%c%d", &h, &c, &m);
  } catch (...) {
  }

  return (3600 * h) + (60 * m);
}

bool mySort(const fs::path& a, const fs::path& b) {
  time_t A = readImageTime(a);
  time_t B = readImageTime(b);
  return (A < B);
}
}  // namespace

int main(int argc, const char* argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  int result = 0;
  const char* program = argv[0];

  const char* types[typeMax];
  types[typeUnknown] = "unknown";
  types[typeDirectory] = "directory";
  types[typeImage] = "image";
  types[typeXML] = "xml";
  types[typeDoc] = "doc";
  types[typeCode] = "code";
  types[typeFile] = "file";

  char const* keywords[kwMAX] = {};
  keywords[kwHELP] = "help";
  keywords[kwVERSION] = "version";
  keywords[kwVERBOSE] = "verbose";
  keywords[kwDRYRUN] = "dryrun";
  keywords[kwASCII] = "ascii";
  keywords[kwDST] = "dst";
  keywords[kwADJUST] = "adjust";
  keywords[kwTZ] = "tz";
  keywords[kwDELTA] = "delta";

  std::map<std::string, std::string> shorts;
  shorts["-?"] = "-help";
  shorts["-h"] = "-help";
  shorts["-v"] = "-verbose";
  shorts["-V"] = "-version";
  shorts["-d"] = "-dst";
  shorts["-a"] = "-adjust";
  shorts["-t"] = "-tz";
  shorts["-D"] = "-delta";
  shorts["-s"] = "-delta";
  shorts["-X"] = "-dryrun";
  shorts["-A"] = "-ascii";

  Options options;
  options.help = sina(keywords[kwHELP], argv) || argc < 2;
  options.verbose = sina(keywords[kwVERBOSE], argv);
  options.dryrun = sina(keywords[kwDRYRUN], argv);
  options.version = sina(keywords[kwVERSION], argv);
  options.dst = sina(keywords[kwDST], argv);
  options.ascii = sina(keywords[kwASCII], argv);

  for (int i = 1; !result && i < argc; i++) {
    const char* arg = argv[i++];
    if (shorts.contains(arg))
      arg = shorts[arg].c_str();

    const char* value = argv[i];
    int ivalue = ::atoi(value ? value : "0");
    int key = ::find(arg, keywords, kwMAX);
    int needv = key < kwMAX && key > (-kwNOVALUE);

    if (!needv)
      i--;
    if (needv && !value)
      key = kwNEEDVALUE;

    switch (key) {
      case kwDST:
        options.dst = true;
        break;
      case kwHELP:
        options.help = true;
        break;
      case kwVERSION:
        options.version = true;
        break;
      case kwDRYRUN:
        options.dryrun = true;
        break;
      case kwVERBOSE:
        options.verbose = true;
        break;
      case kwASCII:
        options.ascii = true;
        break;
      case kwTZ:
        Position::tz_ = parseTZ(value);
        break;
      case kwADJUST:
        Position::adjust_ = ivalue;
        break;
      case kwDELTA:
        Position::deltaMax_ = ivalue;
        break;
      case kwNEEDVALUE:
        fprintf(stderr, "error: %s requires a value\n", arg);
        result = resultSyntaxError;
        break;
      case kwSYNTAX:
      default: {
        int type = getFileType(arg, options);
        if (options.verbose)
          printf("%s %s ", arg, types[type]);
        if (type == typeImage) {
          time_t t = readImageTime(fs::path(arg));
          auto p = fs::absolute(fs::path(arg));
          if (t && !p.empty()) {
            if (options.verbose)
#ifdef _WIN32
              printf("%ls %ld %s", p.c_str(), static_cast<long int>(t), asctime(localtime(&t)));
#else
              printf("%s %ld %s", p.c_str(), static_cast<long int>(t), asctime(localtime(&t)));
#endif
            gFiles.push_back(std::move(p));
          }
        }
        if (type == typeUnknown) {
          fprintf(stderr, "error: illegal syntax %s\n", arg);
          result = resultSyntaxError;
        }
      } break;
    }
  }

  if (options.help)
    ::help(program, keywords, kwMAX, options.verbose);
  if (options.version)
    ::version(program);
  gDeg = options.ascii ? "deg" : "°";

  if (!result) {
    sort(gFiles.begin(), gFiles.end(), mySort);
    if (options.dst)
      Position::dst_ = 3600;
    if (options.verbose) {
      int t = Position::tz();
      int d = Position::dst();
      int a = Position::adjust();
      int A = Position::Adjust();
      int s = A;
      int h = s / 3600;
      s -= h * 3600;
      s = abs(s);
      int m = s / 60;
      s -= m * 60;
      printf("tz,dst,adjust = %d,%d,%d total = %dsecs (= %d:%d:%d)\n", t, d, a, A, h, m, s);
    }
    /*
            if ( options.verbose ) {
                printf("Time Dictionary\n");
                for (auto it = gTimeDict.begin() ;  it != gTimeDict.end() ; it++ ) {
                    std::string sTime = getExifTime(it->first);
                    Position*   pPos  = &it->second;
                    std::string sPos  = Position::toExifString(pPos->lat(),false,true)
                                      + " "
                                      + Position::toExifString(pPos->lon(),false,true)
                                      ;
                    printf("%s %s\n",sTime.c_str(), sPos.c_str());
                }
            }
    */
    for (auto&& path : gFiles) {
      std::string stamp;
      try {
        time_t t = readImageTime(path, &stamp);
        Position* pPos = searchTimeDict(gTimeDict, t, Position::deltaMax_);
        if (auto image = Exiv2::ImageFactory::open(path)) {
          image->readMetadata();
          Exiv2::ExifData& exifData = image->exifData();
          if (pPos) {
            exifData["Exif.GPSInfo.GPSProcessingMethod"] = "charset=Ascii HYBRID-FIX";
            exifData["Exif.GPSInfo.GPSVersionID"] = "2 2 0 0";
            exifData["Exif.GPSInfo.GPSMapDatum"] = "WGS-84";

            exifData["Exif.GPSInfo.GPSLatitude"] = Position::toExifString(pPos->lat(), true, true);
            exifData["Exif.GPSInfo.GPSLongitude"] = Position::toExifString(pPos->lon(), true, false);
            exifData["Exif.GPSInfo.GPSAltitude"] = Position::toExifString(pPos->ele());

            exifData["Exif.GPSInfo.GPSAltitudeRef"] = pPos->ele() < 0.0 ? "1" : "0";
            exifData["Exif.GPSInfo.GPSLatitudeRef"] = pPos->lat() > 0 ? "N" : "S";
            exifData["Exif.GPSInfo.GPSLongitudeRef"] = pPos->lon() > 0 ? "E" : "W";

            exifData["Exif.GPSInfo.GPSDateStamp"] = stamp;
            exifData["Exif.GPSInfo.GPSTimeStamp"] = Position::toExifTimeStamp(stamp);
            exifData["Exif.Image.GPSTag"] = 4908;

#ifdef _WIN32
            printf("%ls %s % 2d\n", path.c_str(), pPos->toString().c_str(), pPos->delta());
#else
            printf("%s %s % 2d\n", path.c_str(), pPos->toString().c_str(), pPos->delta());
#endif
          } else {
#ifdef _WIN32
            printf("%ls *** not in time dict ***\n", path.c_str());
#else
            printf("%s *** not in time dict ***\n", path.c_str());
#endif
          }
          if (!options.dryrun)
            image->writeMetadata();
        }
      } catch (...) {
      }
    }
  }

  return result;
}

// That's all Folks!
////
