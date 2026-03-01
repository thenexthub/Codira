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

{% for sd in self_dependency %}
{% for cm in top_level_class_modifiers %}
/*---
desc: A compile-time error occurs if a type parameter in the type parameter section depends on itself.
params: >
    {{sd}}
    {{cm}}
---*/
class SomeType {}
type T = SomeType

class TT<T> {} // for extends in type params
class TTT<T> {} // for extends in type params

{{cm}} class A<{{sd.decl}}> {}

{% if "abstract" in cm %}
class Child<{{sd.use_child}}> extends A<{{sd.use}}> {}
{% endif %}

function main(): void {
{% if "abstract" not in cm %}
    arktest.assertTrue(new A<{{sd.use}}>() instanceof A)
{% else %}
    arktest.assertTrue(new Child<{{sd.use}}>() instanceof A)
{% endif %}
}

{% endfor %}
{% endfor %}
