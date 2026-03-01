/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: An if statement can be conditionally executed, or one of the two two statements can be selected conditionally.
---*/

let res = ""

function ct(): boolean {
    res += "T"
    return true;
}

function cf(): boolean {
    res += "F"
    return false;
}

function retcode(s: string): int {
    if (res == s) {
        return 0;
    } else {
        return 1;
    }
}

function foo(i: number, j: number): number {
    {{c.use|indent}}
}

function main(): void {
    res = ""; // reset global res variable because of JIT testing
    let i = 1;
    let j = 1;
    arktest.assertEQ(foo(i, j), 0)
    
}
{% endfor %}
