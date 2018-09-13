// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

#ifndef __INIREADER_H__
#define __INIREADER_H__

#include "exiv2lib_export.h"

#include <map>
#include <string>
#include <stdio.h>

namespace Exiv2 {

/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#ifndef __INI_H__
#define __INI_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

//! @brief typedef for prototype of handler function.
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value);

//! Typedef for prototype of fgets-style reader function.
typedef char* (*ini_reader)(char* str, int num, void* stream);

/*!
    @brief Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's configparser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   @param filename path to file
   @param handler  user defined handler
   @param user     void pointer passed to user handler

   @return 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).

   @return 0 on success
*/
int ini_parse(const char* filename, ini_handler handler, void* user);

/*! @brief Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that.

   @param file     open "C" file
   @param handler  user defined handler
   @param user     void pointer passed to user handler
   */
int ini_parse_file(FILE* file, ini_handler handler, void* user);

/*! @brief Same as ini_parse(), but takes an ini_reader function pointer instead of
   filename. Used for implementing custom or string-based I/O.
   @param reader   magic
   @param stream   more magic
   @param handler  user defined handler
   @param user     void pointer passed to user handler

   @return 0 on success
*/
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user);

/*! @brief Nonzero to allow multi-line value parsing, in the style of Python's
    configparser. If allowed, ini_parse() will call the handler with the same
    name for each subsequent line parsed.
*/
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/*! @brief Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21
*/
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/*! @brief Nonzero to allow inline comments (with valid inline comment characters
   specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
   Python 3.2+ configparser behaviour.
*/
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

//! @brief Nonzero to use stack, zero to use heap (malloc/free).
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

//! @brief Stop parsing on first error (default is to keep parsing).
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

//! @brief Maximum line length for any line in INI file.
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

#endif /* __INI_H__ */


/*! @brief Read an INI file into easy-to-access name/value pairs. (Note that I've gone
  for simplicity here rather than speed, but it should be pretty decent.)
  */
class EXIV2API INIReader
{
public:
    /*! @brief Construct INIReader and parse given filename. See ini.h for more info
       about the parsing.
    */
    INIReader(std::string filename);

    /*! @brief Return the result of ini_parse(), i.e., 0 on success, line number of
        first error on parse error, or -1 on file open error.
    */
    int ParseError();

    /*! @brief Get a string value from INI file, returning default_value if not found.

      @param section name of section
      @param name    name of key
      @param default_value default if not found

      @return value
    */
    std::string Get(std::string section, std::string name,
                    std::string default_value);

    /*! @brief Get an integer (long) value from INI file, returning default_value if
        not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").

      @param section name of section
      @param name    name of key
      @param default_value default if not found

      @return value
    */
    long GetInteger(std::string section, std::string name, long default_value);

    /*! @brief Get a real (floating point double) value from INI file, returning
        default_value if not found or not a valid floating point value
        according to strtod().

      @param section name of section
      @param name    name of key
      @param default_value default if not found

      @return value
    */
    double GetReal(std::string section, std::string name, double default_value);

    /*! @brief Get a boolean value from INI file, returning default_value if not found or if
        not a valid true/false value. Valid true values are "true", "yes", "on", "1",
        and valid false values are "false", "no", "off", "0" (not case sensitive).

      @param section name of section
      @param name    name of key
      @param default_value default if not found

      @return value
    */
    bool GetBoolean(std::string section, std::string name, bool default_value);

private:
    int _error;                                                        //!< status
    std::map<std::string, std::string> _values;                        //!< values from file
    static std::string MakeKey(std::string section, std::string name); //!< return key encoded from section/name
    static int ValueHandler(void* user, const char* section, const char* name,
                            const char* value); //!< value handler
};
} // namespace Exiv2

#endif  // __INIREADER_H__
