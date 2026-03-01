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

package std.core;

// synchronize with mirror class in runtime
export class StackTraceElement {
    private className: String = ""
    private methodName: String = ""
    private sourceFileName: String = ""
    private lineNumber: int
    private colNumber: int = 0

    private constructor () {}

    /**
    * Packs data to single string and
    *
    * @returns data in formatted string
    */
    public override toString(): String {
        return "\tat " + this.className + "." + this.methodName +
             // NOTE(@sandee, #24492): Extraction of column number is not implemented  
             " (" + this.sourceFileName + ":" + this.lineNumber + ":" + this.colNumber + ")"
    }
}

export class StackTrace {
    /**
    * Method provides stack of methods at the place of a call
    *
    * @returns stack trace
    */
    static native provisionStackTrace(): FixedArray<StackTraceElement>
}
