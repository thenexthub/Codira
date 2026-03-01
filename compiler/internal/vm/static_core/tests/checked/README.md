# Checked Tests

Checked tests is the tests that have additional functionality to check result of the test being run.
For example, check some event was raised or some IR instruction is appeared after specific optimization.

Each checker's command should start with special token (`'#!'` for panda assembly language, `//!` for other languages) at the
beginning of the line.

Allowed multiple checkers in a single file. Each checker starts with command `CHECKER` and ends with line without
command token at the zero position.

Each command is a valid code in the `ruby` language.

## List of commands

* **CHECKER** (description: string) begin new Checker with specified description
* **RUN** run panda application, following named arguments are allowed:
    - *options: string* - additional options for Panda VM
    - *entry: string* - entry point, default - `_GLOBAL::main`
    - *result: int* - expected value to be returned by the `panda` application
    - *abort: int* - expected terminal signal
    - *env: string* - environment variables setup string to be used as execution command prefix
    - *force_jit: bool* - run jit compilation for every executed method
    - *force_profiling: bool* - enables profdata collection for every interpreter-executed method
    - *pgo_emit_profdata: bool* - enables on-disk generating for profile data to be used in PGO
* **RUN_PAOC** run paoc application on the compiled panda file. Output panda file will be passed to the following panda
    run. Thus, `RUN_PAOC` command must be placed before `RUN` command. Following options are allowed:
    - *options: string* - additional options for paoc
    - *bool: bool* - enables boot output
    - *result: int* - expected value to be returned by the `paoc`
    - *abort: int* - expected terminal signal
    - *env: string* - environment variables setup string to be used as execution command prefix
    - *inputs: string* - list of panda files to compile
    - *output: string* - name of AOT file to be generated (incompatible to subsequent `RUN` call for now)
    - *pgo_use_profdata: bool* - usese profile data for PGO. Should be used after `RUN` call with `pgo_emit_profdata` enabled
* **RUN_PGO_PROF** - runs panda application with forces profiling and PGO profdata emitting. Output profdata will be passed to the following AOT run that use PGO. Syntax sugar for `RUN` command with `pgo_emit_profdata` enabled
* **RUN_PGO_PAOC** - runs paoc to compile panda file using collected profdata. Should be run after `RUN` with `pgo_emit_profdata` enabled. Syntax sugar for `RUN_PAOC` call
* **RUN_LLVM** - runs paoc application on the compiled panda file with LLVM AOT backend. Requires LLVM support at `paoc` and passing `--with-llvm` to `checker.rb`
* **RUN_AOT** - runs paoc application on the compiled panda file with various AOT modes. Currently, command creates separate checkes for `RUN_PAOC` and `RUN_LLVM` backends
* **RUN_BCO** - runs frontend with bytecode optimizer
* **RUN_FRONTEND** - runs frontend with specified options. If no options specified, `--opt-level=2 --ets-strings-concat=true` is used
* **EVENT** (event: pattern) search event within all events
* **EVENT_NEXT** (event: pattern) ordered search event, i.e. search from position of the last founded event
* **EVENT_NOT** (event: pattern) ensure event is not occurred
* **EVENT_NEXT_NOT** (event: pattern) ensure event is not occurred after current position
* **METHOD** (name: string) start check of specified method, all following checks that require specific method will use method specified by this command
* **PASS_AFTER** (pass_name: string) specify pass after which IR commands should operate
* **PASS_BEFORE** (pass_name: string) select pass that is right before the specified one
* **INST** (inst: pattern) search specified instruction in the ir dump file specified by commands `METHOD` and `PASS_AFTER`
* **INST_NOT** (inst: pattern) equal to `NOT INST`, i.e. check that instruction is not exist
* **INST_NEXT_NOT** (event: pattern) ensure instruction is not occurred after current position
* **IR_COUNT** (inst: string) search specified phrase and counts the number in the ir dump file specified by commands `METHOD` and `PASS_AFTER`, returns the value
* **BLOCK_COUNT** () equal to `IR_COUNT ("BB ")`, i.e. search specified basic blocks and counts the number
* **TRUE** (condition) ensure the condition is correct
* **SKIP_IF** (condition) if condition is `true`, skip all commands from that to end of this checker
* **ASM_METHOD** (name: string) select a specified method in disasm file, next "ASM*" checks will be applied only for this method's code.
* **ASM_INST** (inst: pattern) select a specified instruction in disasm file, next "ASM*" checks will be applied only for this instruction's code.
* **ASM/ASM_NEXT/ASM_NOT/ASM_NEXT_NOT** (inst: pattern) same as other similar checks, but search only in a current disasm scope, defined by `ASM_METHOD` or `ASM_INST`.
If none of these checks were specified, then search will be applied in the whole disasm file.
* **IN_BLOCK** (block: pattern) limits the search for instructions to one block. The block is defined by lines "props: ..." and "succs: ...". The search pattern is found in the first line "props: rest\_of\_line\_for\_matching". You can define only one block for searching and you can't return to the search in the entire method after in the currently analized compiler pass.

*pattern* can be a string(surrounded by quotes) or regex(surrounded by slashes): string - `"SearchPattern"`, regex - `/SearchPattern/`.

To reduce code repetition, a check grouping feature is implemented. Each check group begins with the command `CHECK_GROUP` and ends with a line without a command token at position zero. Inside the group, you list the commands that are common to multiple checkers. Within an individual checker, you then specify the `CHECK_GROUP` command instead of repeating those common commands.

Example of check group usage:
```rb

// Before using CHECK_GROUP: both checkers contain identical checks:

//! CHECKER      IR Builder inlining max int
//! RUN          force_jit: true, options: "--compiler-regex='.*test_i32.*'", entry: "ets_max.ETSGLOBAL::test_i32"
//! METHOD       "ets_max.ETSGLOBAL::test_i32"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining max int
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i32.*'"
//! METHOD        "ets_max.ETSGLOBAL::test_i32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_max.ETSGLOBAL::test_i32"

// After using CHECK_GROUP:

// Common commands are defined once in a check group:
//! CHECK_GROUP             CheckGroup1
//! METHOD       "ets_max.ETSGLOBAL::test_i32"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/

// Individual checkers now reference the group instead of repeating the checks:
//! CHECKER      IR Builder inlining max int
//! RUN          force_jit: true, options: "--compiler-regex='.*test_i32.*'", entry: "ets_max.ETSGLOBAL::test_i32"
//! CHECK_GROUP             CheckGroup1

//! CHECKER       AOT IR Builder inlining max int
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i32.*'"
//! CHECK_GROUP             CheckGroup1
//! RUN           entry: "ets_max.ETSGLOBAL::test_i32"
```

The script `checker.rb` is capable of executing compilation. By default, tests are compiled with the options `--opt-level=2 --ets-strings-concat=true --thread=0 --extension=ets`. If a test needs to be compiled with other options, a frontend definition can be used. Each frontend definition starts with the command `DEFINE_FRONTEND_OPTIONS` and ends with a line without a command token at position zero. Inside the frontend definition, one or more `RUN_FRONTEND` commands should be specified. If a frontend definition is used to compile several sources, the last `RUN_FRONTEND` command should specify the options for the current file. To compile dependency files, the path relative to the current file should be used in the `inputs` option. For the current file itself, the `inputs` option is not needed.

A `RUN_FRONTEND` command can overwrite the default options if specified with the name "DEFAULT ARGS". If some checkers within a test file need to be compiled with other options, a frontend definition with a custom name can be used. In this case, the `DEFINE_FRONTEND_OPTIONS` command must be specified inside the checker block.

Compilation is performed on an "as needed" basis, meaning it is executed when a checker runs. If a frontend definition with a given name has already been compiled, the compilation is skipped, and the previous .abc file is reused.

Example of frontend definition usage with `RUN_FRONTEND`:
```rb
// Changing compilation default options:
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

// Will be compiled with new default options (no DEFINE_FRONTEND_OPTIONS specified):
//! CHECKER     Concat loop with length access unoptimized loop AOT
//! RUN_PAOC    options: "--compiler-regex='.*reuse_concat_loop1' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//! RUN         options: "--compiler-regex='.*reuse_concat_loop1'",  entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

// Custom compilation options
//! DEFINE_FRONTEND_OPTIONS            BCO compiler-simplify-string-builder=false
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

// Will be compiled with custom options (DEFINE_FRONTEND_OPTIONS specified):
//! CHECKER      AOT, StringBuilder getStringLength calls optimized
//! DEFINE_FRONTEND_OPTIONS            BCO compiler-simplify-string-builder=false
//! RUN_PAOC     options: "--compiler-inlining=false --compiler-optimize-string-concat=false  --compiler-simplify-string-builder=true --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount31"
//! PASS_AFTER  "BranchElimination"

// If the test has dependencies, several files should be compiled. The last RUN_FRONTEND should specify the current file:
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false", inputs: "ets_string_builder_merge_uber_export.ets", output: "ets_string_builder_merge_uber_part6_ets_string_builder_merge_uber_export.ets.abc"
//! RUN_FRONTEND            options: "--ets-strings-concat=false"
```

The `RUN_FRONTEND` command can be used inside a checker, and similarly, `RUN_BCO` can be used inside a frontend definition. In this case, the first checker that uses a frontend definition with `RUN_BCO` will generate dump files for testing. Subsequent checkers will be able to reuse the `.abc` file, but not the dump files.

Example of frontend definition usage with `RUN_BCO`:
```rb
// Custom compilation options with --bco-compiler --compiler-dump: 
//! DEFINE_FRONTEND_OPTIONS     RUN_BCO reuse .abc
//! RUN_BCO     options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true --bco-compiler --compiler-optimize-string-concat=true"

// Will be compiled with custom options, generated dump files will be used for testing (the first checker to call DEFINE_FRONTEND_OPTIONS):
//! CHECKER     BCO, StringBuilder length field access BCO optimized
//! DEFINE_FRONTEND_OPTIONS     RUN_BCO reuse .abc
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::stringCalculation00"
//! PASS_AFTER  "BranchElimination"

// Will reuse previously compiled .abc file. No acess to previously generated dump file:
//! CHECKER       INTERP-IRTOC, StringBuilder length field access BCO optimized
//! DEFINE_FRONTEND_OPTIONS     RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*stringCalculation0[0-1]' --compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_stringbuilder_length.ETSGLOBAL::main", result: 0
```
