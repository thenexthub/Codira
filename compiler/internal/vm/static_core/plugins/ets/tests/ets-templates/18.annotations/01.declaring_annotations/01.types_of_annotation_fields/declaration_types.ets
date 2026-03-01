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

{% for case in cases %}

/*---
desc: The choice of types for annotation fields is limited to the following
         - Numeric types (see Numeric Types);
         - Type boolean (see Boolean Types and Operations);
         - Type string (see Type string);
         - Enumeration types (see Enumerations);
         - Array of the above types (e.g., FixedArray<string>),
           including multidimensional arrays (e.g., FixedArray<FixedArray<string>>).
tags: [compile-only, {{ case.negative }}]
params: "{{ case.type }}"
---*/

enum MyEnum { EnumField1, EnumField2 }

const s: String = new String ("some string")

// Annotation declaration:
@interface ClassAnnotation {
   field: {{ case.type }}
}

// Annotation use:
@ClassAnnotation({field: {{ case.value }} })
class MyClass {/*body*/}

{% endfor %}
