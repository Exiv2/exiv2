#ifndef HTTP_HPP_
#define HTTP_HPP_

#include <string>
#include <map>
#include <algorithm>

#include "datasets.hpp"

namespace Exiv2 {
    EXIV2API int http(Exiv2::Dictionary& request,Exiv2::Dictionary& response,std::string& errors);
}

#ifndef EXV_USE_CURL
#define EXV_USE_CURL 0
#endif
#if EXV_USE_CURL == 1
#include <curl/curl.h>
#endif

#endif
