#ifndef _WINPOSIX_EXPORT_H_
#define _WINPOSIX_EXPORT_H_

// rmills
// msvc/include/winposix_export.h
//
// I'm very appreciative and respectful of the work of Jaroslaw Staniek <js@iidea.pl>
// in the KDE libraries where I found msvc/src/mmap.cpp and msvc/mman.h
//
// http://websvn.kde.org/trunk/KDE/kdelibs/win/include/msvc/sys/mman.h?revision=517357&view=markup&pathrev=519502
// http://websvn.kde.org/trunk/KDE/kdelibs/win/src/mmap.c?revision=519502&view=markup&pathrev=519502

// I have commented 2 essential tiny changes in mmap.cpp to make it compile with MSVC
// I've also added a dummy sys/socket.h and sys/types.h file to keep everybody happy!
//
// the contents of this file are the minimum required to enable exiv2 to compile link and execute the mmap.cpp code
//

#ifndef ENOTSUP
#define ENOTSUP 911
#endif

#ifndef off_t
#define off_t size_t
#endif

#ifndef KDEWIN32_EXPORT
#define KDEWIN32_EXPORT
#endif

// give MSVC 7.1 (VS 2003 .Net) encouragement to ignore _set_errno !
#if _MSC_VER < 1400
#ifndef _set_errno
#define _set_errno(x)
#endif
#endif

#endif

