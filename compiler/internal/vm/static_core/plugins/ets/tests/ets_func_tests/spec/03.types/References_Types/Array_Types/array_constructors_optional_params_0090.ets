/*
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*---
desc: 3.8.6 Array Types. Constructor of element of array has optional parameters
tags: []
---*/

class C {
    private e: int | undefined;
    private f: int | undefined;
    private g: int | undefined;
    constructor(e?: int, f: int = 20, g: int = 10) {
        this.e = e;
        this.f = f;
        this.g = g;
    }


    public getE() {return this.e}
    public getF() {return this.f}
    public getG() {return this.g}
}

function main() {
    let c  = new C[10]
    for (let i: int = 0; i < 10; i++) {
        arktest.assertEQ( c[i].getE(), undefined)
        arktest.assertEQ( c[i].getF(), 20)
        arktest.assertEQ( c[i].getG(), 10)
    }
}

