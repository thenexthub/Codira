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

/*---
desc: 17.11.3 Constructor Names
tags: []
---*/

class Temperature {
    value: double = 0;

    constructor (value: double) {
        this.value = value;
    }

    constructor Celsius(value: double) {
        this(value)
    }

    constructor Fahrenheit(value: double) {
        this.Celsius((5.0 / 9.0) * (value - 32))
    }
}

function main() {

    let temp  = new Temperature.Celsius(100)
    arktest.assertTrue(temp instanceof Temperature)
    arktest.assertEQ(100, temp.value)

    temp = new Temperature.Fahrenheit(212)
    arktest.assertTrue(temp instanceof Temperature)
    arktest.assertEQ(100, temp.value)
}
