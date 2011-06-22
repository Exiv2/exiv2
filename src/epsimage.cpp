// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2011 Andreas Huggel <ahuggel@gmx.net>
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
  File:      epsimage.cpp
  Version:   $Rev: 2455 $
  Author(s): Michael Ulbrich (mul) <mul@rentapacs.de>
             Volker Grabsch (vog) <vog@notjusthosting.com>
  History:   7-Mar-2011, vog: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: epsimage.cpp $")

// *****************************************************************************

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif
#include "epsimage.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "convert.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

// signature of DOS EPS
static const std::string epsDosSignature = "\xc5\xd0\xd3\xc6";

// first line of EPS
static const std::string epsFirstLine[] = {
    "%!PS-Adobe-3.0 EPSF-3.0",
    "%!PS-Adobe-3.0 EPSF-3.0 ", // OpenOffice
    "%!PS-Adobe-3.1 EPSF-3.0",  // Illustrator
};

// blank EPS file
static const std::string epsBlank = "%!PS-Adobe-3.0 EPSF-3.0\n"
                                    "%%BoundingBox: 0 0 0 0\n";

// list of all valid XMP headers
static const struct { std::string header; std::string charset; } xmpHeadersDef[] = {

    // We do not enforce the trailing "?>" here, because the XMP specification
    // permits additional attributes after begin="..." and id="...".

    // normal headers
    {"<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-8"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-8"},
    {"<?xpacket begin='\xef\xbb\xbf' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-8"},
    {"<?xpacket begin='\xef\xbb\xbf' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-8"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-16BE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-16BE"},
    {"<?xpacket begin='\xef\xbb\xbf' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-16BE"},
    {"<?xpacket begin='\xef\xbb\xbf' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-16BE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-16LE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-16LE"},
    {"<?xpacket begin='\xef\xbb\xbf' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-16LE"},
    {"<?xpacket begin='\xef\xbb\xbf' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-16LE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-32BE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-32BE"},
    {"<?xpacket begin='\xef\xbb\xbf' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-32BE"},
    {"<?xpacket begin='\xef\xbb\xbf' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-32BE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-32LE"},
    {"<?xpacket begin=\"\xef\xbb\xbf\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-32LE"},
    {"<?xpacket begin='\xef\xbb\xbf' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-32LE"},
    {"<?xpacket begin='\xef\xbb\xbf' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-32LE"},

    // deprecated headers (empty begin attribute, UTF-8 only)
    {"<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"", "UTF-8"},
    {"<?xpacket begin=\"\" id='W5M0MpCehiHzreSzNTczkc9d'",   "UTF-8"},
    {"<?xpacket begin='' id=\"W5M0MpCehiHzreSzNTczkc9d\"",   "UTF-8"},
    {"<?xpacket begin='' id='W5M0MpCehiHzreSzNTczkc9d'",     "UTF-8"},
};

// list of all valid XMP trailers
static const struct { std::string trailer; bool readOnly; } xmpTrailersDef[] = {

    // We do not enforce the trailing "?>" here, because the XMP specification
    // permits additional attributes after end="...".

    {"<?xpacket end=\"r\"", true},
    {"<?xpacket end='r'",   true},
    {"<?xpacket end=\"w\"", false},
    {"<?xpacket end='w'",   false},
};

// closing part of all valid XMP trailers
static const std::string xmpTrailerEndDef = "?>";

// *****************************************************************************
// class member definitions
namespace Exiv2
{

    EpsImage::EpsImage(BasicIo::AutoPtr io, bool create)
            : Image(ImageType::eps, mdXmp, io)
    {
        //LogMsg::setLevel(LogMsg::debug);
        if (create) {
            if (io_->open() == 0) {
                #ifdef DEBUG
                EXV_DEBUG << "Exiv2::EpsImage:: Creating blank EPS image\n";
                #endif
                IoCloser closer(*io_);
                if (io_->write(reinterpret_cast<const byte*>(epsBlank.data()), static_cast<long>(epsBlank.size())) != static_cast<long>(epsBlank.size())) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to write blank EPS image.\n";
                    #endif
                    throw Error(21);
                }
            }
        }
    }

    std::string EpsImage::mimeType() const
    {
        return "application/postscript";
    }

    void EpsImage::setComment(const std::string& /*comment*/)
    {
        throw Error(32, "Image comment", "EPS");
    }

    void EpsImage::readMetadata()
    {
        doReadWriteMetadata(/* write = */ false);
    }

    void EpsImage::writeMetadata()
    {
        doReadWriteMetadata(/* write = */ true);
    }

    void EpsImage::doReadWriteMetadata(bool write)
    {
        #ifdef DEBUG
        if (write) {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Writing EPS file " << io_->path() << "\n";
        } else {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Reading EPS file " << io_->path() << "\n";
        }
        #endif

        // open input file
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);

        // read from input file via memory map
        const char *data = reinterpret_cast<const char*>(io_->mmap());
        const size_t size = io_->size();
        size_t pos = 0;
        std::string line;

        // TODO: Add support for DOS EPS (C5 D0 D3 C6)

        // check first line
        const size_t firstLinePos = pos;
        pos = readLine(line, data, firstLinePos, size);
        const std::string firstLine = line;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: First line: " << firstLine << "\n";
        #endif
        bool matched = false;
        for (size_t i = 0; !matched && i < (sizeof epsFirstLine) / (sizeof *epsFirstLine); i++) {
            matched = (firstLine == epsFirstLine[i]);
        }
        if (!matched) {
            throw Error(3, "EPS");
        }

        // determine line ending style of the first line
        if (pos >= size) {
            #ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Premature end of file after first line.\n";
            #endif
            throw Error(write ? 21 : 14);
        }
        const std::string lineEnding(data + firstLinePos + firstLine.size(), pos - (firstLinePos + firstLine.size()));
        #ifdef DEBUG
        if (lineEnding == "\n") {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Line ending style: Unix (LF)\n";
        } else if (lineEnding == "\r") {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Line ending style: Mac (CR)\n";
        } else if (lineEnding == "\r\n") {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Line ending style: DOS (CR LF)\n";
        } else {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Line ending style: (unknown)\n";
        }
        #endif

        // scan comments
        size_t posLanguageLevel = size;
        size_t posContainsXmp = size;
        size_t posPages = size;
        size_t posExiv2Version = size;
        size_t posExiv2Website = size;
        size_t posEndComments = size;
        size_t posPage = size;
        size_t posEndPageSetup = size;
        size_t posPageTrailer = size;
        size_t posEof = size;
        bool implicitPage = false;
        bool photoshop = false;
        bool inDefaultsOrPrologOrSetup = false;
        bool inPageSetup = false;
        while (pos < posEof) {
            const size_t startPos = pos;
            pos = readLine(line, data, startPos, size);
            // implicit comments
            if (line == "%%EOF" || line == "%begin_xml_code" || !(line.size() >= 2 && line[0] == '%' && '\x21' <= line[1] && line[1] <= '\x7e')) {
                if (posEndComments == size) {
                    posEndComments = startPos;
                    #ifdef DEBUG
                    EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found implicit EndComments at position: " << startPos << "\n";
                    #endif
                }
            }
            if (line == "%%EOF" || line == "%begin_xml_code" || (line.size() >= 1 && line[0] != '%')) {
                if (posPage == size && posEndComments != size && !inDefaultsOrPrologOrSetup && !onlyWhitespaces(line)) {
                    posPage = startPos;
                    implicitPage = true;
                    #ifdef DEBUG
                    EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found implicit Page at position: " << startPos << "\n";
                    #endif
                }
                if (posEndPageSetup == size && posPage != size && !inPageSetup) {
                    posEndPageSetup = startPos;
                    #ifdef DEBUG
                    EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found implicit EndPageSetup at position: " << startPos << "\n";
                    #endif
                }
            }
            if (line.size() >= 1 && line[0] != '%') continue; // performance optimization
            if (line == "%%EOF" && posPageTrailer == size) {
                posPageTrailer = startPos;
                #ifdef DEBUG
                EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found implicit PageTrailer at position: " << startPos << "\n";
                #endif
            }
            // explicit comments
            #ifdef DEBUG
            bool significantLine = true;
            #endif
            if (posEndComments == size && posLanguageLevel == size && startsWith(line, "%%LanguageLevel:")) {
                posLanguageLevel = startPos;
            } else if (posEndComments == size && posContainsXmp == size && startsWith(line, "%ADO_ContainsXMP:")) {
                posContainsXmp = startPos;
            } else if (posEndComments == size && posPages == size && startsWith(line, "%%Pages:")) {
                posPages = startPos;
            } else if (posEndComments == size && posExiv2Version == size && startsWith(line, "%Exiv2Version:")) {
                posExiv2Version = startPos;
            } else if (posEndComments == size && posExiv2Website == size && startsWith(line, "%Exiv2Website:")) {
                posExiv2Website = startPos;
            } else if (posEndComments == size && line == "%%EndComments") {
                posEndComments = startPos;
            } else if (line == "%%BeginDefaults") {
                inDefaultsOrPrologOrSetup = true;
            } else if (line == "%%EndDefaults") {
                inDefaultsOrPrologOrSetup = false;
            } else if (line == "%%BeginProlog") {
                inDefaultsOrPrologOrSetup = true;
            } else if (line == "%%EndProlog") {
                inDefaultsOrPrologOrSetup = false;
            } else if (line == "%%BeginSetup") {
                inDefaultsOrPrologOrSetup = true;
            } else if (line == "%%EndSetup") {
                inDefaultsOrPrologOrSetup = false;
            } else if (posPage == size && startsWith(line, "%%Page:")) {
                posPage = startPos;
            } else if (line == "%%BeginPageSetup") {
                inPageSetup = true;
            } else if (posEndPageSetup == size && line == "%%EndPageSetup") {
                inPageSetup = false;
                posEndPageSetup = startPos;
            } else if (posPageTrailer == size && line == "%%PageTrailer") {
                posPageTrailer = startPos;
            } else if (startsWith(line, "%BeginPhotoshop:")) {
                photoshop = true;
            } else if (line == "%%EOF") {
                posEof = startPos;
            } else if (startsWith(line, "%%BeginDocument:")) {
                if (posEndPageSetup == size) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Embedded document at invalid position (before explicit or implicit EndPageSetup): " << startPos << "\n";
                    #endif
                    throw Error(write ? 21 : 14);
                }
                // TODO: Add support for embedded documents!
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Embedded documents are currently not supported. Found embedded document at position: " << startPos << "\n";
                #endif
                throw Error(write ? 21 : 14);
            } else if (posPage != size && startsWith(line, "%%Page:")) {
                if (implicitPage) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Page at position " << startPos << " conflicts with implicit page at position: " << posPage << "\n";
                    #endif
                    throw Error(write ? 21 : 14);
                }
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Unable to handle multiple PostScript pages. Found second page at position: " << startPos << "\n";
                #endif
                throw Error(write ? 21 : 14);
            } else if (startsWith(line, "%%Include")) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Unable to handle PostScript %%Include DSC comments yet. Please provide your"
                               " sample EPS file to the Exiv2 project: http://dev.exiv2.org/projects/exiv2\n";
                #endif
                throw Error(write ? 21 : 14);
            } else {
                #ifdef DEBUG
                significantLine = false;
                #endif
            }
            #ifdef DEBUG
            if (significantLine) {
                EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found significant line \"" << line << "\" at position: " << startPos << "\n";
            }
            #endif
        }

        // interpret comment "%ADO_ContainsXMP:"
        readLine(line, data, posContainsXmp, size);
        bool containsXmp;
        if (line == "%ADO_ContainsXMP: MainFirst" || line == "%ADO_ContainsXMP:MainFirst") {
            containsXmp = true;
        } else if (line == "" || line == "%ADO_ContainsXMP: NoMain" || line == "%ADO_ContainsXMP:NoMain") {
            containsXmp = false;
        } else {
            #ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Invalid line \"" << line << "\" at position: " << posContainsXmp << "\n";
            #endif
            throw Error(write ? 21 : 14);
        }

        std::vector<std::pair<size_t, size_t> > removableEmbeddings;
        bool fixBeginXmlPacket = false;
        size_t xmpPos = size;
        size_t xmpSize = 0;
        if (containsXmp) {
            // search for XMP metadata
            findXmp(xmpPos, xmpSize, data, size, write);
            if (xmpSize == 0) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Unable to find XMP metadata as announced at position: " << posContainsXmp << "\n";
                #endif
                throw Error(write ? 21 : 14);
            }
            // check embedding of XMP metadata
            const size_t posLineAfterXmp = readLine(line, data, xmpPos + xmpSize, size);
            if (line != "") {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Unexpected " << line.size() << " bytes of data after XMP at position: " << (xmpPos + xmpSize) << "\n";
                #endif
                if (write) throw Error(21);
            }
            readLine(line, data, posLineAfterXmp, size);
            if (line == "% &&end XMP packet marker&&" || line == "%  &&end XMP packet marker&&") {
                #ifdef DEBUG
                EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Recognized flexible XMP embedding\n";
                #endif
                const size_t posBeginXmlPacket = readPrevLine(line, data, xmpPos, size);
                if (startsWith(line, "%begin_xml_packet:")) {
                    #ifdef DEBUG
                    EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Found %begin_xml_packet before flexible XMP embedding\n";
                    #endif
                    if (write) {
                        fixBeginXmlPacket = true;
                        xmpSize += (xmpPos - posBeginXmlPacket);
                        xmpPos = posBeginXmlPacket;
                    }
                } else if (photoshop) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Missing %begin_xml_packet in Photoshop EPS at position: " << xmpPos << "\n";
                    #endif
                    if (write) throw Error(21);
                }
            } else {
                removableEmbeddings = findRemovableEmbeddings(data, posEof, posEndPageSetup, xmpPos, xmpSize, write);
                if (removableEmbeddings.empty()) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Unknown XMP embedding at position: " << xmpPos << "\n";
                    #endif
                    if (write) throw Error(21);
                }
            }
        }

        if (!write) {
            // copy and decode XMP metadata
            xmpPacket_.assign(data + xmpPos, xmpSize);
            if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_) > 1) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Failed to decode XMP metadata.\n";
                #endif
                throw Error(14);
            }
        } else {
            const bool useExistingEmbedding = (xmpPos != size && removableEmbeddings.empty());

            // encode XMP metadata if necessary
            if (!writeXmpFromPacket() && XmpParser::encode(xmpPacket_, xmpData_) > 1) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Failed to encode XMP metadata.\n";
                #endif
                throw Error(21);
            }

            // TODO: Add support for deleting XMP metadata. Note that this is not
            //       as simple as it may seem, and requires special attention!
            if (xmpPacket_.size() == 0) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Deleting XMP metadata is currently not supported.\n";
                #endif
                throw Error(21);
            }

            // create temporary output file
            BasicIo::AutoPtr tempIo(io_->temporary());
            assert (tempIo.get() != 0);
            if (!tempIo->isopen()) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Unable to create temporary file for writing.\n";
                #endif
                throw Error(21);
            }
            #ifdef DEBUG
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Created temporary file " << tempIo->path() << "\n";
            #endif

            // sort all positions
            std::vector<size_t> positions;
            positions.push_back(posLanguageLevel);
            positions.push_back(posContainsXmp);
            positions.push_back(posPages);
            positions.push_back(posExiv2Version);
            positions.push_back(posExiv2Website);
            positions.push_back(posEndComments);
            positions.push_back(posPage);
            positions.push_back(posEndPageSetup);
            positions.push_back(posPageTrailer);
            positions.push_back(posEof);
            positions.push_back(size);
            if (useExistingEmbedding) {
                positions.push_back(xmpPos);
            }
            for (std::vector<std::pair<size_t, size_t> >::const_iterator e = removableEmbeddings.begin(); e != removableEmbeddings.end(); e++) {
                positions.push_back(e->first);
            }
            std::sort(positions.begin(), positions.end());

            // assemble result EPS document
            size_t prevPos = 0;
            size_t prevSkipPos = 0;
            for (std::vector<size_t>::const_iterator i = positions.begin(); i != positions.end(); i++) {
                const size_t pos = *i;
                if (pos == prevPos) continue;
                if (pos < prevSkipPos) {
                    #ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Internal error while assembling the result EPS document: "
                                   "Unable to continue at position " << pos << " after skipping to position " << prevSkipPos << "\n";
                    #endif
                    throw Error(21);
                }
                writeTemp(*tempIo, data + prevSkipPos, pos - prevSkipPos);
                const size_t posLineEnd = readLine(line, data, pos, size);
                size_t skipPos = pos;
                // add last line ending if necessary
                if (pos == size && pos >= 1 && data[pos - 1] != '\r' && data[pos - 1] != '\n') {
                    writeTemp(*tempIo, lineEnding);
                    #ifdef DEBUG
                    EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Added missing line ending of last line\n";
                    #endif
                }
                // update and complement DSC comments
                if (pos == posLanguageLevel && posLanguageLevel != size && !useExistingEmbedding) {
                    if (line == "%%LanguageLevel:1" || line == "%%LanguageLevel: 1") {
                        writeTemp(*tempIo, "%%LanguageLevel: 2" + lineEnding);
                        skipPos = posLineEnd;
                    }
                }
                if (pos == posContainsXmp && posContainsXmp != size) {
                    if (line != "%ADO_ContainsXMP: MainFirst") {
                        writeTemp(*tempIo, "%ADO_ContainsXMP: MainFirst" + lineEnding);
                        skipPos = posLineEnd;
                    }
                }
                if (pos == posExiv2Version && posExiv2Version != size) {
                    writeTemp(*tempIo, "%Exiv2Version: " + std::string(version()) + lineEnding);
                    skipPos = posLineEnd;
                }
                if (pos == posExiv2Website && posExiv2Website != size) {
                    writeTemp(*tempIo, "%Exiv2Website: http://www.exiv2.org/" + lineEnding);
                    skipPos = posLineEnd;
                }
                if (pos == posEndComments) {
                    if (posLanguageLevel == size && !useExistingEmbedding) {
                        writeTemp(*tempIo, "%%LanguageLevel: 2" + lineEnding);
                    }
                    if (posContainsXmp == size) {
                        writeTemp(*tempIo, "%ADO_ContainsXMP: MainFirst" + lineEnding);
                    }
                    if (posPages == size) {
                        writeTemp(*tempIo, "%%Pages: 1" + lineEnding);
                    }
                    if (posExiv2Version == size) {
                        writeTemp(*tempIo, "%Exiv2Version: " + std::string(version()) + lineEnding);
                    }
                    if (posExiv2Website == size) {
                        writeTemp(*tempIo, "%Exiv2Website: http://www.exiv2.org/" + lineEnding);
                    }
                    readLine(line, data, posEndComments, size);
                    if (line != "%%EndComments") {
                        writeTemp(*tempIo, "%%EndComments" + lineEnding);
                    }
                }
                if (pos == posPage) {
                    if (!startsWith(line, "%%Page:")) {
                        writeTemp(*tempIo, "%%Page: 1 1" + lineEnding);
                        writeTemp(*tempIo, "%%EndPageComments" + lineEnding);
                    }
                }
                // remove unflexible embeddings
                for (std::vector<std::pair<size_t, size_t> >::const_iterator e = removableEmbeddings.begin(); e != removableEmbeddings.end(); e++) {
                    if (pos == e->first) {
                        skipPos = e->second;
                        break;
                    }
                }
                if (useExistingEmbedding) {
                    // insert XMP metadata into existing flexible embedding
                    if (pos == xmpPos) {
                        if (fixBeginXmlPacket) {
                            writeTemp(*tempIo, "%begin_xml_packet: " + toString(xmpPacket_.size()) + lineEnding);
                        }
                        writeTemp(*tempIo, xmpPacket_.data(), xmpPacket_.size());
                        skipPos += xmpSize;
                    }
                } else {
                    // insert XMP metadata with new flexible embedding
                    if (pos == posEndPageSetup) {
                        if (line != "%%EndPageSetup") {
                            writeTemp(*tempIo, "%%BeginPageSetup" + lineEnding);
                        }
                        writeTemp(*tempIo, "%Exiv2BeginXMP: Before %%EndPageSetup" + lineEnding);
                        if (photoshop) {
                            writeTemp(*tempIo, "%Exiv2Notice: The following line is needed by Photoshop." + lineEnding);
                            writeTemp(*tempIo, "%begin_xml_code" + lineEnding);
                        }
                        writeTemp(*tempIo, "/currentdistillerparams where" + lineEnding);
                        writeTemp(*tempIo, "{pop currentdistillerparams /CoreDistVersion get 5000 lt} {true} ifelse" + lineEnding);
                        writeTemp(*tempIo, "{userdict /Exiv2_pdfmark /cleartomark load put" + lineEnding);
                        writeTemp(*tempIo, "    userdict /Exiv2_metafile_pdfmark {flushfile cleartomark} bind put}" + lineEnding);
                        writeTemp(*tempIo, "{userdict /Exiv2_pdfmark /pdfmark load put" + lineEnding);
                        writeTemp(*tempIo, "    userdict /Exiv2_metafile_pdfmark {/PUT pdfmark} bind put} ifelse" + lineEnding);
                        writeTemp(*tempIo, "[/NamespacePush Exiv2_pdfmark" + lineEnding);
                        writeTemp(*tempIo, "[/_objdef {Exiv2_metadata_stream} /type /stream /OBJ Exiv2_pdfmark" + lineEnding);
                        writeTemp(*tempIo, "[{Exiv2_metadata_stream} 2 dict begin" + lineEnding);
                        writeTemp(*tempIo, "    /Type /Metadata def /Subtype /XML def currentdict end /PUT Exiv2_pdfmark" + lineEnding);
                        writeTemp(*tempIo, "[{Exiv2_metadata_stream}" + lineEnding);
                        writeTemp(*tempIo, "    currentfile 0 (% &&end XMP packet marker&&)" + lineEnding);
                        writeTemp(*tempIo, "    /SubFileDecode filter Exiv2_metafile_pdfmark" + lineEnding);
                        if (photoshop) {
                            writeTemp(*tempIo, "%Exiv2Notice: The following line is needed by Photoshop. "
                                               "Parameter must be exact size of XMP metadata." + lineEnding);
                            writeTemp(*tempIo, "%begin_xml_packet: " + toString(xmpPacket_.size()) + lineEnding);
                        }
                        writeTemp(*tempIo, xmpPacket_.data(), xmpPacket_.size());
                        writeTemp(*tempIo, lineEnding);
                        writeTemp(*tempIo, "% &&end XMP packet marker&&" + lineEnding);
                        writeTemp(*tempIo, "[/Document 1 dict begin" + lineEnding);
                        writeTemp(*tempIo, "    /Metadata {Exiv2_metadata_stream} def currentdict end /BDC Exiv2_pdfmark" + lineEnding);
                        if (photoshop) {
                            writeTemp(*tempIo, "%Exiv2Notice: The following line is needed by Photoshop." + lineEnding);
                            writeTemp(*tempIo, "%end_xml_code" + lineEnding);
                        }
                        writeTemp(*tempIo, "%Exiv2EndXMP" + lineEnding);
                        if (line != "%%EndPageSetup") {
                            writeTemp(*tempIo, "%%EndPageSetup" + lineEnding);
                        }
                    }
                    if (pos == posPageTrailer) {
                        if (pos == size || pos == posEof) {
                            writeTemp(*tempIo, "%%PageTrailer" + lineEnding);
                        } else {
                            skipPos = posLineEnd;
                        }
                        writeTemp(*tempIo, "%Exiv2BeginXMP: After %%PageTrailer" + lineEnding);
                        writeTemp(*tempIo, "[/EMC Exiv2_pdfmark" + lineEnding);
                        writeTemp(*tempIo, "[/NamespacePop Exiv2_pdfmark" + lineEnding);
                        writeTemp(*tempIo, "%Exiv2EndXMP" + lineEnding);
                    }
                }
                // add EOF comment if necessary
                if (pos == size && posEof == size) {
                    writeTemp(*tempIo, "%%EOF" + lineEnding);
                }
                prevPos = pos;
                prevSkipPos = skipPos;
            }

            // copy temporary file to real output file
            io_->close();
            io_->transfer(*tempIo);
        }

        #ifdef DEBUG
        if (write) {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Finished writing EPS file " << io_->path() << "\n";
        } else {
            EXV_DEBUG << "Exiv2::EpsImage::doReadWriteMetadata: Finished reading EPS file " << io_->path() << "\n";
        }
        #endif
    }

    std::vector<std::pair<size_t, size_t> > EpsImage::findRemovableEmbeddings(const char* data, size_t posEof, size_t posEndPageSetup,
                                                                              size_t xmpPos, size_t xmpSize, bool write)
    {
        std::vector<std::pair<size_t, size_t> > removableEmbeddings;
        std::string line;
        size_t pos;

        // check after XMP
        pos = xmpPos + xmpSize;
        pos = readLine(line, data, pos, posEof);
        if (line != "") return removableEmbeddings;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found empty line after XMP\n";
        #endif
        pos = readLine(line, data, pos, posEof);
        if (line != "%end_xml_packet") return removableEmbeddings;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found %end_xml_packet\n";
        #endif
        size_t posEmbeddingEnd = 0;
        for (int i = 0; i < 32; i++) {
            pos = readLine(line, data, pos, posEof);
            if (line == "%end_xml_code") {
                posEmbeddingEnd = pos;
                break;
            }
        }
        if (posEmbeddingEnd == 0) return removableEmbeddings;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found %end_xml_code\n";
        #endif

        // check before XMP
        pos = xmpPos;
        pos = readPrevLine(line, data, pos, posEof);
        if (!startsWith(line, "%begin_xml_packet: ")) return removableEmbeddings;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found %begin_xml_packet: ...\n";
        #endif
        size_t posEmbeddingStart = posEof;
        for (int i = 0; i < 32; i++) {
            pos = readPrevLine(line, data, pos, posEof);
            if (line == "%begin_xml_code") {
                posEmbeddingStart = pos;
                break;
            }
        }
        if (posEmbeddingStart == posEof) return removableEmbeddings;
        #ifdef DEBUG
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found %begin_xml_code\n";
        #endif

        // check at EOF
        pos = posEof;
        pos = readPrevLine(line, data, pos, posEof);
        if (line == "[/EMC pdfmark") {
            // Exiftool style
            #ifdef DEBUG
            EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found [/EMC pdfmark\n";
            #endif
        } else if (line == "[/NamespacePop pdfmark") {
            // Photoshop style
            #ifdef DEBUG
            EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found /NamespacePop pdfmark\n";
            #endif
            pos = readPrevLine(line, data, pos, posEof);
            if (line != "[{nextImage} 1 dict begin /Metadata {photoshop_metadata_stream} def currentdict end /PUT pdfmark") return removableEmbeddings;
            #ifdef DEBUG
            EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Found /PUT pdfmark\n";
            #endif
        } else {
           return removableEmbeddings;
        }

        // check whether another XMP metadata block would take precedence if this one was removed
        {
            size_t xmpPos, xmpSize;
            findXmp(xmpPos, xmpSize, data, posEndPageSetup, write);
            if (xmpSize != 0) {
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Second XMP metadata block interferes at position: " << xmpPos << "\n";
                #endif
                if (write) throw Error(21);
            }
        }

        removableEmbeddings.push_back(std::make_pair(posEmbeddingStart, posEmbeddingEnd));
        removableEmbeddings.push_back(std::make_pair(pos, posEof));
        #ifdef DEBUG
        const size_t n = removableEmbeddings.size();
        EXV_DEBUG << "Exiv2::EpsImage::findRemovableEmbeddings: Recognized Photoshop-style XMP embedding at "
                     "[" << removableEmbeddings[n-2].first << "," << removableEmbeddings[n-2].second << ")"
                     " with trailer "
                     "[" << removableEmbeddings[n-1].first << "," << removableEmbeddings[n-1].second << ")"
                     "\n";
        #endif
        return removableEmbeddings;
    }

    void EpsImage::findXmp(size_t& xmpPos, size_t& xmpSize, const char* data, size_t size, bool write)
    {
        // prepare list of valid XMP headers
        std::vector<std::pair<std::string, std::string> > xmpHeaders;
        for (size_t i = 0; i < (sizeof xmpHeadersDef) / (sizeof *xmpHeadersDef); i++) {
            const std::string &charset = xmpHeadersDef[i].charset;
            std::string header(xmpHeadersDef[i].header);
            if (!convertStringCharset(header, "UTF-8", charset.c_str())) {
                throw Error(28, charset);
            }
            xmpHeaders.push_back(make_pair(header, charset));
        }

        // search for valid XMP header
        xmpSize = 0;
        for (xmpPos = 0; xmpPos < size; xmpPos++) {
            if (data[xmpPos] != '\x00' && data[xmpPos] != '<') continue;
            for (size_t i = 0; i < xmpHeaders.size(); i++) {
                const std::string &header = xmpHeaders[i].first;
                if (xmpPos + header.size() > size) continue;
                if (memcmp(data + xmpPos, header.data(), header.size()) != 0) continue;
                #ifdef DEBUG
                EXV_DEBUG << "Exiv2::EpsImage::findXmp: Found XMP header at position: " << xmpPos << "\n";
                #endif

                // prepare list of valid XMP trailers in the charset of the header
                const std::string &charset = xmpHeaders[i].second;
                std::vector<std::pair<std::string, bool> > xmpTrailers;
                for (size_t j = 0; j < (sizeof xmpTrailersDef) / (sizeof *xmpTrailersDef); j++) {
                    std::string trailer(xmpTrailersDef[j].trailer);
                    if (!convertStringCharset(trailer, "UTF-8", charset.c_str())) {
                        throw Error(28, charset);
                    }
                    xmpTrailers.push_back(make_pair(trailer, xmpTrailersDef[j].readOnly));
                }
                std::string xmpTrailerEnd(xmpTrailerEndDef);
                if (!convertStringCharset(xmpTrailerEnd, "UTF-8", charset.c_str())) {
                    throw Error(28, charset);
                }

                // search for valid XMP trailer
                for (size_t trailerPos = xmpPos + header.size(); trailerPos < size; trailerPos++) {
                    if (data[xmpPos] != '\x00' && data[xmpPos] != '<') continue;
                    for (size_t j = 0; j < xmpTrailers.size(); j++) {
                        const std::string &trailer = xmpTrailers[j].first;
                        if (trailerPos + trailer.size() > size) continue;
                        if (memcmp(data + trailerPos, trailer.data(), trailer.size()) != 0) continue;
                        #ifdef DEBUG
                        EXV_DEBUG << "Exiv2::EpsImage::findXmp: Found XMP trailer at position: " << trailerPos << "\n";
                        #endif

                        const bool readOnly = xmpTrailers[j].second;
                        if (readOnly) {
                            #ifndef SUPPRESS_WARNINGS
                            EXV_WARNING << "Unable to handle read-only XMP metadata yet. Please provide your"
                                           " sample EPS file to the Exiv2 project: http://dev.exiv2.org/projects/exiv2\n";
                            #endif
                            throw Error(write ? 21 : 14);
                        }

                        // search for end of XMP trailer
                        for (size_t trailerEndPos = trailerPos + trailer.size(); trailerEndPos + xmpTrailerEnd.size() <= size; trailerEndPos++) {
                            if (memcmp(data + trailerEndPos, xmpTrailerEnd.data(), xmpTrailerEnd.size()) == 0) {
                                xmpSize = (trailerEndPos + xmpTrailerEnd.size()) - xmpPos;
                                return;
                            }
                        }
                        #ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Found XMP header but incomplete XMP trailer.\n";
                        #endif
                        throw Error(write ? 21 : 14);
                    }
                }
                #ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Found XMP header but no XMP trailer.\n";
                #endif
                throw Error(write ? 21 : 14);
            }
        }
    }

    size_t EpsImage::readLine(std::string& line, const char* data, size_t startPos, size_t size)
    {
        line.clear();
        size_t pos = startPos;
        // step through line
        while (pos < size && data[pos] != '\r' && data[pos] != '\n') {
            line += data[pos];
            pos++;
        }
        // skip line ending, if present
        if (pos >= size) return pos;
        pos++;
        if (pos >= size) return pos;
        if (data[pos - 1] == '\r' && data[pos] == '\n') pos++;
        return pos;
    }

    size_t EpsImage::readPrevLine(std::string& line, const char* data, size_t startPos, size_t size)
    {
        line.clear();
        size_t pos = startPos;
        if (pos > size) return pos;
        // skip line ending of previous line, if present
        if (pos <= 0) return pos;
        if (data[pos - 1] == '\r' || data[pos - 1] == '\n') {
            pos--;
            if (pos <= 0) return pos;
            if (data[pos - 1] == '\r' && data[pos] == '\n') {
                pos--;
                if (pos <= 0) return pos;
            }
        }
        // step through previous line
        while (pos >= 1 && data[pos - 1] != '\r' && data[pos - 1] != '\n') {
            pos--;
            line += data[pos];
        }
        std::reverse(line.begin(), line.end());
        return pos;
    }

    bool EpsImage::startsWith(const std::string& s, const std::string& start)
    {
        return s.size() >= start.size() && memcmp(s.data(), start.data(), start.size()) == 0;
    }

    bool EpsImage::onlyWhitespaces(const std::string& s)
    {
        // According to the DSC 3.0 specification, 4.4 Parsing Rules,
        // only spaces and tabs are considered to be white space characters.
        return s.find_first_not_of(" \t") == std::string::npos;
    }

    std::string EpsImage::toString(size_t size)
    {
        std::ostringstream stream;
        stream << size;
        return stream.str();
    }

    void EpsImage::writeTemp(BasicIo& tempIo, const char* data, size_t size)
    {
        if (size == 0) return;
        if (tempIo.write(reinterpret_cast<const byte*>(data), static_cast<long>(size)) != static_cast<long>(size)) {
            #ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Failed to write to temporary file.\n";
            #endif
            throw Error(21);
        }
    }

    void EpsImage::writeTemp(BasicIo& tempIo, const std::string &data)
    {
        writeTemp(tempIo, data.data(), data.size());
    }

    // *************************************************************************
    // free functions
    Image::AutoPtr newEpsInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new EpsImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isEpsType(BasicIo& iIo, bool advance)
    {
        // read as many bytes as needed for the longest (DOS) EPS signature
        size_t bufSize = epsDosSignature.size();
        for (size_t i = 0; i < (sizeof epsFirstLine) / (sizeof *epsFirstLine); i++) {
            if (bufSize < epsFirstLine[i].size()) {
                bufSize = epsFirstLine[i].size();
            }
        }
        byte buf[bufSize];
        iIo.read(buf, bufSize);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        // check for all possible (DOS) EPS signatures
        bool matched = (memcmp(buf, epsDosSignature.data(), epsDosSignature.size()) == 0);
        for (size_t i = 0; !matched && i < (sizeof epsFirstLine) / (sizeof *epsFirstLine); i++) {
            matched = (memcmp(buf, epsFirstLine[i].data(), epsFirstLine[i].size()) == 0);
        }
        // seek back if possible and requested
        if (!advance || !matched) {
            iIo.seek(-bufSize, BasicIo::cur);
        }
        return matched;
    }
} // namespace Exiv2
