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
desc: An annotation cannot be extended or implemented as inheritance is not supported.
tags: [compile-only, negative]
params: "{{ case.type }}"
---*/

// Annotation declaration:
@interface SomeAnnotation {}

{{ case.type }} InheritedEntity {{ case.inheritance }} SomeAnnotation {}

{% endfor %}
