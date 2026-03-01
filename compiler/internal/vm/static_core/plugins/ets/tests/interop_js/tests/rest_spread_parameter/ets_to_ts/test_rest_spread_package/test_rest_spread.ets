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

package rest_spread.test;

function checkRestOfNumber(obj: FixedArray<number>, ...nums: FixedArray<number>) {
    if(obj.length !== nums.length)
        return false;
    for(let i = 0; i < obj.length; i++) {
        if(obj[i] !== nums[i])
            return false;
    }
    return true;
}

function checkRestOfString(obj: FixedArray<string>, ...strs: FixedArray<string>) {
    if(obj.length !== strs.length)
        return false;
    for(let i = 0; i < obj.length; i++) {
        if(obj[i] !== strs[i])
            return false;
    }
    return true;
}

function checkRestOfTuple(obj: FixedArray<FixedArray<number | string>>, ...tupleArr: FixedArray<FixedArray<number | string>>) {
    if(obj.length !== tupleArr.length)
        return false;
    for(let i = 0; i < obj.length; i++) {
        for(let j = 0; j < 2; j++) {
            if(obj[i][0] !== tupleArr[i][0] || obj[i][1] !== tupleArr[i][1])
                return false;
        }
    }
    return true;
}

function checkRestofUnion(obj: FixedArray<number | string | boolean>, ...unionArr: FixedArray<number | string | boolean>) {
    if(obj.length != unionArr.length)
        return false;
    for(let i = 0; i < obj.length; i++) {
        if(obj[i] != unionArr[i])
            return false;
    }
    return true;
}
