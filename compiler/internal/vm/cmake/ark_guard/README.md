# ArkGuard

ArkGuard is a static bytecode obfuscation tools.

## Ark独立构建 
### Build
#### Linux
```sh
./ark.py x64.release guard_packages_linux --gn-args="abckit_enable=true"
```

### Run UnitTest
```sh
./ark.py x64.release guard_host_unittest --gn-args="abckit_enable=true"
```
