/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

package std.core;


export interface ArrayBufferView {
    /**
     * The ArrayBuffer instance referenced by the array.
     */
    readonly buffer: ArrayBuffer

    /**
     * The length in bytes of the array.
     */
    readonly byteLength: int

    /**
     * The offset in bytes of the array.
     */
    readonly byteOffset: int
}

/**
 * JS ArrayBuffer API-compatible class
 */
export class ArrayBuffer
{
    toString(): String {
        return "[object ArrayBuffer]";
    }

    /**
     * Creates ArrayBuffer with size equal to length parameter
     *
     * @param length size of ArrayBuffer
     */
    public constructor(length: int, maxByteLength?: int)
    {
        super()
        if (length < 0) {
            throw new RangeError("length can't be less than 0")
        }
        if (maxByteLength != undefined) {
            if (maxByteLength! < 0) {
                throw new RangeError("maxByteLength can't be less than 0")
            }
            if (length > maxByteLength) {
                throw new RangeError("length can't be greater than maxByteLength")
            }
        }
        this.isResizable = (maxByteLength != undefined)
        this.data = new FixedArray<byte>(this.isResizable ? maxByteLength! : length)
        this._byteLength = length
        this.dataAddress = 0 // not needed in managed code
    }

    /**
     * Creates ArrayBuffer with size equal to length parameter
     *
     * @param length size of ArrayBuffer
     */
    public constructor(length: number, maxByteLength?: number)
    {
        this(length.toInt(), (maxByteLength !== undefined) ? maxByteLength.toInt() : undefined)
    }

    /**
     * Checks if the passed Object is a View
     *
     * @param obj to check
     *
     * @returns true if obj is instance of typed array
     */
    public static isView(obj: Object): boolean
    {
        return ( obj instanceof DataView
            || obj instanceof Int8Array
            || obj instanceof Int16Array
            || obj instanceof Int32Array
            || obj instanceof BigInt64Array
            || obj instanceof Float32Array
            || obj instanceof Float64Array
            || obj instanceof Uint8Array
            || obj instanceof Uint8ClampedArray
            || obj instanceof Uint16Array
            || obj instanceof Uint32Array
            || obj instanceof BigUint64Array)
    }

    sliceInternal(begin: int, end: int): ArrayBuffer {
        const byteLength = this.getByteLength()

        const startIndex = normalizeIndex(begin, byteLength)
        const endIndex = normalizeIndex(end, byteLength)

        let resultLength = endIndex - startIndex
        if (resultLength < 0) {
            resultLength = 0
        }

        let result = new ArrayBuffer(resultLength)

        if (resultLength == 0) return result
        result.setValues(this, startIndex)

        return result
    }

    /**
     * Creates a new ArrayBuffer with copy of bytes in range [begin;end)
     *
     * @param begin an inclusive index to start copying with
     *
     * @param end a last exclusive index to stop copying
     *
     * @returns data taken from current ArrayBuffer with respect to begin and end parameters
     */
    public slice(begin: int, end?: int): ArrayBuffer {
        if (this.detached) {
            throw new TypeError("ArrayBuffer was detached")
        }
        if (end == undefined) return this.sliceInternal(begin, this.getByteLength()) as ArrayBuffer
        else return this.sliceInternal(begin, end) as ArrayBuffer
    }

    /**
     * Creates a new ArrayBuffer with copy of bytes in range [begin;end)
     *
     * @param begin an inclusive index to start copying with
     *
     * @param end a last exclusive index to stop copying
     *
     * @returns data taken from current ArrayBuffer with respect to begin and end parameters
     */
    public slice(begin: number, end?: number): ArrayBuffer {
        if (end == undefined) return this.slice(begin.toInt())
        else return this.slice(begin.toInt(), end.toInt())
    }

    /**
     * Returns data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param i index
     *
     * @returns byte at index
     *
     * @throws {IndexOutOfBoundsError} if `pos` is out of bound
     */
    public final at(pos: int): byte {
        if (this.data !== undefined) {
            this.doBoundaryCheck(pos)
            // NOTE(dslynko, #24647) research performance of managed code compared to intrinsics
            return this.data![pos]
        }
        // Fallback to access through native pointer
        return this.atImpl(pos)
    }

    /**
     * Sets data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param b new value
     *
     * @param i index
     *
     * @throws {IndexOutOfBoundsError} if `pos` is out of bound
     */
    public final set(pos: int, val: byte): void {
        if (this.data !== undefined) {
            this.doBoundaryCheck(pos)
            // NOTE(dslynko, #24647) research performance of managed code compared to intrinsics
            this.data![pos] = val
        } else {
            // Fallback to access through native pointer
            this.setImpl(pos, val)
        }
    }

    public getByteLength(): int {
        return this._byteLength
    }

    get byteLength(): int {
        return this._byteLength
    }

    /**
     * Checks managed data availability first. Otherwise, it checks if there is native data.
     * ArrayBuffer with managed data cannot be detached.
     */
    get detached(): boolean {
        return this.data != undefined ? false : this.dataAddress == 0
    }

    get resizable(): boolean {
        return this.isResizable
    }

    /**
     * Performs boundary check due to `data` array possibly shrinking down
     *
     * @param pos index to be checked before accessing the underlying buffer
     */
    private final doBoundaryCheck(pos: int): void {
        if (pos < 0 || pos >= this._byteLength) {
            throw new IndexOutOfBoundsError("ArrayBuffer position " + pos + " is out of bounds")
        }
    }

    /**
     * Copies into `this` array `this.byteLength` bytes starting from `begin` index of `other` data.
     * Always valid for a non-detached ArrayBuffer.
     *
     * @param other ArrayBuffer with source data
     *
     * @param begin index inside `other` buffer
     */
    private final native setValues(other: ArrayBuffer, begin: int): void

    /**
     * Reads byte from specified position. Always valid for a non-detached ArrayBuffer
     *
     * @param pos index to read value from
     */
    private final native atImpl(pos: int): byte

    /**
     * Writes byte at specified position. Always valid for a non-detached ArrayBuffer
     *
     * @param pos index to write value into
     */
    private final native setImpl(pos: int, val: byte): void

    /**
     * Resizes the ArrayBuffer
     *
     * @param newLen new length
     */
    public resize(newLen : int): void {
        if (this.detached) {
            throw new TypeError("ArrayBuffer was detached")
        }
        if (!this.resizable) {
            throw new TypeError("ArrayBuffer is not resizable")
        }
        if (newLen < 0 || newLen > this.data!.length) {
            throw new RangeError("new length cannot be larger than maxByteLength")
        }
        this._byteLength = newLen
    }

    /**
     * Allocates non-movable array of fixed length
     */
    private native static allocateNonMovable(length: int): FixedArray<byte>

    /**
     * Returns native address of non-movable array
     *
     * @param array FixedArray to read native address from, must be allocated in non-movable memory
     */
    private native static getAddress(array: FixedArray<byte>): long

    /**
    * Creates a new ArrayBuffer from an array of bytes.
    *
    * This native method initializes an ArrayBuffer using the provided byte array as the underlying data.
    *
    * @param {byte[]} arr - An array of bytes to initialize the ArrayBuffer.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the byte array.
    */
    public static from(arr: FixedArray<byte>): ArrayBuffer {
        const bf = new ArrayBuffer(arr.length)
        copyTo(arr, bf.data!, 0, 0, arr.length)
        return bf
    }

    /**
    * Creates a new ArrayBuffer from a Uint8Array.
    *
    * This method extracts the underlying ArrayBuffer from the provided Uint8Array and converts it
    * to an ArrayBuffer using the native conversion for byte arrays.
    *
    * @param {Uint8Array} u8arr - A typed array of unsigned 8-bit integers.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the same binary data as the Uint8Array.
    */
    public static from(u8arr: Uint8Array): ArrayBuffer {
        const arrBuff = u8arr.buffer as ArrayBuffer
        if (arrBuff.detached) {
            return new ArrayBuffer(0)
        }
        const result = new ArrayBuffer(arrBuff.byteLength)
        result.setValues(arrBuff, 0)
        return result
    }

    /**
    * Creates a new ArrayBuffer from an array of numbers.
    *
    * The numbers in the provided array are converted to bytes, and then a new ArrayBuffer is constructed
    * using these bytes.
    *
    * @param {double[]} array - An array of numbers representing byte values.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the array.
    */
    public static from(array: double[]): ArrayBuffer {
        const buf = new ArrayBuffer(array.length)
        for (let i = 0; i < array.length; ++i) {
            buf.set(i, array[i].toByte())
        }
        return buf
    }

    /**
    * Creates a new ArrayBuffer from a string using the specified encoding.
    *
    * This native method encodes the given string using the provided encoding and creates an
    * ArrayBuffer with the resulting bytes. The resulting ArrayBuffer is expected to have the
    * specified length.
    *
    * @param {string} str - The string to be converted.
    * @param {string} encoding - The encoding to use for converting the string.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the encoded string data.
    */
    public static native from(str: string, encoding: string): ArrayBuffer;

    /**
    * Creates a new ArrayBuffer by copying a segment from an existing ArrayBuffer.
    *
    * This native method copies a range of bytes from the source ArrayBuffer, starting at the
    * specified byte offset and continuing for the specified length, into a new ArrayBuffer.
    *
    * @param {ArrayBuffer} buff - The source ArrayBuffer.
    * @param {int} byteOffset - The starting byte index in the source ArrayBuffer.
    * @param {int} length - The number of bytes to copy.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the specified slice of the source.
    */
    public static native from(buff: ArrayBuffer, byteOffset: int, length: int): ArrayBuffer;

    /**
    * Creates a new ArrayBuffer by slicing a segment from an existing ArrayBuffer.
    *
    * If the byte offset or length parameters are omitted, they default to 0 and the full length of the
    * source ArrayBuffer, respectively.
    *
    * @param {ArrayBuffer} buffer - The source ArrayBuffer.
    * @param {number} [byteOffset=0] - The starting byte index in the source ArrayBuffer.
    * @param {number} [length=buffer.byteLength] - The number of bytes to copy.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the specified slice of the source.
    */
    public static from(buffer: ArrayBuffer, byteOffset?: number, length?: number): ArrayBuffer {
        const resolvedByteOffset = byteOffset ?? 0;
        const resolvedLength = length ?? buffer.byteLength.toDouble();
        return ArrayBuffer.from(buffer, resolvedByteOffset.toInt(), resolvedLength.toInt());
    }

    /**
    * Creates a new ArrayBuffer from an object, interpreting the second parameter as either a byte offset or an encoding.
    *
    * When the object is a string and the second parameter is also a string, the method treats the inputs as
    * parameters for string conversion (using the specified encoding and length). If the object is an ArrayBuffer,
    * it copies a segment starting from the provided byte offset for the specified length.
    *
    * @param {Object} obj - The source object, which should be either a string or an ArrayBuffer.
    * @param {int | string} byteOffsetOrEncoding - If `obj` is an ArrayBuffer, this is the starting byte offset;
    *                                                 if `obj` is a string, this is the encoding to use.
    * @param {int} length - The number of bytes to copy or the desired length of the encoded string data.
    * @returns {ArrayBuffer} A new ArrayBuffer created from the provided object.
    * @throws {Error} If the input object type is unsupported or the arguments are incorrect.
    */
    public static fromObject(obj: Object, byteOffsetOrEncoding: int | string, length: int): ArrayBuffer {
        if (obj instanceof string) {
            const resolvedEncoding: string = byteOffsetOrEncoding instanceof string ? byteOffsetOrEncoding as string : "utf8";
            const resolvedbyteOffset: int = byteOffsetOrEncoding instanceof int ? byteOffsetOrEncoding as int : 0;
            return ArrayBuffer.from(ArrayBuffer.from(obj, resolvedEncoding), resolvedbyteOffset as int, length);
        }
        if (!(obj instanceof ArrayBuffer)) {
            throw Error("Incorrect args");
        }
        return ArrayBuffer.from(obj, byteOffsetOrEncoding as int, length);
    }


    // Fields' order is important due to alignment
    private data: FixedArray<byte> | undefined
    private weakRef: Object | undefined
    /**
     * Pointer to underlying array, which is either non-movable array or native.
     * Not needed in managed code.
     */
    private dataAddress: long
    // Length in bytes
    private _byteLength: int
    private isResizable: boolean


    /**
    * Returns the number of bytes required to encode a given string using the specified encoding.
    *
    * This native method calculates the byte length of the provided text when it is encoded with the
    * specified character encoding. Note that the byte length may differ from the number of characters
    * in the string.
    *
    * @param {string} text - The string to evaluate.
    * @param {string} encoding - The encoding used to convert the string to bytes.
    * @returns {int} The number of bytes required to encode the string.
    */
    public static native bytesLength(text: string, encoding: string): int;

    /**
    * Decodes a segment of an ArrayBuffer to a string using the specified encoding.
    *
    * This native method converts a portion of the ArrayBuffer, starting at the given index and ending
    * before the specified end index, into a string. The decoding is performed using the provided encoding.
    *
    * @param {ArrayBuffer} buffer - The ArrayBuffer containing the data to decode.
    * @param {string} encoding - The encoding to use for decoding the data.
    * @param {int} start - The starting byte index (inclusive) from which to begin decoding.
    * @param {int} end - The ending byte index (exclusive) at which to stop decoding.
    * @returns {string} The decoded string representation of the specified segment of the ArrayBuffer.
    */
    public static native stringify(buffer: ArrayBuffer, encoding: string, start: int, end: int): string;

    private constructor createExternal(data: long, length: int) {
        // This constructor must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("External ArrayBuffer can be instantiated only from boot context")
        }
        this.data = undefined
        this.weakRef = undefined
        this.dataAddress = data
        this._byteLength = length
        this.isResizable = false
    }

    private constructor createFinalizable(data: long, length: int, finalizer: long, finalizerData: long) {
        // This constructor must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("External ArrayBuffer can be instantiated only from boot context")
        }
        this.data = undefined
        this.weakRef = ArrayBuffer.registerWeakRef(this, finalizer, finalizerData)
        this.dataAddress = data
        this._byteLength = length
        this.isResizable = false
    }

    /**
     * Detaches finalizer from `this` and returns the argument which was stored inside
     */
    private detach(): long {
        // This method must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("ArrayBuffer can be detached only from boot context")
        }
        // Check that ArrayBuffer is external and was not detached yet
        if (this.data != undefined || this.detached) {
            return 0
        }
        this.dataAddress = 0
        this._byteLength = 0
        // Need to unregister finalizer to avoid double free
        let finalizerData: long = 0
        if (this.weakRef != undefined) {
            finalizerData = ArrayBuffer.unregisterWeakRef(this.weakRef!)
            if (finalizerData == 0) {
                throw new RuntimeError("Failed to unregister finalizer")
            }
        }
        this.weakRef = undefined
        return finalizerData
    }

    /**
     * Registers `FinalizableWeakRef` for the given ArrayBuffer instance
     */
    private static native registerWeakRef(self: ArrayBuffer, finalizer: long, finalizerData: long): Object

    /**
     * Unregisters `FinalizableWeakRef` instance and returns the finalizer argument which was stored inside
     */
    private static native unregisterWeakRef(weakRef: Object): long

    public native atomicAddI8(index: int, byteOffset: int, value: byte): long;

    public native atomicAndI8(index: int, byteOffset: int, value: byte): long;

    public native atomicCompareExchangeI8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): long;

    public native atomicExchangeI8(index: int, byteOffset: int, value: byte): long;

    public native atomicLoadI8(index: int, byteOffset: int): long;

    public native atomicOrI8(index: int, byteOffset: int, value: byte): long;

    public native atomicStoreI8(index: int, byteOffset: int, value: byte): long;

    public native atomicSubI8(index: int, byteOffset: int, value: byte): long;

    public native atomicXorI8(index: int, byteOffset: int, value: byte): long;

    public native atomicAddI16(index: int, byteOffset: int, value: short): long;

    public native atomicAndI16(index: int, byteOffset: int, value: short): long;

    public native atomicCompareExchangeI16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): long;

    public native atomicExchangeI16(index: int, byteOffset: int, value: short): long;

    public native atomicLoadI16(index: int, byteOffset: int): long;

    public native atomicOrI16(index: int, byteOffset: int, value: short): long;

    public native atomicStoreI16(index: int, byteOffset: int, value: short): long;

    public native atomicSubI16(index: int, byteOffset: int, value: short): long;

    public native atomicXorI16(index: int, byteOffset: int, value: short): long;

    public native atomicAddI32(index: int, byteOffset: int, value: int): long;

    public native atomicAndI32(index: int, byteOffset: int, value: int): long;

    public native atomicCompareExchangeI32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): long;

    public native atomicExchangeI32(index: int, byteOffset: int, value: int): long;

    public native atomicLoadI32(index: int, byteOffset: int): long;

    public native atomicOrI32(index: int, byteOffset: int, value: int): long;

    public native atomicStoreI32(index: int, byteOffset: int, value: int): long;

    public native atomicSubI32(index: int, byteOffset: int, value: int): long;

    public native atomicXorI32(index: int, byteOffset: int, value: int): long;

    public native atomicAddI64(index: int, byteOffset: int, value: long): long;

    public native atomicAndI64(index: int, byteOffset: int, value: long): long;

    public native atomicCompareExchangeI64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): long;

    public native atomicExchangeI64(index: int, byteOffset: int, value: long): long;

    public native atomicLoadI64(index: int, byteOffset: int): long;

    public native atomicOrI64(index: int, byteOffset: int, value: long): long;

    public native atomicStoreI64(index: int, byteOffset: int, value: long): long;

    public native atomicSubI64(index: int, byteOffset: int, value: long): long;

    public native atomicXorI64(index: int, byteOffset: int, value: long): long;

    public native atomicAddU8(index: int, byteOffset: int, value: byte): long;

    public native atomicAndU8(index: int, byteOffset: int, value: byte): long;

    public native atomicCompareExchangeU8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): long;

    public native atomicExchangeU8(index: int, byteOffset: int, value: byte): long;

    public native atomicLoadU8(index: int, byteOffset: int): long;

    public native atomicOrU8(index: int, byteOffset: int, value: byte): long;

    public native atomicStoreU8(index: int, byteOffset: int, value: byte): long;

    public native atomicSubU8(index: int, byteOffset: int, value: byte): long;

    public native atomicXorU8(index: int, byteOffset: int, value: byte): long;

    public native atomicAddU16(index: int, byteOffset: int, value: short): long;

    public native atomicAndU16(index: int, byteOffset: int, value: short): long;

    public native atomicCompareExchangeU16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): long;

    public native atomicExchangeU16(index: int, byteOffset: int, value: short): long;

    public native atomicLoadU16(index: int, byteOffset: int): long;

    public native atomicOrU16(index: int, byteOffset: int, value: short): long;

    public native atomicStoreU16(index: int, byteOffset: int, value: short): long;

    public native atomicSubU16(index: int, byteOffset: int, value: short): long;

    public native atomicXorU16(index: int, byteOffset: int, value: short): long;

    public native atomicAddU32(index: int, byteOffset: int, value: int): long;

    public native atomicAndU32(index: int, byteOffset: int, value: int): long;

    public native atomicCompareExchangeU32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): long;

    public native atomicExchangeU32(index: int, byteOffset: int, value: int): long;

    public native atomicLoadU32(index: int, byteOffset: int): long;

    public native atomicOrU32(index: int, byteOffset: int, value: int): long;

    public native atomicStoreU32(index: int, byteOffset: int, value: int): long;

    public native atomicSubU32(index: int, byteOffset: int, value: int): long;

    public native atomicXorU32(index: int, byteOffset: int, value: int): long;

    public native atomicAddU64(index: int, byteOffset: int, value: long): long;

    public native atomicAndU64(index: int, byteOffset: int, value: long): long;

    public native atomicCompareExchangeU64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): long;

    public native atomicExchangeU64(index: int, byteOffset: int, value: long): long;

    public native atomicLoadU64(index: int, byteOffset: int): long;

    public native atomicOrU64(index: int, byteOffset: int, value: long): long;

    public native atomicStoreU64(index: int, byteOffset: int, value: long): long;

    public native atomicSubU64(index: int, byteOffset: int, value: long): long;

    public native atomicXorU64(index: int, byteOffset: int, value: long): long;
}

export type ArrayBufferLike = ArrayBuffer
