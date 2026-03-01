/*
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

 /*---
 desc: >-
     – Mandatory execution of field initializers (if any) in the order they appear in a class body.
 ---*/


 let recoil: string = '';

 function sf(input: string): string {
    recoil += input
    return input;
 }
 class BT {
     t0: string = sf('T')
     e: string = sf('e')
 }
 class OT extends BT {

    s: string = sf('s')
    t1: string = sf('t')
 }

 function main() {
    recoil = ''
    let ot = new OT
    arktest.assertEQ('Test', recoil)
 }
