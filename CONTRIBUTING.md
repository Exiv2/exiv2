# How to contribute #

All help is appreciated. All contributors are unpaid volunteers. If you really want to see something added to Exiv2 - volunteer!


## General workflow for contributing ##

1. Fork, then clone the repo:
```
git clone git@github.com:your-username/exiv2.git
```

2. Configure the project and build (More details about how to run CMake at XXX) :
```
mkdir build && cd build
cmake .. && cmake --build . --config Release
```

3. Make sure the tests pass:
```
make tests			# Application tests
./bin/unit_tests	# Unit tests
```

4. Make your change. Add tests for your change. Make the tests pass.
5. Create the Pull Request (PR).
	a. Suggest a reviewer if you know that some of the mantainers know about the specific change.
    b. Check if the CI checks pass.
    c. If your PR lives for a long time, try to avoid to press the button "Update branch" in the Pull Request view. We prefer rebasing in top of master, instead of merging master into branches.

## Pull Request guidelines
- PRs should be kept at a manageable size. Try to focus in just one goal per PR. If you find yourself doing several things that in a PR that were not expected, try to split the different tasks in different PRs.
- Commits should always change one logical unit, so that cherry-picking & reverting is simple.
- Commit messages should as informative and concise as possible. The first line of the commit message should be < 80 characters and describe the commit briefly. If the 80 characters are too short for a summary, consider splitting the commit. Below the short summary add one blank line and then a more detailed explanation if required.

## Code guidelines ##
### General ###
- All new code must be properly tested: via unit tests (based on the Gtest framework; see `$REPO/unitTest`) or system tests (scripts exercising the main exiv2 application; see `$REPO/test`).
- Code should be simple to read and to understand.
- Do not invoke undefined behavior! Ensure that with UBSAN, i.e. compile your code with `-fsanitize=undefined` and run the test suite.
- Ensure that your code has no memory errors. Use ASAN for that, i.e. compile your code with `-fsanitize=address`.

### Relevant to the Exiv2 domain ###
- All new code that is added must be resistant to integer overflows, thus if you multiply, add, subtract, divide or bitshift integers you must ensure that no overflow can occur. Please keep in mind that signed integer overflow is undefined behavior, thus you must check for overflows before performing the arithmetic operation, otherwise the compiler is free to optimize your check after the overflow away (this has happened already).
- All new code must be resistant to buffer overflows. Thus before you access arrays a range check must be performed.
- Distrust any data that you extract from images or from external sources. E.g. if the metadata of an image gives you an offset of another information inside that file, do not assume that this offset will not result in an out off bounds read.
- New code must not assume the endianes and the wordsize of the system it is being run on. I.e. don't assume that  `sizeof(int) = 8` or that the following will work:
```cpp
const uint32_t some_var = get_var();
const uint16_t lower_2_bytes = (const uint16_t*) &some_var;
```
since this will give you the upper two bytes on big endian systems.
If in doubt, use the fixed size integer types like `int32_t`, `uint64_t`, `size_t`, etc.


## Code Formatting ##
The project contains a `.clang-format` file defining the code formatting of the project (more details about of this file was defined can be found in this [PR](https://github.com/Exiv2/exiv2/pull/152)). One can apply this format to new or existing code by using the **clang-format** command-line tool, or by using some integration plugins provided by some editors or IDEs. Currently we know about these integrations:
- [QtCreator](http://doc.qt.io/qtcreator/creator-beautifier.html) -> beautifier -> clang-format
- [vim-clang-format](https://github.com/rhysd/vim-clang-format)
- [Emacs](https://clang.llvm.org/docs/ClangFormat.html#emacs-integration)
- Visual Studio: [1](http://clang.llvm.org/docs/ClangFormat.html#visual-studio-integration), [2](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)

Note that some times, the formatting applied to some complex code might result in some un-expected output. If you know how to improve the current `.clang-format` file to deal with such cases, please contribute!. Otherwise, you have two options:
1. Apply clang-format to individual blocks of code (avoid to apply it over the complex piece of code).
2. Indicate which parts of the code should not be treaten by clang-format:
```cpp
// clang-format off
    void    unformatted_code  ;
// clang-format on
```

More information about clang:
- Link to [clang-format](https://clang.llvm.org/docs/ClangFormat.html) tool.
- Link to the [clang-format option](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).


## Other points to consider ##

- [Old document](http://dev.exiv2.org/projects/exiv2/wiki/Contributing_to_Exiv2) with more information about contributions.
