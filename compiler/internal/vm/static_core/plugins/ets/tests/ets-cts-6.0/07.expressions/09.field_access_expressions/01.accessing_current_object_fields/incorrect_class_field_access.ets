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

{% for icf in cases %}

/*---
desc: A compile-time error occurs if the identifier {{icf.text}}
tags: [negative, compile-only]
---*/

interface I {
  i1: int;
}

class A implements I {
  public a1: int;
  public a2: int;
  private pa3: int;
}

class B extends A {
  public b1: int;
  private pb2: int;
}

function main(): void {
  {{icf.init}}
  {{icf.check}}
}

{% endfor %}
