/*---
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
---*/

{% for case in cases %}

/*---
desc: >-
    If no constraint is specified, then type parameter is not compatible with Object,
    and has no methods or fields available for use.
    Lack of constraint effectively means extends Object|null|undefined.
    If the constraint S is a non-nullish type (see Nullish Types), then T is also non-nullish.
    {{case.doc}}
tags: [{{case.tags}}]
---*/

{{case.code}}

{% endfor %}
