/*
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

/*---
desc: Chaining operator, combination of the chaining operator and array methods.
---*/

class Team {
  members?: Person[];

  constructor(members?: Person[]) {
    this.members = members;
 }
}

class Person {
  name: string;

  constructor(name: string) {
    this.name = name;
  }
}

function main(): void {
  const team = new Team();
  arktest.assertEQ(team.members?.[0], undefined);
  arktest.assertEQ(team.members?.[0].name, undefined);
  
  team.members = [new Person("Alice"), new Person("Bob")];
  arktest.assertEQ(team.members?.[0].name, "Alice");
}
