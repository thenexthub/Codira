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


import { doSomething } from './service';
import * as util from '../utils';

class Data {
  constructor(x, y) {
    this.x = x;
    this.y = y;
  }

  Add(o) {
    this.x = o.x;
    this.y = o.y;
  }
}

function foo(o1, o2) {
  let a = 0;
  let c = new Data(1, 2);
  if (o1 > o2) {
    c.Add(o1);
    a = o2;
    return c;
  } else if (o1 < o2) {
    c.Add(o2);
    a = o1;
  } else {
    a = o1 + o2;
    c.Add(new Data(1, 2));
  }

  let func1 = function (o) {
    function func2(o1, o2) {
      console.log(o1, o2);
    }
    let bar = func2;
    bar(c, o);
    return c;
  };
  let res = func1();

  doSomething(a);
  return util.sum(res, o2);
}
