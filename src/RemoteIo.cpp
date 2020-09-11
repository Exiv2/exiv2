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
#include "datasets.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "http.hpp"

#ifdef EXV_USE_CURL
#include <curl/curl.h>
#endif

#include <cstring>  // std::memcpy
#include <cassert>      /// \todo check usages of assert and try to cover the negative case with unit tests.
#include <iostream>

namespace Exiv2
{
    /*!
      @brief Utility class provides the block mapping to the part of data. This
      avoids allocating a single contiguous block of memory to the big data.
     */
    class EXIV2API BlockMap
    {
    public:
        //! the status of the block.
        enum blockType_e
        {
            bNone,
            bKnown,
            bMemory
        };
        //! @name Creators
        //@{
        //! Default constructor. the init status of the block is bNone.
        BlockMap() = default;

        //! Destructor. Releases all managed memory.
        ~BlockMap()
        {
            if (data_) {
                std::free(data_);
                data_ = nullptr;
            }
        }

        //! @brief Populate the block.
        //! @param source The data populate to the block
        //! @param num The size of data
        void populate(const byte* source, size_t num)
        {
            size_ = num;
            data_ = (byte*)std::malloc(size_);
            type_ = bMemory;
            std::memcpy(data_, source, size_);
        }

        /*!
          @brief Change the status to bKnow. bKnow blocks do not contain the
          data, but they keep the size of data. This avoids allocating memory
          for parts of the file that contain image-date (non-metadata/pixel
          data) which never change in exiv2.
          @param num The size of the data
         */
        void markKnown(size_t num)
        {
            type_ = bKnown;
            size_ = num;
        }

        bool isNone() const
        {
            return type_ == bNone;
        }
        bool isInMem() const
        {
            return type_ == bMemory;
        }
        bool isKnown() const
        {
            return type_ == bKnown;
        }
        byte* getData() const
        {
            return data_;
        }
        size_t getSize() const
        {
            return size_;
        }

    private:
      blockType_e type_{bNone};
      byte *data_{nullptr};
      size_t size_{0};
    };

    //! Internal Pimpl abstract structure of class RemoteIo.
    class RemoteIo::Impl
    {
    public:
        //! Constructor
        Impl(const std::string& url, size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        Impl(const std::wstring& wpath, size_t blockSize);
#endif
        //! Destructor. Releases all managed memory.
        virtual ~Impl();

        // DATA
        std::string path_;  //!< (Standard) path
#ifdef EXV_UNICODE_PATH
        std::wstring wpath_;  //!< Unicode path
#endif
        size_t blockSize_;     //!< Size of the block memory.
        BlockMap* blocksMap_;  //!< An array contains all blocksMap
        size_t size_;          //!< The file size
        size_t idx_;           //!< Index into the memory area
        bool isMalloced_;      //!< Was the blocksMap_ allocated?
        bool eof_;             //!< EOF indicator
        Protocol protocol_;    //!< the protocol of url
        uint32_t totalRead_;   //!< bytes requested from host

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the
          length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        virtual long getFileLength() = 0;
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file
          content.
         */
        virtual void getDataByRange(long lowBlock, long highBlock, std::string& response) = 0;
        /*!
          @brief Submit the data to the remote machine. The data replace a part
          of the remote file. The replaced part of remote file is indicated by
          from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data
          replace.
          @param to The end position in the remote file where the data replace.
          @note The write access is available on some protocols. HTTP and HTTPS
          require the script file on the remote machine to handle the data. SSH
          requires the permission to edit the file.
          @throw Error if it fails.
         */
        virtual void writeRemote(const byte* data, size_t size, long from, long to) = 0;
        /*!
          @brief Get the data from the remote machine and write them to the
          memory blocks.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @return Number of bytes written to the memory block successfully
          @throw Error if it fails.
         */
        virtual size_t populateBlocks(size_t lowBlock, size_t highBlock);

    };  // class RemoteIo::Impl

    RemoteIo::Impl::Impl(const std::string& url, size_t blockSize)
        : path_(url)
        , blockSize_(blockSize)
        , blocksMap_(nullptr)
        , size_(0)
        , idx_(0)
        , isMalloced_(false)
        , eof_(false)
        , protocol_(fileProtocol(url))
        , totalRead_(0)
    {
    }
#ifdef EXV_UNICODE_PATH
    RemoteIo::Impl::Impl(const std::wstring& wurl, size_t blockSize)
        : wpath_(wurl)
        , blockSize_(blockSize)
        , blocksMap_(0)
        , size_(0)
        , idx_(0)
        , isMalloced_(false)
        , eof_(false)
        , protocol_(fileProtocol(wurl))
    {
    }
#endif

    size_t RemoteIo::Impl::populateBlocks(size_t lowBlock, size_t highBlock)
    {
        assert(isMalloced_);

        // optimize: ignore all true blocks on left & right sides.
        while (!blocksMap_[lowBlock].isNone() && lowBlock < highBlock)
            lowBlock++;
        while (!blocksMap_[highBlock].isNone() && highBlock > lowBlock)
            highBlock--;

        size_t rcount = 0;
        if (blocksMap_[highBlock].isNone()) {
            std::string data;
            getDataByRange(static_cast<long>(lowBlock), static_cast<long>(highBlock), data);
            rcount = data.length();
            if (rcount == 0) {
                throw Error(kerErrorMessage, "Data By Range is empty. Please check the permission.");
            }
            const auto source = reinterpret_cast<const byte*>(data.c_str());
            size_t remain = rcount, totalRead = 0;
            size_t iBlock = (rcount == size_) ? 0 : lowBlock;

            while (remain) {
                size_t allow = std::min(remain, blockSize_);
                blocksMap_[iBlock].populate(&source[totalRead], allow);
                remain -= allow;
                totalRead += allow;
                iBlock++;
            }
        }

        return rcount;
    }

    RemoteIo::Impl::~Impl()
    {
        if (blocksMap_)
            delete[] blocksMap_;
    }

    RemoteIo::~RemoteIo()
    {
        if (p_) {
            close();
            delete p_;
        }
    }

    int RemoteIo::open()
    {
        close();  // reset the IO position
        bigBlock_ = nullptr;
        if (!p_->isMalloced_) {
            long length = p_->getFileLength();
            if (length < 0) {  // unable to get the length of remote file, get
                               // the whole file content.
                std::string data;
                p_->getDataByRange(-1, -1, data);
                p_->size_ = data.length();
                size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
                p_->blocksMap_ = new BlockMap[nBlocks];
                p_->isMalloced_ = true;
                const auto source = reinterpret_cast<const byte*>(data.c_str());
                size_t remain = p_->size_, iBlock = 0, totalRead = 0;
                while (remain) {
                    size_t allow = std::min(remain, p_->blockSize_);
                    p_->blocksMap_[iBlock].populate(&source[totalRead], allow);
                    remain -= allow;
                    totalRead += allow;
                    iBlock++;
                }
            } else if (length == 0) {  // file is empty
                throw Error(kerErrorMessage, "the file length is 0");
            } else {
                p_->size_ = static_cast<size_t>(length);
                size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
                p_->blocksMap_ = new BlockMap[nBlocks];
                p_->isMalloced_ = true;
            }
        }
        return 0;  // means OK
    }

    int RemoteIo::close()
    {
        if (p_->isMalloced_) {
            p_->eof_ = false;
            p_->idx_ = 0;
        }
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "RemoteIo::close totalRead_ = " << p_->totalRead_ << std::endl;
#endif
        if (bigBlock_) {
            delete[] bigBlock_;
            bigBlock_ = nullptr;
        }
        return 0;
    }

    size_t RemoteIo::write(const byte* /* unused data*/, size_t /* unused wcount*/)
    {
        return 0;
    }

    size_t RemoteIo::write(BasicIo& src)
    {
        assert(p_->isMalloced_);
        if (!src.isopen())
            return 0;

        /*
         * The idea is to compare the file content, find the different bytes and
         * submit them to the remote machine. To simplify it, it:
         *      + goes from the left, find the first different position -> $left
         *      + goes from the right, find the first different position ->
         * $right The different bytes are [$left-$right] part.
         */
        size_t left = 0;
        size_t right = 0;
        size_t blockIndex = 0;
        size_t i = 0;
        size_t readCount = 0;
        size_t blockSize = 0;
        auto buf = (byte*)std::malloc(p_->blockSize_);
        size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;

        // find $left
        src.seek(0, BasicIo::beg);
        bool findDiff = false;
        while (blockIndex < nBlocks && !src.eof() && !findDiff) {
            blockSize = p_->blocksMap_[blockIndex].getSize();
            bool isFakeData = p_->blocksMap_[blockIndex].isKnown();  // fake data
            readCount = src.read(buf, blockSize);
            byte* blockData = p_->blocksMap_[blockIndex].getData();
            for (i = 0; (i < readCount) && (i < blockSize) && !findDiff; i++) {
                if ((!isFakeData && buf[i] != blockData[i]) || (isFakeData && buf[i] != 0)) {
                    findDiff = true;
                } else {
                    left++;
                }
            }
            blockIndex++;
        }

        // find $right
        findDiff = false;
        blockIndex = nBlocks - 1;
        blockSize = p_->blocksMap_[blockIndex].getSize();
        while ((blockIndex + 1 > 0) && right < src.size() && !findDiff) {
            if (src.seek(-1 * (blockSize + right), BasicIo::end)) {
                findDiff = true;
            } else {
                bool isFakeData = p_->blocksMap_[blockIndex].isKnown();  // fake data
                readCount = src.read(buf, (long)blockSize);
                byte* blockData = p_->blocksMap_[blockIndex].getData();
                for (i = 0; (i < readCount) && (i < blockSize) && !findDiff; i++) {
                    if ((!isFakeData && buf[readCount - i - 1] != blockData[blockSize - i - 1]) ||
                        (isFakeData && buf[readCount - i - 1] != 0)) {
                        findDiff = true;
                    } else {
                        right++;
                    }
                }
            }
            blockIndex--;
            blockSize = (long)p_->blocksMap_[blockIndex].getSize();
        }

        // free buf
        if (buf)
            std::free(buf);

        // submit to the remote machine.
        long dataSize = (long)(src.size() - left - right);
        if (dataSize > 0) {
            auto data = (byte*)std::malloc(dataSize);
            src.seek(left, BasicIo::beg);
            src.read(data, dataSize);
            p_->writeRemote(data, (size_t)dataSize, (long)left, (long)(p_->size_ - right));
            if (data)
                std::free(data);
        }
        return (long)src.size();
    }

    int RemoteIo::putb(byte /*unused data*/)
    {
        return 0;
    }

    DataBuf RemoteIo::read(size_t rcount) noexcept
    {
        DataBuf buf(rcount);
        size_t readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    size_t RemoteIo::read(byte* buf, size_t rcount)
    {
        assert(p_->isMalloced_);
        if (p_->eof_)
            return 0;
        p_->totalRead_ += (uint32_t)rcount;

        size_t allow = std::min(rcount, p_->size_ - p_->idx_);
        size_t lowBlock = p_->idx_ / p_->blockSize_;
        size_t highBlock = (p_->idx_ + allow) / p_->blockSize_;

        // connect to the remote machine & populate the blocks just in time.
        p_->populateBlocks(lowBlock, highBlock);
        auto fakeData = (byte*)std::calloc(p_->blockSize_, sizeof(byte));
        if (!fakeData) {
            throw Error(kerErrorMessage, "Unable to allocate data");
        }

        size_t iBlock = lowBlock;
        size_t startPos = p_->idx_ - lowBlock * p_->blockSize_;
        size_t totalRead = 0;
        do {
            byte* data = p_->blocksMap_[iBlock++].getData();
            if (data == nullptr)
                data = fakeData;
            size_t blockR = std::min(allow, p_->blockSize_ - startPos);
            std::memcpy(&buf[totalRead], &data[startPos], blockR);
            totalRead += blockR;
            startPos = 0;
            allow -= blockR;
        } while (allow);

        if (fakeData)
            std::free(fakeData);

        p_->idx_ += (long)totalRead;
        p_->eof_ = (p_->idx_ == p_->size_);

        return totalRead;
    }

    int RemoteIo::getb()
    {
        assert(p_->isMalloced_);
        if (p_->idx_ == p_->size_) {
            p_->eof_ = true;
            return EOF;
        }

        size_t expectedBlock = p_->idx_ / p_->blockSize_;
        // connect to the remote machine & populate the blocks just in time.
        p_->populateBlocks(expectedBlock, expectedBlock);

        byte* data = p_->blocksMap_[expectedBlock].getData();
        return data[p_->idx_++ - expectedBlock * p_->blockSize_];
    }

    void RemoteIo::transfer(BasicIo& src)
    {
        if (src.open() != 0) {
            throw Error(kerErrorMessage, "unable to open src when transferring");
        }
        write(src);
        src.close();
    }

    int RemoteIo::seek(int64 offset, Position pos)
    {
        assert(p_->isMalloced_);
        int64 newIdx = 0;

        switch (pos) {
            case BasicIo::cur:
                newIdx = static_cast<int64>(p_->idx_) + offset;
                break;
            case BasicIo::beg:
                newIdx = offset;
                break;
            case BasicIo::end:
                newIdx = static_cast<int64>(p_->size_) + offset;
                break;
        }

        // #1198.  Don't return 1 when asked to seek past EOF.  Stay calm and
        // set eof_ if (newIdx < 0 || newIdx > (long) p_->size_) return 1;
        p_->idx_ = static_cast<size_t>(newIdx);
        p_->eof_ = newIdx > static_cast<int64>(p_->size_);
        if (p_->idx_ > p_->size_)
            p_->idx_ = p_->size_;
        return 0;
    }

    byte* RemoteIo::mmap(bool /*isWriteable*/)
    {
        if (!bigBlock_) {
            size_t nRealData = 0;
            size_t blockSize = p_->blockSize_;
            size_t blocks = (p_->size_ + blockSize - 1) / blockSize;
            bigBlock_ = new byte[blocks * blockSize];
            for (size_t block = 0; block < blocks; block++) {
                void* p = p_->blocksMap_[block].getData();
                if (p) {
                    nRealData += blockSize;
                    memcpy(bigBlock_ + (block * blockSize), p, blockSize);
                }
            }
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "RemoteIo::mmap nRealData = " << nRealData << std::endl;
#endif
        }

        return bigBlock_;
    }

    int RemoteIo::munmap()
    {
        return 0;
    }

    int64 RemoteIo::tell() const
    {
        return static_cast<int64>(p_->idx_);
    }

    size_t RemoteIo::size() const
    {
        return p_->size_;
    }

    bool RemoteIo::isopen() const
    {
        return p_->isMalloced_;
    }

    int RemoteIo::error() const
    {
        return 0;
    }

    bool RemoteIo::eof() const
    {
        return p_->eof_;
    }

    std::string RemoteIo::path() const
    {
        return p_->path_;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring RemoteIo::wpath() const
    {
        return p_->wpath_;
    }
#endif

    void RemoteIo::populateFakeData()
    {
        assert(p_->isMalloced_);
        size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
        for (size_t i = 0; i < nBlocks; i++) {
            if (p_->blocksMap_[i].isNone())
                p_->blocksMap_[i].markKnown(p_->blockSize_);
        }
    }

    //! Internal Pimpl structure of class HttpIo.
    class HttpIo::HttpImpl : public Impl
    {
    public:
        //! Constructor
        HttpImpl(const std::string& url, size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        HttpImpl(const std::wstring& wpath, size_t blockSize);
#endif
        Exiv2::Uri hostInfo_;  //!< the host information extracted from the path

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the
          length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        long getFileLength() override;
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file
          content.
         */
        void getDataByRange(long lowBlock, long highBlock, std::string& response) override;
        /*!
          @brief Submit the data to the remote machine. The data replace a part
          of the remote file. The replaced part of remote file is indicated by
          from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data
          replace.
          @param to The end position in the remote file where the data replace.
          @note The data are submitted to the remote machine via POST. This
          requires the script file on the remote machine to receive the data and
          edit the remote file. The server-side script may be specified with the
          environment string EXIV2_HTTP_POST. The default value is
                "/exiv2.php". More info is available at
          http://dev.exiv2.org/wiki/exiv2
          @throw Error if it fails.
         */
        void writeRemote(const byte* data, size_t size, long from, long to) override;

        HttpImpl& operator=(const HttpImpl& rhs) = delete;
        HttpImpl& operator=(const HttpImpl&& rhs) = delete;
        HttpImpl(const HttpImpl& rhs) = delete;
        HttpImpl(const HttpImpl&& rhs) = delete;
    };  // class HttpIo::HttpImpl

    HttpIo::HttpImpl::HttpImpl(const std::string& url, size_t blockSize)
        : Impl(url, blockSize)
        , hostInfo_(Exiv2::Uri::Parse(url))
    {
        Exiv2::Uri::Decode(hostInfo_);
    }
#ifdef EXV_UNICODE_PATH
    HttpIo::HttpImpl::HttpImpl(const std::wstring& wurl, size_t blockSize)
        : Impl(wurl, blockSize)
    {
        std::string url;
        url.assign(wurl.begin(), wurl.end());
        path_ = url;

        hostInfo_ = Exiv2::Uri::Parse(url);
        Exiv2::Uri::Decode(hostInfo_);
    }
#endif

    long HttpIo::HttpImpl::getFileLength()
    {
        Exiv2::Dictionary response;
        Exiv2::Dictionary request;
        std::string errors;
        request["server"] = hostInfo_.Host;
        request["page"] = hostInfo_.Path;
        if (!hostInfo_.Port.empty())
            request["port"] = hostInfo_.Port;
        request["verb"] = "HEAD";
        int serverCode = http(request, response, errors);
        if (serverCode < 0 || serverCode >= 400 || !errors.empty()) {
            throw Error(kerTiffDirectoryTooLarge, "Server", serverCode);
        }

        Exiv2::Dictionary_i lengthIter = response.find("Content-Length");
        return (lengthIter == response.end()) ? -1 : atol((lengthIter->second).c_str());
    }

    void HttpIo::HttpImpl::getDataByRange(long lowBlock, long highBlock, std::string& response)
    {
        Exiv2::Dictionary responseDic;
        Exiv2::Dictionary request;
        request["server"] = hostInfo_.Host;
        request["page"] = hostInfo_.Path;
        if (!hostInfo_.Port.empty())
            request["port"] = hostInfo_.Port;
        request["verb"] = "GET";
        std::string errors;
        if (lowBlock > -1 && highBlock > -1) {
            std::stringstream ss;
            ss << "Range: bytes=" << lowBlock * blockSize_ << "-" << ((highBlock + 1) * blockSize_ - 1) << "\r\n";
            request["header"] = ss.str();
        }

        int serverCode = http(request, responseDic, errors);
        if (serverCode < 0 || serverCode >= 400 || !errors.empty()) {
            throw Error(kerTiffDirectoryTooLarge, "Server", serverCode);
        }
        response = responseDic["body"];
    }

    void HttpIo::HttpImpl::writeRemote(const byte* data, size_t size, long from, long to)
    {
        std::string scriptPath(getEnv(envHTTPPOST));
        if (scriptPath.empty()) {
            throw Error(kerErrorMessage,
                        "Please set the path of the server script to handle "
                        "http post data to EXIV2_HTTP_POST "
                        "environmental variable.");
        }

        // standadize the path without "/" at the beginning.
        std::size_t protocolIndex = scriptPath.find("://");
        if (protocolIndex == std::string::npos && scriptPath[0] != '/') {
            scriptPath = "/" + scriptPath;
        }

        Exiv2::Dictionary response;
        Exiv2::Dictionary request;
        std::string errors;

        Uri scriptUri = Exiv2::Uri::Parse(scriptPath);
        request["server"] = scriptUri.Host.empty() ? hostInfo_.Host : scriptUri.Host;
        if (!scriptUri.Port.empty())
            request["port"] = scriptUri.Port;
        request["page"] = scriptUri.Path;
        request["verb"] = "POST";

        // encode base64
        size_t encodeLength = ((size + 2) / 3) * 4 + 1;
        auto encodeData = new char[encodeLength];
        base64encode(data, size, encodeData, encodeLength);
        // url encode
        const std::string urlencodeData = urlencode(encodeData);
        delete[] encodeData;

        std::stringstream ss;
        ss << "path=" << hostInfo_.Path << "&"
           << "from=" << from << "&"
           << "to=" << to << "&"
           << "data=" << urlencodeData;
        std::string postData = ss.str();

        // create the header
        ss.str("");
        ss << "Content-Length: " << postData.length() << "\n"
           << "Content-Type: application/x-www-form-urlencoded\n"
           << "\n"
           << postData << "\r\n";
        request["header"] = ss.str();

        int serverCode = http(request, response, errors);
        if (serverCode < 0 || serverCode >= 400 || !errors.empty()) {
            throw Error(kerTiffDirectoryTooLarge, "Server", serverCode);
        }
    }
    HttpIo::HttpIo(const std::string& url, size_t blockSize)
    {
        p_ = new HttpImpl(url, blockSize);
    }
#ifdef EXV_UNICODE_PATH
    HttpIo::HttpIo(const std::wstring& wurl, size_t blockSize)
    {
        p_ = new HttpImpl(wurl, blockSize);
    }
#endif

#ifdef EXV_USE_CURL
    //! Internal Pimpl structure of class RemoteIo.
    class CurlIo::CurlImpl : public Impl
    {
    public:
        //! Constructor
        CurlImpl(const std::string& path, size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        CurlImpl(const std::wstring& wpath, size_t blockSize);
#endif
        //! Destructor. Cleans up the curl pointer and releases all managed
        //! memory.
        ~CurlImpl();

        CURL* curl_;  //!< libcurl pointer

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the
          length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        long getFileLength();
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file
          content.
         */
        void getDataByRange(long lowBlock, long highBlock, std::string& response);
        /*!
          @brief Submit the data to the remote machine. The data replace a part
          of the remote file. The replaced part of remote file is indicated by
          from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data
          replace.
          @param to The end position in the remote file where the data replace.
          @throw Error if it fails.
          @note The write access is only available on HTTP & HTTPS protocols.
          The data are submitted to server via POST method. It requires the
          script file on the remote machine to receive the data and edit the
          remote file. The server-side script may be specified with the
          environment string EXIV2_HTTP_POST. The default value is "/exiv2.php".
          More info is available at http://dev.exiv2.org/wiki/exiv2
         */
        void writeRemote(const byte* data, size_t size, long from, long to);

        CurlImpl& operator=(const CurlImpl& rhs) = delete;
        CurlImpl& operator=(const CurlImpl&& rhs) = delete;
        CurlImpl(const CurlImpl& rhs) = delete;
        CurlImpl(const CurlImpl&& rhs) = delete;

    private:
        long timeout_;  //!< The number of seconds to wait while trying to
                        //!< connect.
    };                  // class RemoteIo::Impl

    CurlIo::CurlImpl::CurlImpl(const std::string& url, size_t blockSize)
        : Impl(url, blockSize)
    {
        // init curl pointer
        curl_ = curl_easy_init();
        if (!curl_) {
            throw Error(kerErrorMessage, "Uable to init libcurl.");
        }

        // The default block size for FTP is much larger than other protocols
        // the reason is that getDataByRange() in FTP always creates the new
        // connection, so we need the large block size to reduce the overhead of
        // creating the connection.
        if (blockSize_ == 0) {
            blockSize_ = protocol_ == pFtp ? 102400 : 1024;
        }

        std::string timeout = getEnv(envTIMEOUT);
        timeout_ = atol(timeout.c_str());
        if (timeout_ == 0) {
            throw Error(kerErrorMessage, "Timeout Environmental Variable must be a positive integer.");
        }
    }
#ifdef EXV_UNICODE_PATH
    CurlIo::CurlImpl::CurlImpl(const std::wstring& wurl, size_t blockSize)
        : Impl(wurl, blockSize)
    {
        std::string url;
        url.assign(wurl.begin(), wurl.end());
        path_ = url;

        // init curl pointer
        curl_ = curl_easy_init();
        if (!curl_) {
            throw Error(kerErrorMessage, "Uable to init libcurl.");
        }

        // The default block size for FTP is much larger than other protocols
        // the reason is that getDataByRange() in FTP always creates the new
        // connection, so we need the large block size to reduce the overhead of
        // creating the connection.
        if (blockSize_ == 0) {
            blockSize_ = protocol_ == pFtp ? 102400 : 1024;
        }
    }
#endif

    long CurlIo::CurlImpl::getFileLength()
    {
        curl_easy_reset(curl_);  // reset all options
        std::string response;
        curl_easy_setopt(curl_, CURLOPT_URL, path_.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOBODY, 1);  // HEAD
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curlWriter);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout_);
        // curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl_);
        if (res != CURLE_OK) {  // error happends
            throw Error(kerErrorMessage, curl_easy_strerror(res));
        }
        // get return code
        long returnCode;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE,
                          &returnCode);  // get code
        if (returnCode >= 400 || returnCode < 0) {
            throw Error(kerTiffDirectoryTooLarge, "Server", returnCode);
        }
        // get length
        double temp;
        curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                          &temp);  // return -1 if unknown
        return (long)temp;
    }

    void CurlIo::CurlImpl::getDataByRange(long lowBlock, long highBlock, std::string& response)
    {
        curl_easy_reset(curl_);  // reset all options
        curl_easy_setopt(curl_, CURLOPT_URL, path_.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS,
                         1L);  // no progress meter please
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curlWriter);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout_);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);

        // curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode

        if (lowBlock > -1 && highBlock > -1) {
            std::stringstream ss;
            ss << lowBlock * blockSize_ << "-" << ((highBlock + 1) * blockSize_ - 1);
            std::string range = ss.str();
            curl_easy_setopt(curl_, CURLOPT_RANGE, range.c_str());
        }

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl_);

        if (res != CURLE_OK) {
            throw Error(kerErrorMessage, curl_easy_strerror(res));
        } else {
            long serverCode;
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE,
                              &serverCode);  // get code
            if (serverCode >= 400 || serverCode < 0) {
                throw Error(kerTiffDirectoryTooLarge, "Server", serverCode);
            }
        }
    }

    void CurlIo::CurlImpl::writeRemote(const byte* data, size_t size, long from, long to)
    {
        std::string scriptPath(getEnv(envHTTPPOST));
        if (scriptPath == "") {
            throw Error(kerErrorMessage,
                        "Please set the path of the server script to handle "
                        "http post data to EXIV2_HTTP_POST "
                        "environmental variable.");
        }

        Exiv2::Uri hostInfo = Exiv2::Uri::Parse(path_);

        // add the protocol and host to the path
        std::size_t protocolIndex = scriptPath.find("://");
        if (protocolIndex == std::string::npos) {
            if (scriptPath[0] != '/')
                scriptPath = "/" + scriptPath;
            scriptPath = hostInfo.Protocol + "://" + hostInfo.Host + scriptPath;
        }

        curl_easy_reset(curl_);  // reset all options
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS,
                         1L);  // no progress meter please
        // curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode
        curl_easy_setopt(curl_, CURLOPT_URL, scriptPath.c_str());
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);

        // encode base64
        size_t encodeLength = ((size + 2) / 3) * 4 + 1;
        char* encodeData = new char[encodeLength];
        base64encode(data, size, encodeData, encodeLength);
        // url encode
        const std::string urlencodeData = urlencode(encodeData);
        delete[] encodeData;
        std::stringstream ss;
        ss << "path=" << hostInfo.Path << "&"
           << "from=" << from << "&"
           << "to=" << to << "&"
           << "data=" << urlencodeData;
        std::string postData = ss.str();

        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, postData.c_str());
        // Perform the request, res will get the return code.
        CURLcode res = curl_easy_perform(curl_);

        if (res != CURLE_OK) {
            throw Error(kerErrorMessage, curl_easy_strerror(res));
        } else {
            long serverCode;
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &serverCode);
            if (serverCode >= 400 || serverCode < 0) {
                throw Error(kerTiffDirectoryTooLarge, "Server", serverCode);
            }
        }
    }

    CurlIo::CurlImpl::~CurlImpl()
    {
        curl_easy_cleanup(curl_);
    }

    size_t CurlIo::write(const byte* data, size_t wcount)
    {
        if (p_->protocol_ == pHttp || p_->protocol_ == pHttps) {
            return RemoteIo::write(data, wcount);
        } else {
            throw Error(kerErrorMessage, "doesnt support write for this protocol.");
        }
    }

    size_t CurlIo::write(BasicIo& src)
    {
        if (p_->protocol_ == pHttp || p_->protocol_ == pHttps) {
            return RemoteIo::write(src);
        } else {
            throw Error(kerErrorMessage, "doesnt support write for this protocol.");
        }
    }

    CurlIo::CurlIo(const std::string& url, size_t blockSize)
    {
        p_ = new CurlImpl(url, blockSize);
    }
#ifdef EXV_UNICODE_PATH
    CurlIo::CurlIo(const std::wstring& wurl, size_t blockSize)
    {
        p_ = new CurlImpl(wurl, blockSize);
    }
#endif

    size_t curlWriter(char* data, size_t size, size_t nmemb, std::string* writerData)
    {
        if (writerData == nullptr)
            return 0;
        writerData->append(data, size * nmemb);
        return size * nmemb;
    }
#endif  // EXV_USE_CURL

}  // namespace Exiv2
