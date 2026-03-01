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

{% for case in cases %}
/*---
desc: A floating-point literal in variable and constant declarations can be implicitly converted to type float
params: {{case}}
---*/

let x1 = {{case}}
const x2  = {{case}}

function main(): void {
    let y1: Float = x1
    const y2: Float = x2
}
{% endfor %}