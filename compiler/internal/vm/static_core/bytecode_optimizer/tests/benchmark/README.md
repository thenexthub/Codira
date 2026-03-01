## Benchmark of Bytecode

This folder contains scripts and documentaions for benchmarking the size
of the bytecode.


## `run_benchmark.py`

### Help

```sh
usage: Run bytecode benchmarks [-h] [--testdir TESTDIR] [--bindir BINDIR]
                               [--input-type {class,pa}] [--compiler-options LIST]
                               [--compiler-options-file FILE] [--json FILE] [--verbose]

optional arguments:
  -h, --help            show this help message and exit
  --testdir TESTDIR     Directory with tests (*.class or *.pa). Default:
                        './bytecode_optimizer/tests/benchmark/suite'
  --bindir BINDIR       Directory with compiled binaries (eg.: c2p). Default: './build/bin'
  --input-type {class,pa}
                        Type of the test input. Default: 'class'
  --compiler-options LIST
                        Comma separated list of compiler options for C2P (see 'build/bin/c2p
                        --help' for details
  --compiler-options-file FILE
                        Input file containing compiler options for C2P (see 'build/bin/c2p
                        --help' for details
  --json FILE           JSON dump file name
  --verbose, -v         Enable verbose messages
```

### How to use

```sh
bytecode_optimizer/tests/benchmark/run_benchmark.py --testdir=../benchmark-input --compiler-options-file=compiler.config --json=benchmark-results.json -v
```

where `compiler.config` contains a list of compiler options one per line, e.g.:

```sh
--compiler-lowering=true
--compiler-inlining=true
--compiler-lowering=true
--compiler-loop-peeling=true
--compiler-lse=true
--compiler-loop-unroll=true
```

### Example output

```
Average sizes (in bytes):
     annotation_item section: 34
      class_idx_item section: 32
          class_item section: 223
           code_item section: 158
     debug_info_item section: 25
        foreign_item section: 226
         header_item section: 40
          proto_item section: 61
         string_item section: 239
                       total: 1042

Minimum sizes (in bytes):
     annotation_item section: 16
      class_idx_item section: 24
          class_item section: 143
           code_item section: 11
     debug_info_item section: 7
        foreign_item section: 144
         header_item section: 40
          proto_item section: 48
         string_item section: 149
                       total: 713

Maximum sizes (in bytes):
     annotation_item section: 72
      class_idx_item section: 40
          class_item section: 282
           code_item section: 264
     debug_info_item section: 43
        foreign_item section: 307
         header_item section: 40
          proto_item section: 68
         string_item section: 347
                       total: 1333
Summary:
========
  Tests : 10
  Passed: 7
  Failed: 3
```

### JSON example
```
{
    "StrictMath$RandomNumberGeneratorHolder.class": {
        "annotation_item section": 43,
        "class_idx_item section": 24,
        "class_item section": 141,
        "code_item section": 31,
        "debug_info_item section": 12,
        "foreign_item section": 158,
        "header_item section": 40,
        "line_number_program_item section": 8,
        "proto_item section": 16,
        "string_item section": 171,
        "total": 644
    },
    "HttpDate.class": {
        "error": "c2p: /panda/bytecode_optimizer/inst_builder.cpp:128:
        void ark::bytecodeopt::InstBuilder::AddCatchPhi():
        Assertion `catch_begin->IsCatchBegin()' failed.\n"
    },
    "SSLContextSpi.class": {
        "error": "Unsupported instruction in alias analysis: 7.ref CatchPhi v10, v12, v12 -> (v19, v19, v18, v17, v17, v17)
        c2p: /panda/compiler/optimizer/analysis/alias_analysis.cpp:395:
        virtual void ark::compiler::AliasVisitor::VisitDefault(ark::compiler::Inst*):
        Assertion inst->GetType() != DataType::REFERENCE && cond_val' failed."
    }
}
```
## `compare.py`

This script can be used to compare the results of the `run_becnhmark.py` script.

### Help

```sh
usage: Compare benchmark results [-h] --old JSON_FILE_PATH --new JSON_FILE_PATH --failed JSON_FILE_PATH

optional arguments:
  -h, --help            show this help message and exit
  --old JSON_FILE_PATH  Base or reference benchmark result
  --new JSON_FILE_PATH  Benchmark result to be compared with the reference
  --failed JSON_FILE_PATH
                        File to log error messages from c2p
```

### How to use

```sh
bytecode_optimizer/tests/benchmark/compare.py --old=reference.json --new=new.json --failed=error.json
```

### Example output

```
Classes that have been optimized:
  Code_item section size:
|Old: |New: |Diff:|Per:  |File:
| 1045| 1037|    8| 0.77%| ICULocaleService$LocaleKey.class
|  302|  294|    8| 2.65%| Locale$LocaleKey.class
| 5700| 5693|    7| 0.12%| LanguageTag.class
|  957|  949|    8| 0.84%| LinkedList$ListItr.class
|  780|  772|    8| 1.03%| FieldPosition.class

Summary:
=============
 Total code_item section size of baseline files: 9201 bytes
 Total code_item section size of compared files: 9162 bytes
 Difference: 39 bytes [0.42%]
 Number of optimized files: 5
 Number of not optimized files : 1
 Files with no code item section: 0
 Files that are bigger than baseline: 0
 Failed tests on baseline: 1
 Failed tests compared to baseline: 0
=============

Statistics on optimized files:
=============
 Total code_item section size of baseline files: 8784 bytes
 Total code_item section size of compared files: 8745 bytes
 Difference: 39 bytes [0.44%]
=============
```
