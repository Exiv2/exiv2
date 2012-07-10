// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
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
  File:      version.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Mar-07, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "version.hpp"

// + standard includes
#include <iomanip>
#include <sstream>

namespace Exiv2 {
    int versionNumber()
    {
        return EXIV2_MAKE_VERSION(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION);
    }

    std::string versionNumberHexString()
    {
        std::ostringstream os;
        os << std::hex << std::setw(6) << std::setfill('0') << Exiv2::versionNumber();
        return os.str();
    }

    const char* version()
    {
        return EXV_PACKAGE_VERSION;
    }

    bool testVersion(int major, int minor, int patch)
    {
        return versionNumber() >= EXIV2_MAKE_VERSION(major,minor,patch);
    }
}                                       // namespace Exiv2


#include <stdio.h>

// platform specific support for dumpLibraryInfo
#ifdef  WIN32
# include <windows.h>
# include <psapi.h>

#ifndef lengthof
#define lengthof(x) sizeof(x)/sizeof(x[0])
#endif

#ifdef	_MSC_VER
#pragma comment( lib, "psapi" )
#endif

#elif   __APPLE__
# include <mach-o/dyld.h>

#else   __linux__
// http://syprog.blogspot.com/2011/12/listing-loaded-shared-objects-in-linux.html
# include "link.h"
# include <dlfcn.h>
  struct something
  {
	void*  pointers[3];
	struct something* ptr;
  };
  struct lmap
  {
    void*    base_address;   /* Base address of the shared object */
    char*    path;           /* Absolute file name (path) of the shared object */
    void*    not_needed1;    /* Pointer to the dynamic section of the shared object */
    struct lmap *next, *prev;/* chain of loaded objects */
  };
#endif

EXIV2API void dumpLibraryInfo(std::ostream& os)
{
	char    path     [500];
	char    szBuilder[200];
	size_t  path_l   = 0  ;
	size_t  path_max = sizeof(path) - 2;
	bool    bReport  = false;
	int     bits     = sizeof(void*);

#if defined(_DEBUG) || defined(DEBUG)
	int debug=1;
#else
	int debug=0;
#endif

#if defined(EXV_HAVE_DLL)
	int dll=1;
#else
	int dll=0;
#endif

#ifdef	_MSC_VER
	sprintf(szBuilder,"MSVC=%d,DEBUG=%d,DLL=%d,Bits=%d:",((_MSC_VER-600)/100),debug,dll,bits);
#else
	sprintf(szBuilder,"GCC=%s,DEBUG=%d,DLL=%d,Bits=%d: ",__VERSION__,debug,dll,bits);
#endif
	path[0]=0;

	// enumerate loaded libraries and determine path to executable
#if WIN32
	bReport = true;
	HMODULE handles[100];
	DWORD   cbNeeded;
	if ( EnumProcessModules(GetCurrentProcess(),handles,lengthof(handles),&cbNeeded)) {
		char szFilename[_MAX_PATH];
		for ( DWORD h = 0 ; h < cbNeeded/sizeof(handles[0]) ; h++ ) {
			GetModuleFileNameA(handles[h],szFilename,lengthof(szFilename)) ;
			os << szBuilder << "module=" << szFilename << std::endl;
			if ( h==0 ) {
				path_l = strlen(szFilename);
				if ( path_l > path_max ) path_l = 0;
				if ( path_l > 0        ) strcpy(path,szFilename);
			}
		}
	}
#elif __APPLE__
	bReport = true;
	// man 3 dyld
	uint32_t count = _dyld_image_count();
	for (uint32_t image = 0 ; image < count ; image++ ) {
		 const char* image_path = _dyld_get_image_name(image);
		 os << szBuilder << "library=" << image_path << std::endl;
	     if ( image==0 ) {
	     	path_l = strlen(image_path);
	     	if ( path_l > path_max ) path_l = 0;
	     	if ( path_l > 0        ) strcpy(path,image_path);
	     }
	}

#elif __linux__
	bReport = true;
	// http://syprog.blogspot.com/2011/12/listing-loaded-shared-objects-in-linux.html
	struct lmap* pl;
	void* ph = dlopen(NULL, RTLD_NOW);
	struct something* p = (struct something*)ph;
	p = p->ptr;
	pl = (struct lmap*)p->ptr;

	while(NULL != pl)
	{
		os << szBuilder << "library=" << pl->path << std::endl;
		pl = pl->next;
	}
	// http://stackoverflow.com/questions/606041/how-do-i-get-the-path-of-a-process-in-unix-linux
	char proc[100];
	sprintf(proc,"/proc/%d/exe", getpid());
	path_l = readlink (proc, path, path_max);
#endif

	if ( bReport ) {
		os << "builder=" << szBuilder << std::endl;
		if ( path_l > 0 &&  path_l < path_max ) {
			path[path_l]=0;
			os << szBuilder << "executable=" << path << std::endl;
			os << szBuilder << "date=\"" << __DATE__ << "\",time=" __TIME__ << std::endl;
		}
	}
}

