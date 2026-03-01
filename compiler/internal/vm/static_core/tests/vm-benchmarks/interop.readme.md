# Developing and running interop benchmarks in VMB

Note: This approach is outdated. For actual docs see [this manual](./examples/benchmarks/interop/readme.md)

## Preparing project

- ensure your runtime core and ets frontend are on latest version
- follow the regular steps to cmake the project:
    - create a symbolic link from ets_frontend/ets2panda at runtime_core/static_core/tools/es2panda (note that e**t**s2panda points to es2panda) with `ln -s $(realpath ./ets_frontend/ets2panda) $(realpath ./runtime_core/static_core/tools/es2panda)`
    - create `runtime_core/static_core/build` directory 
    - run `cmake .. -GNinja -DCMAKE_BUILD_TYPE=debug -DCMAKE_EXPORTCOMPILECOMMANDS=ON -DPANDA_ETS_INTEROP_JS=1` from `runtime_core/static_core/build`
- build binaries required by VMB:
    `ninja es2panda ark ark_aot etsstdlib ets_interop_js_napi`

- (optionally) build VMB via install-deps script to run it natively.
```
sudo ./static_core/scripts/install-deps-ubuntu --install=vmb
```

## Developing interop benchmarks

### File structure and location

All the interop benchmarks will be located in `runtime_core/static_core/plugins/ets/tests/benchmarks`. Example tests to be used as boilerplates can be found in `runtime_core/static_core/tests/vm-benchmarks/examples/benchmarks-interop-freestyle` 

Within this folder, every benchmark suite is put into a subdirectory with a related name. This folder contains named separate benchmarks, or bench units - also in subdirectories, prefixed with "bu_". 

Within the bench unit, VMB searches for files matching the pattern "bench_%benchUnitName%.(abc|zip|js|ets)", in this exact order.

- if **abc | zip** files are found, VMB will assume these are precompiled binaries (or archives of such). Therefore, source compilation will be skipped, and the binaries will be executed as is.
- **js** overrides default interop runner. Since that, it can be used BOTH for benchmarking pure javascript code AND custom interop logic.
- If only **ets** files are found matching the pattern, they will compiled and executed using the default runner.  

### Naming convention

A benchmark suite should contain four bench units: dynamic runtime (pure JS), static runtime (pure ArkTS), static invokes dynamic (ArkTS invokes JS), dynamic invokes static (JS invokes ArkTS). To distinguish those within a suite, bench units should be suffixed with `d2d`, `s2s`, `s2d`, or `d2s`, respectively. Letter `d` here stands for _dynamic_ and `s` for _static_. First letter represents the runtime invoking a certain feature and (optionally) handling the invocation outcome, while the latter indicates a runtime the feature is executed in.

### Sample structure

```
*
└── feature_name
    ├── bu_feature_name_s2d
    │   ├── bench_a2j.ets
    │   └── test_import.js
    ├── bu_feature_name_d2s
    │   ├── bench_a2j.ets
    │   └── bench_a2j.js
    ├── bu_feature_name_d2d
    │   └── bench_j2j.js
    └── bu_feature_name_s2s
        └── bench_j2a.ets

```


### Code structure

### General tips:
- VMB reads stdout for benchmark unit results. You'll have to console.log a certain pattern there to make results received and parsed properly: `Benchmark result: %YOUR_BENCHMARK_UNIT_NAME% %UNIT_TIMING%`
- When using custom functions to time an iteration, please ensure timing calls are placed as close to the test calls as humanly possible.
- `arktsconfig.json` is generated when a benchmark is ran. All JS files in a benchmark unit are automatically added to this JSON with path names as file name minus extension (e.g. `test_import.js` will be accessible as `test_import`)
- use `Chrono.nanoNow` from `std/time` package in ARK, or `process.hrtime` in JS to provide better precision. 

### *.ets:

ETS benchmark file should export a class with a name matching the overall bench unit name, but in pascal case (camelcase with a capital first letter). E.g., for a bench unit `call_async_function` a class `CallAsyncFunction` would be expected, while `SHA256` benchmark will need a class also called `SHA256`.

####  The class **must** expose these public methods:

- `setup(): void`

This method is called once before the benchmark is ran and can be used either for warmup or preparing whatever is needed.


- `test(): void`

This method is called every benchmark iteration and basically IS the iteration.

#### The class **may also** expose these public properties or methods

- `runsLeft: number` (or an exposed setter for it)
- `totalTime: number` (or an exposed getter)

These properties indicate that VMB would expect iterations to be timed in this particular class (opposed to timing them externally). Since that, **please ensure every `test()` call is somehow timed, and iteration time is added to totalTime property**

### *.js

JS benchmark file is relatively free in structure, except for these two limitations:

- VMB can not call a particular method inside a JS script by itself, so your code should be invoked on module load. E.g.:
```
function main() {
    // RUN_SOME_CODE_HERE
}
main()
``` 


## Running benchmarks

```
export PANDA_BUILD=$(realpath %PATH TO YOUR static_core/build%)
vmb run -p arkts_node_interop_host -v debug `pwd`/%PATH YO YOUR BENCHMARKS%
```

Notes:
- if you didn't install vmb as a CLI app, you can use it via `./static_core/tests/vm-benchmarks/run-vmb.sh` with the same syntax
