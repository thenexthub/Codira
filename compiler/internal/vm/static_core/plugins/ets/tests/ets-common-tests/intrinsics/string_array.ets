/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

let what:String = '';

function finder(s: String){
    return s.compareTo(what) ==0;
}

function lookfor(arr:Array<String> , what: String): String|undefined{
    let res = -1;

    for(let i =0; i < arr.length; i++){
        if(arr[i].compareTo(what)==0){
            res = i;
            break;
        }
    }

    if(res == -1){
        return undefined;
    }

    return arr[res];
}

function testLatin(){
    let strs = new Array<String>();
    let n =5000;
    for(let i =0; i < n; i+=10){
        strs.push(`${i}ww`);
        what = `${i}ww`;
        let found: String|undefined = lookfor(strs , what);
        arktest.assertEQ(found , what , 'The string should be found');
    }
}

function main():int{
    let suite = new arktest.ArkTestsuite('StringArray');
    suite.addTest('testLatin', testLatin);
    return suite.run(); 
}
