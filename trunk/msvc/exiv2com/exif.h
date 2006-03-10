// exif.h : Declaration of the Cexif

#pragma once
#include "resource.h"       // main symbols

#include "exiv2com.h"


// Cexif

class ATL_NO_VTABLE Cexif : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Cexif, &CLSID_exif>,
	public IDispatchImpl<Iexif, &IID_Iexif, &LIBID_exiv2comLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	Cexif()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EXIF)


BEGIN_COM_MAP(Cexif)
	COM_INTERFACE_ENTRY(Iexif)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

	STDMETHOD(GetExif)(BSTR FileName, BSTR Tag, VARIANT* OutVal);
	STDMETHOD(GetExifInterpreted)(BSTR FileName, VARIANT* OutVal);
};

OBJECT_ENTRY_AUTO(__uuidof(exif), Cexif)
