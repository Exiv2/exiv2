// *****************************************************************************
// exiv2json.cpp - Modern JSON export for Exiv2
// Restored and modernized from 0.27-maintenance branch
// Uses nlohmann/json instead of deprecated Jzon library
// Compatible with Exiv2 0.28+ API
// *****************************************************************************

#include <exiv2/exiv2.hpp>
#include <exiv2/contrib/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

using json = nlohmann::json;

// Helper: Convert Exiv2 value to appropriate JSON type
json exiv2ToJson(const Exiv2::Value& value) {
    json result;
    
    switch (value.typeId()) {
        case Exiv2::unsignedByte:
        case Exiv2::unsignedShort:
        case Exiv2::unsignedLong:
        case Exiv2::signedByte:
        case Exiv2::signedShort:
        case Exiv2::signedLong:
            // FIX: Use toInt64() instead of toLong()
            result = value.toInt64();
            break;
            
        case Exiv2::unsignedRational:
        case Exiv2::signedRational: {
            // FIX: Array detection - rationals are always [num, den]
            Exiv2::Rational rat = value.toRational();
            result = json::array({rat.first, rat.second});
            break;
        }
        
        case Exiv2::xmpText:
        case Exiv2::asciiString: {
            std::string s = value.toString();
            result = s;
            break;
        }
        
        default:
            result = value.toString();
    }
    
    return result;
}

// NEW API: Export metadata to JSON
// FIX: Use std::unique_ptr instead of AutoPtr
json metadataToJson(std::unique_ptr<Exiv2::Image>& image) {
    json root;
    
    // Exif data
    json exifNode;
    Exiv2::ExifData& exifData = image->exifData();
    for (auto& md : exifData) {
        std::string key = md.key();
        json value = exiv2ToJson(md.value());
        exifNode[key] = value;
    }
    root["Exif"] = exifNode;
    
    // IPTC data
    json iptcNode;
    Exiv2::IptcData& iptcData = image->iptcData();
    for (auto& md : iptcData) {
        iptcNode[md.key()] = exiv2ToJson(md.value());
    }
    root["Iptc"] = iptcNode;
    
    // XMP data
    json xmpNode;
    Exiv2::XmpData& xmpData = image->xmpData();
    for (auto& md : xmpData) {
        xmpNode[md.key()] = exiv2ToJson(md.value());
    }
    root["Xmp"] = xmpNode;
    
    return root;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image>" << std::endl;
        return 1;
    }
    
    try {
        // FIX: Modern API - ImageFactory::open returns std::unique_ptr
        auto image = Exiv2::ImageFactory::open(argv[1]);
        image->readMetadata();
        
        json output = metadataToJson(image);
        std::cout << output.dump(4) << std::endl;
        
    } catch (Exiv2::Error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
