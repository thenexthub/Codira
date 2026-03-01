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
desc: Required interface properties
---*/

enum Color { Red, Green, Blue }

interface I {
  fld: {{c[0]}}
}

class A implements I {
  private _fld: {{c[0]}}
  set fld(p: {{c[0]}}) { this._fld = p } 
  get fld(): {{c[0]}} { return this._fld }
  constructor(p: {{c[0]}}) { this._fld = p }
}

let a: A = new A({{c[1]}})
arktest.assertNE(a.fld, null)

{% endfor %}
