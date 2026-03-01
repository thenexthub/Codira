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


import * as ns from '../../mod1';
import { bar as bar2 } from './mod2';

function func1(o1, o2) { }

// call scope-defined function/global function
function foo(o1, o2) {
  let func2 = (x) => { return x * 2; };
  func2(o2.x);

  // call func from global variable
  console.log('log');
  globalvar.hilog.logd('logd');

  let func3 = func2;
  let func4 = func3;
  func4(o1.x);

  let a = o1.x;
  if (a) {
    func1(2, o2.y);
    o2.bar();
  }
}

// indirect call via 'apply' or 'call'
function foo1(o1, o2, o3) {
  let obj = { z: 2 };
  function fn(x, y) {
    print(x + y + this.z);
  }

  fn.call(obj, 2, 3);
  fn.apply(obj, [2, 3]);
  let bound = fn.bind(obj, 4);
}

class Point {
  constructor(x, y) {
    this.x = x;
    this.y = y;
  }

  get getCoordinateX() {
    return this.x;
  }

  set setCoordinateX(x) {
    return this.x = x;
  }

  plus(other) {
    this.x += other.x;
    this.y += other.y;
  }

}

function func5() {
  // call super
  class ColorPoint extends Point {
    constructor(x, y, color) {
      super(x, y);
      this.color = color;
    }
  }
}

// call function of across module
function func6(o1, o2) {
  bar2();

  function func7() {
    let func8 = ns.math.sum;
    func8(1, 2);
    ns.sub(o2, o1);
  }
}

// call member function
function func9() {
  let point = new Point(2, 3);

  function callMemberFunc1(p) {
    let x = point.getCoordinateX();
    point.setCoordinateX(x);
    let point2 = point;

    function callMemberFunc2(p) {
      point2.plus(p);
      point2.sub(p);
    }
  }
}

function func10(o1, o2) {
  let service = new ns.PhoneService();
  function callExClassMemberFunc1(o) {
    service.makePhoneCall(o);
  }
}
