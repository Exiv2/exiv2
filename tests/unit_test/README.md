# README

- Plan to convert the bash test scripts `../../test/*.sh` into Python scripts and save them to this directory.
- The test cases in this directory are based on [unittest](https://docs.python.org/3/library/unittest.html) and written in a common format, which is different from the format described in `../writing_tests.md`, but can be executed compatibly.


## Running the test cases

- Use runner.py to collect test cases and execute them:
  ```sh
  cd EXIV2_DIR/tests
  python3 runner.py -v unit_test
  ```

- Also, you can use pytest to execute the test cases for unittest, so that the test report looks better:
  ```sh
  cd EXIV2_DIR/tests
  pytest -v unit_test
  ```

- If no exception occurs during the execution of the test case, it passes. Otherwise it fails.
