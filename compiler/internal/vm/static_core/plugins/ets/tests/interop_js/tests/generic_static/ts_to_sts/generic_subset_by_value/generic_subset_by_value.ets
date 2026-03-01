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
let SubsetByValueStatic = module.getProperty('SubsetByValueStatic');
let genericSubsetByValueStaticCallFromTs = module.getProperty('genericSubsetByValueStaticCallFromTs');
let BaseObj = module.getProperty('BaseObj');

function checkSubsetByValueStatic(): boolean {
    try {
        const baseObjA = BaseObj.getProperty('a');
        
        if (baseObjA == null) {
            return false;
        }
        
        const res = SubsetByValueStatic.invokeMethod('get', baseObjA);
        
        if (res == null) {
            return false;
        }
        
        return res.toNumber() === baseObjA.toNumber();
    } catch (e) {
        return false;
    }
}

function checkGenericSubsetByValueStaticCallFromTs(): boolean {
    try {
        const baseObjA = BaseObj.getProperty('a');
        
        if (baseObjA == null) {
            return false;
        }
        
        const res = genericSubsetByValueStaticCallFromTs.invoke(baseObjA);
        
        if (res == null) {
            return false;
        }
        
        return res.toNumber() === baseObjA.toNumber();
    } catch (e) {
        return false;
    }
}