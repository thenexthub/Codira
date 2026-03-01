
### Testing Commmand

#### ArkTS to Codira binding

```bash
./target/bin/main --lib --module-name="my_module" -d ./tests/cases -o ./tests/expected/my_module/
```

#### C to Codira binding

./target/bin/main -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/include"

./target/bin/main -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"

./target/bin/main --no-detect-include-path -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"

```
