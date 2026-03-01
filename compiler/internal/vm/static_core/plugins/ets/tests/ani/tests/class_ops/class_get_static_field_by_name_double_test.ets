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

class Woman {
    static name: String = "Bob";
    static age: double = 20.0;
    static specia1: int = 0;
    static specia3: string = "";
    static specia4: null = null;
    static specia5: undefined  = undefined ;
    static doubleMin: double = -1.7976931348623157e+308;
    static minpositive: double = 2.2250738585072014e-308;
    static doubleMax: double = 1.7976931348623157e+308;
}

class DoubleStaticA extends Woman {
    static double_value: double = 0.0;
}

final class DoubleStaticFinal extends Woman {
    static double_value: double = 0.0;
}