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

/*---
desc: The exception extends to the surrounding scope if no catch clause can handle the exception.
---*/

class Ex extends Error {}
class F extends Error {}

function main(): int {
    try {
        try {
            try{
                try {
                    try {
                        try {
                            try {
                                try {
                                    try {
                                        try {
                                            try {
                                                throw new F();
                                            } catch (e: Ex) {
                                                return 1;
                                            }
                                        } catch (e: Ex) {
                                            return 1;
                                        }
                                    } catch (e: Ex) {
                                        return 1;
                                    }
                                } catch (e: Ex) {
                                    return 1;
                                }
                            } catch (e: Ex) {
                                return 1;
                            }
                        } catch (e: Ex) {
                            return 1;
                        }
                    } catch (e: Ex) {
                        return 1;
                    }
                } catch (e: Ex) {
                    return 1;
                }
            } catch (e: Ex) {
                return 1;
            }
        } catch (e: Ex) {
            return 1;
        }
    } catch (e: Ex) {
        return 1;
    } catch (f: F) {
        return 0;
    }
    return 1;
}
