# Src dumper test system


## Testing algorithm:
1) Copy test file to build.
2) Compile with ast dump original file.
3) Compile with src dump original file.
4) Compile with ast dump dumped file.
4) Compare original and dumped asts.


## Artefacts:
You can find dumped src in `<work_dor>/dumped_src`.


## Ignore list:
You can find it in `<ets_frontend>/ets2panda/test/test-lists/srcdumper/srcdumper-ets-ignored.txt`.  

If you add new test and it fails with FailKind.ES2PANDA_FAIL, without a doubt, add it to ignore list in FailKind.ES2PANDA_FAIL section.  

If you add new test and it fails with FailKind.SRC_DUMPER_FAIL, you can fix src dumper or add it to ignore list in FailKind.SRC_DUMPER_FAIL section.  


## How to run:

```bash
# Start runner
bash runner.sh --srcdumper --processes=$(nproc) --build-dir=<build> --work-dir=<build>tools/es2panda/test/unit/src_dumper/dumped_src --es2panda-timeout=120 --timeout=120

# Manually dump src
<build>/bin/es2panda --extension=ets --output=/dev/null --dump-ets-src-after-phases plugins-after-parse --exit-after-phases plugins-after-parse <path_to_test> > <path_to_dumped>

# Manually fast dump ast
<build>/bin/es2panda --extension=ets --output=/dev/null --dump-after-phases plugins-after-parse --exit-after-phases plugins-after-parse <path_to_test> > <path_to_ast>
```


## Notes for someone, who will fix src dumper:

Main goal is to reduce ignore list (FailKind.SRC_DUMPER_FAIL).  

**Important:** You're allowed to change algorithm of `util_srcdumper.py::AstComparator`. I suppose that it is not perfect for now.
