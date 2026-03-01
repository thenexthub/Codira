/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: If an ambient namespace (see Ambient Namespace Declarations) defined in a module (see Modules),
      then all ambient namespace declarations are accessible across all declarations
      and top-level statements of the module.
      {{c.doc}}
files:
 - ./ns_ambient_decl.ets
---*/

import { NS } from './ns_ambient_decl.d'

{{c.decl}}

{% endfor %}
