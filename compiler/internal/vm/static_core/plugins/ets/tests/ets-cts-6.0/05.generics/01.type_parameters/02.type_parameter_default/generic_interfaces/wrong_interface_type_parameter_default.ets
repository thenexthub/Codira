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

{% for c in cases %}
/*---
desc: |-
    A compile-time error occurs if
        - a type parameter without a default type follows a type parameter
        with a default value in the declaration of a generic type.
        - Type parameter default refers to a type parameter defined after the current type parameter.
    Also covered some wrong declarations where the default parameter is not a type.
tags: [compile-only, negative]
params: {{c}}
---*/

class SomeType {}
class OtherType {}

interface Base<{{c}}> {}

{% endfor %}
