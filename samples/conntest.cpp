// SPDX-License-Identifier: GPL-2.0-or-later
// Tester application for testing the http/https/ftp/ssh/sftp connection

#include <exiv2/exiv2.hpp>
#ifdef EXV_USE_CURL
#include <curl/curl.h>
#endif
#include <cstdlib>
#include <iostream>

void httpcon(const std::string& url, bool useHttp1_0 = false) {
  Exiv2::Dictionary response;
  Exiv2::Dictionary request;
  std::string errors;

  Exiv2::Uri uri = Exiv2::Uri::Parse(url);
  Exiv2::Uri::Decode(uri);

  request["server"] = uri.Host;
  request["page"] = uri.Path;
  request["port"] = uri.Port;
  if (!useHttp1_0)
    request["version"] = "1.1";

  int serverCode = Exiv2::http(request, response, errors);
  if (serverCode < 0 || serverCode >= 400 || !errors.empty()) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerTiffDirectoryTooLarge, "Server", serverCode);
  }
}

#ifdef EXV_USE_CURL
void curlcon(const std::string& url, bool useHttp1_0 = false) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Unable to init libcurl.");
  }

  // get the timeout value
  std::string timeoutStr = Exiv2::getEnv(Exiv2::envTIMEOUT);
  long timeout = atol(timeoutStr.c_str());
  if (timeout == 0) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Timeout Environmental Variable must be a positive integer.");
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Exiv2::curlWriter);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // debug
  if (useHttp1_0)
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  else
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

  /* Perform the request, res will get the return code */
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {  // error happened
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, curl_easy_strerror(res));
  }

  // get return code
  long returnCode;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &returnCode);  // get code
  curl_easy_cleanup(curl);

  if (returnCode >= 400 || returnCode < 0) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerTiffDirectoryTooLarge, "Server", returnCode);
  }
}
#endif

int main(int argc, const char** argv) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
  Exiv2::enableBMFF();
#endif

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " url {-http1_0}" << std::endl;
    return EXIT_FAILURE;
  }
  std::string url(argv[1]);
  Exiv2::Protocol prot = Exiv2::fileProtocol(url);

  bool useHttp1_0 = false;
  for (int a = 1; a < argc; a++) {
    std::string arg(argv[a]);
    if (arg == "-http1_0")
      useHttp1_0 = true;
  }

  bool isOk = false;
  try {
#ifdef EXV_USE_CURL
    if (prot == Exiv2::pHttp || prot == Exiv2::pHttps || prot == Exiv2::pFtp) {
      curlcon(url, useHttp1_0);
      isOk = true;
    }
#endif
    if (!isOk && prot == Exiv2::pHttp) {
      httpcon(url, useHttp1_0);
      isOk = true;
    }
  } catch (const Exiv2::Error& e) {
    std::cout << "Error: '" << e << "'" << std::endl;
    return EXIT_FAILURE;
  }

  if (!isOk)
    std::cout << "The protocol is unsupported." << std::endl;
  else
    std::cout << "OK." << std::endl;
  return EXIT_SUCCESS;
}

// That's all Folks!
