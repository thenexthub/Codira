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
let LiteralStatic = module.getProperty('LiteralStatic');
let genericLiteralCallFromTs = module.getProperty('genericLiteralCallFromTs');


function checkGenericLiteralStart(): boolean {
    try {
        const tsLiteralStart = ESValue.wrapString("start");
        const res = LiteralStatic.invokeMethod('get', tsLiteralStart);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "start";
    } catch (e) {
        return false;
    }
}

function checkGenericLiteralStop(): boolean {
    try {
        const tsLiteralStop = ESValue.wrapString("stop");
        const res = LiteralStatic.invokeMethod('get', tsLiteralStop);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "stop";
    } catch (e) {
        return false;
    }
}

function checkGenericLiteralCallFromTsStart(): boolean {
    try {
        const tsLiteralStart = ESValue.wrapString("start");
        const res = genericLiteralCallFromTs.invoke(tsLiteralStart);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "start";
    } catch (e) {
        return false;
    }
}

function checkGenericLiteralCallFromTsStop(): boolean {
    try {
        const tsLiteralStop = ESValue.wrapString("stop");
        const res = genericLiteralCallFromTs.invoke(tsLiteralStop);
        
        if (res == null) {
            return false;
        }
        
        return res.toString() === "stop";
    } catch (e) {
        return false;
    }
}
