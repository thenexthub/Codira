/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Ambient declarations
tags: [not-a-test, compile-only]
---*/

export function foo(message: string): number { return 1 };

export class A {
  x: string;
  constructor(x: string) { this.x = x };
  foo(): string { return this.x };
}

export function bar(message: string): number { return 3 };

export class B {
  x: number;
  constructor(x: number) { this.x = x };
  foo(): number { return this.x };
}
