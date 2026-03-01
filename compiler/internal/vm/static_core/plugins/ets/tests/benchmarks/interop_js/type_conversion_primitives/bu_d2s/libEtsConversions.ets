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
 
export class EtsConversions {

    public static binaryStringToNumberToString(bin: string): string {
        const res = parseInt(bin, 2);
        return String(res);
    }

    public static charToNumberToString(chr: string): string {
        const res = chr.charCodeAt(0);
        return String(res);
    }

    public static decimalStringToNumberToString(str: string): string {
        const res = Number(str);
        return String(res);
    }

    public static hexStringToNumberToString(str:string): string {
        const res = parseInt(str, 16);
        return String(res);
    }

    public static octalStringToNumberToString(str: string): string {
        const res = parseInt(str, 8);
        return String(res);
    }

}
