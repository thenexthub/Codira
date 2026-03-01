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

let module = ESValue.load('js_code')
let etsUndefinedProduce = module.getProperty('etsUndefinedProduce');
let etsUndefinedConsume = module.getProperty('etsUndefinedConsume');
let etsNullProduce = module.getProperty('etsNullProduce');
let etsNullConsume = module.getProperty('etsNullConsume');

function TestPassUndefinedToEts(): boolean {
  
    let a: ESValue = etsUndefinedProduce.invoke()
    if (a.toUndefined() !== JSRuntime.getUndefined()) {
        return false;
    }
    if (a == null) {
        return false;
    }
    return true;}

function TestPassUndefinedFromEts(): boolean {
    let mUndefined: Any = undefined;
    let testtmp =etsUndefinedConsume.invoke(mUndefined);
    return testtmp.toBoolean();
}

function TestPassNullToEts(): boolean {
    if (etsNullProduce.invoke().toNull() as Any === undefined) {
        return false;
    }
    return true;
}

function TestPassNullFromEts(): boolean {
    let mNull: Any = null;
    return etsNullConsume.invoke(ESValue.wrap(mNull)).toBoolean();
}
