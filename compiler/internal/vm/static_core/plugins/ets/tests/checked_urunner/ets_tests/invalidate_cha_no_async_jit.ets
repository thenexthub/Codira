/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class Base {
    foo() { return 1 }
}
class Child extends Base {
    foo() { return 2 }
}
function __noinline__foo(x: Base) { return x.foo() }

function __noinline__test() {
    __noinline__foo(new Child())
}

//! CHECKER  Check no deadlock when loading Child and CHA-invalidating __noinline__foo in no async JIT
//! # use verification-mode=disabled so that verifier doesn't load the class in advance
//! RUN      force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::.*' --verification-mode=disabled", entry: "invalidate_cha_no_async_jit.ETSGLOBAL::main"
//! EVENT    /Inline,invalidate_cha_no_async_jit.ETSGLOBAL::__noinline__foo,invalidate_cha_no_async_jit.Base::foo,.*,VIRTUAL_CHA,SUCCESS/
//! EVENT    /ChaInvalidate,invalidate_cha_no_async_jit.Base::foo,invalidate_cha_no_async_jit.Child/
//! EVENT    /ChaDeoptimize,invalidate_cha_no_async_jit.ETSGLOBAL::__noinline__foo/

function main(): int {
    __noinline__foo(new Base())
    __noinline__test()
    return 0
}
