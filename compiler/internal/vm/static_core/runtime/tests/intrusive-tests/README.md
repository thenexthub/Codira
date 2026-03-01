Intrusive testing
=================

The basic idea is to have a possibility to find and to reproduce rare multithreading bugs, for example, data races.
We may increase a chance to catch such bugs by controlling the order of synchronization events between different threads.
The intrusive test relies on a special comments in a source code introducing control points where different threads are synchronized.
Each intrusive test contains a map from control points to the testing code which controls the sequence of execution by waiting and signaling on test events.

Each intrusive test consists of the following elements:
- specification of synchronization points in ARK source code;
- implementing synchronization points, which are source code modules with parameterized functions, performing synchronization actions on the base of synchronization point context and state of the test;
- mapping of synchronization points - yaml module mapping synchronization points specifications to its implementations.

Prerequisites for using intrusive tests:
- python 3.7;
- libclang 14.0.1;
- python modules: clang, libclang, clade 3.5.1.

How to create a new intrusive test
----------------------------------

Each intrusive test should have the following identifiers:
- <INTRISIVE_TEST_NAME>, which represents index of this test (e.g., `CLASS_GET_BASE_INTRUSIVE_TEST`). This name is used in enum.
- <intrusive_test_name>, which is the name of a directory with this test (e.g., `class_get_base_intrusive_test`).

Let us consider, that directory with intrusive test set is `<INTRUSIVE_TESTS_RELATIVE_DIR>=runtime/tests/intrusive-tests`

1. Add synchronization points in the source code in the following format:
```c++
/* @sync <num>
 * @description ...
 */
```
`<num>` is an index of the given synchronization point in current file.

2. Add new test identifier (`<INTRISIVE_TEST_NAME>`) in enum in `<INTRUSIVE_TESTS_RELATIVE_DIR>/intrusive_test_suite.h`.

3. Create directory for intrusive test `<INTRUSIVE_TESTS_RELATIVE_DIR>/<intrusive_test_name>` and add there:
- `bindings.yml`: mapping of synchronization code onto synchronization points in the following format:
```yaml
declaration: sync_api.h
mapping:
-
  file: <file name>
  class: <class name>
  method: <method name>
  index: <num>
  code: |
      <synchronization code, e.g. synchronization function calls>
-
...
```
- `sync_api.h` - declaration of synchronization functions, that usually implement wait/notify logic.
They should be placed in a specific class as static methods to prevent name collisions, for example:
```c++
class IntrusiveTestNameAPI {
public:
    static void WaitForEvent(void);
    static void NotifyOnEvent(void);
};
```
- `sync_api.cpp` – implementation of synchronization functions.
- source code of intrusive test (may be based on original test). This code should set test identifier with `SetTestId(<INTRISIVE_TEST_NAME>)`. One way to do this is:
```c++
RuntimeOptions options;
...
options.SetIntrusiveTest(<INTRISIVE_TEST_NAME>);
Runtime::Create(options);
```
- `CMakeLists.txt` – describes how to build of the test. Example:
```cmake
if (IS_RUNTIME_INTRUSIVE_BUILD)
  target_sources(arkrintime_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sync_api.cpp)
else()
  # sub-targets for building/running of the test
  add_dependencies(intrusive_test <building target>)
  add_dependencies(intrusive_test_run <running target>)
endif()
```

4. Add new targets in `<INTRUSIVE_TESTS_RELATIVE_DIR>/CMakeLists.txt`:
```cmake
add_subdirectory(<intrusive_test_name>);
```

5. Launch the test
```shell
local_intrusive_testing.sh [-cr] <path_to_repo> <path_to_build_dir>
```
This script copies source code to <path_to_build_dir>, performs instrumentation there, and then launches intrusive test set.
Use `-c` flag to clear existing build directory `<path_to_build_dir>` and `-r` to skip instrumentation stage and only run tests.
If you need to add a file with TSAN suppressions for the tests, set up environment variable TSAN_SUPPRESSIONS.

An example of intrusive test is located in `runtime/tests/intrusive-tests/clear_interrupted_intrusive_test` directory.
This test checks a data race that would happen if we remove the lock in `ClearInterrupted()` method.
Existing test `runtime/tests/intrusive-tests/clear_interrupted_intrusive_test/clear_interrupted_intrusive_test.cpp` checking `ClearInterrupted()` method, rarely finds the described data race, because one of two conflicting threads often finishes before `ClearInterrupted()` method is even called.
Intrusive testing method helps TSAN data race checker to see this data race by organizing a thread interleaving on which the data race can be detected by TSan. This intrusive test (`runtime/tests/intrusive-tests/clear_interrupted_intrusive_test`) illustrates intrusive testing on example and can be used as a basis or starting point for new tests development.
