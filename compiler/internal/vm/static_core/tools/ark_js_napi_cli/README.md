# Ark JS with NAPI CLI

ark_js_napi_cli is a CLI tool to execute JS abc files (similar to `ark_js_vm`) based on `arkui/napi`.

## How to build (workaround)

1. Add ark_js_napi_cli to ark_host_linux_tools_packages in ohos:
```bash
$ vim ./arkcompiler/runtime_core/BUILD.gn
...
       "$ark_root/libziparchive:libarkziparchive(${host_toolchain})",
       "$ark_root/verifier:ark_verifier(${host_toolchain})",
+      "$ark_root/static_core/tools/ark_js_napi_cli:ark_js_napi_cli(${host_toolchain})",
     ]
   }
...
```

2. Run build command:
```bash
$ ./build.sh --product-name rk3568 --build-target ark_js_napi_cli
```

## How to build in standalone mode
```bash
python3 ark.py x64.debug hybrid
```
