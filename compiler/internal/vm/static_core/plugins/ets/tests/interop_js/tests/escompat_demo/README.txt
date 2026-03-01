# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

Sources:
main.js - entrypoint
foo.ets / foo.ts - implementation / declaration of ets module
demo.ts - test app

Build arkruntime:
cmake -GNinja -DCMAKE_BUILD_TYPE=FastVerify -DPANDA_ETS_INTEROP_JS=1 -DCMAKE_TOOLCHAIN_FILE=../arkcompiler_runtime_core/cmake/toolchain/host_clang_14.cmake ../arkcompiler_runtime_core

Compile:
tsc --types node --typeRoots /usr/local/lib/node_modules/@types/ demo.ts foo.ts --outDir out

Launch:
ninja ets_interop_js__test_escompat_demo

Sample output:

FooFunction: check instanceof Array: true
FooFunction: zero
FooFunction: {Foo named "one"}
test: FooFunction: {Foo named "zero"},{Foo named "one"},{Foo named "two"}
test: check instanceof Array: true
test: three
test: four
test: five

Demo:

* out/main.js - launcher that initialises ArkTS VM and imports `demo`
* demo.ts - test that uses functions and classes from `foo.ets`. It does following:
    - Creates dynamic `Array` and passes it to static function `FooFunction`
    - Calls static function `BarFunction` that returns static `Array`
* foo.ets - has definitions of `FooFunction` and `BarFunction`
* foo.ts - file with TS declarations that can be generated from `foo.ets`
