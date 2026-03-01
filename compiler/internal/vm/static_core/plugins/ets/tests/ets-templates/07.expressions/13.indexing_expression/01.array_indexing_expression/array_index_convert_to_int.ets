/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: >-
    Other numeric types (long, float, and double/number) must be converted explicitly by applying the methods
    defined in the classes of the Standard Library.
---*/

const a = ["Alice", "Bob", "Carol"];

function main(): void {
    let index: {{c.type}} = {{c.value}};
    arktest.assertEQ("{{c.expected}}", a[index.toInt()]);
}
{% endfor %}
