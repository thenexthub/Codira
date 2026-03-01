/*
 Copyright (c) 2023 Huawei Device Co., Ltd.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 *
 http://www.apache.org/licenses/LICENSE-2.0
 *
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

let nums = [2, 3, 1, 4, 8, 2];

function findMax() {
  let max = 0;
  nums.forEach(i => {
    if (i > max) {
      max = i;
    }
  });
  return max;
}

class Fruit {
  constructor(name, color) {
    this.name = name;
    this.color = color;
  }
}

let apple = new Fruit('apple', 'red');

let dog = {
  name: 'lucky',
  age: 2,
  color: 'yellow',
};

for (let i = 0; i < 10; i++) {
  if (i === 3) {
    break;
  }
}

let myDoubles = [12.34, 56.78];

function testTryCatch() {
  try {
      let a = 1;
      let b = 0;
      let c = a / b;
  } catch (error) {
      console.log('This is a simulated error ', error);
  }
}