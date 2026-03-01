/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
{% for type in types %}
/*---
desc: Left-hand expression testing - record element access testing.
---*/

type Keys = 'key1' | 'key2' | 'key3'

function main(): void {
  let myRecord: Record<{{type}}, number> = {
    'key1': 1,
    'key2': 2,
    'key3': 4,
  }
  myRecord['key2'] = 6;
  arktest.assertEQ(myRecord['key2'], 6);
}
{% endfor %}