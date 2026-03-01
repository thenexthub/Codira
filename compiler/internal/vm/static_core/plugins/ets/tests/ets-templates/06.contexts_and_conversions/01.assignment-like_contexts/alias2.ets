/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
 */

{% for case in cases %}
/*---
desc: Assignment contexts followed by a widening primitive conversion.
params: from '{{case.src_type}}' to '{{case.dst_type}}'
---*/

function main(): void {
  {% for v in case['values'] %}
  let src_{{loop.index}}: {{case.src_type}} = new {{case.src_type}}({{v|safe}});
  let dst_{{loop.index}}: {{case.dst_type}} = src_{{loop.index}};
  arktest.assertEQ(dst_{{loop.index}}, (src_{{loop.index}}.to{{case.dst_type}}()))
  {% endfor %}
}
{% endfor %}
