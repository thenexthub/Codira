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
desc: Chaining operator, access nested object properties.
---*/

interface AddressType {
  street?: string;
  city?: string;
}

class Person {
  name?: string;
  address: AddressType;
  constructor(address: AddressType) {
    this.address = address;
  }
}

function main(): void {
  let person1 = new Person({});
  person1.address.street = "123";
  arktest.assertEQ(person1.address?.city, undefined);
  person1.address.city = "xian";
  person1.name = "Alice";
  arktest.assertEQ(person1?.address.city, "xian");
}