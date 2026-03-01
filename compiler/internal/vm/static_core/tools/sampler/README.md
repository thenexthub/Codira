# Sampler usage

## Generate flamegraph

Download `flamegraph`:

```bash
cd ${BUILD_DIR}
git clone https://github.com/brendangregg/FlameGraph.git
```

```bash
# get abc bin
${BUILD_DIR}/bin/es2panda ${BUILD_SOURCE}/plugins/ets/tests/runtime/tooling/sampler/SamplerTest.ets ${BUILD_DIR}/sampling_app.abc

# get sample dump
${BUILD_DIR}/bin/ark --load-runtimes=ets --boot-panda-files=${BUILD_DIR}/plugins/ets/etsstdlib.abc --sampling-profiler-create --sampling-profiler-startup-run --sampling-profiler-interval=200 --sampling-profiler-output-file=${BUILD_DIR}/outfile.aspt ${BUILD_DIR}/sampling_app.abc ETSGLOBAL::main

# convert sample dump to csv
${BUILD_DIR}/bin/aspt_converter --input=${BUILD_DIR}/outfile.aspt --output=${BUILD_DIR}/traceout.csv

# generate flamegrath svg
${BUILD_DIR}/FlameGraph/flamegraph.pl ${BUILD_DIR}/traceout.csv > ${BUILD_DIR}/out.svg
```

If option `--sampling-profiler-collect-stats` passed on ark launch it creates file at the end of vm's life with information about quantity of sent signal to every thread and created samples.
    Signals can be ignored if threads are suspended to wait for cpu time(major part of ignored signals), or mutator threads are not executing bytecode (not started or finished).

## AsptConverter parameters

|           Parameter             |          Possible values          |                        Description                          |
| ------------------------------- | -------------------------------   | ----------------------------------------------------------- |
| --csv-tid-separation            | single-csv-single-tid             | Doesn't distinguish threads, dump samples in single csv     |
|                                 | single-csv-multi-tid (by default) | Distinguish threads by thread_id in a single csv file       |
|                                 | multi-csv                         | Distinguish threads by creating csv files for each thread   |
| --cold-graph-enable             | true/false (by default: false)    | Add information about thread status (running/suspended)     |
| --substitute-module-dir         | true/false (by default: false)    | Enable substitution of panda files directories, e.g. run converter on host for .aspt got from device |
| --substitute-source-str         | {dir1}, {dir2}                    | Substring that will be replaced with substitude-destination |
| --substitute-destination-str    | {dir_target1}, {dir_target2}      | Substring that will be places instead of substitude-source  |
| --dump-modules                  | true/false (by default: false)    | In this mode converter only dump modules paths to outfile   |
| --dump-system-frames            | true/false (by default: false)    | Add system frame in dump                                    |

Note: In substitution parameters (source and destination str) number of strings should be equal and i-th string from source changes only to i-th from destination.

## Limitations
    1. Samples can be lost but not necessarily in case if signal was sent to interpreter thread while it was in the bridge and frame was not constructed yet. (Relevant to profiling with JIT or ArkTS NAPI)
        1.1 You can track lost samples by adding options to runtime `--log-level=debug --log-components=profiler`

    2. Tests are not running with TSAN enabled because TSAN can affect user signals and rarely it causes incorrect sampler work.
