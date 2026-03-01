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

{% for qname in qualified_name_import %}
/*---
desc: imported qualified names
files: ['./qualified_name_export.ets']
---*/
import { Clazz as C } from './qualified_name_export'
import { Intf as I } from './qualified_name_export'

{{qname.decl}}

function main() {
    arktest.assertEQ( {{qname.use}} )
}

{% endfor %}
