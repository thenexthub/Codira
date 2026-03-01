/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc:  Use imported class which implements an interface with mandatory fields as T for Partial<T>
files:
    - ../exported_data.ets
---*/

import {CI} from "../exported_data"
import {gs} from "../exported_data"

function foo(i: Partial<CI>) {
  let v: int = 0;
  if(i.j != null) {
    v += 1;
  }
  if(i.i != 0) {
    v += 2;
  }
  return v;
}

function main() {
  arktest.assertTrue( foo( {{c.argument}} ) == {{c.result}} )
}

{% endfor %}
