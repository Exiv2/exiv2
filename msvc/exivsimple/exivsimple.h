// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXIVSIMPLE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXIVSIMPLE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EXIVSIMPLE_EXPORTS
#define EXIVSIMPLE_API __declspec(dllexport)
#else
#define EXIVSIMPLE_API __declspec(dllimport)
#endif

DECLARE_HANDLE (HIMAGE);

#ifdef __cplusplus
extern "C"
{
#endif

// These types should match those in types.hpp. For all of the functions
// that take a type, passing invalidTypeId causes the type to be guessed.
// Guessing types is accurate for IPTC but not for EXIF.
enum DllTypeId { invalidTypeId, unsignedByte, asciiString, unsignedShort, 
                unsignedLong, unsignedRational, invalid6, undefined, 
                signedShort, signedLong, signedRational, 
                string, date, time,
                lastTypeId };

typedef bool (CALLBACK* METAENUMPROC)(const char *key, const char *value, void *user);

EXIVSIMPLE_API HIMAGE OpenImage(const char *file);
EXIVSIMPLE_API void FreeImage(HIMAGE img);
EXIVSIMPLE_API int SaveImage(HIMAGE img);
EXIVSIMPLE_API int ReadMeta(HIMAGE img, const char *key, char *buff, int buffsize);
EXIVSIMPLE_API int EnumMeta(HIMAGE img, METAENUMPROC proc, void *user);
EXIVSIMPLE_API int AddMeta(HIMAGE img, const char *key, const char *val, DllTypeId type);
EXIVSIMPLE_API int ModifyMeta(HIMAGE img, const char *key, const char *val, DllTypeId type);
EXIVSIMPLE_API int RemoveMeta(HIMAGE img, const char *key);

#ifdef __cplusplus
}
#endif
