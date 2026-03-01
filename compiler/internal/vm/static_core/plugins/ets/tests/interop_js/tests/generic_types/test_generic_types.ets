/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package generic_types.test;


class EtsGenericValueHandle<T> {
    constructor(value: T) {
        this.value = value;
    }
    public setValue(value: T) {
        this.value = value;
    }
    public getValue(): T {
        return this.value;
    }
    public value: T;
}

class EtsGenericErrorHandle<T extends Error> {
    constructor(err: T) {
        this.err = err;
    }
    public getErrorMessage(): String {
        return this.err.message;
    }
    public err: T;
}

function etsGetGenericValueIdentity<T>(v: T): T {
    return v;
}

function etsGetGenericErrorIdentity<T extends Error>(e: T): T {
    return e;
}
