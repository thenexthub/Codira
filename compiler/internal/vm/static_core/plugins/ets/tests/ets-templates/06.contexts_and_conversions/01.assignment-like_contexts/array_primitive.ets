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

{% for c in cases %}
/*---
desc: Assignment contexts of a primitive types
params: Assignment from {{c.src_type}} to {{c.dst_type}}
tags: [compile-only]
---*/

function main() {
  let src_{{loop.index}}: FixedArray<{{c.src_type}}> = new FixedArray<{{c.src_type}}>(1);
  let dst_{{loop.index}}: FixedArray<{{c.dst_type}}> = new FixedArray<{{c.dst_type}}>(1);
  dst_{{loop.index}} = src_{{loop.index}};
}
{% endfor %}
