# Exiv2 fuzzing

This directory contains a [libFuzzer](https://llvm.org/docs/LibFuzzer.html) fuzzing target for Exiv2. The fuzzer is run for a short period of time on every pull request by the [`on_PR_linux_fuzz`](/.github/workflows/on_PR_linux_fuzz.yml) Action.

## Running the fuzzer

To run the fuzzer locally, first build it:

```bash
cd <exiv2dir>
mkdir build-fuzz
cd build-fuzz
cmake -DEXIV2_ENABLE_PNG=ON -DEXIV2_ENABLE_WEBREADY=ON -DEXIV2_ENABLE_CURL=ON -DEXIV2_ENABLE_BMFF=ON -DEXIV2_TEAM_WARNINGS_AS_ERRORS=ON -DCMAKE_CXX_COMPILER=$(which clang++) -DEXIV2_BUILD_FUZZ_TESTS=ON -DEXIV2_TEAM_USE_SANITIZERS=ON ..
make -j $(nproc)
```

This is the command to run the fuzzer for 2 minutes:

```bash
cd <exiv2dir>/build-fuzz
mkdir corpus
LSAN_OPTIONS=suppressions=../fuzz/knownleaks.txt ./bin/fuzz-read-print-write corpus ../test/data/ -dict=../fuzz/exiv2.dict -jobs=$(nproc) -workers=$(nproc) -max_len=20480 -max_total_time=120
```

Alternatively, a simple script is provided for running the fuzzer in a continuous loop:

```bash
../fuzz/fuzzloop.sh
```

### HEIF metadata writing

Both `fuzz-read-write` and `fuzz-read-print-write` exercise HEIF edits after
their initial unchanged write. The input size modulo four selects an Exif
description update, Exif removal, XMP removal, or removal of both categories.
After a successful edit, the result is read again through the public image API.

For a focused run, build with BMFF support and Clang's sanitizers. From the
repository root, with the [build dependencies](../README.md#dependencies)
installed:

```bash
cmake -S . -B build-heif-fuzz -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DEXIV2_ENABLE_BMFF=ON -DEXIV2_BUILD_FUZZ_TESTS=ON \
  -DEXIV2_TEAM_USE_SANITIZERS=ON
cmake --build build-heif-fuzz --parallel 4 \
  --target fuzz-read-write fuzz-read-print-write
```

The following existing fixtures select all four edit operations. Copy them
into the build directory and use a separate writable corpus for each target.
The 2 MiB input limit accommodates these HEIF seeds, including files larger
than the limits in the general examples. Each target runs for about 30 seconds.

```bash
(
  set -e
  cd build-heif-fuzz
  mkdir -p heif-seeds
  cp ../test/data/Canon.HIF ../test/data/Stonehenge.heic \
    ../test/data/heic.heic ../test/data/issue_9292_sony_8_left-bottom.HIF heif-seeds/
  for target in fuzz-read-write fuzz-read-print-write; do
    mkdir -p "heif-corpus-$target"
    LSAN_OPTIONS=suppressions=../fuzz/knownleaks.txt \
      "./bin/$target" "heif-corpus-$target" heif-seeds \
      -dict=../fuzz/exiv2.dict -max_len=2097152 -max_total_time=30 \
      -timeout=10 -rss_limit_mb=2048 -artifact_prefix="heif-corpus-$target/"
  done
)
```

## Generating a dictionary

Fuzzers perform better with a [dictionary](https://llvm.org/docs/LibFuzzer.html#dictionaries). For example, suppose the code contains a condition like [this](https://github.com/Exiv2/exiv2/blob/15098f4ef50cc721ad0018218acab2ff06e60beb/src/xmpsidecar.cpp#L177-L179):

```c
if (xmpPacket_.substr(0, 5)  != "<?xml") {
    xmpPacket_ = xmlHeader + xmpPacket_ + xmlFooter;
}
```

Adding the string `"<?xml"` to the dictionary will help the fuzzer to trigger both branches of this condition.

This directory contains a simple [CodeQL query](mkdictionary.ql) which searches the source code for string literals that are used in conditions. Since the resulting dictionary is relatively small, and unlikely to need to change very often, it has been checked into the repository as a text file: [exiv2.dict](exiv2.dict).

To run the CodeQL query to generate a new dictionary, you first need to build a database:

```bash
cd <exiv2dir>
codeql database create --language=cpp exiv2db
```

Then run the query and convert the results to JSON:

```bash
codeql query run --database=exiv2db --output=dict.bqrs fuzz/mkdictionary.ql
codeql bqrs decode --format=json --output dict.json dict.bqrs
```

Finally, use [`mkdictionary.py`](mkdictionary.py) to convert the JSON to libFuzzer's dictionary format:

```bash
./fuzz/mkdictionary.py dict.json | sort > ./fuzz/exiv2.dict
```
