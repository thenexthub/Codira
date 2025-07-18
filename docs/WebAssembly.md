# WebAssembly support in Codira

WebAssembly is a platform that significantly differs from hardware platforms that Codira already supports.
While it's a virtual machine, there are considerations to be taken into account when targeting it:

* WebAssembly is still at an early stage, so many features you'd be expect from other platforms are not
available yet, specifically:
  1. `wasm64` variant is not specified yet, only the 32-bit `wasm32` variant is supported in WebAssembly
  hosts such as browsers.
  2. While a preview of multi-threading and atomics is available in some browsers and stand-alone 
  WebAssembly hosts, [the corresponding proposal](https://github.com/WebAssembly/threads/) haven't 
  formally reached the implementation phase yet.
    The multi-threading feature is available in `wasm32-unknown-wasip1-threads` target, but it's not
    in `wasm32-unknown-wasi` target.
  3. Dynamic linking is not formally specified and tooling for it is not available yet.
* Binary size is a high priority requirement. Since WebAssembly payloads are usually served in browsers,
one wouldn't want end users to download multi-megabyte binaries.

## Building Codira SDK for WebAssembly

The [Codira SDK](https://github.com/languagelang/language-evolution/blob/main/proposals/0387-cross-compilation-destinations.md)
for WebAssembly is built using the following command:

```bash
./utils/build-script --build-wasm-stdlib
```

This command will build the Codira compiler for the host platform and then build the Codira standard library
for WebAssembly targets. The resulting Codira SDK `.artifactbundle` will be placed in the `../language-sdk-generator/Bundles`
directory.

## Building Codira SDK for WebAssembly without building the compiler

Building the Codira compiler is a time-consuming process. If you only want to build the Codira standard library
with pre-built Codira compiler, you can use the following command:

```console
$ LANGUAGE_TOOLS_PATH=path/to/language-development-snapshot/usr/bin
$ ./utils/build-script \
    --skip-build-toolchain \
    --skip-build-language \
    --skip-build-cmark \
    --build-wasm-stdlib \
    --native-language-tools-path="$LANGUAGE_TOOLS_PATH" \
    --native-clang-tools-path="$LANGUAGE_TOOLS_PATH" \
    --native-toolchain-tools-path="$LANGUAGE_TOOLS_PATH"
```

## Notes on the implementation

Here we're describing some decisions that were made while developing
the implementation.

### Relative Pointers

Relative pointers are used in Codira runtime, but currently it's not feasible to use them for some cases
where the pointee is a function pointer. The reason is that WebAssembly has a separate address space for
functions and data, and the offset bwtween a function and a data pointer cannot be defined. Therefore,
we have to use absolute pointers for function pointers in WebAssembly (see `include/language/ABI/CompactFunctionPointer.h`
for more details).

