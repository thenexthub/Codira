# ETS micro benchmarks

## Options

| Option | Description | Default value |
| --- | --- | --- |
| `--mode` | Running mode: `int`, `jit`, `aot`. | `int` |
| `--interpreter-type` | Type of interpreter: `cpp`, `irtoc`, `llvm`. | `cpp` |
| `--target` | Launch target: `host`, `device`. | `host` |
| `--log-level` | Logs level: `silence`, `info`, `debug`. | `info` |


| Option | Description|
| --- | --- |
| `--bindir` | Directory with compiled binaries (eg.: ark_asm, ark_aot, ark). Required. |
| `--sourcedir` | Directory with source asm files. Required for arm64. |
| `--libdir` | Directory with etsstdlib. Required for arm64. |
| `--host-builddir` | Directory of panda host build. Required for arm64. |
| `--runtime-options` | Comma separated list of runtime options for ARK. Example: --runtime-options="interpreter-type=irtoc, compiler-enable-jit=true" |
| `--aot-options` | Comma separated list of aot options for ARK_AOT. |
| `--test-name` | Run a specific benchmark by name. |
| `--test-list` | List with benchmarks to be launched. |

## Launch on host

`python3 run_micro_benchmarks.py --bindir ${ARK_BUILD_DIR}/bin`

## Launch on device

1. Load binaries (ark_asm, ark_aot, ark) and libraries (`${ARK_BUILD_DIR}/bin ${ARK_BUILD_DIR}/lib`) to device directory (`${DEVICE_TEST_DIR}`)

2. Load stdlib (`${ARK_BUILD_DIR}/plugins/ets/etsstdlib.abc`) to device library directory (`${DEVICE_TEST_DIR}/lib/`)

3. Load libdwarf.so and libunwind.so to device library directory (`${DEVICE_TEST_DIR}/lib/`)

4. Run benchmarks: `python3 run_micro_benchmarks.py --target device --bindir ${DEVICE_TEST_DIR}/bin --libdir ${DEVICE_TEST_DIR}/lib --host-builddir ${ARK_BUILD_DIR}`