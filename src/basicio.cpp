// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
 *
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
/*
  File:      basicio.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   04-Dec-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "basicio.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <memory>
#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>                       // for remove, rename
#include <cstdlib>                      // for alloc, realloc, free
#include <sys/types.h>                  // for stat, chmod
#include <sys/stat.h>                   // for stat, chmod
#ifdef EXV_HAVE_SYS_MMAN_H
# include <sys/mman.h>                  // for mmap and munmap
#endif
#ifdef EXV_HAVE_PROCESS_H
# include <process.h>
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for getpid, stat
#endif

#if defined WIN32 && !defined __CYGWIN__
# include <io.h>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    BasicIo::~BasicIo()
    {
    }

    FileIo::FileIo(const std::string& path)
        : path_(path),
#ifdef EXV_UNICODE_PATH
          wpMode_(wpStandard),
#endif
          fp_(0), opMode_(opSeek),
          pMappedArea_(0), mappedLength_(0), isMalloced_(false), isWriteable_(false)
    {
    }

#ifdef EXV_UNICODE_PATH
    FileIo::FileIo(const std::wstring& wpath)
        : wpath_(wpath),
          wpMode_(wpUnicode),
          fp_(0), opMode_(opSeek),
          pMappedArea_(0), mappedLength_(0), isMalloced_(false), isWriteable_(false)
    {
    }

#endif
    FileIo::~FileIo()
    {
        close();
    }

    int FileIo::munmap()
    {
        int rc = 0;
        if (pMappedArea_ != 0) {
#if defined EXV_HAVE_MMAP && defined EXV_HAVE_MUNMAP
            if (::munmap(pMappedArea_, mappedLength_) != 0) {
                rc = 1;
            }
#else
            if (isWriteable_) {
                write(pMappedArea_, mappedLength_);
            }
            if (isMalloced_) {
                delete[] pMappedArea_;
                isMalloced_ = false;
            }
#endif
        }
        if (isWriteable_) {
            if (fp_ != 0) switchMode(opRead);
            isWriteable_ = false;
        }
        pMappedArea_ = 0;
        mappedLength_ = 0;
        return rc;
    }

    byte* FileIo::mmap(bool isWriteable)
    {
        assert(fp_ != 0);
        if (munmap() != 0) {
            throw Error(2, path_, strError(), "munmap");
        }
        mappedLength_ = size();
        isWriteable_ = isWriteable;
#if defined EXV_HAVE_MMAP && defined EXV_HAVE_MUNMAP
        int prot = PROT_READ;
        if (isWriteable_) {
            prot |= PROT_WRITE;
            if (switchMode(opWrite) != 0) return 0;
        }
        void* rc = ::mmap(0, mappedLength_, prot, MAP_SHARED, fileno(fp_), 0);
        if (MAP_FAILED == rc) {
            throw Error(2, path_, strError(), "mmap");
        }
        pMappedArea_ = static_cast<byte*>(rc);
#else
        // Workaround for platforms without mmap: Read the file into memory
        DataBuf buf(static_cast<long>(mappedLength_));
        read(buf.pData_, buf.size_);
        if (error() || eof()) throw Error(2, path_, strError(), "FileIo::mmap");
        pMappedArea_ = buf.release().first;
        isMalloced_ = true;
#endif
        return pMappedArea_;
    }

    int FileIo::stat(StructStat& buf) const
    {
        int ret = 0;
#ifdef EXV_UNICODE_PATH
        if (wpMode_ == wpUnicode) {
            struct _stat st;
            ret = ::_wstat(wpath_.c_str(), &st);
            if (0 == ret) {
                buf.st_size = st.st_size;
                buf.st_mode = st.st_mode;
            }
        }
        else
#endif
        {
            struct stat st;
            ret = ::stat(path_.c_str(), &st);
            if (0 == ret) {
                buf.st_size = st.st_size;
                buf.st_mode = st.st_mode;
            }
        }
        return ret;
    }

    BasicIo::AutoPtr FileIo::temporary() const
    {
        BasicIo::AutoPtr basicIo;

        StructStat buf;
        int ret = stat(buf);

        // If file is > 1MB then use a file, otherwise use memory buffer
        if (ret != 0 || buf.st_size > 1048576) {
            pid_t pid = ::getpid();
            std::auto_ptr<FileIo> fileIo;
#ifdef EXV_UNICODE_PATH
            if (wpMode_ == wpUnicode) {
                std::wstring tmpname = wpath_ + s2ws(toString(pid));
                fileIo = std::auto_ptr<FileIo>(new FileIo(tmpname));
            }
            else
#endif
            {
                std::string tmpname = path_ + toString(pid);
                fileIo = std::auto_ptr<FileIo>(new FileIo(tmpname));
            }
            if (fileIo->open("w+b") != 0) {
                throw Error(10, path_, "w+b", strError());
            }
            basicIo = fileIo;
        }
        else {
            basicIo.reset(new MemIo);
        }

        return basicIo;
    }

    int FileIo::switchMode(OpMode opMode)
    {
        assert(fp_ != 0);
        if (opMode_ == opMode) return 0;
        OpMode oldOpMode = opMode_;
        opMode_ = opMode;

        bool reopen = true;
        switch(opMode) {
        case opRead:
            // Flush if current mode allows reading, else reopen (in mode "r+b"
            // as in this case we know that we can write to the file)
            if (openMode_[0] == 'r' || openMode_[1] == '+') reopen = false;
            break;
        case opWrite:
            // Flush if current mode allows writing, else reopen
            if (openMode_[0] != 'r' || openMode_[1] == '+') reopen = false;
            break;
        case opSeek:
            reopen = false;
            break;
        }

        if (!reopen) {
            // Don't do anything when switching _from_ opSeek mode; we
            // flush when switching _to_ opSeek.
            if (oldOpMode == opSeek) return 0;

            // Flush. On msvcrt fflush does not do the job
            std::fseek(fp_, 0, SEEK_CUR);
            return 0;
        }

        // Reopen the file
        long offset = std::ftell(fp_);
        if (offset == -1) return -1;
        // 'Manual' open("r+b") to avoid munmap()
        if (fp_ != 0) {
            std::fclose(fp_);
            fp_= 0;
        }
        openMode_ = "r+b";
        opMode_ = opSeek;
        fp_ = std::fopen(path_.c_str(), openMode_.c_str());
        if (!fp_) return 1;
        return std::fseek(fp_, offset, SEEK_SET);
    }

    long FileIo::write(const byte* data, long wcount)
    {
        assert(fp_ != 0);
        if (switchMode(opWrite) != 0) return 0;
        return (long)std::fwrite(data, 1, wcount, fp_);
    }

    long FileIo::write(BasicIo& src)
    {
        assert(fp_ != 0);
        if (static_cast<BasicIo*>(this) == &src) return 0;
        if (!src.isopen()) return 0;
        if (switchMode(opWrite) != 0) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            writeTotal += writeCount = (long)std::fwrite(buf, 1, readCount, fp_);
            if (writeCount != readCount) {
                // try to reset back to where write stopped
                src.seek(writeCount-readCount, BasicIo::cur);
                break;
            }
        }

        return writeTotal;
    }

    void FileIo::transfer(BasicIo& src)
    {
        const bool wasOpen = (fp_ != 0);
        const std::string lastMode(openMode_);

        FileIo *fileIo = dynamic_cast<FileIo*>(&src);
        if (fileIo) {
            // Optimization if src is another instance of FileIo
            fileIo->close();
            // Check if the file can be written to, if it already exists
            if (open("w+b") != 0) {
                // Remove the (temporary) file
#ifdef EXV_UNICODE_PATH
                if (fileIo->wpMode_ == wpUnicode) {
                    ::_wremove(fileIo->wpath_.c_str());
                }
                else
#endif
                {
                    ::remove(fileIo->path_.c_str());
                }
                throw Error(10, path_, "w+b", strError());
            }
            close();

            bool statOk = true;
            mode_t origStMode = 0;
            char* pf = 0;
#ifdef EXV_UNICODE_PATH
            wchar_t* wpf = 0;
            if (wpMode_ == wpUnicode) {
                wpf = const_cast<wchar_t*>(wpath_.c_str());
            }
            else
#endif
            {
                pf = const_cast<char*>(path_.c_str());
            }

            // Get the permissions of the file, or linked-to file, on platforms which have lstat
#ifdef EXV_HAVE_LSTAT

# ifdef EXV_UNICODE_PATH
#  error EXV_UNICODE_PATH and EXV_HAVE_LSTAT are not compatible. Stop.
# endif
            struct stat buf1;
            if (::lstat(pf, &buf1) == -1) {
                statOk = false;
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: " << Error(2, pf, strError(), "::lstat") << "\n";
#endif
            }
            origStMode = buf1.st_mode;
            DataBuf lbuf; // So that the allocated memory is freed. Must have same scope as pf
            // In case path_ is a symlink, get the path of the linked-to file
            if (statOk && S_ISLNK(buf1.st_mode)) {
                lbuf.alloc(buf1.st_size + 1);
                memset(lbuf.pData_, 0x0, lbuf.size_);
                pf = reinterpret_cast<char*>(lbuf.pData_);
                if (::readlink(path_.c_str(), pf, lbuf.size_ - 1) == -1) {
                    throw Error(2, path_, strError(), "readlink");
                }
                // We need the permissions of the file, not the symlink
                if (::stat(pf, &buf1) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: " << Error(2, pf, strError(), "::stat") << "\n";
#endif
                }
                origStMode = buf1.st_mode;
            }
#else // EXV_HAVE_LSTAT
            StructStat buf1;
            if (stat(buf1) == -1) {
                statOk = false;
            }
            origStMode = buf1.st_mode;
#endif // !EXV_HAVE_LSTAT

            // MSVCRT rename that does not overwrite existing files
#ifdef EXV_UNICODE_PATH
            if (wpMode_ == wpUnicode) {
                if (fileExists(wpf) && ::_wremove(wpf) != 0) {
                    throw Error(2, wpf, strError(), "::_wremove");
                }
                if (::_wrename(fileIo->wpath_.c_str(), wpf) == -1) {
                    throw Error(17, ws2s(fileIo->wpath_), wpf, strError());
                }
                ::_wremove(fileIo->wpath_.c_str());
                // Check permissions of new file
                struct _stat buf2;
                if (statOk && ::_wstat(wpf, &buf2) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: " << Error(2, wpf, strError(), "::_wstat") << "\n";
#endif
                }
                if (statOk && origStMode != buf2.st_mode) {
                    // Set original file permissions
                    if (::_wchmod(wpf, origStMode) == -1) {
#ifndef SUPPRESS_WARNINGS
                        std::cerr << "Warning: " << Error(2, wpf, strError(), "::_wchmod") << "\n";
#endif
                    }
                }
            } // if (wpMode_ == wpUnicode)
            else
#endif // EXV_UNICODE_PATH
            {
                if (fileExists(pf) && ::remove(pf) != 0) {
                    throw Error(2, pf, strError(), "::remove");
                }
                if (::rename(fileIo->path_.c_str(), pf) == -1) {
                    throw Error(17, fileIo->path_, pf, strError());
                }
                ::remove(fileIo->path_.c_str());
                // Check permissions of new file
                struct stat buf2;
                if (statOk && ::stat(pf, &buf2) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: " << Error(2, pf, strError(), "::stat") << "\n";
#endif
                }
                if (statOk && origStMode != buf2.st_mode) {
                    // Set original file permissions
                    if (::chmod(pf, origStMode) == -1) {
#ifndef SUPPRESS_WARNINGS
                        std::cerr << "Warning: " << Error(2, pf, strError(), "::chmod") << "\n";
#endif
                    }
                }
            }
        } // if (fileIo)
        else {
            // Generic handling, reopen both to reset to start
            if (open("w+b") != 0) {
                throw Error(10, path_, "w+b", strError());
            }
            if (src.open() != 0) {
                throw Error(9, src.path(), strError());
            }
            write(src);
            src.close();
        }

        if (wasOpen) {
            if (open(lastMode) != 0) {
                throw Error(10, path_, lastMode, strError());
            }
        }
        else close();

        if (error() || src.error()) throw Error(18, path_, strError());
    }

    int FileIo::putb(byte data)
    {
        assert(fp_ != 0);
        if (switchMode(opWrite) != 0) return EOF;
        return putc(data, fp_);
    }

    int FileIo::seek(long offset, Position pos)
    {
        assert(fp_ != 0);

        int fileSeek = 0;
        switch (pos) {
        case BasicIo::cur: fileSeek = SEEK_CUR; break;
        case BasicIo::beg: fileSeek = SEEK_SET; break;
        case BasicIo::end: fileSeek = SEEK_END; break;
        }

        if (switchMode(opSeek) != 0) return 1;
        return std::fseek(fp_, offset, fileSeek);
    }

    long FileIo::tell() const
    {
        assert(fp_ != 0);
        return std::ftell(fp_);
    }


    long FileIo::size() const
    {
        // Flush and commit only if the file is open for writing
        if (fp_ != 0 && (openMode_[0] != 'r' || openMode_[1] == '+')) {
            std::fflush(fp_);
#if defined WIN32 && !defined __CYGWIN__
            // This is required on msvcrt before stat after writing to a file
            _commit(_fileno(fp_));
#endif
        }

        StructStat buf;
        int ret = stat(buf);

        if (ret != 0) return -1;
        return buf.st_size;
    }

    int FileIo::open()
    {
        // Default open is in read-only binary mode
        return open("rb");
    }

    int FileIo::open(const std::string& mode)
    {
        close();
        openMode_ = mode;
        opMode_ = opSeek;
#ifdef EXV_UNICODE_PATH
        if (wpMode_ == wpUnicode) {
            fp_ = ::_wfopen(wpath_.c_str(), s2ws(mode).c_str());
        }
        else
#endif
        {
            fp_ = ::fopen(path_.c_str(), mode.c_str());
        }
        if (!fp_) return 1;
        return 0;
    }

    bool FileIo::isopen() const
    {
        return fp_ != 0;
    }

    int FileIo::close()
    {
        int rc = 0;
        if (munmap() != 0) rc = 2;
        if (fp_ != 0) {
            if (std::fclose(fp_) != 0) rc |= 1;
            fp_= 0;
        }
        return rc;
    }

    DataBuf FileIo::read(long rcount)
    {
        assert(fp_ != 0);
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long FileIo::read(byte* buf, long rcount)
    {
        assert(fp_ != 0);
        if (switchMode(opRead) != 0) return 0;
        return (long)std::fread(buf, 1, rcount, fp_);
    }

    int FileIo::getb()
    {
        assert(fp_ != 0);
        if (switchMode(opRead) != 0) return EOF;
        return getc(fp_);
    }

    int FileIo::error() const
    {
        return fp_ != 0 ? ferror(fp_) : 0;
    }

    bool FileIo::eof() const
    {
        assert(fp_ != 0);
        return feof(fp_) != 0;
    }

    std::string FileIo::path() const
    {
#ifdef EXV_UNICODE_PATH
        if (wpMode_ == wpUnicode) {
            return ws2s(wpath_);
        }
#endif
        return path_;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring FileIo::wpath() const
    {
        if (wpMode_ == wpStandard) {
            return s2ws(path_);
        }
        return wpath_;
    }

#endif
    MemIo::MemIo()
        : data_(0),
          idx_(0),
          size_(0),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    MemIo::MemIo(const byte* data, long size)
        : data_(const_cast<byte*>(data)),
          idx_(0),
          size_(size),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    MemIo::~MemIo()
    {
        if (isMalloced_) {
            std::free(data_);
        }
    }

    BasicIo::AutoPtr MemIo::temporary() const
    {
        return BasicIo::AutoPtr(new MemIo);
    }

    void MemIo::reserve(long wcount)
    {
        long need = wcount + idx_;

        if (!isMalloced_) {
            // Minimum size for 1st block is 32kB
            long size  = std::max(32768 * (1 + need / 32768), size_);
            byte* data = (byte*)std::malloc(size);
            std::memcpy(data, data_, size_);
            data_ = data;
            sizeAlloced_ = size;
            isMalloced_ = true;
        }

        if (need > size_) {
            if (need > sizeAlloced_) {
                // Allocate in blocks of 32kB
                long want = 32768 * (1 + need / 32768);
                data_ = (byte*)std::realloc(data_, want);
                sizeAlloced_ = want;
                isMalloced_ = true;
            }
            size_ = need;
        }
    }

    long MemIo::write(const byte* data, long wcount)
    {
        reserve(wcount);
        assert(isMalloced_);
        std::memcpy(&data_[idx_], data, wcount);
        idx_ += wcount;
        return wcount;
    }

    void MemIo::transfer(BasicIo& src)
    {
        MemIo *memIo = dynamic_cast<MemIo*>(&src);
        if (memIo) {
            // Optimization if src is another instance of MemIo
            if (true == isMalloced_) {
                std::free(data_);
            }
            idx_ = 0;
            data_ = memIo->data_;
            size_ = memIo->size_;
            isMalloced_ = memIo->isMalloced_;
            memIo->idx_ = 0;
            memIo->data_ = 0;
            memIo->size_ = 0;
            memIo->isMalloced_ = false;
        }
        else {
            // Generic reopen to reset position to start
            if (src.open() != 0) {
                throw Error(9, src.path(), strError());
            }
            idx_ = 0;
            write(src);
            src.close();
        }
        if (error() || src.error()) throw Error(19, strError());
    }

    long MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this) == &src) return 0;
        if (!src.isopen()) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            write(buf, readCount);
            writeTotal += readCount;
        }

        return writeTotal;
    }

    int MemIo::putb(byte data)
    {
        reserve(1);
        assert(isMalloced_);
        data_[idx_++] = data;
        return data;
    }

    int MemIo::seek(long offset, Position pos)
    {
        long newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = size_ + offset; break;
        }

        if (newIdx < 0 || newIdx > size_) return 1;
        idx_ = newIdx;
        eof_ = false;
        return 0;
    }

    byte* MemIo::mmap(bool /*isWriteable*/)
    {
        return data_;
    }

    int MemIo::munmap()
    {
        return 0;
    }

    long MemIo::tell() const
    {
        return idx_;
    }

    long MemIo::size() const
    {
        return size_;
    }

    int MemIo::open()
    {
        idx_ = 0;
        eof_ = false;
        return 0;
    }

    bool MemIo::isopen() const
    {
        return true;
    }

    int MemIo::close()
    {
        return 0;
    }

    DataBuf MemIo::read(long rcount)
    {
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long MemIo::read(byte* buf, long rcount)
    {
        long avail = size_ - idx_;
        long allow = std::min(rcount, avail);
        std::memcpy(buf, &data_[idx_], allow);
        idx_ += allow;
        if (rcount > avail) eof_ = true;
        return allow;
    }

    int MemIo::getb()
    {
        if (idx_ == size_) {
            eof_ = true;
            return EOF;
        }
        return data_[idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }

    bool MemIo::eof() const
    {
        return eof_;
    }

    std::string MemIo::path() const
    {
        return "MemIo";
    }

#ifdef EXV_UNICODE_PATH
    std::wstring MemIo::wpath() const
    {
        return EXV_WIDEN("MemIo");
    }

#endif
    // *************************************************************************
    // free functions

    DataBuf readFile(const std::string& path)
    {
        FileIo file(path);
        if (file.open("rb") != 0) {
            throw Error(10, path, "rb", strError());
        }
        struct stat st;
        if (0 != ::stat(path.c_str(), &st)) {
            throw Error(2, path, strError(), "::stat");
        }
        DataBuf buf(st.st_size);
        long len = file.read(buf.pData_, buf.size_);
        if (len != buf.size_) {
            throw Error(2, path, strError(), "FileIo::read");
        }
        return buf;
    }

#ifdef EXV_UNICODE_PATH
    DataBuf readFile(const std::wstring& wpath)
    {
        FileIo file(wpath);
        if (file.open("rb") != 0) {
            throw Error(10, ws2s(wpath), "rb", strError());
        }
        struct _stat st;
        if (0 != ::_wstat(wpath.c_str(), &st)) {
            throw Error(2, ws2s(wpath), strError(), "::_wstat");
        }
        DataBuf buf(st.st_size);
        long len = file.read(buf.pData_, buf.size_);
        if (len != buf.size_) {
            throw Error(2, ws2s(wpath), strError(), "FileIo::read");
        }
        return buf;
    }

#endif
    long writeFile(const DataBuf& buf, const std::string& path)
    {
        FileIo file(path);
        if (file.open("wb") != 0) {
            throw Error(10, path, "wb", strError());
        }
        return file.write(buf.pData_, buf.size_);
    }

#ifdef EXV_UNICODE_PATH
    long writeFile(const DataBuf& buf, const std::wstring& wpath)
    {
        FileIo file(wpath);
        if (file.open("wb") != 0) {
            throw Error(10, ws2s(wpath), "wb", strError());
        }
        return file.write(buf.pData_, buf.size_);
    }

#endif
}                                       // namespace Exiv2
