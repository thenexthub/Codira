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

{% for c in cases %}
/*---
desc: Any else corresponds to the nearest preceding if of an if statement.
---*/

function main(): void {
    if ({{c.cond1}})
        if ({{c.cond2}}) arktest.assertTrue({{c.bool1}})
    else arktest.assertTrue({{c.bool2}}) // Executes only if: Cond1 && !Cond2
}
{% endfor %}
