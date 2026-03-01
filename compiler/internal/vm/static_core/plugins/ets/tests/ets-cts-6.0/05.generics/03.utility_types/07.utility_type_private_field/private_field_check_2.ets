/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Private fields of the initial type stay in the utility type but they are not
      accessible and cannot be accessed in any way
tags: [compile-only, negative]
---*/

class Sample {
    publicField: int = 0;
    private description: String = "Hello Private";
}


function foo(value: Readonly<Sample>): void {
    let ff = value.description;
}


function main() {
    foo(new Sample());
}
