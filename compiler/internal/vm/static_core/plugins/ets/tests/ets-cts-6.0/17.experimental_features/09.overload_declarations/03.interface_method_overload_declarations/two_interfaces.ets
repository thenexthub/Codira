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
desc: >-
    Interface method overload declaration allows declaring an overload
    alias as an interface member for a set of interface methods.
---*/

let res: number = 0;

type TPL = [boolean, float];
type FUN = () => boolean;

class Gen<T> {
    foo(i: Gen<T>) {};
}

class Gen2<T,S> {
    foo(i: T) {};
}

interface IBase<T,S> {
    {%- for d in cases %}
    foo{{loop.index}}({{d.arg}}) { res = 100 + {{d.res}}; }
    {%- endfor %}
    overload foo { {% for d in cases %}foo{{loop.index}}{{ ", " if not loop.last }}{% endfor %} }
}

interface IDerived<T,S> extends IBase<T,S> {
    foo({{c.arg}}) {
        res = 200 + {{c.res}};
    }
}

class Base<T,S> implements IBase<T,S> {}
class Derived<T,S> implements IDerived<T,S> {}

function main(): void {
  let a = new Derived<Number, String>();
  let b = new Base<Number, String>();
  let c: TPL = [ false, 1.2f ];
  b.foo({{c.call}}); // base instance method call
  arktest.assertTrue(res == 100 + {{c.res}});
  a.foo({{c.call}}); // derived instance method call
  arktest.assertTrue(res == 200 + {{c.res}});
}

{% endfor %}
