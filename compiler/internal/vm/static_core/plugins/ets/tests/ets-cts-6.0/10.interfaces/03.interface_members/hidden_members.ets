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

{% for im in interface_members %}
/*---
desc: Hide inheritance members.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for fields that it hides.
tags: [compile-only]
---*/

interface A {
  {{im}}
}

interface B extends A {
  {{im}}
}

{% endfor %}
