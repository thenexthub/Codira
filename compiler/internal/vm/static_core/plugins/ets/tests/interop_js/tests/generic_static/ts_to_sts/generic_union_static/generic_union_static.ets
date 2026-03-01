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

let module = ESValue.load('main_js')
let UnionStatic = module.getProperty('UnionStatic');
let genericUnionStaticCallFromTs = module.getProperty('genericUnionStaticCallFromTs');

function checkGenericUnionInt(): boolean {
    try {
        const tsInt = ESValue.wrapInt(1);
        const res = UnionStatic.invokeMethod('get', tsInt);
        
        if (res == null) {
            return false;
        }
        
        return res.toNumber() === 1;
    } catch (e) {
        return false;
    }
}

function checkGenericUnionString(): boolean {
    try {
        const tsString = ESValue.wrapString("test");
        const res = UnionStatic.invokeMethod('get', tsString);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "test";
    } catch (e) {
        return false;
    }
}

function checkGenericUnionCallFromTsInt(): boolean {
    try {
        const tsInt = ESValue.wrapInt(1);
        const res = genericUnionStaticCallFromTs.invoke(tsInt);
        
        if (res == null) {
            return false;
        }
        
        return res.toNumber() === 1;
    } catch (e) {
        return false;
    }
}

function checkGenericUnionCallFromTsString(): boolean {
    try {
        const tsString = ESValue.wrapString("test");
        const res = genericUnionStaticCallFromTs.invoke(tsString);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "test";
    } catch (e) {
        return false;
    }
}