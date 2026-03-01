/**
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

@interface AI1 {
  f1: string
}

@interface AI2 {}

@interface AI3 {}

@interface AI4 {}

@AI1({f1: 'Hello'})
class C1 {
  @AI2()
  public c1F2: int;
  async test() {
      console.log("123");
  }
}

@AI3()
interface I1 {
  @AI1({f1: 'interface field'})
  f3: string;
}

@AI3()
function bar(){};

namespace Ns1 {
  @AI4()
  class C2 {}

  @interface AI5 {}

  @AI5()
  interface I2 {}

  @AI2()
  function foo(){}
}


function main(){
  let c=new C1();
  c.test();
}

