/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


import { BaseService } from '../base/service';
import * as ns from '../mod1';

async function func1() { }

function func2(o1, o2) {
  let func3 = function* () { };
  let obj = {
    'num': 3,
    'func4': (n1) => { return n1 * n1 },
  };
}

class Bar {
  constructor(name, color) {
    this.name = name;
    this.color = color;
  }

  func6() { }

  get getName() {
    return this.name;
  }
  set setName(name) {
    this.name = name;
  }

  func9(o1, o2, o3) { }
}

let a = 'func7';
let symbol = Symbol('func8');
function func10() {
  class Bar {
    baseFoo1() { }
  }

  class Bar2 extends Bar {
    func12(o1) { }
    [a]() { }
    [symbol]() { }
    func15(o1, o2) { }
  }
}

class ExampleClass1 {
  func17() { }
}
function func18() {
  class ExampleClass1 {
    func19() { }
  }

  function func20() {
    class ExampleClass2 extends ExampleClass1 { }
  }
}

function func21() {
  class ExtendService extends BaseService { }
  class ExtendPhoneService extends ns.Phone.PhoneService { }
  class ExtendDataSource extends globalvar.BasicDataSource { }
  class ExtendDataItem extends globalvar2.Data.DataItem { }
}
