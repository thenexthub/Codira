/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

{% for option in [1, 2] %}
{% for c in cases %}
/*---
desc: >-
      In multi-layer inheritance, the sub-interface inherits the methods of
      all upper-layer interfaces. The return data type of the method is {{c.type}}.
---*/
{% if c.type == "ColorSet" %}
enum ColorSet {
  Red = 0,
  Green = 1,
  Blue = 2
}{% elif c.type == "Person" %}
interface Person {
    name: string;
    age: number;
    hobbies: string[];
}
let j1: Person = {
    name: "John1",
    age: 30,
    hobbies: ["reading", "swimming"]
};
let j2: Person = {
    name: "John2",
    age: 30,
    hobbies: ["playing"]
};
{% endif %}

interface GrandParentInterface {
{% if option == 1 %}
  grandparentMethod(): {{c.type}} {
    return {{c.val1}};
  }
{% else %}
  grandparentMethod(): {{c.type}}
{% endif %}
}

interface ParentInterface extends GrandParentInterface {
{% if option == 1 %}
  parentMethod(): {{c.type}} {
    return {{c.val2}};
  }
{% else %}
  parentMethod(): {{c.type}}
{% endif %}
}

interface ChildInterface extends ParentInterface {
{% if option == 2 %}
  grandparentMethod(): {{c.type}} {
    return {{c.val1}};
  }
  parentMethod(): {{c.type}} {
    return {{c.val2}};
  }
{% endif %}
}

class ConcreteClass implements ChildInterface {}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();
  arktest.assertEQ(obj.grandparentMethod(){{c.option}}, {{c.res1}});
  arktest.assertEQ(obj.parentMethod(){{c.option}}, {{c.res2}});
}
{% endfor %}
{% endfor %}
