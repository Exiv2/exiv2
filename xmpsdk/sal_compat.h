// SPDX-License-Identifier: GPL-2.0-or-later
//
// SAL (Source Annotation Language) compatibility for non-MSVC compilers on Windows.
//
// The XMP-Toolkit-SDK uses MSVC SAL annotations (e.g. _Check_return_wat_,
// _In_z_, _Out_z_cap_) in SafeStringAPIs.h. These annotations are defined
// in MSVC's <sal.h> header. The SDK's SuppressSAL.h provides empty
// definitions for all SAL macros, but only activates on non-Windows
// platforms (#if !defined(_WIN32) && !defined(_WIN64)).
//
// Non-MSVC compilers on Windows (e.g. MSYS2-Clang, MinGW-GCC) define
// _WIN32 but don't ship MSVC's <sal.h>, so the SAL macros remain undefined
// and compilation fails.
//
// This header works around the issue by temporarily undefining _WIN32/_WIN64
// so that SuppressSAL.h's guard condition passes and the SAL macros get
// defined as empty. push_macro/pop_macro restores _WIN32/_WIN64 afterward
// so the rest of the build sees them normally.
//
// Used via -include in the meson build for non-MSVC Windows compilers.

#if (defined(_WIN32) || defined(_WIN64)) && !defined(_MSC_VER)
#pragma push_macro("_WIN32")
#pragma push_macro("_WIN64")
#undef _WIN32
#undef _WIN64
#include "source/SuppressSAL.h"
#pragma pop_macro("_WIN64")
#pragma pop_macro("_WIN32")
#endif
