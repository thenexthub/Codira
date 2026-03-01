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
  files: [MyUtil.ets]
  files: [namespace_export.ets]
---*/

import MyUtil from './MyUtil';
import * as compareSpecFunc from './namespace_export';

class MyPriKey implements MyUtil.PriKey {
    public privateKey: string;

    constructor(privateKey: string) {
        this.privateKey = privateKey;
    }

    getKey(key: string): string {
        if (key === this.privateKey) {
            return 'correct';
        } else {
            return 'error';
        }
    }
}

class MyPubKey implements MyUtil.PubKey {
    public publicKey: string;

    constructor(publicKey: string) {
        this.publicKey = publicKey;
    }

    getKey(key: string): string {
        if (key === this.publicKey) {
            return 'correct';
        } else {
            return 'error';
        }
    }
}

function main() {
    let priKey = new MyPriKey('private')
    let pubKey = new MyPubKey('public')
    let res: boolean = compareSpecFunc.DHKeyPair(priKey, pubKey) && true;
    arktest.assertTrue(res)
}