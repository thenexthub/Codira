/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Decalre interface with parameters and superinterface with same parameters.
assert: Given a (possibly generic) interface declaration I<F1,...,Fn> (n 0), the direct superinterfaces of the interface type I<F1,...,Fn> are the types given in the extends clause of the declaration of I, if an extends clause is present.
tags: [compile-only]
---*/

interface A<T, G> {}

interface B<T, G> extends A<Object, String> {}
