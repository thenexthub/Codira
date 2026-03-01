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

 export interface User {
  id: number;
  name: string;
  age?: number;
}

 export class Person implements User {
  public id: number = 0;
  public name: string = 'aaa';
  public age?: number = 1;
  show(): void {
    console.log('b');
  }
}

export interface AccountUser extends User {
  balance: number;
}

export class Person2 implements AccountUser {
  id: number = 1001;
  name: string = 'Alice';
  age?: number = 1;
  balance: number = 500;
}

export class VipPerson extends Person2 {
  id: number = 1001;
  name: string = 'Alice';
  age?: number = 1;
  balance: number = 500;
  vipLevel: number = 3;
}


function printUser(user: User) {
  console.log(`ID: ${user.id}, NAME: ${user.name}`);
}

function main() {
  let p = new Person();
  p.show();
}