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
    An instance method is defined in a subclass with the same name as the instance method in a superclass.
    If signatures are override-compatible, then overriding is used. Otherwise, a compile-time error occurs.

    A static method is defined in a subclass with the same name as the static method in a superclass.
    If signatures are override-compatible (see Override-Compatible Signatures), then the static method in the
    subclass hides the previous static method. Otherwise, a compile-time error occurs.
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

class Base<T,S> {
  {% for d in cases %}
    foo({{d.arg}}) { res = 100 + {{d.res}}; }
    static foo<T, S>({{d.arg}}) { res = 300 + {{d.res}}; }
  {% endfor %}
}

class Derived<T,S> extends Base<T,S> {
    foo({{c.arg}}) {
        res = 200 + {{c.res}};
    }
}

class Derived2<T,S> extends Base<T,S> {
    static foo<T, S>({{c.arg}}) {
        res = 400 + {{c.res}};
    }
}

function main(): void {
  let a = new Derived<String, Number>();
  let b = new Derived2<String, Number>();
  let c: TPL = [ false, 1.2f];
  b.foo({{c.call}}); // base instance method call
  arktest.assertEQ( res, 100 + {{c.res}} );
  a.foo({{c.call}}); // derived instance method call
  arktest.assertEQ( res, 200 + {{c.res}} );
  Derived.foo({{c.call}}); // base static method call
  arktest.assertEQ( res, 300 + {{c.res}} );
  Derived2.foo({{c.call}}); // derived static method call
  arktest.assertEQ( res, 400 + {{c.res}} );
}

{% endfor %}
