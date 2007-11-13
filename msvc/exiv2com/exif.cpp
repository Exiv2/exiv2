// exif.cpp : Implementation of Cexif

#include "stdafx.h"
#include "exif.h"

// *****************************************************************************
// included header files
#include <exiv2/exv_msvc.h>
#include <exiv2/actions.hpp>
#include <exiv2/image.hpp>
#include <exiv2/jpgimage.hpp>
#include <exiv2/exiv2.hpp>
#include <exiv2/utils.hpp>
#include <exiv2/types.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/canonmn.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/futils.hpp>
#ifndef EXV_HAVE_TIMEGM
# include <exiv2/timegm.h>
#endif

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cassert>
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for stat()
#endif
#ifdef _MSC_VER
# include <sys/utime.h>
#include ".\exif.h"
#else
# include <utime.h>
#endif


// Cexif


char* ConvertBSTRToLPSTR (BSTR bstrIn)
   {
   LPSTR pszOut = NULL;

   if (bstrIn != NULL)
   	{
   		int nInputStrLen = SysStringLen (bstrIn);
   
   		// Double NULL Termination
   		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, NULL, 0, 0, 0) + 2;	
   
   		pszOut = new char [nOutputStrLen];
   
   		if (pszOut)
   		{
   		    memset (pszOut, 0x00, sizeof (char)*nOutputStrLen);
   
 		 WideCharToMultiByte (CP_ACP, 0, bstrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
   		}
   	 }

	return pszOut;
   }

STDMETHODIMP Cexif::GetExif(BSTR FileName, BSTR Tag, VARIANT* OutVal)
{
    if (!Exiv2::fileExists(ConvertBSTRToLPSTR(FileName), true))
        return S_FALSE;

	Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(ConvertBSTRToLPSTR(FileName));
    image->readMetadata();

	Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty())
        return S_FALSE;

	Exiv2::ExifData::iterator md = exifData.findKey(Exiv2::ExifKey(ConvertBSTRToLPSTR(Tag)));
	if (md != exifData.end()) {
		BSTR a = A2BSTR( (md->value().toString()).c_str() );
		OutVal->vt = VT_BSTR;
		OutVal->bstrVal = a;
	}

	return S_OK;
}

STDMETHODIMP Cexif::GetExifInterpreted(BSTR FileName, VARIANT* OutVal)
{
    if (!Exiv2::fileExists(ConvertBSTRToLPSTR(FileName), true))
        return S_FALSE;

	std::stringstream ssRetVal; 

	Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(ConvertBSTRToLPSTR(FileName));
    image->readMetadata();

	//exif info
	Exiv2::ExifData &exifData = image->exifData();
    if (!exifData.empty())
	{
		Exiv2::ExifData::const_iterator md;
		for (md = exifData.begin(); md != exifData.end(); ++md)
		{
			ssRetVal	<< md->key() << ": "
						<< std::dec << *md
						<< std::endl;
		}
	}

	//iptc
    Exiv2::IptcData &iptcData = image->iptcData();
    if (!iptcData.empty())
	{
		Exiv2::IptcData::const_iterator md;
		for (md = iptcData.begin(); md != iptcData.end(); ++md)
		{
			ssRetVal	<< md->key() << ": "
						<< std::dec << *md
						<< std::endl;
		}
	}

	OutVal->vt = VT_BSTR;
	OutVal->bstrVal = A2BSTR(ssRetVal.str().c_str());

	return S_OK;
}
