// ***************************************************************** -*- C++ -*-
// iptceasy.cpp, $Rev$
// The quickest way to access, set or modify Iptc metadata.

#include "iptc.hpp"
#include <iostream>
#include <iomanip>

void printIptc(const Exiv2::IptcData& iptcData);

int main()
try {

    Exiv2::IptcData iptcData;

    iptcData["Iptc.Application2.Headline"] = "The headline I am";
    iptcData["Iptc.Application2.Keywords"] = "Yet another keyword";
    iptcData["Iptc.Application2.DateCreated"] = "2004-8-3";
    iptcData["Iptc.Application2.Urgency"] = uint16_t(3);
    iptcData["Iptc.Application2.SuppCategory"] = "bla bla bla";
    iptcData["Iptc.Envelope.ModelVersion"] = 2;
    iptcData["Iptc.Envelope.TimeSent"] = "14:41:0-05:00";
    iptcData["Iptc.Application2.RasterizedCaption"] = "230 42 34 2 90 84 23 146";

    std::cout << "Time sent: " << iptcData["Iptc.Envelope.TimeSent"] << "\n\n";

    printIptc(iptcData);    

    return 0;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}

void printIptc(const Exiv2::IptcData& iptcData)
{
    Exiv2::IptcData::const_iterator end = iptcData.end();
    Exiv2::IptcData::const_iterator md;
    for (md = iptcData.begin(); md != end; ++md) {
        std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << md->tag() << " " 
                  << std::setw(12) << std::setfill(' ') << std::left
                  << md->recordName() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3) 
                  << std::setfill(' ') << std::right
                  << md->count() << " "
                  << std::setw(27) << std::setfill(' ') << std::left
                  << md->tagName() << " "
                  << std::dec << md->value() 
                  << std::endl;
    }
}
