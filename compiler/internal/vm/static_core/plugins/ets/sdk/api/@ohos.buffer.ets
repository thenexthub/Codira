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

import { BusinessError } from "@ohos.base";
import { jsonx } from "std/core"

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;
const IncorrectBufferSizeId: int = 10200009;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

function getArrayBufferFrom(value: buffer.Buffer | Uint8Array): ArrayBuffer {
    if (value instanceof Uint8Array) {
        return ArrayBuffer.from(value);
    }
    return value.buffer;
}

function normalizeBase64Url(str: string): string {
    return str
        .replace(new RegExp('-', 'g'), '+')
        .replace(new RegExp('_', 'g'), '/');
}

function sanitizeBase64(str: string, urlSafe: boolean = false): string {
    const allowedPattern = urlSafe
        ? "[^A-Za-z0-9\\-_=]"
        : "[^A-Za-z0-9+/=]";
    return str.replace(new RegExp(allowedPattern, "g"), "");
}

function getLength(value: buffer.Buffer | Uint8Array): number {
    if (value instanceof Uint8Array) {
        return value.length;
    }
    return value.buffer.byteLength;
}

function stringify(buffer: ArrayBuffer, encoding: string, resolvedStart: int, resolvedEnd: int): string {
    let val  = '';
    if (encoding == "utf8" || encoding == "utf-8" || encoding == "ascii" || encoding === "latin1") {
        return ArrayBuffer.stringify(buffer, encoding, resolvedStart, resolvedEnd);
    } else if (encoding == "utf16le" || encoding == "utf-16le" || encoding == "ucs2" || encoding == "ucs-2") {
        for (let i = resolvedStart; i + 1 < resolvedEnd; i += 2) { // 2 is array->NextIndex
            const a: Int = (buffer.at(i) & 0xff);
            const b: Int = (buffer.at(i + 1) & 0xff);
            val += String.fromCharCode((b << 8) + a);
        }
    } else if (encoding === "binary") {
        for (let i = resolvedStart; i < resolvedEnd; i++) {
            val += String.fromCharCode(+(buffer.at(i) & 0xff));
        }
    } else if (encoding == "base64" || encoding == "base64url") {
        let base64 = '';
        let encodings = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
        if (encoding == "base64url") {
            encodings = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
        }
        for (let i = resolvedStart; i < resolvedEnd; i += 3) {
            const byte1 = buffer.at(i) & 0xff;
            const byte2 = i + 1 < resolvedEnd ? (buffer.at(i + 1) & 0xff) : NaN;
            const byte3 = i + 2 < resolvedEnd ? (buffer.at(i + 2) & 0xff) : NaN;

            const enc1 = byte1 >> 2;
            const enc2 = (((byte1 & 3) << 4) | (byte2 >> 4)).toInt();
            const enc3 = (((byte2 & 15) << 2) | (byte3 >> 6)).toInt();
            const enc4 = (byte3 & 63).toInt();

            if (isNaN(byte2)) {
                base64 += encodings.charAt(enc1).toString() + encodings.charAt(enc2).toString() + '==';
            } else if (isNaN(byte3)) {
                base64 += encodings.charAt(enc1).toString() + encodings.charAt(enc2).toString() + encodings.charAt(enc3).toString() + '=';
            } else {
                base64 += encodings.charAt(enc1).toString() + encodings.charAt(enc2).toString() + encodings.charAt(enc3).toString() + encodings.charAt(enc4).toString();
            }
        }
        if (encoding == "base64url") {
            let lastNonEqualPos = base64.length - 1;
            while (lastNonEqualPos >= 0 && base64.charAt(lastNonEqualPos) == c'=') {
                lastNonEqualPos--;
            }
            if (lastNonEqualPos >= 0) {
                base64 = base64.substring(0, lastNonEqualPos + 1);
            }
        }
        return base64;
    } else if (encoding == "hex") {
        for (let i = resolvedStart, len = resolvedEnd; i < len; i++) {
            let tmpstr = Number(buffer.at(i) & 0xff).toString(16); // 16 : 16 decimal
            tmpstr = (tmpstr.length === 1) ? `0${tmpstr}` : tmpstr;
            val += tmpstr;
        }
    }
    return val;
}

function fillInPaddingBase64(str: string): string {
    const base64Divisible: int = 4;
    const normalized: String = normalizeBase64Url(str);
    const clean: String = sanitizeBase64(normalized);
    const remainder: int = clean.length.toInt() % base64Divisible;
    if (remainder === 1) {
        return clean.slice(0, -1);
    }
    const paddingCount: int = remainder != 0 ? base64Divisible - remainder : 0;
    return clean.padRight(c'=', clean.length.toInt() + paddingCount);
}

function takeValidHexPrefix(str: string): string {
    const hexPairRegex = new RegExp('^[0-9A-Fa-f]{2}$');
    let prefix = '';
    for (let i = 0; i + 1 < str.length; i += 2) {
        const pair = str.slice(i, i + 2);
        if (hexPairRegex.test(pair)) {
            prefix += pair;
        } else {
            break;
        }
    }
    return prefix;
}

function sanitizeAscii(str: string): string {
    let result = '';
    for (let i = 0; i < str.length; i++) {
        result += String.fromCharCode(str.charCodeAt(i) & 0x7F);
    }
    return result;
}

function sanitizeUtf8(str: string): string {
    const sb = new StringBuilder();
    for (let i = 0; i < str.length; i++) {
        const cu = str.charCodeAt(i).toInt().toChar();
        if (Char.isHighSurrogate(cu) && i + 1 < str.length) {
            const cu2 = str.charCodeAt(i + 1).toInt().toChar();
            if (Char.isLowSurrogate(cu2)) {
                sb.append(str.charAt(i));
                sb.append(str.charAt(i + 1));
                i++;
                continue;
            }
        }
        if (Char.isHighSurrogate(cu) || Char.isLowSurrogate(cu)) {
            sb.append('\uFFFD');
        } else {
            sb.append(str.charAt(i));
        }
    }
    return sb.toString();
}

function sanitizeLatin1(str: string): string {
    const sb = new StringBuilder();
    for (let i = 0; i < str.length; i++) {
        const low = str.charCodeAt(i) & 0xFF;
        sb.append(String.fromCharCode(low));
    }
    return sb.toString();
}

function checkLengthOffset(offset: int, length: int, objLength: int): int{
    let resolvedLength: int = length
    if (length < 0 || length > objLength - offset) {
        resolvedLength = objLength > offset ? objLength - offset : 0;
    }
    return resolvedLength;
}

/**
 * Namespace containing Buffer implementation and related utilities
 * for handling binary data and various encoding operations.
 */
export default namespace buffer {

    /**
     * Supported character encodings for Buffer operations
     */

    export type BufferEncoding = 'ascii'
        | 'utf8'
        | 'utf-8'
        | 'utf16le'
        | 'ucs2'
        | 'ucs-2'
        | 'base64'
        | 'base64url'
        | 'latin1'
        | 'binary'
        | 'hex';

    type TypedArray = Int8Array
        | Uint8Array
        | Uint8ClampedArray
        | Int16Array
        | Uint16Array
        | Int32Array
        | Uint32Array
        | Float32Array
        | Float64Array
        | BigInt64Array
        | BigUint64Array;

    type supportedObjectType = ArrayBuffer | string | Uint8Array

    /**
     * Allocates a new Buffer using an array of bytes in the range 0 – 255.
     * Array entries outside that range will be truncated to fit into it.
     *
     * @param {double[]} array - An array of bytes in the range 0 – 255
     * @returns {Buffer} A new allocated Buffer containing the array data
     */
    export function from(array: double[]): Buffer {
        let arr: FixedArray<byte> = new FixedArray<byte>(array.length)
        for (let i = 0; i < array.length; ++i) {
            arr[i] = array[i].toByte()
        }
        return new Buffer(ArrayBuffer.from(arr))
    }

    /**
     * Creates a view of the ArrayBuffer without copying the underlying memory.
     *
     * @param {ArrayBuffer} arrayBuffer - The source ArrayBuffer to create a view from
     * @param {int} [byteOffset=0] - Index of first byte to expose
     * @param {int} [length=arrayBuffer.byteLength - byteOffset] - Number of bytes to expose
     * @returns {Buffer} A view of the ArrayBuffer
     * @throws {Error} If byteOffset or length are out of valid range
     */
    export function from(arrayBuffer: ArrayBuffer, byteOffset?: int, length?: int): Buffer {
        const resolvedByteOffset: int = byteOffset ?? 0;
        const resolvedLength: int = length ?? arrayBuffer.byteLength.toInt() - resolvedByteOffset;
        if (byteOffset != undefined) {
            if (byteOffset < 0 || byteOffset > arrayBuffer.byteLength) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "byteOffset" is out of range. `+
                    `It must be >= 0 and <= ${arrayBuffer.byteLength}. Received value is: ${byteOffset}`)
            }
        }
        if (length != undefined) {
            if (length < 0 || length > arrayBuffer.byteLength.toInt() - resolvedByteOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "length" is out of range. `+
                    `It must be >= 0 and <= ${arrayBuffer.byteLength.toInt() - resolvedByteOffset}. Received value is: ${length}`)
            }
            return new Buffer(arrayBuffer, resolvedByteOffset, length)
        }
        let buffer = new Buffer(arrayBuffer, resolvedByteOffset)
        return buffer
    }

    /**
     * Copies the passed buffer data onto a new Buffer instance.
     *
     * @param {Buffer | Uint8Array} buffer - An existing Buffer or Uint8Array from which to copy data
     * @returns {Buffer} A new Buffer containing a copy of the provided buffer's data
     */
    export function from(buff: Buffer | Uint8Array): Buffer {
        if (buff instanceof Buffer) {
            const arrBuff: ArrayBuffer = (buff as Buffer).buffer;
            return new Buffer(ArrayBuffer.from(arrBuff));
        }
        const u8arr = buff as Uint8Array;
        return new Buffer(u8arr.buffer as ArrayBuffer);
    }

    /**
    * For the object whose value returned by valueof() function is strictly equal to object
    * or supports symbol To primitive object, a new buffer instance is created.
    *
    * @param { Object } input - object object An object supporting Symbol.toPrimitive or valueOf()
    * @param { int | string } offsetOrEncoding - offsetOrEncoding offsetOrEncoding A byte-offset or encoding
    * @param { int } length - length length A length
    * @returns { Buffer } Return a new allocated Buffer
    */
    export function from(input: Object, offsetOrEncoding: int | string, length: int): Buffer {
        if (offsetOrEncoding instanceof string && (!isEncoding(offsetOrEncoding))) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "encoding" must be BufferEncoding. ` +
                `the encoding ${offsetOrEncoding} is unknown`);
        }
        if(!(input instanceof supportedObjectType)) {
            throw createBusinessError(TypeErrorCodeId,
                'Parameter error. The type of "input" must be Buffer or ArrayBuffer, Array, Array-like');
        }
        let resolvedByteOffset: int = (offsetOrEncoding instanceof string) ? 0 : offsetOrEncoding;
        let resolvedLength: int = length;
        if (input instanceof string) {
            resolvedLength = checkLengthOffset(resolvedByteOffset, resolvedLength, (input as string).length)
            return new Buffer(ArrayBuffer.fromObject(input, offsetOrEncoding, resolvedLength));
        }
        if (input instanceof ArrayBuffer) {
            resolvedLength = checkLengthOffset(resolvedByteOffset, resolvedLength, (input as ArrayBuffer).byteLength)
                return new Buffer(ArrayBuffer.fromObject(input, offsetOrEncoding, resolvedLength));
        } 
        resolvedLength = checkLengthOffset(resolvedByteOffset, resolvedLength, (input as Uint8Array).byteLength)
        if (offsetOrEncoding instanceof string) {
            return new Buffer(ArrayBuffer.fromObject((input as Uint8Array).buffer, offsetOrEncoding, resolvedLength));
        }
        return new Buffer(ArrayBuffer.from(input as Uint8Array));
    }

    /**
     * Creates a new Buffer containing the provided string encoded using the specified encoding.
     *
     * @param {string} input - The string to encode into the buffer
     * @param {BufferEncoding} [encoding='utf8'] - The character encoding to use
     * @returns {Buffer} A new Buffer containing the encoded string
     */
    export function from(input: string, encoding?: BufferEncoding): Buffer {
        const resolvedEncoding: string = (encoding ?? "utf8");
        let resolvedString = input;
        if (resolvedEncoding == 'ascii') {
            resolvedString = sanitizeAscii(resolvedString);
        } else if (resolvedEncoding == 'utf8' || resolvedEncoding == 'utf-8') {
            resolvedString = sanitizeUtf8(resolvedString);
        } else if (resolvedEncoding == 'latin1' || resolvedEncoding == 'binary') {
            resolvedString = sanitizeLatin1(resolvedString);
        } else if (resolvedEncoding == 'hex') {
            resolvedString = takeValidHexPrefix(resolvedString);
            if (resolvedString.length == 0) {
                return new Buffer(new ArrayBuffer(0));
            }
        } else if (encoding == 'base64' || encoding == 'base64url') {
            resolvedString = fillInPaddingBase64(input);
        } else if (resolvedEncoding == 'utf16le' || resolvedEncoding == 'ucs2' || resolvedEncoding == 'ucs-2') {
            return new Buffer(ArrayBuffer.from(resolvedString, resolvedEncoding));
        } else {
            throw createBusinessError(TypeErrorCodeId, 'Parameter error. The type of "encoding" must be BufferEncoding. the encoding test is unknown');
        }
        return new Buffer(ArrayBuffer.from(resolvedString, resolvedEncoding));
    }

    /**
     * Returns the byte length of a string when encoded using `encoding`.
     * This is not the same as String.prototype.length, which does not account
     * for the encoding that is used to convert the string into bytes.
     *
     * @param {string | Buffer | TypedArray | DataView | ArrayBuffer} doc - A value to calculate the length of
     * @param {BufferEncoding} [encoding='utf8'] - If `string` is a string, this is its encoding
     * @returns {int} The number of bytes contained within `string`
     * @throws {BusinessError} 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     */
    export function byteLength(
        doc: string | Buffer | TypedArray | DataView | ArrayBuffer,
        encoding?: BufferEncoding
    ): int {
        if (doc instanceof string) {
            let resolvedEncoding: string = encoding ?? "utf8";
            return ArrayBuffer.bytesLength(doc, resolvedEncoding);
        }
        // NOTE (templin.konstantin): Can't completely use smart casts due internal issue #21021
        //  now is used temporarily solution with reflection
        if (doc instanceof Buffer) {
            return (doc as Buffer).length;
        } else if (doc instanceof Int8Array) {
            return (doc as Int8Array).byteLength.toInt();
        } else if (doc instanceof Uint8Array) {
            return (doc as Uint8Array).byteLength.toInt();
        } else if (doc instanceof Uint8ClampedArray) {
            return (doc as Uint8ClampedArray).byteLength.toInt();
        } else if (doc instanceof Int16Array) {
            return (doc as Int16Array).byteLength.toInt();
        } else if (doc instanceof Uint16Array) {
            return (doc as Uint16Array).byteLength.toInt();
        } else if (doc instanceof Int32Array) {
            return (doc as Int32Array).byteLength.toInt();
        } else if (doc instanceof Uint32Array) {
            return (doc as Uint32Array).byteLength.toInt();
        } else if (doc instanceof Float32Array) {
            return (doc as Float32Array).byteLength.toInt();
        } else if (doc instanceof Float64Array) {
            return (doc as Float64Array).byteLength.toInt();
        } else if (doc instanceof BigInt64Array) {
            return (doc as BigInt64Array).byteLength.toInt();
        } else if (doc instanceof BigUint64Array) {
            return (doc as BigUint64Array).byteLength.toInt();
        } else if (doc instanceof DataView) {
            return (doc as DataView).byteLength.toInt();
        } else if (doc instanceof ArrayBuffer) {
            return (doc as ArrayBuffer).getByteLength();
        }
        throw createBusinessError(TypeErrorCodeId, "Invalid type")
    }

    /**
     * Allocates a new Buffer for a fixed size bytes. If fill is undefined, the Buffer will be zero-filled.
     *
     * @param { int } size - The desired length of the new Buffer
     * @param { string | Buffer | int | double | long } [fill] - A value to pre-fill the new Buffer with
     * @param { BufferEncoding } [encoding] - If `fill` is a string, this is its encoding
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function alloc(
        size: int,
        fill?: string | Buffer | int | double | long,
        encoding?: BufferEncoding
    ): Buffer
    {
        if (size < 0) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "size" must be number and ` +
                `the value cannot be negative. Received value is: ${size}`);
        }
        const buffer = new Buffer(new ArrayBuffer(size));
        const resolvedEncoding = encoding ?? "utf-8";
        if (fill != undefined) {
            buffer.fill(fill!, 0, size, resolvedEncoding);
        } else {
            buffer.fill(0 as long, 0);
        }
        return buffer;
    }

    /**
     * Allocates a new Buffer for a fixed size bytes. The Buffer will not be initially filled.
     *
     * @param { int } size - The desired length of the new Buffer
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function allocUninitializedFromPool(size: int): Buffer {
        if (size < 0) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "size" must be number and ` +
                `the value cannot be negative. Received value is: ${size}`);
        }
        return new Buffer(new ArrayBuffer(size));
    }

    /**
     * Allocates a new un-pooled Buffer for a fixed size bytes. The Buffer will not be initially filled.
     *
     * @param { int } size - The desired length of the new Buffer
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function allocUninitialized(size: int): Buffer {
        if (size < 0) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "size" must be number and ` +
                `the value cannot be negative. Received value is: ${size}`);
        }
        return new Buffer(new ArrayBuffer(size));
    }

    /**
     * Compares buf1 to buf2
     *
     * @param { Buffer | Uint8Array } buf1 - A Buffer or Uint8Array instance.
     * @param { Buffer | Uint8Array } buf2 - A Buffer or Uint8Array instance.
     * @returns { int } 0 is returned if target is the same as buf
     *         1 is returned if target should come before buf when sorted.
     *        -1 is returned if target should come after buf when sorted.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function compare(buf1: Buffer | Uint8Array, buf2: Buffer | Uint8Array): int {
        const resolvedBuf1 = getArrayBufferFrom(buf1);
        const resolvedBuf2 = getArrayBufferFrom(buf2);
        const len1 = getLength(buf1);
        const len2 = getLength(buf2);
        const minLength = Math.min(len1, len2);
        let i1 = (buf1 instanceof Buffer) ? (buf1 as Buffer).byteOffset : (buf1 as Uint8Array).byteOffset.toInt()
        let i2 = (buf2 instanceof Buffer) ? (buf2 as Buffer).byteOffset : (buf2 as Uint8Array).byteOffset.toInt()
        for (let i = 0; i < minLength; i++) {
            if (resolvedBuf1.at(i1) != resolvedBuf2.at(i2)) {
                return resolvedBuf1.at(i1) < resolvedBuf2.at(i2) ? -1 : 1;
            }
            i1++;
            i2++;
        }
        return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
    }

    /**
     * Returns a new `Buffer` which is the result of concatenating all the `Buffer`instances in the `list` together.
     *
     * @param { Buffer[] | Uint8Array[] } list - List of `Buffer` or Uint8Array instances to concatenate
     * @param { int } [totalLength] - Total length of the `Buffer` instances in `list` when concatenated
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     * @throws { BusinessError } 10200001 - The value of "length" is out of range. It must be >= 0 and <= 4294967296. Received value is: [length]
     */
    export function concat(list: Buffer[] | Uint8Array[], totalLength?: int): Buffer {
        const resolvedList = new Array<ArrayBuffer>();
        for (const item of list) {
            resolvedList.push(getArrayBufferFrom(item));
        }
        if (totalLength == 0) totalLength = undefined;
        const length: int = totalLength ?? resolvedList.reduce(
            (acc: number, item: ArrayBuffer): number => acc + item.byteLength.toInt(), 0).toInt()
        if (length < 0) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "totalLength" is out of range. ` +
                `It must be >= 0 and <= 2147483647. Received value is: ${length}`)
        }
        const listOfBytes = new Array<byte>();
        for (const item of resolvedList) {
            for (let i = 0; i < item.byteLength; i++) {
                listOfBytes.push(item.at(i));
            }
        }
        const buffer = new ArrayBuffer(length);
        let offset = 0;
        for (let i = 0; i < length; i++) {
            if(i < listOfBytes.length) {
                buffer.set(offset, listOfBytes[i]);
            } else {
                buffer.set(offset, 0);
            }
            offset += 1;
        }
        return new Buffer(buffer);
    }

    /**
     * Returns true if obj is a Buffer, false otherwise
     *
     * @param { Object } obj - Objects to be judged
     * @returns { boolean } true or false
     */
    export function isBuffer(obj: Object): boolean {
        return obj instanceof Buffer;
    }

    /**
     * Returns true if encoding is the name of a supported character encoding, or false otherwise.
     *
     * @param { string } encoding - A character encoding name to check
     * @returns { boolean } true or false
     */
    export function isEncoding(encoding: string): boolean {
        switch (encoding) {
            case 'ascii':
            case 'utf8':
            case 'utf-8':
            case 'utf16le':
            case 'ucs2':
            case 'ucs-2':
            case 'base64':
            case 'base64url':
            case 'latin1':
            case 'binary':
            case 'hex':
                return true;
            default:
                return false;
        }
    }

    /**
     * Re-encodes the given Buffer or Uint8Array instance from one character encoding to another.
     *
     * @param { Buffer | Uint8Array } source - A Buffer or Uint8Array instance.
     * @param { string } fromEnc - The current encoding
     * @param { string } toEnc - To target encoding
     * @returns { Buffer } Returns a new Buffer instance
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function transcode(source: Buffer | Uint8Array, fromEnc: string, toEnc: string): Buffer {
        if (!isEncoding(fromEnc)) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "fromEnc" must be BufferEncoding. ` +
                `the fromEnc ${fromEnc} is unknown`);
        }
        if (!isEncoding(toEnc)) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of "toEnc" must be BufferEncoding. ` +
                `the toEnc ${toEnc} is unknown`);
        }
        let resolvedSource = getArrayBufferFrom(source);
        let sourceBytesLength = resolvedSource.byteLength.toInt();
        let str = '';
        if (source instanceof Uint8Array) {
            str = source.toString();
        } else if (source instanceof Buffer) {
            str = stringify(resolvedSource, fromEnc, 0, sourceBytesLength);
        }
        if (toEnc == 'ascii') {
            let newStr = "";
            for (let i = 0; i < str.length; ++i) {
                if (str.charAt(i).toInt() <= 128) {
                    newStr += str.charAt(i);
                } else {
                    newStr += '?';
                }
            }
            str = newStr;
        } else if (toEnc == 'base64') {
            str = str.replace("/[\r\n]/g", '');
        }
        const newBytes = ArrayBuffer.from(str, toEnc);
        return new Buffer(newBytes);
    }

    class BufferIteratorKeys implements IterableIterator<int> {
        private length: int
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length.toInt()
        }
        public override $_iterator(): IterableIterator<int> {
            return this
        }
        override next(): IteratorResult<int> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<int>()
            }
            return new IteratorResult<int>(false, (this.idx++))
        }
    }

    class BufferIteratorValues implements IterableIterator<long> {
        private length: int
        private parent: Buffer
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length.toInt()
            this.parent = parent
        }
        public override $_iterator(): IterableIterator<long> {
            return this
        }
        override next(): IteratorResult<long> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<long>()
            }
            return new IteratorResult<long>(false, this.parent.at(this.idx++))
        }
    }

    class BufferEntriesIterator implements IterableIterator<[int, long]> {
        private length: int
        private parent: Buffer
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length.toInt()
            this.parent = parent
        }
        public override $_iterator(): IterableIterator<[int, long]> {
            return this
        }
        override next(): IteratorResult<[int, long]> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<[int, long]>()
            }
            return new IteratorResult<[int, long]>(false, [this.idx, this.parent.at(this.idx++)])
        }
    }

    /**
     * A class representing a fixed-length sequence of bytes.
     * Provides methods for reading and manipulating binary data with various encodings.
     */
    export class Buffer implements jsonx.JsonElementSerializable {
        /** The underlying ArrayBuffer storing the binary data */
        private bufferData: ArrayBuffer;
        /** The offset into the buffer where this Buffer instance starts */
        private readonly byteOffsetNumber: int;
        /** The length of ArrayBuffer */
        private readonly byteLength: int;

        /**
         * Gets the length of the buffer in bytes
         * @returns {int} The number of bytes in the buffer
         */
        get length(): int { 
            return this.byteLength 
        }

        get byteOffset(): int { 
            return this.byteOffsetNumber 
        }

        get buffer(): ArrayBuffer { 
            return this.bufferData 
        }
        /**
         * Creates a new Buffer instance
         * @param {ArrayBuffer} buffer - The underlying ArrayBuffer to use
         * @param {int} [byteOffset=0] - The starting offset into the ArrayBuffer
         */
        public constructor(buffer: ArrayBuffer, byteOffset: int = 0, byteLength?: int) {
            this.bufferData = buffer;
            this.byteOffsetNumber = byteOffset;
            if (byteLength == undefined) {
                this.byteLength = buffer.getByteLength() - byteOffset;
            } else {
                this.byteLength = byteLength;
            }
            if (byteOffset >= this.buffer.getByteLength() && this.buffer.getByteLength() > 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "byteOffset" is out of range. Received value is: ${byteOffset}`)
            }
            if (byteOffset < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "byteOffset" is out of range. It must be >= 0. Received value is: ${byteOffset}`)
            }
        }

        /**
         * Returns a string decoded from the buffer's contents.
         *
         * @returns {string} The decoded string
         */
        public toString(): string {
            let resolvedEncoding: string = "utf8";
            let resolvedStart: int = this.byteOffset;
            let resolvedEnd: int = this.length + this.byteOffset;
            return ArrayBuffer.stringify(this.buffer, resolvedEncoding, resolvedStart, resolvedEnd);
        }

        /**
         * Returns a string decoded from the buffer's contents.
         *
         * @param {BufferEncoding} [encoding='utf8'] - Character encoding to use for decoding
         * @param {int} [start=0] - Where to start decoding
         * @param {int} [end=buffer.length] - Where to stop decoding
         * @returns {string} The decoded string
         */
        public toString(encoding?: BufferEncoding, start?: int, end?: int): string {
            let resolvedEncoding: string = encoding ?? "utf8";
            let resolvedStart: int = start ?? 0;
            let resolvedEnd: int = end ?? this.length;
            resolvedStart = resolvedStart < 0 ? 0 : resolvedStart;
            resolvedEnd = resolvedEnd > this.length ? this.length : resolvedEnd;
            if (resolvedStart > this.length || resolvedStart > resolvedEnd) {
                return '';
            }
            resolvedStart += this.byteOffset;
            resolvedEnd += this.byteOffset;
            return stringify(this.buffer, resolvedEncoding, resolvedStart, resolvedEnd);
        }

        /**
         * Returns the byte at the specified index
         *
         * @param {int} index - Index of the byte to return
         * @returns {long} The byte at the specified position
         */
        public at(index: int): long {
            return this.buffer.at(index + this.byteOffset);
        }

        /**
         * Checks if the buffer includes the given value.
         *
         * @param {string | int | double | long | Buffer | Uint8Array} value - The value to search for
         * @param {int} [byteOffset=0] - The byte position to start searching from
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {boolean} `true` if the value is found, otherwise `false`
         */
        public includes(value: string | int | double | long | Buffer | Uint8Array, byteOffset?: int, encoding?: BufferEncoding): boolean {
            return this.indexOf(value, byteOffset ?? 0, encoding ?? "utf8") != -1;
        }

        /**
         * Returns the first index where `value` is found in the buffer.
         *
         * @param {string | int | double | long | Buffer | Uint8Array} value - The value to search for
         * @param {int} [byteOffset=0] - The byte position to start searching from.
         *   - If negative, it is counted from the end of the buffer (`len + byteOffset`).
         *   - If out of bounds (`>= buffer length`), returns `-1`.
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {int} The index of the first occurrence of `value`, or -1 if not found
         */
        public indexOf(value: string | int | double | long | Buffer | Uint8Array, byteOffset?: int, encoding?: BufferEncoding): int {
            const searchBuffer = this.normalizeValueToBuffer(value, encoding ?? "utf8");
            const len: int = this.length;
            const searchLen = searchBuffer.length;
            let startIndex: int = byteOffset ?? 0;

            if (startIndex < 0) {
                startIndex = Math.max(0, len + startIndex).toInt()
            } else if (startIndex >= len) {
                return -1;
            }
            for (let i = startIndex; i <= len - searchLen; i++) {
                if (this.compareSubarray(i, searchBuffer)) return i;
            }
            return -1;
        }

        /**
         * Returns the last index where `value` is found in the buffer.
         *
         * @param {string | int | double | long | Buffer | Uint8Array} value - The value to search for
         * @param {int} [byteOffset=buffer length] - The byte position to start searching from (backwards).
         *   - If negative, it is counted from the end of the buffer (`len + byteOffset`).
         *   - If out of bounds (`>= buffer length`), it is clamped to `buffer length - searchLen`.
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {int} The index of the last occurrence of `value`, or -1 if not found
         */
        public lastIndexOf(value: string | int | double | long | Buffer | Uint8Array, byteOffset?: int , encoding?: BufferEncoding): int {
            const searchBuffer = this.normalizeValueToBuffer(value, encoding ?? "utf8");
            const searchLen = searchBuffer.length;
            const len = this.length;

            if (searchLen == 0 || len < searchLen) {
                return -1;
            }
            let endIndex: int = byteOffset ?? len;
            if (endIndex < 0) {
                endIndex = Math.max(0, len + endIndex).toInt()
            } else if (endIndex >= len) {
                endIndex = len - searchLen;
            }
            for (let i = endIndex; i >= 0; i--) {
                if (this.compareSubarray(i, searchBuffer)) {
                    return i;
                }
            }
            return -1;
        }

        private normalizeValueToBuffer(value: string | int | double | long | Buffer | Uint8Array, encoding: BufferEncoding): Buffer {
            if (value instanceof string) {
                return buffer.from(value, encoding ?? "utf8");
            } else if (value instanceof Double) {
                const arr: double[] = [value];
                return buffer.from(arr);
            } else if (value instanceof int) {
                const arr: double[] = [value.toDouble()];
                return buffer.from(arr);
            } else if (value instanceof Long) {
                const arr: double[] = [value.toDouble()];
                return buffer.from(arr);
            } else if (value instanceof Buffer) {
                return value;
            } else {
                return buffer.from(value);
            }
        }

        private compareSubarray(startIndex: int, searchBuffer: Buffer): boolean {
            if (startIndex + searchBuffer.length > this.length){
                return false;
            }
            for (let j: int = 0; j < searchBuffer.length; j++) {
                if (this.at(startIndex.toInt() + j) != searchBuffer.at(j)) {
                    return false;
                }
            }
            return true;
        }

        /**
         Compares buf with target and returns a number indicating whether buf comes before, after,
         * or is the same as target in sort order. Comparison is based on the actual sequence of bytes in each Buffer.
         *
         * @param { Buffer | Uint8Array } target - A Buffer or Uint8Array with which to compare buf
         * @param { int } [targetStart] - The offset within target at which to begin comparison
         * @param { int } [targetEnd] - The offset within target at which to end comparison (not inclusive)
         * @param { int } [sourceStart] - The offset within buf at which to begin comparison
         * @param { int } [sourceEnd] - The offset within buf at which to end comparison (not inclusive)
         * @returns { int } 0 is returned if target is the same as buf
         *         1 is returned if target should come before buf when sorted.
         *        -1 is returned if target should come after buf when sorted.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         * @throws { BusinessError } 10200001 - The value of "[targetStart/targetEnd/sourceStart/sourceEnd]" is out of range.
         *         It must be >= 0 and <= [right range]. Received value is: [targetStart/targetEnd/sourceStart/sourceEnd]
         */
        public compare(
            target: Buffer | Uint8Array,
            targetStart?: int,
            targetEnd?: int,
            sourceStart?: int,
            sourceEnd?: int): int
        {
            const resolvedSource: ArrayBuffer = this.buffer;
            const resolvedTarget: ArrayBuffer = getArrayBufferFrom(target);

            const resolvedTargetStart = targetStart ?? 0;
            const resolvedTargetEnd = targetEnd ?? target.length;
            const resolvedSourceStart = sourceStart ?? 0;
            const resolvedSourceEnd = sourceEnd ?? this.length;
            if (resolvedTargetStart < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "targetStart" is out of range. ` +
                    `It must be >= 0 and <= 2147483647. Received value is: ${targetStart}`);
            }
            if (resolvedTargetEnd < 0 || resolvedTargetEnd > target.length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "targetEnd" is out of range. ` +
                    `It must be >= 0 and <= ${target.length}. Received value is: ${targetEnd}`);
            }
            if (resolvedSourceStart < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "sourceEnd" is out of range. ` +
                    `It must be >= 0 and <= 2147483647. Received value is: ${sourceStart}`);
            }
            if (resolvedSourceEnd < 0 || resolvedSourceEnd > this.length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "sourceEnd" is out of range. ` +
                    `It must be >= 0 and <= ${this.length}. Received value is: ${sourceEnd}`);
            }
            const targetSlice = resolvedTarget.slice(resolvedTargetStart + target.byteOffset,
                                                     resolvedTargetEnd + target.byteOffset);
            const sourceSlice = resolvedSource.slice(resolvedSourceStart + this.byteOffset,
                                                     resolvedSourceEnd + this.byteOffset);
            return compare(new Buffer(sourceSlice), new Buffer(targetSlice));
        }

        /**
         * Copies data from a region of buf to a region in target, even if the target memory region overlaps with buf.
         * If sourceEnd is greater than the length of the target, the length of the target shall prevail, and the extra part will not be overwritten.
         *
         * @param { Buffer | Uint8Array } target - A Buffer or Uint8Array to copy into
         * @param { int } [targetStart] - The offset within target at which to begin writing
         * @param { int } [sourceStart] - The offset within buf from which to begin copying
         * @param { int } [sourceEnd] - The offset within buf at which to stop copying (not inclusive)
         * @returns { int } The number of bytes copied
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         * @throws { BusinessError } 10200001 - The value of "[targetStart/sourceStart/sourceEnd]" is out of range. It must be >= 0.
         *                                    Received value is: [targetStart/sourceStart/sourceEnd]
         */
        public copy(
            target: Buffer | Uint8Array,
            targetStart?: int,
            sourceStart?: int,
            sourceEnd?: int): int
        {
            const resolvedTarget = target.buffer;
            const resolvedSource: ArrayBuffer = this.buffer;
            const resolvedTargetStart = (targetStart ?? 0).toInt();
            const resolvedSourceStart = (sourceStart ?? 0).toInt();
            const resolvedSourceEnd = (sourceEnd ?? this.length).toInt();

            if (resolvedTargetStart < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "targetStart" is out of range. ` +
                    `It must be >= 0. Received value is: ${targetStart}`);
            }
            if (resolvedSourceStart < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "sourceStart" is out of range. ` +
                    `It must be >= 0. Received value is: ${sourceStart}`);
            }
            if (resolvedSourceEnd < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "sourceEnd" is out of range. ` +
                    `It must be >= 0. Received value is: ${sourceEnd}`);
            }
            if (resolvedTargetStart >= target.length) {
                return 0;
            }
            if (resolvedSourceStart >= resolvedSourceEnd || resolvedSourceStart >= this.length) {
                return 0;
            }
            const sourceSlice: ArrayBuffer = resolvedSource.slice(resolvedSourceStart + this.byteOffset,
                resolvedSourceEnd + this.byteOffset);
            let copyLen = 0;
            for (; copyLen < sourceSlice.byteLength
                && resolvedTargetStart + target.byteOffset + copyLen < resolvedTarget.byteLength;
                copyLen++) {
                resolvedTarget.set(resolvedTargetStart + target.byteOffset + copyLen, sourceSlice.at(copyLen));
            }
            return copyLen;
        }

        /**
         * Creates and returns an iterator of [index, byte] pairs from the contents of buf.
         *
         * @returns { IterableIterator<[int, long]> }
         */
        public entries(): IterableIterator<[int, long]> {
            return new BufferEntriesIterator(this);
        }

        /**
         * Returns true if both buf and otherBuffer have exactly the same bytes, false otherwise
         *
         * @param { Uint8Array | Buffer } otherBuffer - A Buffer or Uint8Array with which to compare buf
         * @returns { boolean } true or false
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         */
        public equals(otherBuffer: Uint8Array | Buffer): boolean {
            return this.compare(otherBuffer) == 0;
        }

        /**
         * Fills buf with the specified value. If the offset and end are not given, the entire buf will be filled.
         *
         * @param { string | Buffer | Uint8Array | int | double | long } value - The value with which to fill buf
         * @param { int } [offset] - Number of bytes to skip before starting to fill buf
         * @param { int } [end] - Where to stop filling buf (not inclusive)
         * @param { BufferEncoding } [encoding] - The encoding for value if value is a string
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200001 - The value of "[offset/end]" is out of range. It must be >= 0 and <= [right range]. Received value is: [offset/end]
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         */
        public fill(
            value: string | Buffer | Uint8Array | int | double | long,
            offset: int = 0,
            end?: int,
            encoding?: BufferEncoding
        ): Buffer {
            const resolvedEnd = end ?? this.length;

            if (offset < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= 2147483647. Received value is: ${offset}`)
            }
            if (resolvedEnd < 0 || resolvedEnd > this.length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "end" is out of range. ` +
                    `It must be >= 0 and <= ${this.length}. Received value is: ${resolvedEnd}`)
            }

            if (value instanceof string && value as string == "") {
                return this.fill(0 as long, offset, end, encoding);
            }

            const offsetInt = offset + this.byteOffset;
            const resolvedEndInt = resolvedEnd + this.byteOffset;
            const resolvedEncoding: BufferEncoding = encoding ?? "utf-8";

            if (value instanceof string) {
                let strValue = value as string;
                if (resolvedEncoding == 'ascii') {
                    strValue = sanitizeAscii(strValue);
                } else if (resolvedEncoding == 'utf8' || resolvedEncoding == 'utf-8') {
                    if (strValue.length == 0) {
                        return this;
                    }
                    strValue = sanitizeUtf8(strValue);
                } else if (resolvedEncoding == 'latin1' || resolvedEncoding == 'binary') {
                    strValue = sanitizeLatin1(strValue);
                } else if (resolvedEncoding == 'hex') {
                    strValue = takeValidHexPrefix(strValue);
                    if (strValue.length == 0) {
                        return this;
                    }
                } else if (resolvedEncoding == 'base64' || resolvedEncoding == 'base64url') {
                    strValue = fillInPaddingBase64(strValue);
                }

                let buff: Uint8Array;
                if (resolvedEncoding === 'ucs2' || resolvedEncoding === 'ucs-2' || resolvedEncoding === 'utf16le') {
                    const u16 = new Uint16Array(strValue.length);
                    for (let i = 0; i < strValue.length; i++) {
                        u16[i] = strValue.charCodeAt(i);
                    }
                    buff = new Uint8Array(u16.buffer);
                } else {
                    buff = new Uint8Array(ArrayBuffer.from(strValue, resolvedEncoding as string));
                }

                let pos = offsetInt;
                while (buff.byteLength > 0 && pos + buff.byteLength <= resolvedEndInt) {
                    for (let j: int = 0; j < buff.byteLength; j++) {
                        this.buffer.set(pos++, buff[j].toByte());
                    }
                }

                for (let j: int = 0; pos < resolvedEndInt && buff.byteLength > 0; j++, pos++) {
                    this.buffer.set(pos, buff[j].toByte());
                }
                return this;
            } else if (value instanceof int) {
                let asInt: int = value as int;
                for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                    this.buffer.set(i, asInt.toByte());
                }
                return this;
            } else if (value instanceof long) {
                let asLong: long = value as long;
                for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                    this.buffer.set(i, asLong.toByte());
                }
                return this;
            } else if (value instanceof double) {
                let asDouble: double = value as double;
                for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                    this.buffer.set(i, asDouble.toByte());
                }
                return this;
            } else if (value instanceof Buffer) {
                let asBuf = value as Buffer;
                for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                    this.buffer.set(i, asBuf.at((i % asBuf.length)).toByte());
                }
                return this;
            } else if (value instanceof Uint8Array) {
                let asU8a = value as Uint8Array;
                for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                    this.buffer.set(i, asU8a.at(i % asU8a.length)!.toByte());
                }
                return this;
            } else {
                    throw createBusinessError(TypeErrorCodeId, `Parameter error: value must be a string, Buffer, Uint8Array, or int. \
Received value is: ${value}`)
            }
        }

        /**
        * Throws an error when a parameter value is outside allowed range bounds
        *
        * @param {string} param_name - Name of the parameter that caused the error
        * @param {double} param_value - The value that is out of range
        * @param {int} left_bound - Minimum allowed value (inclusive)
        * @param {int} right_bound - Maximum allowed value (inclusive)
        * @returns {void}
        * @throws {Error} With a message indicating the parameter name, its value, and valid range
        * @private
        */
        private static throwOutOfRangeError(param_name: string, param_value: double, left_bound: int, right_bound: int) {
            let message = `The value of "${param_name}" is out of range.`;
            message += `It must be >= ${left_bound} and <= ${right_bound}. Received value is: ${param_value}`;
            // NOTE (templin.konstantin): must be changed to BusinessError later
            throw createBusinessError(OutOfBoundsErrorCodeId, message);
        }

        /**
        * Checks if the byteLength parameter is within valid range
        *
        * @param {int} byteLength - Number of bytes to read or write
        * @returns {void}
        * @throws {Error} If byteLength is less than 1 or greater than 6
        * @private
        */
        private static checkByteLengthConstraint(byteLength: int) {
            if (byteLength < 1 || byteLength > 6) {
                Buffer.throwOutOfRangeError("byteLength", byteLength, 1, 6);
            }
        }

        /**
        * Checks if the offset parameter is within valid range
        *
        * @param {int} offset - Number of bytes to skip before reading
        * @param {int} lengthOffset - Maximum allowed value (inclusive)
        * @returns {void}
        * @throws {Error} If byteLength is less than 0 or greater than lengthOffset
        * @private
        */
        private static checkOffsetConstraint(offset: int, lengthOffset: int) {
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
        }
        /**
         * Creates and returns an iterator of buf keys (indices).
         *
         * @returns { IterableIterator<int> }
         */
        public keys(): IterableIterator<int> {
            return new BufferIteratorKeys(this);
        }

        /**
         * Creates and returns an iterator for buf values (bytes).
         *
         * @returns { IterableIterator<long> }
         */
        public values(): IterableIterator<long> {
            return new BufferIteratorValues(this);
        }

        /**
         * Returns a new Buffer that references the same memory as the original, but offset and cropped by the start and end indices.
         *
         * @param { int } [start] - Where the new Buffer will start
         * @param { int } [end] - Where the new Buffer will end (not inclusive)
         * @returns { Buffer } Returns a new Buffer that references the same memory as the original
         */
        public subarray(start?: int, end?: int): Buffer {
            if (start == undefined || isNaN(start!)) {
                start = 0
            }
            if (end == undefined || isNaN(end!) || end > this.length) {
                end = this.length
            }
            let s = start + this.byteOffset
            let e = end + this.byteOffset
            if (s < 0 || e < 0 || e <= s) {
                return new Buffer(new ArrayBuffer(0))
            }
            return new Buffer(this.buffer.slice(s, e));
        }

        /**
         * Interprets buf as an array of unsigned 16-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 16-bits
         */
        public swap16(): Buffer {
            if (this.length % 2 != 0) {
                throw createBusinessError(IncorrectBufferSizeId, 'The buffer size must be a multiple of 16-bits')
            }
            for (let i = this.byteOffset; i < this.length + this.byteOffset; i += 2) {
                const a = this.buffer.at(i);
                this.buffer.set(i, this.buffer.at(i + 1));
                this.buffer.set(i + 1, a);
            }
            return this;
        }

        /**
         * Interprets buf as an array of unsigned 32-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 32-bits
         */
        public swap32(): Buffer {
            if (this.length % 4 != 0) {
                throw createBusinessError(IncorrectBufferSizeId, 'The buffer size must be a multiple of 32-bits')
            }
            for (let i = this.byteOffset; i < this.length + this.byteOffset; i += 4) {
                const a = this.buffer.at(i);
                const b = this.buffer.at(i + 1);
                this.buffer.set(i, this.buffer.at(i + 3));
                this.buffer.set(i + 1, this.buffer.at(i + 2));
                this.buffer.set(i + 2, b);
                this.buffer.set(i + 3, a);
            }
            return this;
        }

        /**
         * Interprets buf as an array of unsigned 64-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 64-bits
         */
        public swap64(): Buffer {
            if (this.length % 8 != 0) {
                throw createBusinessError(IncorrectBufferSizeId, 'The buffer size must be a multiple of 64-bits')
            }
            for (let i = this.byteOffset; i < this.length + this.byteOffset; i += 8) {
                const a = this.buffer.at(i);
                const b = this.buffer.at(i + 1);
                const c = this.buffer.at(i + 2);
                const d = this.buffer.at(i + 3);
                this.buffer.set(i, this.buffer.at(i + 7));
                this.buffer.set(i + 1, this.buffer.at(i + 6));
                this.buffer.set(i + 2, this.buffer.at(i + 5));
                this.buffer.set(i + 3, this.buffer.at(i + 4));
                this.buffer.set(i + 4, d);
                this.buffer.set(i + 5, c);
                this.buffer.set(i + 6, b);
                this.buffer.set(i + 7, a);
            }
            return this;
        }

                /**
        * Creates a DataView of the buffer
        *
        * @returns {DataView} A DataView of the buffer
        * @private
        */
        private getDataView(): DataView {
            return new DataView(this.buffer, this.byteOffset, this.length);
        }

        /**
        * Writes a signed integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} offset - Number of bytes to skip before writing
        * @param {int} byteLength - Number of bytes to write (maximum 6)
        * @returns {int} Offset plus the number of bytes written
        * @throws {Error} If byteLength is greater than 6
        */
        public writeIntBE(value: long, offset: int, byteLength: int): int {
            Buffer.checkByteLengthConstraint(byteLength);
            this.checkWriteInt(value, offset, byteLength);
            const view = this.getDataView();
            let remaining = value;
            for (let i = byteLength - 1; i >= 0; i--) {
                view.setUint8((offset + i).toInt(), remaining & 0xff);
                remaining >>= 8;
            }
            return offset + byteLength;
        }

        private getMultiplier(i: int): long {
            return Math.pow(2, i * 8).toLong()
        }

        private toUByte(b: byte): int {
            if (b < 0) {
                return b + 256;
            }
            return b;
        }

        private readBytes(view: DataView, offset: int, byteLength: int, LE: boolean = false): long {
            let val: long = 0;
            for (let i = 0; i < byteLength; i++) {
                let idx = LE ? i : byteLength - i - 1
                const byt: int = this.toUByte(this.$_get(offset + idx)!.toByte())
                val += byt * this.getMultiplier(i);
            }
            return val
        }

        private toSigned(val: long, byteLength: int)
        {
            const unsignedMax = this.getMultiplier(byteLength)
            const half = unsignedMax / 2
            const max = half - 1

            if (val > max) {
                return val - unsignedMax
            }
            return val
        }

        /**
        * Reads a signed integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} offset - Number of bytes to skip before reading
        * @param {int} byteLength - Number of bytes to read (maximum 6)
        * @returns {int} The read value
        * @throws {Error} If byteLength is greater than 6
        */
        public readIntBE(offset: int, byteLength: int): long {
            let lengthOffset: int = this.length - byteLength;
            Buffer.checkOffsetConstraint(offset, lengthOffset);
            Buffer.checkByteLengthConstraint(byteLength);
            let val = this.readBytes(this.getDataView(), offset, byteLength)
            return this.toSigned(val, byteLength)
        }

        /**
        * Writes a signed integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} offset - Number of bytes to skip before writing
        * @param {int} byteLength - Number of bytes to write (maximum 6)
        * @returns {int} Offset plus the number of bytes written
        * @throws {Error} If byteLength is greater than 6
        */
        public writeIntLE(value: long, offset: int, byteLength: int): int {
            Buffer.checkByteLengthConstraint(byteLength);
            this.checkWriteInt(value, offset, byteLength);
            const view = this.getDataView();
            let remaining = value;
            for (let i = 0; i < byteLength; i++) {
                view.setUint8((offset + i).toInt(), remaining & 0xff);
                remaining >>= 8;
            }
            return offset + byteLength;
        }

        /**
        * Reads a signed integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} offset - Number of bytes to skip before reading
        * @param {int} byteLength - Number of bytes to read (maximum 6)
        * @returns {long} The read value
        * @throws {Error} If byteLength is greater than 6
        */
        public readIntLE(offset: int, byteLength: int): long {
            let lengthOffset: int = this.length - byteLength;
            Buffer.checkOffsetConstraint(offset, lengthOffset);
            Buffer.checkByteLengthConstraint(byteLength);
            let val = this.readBytes(this.getDataView(), offset, byteLength, true)
            return this.toSigned(val, byteLength)
        }

        /**
        * Writes an unsigned integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} offset - Number of bytes to skip before writing
        * @param {int} byteLength - Number of bytes to write (maximum 6)
        * @returns {int} Offset plus the number of bytes written
        * @throws {Error} If byteLength is greater than 6
        */
        public writeUIntBE(value: long, offset: int, byteLength: int): int {
            Buffer.checkByteLengthConstraint(byteLength);
            this.checkWriteUInt(value, offset, byteLength);
            const view = this.getDataView();
            let remaining = value;
            for (let i = byteLength - 1; i >= 0; i--) {
                view.setUint8((offset + i).toInt(), remaining & 0xFF);
                remaining >>= 8;
            }
            return offset + byteLength;
        }

        private checkWriteInt(value: long, offset: int, byteLength: int): void {
            this.checkOffset(offset, byteLength);
            const start = (-Math.pow(2, byteLength * 8 - 1)).toLong();
            const end = -start - 1;
            this.checkValue(value, start, end, start.toString(), end.toString());
        }

         private checkWriteUInt(value: long, offset: int, byteLength: int): void {
            this.checkOffset(offset, byteLength);
            const end = (Math.pow(2, byteLength * 8).toLong()) - 1;
            this.checkValue(value, 0, end, "0", end.toString());
        }

        private checkOffset(offset: int, size: int): void {
            let lengthOffset: int = this.length - size;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
        }

        private checkValue(value: long, start: long , end: long, startText: string, endText: string): void {
            if (value < start || value > end) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "value" is out of range. ` +
                    `It must be >= ${startText} and <= ${endText}. Received value is: ${value}`)
            }
        }

        private checkValue(value: bigint, start: long , end: bigint, startText: string, endText: string): void {
            if (value < start || value > end) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "value" is out of range. ` +
                    `It must be >= ${startText} and <= ${endText}. Received value is: ${value}`)
            }
        }

        private checkValue(value: bigint, start: long , end: long, startText: string, endText: string): void {
            if (value < start || value > end) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "value" is out of range. ` +
                    `It must be >= ${startText} and <= ${endText}. Received value is: ${value}`)
            }
        }


        /**
        * Reads an unsigned integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} offset - Number of bytes to skip before reading
        * @param {int} byteLength - Number of bytes to read (maximum 6)
        * @returns {long} The read value
        * @throws {Error} If byteLength is greater than 6
        */
        public readUIntBE(offset: int, byteLength: int): long {
            let lengthOffset: int = this.length - byteLength;
            Buffer.checkOffsetConstraint(offset, lengthOffset);
            Buffer.checkByteLengthConstraint(byteLength);
            return this.readBytes(this.getDataView(), offset, byteLength)
        }

        /**
        * Writes an unsigned integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} offset - Number of bytes to skip before writing
        * @param {int} byteLength - Number of bytes to write (maximum 6)
        * @returns {int} Offset plus the number of bytes written
        * @throws {Error} If byteLength is greater than 6
        */
        public writeUIntLE(value: long, offset: int, byteLength: int): int {
            Buffer.checkByteLengthConstraint(byteLength);
            this.checkWriteUInt(value, offset, byteLength);
            const view = this.getDataView();
            let remaining = value;
            for (let i = 0; i < byteLength; i++) {
                view.setUint8((offset + i).toInt(), remaining & 0xFF);
                remaining >>= 8;
            }
            return offset + byteLength;
        }

        /**
        * Reads an unsigned integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} offset - Number of bytes to skip before reading
        * @param {int} byteLength - Number of bytes to read (maximum 6)
        * @returns {long} The read value
        * @throws {Error} If byteLength is greater than 6
        */
        public readUIntLE(offset: int, byteLength: int): long {
            let lengthOffset: int = this.length - byteLength;
            Buffer.checkOffsetConstraint(offset, lengthOffset);
            Buffer.checkByteLengthConstraint(byteLength);
            return this.readBytes(this.getDataView(), offset, byteLength, true)
        }

        /**
        * Writes a string to the buffer at the specified offset
        *
        * @param {string} str - String to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @param {int} [length] - Maximum number of bytes to write
        * @param {string} [encoding='utf8'] - Character encoding of the string
        * @returns {int} Number of bytes written
        * @throws { BusinessError } 10200001 - The value of "[offset/length]" is out of range. It mast be >= 0 and
            <= buf.length. Received value is: [offset/length]
        */
        public write(str: string, offset: int = 0, length?: int, encoding: string = 'utf8'): int {
            if (this.length === 0 ) {
                throw createBusinessError(OutOfBoundsErrorCodeId, 'The buffer length is 0, and writing data is not allowed');
            }
            if (offset != 0) {
                if (offset < 0 || offset > this.length - 1) {
                    throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                        `It must be >= 0 and <= ${this.length - 1}. Received value is: ${offset}`);
                }
            }
            if (!isEncoding(encoding)) {
                throw createBusinessError(TypeErrorCodeId, 'Parameter error. The type of "encoding" must be BufferEncoding. the encoding invalid_encoding is unknown')
            }
            const resolvedEncoding = encoding as buffer.BufferEncoding;
            const tmpLength = length ?? this.length - offset
            if (tmpLength < 0 || tmpLength > this.length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "length" is out of range. `+
                    `It must be >= 0 and <= ${this.length}. Received value is: ${tmpLength}`)
            }
            const resolvedLength = Math.min(tmpLength, this.length - offset)
            let resolvedString = str
            if (encoding == 'base64' || encoding == 'base64url') {
                resolvedString = fillInPaddingBase64(str)
            }
            const byteLen = buffer.byteLength(resolvedString, resolvedEncoding);
            const strBuffer = buffer.from(resolvedString, resolvedEncoding);
            for (let i = 0; i < resolvedLength && i < byteLen; i++) {
                this.buffer.set((offset + i).toInt(), strBuffer.at(i).toByte());
            }
            return Math.min(resolvedLength, byteLen).toInt();
        }

        /**
        * Reads a signed 64-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {bigint} The read value
        * @throws { BusinessError } 10200001 - The value of "[offset]" is out of range. It mast be >= 0 and
            <= buf.length - 8. Received value is: [offset]
        */
        public readBigInt64BE(offset: int = 0): bigint {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getBigInt64(offset.toInt(), false);
        }

        /**
        * Reads a signed 64-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {bigint} The read value
        */
        public readBigInt64LE(offset: int = 0): bigint {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getBigInt64(offset.toInt(), true);
        }

        /**
        * Reads an unsigned 64-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {number} [offset=0] - Number of bytes to skip before reading
        * @returns {bigint} The read value
        */
        public readBigUInt64BE(offset: int = 0): bigint {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getBigUint64(offset.toInt(), false);
        }

        /**
        * Reads an unsigned 64-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {bigint} The read value
        */
        public readBigUInt64LE(offset: int = 0): bigint {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getBigUint64(offset.toInt(), true);
        }

        /**
        * Reads a 64-bit double from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {double} The read value
        */
        public readDoubleBE(offset: int = 0): double {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getFloat64(offset.toInt(), false);
        }

        /**
        * Reads a 64-bit double from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {double} The read value
        */
        public readDoubleLE(offset: int = 0): double {
            let lengthOffset: int = this.length - 8;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getFloat64(offset.toInt(), true);
        }

        /**
        * Reads a 32-bit float from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {double} The read value
        */
        public readFloatBE(offset: int = 0): double {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getFloat32(offset.toInt(), false);
        }

        /**
        * Reads a 32-bit float from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {double} The read value
        */
        public readFloatLE(offset: int = 0): double {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getFloat32(offset.toInt(), true);
        }

        /**
        * Reads a signed 8-bit integer from the buffer at the specified offset
        *
        * @param {long} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readInt8(offset: int = 0): long {
            let lengthOffset: int = this.length - 1;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getInt8(offset.toInt()).toLong()
        }

        /**
        * Reads a signed 16-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readInt16BE(offset: int = 0): long {
            let lengthOffset: int = this.length - 2;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getInt16(offset.toInt(), false).toLong()
        }

        /**
        * Reads a signed 16-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readInt16LE(offset: int = 0): long {
            let lengthOffset: int = this.length - 2;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getInt16(offset.toInt(), true).toLong()
        }

        /**
        * Reads a signed 32-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readInt32BE(offset: int = 0): long {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.read32Big(offset);
        }

        private read32Big(byteOffset: int): int {
            let len = this.length;
            if (byteOffset < 0 || byteOffset + 4 > len) {
                throw new RangeError("wrong index");
            }
            let res: int = 0;
            const startByte = byteOffset + this.byteOffset;
            for (let i = 0; i < 4; i++) {
                let byteVal: int = (this.bufferData.at(startByte + 3 - i));
                byteVal &= 0xff;
                res = (res | byteVal << (8 * i));
            }
            return res;
        }

        /**
        * Reads a signed 32-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readInt32LE(offset: int = 0): long {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.read32Little(offset);
        }

        private read32Little(byteOffset: int): int {
            let len = this.length;
            if (byteOffset < 0 || byteOffset + 4 > len) {
                throw new RangeError("wrong index");
            }
            let res: int = 0;
            const startByte = byteOffset + this.byteOffset;
            for (let i = 0; i < 4; i++) {
                let byteVal: int = (this.buffer.at(startByte + i));
                byteVal &= 0xff;
                res = (res | byteVal << (8 * i));
            }
            return res;
        }

        /**
        * Reads an unsigned 8-bit integer from the buffer at the specified offset
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readUInt8(offset: int = 0): long {
            let lengthOffset: int = this.length - 1;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getUint8(offset.toInt()).toLong()
        }

        /**
        * Reads an unsigned 16-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readUInt16BE(offset: int = 0): long {
            let lengthOffset: int = this.length - 2;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getUint16(offset.toInt(), false).toLong()
        }

        /**
        * Reads an unsigned 16-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readUInt16LE(offset: int = 0): long {
            let lengthOffset: int = this.length - 2;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.getDataView().getUint16(offset.toInt(), true).toLong()
        }

        /**
        * Reads an unsigned 32-bit integer from the buffer at the specified offset using big-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readUInt32BE(offset: int = 0): long {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.readUInt32Big(offset).toLong();
        }

        private readUInt32Big(byteOffset: int): long {
            let len = this.length;
            if (byteOffset < 0 || byteOffset + 4 > len) {
                throw new RangeError("wrong index");
            }
            let res: long = 0;
            const startByte = byteOffset + this.byteOffset;
            for (let i = 0; i < 4; i++) {
                let byteVal: long = (this.buffer.at(startByte + 3 - i));
                byteVal &= 0xff;
                res = (res | byteVal << (8 * i));
            }
            return res;
        }

        /**
        * Reads an unsigned 32-bit integer from the buffer at the specified offset using little-endian format
        *
        * @param {int} [offset=0] - Number of bytes to skip before reading
        * @returns {long} The read value
        */
        public readUInt32LE(offset: int = 0): long {
            let lengthOffset: int = this.length - 4;
            if (offset < 0 || offset > lengthOffset) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "offset" is out of range. ` +
                    `It must be >= 0 and <= ${lengthOffset}. Received value is: ${offset}`);
            }
            return this.readUInt32Little(offset);
        }

        private readUInt32Little(offset: int): long {
            let len = this.length;
            if (offset < 0 || offset + 4 > len) {
                throw new RangeError("wrong index");
            }
            let res: long = 0;
            const startByte = offset + this.byteOffset;
            for (let i = 0; i < 4; i++) {
                let byteVal: long = (this.bufferData.at(startByte + i));
                byteVal &= 0xff;
                res = (res | byteVal << (8 * i));
            }
            return res;
        }

        /**
        * Writes a signed 64-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {bigint} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeBigInt64BE(value: bigint, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.checkValue(value, Long.MIN_VALUE, Long.MAX_VALUE, "-(2n ** 63n)", "2n ** 63n");
            this.getDataView().setBigInt64(offset.toInt(), value, false);
            return offset + 8;
        }

        /**
        * Writes a signed 64-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {bigint} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeBigInt64LE(value: bigint, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.checkValue(value, Long.MIN_VALUE, Long.MAX_VALUE, "-(2n ** 63n)", "2n ** 63n");
            this.getDataView().setBigInt64(offset.toInt(), value, true);
            return offset + 8;
        }

        /**
        * Writes an unsigned 64-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {bigint} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeBigUInt64BE(value: bigint, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.checkValue(value, 0, ((1n << 64n) - 1n), "0", "2n ** 64n - 1n");
            this.getDataView().setBigUint64(offset.toInt(), value, false);
            return offset + 8;
        }

        /**
        * Writes an unsigned 64-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {bigint} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeBigUInt64LE(value: bigint, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.checkValue(value, 0, ((1n << 64n) - 1n), "0", "2n ** 64n - 1n");
            this.getDataView().setBigUint64(offset.toInt(), value, true);
            return offset + 8;
        }

        /**
        * Writes a 64-bit double to the buffer at the specified offset using big-endian format
        *
        * @param {double} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeDoubleBE(value: double, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.getDataView().setFloat64(offset.toInt(), value, false);
            return offset + 8;
        }

        /**
        * Writes a 64-bit double to the buffer at the specified offset using little-endian format
        *
        * @param {double} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeDoubleLE(value: double, offset: int = 0): int {
            this.checkOffset(offset, 8);
            this.getDataView().setFloat64(offset.toInt(), value, true);
            return offset + 8;
        }

        /**
        * Writes a 32-bit float to the buffer at the specified offset using big-endian format
        *
        * @param {double} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeFloatBE(value: double, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.getDataView().setFloat32(offset.toInt(), value, false);
            return offset + 4;
        }

        /**
        * Writes a 32-bit float to the buffer at the specified offset using little-endian format
        *
        * @param {double} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeFloatLE(value: double, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.getDataView().setFloat32(offset.toInt(), value, true);
            return offset + 4;
        }

        /**
        * Writes a signed 8-bit integer to the buffer at the specified offset
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeInt8(value: long, offset: int = 0): int {
            this.checkOffset(offset, 1);
            this.checkValue(value, (-Math.pow(2, 7).toLong()), (Math.pow(2, 7).toLong() - 1), (-Math.pow(2, 7)).toString(), (Math.pow(2, 7) - 1).toString());
            this.getDataView().setInt8(offset.toInt(), value);
            return offset + 1;
        }

        /**
        * Writes a signed 16-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeInt16BE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 2);
            this.checkValue(value, (-Math.pow(2, 15).toLong()), (Math.pow(2, 15).toLong() - 1), (-Math.pow(2, 15)).toString(), (Math.pow(2, 15) - 1).toString());
            this.getDataView().setInt16(offset.toInt(), value, false);
            return offset + 2;
        }

        /**
        * Writes a signed 16-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeInt16LE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 2);
            this.checkValue(value, (-Math.pow(2, 15).toLong()), (Math.pow(2, 15).toLong() - 1), (-Math.pow(2, 15)).toString(), (Math.pow(2, 15) - 1).toString());
            this.getDataView().setInt16(offset.toInt(), value, true);
            return offset + 2;
        }

        /**
        * Writes a signed 32-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeInt32BE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.checkValue(value, (-Math.pow(2, 31).toLong()), (Math.pow(2, 31).toLong() - 1), (-Math.pow(2, 31)).toString(), (Math.pow(2, 31) - 1).toString());
            this.getDataView().setInt32(offset.toInt(), value, false);
            return offset + 4;
        }

        /**
        * Writes a signed 32-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeInt32LE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.checkValue(value, (-Math.pow(2, 31).toLong()), (Math.pow(2, 31).toLong() - 1), (-Math.pow(2, 31)).toString(), (Math.pow(2, 31) - 1).toString());
            this.getDataView().setInt32(offset.toInt(), value, true);
            return offset + 4;
        }

        /**
        * Writes an unsigned 8-bit integer to the buffer at the specified offset
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeUInt8(value: long, offset: int = 0): int {
            this.checkOffset(offset, 1);
            this.checkValue(value, 0, 255, "0", "255");
            this.getDataView().setUint8(offset.toInt(), value);
            return offset + 1;
        }

        /**
        * Writes an unsigned 16-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeUInt16BE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 2);
            this.checkValue(value, 0, (Math.pow(2, 16).toLong() - 1), "0", (Math.pow(2, 16) - 1).toString());
            this.getDataView().setUint16(offset.toInt(), value, false);
            return offset + 2;
        }

        /**
        * Writes an unsigned 16-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeUInt16LE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 2);
            this.checkValue(value, 0, (Math.pow(2, 16).toLong() - 1), "0", (Math.pow(2, 16) - 1).toString());
            this.getDataView().setUint16(offset.toInt(), value, true);
            return offset + 2;
        }

        /**
        * Writes an unsigned 32-bit integer to the buffer at the specified offset using big-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeUInt32BE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.checkValue(value, 0, (Math.pow(2, 32).toLong() - 1), "0", (Math.pow(2, 32) - 1).toString());
            this.getDataView().setUint32(offset.toInt(), value, false);
            return offset + 4;
        }

        /**
        * Writes an unsigned 32-bit integer to the buffer at the specified offset using little-endian format
        *
        * @param {long} value - Value to write
        * @param {int} [offset=0] - Number of bytes to skip before writing
        * @returns {int} Offset plus the number of bytes written
        */
        public writeUInt32LE(value: long, offset: int = 0): int {
            this.checkOffset(offset, 4);
            this.checkValue(value, 0, (Math.pow(2, 32).toLong() - 1), "0", (Math.pow(2, 32) - 1).toString());
            this.set32Litter(offset, value);
            return offset + 4;
        }

        private set32Litter(byteOffset: int, value: long): void {
            let len = this.length;
            if (byteOffset < 0 || byteOffset + 4 > len) {
                throw new RangeError("wrong index");
            }
            let bits = value;
            if (bits == Long.MAX_VALUE || bits == Long.MIN_VALUE) {
                bits = 0;
            }
            const startByte = byteOffset;
            for (let i = 0; i < 4; i++) {
                let byteVal = ((bits >>> (i * 8)) & 0xff).toByte();
                this.buffer.set(startByte + i, byteVal);
            }
        }

        /**
        * Returns the item at that index.
        *
        * @param { int } index - The zero-based index of the desired code unit.
        *     Throws error if index < 0 or index >= buffer.length.
        * @returns { int } The element in the buffer matching the given index.
        * @throws { BusinessError } 10200001 - The value of index is out of range.
        */
        $_get(index: int): long {
            if (index < 0 || index >= this.length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of "length" is out of range. \
It must be >= 0 and <= ${this.length}. Received value is: ${index}`)
            }
            return this.getDataView().getUint8(index.toInt()).toLong()
        }

        /**
         * Sets the byte at the specified index.
         *
         * @param {int} index – byte index to write
         * @param {long} value – byte value (0–255)
         */
        $_set(index: int, value: long): void {
            if (index < 0 || index >= this.length) {
                return;
            }
            this.getDataView().setUint8(index.toInt(), value);
        }

        /**
         * Converts this Buffer instance into a JsonElement.
         *
         * @returns {JsonElement} A new JsonElement containing the Buffer
         */
        public toJSON(): jsonx.JsonElement {
            const arr = new Array<jsonx.JsonElement>;
            for (let i: int = 0; i < this.length; ++i) {
                const intElement = new jsonx.JsonElement();
                intElement.setInteger(this.at(i).toInt());
                arr.push(intElement);
            }
            const objectElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
            objectElem.setElement("type", jsonx.JsonElement.createString("Buffer"));
            objectElem.setElement("data", jsonx.JsonElement.createArray(arr));
            return objectElem;
        }
    }

    export interface BlobOptions {
        /** The Blob content-type. The intent is for type to convey the MIME media type of the data, however no validation of the type format is performed. */
        type: string
        /** One of either 'transparent' or 'native'. When set to 'native', line endings in string source parts will be converted to the platform native line-ending as specified by require('node:os').EOL.*/
        endings: 'transparent'|'native'
    };

    type FixedArrayUnionType = FixedArray<string>
        | FixedArray<ArrayBuffer>
        | FixedArray<Int8Array>
        | FixedArray<Uint8Array>
        | FixedArray<Uint8ClampedArray>
        | FixedArray<Int16Array>
        | FixedArray<Uint16Array>
        | FixedArray<Int32Array>
        | FixedArray<Uint32Array>
        | FixedArray<Float32Array>
        | FixedArray<Float64Array>
        | FixedArray<BigInt64Array>
        | FixedArray<BigUint64Array>
        | FixedArray<DataView>
        | FixedArray<Blob>;

    type ArrayItemType = string
        | ArrayBuffer
        | Int8Array
        | Uint8Array
        | Uint8ClampedArray
        | Int16Array
        | Uint16Array
        | Int32Array
        | Uint32Array
        | Float32Array
        | Float64Array
        | BigInt64Array
        | BigUint64Array
        | DataView
        | Blob;

    type ArrayUnionType =  Array<Int8Array>
         | Array<Uint8Array>
         | Array<Uint8ClampedArray>
         | Array<Int16Array>
         | Array<Uint16Array>
         | Array<Int32Array>
         | Array<Uint32Array>
         | Array<Float32Array>
         | Array<Float64Array>
         | Array<BigInt64Array>
         | Array<BigUint64Array>
         | Array<string>
         | Array<ArrayBuffer>
         | Array<DataView>
         | Array<Blob>;

    /**
    * A Blob encapsulates immutable, raw data that can be safely shared across multiple worker threads.
    */
    export class Blob {
        /**
         * The content-size of the Blob.
         */
        get size(): int { return this.blobSize }

        /**
         * The content-type of the Blob.
         */
        get type(): string { return this.blobType }

        /**
         * The buffer of the Blob.
         */
        private arrBuffer: ArrayBuffer

        /**
         * Get the array of buffer.
         */
        private arr: ArrayUnionType

        /**
         * The size of the Blob.
         */
        private blobSize: int;

        /**
         * The type of the Blob.
         */
        private blobType: string;
        /**
         * Creates a new Blob object containing a concatenation of the given sources.
         * <ArrayBuffer>, <TypedArray>, <DataView>, and <Buffer> sources are copied into the 'Blob' and can therefore be safely modified after the 'Blob' is created.
         * String sources are encoded as UTF-8 byte sequences and copied into the Blob. Unmatched surrogate pairs within each string part will be replaced by Unicode U+FFFD replacement characters.
         * @param { FixedArray<string> | FixedArray<ArrayBuffer> | FixedArray<TypedArray> | FixedArray<DataView> | FixedArray<Blob> } [sources] - An array of string, <ArrayBuffer>, <TypedArray>, <DataView>, or <Blob> objects, or any mix of such objects, that will be stored within the Blob.
         * @param { BlobOptions } [options] - options
         */
        constructor(sources: FixedArrayUnionType, options?: BlobOptions) {
            this.handleParameters(sources)
            this.arrBuffer = this.flattenData(this.arr)
            this.blobSize = this.arrBuffer.byteLength;
            if (options != undefined) {
                this.blobType = options!.type ? options!.type! : ''
            } else {
                this.blobType = ''
            }
        }

        /**
         * Creates a new Blob object containing a concatenation of the given sources.
         * <ArrayBuffer>, <TypedArray>, <DataView>, and <Buffer> sources are copied into the 'Blob' and can therefore be safely modified after the 'Blob' is created.
         * String sources are encoded as UTF-8 byte sequences and copied into the Blob. Unmatched surrogate pairs within each string part will be replaced by Unicode U+FFFD replacement characters.
         * @param { Array<TypedArray> | Array<string> | Array<ArrayBuffer> | Array<DataView> | Array<Blob> } [sources] - An array of string, <ArrayBuffer>, <TypedArray>, <DataView>, or <Blob> objects, or any mix of such objects, that will be stored within the Blob.
         * @param { BlobOptions } [options] - options
         */
        constructor(sources: ArrayUnionType, options?: BlobOptions) {
            this.arrBuffer = this.flattenData(sources);
            this.blobSize = this.arrBuffer.byteLength;
            if (options != undefined) {
                this.blobType = options!.type ? options!.type! : ''
            } else {
                this.blobType = ''
            }
        }

        private constructor(sources: Array<ArrayBuffer>, typeKey: string, type?: string) {
            this.arrBuffer = this.flattenData(sources)
            this.blobSize = this.arrBuffer.byteLength;
            if (type != undefined) {
                this.blobType = type
            } else {
                this.blobType = ''
            }
        }

        /**
         * handle Parameters, and retrun ArrSrcType.
         */
        private handleParameters(sources: FixedArrayUnionType) {
            if (sources instanceof FixedArray<string>) {
                this.arr = Array.from(sources as FixedArray<string>)
            } else if (sources instanceof FixedArray<ArrayBuffer>) {
                this.arr = Array.from(sources as FixedArray<ArrayBuffer>);
            } else if (sources instanceof FixedArray<DataView>) {
                this.arr = Array.from(sources as FixedArray<DataView>);
            } else if (sources instanceof FixedArray<Int8Array>) {
                this.arr = Array.from(sources as FixedArray<Int8Array>);
            } else if (sources instanceof FixedArray<Uint8Array>) {
                this.arr = Array.from(sources as FixedArray<Uint8Array>);
            } else if (sources instanceof FixedArray<Uint8ClampedArray>) {
                this.arr = Array.from(sources as FixedArray<Uint8ClampedArray>);
            } else if (sources instanceof FixedArray<Int16Array>) {
                this.arr = Array.from(sources as FixedArray<Int16Array>);
            } else if (sources instanceof FixedArray<Uint16Array>) {
                this.arr = Array.from(sources as FixedArray<Uint16Array>);
            } else if (sources instanceof FixedArray<Int32Array>) {
                this.arr = Array.from(sources as FixedArray<Int32Array>);
            } else if (sources instanceof FixedArray<Uint32Array>) {
                this.arr = Array.from(sources as FixedArray<Uint32Array>);
            } else if (sources instanceof FixedArray<Float32Array>) {
                this.arr = Array.from(sources as FixedArray<Float32Array>);
            } else if (sources instanceof FixedArray<Float64Array>) {
                this.arr = Array.from(sources as FixedArray<Float64Array>);
            } else if (sources instanceof FixedArray<BigInt64Array>) {
                this.arr = Array.from(sources as FixedArray<BigInt64Array>);
            } else if (sources instanceof FixedArray<BigUint64Array>) {
                this.arr = Array.from(sources as FixedArray<BigUint64Array>);
            } else {
                this.arr = Array.from(sources as FixedArray<Blob>);
            }
        }

        private transformToArrayBuffer(value: ArrayItemType): ArrayBuffer {
            if (value instanceof String) {
                return ArrayBuffer.from(value, "utf-8");
            } else if (value instanceof ArrayBuffer) {
                return value;
            } else if (value instanceof DataView) {
                return value.buffer;
            }  else if (value instanceof Blob) {
                return value.getArrayBuffer() as ArrayBuffer;
            } else {
                return value.buffer as ArrayBuffer;
            }
        }

        private flattenData(value: ArrayUnionType): ArrayBuffer {
            const resolvedList = new Array<ArrayBuffer>();
            for (const item of value) {
                resolvedList.push(this.transformToArrayBuffer(item));
            }
            const bufferLength = resolvedList.reduce(
                (acc: number, item: ArrayBuffer): number => acc + item.byteLength, 0);
            const listOfBytes = new Array<byte>();
            for (const item of resolvedList) {
                for (let i = 0; i < item.byteLength; i++) {
                    listOfBytes.push(item.at(i));
                }
            }
            const buffer = new ArrayBuffer(bufferLength);
            let offset = 0;
            for (let i = 0; i < listOfBytes.length; i++) {
                buffer.set(offset, listOfBytes[i]);
                offset += 1;
            }
            return buffer;
        }

        /**
         * Get flattened ArrayBuffer through sync interface.
         */
        getArrayBuffer(): ArrayBuffer {
            return ArrayBuffer.from(this.arrBuffer as ArrayBuffer);
        }

        /**
         * Creates and returns a new Blob containing a subset of this Blob objects data. The original Blob is not altered.
         *
         * @param {int} [start=0] - The starting index.
         * @param {int} [end=buffer.length] -  The ending index.
         * @param {string} [type] - The content-type for the new Blob
         * @returns {Blob}
         */
        slice(start?: int, end?: int, type?: string): Blob {
            if (start == undefined) {
                start = 0;
            }
            const slicedArrayBuffer = this.arrBuffer.slice(start, end) as ArrayBuffer;
            return new Blob([slicedArrayBuffer], "ArrayBuffer", type)
        }

        /**
         * Returns a promise that fulfills with the contents of the Blob decoded as a UTF-8 string.
         *
         * @returns {Promise}
         */
        text(): Promise<string> {
            let Encoding: string = "utf8";
            let resolvedStart: int = 0;
            let resolvedEnd: int = this.arrBuffer.byteLength as int;
            let decodeData: string = ArrayBuffer.stringify(this.arrBuffer, Encoding, resolvedStart, resolvedEnd);
            return Promise.resolve(decodeData);
        }

        /**
         * Returns a promise that fulfills with an <ArrayBuffer> containing a copy of the Blob data.
         *
         * @returns {Promise}
         */
        arrayBuffer(): Promise<ArrayBuffer> {
            return Promise.resolve(ArrayBuffer.from(this.arrBuffer as ArrayBuffer))
        }
    }
}
