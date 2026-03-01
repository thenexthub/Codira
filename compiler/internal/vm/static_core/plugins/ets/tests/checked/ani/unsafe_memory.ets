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

//! CHECKER       Unsafe memory access, INT
//! RUN           options: "--compiler-enable-jit=false", entry: 'unsafe_memory.ETSGLOBAL::main'

//! CHECKER       Unsafe memory access, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*' --compiler-hotness-threshold=0", entry: 'unsafe_memory.ETSGLOBAL::main'

//! CHECKER       Unsafe memory, JIT, cpp only
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*' --compiler-hotness-threshold=1", entry: 'unsafe_memory.ETSGLOBAL::main'

//! CHECKER       Unsafe memory access, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='unsafe_memory.ETSGLOBAL::.*'"
//! RUN           entry: 'unsafe_memory.ETSGLOBAL::main'

import {unsafeMemory} from "std/core"

native function allocMem(size: int) : long;
native function freeMem(addr: long) : void;

function testBoolean() : int {
    let vals: boolean[] = [true, false, false, false, true, true, false, true];
    let size: int = vals.length * Byte.BYTE_SIZE;
    let addr: long = allocMem(size);
    let ptr: long = addr
    for (let i: int  = 0; i < vals.length; ++i) {
        unsafeMemory.writeBoolean(ptr, vals[i]);
        ptr += Byte.BYTE_SIZE
    }
    ptr = addr
    for (let i: int = 0; i < vals.length; ++i) {
        let v: boolean = unsafeMemory.readBoolean(ptr);
        if (v != vals[i]) {
            console.println("expected: " + vals[i] + ", got: " + v);
	    freeMem(addr);
	    return 1;
        }
        ptr += Byte.BYTE_SIZE;
    }
    freeMem(addr);
    return 0;
}

function testInt8() : int {
    let vals: byte[] = [1, -1, 0, 2, -2, 127, -128];
    let size: int = vals.length * Byte.BYTE_SIZE;
    let addr: long = allocMem(size);
    let ptr: long = addr
    for (let i: int  = 0; i < vals.length; ++i) {
        unsafeMemory.writeInt8(ptr, vals[i]);
        ptr += Byte.BYTE_SIZE
    }
    ptr = addr
    for (let i: int = 0; i < vals.length; ++i) {
        let v: byte = unsafeMemory.readInt8(ptr);
        if (v != vals[i]) {
            console.println("expected: " + vals[i] + ", got: " + v);
	    freeMem(addr);
	    return 1;
        }
        ptr += Byte.BYTE_SIZE;
    }
    freeMem(addr);
    return 0;
}

function testInt16() : int {
    let vals: short[] = [1, -1, 0, 2, -2, 0x7fff.toShort(), 0x8000.toShort() ];
    let size: int = vals.length * Short.BYTE_SIZE;
    let addend = Short.BYTE_SIZE - 1
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            unsafeMemory.writeInt16(ptr, vals[i]);
            ptr += Short.BYTE_SIZE
        }
        ptr = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let v: short = unsafeMemory.readInt16(ptr);
            if (v != vals[i]) {
                console.println("expected: " + vals[i] + ", got: " + v);
                freeMem(addr);
                return 1;
            }
            ptr += Short.BYTE_SIZE;
        }
    }
    freeMem(addr);
    return 0;
}

function testInt32() : int {
    let vals: int[] = [1, -1, 0, 2, -2, 0x7fffffff, -0x80000000];
    let size: int = vals.length * Int.BYTE_SIZE;
    let addend = Int.BYTE_SIZE - 1
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int  = 0; i < vals.length; ++i) {
            unsafeMemory.writeInt32(ptr, vals[i]);
            ptr += Int.BYTE_SIZE
        }
        ptr = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let v: int = unsafeMemory.readInt32(ptr);
            if (v != vals[i]) {
                console.println("expected: " + vals[i] + ", got: " + v);
                freeMem(addr);
                return 1;
            }
            ptr += Int.BYTE_SIZE;
        }
    }
    freeMem(addr);
    return 0;
}

function testInt64() : int {
    let vals: long[] = [1, -1, 0, 2, -2, 0x7fffffffffffffff as long, -0x8000000000000000 as long];
    let size: int = vals.length * Long.BYTE_SIZE;
    let addend = Long.BYTE_SIZE - 1
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int  = 0; i < vals.length; ++i) {
            unsafeMemory.writeInt64(ptr, vals[i]);
            ptr += Long.BYTE_SIZE
        }
        ptr = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let v: long = unsafeMemory.readInt64(ptr);
            if (v != vals[i]) {
                console.println("expected: " + vals[i] + ", got: " + v);
                freeMem(addr);
                return 1;
            }
            ptr += Long.BYTE_SIZE;
        }
    }
    freeMem(addr);
    return 0;
}

function testFloat32() : int {
    let vals: float[] = [1.123456f, -1.234567f, 1.2E-30f, -1.2E+30f, 1.23456E+30f, -1.23456E-30f];
    let size: int = vals.length * Float.BYTE_SIZE;
    let addend = Float.BYTE_SIZE - 1
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int  = 0; i < vals.length; ++i) {
            unsafeMemory.writeFloat32(ptr, vals[i]);
            ptr += Float.BYTE_SIZE
        }
        ptr = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let v: float = unsafeMemory.readFloat32(ptr);
            if (v != vals[i]) {
                console.println("expected: " + vals[i] + ", got: " + v);
                freeMem(addr);
                return 1;
            }
            ptr += Float.BYTE_SIZE;
        }
    }
    freeMem(addr);
    return 0;
}

function testFloat64() : int {
    let vals: double[] = [1.23456789, -1.23456789, 1.2E-300, 1.2E+300,
                          1.23456789E+300, -1.23456789E-300];
    let size: int = vals.length * Double.BYTE_SIZE;
    let addend = Double.BYTE_SIZE - 1
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int  = 0; i < vals.length; ++i) {
            unsafeMemory.writeFloat64(ptr, vals[i]);
            ptr += Double.BYTE_SIZE
        }
        ptr = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let v: double = unsafeMemory.readFloat64(ptr);
            if (v != vals[i]) {
                console.println("expected: " + vals[i] + ", got: " + v);
                freeMem(addr);
                return 1;
            }
            ptr += Double.BYTE_SIZE;
        }
    }
    freeMem(addr);
    return 0;
}

function testString() : int {
    let vals: String[] = ["a", "ФФ", "ccc", "ЯЯЯЯ", "12345", "ЯФ2233"];
    let size: int  = 0;
    let sizes: int[] = new int[vals.length]
    for (let i: int = 0; i < vals.length; ++i) {
        sizes[i] = unsafeMemory.getStringSizeInBytes(vals[i]);
        size += sizes[i];
    }
    let addend: int = 15
    let addr: long = allocMem(size + addend);
    for (let l: int = 0; l <= addend; ++l) {
        let ptr: long = addr + l;
        for (let i: int = 0; i < vals.length; ++i) {
            let sz: int = unsafeMemory.writeString(ptr, vals[i]);
            if(sz != sizes[i]) {
                console.println("value: " + vals[i] + ", expected size: " + sizes[i] + ", got: " + sz);
                freeMem(addr);
                return 1;
            }
            ptr += sz;
        }
        ptr = addr + l;
        let err: int = 0;
        for (let i: int = 0; i < vals.length; ++i) {
            let s: String = unsafeMemory.readString(ptr, sizes[i]);
            if (s.getLength() != vals[i].getLength() || s != vals[i]) {
                console.println("Error: got size: " + s.getLength() + ", expected: " +  vals[i].getLength() + ", got value: " + s + ", expected: " + vals[i]);
                freeMem(addr);
                return 1;
            }
            ptr += sizes[i]
        }
    }
    freeMem(addr);
    return 0;
}
function main(): int {
    loadLibrary("checked_test_unsafe_memory");

    arktest.assertEQ(testBoolean(), 0, "boolean test failed");
    arktest.assertEQ(testInt8(), 0, "int8 test failed");
    arktest.assertEQ(testInt16(), 0, "int16 test failed");
    arktest.assertEQ(testInt32(), 0, "int32 test failed");
    arktest.assertEQ(testInt64(), 0, "int64 test failed");
    arktest.assertEQ(testFloat64(), 0, "float64 test failed");
    arktest.assertEQ(testFloat32(), 0, "float32 test failed");
    arktest.assertEQ(testString(), 0, "string test failed");

    return 0;
}

