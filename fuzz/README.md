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
