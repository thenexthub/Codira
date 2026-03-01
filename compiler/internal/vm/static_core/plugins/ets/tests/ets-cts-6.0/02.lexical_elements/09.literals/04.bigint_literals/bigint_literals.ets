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

{% for lit in bigint_literals %}
/*---
desc: >-
    BigInt literals use decimal base only. It is a sequence of digits which ends with symbol 'n'.
    BigInt literals are always of the type bigint.
params: '{{lit}}'
---*/

function main() {
    let x = {{lit}};
    arktest.assertEQ(x, {{lit}})
}
{% endfor %}
