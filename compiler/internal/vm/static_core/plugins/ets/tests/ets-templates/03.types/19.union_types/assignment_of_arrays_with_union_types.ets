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

{%- for c in cases %}

/*---
desc: {{c.desc}}
ark_options: ['--compiler-inlining=false']
---*/
// NOTE(mgroshev): remove ark_options after #29029

interface GenericInterface<T> {
  process(data: T): T;
}

class GenericImplementation<T> implements GenericInterface<T> {
  process(data: T): T {
    return data;
  }
}

{{c.enumerate}}
{{c.function}}

function main(): void {
  let arrImplementation: GenericInterface<({{c.type}})[]> = new GenericImplementation<({{c.type}})[]>();
  let inputArray: ({{c.type}})[] = [{{c.value}}];
  let result = arrImplementation.process(inputArray);
  arktest.assertEQ(result, inputArray);
}
{%- endfor %}
