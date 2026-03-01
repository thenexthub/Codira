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
desc: Declaring interface property
tags: [compile-only]
---*/

interface I1 {
    text: string;
}

interface I2 {
    get text(): string;
    set text(s: string);
}

interface I3 {
    get text(): string;
}

interface I4 {
    readonly text: string;
}

interface I5 {
    text?: string;
}
