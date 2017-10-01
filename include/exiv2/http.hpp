#ifndef HTTP_HPP_
#define HTTP_HPP_

#include "config.h"
#include <string>
#include <map>
#include <algorithm>

#include "datasets.hpp"

namespace Exiv2 {
    /*!
     @brief execute an HTTP request
     @param request -  a Dictionary of headers to send to server
     @param response - a Dictionary of response headers (dictionary is filled by the response)
     @param errors   - a String with an error
     @return Server response 200 = OK, 404 = Not Found etc...
    */
    EXIV2API int http(Exiv2::Dictionary& request,Exiv2::Dictionary& response,std::string& errors);
}

#ifdef EXV_USE_CURL
#include <curl/curl.h>
#endif

#endif
