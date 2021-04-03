// ***************************************************************** -*- C++ -*-
// Test for large (>65535 bytes) IPTC buffer
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

#include <exiv2/exiv2.hpp>
#include <cassert>
#include <iostream>

int main(int argc, char* const argv[])
{
    try {
        Exiv2::XmpParser::initialize();
        ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
        Exiv2::enableBMFF();
#endif

        if (argc != 3) {
            std::cout << "Usage: " << argv[0] << " image datafile\n";
            return 1;
        }
        std::string file(argv[1]);
        std::string data(argv[2]);

        // Read data file into data buffer
        Exiv2::FileIo io(data);
        if (io.open() != 0) {
            throw Exiv2::Error(Exiv2::kerDataSourceOpenFailed, io.path(), Exiv2::strError());
        }
        Exiv2::DataBuf buf(static_cast<long>(io.size()));
        std::cout << "Reading " << buf.size_ << " bytes from " << data << "\n";
        long readBytes = io.read(buf.pData_, buf.size_);
        if (readBytes != buf.size_ || io.error() || io.eof()) {
            throw Exiv2::Error(Exiv2::kerFailedToReadImageData);
        }

        // Read metadata from file
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
        assert(image.get() != 0);
        image->readMetadata();

        // Set Preview field to the content of the data file
        Exiv2::DataValue value;
        value.read(buf.pData_, buf.size_);
        Exiv2::IptcData& iptcData = image->iptcData();
        std::cout << "IPTC fields: " << iptcData.size() << "\n";
        iptcData["Iptc.Application2.Preview"] = value;
        std::cout << "IPTC fields: " << iptcData.size() << "\n";

        // Set IRB, compare with IPTC raw data
        Exiv2::DataBuf irb = Exiv2::Photoshop::setIptcIrb(0, 0, iptcData);
        std::cout << "IRB buffer : " << irb.size_ << "\n";
        const Exiv2::byte* record;
        uint32_t sizeHdr;
        uint32_t sizeData;
        Exiv2::Photoshop::locateIptcIrb(irb.pData_, irb.size_, &record, &sizeHdr, &sizeData);
        Exiv2::DataBuf rawIptc = Exiv2::IptcParser::encode(iptcData);
        std::cout << "Comparing IPTC and IRB size... ";
        if (static_cast<uint32_t>(rawIptc.size_) != sizeData) {
            std::cout << "not ";
        }
        std::cout << "ok\n";

        std::cout << "Comparing IPTC and IRB data... ";
        if (0 != memcmp(rawIptc.pData_, record + sizeHdr, sizeData)) {
            std::cout << "not ";
        }
        std::cout << "ok\n";

        // Set Iptc data and write it to the file
        image->writeMetadata();

        return 0;
    } catch (Exiv2::AnyError& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        return -1;
    }
}
