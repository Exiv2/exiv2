#ifndef HTTP_H_
#define HTTP_H_

#include <string>
#include <map>
#include <algorithm>

#include <exiv2/exiv2.hpp>

namespace Exiv2 {
    typedef std::map<std::string,std::string> dict_t;
    typedef dict_t::iterator                  dict_i;

    EXIV2API int http(dict_t& request,dict_t& response,std::string& errors);

}
#endif


