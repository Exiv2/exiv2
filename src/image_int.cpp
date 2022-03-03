// SPDX-License-Identifier: GPL-2.0-or-later

#include "image_int.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstring>
#include <cstdio>

namespace Exiv2
{
    namespace Internal
    {
        std::string stringFormat(const char* format, ...)
        {
            std::string result;
            std::vector<char> buffer;
            size_t need = std::strlen(format)*8;  // initial guess
            int rc = -1;

            // vsnprintf writes at most size (2nd parameter) bytes (including \0)
            //           returns the number of bytes required for the formatted string excluding \0
            // the following loop goes through:
            // one iteration (if 'need' was large enough for the for formatted string)
            // or two iterations (after the first call to vsnprintf we know the required length)
            do {
                buffer.resize(need + 1);
                va_list args;            // variable arg list
                va_start(args, format);  // args start after format
                rc = vsnprintf(&buffer[0], buffer.size(), format, args);
                va_end(args);     // free the args
                assert(rc >= 0);  // rc < 0 => we have made an error in the format string
                if (rc > 0)
                    need = static_cast<size_t>(rc);
            } while (buffer.size() <= need);

            if (rc > 0)
                result = std::string(&buffer[0], need);
            return result;
        }

        std::string indent(int32_t d)
        {
            std::string result;
            if (d > 0)
                while (d--)
                    result += "  ";
            return result;
        }

    }  // namespace Internal

}  // namespace Exiv2
