// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program test the Iptc reading and writing.
             This is not designed to be a robust application.

  File     : iptctest.cpp
  Version  : $Name:  $ $Revision: 1.3 $
  Author(s): Brad Schick (brad) <schick@robotbattle.com>
  History  : 01-Aug-04, brad: created
 */
// *****************************************************************************
// included header files
#include "iptc.hpp"
#include "datasets.hpp"
#include "value.hpp"
#include <iostream>
#include <iomanip>

using namespace Exiv2;

bool processLine(const std::string& line, int num);
void processAdd(const std::string& line, int num);
void processRemove(const std::string& line, int num);
void processModify(const std::string& line, int num);

IptcData g_iptcData;

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
    try {

        if (argc != 2) {
            std::cout << "Usage: " << argv[0] << " image\n";
            std::cout << "Commands read from stdin.\n";
            return 1;
        }

        int rc = g_iptcData.read(argv[1]);
        if (rc) {
            std::string error = IptcData::strError(rc, argv[1]);
            throw Error(error);
        }

        std::string line;
        int num = 0;
        std::getline(std::cin, line);
        while (line.length() && processLine(line, ++num)) {
            std::getline(std::cin, line);
        }
     
        rc = g_iptcData.write(argv[1]);
        if (rc) {
            std::string error = IptcData::strError(rc, argv[1]);
            throw Error(error);
        }

        return rc;
    }
    catch (Error& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        return -1;
    }
}

bool processLine(const std::string& line, int num )
{
    switch (line.at(0)) {
        case 'a':
        case 'A':
            processAdd(line, num);
            break;
        case 'r':
        case 'R':
            processRemove(line, num);
            break;
        case 'm':
        case 'M':
            processModify(line, num);
            break;
        case 'q':
        case 'Q':
            return false;
        default:
            std::ostringstream os;
            os << "Unknown command (" << line.at(0) << ") at line " << num;
            throw Error(os.str());
    }
    return true;
}

void processAdd(const std::string& line, int num)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
    std::string::size_type keyEnd = line.find_first_of(" \t", keyStart+1);
    std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd+1);

    if (keyStart == std::string::npos ||
        keyEnd == std::string::npos ||
        dataStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'a\' command at line " << num;
        throw Error(os.str());
    }

    std::string key(line.substr(keyStart, keyEnd-keyStart));
    std::pair<uint16_t, uint16_t> p = IptcDataSets::decomposeKey(key);
    if (p.first == 0xffff) throw Error("Invalid key " + key);
    if (p.second == IptcDataSets::invalidRecord) throw Error("Invalid key " + key);

    std::string data(line.substr(dataStart));
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }
    TypeId type = IptcDataSets::dataSetType(p.first, p.second);
    Value *val = Value::create(type);
    val->read(data);

    int rc = g_iptcData.add(IptcKey(key), val);
    if (rc) {
        std::string error = IptcData::strError(rc, "Input file");
        throw Error(error);
    }
    
}

void processRemove(const std::string& line, int num)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);

    if (keyStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'r\' command at line " << num;
        throw Error(os.str());
    }

    const std::string key( line.substr(keyStart) );
    std::pair<uint16_t, uint16_t> p = IptcDataSets::decomposeKey(key);
    if (p.first == 0xffff) throw Error("Invalid key" + key);
    if (p.second == IptcDataSets::invalidRecord) throw Error("Invalid key" + key);

    IptcData::iterator iter = g_iptcData.findId(p.first, p.second);
    if (iter != g_iptcData.end()) {
        g_iptcData.erase(iter);
    }
}

void processModify(const std::string& line, int num)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
    std::string::size_type keyEnd = line.find_first_of(" \t", keyStart+1);
    std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd+1);

    if (keyStart == std::string::npos ||
        keyEnd == std::string::npos ||
        dataStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'m\' command at line " << num;
        throw Error(os.str());
    }

    std::string key(line.substr(keyStart, keyEnd-keyStart));
    std::pair<uint16_t, uint16_t> p = IptcDataSets::decomposeKey(key);
    if (p.first == 0xffff) throw Error("Invalid key" + key);
    if (p.second == IptcDataSets::invalidRecord) throw Error("Invalid key" + key);

    std::string data(line.substr(dataStart));
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }
    TypeId type = IptcDataSets::dataSetType(p.first, p.second);
    Value *val = Value::create(type);
    val->read(data);

    IptcData::iterator iter = g_iptcData.findId(p.first, p.second);
    if (iter != g_iptcData.end()) {
        iter->setValue(val);
    }
    else {
        int rc = g_iptcData.add(IptcKey(key), val);
        if (rc) {
            std::string error = IptcData::strError(rc, "Input file");
            throw Error(error);
        }
    }
}
