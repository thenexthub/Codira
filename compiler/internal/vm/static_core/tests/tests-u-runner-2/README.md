# Universal test runner, version 2

## Description
Universal test runner, version 2, for Open Harmony.

## Prerequisites
-   Panda build
-   Python3 version at least 3.8. 3.10 is recommended.
-   Make sure to run `sudo static_core/scripts/install-deps-ubuntu -i=test` to create a ~/.venv-panda virtual environment with
all required python libraries  (`tqdm`, `dataclasses`, `python-dotenv`, etc). 
-   Suite `ets-es-checked` requires [node and some packages](#ETS-ES-checked-dependencies)

## Quick run

It is possible to run tests either using script `runner.sh` or `main.py` directly.

### Before the first start
- Create at your home the file `.urunner.env`
- Specify there following environment variables:
```bash
ARKCOMPILER_RUNTIME_CORE_PATH=<your path to the folder with cloned repository arkcompiler_runtime_core>
ARKCOMPILER_ETS_FRONTEND_PATH=<your path to the folder with cloned repository arkcompiler_ets_frontend>
PANDA_BUILD=<your path to build folder>
WORK_DIR=<your path to temporary folder where all intermediate files and reports will be kept>
```
- or run
```commandline
urunner2> ./runner.sh init
```
To create it in interactive mode

### Shell script
Script `runner.sh` activates the virtual environment with all required libraries
installed by `sudo static_core/scripts/install-deps-ubuntu -i=test` and then runs test(s).
This way is preferable.

After install script finishes you can run

```bash
urunner2> ./runner.sh <workflow-name> <test-suite-name> [option1... option]
```

### Python script
You can run `main.py` directly. In order to do so you have to activate
the virtual environment `$HOME/.venv-panda` manually or propose all required
libraries in your working environment. Then `main.py` will run test(s) for you.

```bash
urunner2>source ~/.venv-panda/bin/activate
urunner2>python3 main.py <workflow-name> <test-suite-name> [option1... option]
urunner2>deactivate
```

## Mandatory options

<workflow_name> should be from cfg/workflows config files

<test_suite_name> should be from cfg/test-suites config files

## Reproducing CI test run

In case of fail on CI there would be message in log how to rerun affected test locally:

```
To reproduce with URunner run:
(min parameters set): ${runner_path}/runner.sh panda-int-2 ets-cts --processes all --force-generate --show-progress --filter=07.expressions/25.equality_expressions/bigint_equality_operators/ne1*  --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets
(full parameters set): ${runner_path}/runner.sh panda-int-2 ets-cts --extension ets --load-runtimes ets --work-dir ${path to workflow folder} --build ${build path} --opt-level 2 --es2panda-timeout 30 --verifier-timeout 30 --enable-es2panda True --enable-verifier True --ark-timeout 180 --gc-type g1-gc --full-gc-bombing-frequency 0 --heap-verifier fail_on_verification --is-panda True --processes all --show-progress True --force-generate True --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets
```

To reproduce this run just copy runner parameters from it and run locally.

## Test lists

Runner supports following kinds of test lists:

-   **excluded** test lists - tests specified here are excluded from execution. "Don't try to run"
-   **ignored** test lists - tests specified here run, but failures are ignored. Such lists are named "known failures list" or KFL as well.

Test list name usually has the following format: `<test-suite-name>[-<additional info>]-<kind>[-<OS>][-<architecture>][-<configuration>][-<interpreter>][-<sanitizer>][-<opt-level>][-REPEATS][-<gc-type>][-<build-type>].txt`

-   `kind` is one of `excluded` or `ignored`
-   `OS` is one of `LIN`, `WIN`, `MAC`.   If an operating system is set explicitly, the test list is applied only to this OS. If none is set, the list is applied to any OS.
-   `architecture` is one of `ARM32`, `ARM64`, `AMD32`, `AMD64`.  If an architecture is set explicitly, the test list is applied only to this architecture. If none is set, the list is applied to any architecture.
-   `configuration` is one of `INT`, `AOT`, `AOT-FULL`, `IRTOC`, `LLVM`, `JIT`. If a configuration is set explicitly, the test list is applied only to this configuration. If none is set, the list is applied to any configuration. 
-   `interpreter`  used value for `interpreter-type` option. `DEFAULT` can be used to specify specific only for default interpreter. If an interpreter is set explicitly, the test list is applied only to this interpreter. If none is set, the list is applied to any interpreter.
-   `sanitizer` is one of `ASAN` or `TSAN`. If a sanitizer is set explicitly, the test list is applied only to this build configuration. If none is set, the list is applied to any configuration.
-   `opt-level` is `OLx`, where `x` is opt-level, usually 0 or 2.
-   `REPEATS` is set if the test list should apply to runs with the option `--jit-repeats` sets number of repeats more than 1.
-   `gc-type` is the value of option `--gc-type`. Usually, one of `G1-GC`, `GEN-GC`, `STW`.
-   `build-type` is one of `DEBUG`, `RELEASE`, `FAST-VERIFY`.   If a build type is set explicitly, the test list is applied only to this build type. If none is set, the list is applied to any build type.

Examples:

-   `test262-flaky-ignored-JIT.txt` - list of ignored tests from Test262 suite, applied only in JIT configuration. `flaky` is additional info, it's only for more clear description what tests are collected there.
-   `hermes-excluded.txt` - list of excluded tests from Hermes suite, applied in any configuration.
-   `parser-js-ignored.txt` - list of ignored tests from JS Parser suite, applied in any configuration.
-   `ets-func-tests-ignored.txt` - list of ignored tests in `ets-func-tests` test suite
-   `ets-cts-FastVerify-ignored-OL2.txt` - list of ignored tests for `ets-cts` test suite and for opt-level=2.  `FastVerify` is additional info.

In any test list the test is specified as path to the test file relative from the `test_root`: Examples:

-   array-large.js
-   built-ins/Date/UTC/fp-evaluation-order.js
-   tests/stdlib/std/math/sqrt-negative-01.ets

Test file specified in the option `--test-file`/`test-lists.explicit-file` should be set in this format too.
By default, ignored or excluded lists are located near tests themselves.

All test lists are loaded automatically from the specified `LIST_ROOT` and got architecture and sanitizer options from cmake_cache.

> **Note**: these options just specifies what test lists to load and do not affect on how and where to start the runner
> itself and binaries used within.

It is possible to describe an expected failure for a test in the ignore list.  
Place the Failure markup in the comment immediately before the test entry:
@@Failure: _Expected failure description_@@

Example:

#26512 @@Failure: SyntaxError: Unexpected token *@@
02.lexical_elements/09.literals/08.regex_literal/regex_anchor_boundaries.ets


## Utility runner options:

-   `--skip-test-lists` - do not use ignored or excluded lists, run all available tests, report all found failures
-   `--test-list TEST_LIST` - run tests ONLY listed in TEST\_LIST file.
-   `--test-file TEST_FILE` - run ONLY ONE specified test. **Attention** - not test suite, but the single test from the suite.
-   `--update-excluded` - regenerates excluded test lists
-   `--update-expected` - regenerates expected test lists (applied only for JS Parser test suite)
-   `--report-format` - specifies in what format to generate failure reports. By default, `md`. Possible value: `html`. As well reports in the plain text format with `.log` extension are always generated.
-   `--filter FILTER` - test filter regexp
-   `--show-progress` - show progress bar during test execution
-   `--time-report` - generates report with grouping tests by execution time.

## Detailed report

Detailed report shows test statistics for every folder
- `--detailed-report` - if it's specified the report is generated
- `--general.detailed-report-file` - specifies file/path where the report should be saved to

## Verbose and logging options:

-  `--verbose`, `-v` - Enable verbose output. Possible values one of:
   - `all` - the most detailed output,
   - `short` - test status and output.
   - if none specified (by default): only test status for new failed tests
   - in config file use `general.verbose` property with the save values.
-  `--verbose-filter` - Filter for what kind of tests to output stdout and stderr.
   Supported values:
   - `all` - for all executed tests both passed and failed.
   - `ignored` - for new failures and tests from ignored test lists including both passed and failed. '
   - `new` - only for new failures. Default value.
   - in config file use `general.verbose-filter` property with the same values.

## Generation options:

-   `--generate-only` - only generate tests without running them. Tests are run as usual without this option.
-   `--force-generate` - force ETS tests generation from templates

## Other options:

Main help function for the runner gives mandatory parameters, <workflow_name> <test_suite_name>

```./runner.sh --help```

If use ```'runner.sh <workflow_name> <test_suite_name> --help'``` 
e.g.:

```./runner.sh panda-int ets-cts --help```

There would be descriptions for parameters from 3 sections:

1. Parameters of the workflow stated (panda-int in example above)
2. Parameters of the test suite stated (ets-cts in example above)
3. Common runner parameters


## Execution time report

It is possible to collect statistics how long separate tests work. In the result report tests are grouped by execution time.

The grouping edges are set in seconds. For example, the value `1 5 10` specifies 4 groups - less than 1 second, from 1 second to 5 seconds, from 5 seconds to 10 seconds and from 10 seconds and more. For the last group the report contains real durations.

-   Specify the option `--time-report`
-   Specify in yaml config file `time-report.time-edges: [1, 5, 10]` where value is a list of integers.
-   After test run the short report will be output to the console
-   And full report will be created at the `<work-dir/report>/time_report.txt`


## Linter and type checks

A script running linter and type checks starts as:

`urunner>./linter.sh`

It performs checks by running `pylint` and `mypy`.

For `pylint` settings see `.pylintrc` file. For `mypy` settings see `mypy.ini` file.

## Customized run
If you want to run arbitrary set of ETS tests with URunner you can use your own test suite configuration file or your own workflow file:

1. If required, create new workflow config in cfg/workflows with required parameters and steps
2. If required, create new test suite config in cfg/test-suites with required paths to test templates and ignore lists
3. Run with these configs 
```commandline
urunner2>./runner.sh <your_new_workflow_name> <your_new_test_suite_name> <other parameters if required>
```

## Steps

It's possible to specify steps what should be executed:
```yaml
steps:
    es2panda: # step name, user-defined value
        executable-path: ${parameters.build}/bin/es2panda # the binary to execute
        timeout: ${parameters.es2panda-timeout} # in seconds
        enabled: ${parameters.enable-es2panda} # switches on or off the step execution
        step-type: compiler
        args: # a list of options to transfer to the binary
            - "${parameters.es2panda-full-args}"
            - "${parameters.work-dir}/gen/${test-id}"
```
The values can contain macros detected by `${}`. The macro's name is yaml path to the wished property.

The special macro `${test-id}` refers to the test file name (or path to the test file if some folder hierarchy is used).
It will be expanded during the binary executing.

## ETS ES checked dependencies
- `ruby` (installed by default with `$PROJECT/static_core/scripts/install-deps-ubuntu -i=dev`)
- `node` and `ts-node`, to install them see commands below

```bash
sudo apt-get -y install npm
sudo npm install -g n
sudo n install 22
cd $PROJECT/static_core/tests/tests-u-runner-2/runner/extensions/generators/ets_es_checked/generate-es-checked
npm install
```
