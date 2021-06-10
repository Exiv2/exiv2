// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#ifndef HTTP_HPP_
#define HTTP_HPP_

#include "exiv2lib_export.h"

#include "datasets.hpp"

#include <string>


namespace Exiv2 {
    /*!
     @brief execute an HTTP request
     @param request -  a Dictionary of headers to send to server
     @param response - a Dictionary of response headers (dictionary is filled by the response)
     @param errors   - a String with an error
     @return Server response 200 = OK, 404 = Not Found etc...
    */
    EXIV2API int http(Exiv2::Dictionary& request,Exiv2::Dictionary& response,std::string& errors);
}  // namespace Exiv2

#endif
