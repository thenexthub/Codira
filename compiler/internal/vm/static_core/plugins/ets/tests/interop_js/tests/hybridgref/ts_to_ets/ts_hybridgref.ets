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
package ets_functions;

native function nativeGetRef(): ESValue;

export function etsCheckNativeGrefString(): boolean {
    let value = nativeGetRef();
    return value.toString() === "NativeGref String";
}

export function etsCheckNativeGrefNumber(): boolean {
    let obj = nativeGetRef();
    return obj.toNumber() === 123.456;
}

export function etsCheckNativeGrefObjectField(): boolean {
    let obj = nativeGetRef();
    let field = obj.getProperty("name");
    return field.toString() === "ArkTS";
}

export function etsCheckNativeGrefArray(): boolean {
    let arr = nativeGetRef();
    let len = arr.getProperty("length").toNumber();
    if (len !== 3) return false;
    return arr.getProperty(0).toNumber() === 10
        && arr.getProperty(1).toNumber() === 20
        && arr.getProperty(2).toNumber() === 30;
}

export function etsCheckNativeGrefCallback(): boolean {
    let func = nativeGetRef();
    let res = func.invoke(5, 10);  // simulate: func(5, 10)
    return res.toNumber() === 50;
}

export function etsCheckNativeGrefNullSafety(): boolean {
    let value = nativeGetRef();
    return value.isNull() || value.isUndefined();
}


export function etsCheckNestedObject(): boolean {
  const obj = nativeGetRef();
  const tag = obj.getProperty("meta").getProperty("tag").toString();
  const active = obj.getProperty("meta").getProperty("inner").getProperty("active").toBoolean();
  return tag === "nested" && active;
}

export function etsCheckNestedArray(): boolean {
  const arr = nativeGetRef();
  return arr.getProperty(0).getProperty(1).toNumber() === 2 &&
         arr.getProperty(2).getProperty(0).toNumber() === 5;
}

export function etsCheckObjectWithMethod(): boolean {
    let obj = nativeGetRef();
    let result = obj.invokeMethod("getDouble").toNumber();
    return result === 84;
}

export function etsCheckSymbolFieldIgnored(): boolean {
    let obj = nativeGetRef();
    let visible = obj.getProperty("visible").toString();
    return visible === "shown";
}


export function etsCheckUndefinedField(): boolean {
    let obj = nativeGetRef();
    return obj.getProperty("a").toNumber() === 1 && obj.getProperty("b").isUndefined();
}

export function etsCheckEmptyArray(): boolean {
    let arr = nativeGetRef();
    let len = arr.getProperty("length").toNumber();
    return len === 0;
}

export function etsCheckDateObject(): boolean {
    let date = nativeGetRef();
    let dateStr = date.invokeMethod("toString").toString();
    return dateStr.includes("2025");
}

export function etsCheckArrayBuffer(): boolean {
    const buffer = nativeGetRef();
    const len = buffer.getProperty("byteLength").toNumber();
    return len === 8;
}

export function etsCheckTypedArray(): boolean {
    const arr = nativeGetRef();
    const val0 = arr.getProperty(0).toNumber();
    const val1 = arr.getProperty(1).toNumber();
    const val2 = arr.getProperty(2).toNumber();
    return val0 === 1 && val1 === 2 && val2 === 3;
}

export function etsCheckPromiseResolved(): boolean {
    const promise = nativeGetRef();

    let result: number = -1;
    let done: boolean = false;
    let cb = (val: number) => {
        result = val;
        done = true;
    }
    promise.invokeMethod("then",cb);
    return done && result === 42;
}

export function etsCheckErrorObject(): boolean {
    const err = nativeGetRef();
    const msg = err.getProperty("message").toString();
    const name = err.getProperty("name").toString();
    return name === "Error" && msg.includes("hybridgref error test");
}

export function etsCheckSetObject(): boolean {
    const set = nativeGetRef();
    let size = set.getProperty("size").toNumber();
    let hasValue = set.invokeMethod("has", ESValue.wrap(3)).toBoolean();

    return size === 3 && hasValue;
}
