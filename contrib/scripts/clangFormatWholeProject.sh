# This script is making the following assumptions:
# - 1) You are running the script from the project root directory
# - 2) clang-format is in your $PATH

find src/ -iname *.h* -o -iname *.c* | xargs clang-format -i
find include/ -iname *.h* -o -iname *.c* | xargs clang-format -i
find samples/ -iname *.h* -o -iname *.c* | xargs clang-format -i
find unitTests/ -iname *.h* -o -iname *.c* | xargs clang-format -i
find app/ -iname *.h* -o -iname *.c* | xargs clang-format -i
find fuzz/ -iname *.h* -o -iname *.c* | xargs clang-format -i
