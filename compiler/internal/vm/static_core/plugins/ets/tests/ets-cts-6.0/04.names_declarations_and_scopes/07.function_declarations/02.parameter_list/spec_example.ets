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
desc: >-
  A signature contains a parameter list that specifies an identifier
  and a type of each parameter. Every parameter's type must be
  explicitly defined.
---*/

function power(base: double, exponent: double): double {
    return Math.pow(base, exponent)
}

function main() {
    arktest.assertEQ( 8, power(2, 3)) // both arguments are required in the call
}
