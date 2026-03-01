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
desc: Multi-line string literals that can include line continuation \u2029
---*/

let sentence2 = `This is an example of multi-line string,
which should be enclosed in backticks (\`)\u2029`

function main() {
    let sentence = "This is an example of multi-line string,\nwhich should be enclosed in backticks (\`)\u2029"
    arktest.assertEQ( sentence, sentence2)
}
