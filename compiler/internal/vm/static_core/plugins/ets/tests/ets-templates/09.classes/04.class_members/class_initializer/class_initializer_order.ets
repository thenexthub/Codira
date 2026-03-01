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

{% for case in cases %}

/*---
desc: >-
    When a class is initialized, the class initializer declared in the class is executed along with
    all class initializers of all superclasses.
    The order of execution is from the top superclass to the current class.
---*/

let res = ""

class A {
  static a: int = 0
  static {
    A.a = 1
    res += "A"
  }
}

class B extends A {
  static b: int = 0
  static {
    B.b = 2
    res += "B"
  }
}

class C extends A {
  static c: int = 0
  static {
    C.c = 3
    res += "C"
  }
}

class D extends C {
  static d: int = 0
  static {
    D.d = 4
    res += "D"
  }
}

abstract class E {
  static e: int = 0
  static {
    E.e = 5
    res += "E"
  }
}

class F extends E {
  static f: int = 0
  static {
    F.f = 6
    res += "F"
  }
}

class G extends F {
  static g: int = 0
  static {
    G.g = 7
    res += "G"
  }
}

function main(): void {
  {{case.main}}
}

{% endfor %}
