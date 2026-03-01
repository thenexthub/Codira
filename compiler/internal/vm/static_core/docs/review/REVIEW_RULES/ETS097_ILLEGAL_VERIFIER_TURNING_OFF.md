# Verifier should be always turned on

## Severity
Error

## Applies To
/(.*)$

## Detect

A violation occurs when any file contains `--verification-mode=disabled` statement. Panda assembler shuld be verified

## Example BAD code

```pa
// BAD: verification is disabled
#! RUN          options: "--verification-mode=disabled --interpreter-type=irtoc --compiler-enable-jit=true --compiler-check-final=true --compiler-force-unresolved=true --compiler-hotness-threshold=3 --enable-an --no-async-jit=true", entry: "_GLOBAL::main"
```

```cmake
// BAD: verification is disabled
add_test_file(FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/fail_in_runtime_on_the_fly.pa"
  SKIP_VERIFICATION
  SKIP_AOT SKIP_OSR
  RUNTIME_OPTIONS "${PATCHED_RUNTIME_OPTIONS}" "--verification-mode=disabled"
  RUNTIME_FAIL_TEST
  DEBUG_LOG_MESSAGE "Bad call incompatible parameter"
)
```

## Example GOOD code

```pa
// GOOD: verification mode is not specified at all
#! RUN          options: "--interpreter-type=irtoc --compiler-enable-jit=true --compiler-check-final=true --compiler-force-unresolved=true --compiler-hotness-threshold=3 --enable-an --no-async-jit=true", entry: "_GLOBAL::main"
```

```cmake
// GOOD: Legal verification mode is set
add_test_file(FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/fail_in_runtime_on_the_fly.pa"
  SKIP_VERIFICATION
  SKIP_AOT SKIP_OSR
  RUNTIME_OPTIONS "${PATCHED_RUNTIME_OPTIONS}" "--verification-mode=on-the-fly"
  RUNTIME_FAIL_TEST
  DEBUG_LOG_MESSAGE "Bad call incompatible parameter"
)
```

## Fix suggestion

- **Use `--verification-mode=ahead-of-time`, `--verification-mode=on-the-fly`** instead
- **Do not specify  `--verification-mode` at all**

## Message

Do not disable bytecode verification. Use permited modes or do not specify this option
