# How to build es2panda

This document is valid at the time of writing (12.10.2022) and gives steps to build `es2panda` with clean Ubuntu 22.04 LTS.

## Try bootstrap script first

Make sure your Git is set up with Gitee account

```
git config --global user.name "<gitee_user>"
git config --global user.email "<your_email@domain.com>"
```

For bootstrapping steps see [bootstraping](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/README.md#bootstrapping).

### Clone repositories

```
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_runtime_core.git panda
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_ets_runtime.git panda/plugins/ecmascript
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_ets_frontend.git panda/tools/es2panda
git clone https://gitee.com/nsizov/ets panda/plugins/ets
```

Follow **Build `es2panda`** and **Running** sections.

## Another method

Below are steps to build `es2panda` if and when bootstrap scripts will fail.

### Update your system

```
sudo apt update
```

### Install Git

```
sudo apt install git
```

Setup Git with your Gitee account

```
git config --global user.name "<gitee_user>"
git config --global user.email "<your_email@domain.com>"
```

### Clone repositories

```
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_runtime_core.git panda
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_ets_runtime.git panda/plugins/ecmascript
git clone --branch ets-stdlib-dev https://gitee.com/aakmaev/arkcompiler_ets_frontend.git panda/tools/es2panda
git clone https://gitee.com/nsizov/ets panda/plugins/ets
```

### Install required software

Install Curl, Make, Ninja, Ruby, CMake

```
sudo apt install curl make ninja-build ruby cmake
```

Install `g++` compiler, cross-compiler and debugger

```
sudo apt install g++ gdb
sudo apt install g++-arm-linux-gnueabi
```

Install DWARF and Z libraries

```
sudo apt install libdwarf-dev zlib1g-dev
```

### Install third-party libraries

Enter Panda folder
```
cd panda
```

Edit file `scripts/third-party-lists/public` and change line for vixl, change from https://git.linaro.org/arm/vixl.git to https://github.com/Linaro/vixl.git

Run

```
sudo ./scripts/install-third-party --force-clone
```

## Build `es2panda`

Make `build` folder

```
mkdir build
cd build
```

Run CMake

```
cmake .. -GNinja -DCMAKE_BUILD_TYPE=DEBUG
```

Build

```
ninja -j 8 ark ark_aot es2panda

cd ..
```

If nothing failed you have ready to work environment.

## Running

### Build eTS stdlib first

```
./bin/es2panda [--arktsconfig path/to/your/arktsconfig.json] --gen-stdlib=true --extension=ets --output=etsstdlib.abc --opt-level=0
```

File `etsstdlib.abc` should be generated.

### Build dummy file and run it

Feed file `x.ets` with code:

```
function main(): void {
	assert true;
}
```
and compile it

```
./bin/es2panda [--arktsconfig path/to/your/arktsconfig.json] --extension=ets --output=x.abc --opt-level=0 x.ets
```

Now run things out:

```
./bin/ark --boot-panda-files=etsstdlib.abc --load-runtimes=ets x.abc ETSGLOBAL::main
```
It should not fail.

Congratulations, you have working environment with `es2panda` and `ark`
