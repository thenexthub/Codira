
# Starting VMB on Windows host (Experimental)

### Build and Install Python Module

```shell
# 1) build package or download it from CI builds and skip to step 2
# inside this source directory
py -m pip install build
py -m build
py -m pip uninstall -y vmb  # optionally
# 2) install
py -m pip install .\dist\vmb-*-py3-none-any.whl
# 3) check if it works
vmb version
```

Alternative variant is running vmb as script (without installation):
```shell
# 1) clone this repo and cd to src root
cd static_core\tests\vm-benchmarks\src
# 2) set path to module
set PYTHONPATH=<full-path-to-repo>\static_core\tests\vm-benchmarks\src\vmb
# 3) run vmb
py -m vmb all -p node_host .\examples\benchmarks\ts\VoidBench.ts
```

### Node Host platform

```shell
vmb all -p node_host <full-path-to-repo>\static_core\tests\vm-benchmarks\examples\benchmarks\ts\VoidBench.ts
```

### ArkJS on OHOS device platform

```shell
# (optionally) Set env var pointing to hdc binary if it is not in the PATH
$Env:HDC = "C:\path\to\hdc.exe"
# or add it to PATH
$env:Path += ";C:\path\to"

vmb all -p ark_js_vm_ohos [-v debug] \
    [--device=DEVSERIAL] [--device-host=DEVICE_HOST] [--device-dir=DEVICE_DIR] \
    --es2abc-path=C:\hu\workload\1.1\tools\windows\es2abc.exe \
    --ark_js_vm-path=d:\hu\vm-exec-stub.bat .\examples\benchmarks\ts\VoidBench.ts
```

### ArkTs on OHOS device platform

```shell
vmb all --platform=arkts_ohos [ --log-level=debug ] \
    --src-langs=ets --langs=ets \
    --es2panda-path=C:\static\build-tools\es2panda\bin\es2panda.exe \
    [ --device-dir=/data/local/tmp/my ] \
    --device-host=10.1.1.1:11111 \
    .\benchmarks\ets
```
