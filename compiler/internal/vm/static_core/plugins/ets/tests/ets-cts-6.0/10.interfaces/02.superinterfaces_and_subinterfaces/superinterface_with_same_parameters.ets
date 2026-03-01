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
desc: Declare two interfaces with same parameters, A - superinterface of B, B - subinterface of A.
assert: Given a generic interface declaration I<F1,...,Fn> (n > 0), the direct superinterfaces of the parameterized interface type I<T1,...,Tn>, where Ti(1 in) is a type, are all types J<U1,...,Uk >, where J<U1,...,Uk> is a direct superinterface of I<F1,...,Fn> and is the substitution [F1:=T1,...,Fn:=Tn].
tags: [compile-only]
---*/

interface C {}

interface A<T extends C> {}

interface B<T extends C> extends A<T> {}
