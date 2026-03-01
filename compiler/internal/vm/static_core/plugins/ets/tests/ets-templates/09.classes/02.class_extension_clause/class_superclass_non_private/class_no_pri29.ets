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
desc: >-
    The child class calls the parent class method, and the asynchronous method overridden by the child class
    is used in the parent class method.
---*/

class Parent {
  public async fetchMessage(): Promise<string> {
    return "Hello from Parent!";
  }

  public async greet(): Promise<string> {
    return await this.fetchMessage();
  }
}

class Child extends Parent {
  public async fetchMessage(): Promise<string> {
    return "Hello from Child!";
  }

  public async callParentGreet(): Promise<string> {
    return await this.greet();
  }
}

function main(): void {
  waitForCompletion(async () => {
    const child = new Child();
    arktest.assertEQ(await child.callParentGreet(), "Hello from Child!");
  });
}
