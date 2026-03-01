
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

/*---
 desc: 17.3 Indexable Types
 tags:
 ---*/

type fooLambda = (arg: string) => string

class Bar {
    public lambda: fooLambda = (s: String) => {
        return s + s ;
    }
    $_get (index: int) : fooLambda  {return data}
    $_set (index: int, value: fooLambda) {data = value}

}

function main() {
    let x : Bar = new Bar();
    let lambda = x[0];
    arktest.assertEQ(lambda("hello"), "hellohello")

    let lambda2: fooLambda = (s: String) => {
        return s + s + s;
    }

    x[0] = lambda2;
    lambda = x[0];
    arktest.assertEQ(lambda("hello"), "hellohellohello")
    return 0;
}
