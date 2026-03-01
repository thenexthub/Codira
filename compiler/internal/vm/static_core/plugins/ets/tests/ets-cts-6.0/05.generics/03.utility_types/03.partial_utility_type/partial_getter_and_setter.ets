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
desc:  If class T has a user-defined getter, setter, or both, then none of those is called when object literal is used with
      Partial<T> variables. Object literal has its own built-in getters and setters to modify its variables
---*/

let z: int = 0;

class C {
  _i: number = 0;
  _j: string = "";
  set i(i: number) { z += 1; this._i = i; }
  get i() { z += 10; return this._i; }
  set j(j: string) { z += 5; this._j = j; }
  get j() { z += 50; return this._j; }
}

function foo(i: Partial<C>) {
  let v: int = 0;
  if(i._j != undefined) {
    v += 1;
  }
  if(i._i != 0) {
    v += 2;
  }
  return v;
}

function main() {
  z = 0;
  arktest.assertTrue( foo( {{c.argument}} ) == {{c.result}} )
  arktest.assertTrue( z == 0 );
}

{% endfor %}
