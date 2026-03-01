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

{% for fd in overload_function_decls %}
/*---
desc: >-
    The syntax of a function with overload signatures is presented below (see also Function Declarations):
    functionWithOverloadSignatures:
        functionOverloadSignature*
        functionDeclaration
    functionOverloadSignature:
        'async'? 'function' identifier typeParameters? signature
---*/

{{fd.decl}}

function main(): void {
    {{fd.call|indent}}
}

{% endfor %}
