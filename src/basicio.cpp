/*
 * Copyright (C) 2004-2019 Exiv2 authors
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

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"

namespace Exiv2
{
    BasicIo::BasicIo()
        : bigBlock_(nullptr)
    {
    }

    void BasicIo::readOrThrow(byte* buf, size_t rcount)
    {
        const size_t nread = read(buf, rcount);
        enforce(nread == rcount, kerInputDataReadFailed);
        enforce(!error(), kerInputDataReadFailed);
    }

    IoCloser::IoCloser(BasicIo& bio)
        : bio_(bio)
    {
    }

    IoCloser::~IoCloser()
    {
        close();
    }

    void IoCloser::close()
    {
        if (bio_.isopen())
            bio_.close();
    }

    std::string ReplaceStringInPlace(std::string subject, const std::string& search, const std::string& replace)
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring ReplaceStringInPlace(std::wstring subject, const std::wstring& search, const std::wstring& replace)
    {
        std::wstring::size_type pos = 0;
        while ((pos = subject.find(search, pos)) != std::wstring::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }
#endif
}
