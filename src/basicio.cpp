// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      basicio.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   04-Dec-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

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
#include <cassert>
#include <cstdio>                       // for remove()
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
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

    FileIo::FileIo(const std::string& path) : 
        path_(path), fp_(0), opMode_(opSeek)
    {
    }
        
    FileIo::~FileIo()
    {
        close();
    }

    BasicIo::AutoPtr FileIo::temporary() const
    {
        BasicIo::AutoPtr basicIo;
        
        struct stat buf;
        int ret = stat(path_.c_str(), &buf);
        
        // If file is > 1MB then use a file, otherwise use memory buffer
        if (ret != 0 || buf.st_size > 1048576) {
            pid_t pid = getpid();
            std::string tmpname = path_ + toString(pid);
            std::auto_ptr<FileIo> fileIo(new FileIo(tmpname));
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
        std::string mode = "r+b";

        switch(opMode) {
        case opRead:
            // Flush if current mode allows reading, else reopen (in mode "r+b"
            // as in this case we know that we can write to the file)
            if (   openMode_[0] == 'r' 
                || openMode_.substr(0, 2) == "w+" 
                || openMode_.substr(0, 2) == "a+") reopen = false;
            break;
        case opWrite:
            // Flush if current mode allows writing, else reopen
            if (   openMode_.substr(0, 2) == "r+" 
                || openMode_[0] == 'w'
                || openMode_[0] == 'a') reopen = false;
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
            fseek(fp_, 0, SEEK_CUR);
            return 0;
        }

        // Reopen the file
        long offset = ftell(fp_);
        if (offset == -1) return -1;
        if (open(mode) != 0) return 1;
        return fseek(fp_, offset, SEEK_SET);
    }

    long FileIo::write(const byte* data, long wcount)
    {
        assert(fp_ != 0);
        if (switchMode(opWrite) != 0) return 0;
        return (long)fwrite(data, 1, wcount, fp_);
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
            writeTotal += writeCount = (long)fwrite(buf, 1, readCount, fp_);
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
            // Optimization if this is another instance of FileIo
            close();
            fileIo->close();
            // MSVCRT rename that does not overwrite existing files
            if (remove(path_.c_str()) != 0) {
                throw Error(2, path_, strError(), "::remove");
            }
            if (rename(fileIo->path_.c_str(), path_.c_str()) == -1) {
                throw Error(17, fileIo->path_, path_, strError());
            }
            remove(fileIo->path_.c_str());
        }
        else{
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
        int fileSeek;
        if (pos == BasicIo::cur) {
            fileSeek = SEEK_CUR;
        }
        else if (pos == BasicIo::beg) {
            fileSeek = SEEK_SET;
        }
        else {
            assert(pos == BasicIo::end);
            fileSeek = SEEK_END;
        }

        if (switchMode(opSeek) != 0) return 1;
        return fseek(fp_, offset, fileSeek);
    }
        
    long FileIo::tell() const
    {
        assert(fp_ != 0);
        return ftell(fp_);
    }


    long FileIo::size() const
    {
        if (fp_ != 0) {
            fflush(fp_);
#if defined WIN32 && !defined __CYGWIN__
            // This is required on msvcrt before stat after writing to a file
            _commit(_fileno(fp_));
#endif
        }

        struct stat buf;
        int ret = stat(path_.c_str(), &buf);
        
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
        if (fp_ != 0) {
            fclose(fp_);
        }

        openMode_ = mode;
        opMode_ = opSeek;
        fp_ = fopen(path_.c_str(), mode.c_str());
        if (!fp_) return 1;
        return 0;
    }

    bool FileIo::isopen() const
    {
        return fp_ != 0;
    }
    
    int FileIo::close()
    {
        if (fp_ != 0) {
            fclose(fp_);
            fp_= 0;
        }
        return 0;
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
        return (long)fread(buf, 1, rcount, fp_);
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
        return path_;
    }

    MemIo::MemIo(const byte* data, long size)
    {
        // If copying data is too slow it might be worth
        // creating a readonly MemIo variant
        data_.reserve(size);
        data_.assign(data, data+size);
        idx_ = 0;
    }

    BasicIo::AutoPtr MemIo::temporary() const
    {
        return BasicIo::AutoPtr(new MemIo);
    }
        
    void MemIo::checkSize(long wcount)
    {
        ByteVector::size_type need = wcount + idx_;
        if (need > data_.size()) {
            data_.resize(need);
        }
    }

    long MemIo::write(const byte* data, long wcount)
    {
        checkSize(wcount);
        // According to Josuttis 6.2.3 this is safe
        memcpy(&data_[idx_], data, wcount);
        idx_ += wcount;
        return wcount;
    }

    void MemIo::transfer(BasicIo& src)
    {
        MemIo *memIo = dynamic_cast<MemIo*>(&src);
        if (memIo) {
            // Optimization if this is another instance of MemIo
            data_.swap(memIo->data_);
            idx_ = 0;
        }
        else{
            // Generic reopen to reset position to start
            data_.clear();
            idx_ = 0;
            if (src.open() != 0) {
                throw Error(9, src.path(), strError());
            }
            write(src);
            src.close();    
        }
        if (error() || src.error()) throw Error(19, strError());
    }

    long MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this)==&src) return 0;
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
        checkSize(1);
        data_[idx_++] = data;
        return data;
    }
    
    int MemIo::seek(long offset, Position pos)
    {
        ByteVector::size_type newIdx;
        
        if (pos == BasicIo::cur ) {
            newIdx = idx_ + offset;
        }
        else if (pos == BasicIo::beg) {
            newIdx = offset;
        }
        else {
            assert(pos == BasicIo::end);
            newIdx = data_.size() + offset;
        }

        if (newIdx < 0 || newIdx > data_.size()) return 1;
        idx_ = newIdx;
        return 0;
    }

    long MemIo::tell() const
    {
        return (long)idx_;
    }

    long MemIo::size() const
    {
        return (long)data_.size();
    }
    
    int MemIo::open()
    {
        idx_ = 0;
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
        long avail = (long)(data_.size() - idx_);
        long allow = std::min(rcount, avail);
        
        // According to Josuttis 6.2.3 this is safe
        memcpy(buf, &data_[idx_], allow);
        idx_ += allow;
        return allow;
    }

    int MemIo::getb()
    {
        if (idx_ == data_.size())
            return EOF;
        return data_[idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }
    
    bool MemIo::eof() const
    {
        return idx_ == data_.size();
    }

    std::string MemIo::path() const
    {
        return "MemIo";
    }

}                                       // namespace Exiv2
