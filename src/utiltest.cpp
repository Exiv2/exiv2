// ***************************************************************** -*- C++ -*-
/*
  AUTHOR(S): Andreas Huggel (ahu)
  HISTORY:
   10-Dec-03, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.4 $
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.4 $ $RCSfile: utiltest.cpp,v $");

// *****************************************************************************
// included header files
#include "utils.hpp"

#include <iostream>
#include <iomanip>

void testStrtol();
void testStrError();
void testPaths();
void testPath(const std::string& path);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
    testPaths();
    return 0;
}

void testStrtol()
{
    bool rc;
    long n(0);
    std::string s;

    s = "22"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "1"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "-22222222222222222"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "0x0"; rc = Util::strtol(0, n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = ""; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "abc"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "1.2"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
    s = "12p"; rc = Util::strtol(s.c_str(), n);
    std::cout << "s = `" << s << "' rc = " << rc << " n = " << n << "\n";
}

void testStrError()
{
    std::string oldname = "/tmp/foo";
    std::string newname = "/tmp/bar";

    if (::rename(oldname.c_str(), newname.c_str()) == -1) {
        std::cerr << "failed to rename " 
                  << oldname << " to " << newname << ": ";
        std::cerr << Util::strError() << "\n";
    }
}

void testPaths()
{
    std::string path;
    path = "/usr/lib"; testPath(path);
    path = "/usr/";  testPath(path);
    path = "usr";  testPath(path);
    path = "/";  testPath(path);
    path = ".";  testPath(path);
    path = "..";  testPath(path);
    path = "///";  testPath(path);
    path = "/usr/.emacs";  testPath(path);
    path = "/usr/.emacs/";  testPath(path);
    path = "/usr/.emacs//";  testPath(path);
    path = "usr/.emacs";  testPath(path);
    path = ".emacs";  testPath(path);
    path = ".emacs.gz";  testPath(path);
    path = "/tmp/image.jpg";  testPath(path);
    path = "/tmp/.image.jpg";  testPath(path);
    path = "/image.jpg";  testPath(path);
    path = "image.jpg";  testPath(path);
    path = "image.jpg//";  testPath(path);
    path = "/////image.jpg";  testPath(path);
    path = "/foo.bar/image";  testPath(path);
    path = "/foo.bar/images.tar.gz";  testPath(path);
    path = "d:\\foo.bar\\images.tar.gz";  testPath(path);
}

void testPath(const std::string& path)
{
    std::cout << std::setw(15) << path << " " 
              << std::setw(15) << Util::dirname(path) << " " 
              << std::setw(15) << Util::basename(path) << " " 
              << std::setw(15) << Util::suffix(path) << "\n";
}
