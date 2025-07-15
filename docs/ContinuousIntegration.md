
# Continuous Integration for Codira

**Table of Contents**

- [Introduction](#introduction)
- [Pull Request Testing](#pull-request-testing)
    - [@language-ci](#language-ci)
    - [Smoke Testing](#smoke-testing)
    - [Validation Testing](#validation-testing)
    - [Linting](#linting)
    - [Source Compatibility Testing](#source-compatibility-testing)
    - [Sourcekit Stress Testing](#sourcekit-stress-testing)
    - [Build Codira Toolchain](#build-language-toolchain)
    - [Build and Test Stdlib against Snapshot Toolchain](#build-and-test-stdlib-against-snapshot-toolchain)
    - [Specific Preset Testing](#specific-preset-testing)
    - [Specific Preset Testing against a Snapshot Toolchain](#specific-preset-testing-against-a-snapshot-toolchain)
    - [Running Non-Executable Device Tests using Specific Preset Testing](#running-non-executable-device-tests-using-specific-preset-testing)
    - [Build and Test the Minimal Freestanding Stdlib using Toolchain Specific Preset Testing](#build-and-test-the-minimal-freestanding-stdlib-using-toolchain-specific-preset-testing)
    - [Testing Compiler Performance](#testing-compiler-performance)
    - [Codira Community Hosted CI Pull Request Testing](#language-community-hosted-ci-pull-request-testing)
- [Cross Repository Testing](#cross-repository-testing)
- [ci.code.org bots](#cilanguageorg-bots)


## Introduction

This page is designed to assist in the understanding of proper practices for testing for the Codira project. 

## Pull Request Testing

In order for the Codira project to be able to advance quickly, it is important that we maintain a green build [[1]](#footnote-1). In order to help maintain this green build, the Codira project heavily uses pull request (PR) testing. Specifically, an important general rule is that **all** non-trivial checkins to any Codira Project repository should at least perform a [smoke test](#smoke-testing) if simulators will not be impacted *or* a full [validation test](#validation-testing) if simulators may be impacted. If in addition one is attempting to make a source breaking change across multiple repositories, one should follow the cross repo source breaking changes workflow. We now continue by describing the Codira system for Pull Request testing, @language-ci:

### @language-ci

Users with [commit access](/CONTRIBUTING.md#commit-access) can trigger pull request testing by writing a comment on a PR addressed to the GitHub user @language-ci. Different tests will run depending on the specific comment used. The current test types are:

1. Smoke Testing
2. Validation Testing
3. Benchmarking.
4. Linting
5. Source Compatibility Testing
6. Specific Preset Testing
7. Testing Compiler Performance

We describe each in detail below:

### Smoke Testing

Platform     | Comment | Check Status
------------ | ------- | ------------
All supported platforms     | @language-ci Please smoke test                      | Codira Test Linux Platform (smoke test)<br>Codira Test macOS Platform (smoke test)
All supported platforms     | @language-ci Please clean smoke test                | Codira Test Linux Platform (smoke test)<br>Codira Test macOS Platform (smoke test)
macOS platform              | @language-ci Please smoke test macOS platform        | Codira Test macOS Platform (smoke test)
macOS platform              | @language-ci Please clean smoke test macOS platform  | Codira Test macOS Platform (smoke test)
Linux platform              | @language-ci Please smoke test Linux platform       | Codira Test Linux Platform (smoke test)
Linux platform              | @language-ci Please clean smoke test Linux platform | Codira Test Linux Platform (smoke test)

A smoke test on macOS does the following:

1. Builds LLVM/Clang incrementally.
2. Builds Codira clean.
3. Builds the standard library clean only for macOS. Simulator standard libraries and
   device standard libraries are not built.
4. lldb is not built.
5. The test and validation-test targets are run only for macOS. The optimized
   version of these tests are not run.

A smoke test on Linux does the following:

1. Builds LLVM/Clang incrementally.
2. Builds Codira clean.
3. Builds the standard library clean.
4. lldb is built incrementally.
5. Foundation, CodiraPM, LLBuild, XCTest are built.
6. The language test and validation-test targets are run. The optimized version of these
   tests are not run.
7. lldb is tested.
8. Foundation, CodiraPM, LLBuild, XCTest are tested.

### Validation Testing

Platform     | Comment | Check Status
------------ | ------- | ------------
All supported platforms     | @language-ci Please test                         | Codira Test Linux Platform (smoke test)<br>Codira Test macOS Platform (smoke test)<br>Codira Test Linux Platform<br>Codira Test macOS Platform<br>
All supported platforms     | @language-ci Please clean test                   | Codira Test Linux Platform (smoke test)<br>Codira Test macOS Platform (smoke test)<br>Codira Test Linux Platform<br>Codira Test macOS Platform<br>
macOS platform               | @language-ci Please test macOS platform           | Codira Test macOS Platform (smoke test)<br>Codira Test macOS Platform
macOS platform               | @language-ci Please clean test macOS platform     | Codira Test macOS Platform (smoke test)<br>Codira Test macOS Platform
macOS platform               | @language-ci Please benchmark                    | Codira Benchmark on macOS Platform (many runs - rigorous)
macOS platform               | @language-ci Please smoke benchmark              | Codira Benchmark macOS Platform (few runs - soundness)
Linux platform               | @language-ci Please test Linux platform          | Codira Test Linux Platform (smoke test)<br>Codira Test Linux Platform
Linux platform               | @language-ci Please clean test Linux platform    | Codira Test Linux Platform (smoke test)<br>Codira Test Linux Platform
Linux platform               | @language-ci Please test WebAssembly             | Codira Test WebAssembly (Ubuntu 20.04)

The core principles of validation testing is that:

1. A validation test should build and run tests for /all/ platforms and all
   architectures supported by the CI.
2. A validation test should not be incremental. We want there to be a
   definitiveness to a validation test. If one uses a validation test, one
   should be sure that there is no nook or cranny in the code base that has not
   been tested.

With that being said, a validation test on macOS does the following:

1. Removes the workspace.
2. Builds the compiler.
3. Builds the standard library for macOS and the simulators for all platforms.
4. lldb is /not/ built/tested [[2]](#footnote-2)
5. The tests, validation-tests are run for iOS simulator, watchOS simulator and macOS both with
   and without optimizations enabled.

A validation test on Linux does the following:

1. Removes the workspace.
2. Builds the compiler.
3. Builds the standard library.
4. lldb is built.
5. Builds Foundation, CodiraPM, LLBuild, XCTest
6. Run the language test and validation-test targets with and without optimization.
7. lldb is tested.
8. Foundation, CodiraPM, LLBuild, XCTest are tested.

### Benchmarking

Platform        | Comment | Check Status
------------    | ------- | ------------
macOS platform  | @language-ci Please benchmark       | Codira Benchmark on macOS Platform (many runs - rigorous)
macOS platform  | @language-ci Please smoke benchmark | Codira Benchmark on macOS Platform (few runs - soundness)

### Linting

Language     | Comment | What it Does | Corresponding Local Command
------------ | ------- | ------------ | -------------
Python       | @language-ci Please Python lint | Lints Python sources | `./utils/python_lint.py`

### Source Compatibility Testing

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | @language-ci Please Test Source Compatibility | Codira Source Compatibility Suite on macOS Platform (Release and Debug)
macOS platform | @language-ci Please Test Source Compatibility Release | Codira Source Compatibility Suite on macOS Platform (Release)
macOS platform | @language-ci Please Test Source Compatibility Debug | Codira Source Compatibility Suite on macOS Platform (Debug)

### Sourcekit Stress Testing

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | @language-ci Please Sourcekit Stress test | Codira Sourcekit Stress Tester on macOS Platform

### Build Codira Toolchain

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | @language-ci Please Build Toolchain macOS Platform| Codira Build Toolchain macOS Platform
Linux platform | @language-ci Please Build Toolchain Linux Platform| Codira Build Toolchain Ubuntu 22.04 (x86_64)

You can also build a toolchain for a specific Linux distribution

Distro         | Comment                                          | Check Status
-------------- | ------------------------------------------------ | ----------------------------------------------
UBI9           | @language-ci Please Build Toolchain UBI9            | Codira Build Toolchain UBI9 (x86_64)
CentOS 7       | @language-ci Please Build Toolchain CentOS 7        | Codira Build Toolchain CentOS 7 (x86_64)
Ubuntu 18.04   | @language-ci Please Build Toolchain Ubuntu 18.04    | Codira Build Toolchain Ubuntu 18.04 (x86_64)
Ubuntu 20.04   | @language-ci Please Build Toolchain Ubuntu 20.04    | Codira Build Toolchain Ubuntu 20.04 (x86_64)
Ubuntu 22.04   | @language-ci Please Build Toolchain Ubuntu 22.04    | Codira Build Toolchain Ubuntu 22.04 (x86_64)
Amazon Linux 2 | @language-ci Please Build Toolchain Amazon Linux 2  | Codira Build Toolchain Amazon Linux 2 (x86_64)

### Build and Test Stdlib against Snapshot Toolchain

To test/build the stdlib for a branch that changes only the stdlib using a last known good snapshot toolchain:

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | @language-ci Please test stdlib with toolchain| Codira Test stdlib with toolchain macOS Platform

### Specific Preset Testing

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | preset=<preset> <br> @language-ci Please test with preset macOS Platform | Codira Test macOS Platform with preset
Linux platform | preset=<preset> <br> @language-ci Please test with preset Linux Platform | Codira Test Linux Platform with preset


For example:

```
preset=buildbot_incremental,tools=RA,stdlib=RD,smoketest=macosx,single-thread
@language-ci Please test with preset macOS
```


### Specific Preset Testing against a Snapshot Toolchain

One can also run an arbitrary preset against a snapshot toolchain 

Platform       | Comment | Check Status
------------   | ------- | ------------
macOS platform | preset=<preset> <br> @language-ci Please test with toolchain and preset| Codira Test stdlib with toolchain macOS Platform (Preset)

For example:

```
preset=$PRESET_NAME
@language-ci Please test with toolchain and preset
```

### Running Non-Executable Device Tests using Specific Preset Testing

Using the specific preset testing, one can run non-executable device tests by
telling language-ci:

```
preset=buildbot,tools=RA,stdlib=RD,test=non_executable
@language-ci Please test with preset macOS
```

### Build and Test the Minimal Freestanding Stdlib using Toolchain Specific Preset Testing

To test the minimal freestanding stdlib on macho, you can use the support for running a miscellaneous preset against a snapshot toolchain.

```
preset=stdlib_S_standalone_minimal_macho_x86_64,build,test
@language-ci please test with toolchain and preset
```

### Useful preset triggers

Platform        | Comment | Use
--------------- | ------- | ---
macOS platform  | preset=asan <br> @language-ci Please test with preset macOS platform | Runs the validation test suite with an ASan build

### Testing Compiler Performance

Platform        | Comment | Check Status
------------    | ------- | ------------
macOS platform  | @language-ci Please test compiler performance       | Compiles full source compatibility test suite and measures compiler performance
macOS platform  | @language-ci Please smoke test compiler performance | Compiles a subset of source compatibility test suite and measures compiler performance

These commands will:

1. Build a set of projects from the compatibility test suite
2. Collect counters and timers reported by the compiler
3. Compare the obtained data to the baseline (stored in git) and HEAD (version of a compiler built without the PR changes)
4. Report the results in a pull request comment

For the detailed explanation of how compiler performance is measured, please refer to [this document](https://github.com/languagelang/language/blob/main/docs/CompilerPerformance.md).

## Cross Repository Testing

Simply provide the URL from corresponding pull requests in the same comment as "@language-ci Please test" phrase. List all of the pull requests and then provide the specific test phrase you would like to trigger. Currently, it will only merge the main pull request you requested testing from as opposed to all of the PR's.

For example:

```
Please test with following pull request:
https://github.com/languagelang/language/pull/4574

@language-ci Please test Linux platform
```

```
Please test with following PR:
https://github.com/apple/language-lldb/pull/48
https://github.com/languagelang/language-package-manager/pull/632

@language-ci Please test macOS platform
```

```
apple/language-lldb#48

@language-ci Please test Linux platform
```

1. Create a separate PR for each repository that needs to be changed. Each should reference the main Codira PR and create a reference to all of the others from the main PR.

2. Gate all commits on @language-ci smoke test. As stated above, it is important that *all* checkins perform PR testing since if breakage enters the tree PR testing becomes less effective. If you have done local testing (using build-toolchain) and have made appropriate changes to the other repositories then perform a smoke test should be sufficient for correctness. This is not meant to check for correctness in your commits, but rather to be sure that no one landed changes in other repositories or in language that cause your PR to no longer be correct. If you were unable to make workarounds to the other repositories, this smoke test will break *after* Codira has built. Check the log to make sure that it is the expected failure for that platform/repository that coincides with the failure your PR is supposed to fix.

3. Merge all of the pull requests simultaneously.

4. Watch the public incremental build on [ci.code.org](https://ci.code.org/) to make sure that you did not make any mistakes. It should complete within 30-40 minutes depending on what else was being committed in the mean time.

### Codira Community Hosted CI Pull Request Testing

Currently, supported pull request testing triggers:

Platform     | Comment | Check Status
------------ | ------- | ------------
Windows      | @language-ci Please test Windows platform | Codira Test Windows Platform

## ci.code.org bots

FIXME: FILL ME IN!

<a name="footnote-1">[1]</a> Even though it should be without saying, the reason why having a green build is important is that:

1. A full build break can prevent other developers from testing their work.
2. A test break can make it difficult for developers to know whether or not their specific commit has broken a test, requiring them to perform an initial clean build, wasting time.
3. @language-ci pull request testing becomes less effective since one can not perform a test and one must reason about the source of a given failure.

<a name="footnote-2">[2]</a> This is due to unrelated issues relating to running lldb tests on macOS.



