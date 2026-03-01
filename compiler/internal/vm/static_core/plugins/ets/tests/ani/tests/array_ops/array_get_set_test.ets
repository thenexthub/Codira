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

class ArrayWithCounters<T> extends Array<T> {
    public getCounter: int = 0
    public setCounter: int = 0
    private length_: int = 0
    private realLength: int = 0

    public constructor(arr: Array<T>) {
        super(arr.length)
        this.realLength = arr.length
        this.length_ = this.realLength
        for (let i = 0; i < arr.length; i++) {
            super.$_set(i, arr[i])
        }
        this.length_ = 0
    }

    public override get length(): int {
        return this.length_
    }

    public override $_get(index: int): T {
        this.getCounter += 1
        this.length_ = this.realLength
        try {
            return super.$_get(index)
        } finally {
            this.length_ = 0
        }
    }

    public override $_set(index: int, val: T): void {
        this.setCounter += 1
        this.length_ = this.realLength
        try {
            super.$_set(index, val)
        } finally {
            this.length_ = 0
        }
    }
}

export function getExtendedArray(): ArrayWithCounters<Any> {
    return new ArrayWithCounters<Any>([null, undefined, 'sample'])
}

export function checkChangedArray(arr: ArrayWithCounters<Any>): boolean {
    for (let i = 0; i < 3; i++) {
        if ((arr[i] as int) != i) {
            return false
        }
    }
    return true
}
