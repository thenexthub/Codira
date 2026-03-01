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
desc: Left-hand expression testing - field access testing.
---*/

class test_class {
  private _field: number;
  constructor() {
    this._field = 0;
  }
  set field(value: number) {
    this._field = value;
  }
  get field(): number {
    return this._field}
}

function main(): void {
  const test_object = new test_class();
  test_object.field = 20;
  arktest.assertEQ(test_object.field = 20, 20);
}