/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    Entity is considered accessible if it belongs to the current scope (see Scopes) and means that its name can be used
    for different purposes as follows
    - Type name is used to declare interface properties
    - Function or method name is used to call the function or method
    - Variable name is used to read or change the value of the variable
tags: [{{c.tags}}]
---*/

{{c.decl}}

interface I {
    foo: T;
    bar(i: T): T {
        return i;
    }
}

class A implements I {
    {{c.init}}
}

{{c.check}}

{% endfor %}
