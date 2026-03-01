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

function double2jsvalue(v: double): Any {
    return v;
}

function jsvalue2double(v: Any): double {
    return ESValue.wrap(v).toNumber();
}

function getPropertyFromJsvalue(obj: Any): double {
    return ESValue.wrap(obj).getProperty('prop').toNumber();
}

function getPropertyFromJsvalue2(obj: Any): double {
    return ESValue.wrap(obj).getProperty('prop_1').getProperty('prop_2').toNumber();
}

function setPropertyToJsvalue(obj: Any, v: double): void {
    ESValue.wrap(obj).setProperty('prop', v);
}
