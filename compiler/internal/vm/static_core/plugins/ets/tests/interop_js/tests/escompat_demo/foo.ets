/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

class FooClass {
    constructor(name: String) {
        this.name = name;
    }
    // TODO(vpukhov): remove override
    public override toString(): String {
        return "{Foo named \"" + this.name + "\"}"
    }
    public name: String;
};

// TODO(vpukhov): add console.log
function FooFunction(arr: Array<FooClass>): String {
    console.println("FooFunction: check instanceof Array: " +
        ((arr as Object) instanceof Array));
    console.println("FooFunction: " + (arr.at(0)! as Object as FooClass).name);
    console.println("FooFunction: " + arr.at(1));
    return "FooFunction: " + arr;
}

function BarFunction(): Array<FooClass> {
    let arr = new Array<FooClass>();
    arr.push(new FooClass("three"));
    arr.push(new FooClass("four"));
    arr.push(new FooClass("five"));
    return arr;
}
