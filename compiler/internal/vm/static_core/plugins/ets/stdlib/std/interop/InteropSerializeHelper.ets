/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package std.interop;

export enum InteropArgType {
    DIRECT = 0,
    UNWRAP = 1,
    JSVALUE = 2
}

export class InteropArgRecord {
    constructor() {
        this.type = InteropArgType.DIRECT;
        this.value = undefined;
    }

    constructor(type: InteropArgType, value: Any) {
        this.type = type;
        this.value = value;
    }

    type: InteropArgType;
    value: Any;
}

// This class will be internal in future. #27641
export class InteropSerializeHelper {
    constructor(args: FixedArray<Any>) {
        this.argsHolder = args;
        this.argRecords = new FixedArray<InteropArgRecord>(args.length);
        for (let i = 0; i < args.length; i++) {
            this.argRecords[i] = this.classifyAndSerializeArg(args[i]);
        }
    }

    public deserialize(): FixedArray<Any> {
        let res: FixedArray<Any> = new FixedArray<Any>(this.argRecords.length);
        for (let i = 0; i < this.argRecords.length; i++) {
            let type = this.argRecords[i].type;
            let value = this.argRecords[i].value;
            if (type == InteropArgType.JSVALUE) {
                res[i] = InteropSerializeHelper.deserializeHandle(value as long);
            } else if (type == InteropArgType.UNWRAP) {
                let deser = InteropSerializeHelper.deserializeHandle(value as long);
                res[i] = ESValue.wrap(deser);
            } else {
                res[i] = value;
            }
        }
        return res;
    }

    private classifyAndSerializeArg(arg: Any): InteropArgRecord {
        if (arg == undefined) {
            return new InteropArgRecord(InteropArgType.DIRECT, arg);
        }
        if (arg instanceof ESValue && !arg.isUndefined() && !arg.isNull()) {
            let unwrapped = (arg as ESValue).unwrap();
            if (unwrapped != undefined && !arg.isUndefined() && InteropSerializeHelper.isJSInteropRef(unwrapped)) {
                let ser = InteropSerializeHelper.serializeHandle(unwrapped as JSValue);
                return new InteropArgRecord(InteropArgType.UNWRAP, ser);
            }
        } else if (InteropSerializeHelper.isJSInteropRef(arg)) {
            let ser = InteropSerializeHelper.serializeHandle(arg as JSValue);
            return new InteropArgRecord(InteropArgType.JSVALUE, ser);
        }

        return new InteropArgRecord(InteropArgType.DIRECT, arg);
    }

    // Holds args reference for GC (keep alive)
    argsHolder: FixedArray<Any>;
    argRecords: FixedArray<InteropArgRecord>;

    public static native isJSInteropRef(value: Any): boolean;
    public static native serializeHandle(value: JSValue): long;
    public static native deserializeHandle(value: long): JSValue;
}
