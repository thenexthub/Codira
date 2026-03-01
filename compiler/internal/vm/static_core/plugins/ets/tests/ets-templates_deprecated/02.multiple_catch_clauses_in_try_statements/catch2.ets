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

class Ex extends Error {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

class F extends Error {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

function main(): int {
    try {
        try {
            throw new Ex("0");
        } catch (e: Ex) {
            try {
                throw new F("1", e);
            } catch (f: F) {
                try {
                    throw new Ex("2", f);
                } catch (e: Ex) {
                    try {
                        throw new F("3", e);
                    } catch (f: F) {
                        try {
                            throw new Ex("4", f);
                        } catch (e: Ex) {
                            try {
                                throw new F("5", e);
                            } catch (f: F) {
                                try {
                                    throw new Ex("6", f);
                                } catch (e: Ex) {
                                    try {
                                        throw new F("7", e);
                                    } catch (f: F) {
                                        try {
                                            throw new Ex("8", f);
                                        } catch (e: Ex) {
                                            try {
                                                throw new F("9", e);
                                            } catch (f: F) {
                                                try {
                                                    throw new Ex("10", f);
                                                } catch (e: Ex) {
                                                    try {
                                                        throw new F("11", e);
                                                    } catch (f: F) {
                                                        throw new F("12", f);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (f: F) {
      return 0;
    } catch (def) {
      arktest.assertTrue( false, "Unexpected catch clause" );
    }
    return 1;
}
