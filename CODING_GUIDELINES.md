Coding Guidelines
======================

# Contents #

* [1. General Guidelines](#10-general-guidelines)
* [2. Integer and Array Overflows](#20-integer-and-array-overflows)
* [3. Code Formatting](#30-code-formatting)
  * [3.1 Guidelines to Apply clang-format](#31-guidelines-to-apply-clang-format)

# 1. General Guidelines #
- All new code must be properly tested: via unit tests (based on the Gtest framework; see `$REPO/unitTest`) or system tests (scripts exercising the main exiv2 application; see `$REPO/test`).
- Code should be simple to read and to understand.
- Do not invoke undefined behavior. [Optional] Ensure that with UBSAN, i.e. compile your code with `-fsanitize=undefined` and run the test suite.
- Ensure that your code has no memory errors. [Optional] Use ASAN for that, i.e. compile your code with `-fsanitize=address`.

# 2. Integer and Array Overflows #
- All new code that is added must be resistant to integer overflows, thus if you multiply, add, subtract, divide or bitshift integers you must ensure that no overflow can occur. Please keep in mind that signed integer overflow is undefined behavior, thus you must check for overflows before performing the arithmetic operation, otherwise the compiler is free to optimize your check after the overflow away (this has happened already).
- All new code must be resistant to buffer overflows. Thus before you access arrays a range check must be performed.
- Distrust any data that you extract from images or from external sources. E.g. if the metadata of an image gives you an offset of another information inside that file, do not assume that this offset will not result in an out off bounds read.
- New code must not assume the endianes and the word size of the system it is being run on. I.e. don't assume that  `sizeof(int) = 8` or that the following will work:
```cpp
const uint32_t some_var = get_var();
const uint16_t lower_2_bytes = (const uint16_t*) &some_var;
```
since this will give you the upper two bytes on big endian systems.
If in doubt, then use the fixed size integer types like `int32_t`, `uint64_t`, `size_t`, etc.

# 3. Code Formatting #

The project contains a `.clang-format.optional` file defining the code formatting of the project (more details about of this file was defined can be found in this [PR](https://github.com/Exiv2/exiv2/pull/152)). We do not provide it via the standard name (`.clang-format`), since we do not enforce code formatting and do not want editors to automatically format everything.

Nevertheless, we suggest you to respect the code formatting by symlinking `.clang-format.optional` to `.clang-format` and applying `clang-format` to new or existing code. You can do it by using the `clang-format` command-line tool or by using one of the integration plugins provided by various editors or IDEs. Currently we know about these integrations:

- [QtCreator](http://doc.qt.io/qtcreator/creator-beautifier.html) -> beautifier -> clang-format
- [vim-clang-format](https://github.com/rhysd/vim-clang-format)
- [Emacs](https://clang.llvm.org/docs/ClangFormat.html#emacs-integration)
- Visual Studio: [1](http://clang.llvm.org/docs/ClangFormat.html#visual-studio-integration), [2](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)

Note that some times the formatting applied to complex code might result in some unexpected output. If you know how to improve the current `.clang-format` file to deal with such cases, then please contribute!. Otherwise, you have two options:
1. Apply `clang-format` to individual blocks of code (avoid to apply it over the complex piece of code).
2. Indicate which parts of the code that should not be `clang-format` formatted:

```cpp
// clang-format off
    void    unformatted_code  ;
// clang-format on
```

## 3.1 Guidelines to Apply clang-format ##

- New files should follow the clang-format style.
- Old files will be completely re-formatted only if we need to touch several lines/functions/methods of that file. In that case, we suggest to first create a PR just re-formatting the files that will be touched. Later we create another PR with the code changes.
- If we only need to fix a small portion of a file then we do not apply clang-format at all, or we just do it in the code block that we touch.

More information about clang:
- Link to [clang-format](https://clang.llvm.org/docs/ClangFormat.html) tool.
- Link to the [clang-format option](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

