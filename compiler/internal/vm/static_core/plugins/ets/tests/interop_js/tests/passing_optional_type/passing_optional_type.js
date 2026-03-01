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
export function fnWithAnyParams(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
export function fnWithAnyParamsObject(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
export function fnWithLiteralParam(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
export function fnWithLiteralObjectParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
export function fnWithExtraSetParam(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
export function fnWithExtraSetObjectParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
export function fnWithSubSetParam(obj) {
    return obj.city ? `${obj.name}; ${obj.city}` : obj.name;
}
export function fnWithUnionParam(id) {
    return id ? id : 'id not found';
}
export function fnWithUnionObjectParam(obj) {
    return obj.id ? obj.id : 'id not found';
}
export function fnWithSubSetReduseParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
export function fnWithSubSetPartialParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}