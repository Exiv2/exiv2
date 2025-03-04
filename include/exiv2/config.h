// config.h

#ifndef _CONFIG_H_
#define _CONFIG_H_

///// Start of Visual Studio Support /////
#ifdef _MSC_VER

#pragma warning(disable : 4996)  // Disable warnings about 'deprecated' standard functions
#pragma warning(disable : 4251)  // Disable warnings from std templates about exporting interfaces

#endif  // _MSC_VER
///// End of Visual Studio Support /////

#include "exv_conf.h"
////////////////////////////////////////

#endif  // _CONFIG_H_
