#!/bin/bash

# Stop when an error is found
set -e

# The memory and disk usage grows over time, so this loop restarts the
# fuzzer every 4 hours. The `-merge=1` option is used to minimize the
# corpus on each iteration.
while :
do
    date
    echo restarting loop

    # Minimize the corpus
    mv corpus/ corpus2
    mkdir corpus
    echo minimizing corpus
    LSAN_OPTIONS=suppressions=../fuzz/knownleaks.txt ./bin/fuzz-read-print-write -max_len=20480 -merge=1 corpus ../test/data/ corpus2/
    rm -r corpus2

    # Run the fuzzer for 4 hours
    date
    echo start fuzzer
    LSAN_OPTIONS=suppressions=../fuzz/knownleaks.txt ./bin/fuzz-read-print-write corpus -dict=../fuzz/exiv2.dict -jobs=$(nproc) -workers=$(nproc) -max_len=20480 -max_total_time=14400
done
