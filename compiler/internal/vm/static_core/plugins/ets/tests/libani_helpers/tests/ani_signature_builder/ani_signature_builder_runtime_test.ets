/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class TestClass{
    static func(a:boolean, b:boolean){
        return a||b;
    }
    static func(a:char, b:char){
        return a;
    }
    static func(a:byte, b:byte){
        return a;
    }
    static func(a:short, b:short){
        return a;
    }
    static func(a:int, b:int){
        return a+b;
    }
    static func(a:long, b:long){
        return a+b;
    }
    static func(a:float, b:float){
        return a+b;
    }
    static func(a:double, b:double){
        return a+b;
    }
}

enum COLOR{
    RED,
    GREEN,
    BLACK,
    WHITE,
}
namespace TestNamespace{
    type fnType = (a:int, b:int)=>void;
    type fnTypeRestArgs = (a:int, b:int, ...c:int[])=>void;

    function func(a:string){
        let res:int = 1;
        return res;
    }
    function func(a:COLOR){
        let res:int = 2;
        return res;
    }
    function func(a:fnType){
        let res:int = 3;
        return res;
    }
    function func(a:fnTypeRestArgs){
        let res:int = 4;
        return res;
    }
    function funcUndefined(a:undefined){
        let res:int = 5;
        return res;
    }
    function funcNull(a:null){
        let res:int = 6;
        return res;
    }
}