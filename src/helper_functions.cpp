// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"
#include <string.h>

std::string string_from_unterminated(const char* data, size_t data_length)
{
    const size_t StringLength = strnlen(data, data_length);

    return std::string(data, StringLength);
}
