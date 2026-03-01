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
package ets_any;

native function nativeCheckAnyGetProperty(anyObject: Any): boolean;
native function nativeCheckAnyGetPropertyByIndex(anyObject: Any): boolean;
native function nativeCheckAnyGetPropertyByValue(anyObject: Any): boolean;

native function nativeCheckAnySetProperty(anyObject: Any): boolean;
native function nativeCheckAnySetByIndex(anyObject: Any): boolean;
native function nativeCheckAnySetByValue(anyObject: Any): boolean;

native function nativeCheckAnyCall(func: Any, nameStr: string):boolean;

native function nativeCheckAnyCallMethod(anyObj: Any): boolean;

native function nativeCheckAnyNew(ctor: Any): boolean;

native function nativeCheckAnyInstanceOf(obj: Any, type: Any): boolean;

export function etsCheckAnyGetPropertyFromTS(): boolean {
    let module = ESValue.getGlobal();
    let esValue = module.getPropertySafe('tsAnyObject')
    let anyValue = esValue.unwrap();
    if (!nativeCheckAnyGetProperty(anyValue)) {
        return false;
    }
    let esArrayValue = module.getPropertySafe('tsAnyArray')
    let anyArrayValue = esArrayValue.unwrap()
    if (!nativeCheckAnyGetPropertyByIndex(anyArrayValue)) {
        return false;
    }
    if (!nativeCheckAnyGetPropertyByValue(anyValue)) {
        return false;
    }
    return true;
}

export function etsCheckAnySetPropertyToTS(): boolean {
    let module = ESValue.getGlobal();
    let obj = module.getPropertySafe('tsAnySetObject').unwrap();
    let arr = module.getPropertySafe('tsAnySetArray').unwrap();

    if (!nativeCheckAnySetProperty(obj)) return false;
    if (!nativeCheckAnySetByIndex(arr)) return false;
    if (!nativeCheckAnySetByValue(obj)) return false;

    return true;
}

export function etsCheckAnyCallFromTS(): boolean {
    let module = ESValue.getGlobal();
    let func = module.getPropertySafe('tsAnyFunc').unwrap();

    let nameStr = new String("Leechy");
    if (!nativeCheckAnyCall(func, nameStr)) return false;
    return true;
}

export function etsCheckAnyCallMethodFromTS(): boolean {
    let module = ESValue.getGlobal();
    let obj = module.getPropertySafe('tsAnyMethodObject').unwrap();
    return nativeCheckAnyCallMethod(obj);
}

export function etsCheckAnyNewFromTS(): boolean {
    let module = ESValue.getGlobal();
    let ctor = module.getPropertySafe('tsCtor').unwrap();
    return nativeCheckAnyNew(ctor);
}

export function etsCheckAnyInstanceOfFromTS(): boolean {
    let module = ESValue.getGlobal();
    let obj = module.getPropertySafe('tsDogInstance').unwrap();
    let type = module.getPropertySafe('tsAnimalClass').unwrap();
    return nativeCheckAnyInstanceOf(obj, type);
}