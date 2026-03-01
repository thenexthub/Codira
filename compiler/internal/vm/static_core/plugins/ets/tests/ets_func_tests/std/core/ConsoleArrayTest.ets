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

class Connection {
    id: number;
    user: string;

    constructor(id: number, user: string) {
        this.id = id;
        this.user = user;
    }
}

function main() {
    let d: Connection[] = [
        new Connection(1, "user1"),
        new Connection(2, "user2")
    ]

    // Debug arrays
    console.debug("Active connections:", d);
    console.debug([1, 2, 3, 4, 5]);

    console.info("Active connections:", d);
    console.info([1, 2, 3, 4, 5]);

    console.log("Active connections:", d);
    console.log([1, 2, 3, 4, 5]);

    console.warn("Active connections:", d);
    console.warn([1, 2, 3, 4, 5]);

    // Note(ivan-tyulyandin): uncomment and fix .expected file after fix of #25014
    // console.error("Active connections:", d);
    // console.error([1, 2, 3, 4, 5]);
}
