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

package std.interop.js;

export final class ESError extends Error {
    private err_: ESValue

    private constructor(err: Any) {
        super("ESError", "", undefined);
        this.err_ = new ESValue(err);
    }

    /**
     * Retrieves the name of the error from the ESValue.
     * If the property does not exist, returns "UnknownError".
     */
    override get name(): string {
        return this.err_.hasProperty("name") 
            ? this.err_.getProperty("name").toString() 
            : "UnknownError";
    }

    /**
     * Sets the name of the error in the ESValue.
     */
    override set name(val: string) {
        this.err_.setProperty("name", ESValue.wrapString(val));
    }

    /**
     * Retrieves the message of the error from the ESValue.
     * If the property does not exist, returns an empty string.
     */
    override get message(): string {
        return this.err_.hasProperty("message") 
            ? this.err_.getProperty("message").toString() 
            : "";
    }

    /**
     * Sets the message of the error in the ESValue.
     */
    override set message(val: string) {
        this.err_.setProperty("message", ESValue.wrapString(val));
    }

    /**
     * Retrieves the cause of the error from the ESValue.
     * If the property does not exist, returns undefined.
     */
    override get cause(): ESValue | undefined {
        return this.err_.hasProperty("cause") 
            ? this.err_.getProperty("cause") 
            : undefined;
    }

    /**
     * Sets the cause of the error in the ESValue.
     */
    override set cause(val: Object | undefined) {
        if (val !== undefined) {
            this.err_.setProperty("cause", val);
        }
    }

    /**
     * Retrieves the ESValue associated with this error.
     */
    getValue(): ESValue {
        return this.err_;
    }
}
