/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

export default abstract class MyClass{
    A:number;
    constructor(a: number) {
        this.A = a;
    }
}

export class UserIns{
    name: string;
    age: number;

    constructor(name: string, age: number) {
        this.name=name;
        this.age=age;
    }
}

export class user extends UserIns{
    sex:string;
    constructor(name:string, age:number,sex:string){
        super(name, age);
        this.sex=sex;
    }
}

export interface X{
    name:string;
    print():void;
}

export class cla implements X{
    name:string;
    constructor(name:string){
        this.name=name;
    }

    print(){
        console.log("print");
    }
}
