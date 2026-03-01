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

 function main(): void {
    let x = new Array<number>(4);
    x[0] = 0;
    x[3] = 3;
    arktest.assertEQ(x.join(), "0,,,3", 'x.join() === "0,,,3". Actual:' + (x.join()));

    let z = new Array<undefined>(4)
    arktest.assertEQ(z.join(), ",,,", 'z.join() === ",,,". Actual:' + (z.join()));

    let y = new Array<null>(null,null,null);
    arktest.assertEQ(y.toString(), ",,", 'let y = new Array<null>(null,null,null); y.toString(null,null,null) === ",,". Actual:' + (y.toString()));
 }
 
