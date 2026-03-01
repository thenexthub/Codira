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
desc: The default value of an annotation field can be specified by using initializer as constant expression
tags: [compile-only, negative]
params: "{{ case.type }}"
---*/


let a: String = "Some String"
let b: Number = 1
enum MyEnum { EnumField1 = b + 1, EnumField2 = b - 1 }

// Annotation declaration:
@interface ClassAnnotation {
   field: {{ case.type }} = {{ case.value }}
}

// Annotation use:
@ClassAnnotation
class MyClass {}

{% endfor %}
