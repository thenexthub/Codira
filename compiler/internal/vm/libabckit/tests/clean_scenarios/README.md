AbcKit was also tested on user scenarios from existing apps.

To see the examples of using abckit api for user scenarios - check `c_api` with c api using and `cpp_api` - with cpp api using.

# Testing
## Build abckit and run scenario tests:
```shell
./ark.py x64.release abckit_clean_scenario_tests --gn-args="is_standard_system=true abckit_enable=true"
```

## Run separate test or tests
```shell
cd out/x64.release

LD_LIBRARY_PATH=./arkcompiler/runtime_core/:./arkcompiler/ets_runtime/:./thirdparty/icu/:./thirdparty/zlib/:./test/test \
./tests/unittest/arkcompiler/runtime_core/libabckit/abckit_clean_scenarios --gtest_filter="*Your filter*"
```

Common gtest_filter values:
`"AbckitScenarioCTestClean*"` - run only c api tests
`"AbckitScenarioCppTestClean*"` - run only cpp api tests
`"*<name>"` - run test with name `<name>` from c and ccp apis
`"AbckitScenarioCppTestClean.<name>"` - run test with name `<name>` from ccp apis
`"AbckitScenarioCTestClean.<name>"` - run test with name `<name>` from c apis

You can find the name of test in corresponding .cpp

# Run tests with debug output
If you want to see the debug messages about test:
```shell
export LIBABCKIT_DEBUG_MODE=1
./ark.py x64.debug abckit_clean_scenario_tests --gn-args="is_standard_system=true abckit_enable=true"
```
and run tests from `out/x64.debug`

To turn off the debug messages:
```shell
export LIBABCKIT_DEBUG_MODE=2
```
