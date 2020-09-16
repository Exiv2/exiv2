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
#include "error.hpp"
#include "futils.hpp"

#include <cstring>  // std::memcpy
#include <cassert>      /// \todo check usages of assert and try to cover the negative case with unit tests.

namespace Exiv2
{
    //! Internal Pimpl structure of class MemIo.
    class MemIo::Impl
    {
    public:
        Impl() = default;
        Impl(const byte* data, size_t size)
            : data_(const_cast<byte*>(data))
            , size_(size)
        {
        }

        Impl& operator=(const Impl& rhs) = delete;
        Impl& operator=(const Impl&& rhs) = delete;
        Impl(const Impl& rhs) = delete;
        Impl(const Impl&& rhs) = delete;

        // DATA
        byte* data_{nullptr};     //!< Pointer to the start of the memory area
        size_t idx_{0};           //!< Index into the memory area
        size_t size_{0};          //!< Size of the memory area
        size_t sizeAlloced_{0};   //!< Size of the allocated buffer
        bool isMalloced_{false};  //!< Was the buffer allocated?
        bool eof_{false};         //!< EOF indicator

        // METHODS
        void reserve(size_t wcount);  //!< Reserve memory
    };

    void MemIo::Impl::reserve(size_t wcount)
    {
        const size_t need = wcount + idx_;
        size_t blockSize = 32 * 1024;  // 32768           `
        const size_t maxBlockSize = 4 * 1024 * 1024;

        if (!isMalloced_) {
            // Minimum size for 1st block
            size_t size = std::max(blockSize * (1 + need / blockSize), size_);
            auto data = static_cast<byte*>(std::malloc(size));
            if (data == nullptr) {
                throw Error(kerMallocFailed);
            }
            if (data_ != nullptr) {
                std::memcpy(data, data_, size_);
            }
            data_ = data;
            sizeAlloced_ = size;
            isMalloced_ = true;
        }

        if (need > size_) {
            if (need > sizeAlloced_) {
                blockSize = 2 * sizeAlloced_;
                if (blockSize > maxBlockSize)
                    blockSize = maxBlockSize;
                // Allocate in blocks
                size_t want = blockSize * (1 + need / blockSize);
                data_ = static_cast<byte*>(std::realloc(data_, want));
                if (data_ == nullptr) {
                    throw Error(kerMallocFailed);
                }
                sizeAlloced_ = want;
                isMalloced_ = true;
            }
            size_ = need;
        }
    }

    MemIo::MemIo()
        : p_(new Impl())
    {
    }

    MemIo::MemIo(const byte* data, size_t size)
        : p_(new Impl(data, size))
    {
    }

    MemIo::~MemIo()
    {
        if (p_->isMalloced_) {
            std::free(p_->data_);
        }
    }

    size_t MemIo::write(const byte* data, size_t wcount)
    {
        p_->reserve(wcount);
        assert(p_->isMalloced_);
        if (data != nullptr) {
            std::memcpy(&p_->data_[p_->idx_], data, wcount);
        }
        p_->idx_ += wcount;
        return wcount;
    }

    size_t MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this) == &src)
            return 0;
        if (!src.isopen())
            return 0;

        byte buf[4096];
        size_t readCount = 0;
        size_t writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            write(buf, readCount);
            writeTotal += readCount;
        }

        return writeTotal;
    }

    void MemIo::transfer(BasicIo& src)
    {
        auto memIo = dynamic_cast<MemIo*>(&src);
        if (memIo) {
            // Optimization if src is another instance of MemIo
            if (p_->isMalloced_) {
                std::free(p_->data_);
            }
            p_->idx_ = 0;
            p_->data_ = memIo->p_->data_;
            p_->size_ = memIo->p_->size_;
            p_->isMalloced_ = memIo->p_->isMalloced_;
            memIo->p_->idx_ = 0;
            memIo->p_->data_ = nullptr;
            memIo->p_->size_ = 0;
            memIo->p_->isMalloced_ = false;
        } else {
            // Generic reopen to reset position to start
            if (src.open() != 0) {
                throw Error(kerDataSourceOpenFailed, src.path(), strError());
            }
            p_->idx_ = 0;
            write(src);
            src.close();
        }
        if (error() || src.error())
            throw Error(kerMemoryTransferFailed, strError());
    }

    int MemIo::putb(byte data)
    {
        p_->reserve(1);
        assert(p_->isMalloced_);
        p_->data_[p_->idx_++] = data;
        return data;
    }

    int MemIo::seek(int64 offset, Position pos)
    {
        int64 newIdx = 0;

        switch (pos) {
            case BasicIo::cur:
                newIdx = p_->idx_ + offset;
                break;
            case BasicIo::beg:
                newIdx = offset;
                break;
            case BasicIo::end:
                newIdx = p_->size_ + offset;
                break;
        }

        if (newIdx < 0)
            return 1;

        if (static_cast<size_t>(newIdx) > p_->size_) {
            p_->eof_ = true;
            return 1;
        }

        p_->idx_ = static_cast<size_t>(newIdx);
        p_->eof_ = false;
        return 0;
    }

    byte* MemIo::mmap(bool /*isWriteable*/)
    {
        return p_->data_;
    }

    int MemIo::munmap()
    {
        return 0;
    }

    int64 MemIo::tell() const
    {
        return static_cast<int64>(p_->idx_);
    }

    size_t MemIo::size() const
    {
        return p_->size_;
    }

    int MemIo::open()
    {
        p_->idx_ = 0;
        p_->eof_ = false;
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

    /// \todo change type of rcount to size_t
    DataBuf MemIo::read(size_t rcount) noexcept
    {
        if (rcount > size()) {
            return {};
        }
        DataBuf buf(rcount);
        size_t readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    size_t MemIo::read(byte* buf, size_t rcount)
    {
        size_t avail = std::max(p_->size_ - p_->idx_, 0_z);
        size_t allow = std::min(rcount, avail);
        if (p_->data_ == nullptr) {
            throw Error(kerCallFailed, "std::memcpy with src == nullptr");
        }
        std::memcpy(buf, &p_->data_[p_->idx_], allow);
        p_->idx_ += allow;
        if (rcount > avail)
            p_->eof_ = true;
        return allow;
    }

    int MemIo::getb()
    {
        if (p_->idx_ >= p_->size_) {
            p_->eof_ = true;
            return EOF;
        }
        return p_->data_[p_->idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }

    bool MemIo::eof() const
    {
        return p_->eof_;
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
    void MemIo::populateFakeData()
    {
    }
}  // namespace Exiv2
