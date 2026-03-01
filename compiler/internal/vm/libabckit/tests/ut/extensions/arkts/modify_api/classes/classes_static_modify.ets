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

@interface Anno1{}

export class Class1 {
    @Anno1
    public field1 = 1;

    public m1() {
    };

    public static create(): Class1 {
        return new Class1();
    }
    // After AOP classAddMethod
    // foo() { return this.field1 + 1 ;}

}

export abstract class Class2 {
    public abstract m2();
}

class Class3 extends Class1 {
    public create(): Class3 {
        return new Class3();
    }
}

function foo(c: Class1){}

function main(){
    let klass =new Class1();
    console.log(klass.field1); // After AOP classAddMethod -> cosole.log(klass.foo());
}