/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Function return another function through type, check parameters chaining
---*/

type CustomFunction = (name: string) => string;

const outerFunc = (test: string): CustomFunction => {
  return (test2: string): string => {
    return `${test} ${test2}`;
  };
};

function main(): void {
    //outer function
    const check = outerFunc('hello')
    //check inner function return
    arktest.assertEQ( check('world'), 'hello world' )
}
