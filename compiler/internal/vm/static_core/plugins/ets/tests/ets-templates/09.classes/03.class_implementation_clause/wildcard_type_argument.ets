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

{% for option in wildcards %}

/*---
desc: Use wildcard in type arguments.
assert: >-
    If the typeReference has type arguments, none of the type arguments may be wildcard type arguments,
    or a compile-time error occurs.
tags: [compile-only]
---*/

interface A<{{option}}> {}

class B implements A<Int> {}

{% endfor %}
