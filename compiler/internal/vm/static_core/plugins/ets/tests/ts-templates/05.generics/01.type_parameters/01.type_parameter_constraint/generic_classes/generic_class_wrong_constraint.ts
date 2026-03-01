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

{% for wc in wrong_constaints %}
{% for cm in top_level_class_modifiers %}
/*---
desc: generic parameters with wrong constraint in extends
tags: [compile-only, negative]
params: >
    {{wc}}
    {{cm}}

---*/

{{cm}} class Point<T extends {{wc}}> {}

function main(): void {}

{% endfor %}
{% endfor %}
