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

import { BusinessError, RecordData } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const percentRate = 100;
const syntaxErrorCode: int = 10200002;
export type PromisifiedFunc = (...args: FixedArray<Any>) => Promise<Any>;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export namespace util {

    export class LRUCache<K, V> {
        private cache: Map<K, V>;
        // Default current size
        private maxSize: int;
        // Default maximum size
        private putCount: long = 0;
        private createCount: long = 0;
        private evictionCount: long = 0;
        private hitCount: long = 0;
        private missCount: long = 0;

        get length(): int {
            return this.cache.size;
        }

        public constructor(capacity?: int) {
            if (capacity == undefined) {
                this.maxSize = 64;
            } else {
                if (this.checkCapacityIsIllegal(capacity)) {
                    throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${capacity} must be small integer`)
                }
                this.maxSize = capacity;
            }
            this.cache = new Map<K, V>();
        }

        private checkCapacityIsIllegal(capacity: int): boolean {
            return (capacity <= 0 || (capacity % 1) != 0);
        }

        private getLRUKey(): K {
            return this.cache.keys().next().value as K;
        }

        private getLRUValue(): V {
            return this.cache.values().next().value as V;
        }

        private reduceCapacity(newCapacity: int): void {
            while (this.cache.keys().next() && this.cache.size > newCapacity) {
                let lruKey = this.getLRUKey();
                this.afterRemoval(true, lruKey, this.getLRUValue(), undefined);
                this.cache.delete(lruKey);
                this.evictionCount++;
            }
        }

        protected afterRemoval(isEvict: boolean, key: K | undefined, value: V | undefined,
            newValue: V | undefined): void {
        }

        public updateCapacity(newCapacity: int): void {
            if (this.checkCapacityIsIllegal(newCapacity)) {
                throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${newCapacity} must be small integer`)
            } else if (this.cache.size > newCapacity) {
                this.reduceCapacity(newCapacity);
            }
            this.maxSize = newCapacity;
        }

        public getCapacity(): int {
            return this.maxSize;
        }

        public clear(): void {
            this.cache.forEach((v, k) => {
                this.afterRemoval(false, k, v, undefined);
            })
            this.cache.clear();
        }

        public isEmpty(): boolean {
            return (this.cache.size == 0);
        }

        public put(key: K, value: V): V | undefined {
            if (key == undefined) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${key} must be Object`);
            }
            if (value === undefined) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${value} must be Object`);
            }
            if (key === null || value === null) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of key and value must be Object`);
            }
            let valueBefore: V | undefined = undefined;
            this.putCount++;
            if (this.cache.has(key)) {
                valueBefore = this.cache.get(key);
                this.cache.delete(key);
                this.afterRemoval(false, key, valueBefore, value);
            }
            this.cache.set(key, value);
            if (this.cache.size > this.maxSize) {
                this.reduceCapacity(this.maxSize);
            }
            return valueBefore;
        }

        override toString(): String {
            let peek: long = 0;
            let hitRate: long = 0;
            peek = this.hitCount + this.missCount;
            if (peek != 0) {
                // The value is 100 times larger
                hitRate = percentRate * this.hitCount / peek;
            }
            return 'LRUCache[ maxSize = ' + this.maxSize + ', hits = ' + this.hitCount +
                ', misses = ' + this.missCount + ', hitRate = ' + hitRate + '% ]';
        }

        public getCreateCount(): long {
            return this.createCount;
        }

        public getMissCount(): long {
            return this.missCount;
        }

        public getRemovalCount(): long{
            return this.evictionCount;
        }

        public getMatchCount(): long {
            return this.hitCount;
        }

        public getPutCount(): long {
            return this.putCount;
        }

        public $_iterator(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }

        public entries(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }

        public keys(): Array<K> {
            let arr = Array.from<K>(this.cache.keys());
            return arr;
        }

        public values(): Array<V> {
            let arr = Array.from<V>(this.cache.values());
            return arr;
        }

        public remove(key: K): V | undefined {
            if (key == null) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${key} must be Object`);
            }
            if (this.cache.has(key)) {
                let former: V | undefined = this.cache.get(key);
                this.cache.delete(key);
                if (former != undefined) {
                    this.afterRemoval(false, key, former, undefined);
                    return former;
                }
            }
            return undefined;
        }

        public get(key: K): V | undefined {
            if (key == null) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${key} must be Object`);
            }
            if (this.cache.has(key)) {
                return this.containsInner(key);
            }
            this.missCount++;
            let createValue: V | undefined = this.createDefault(key);
            if (createValue == undefined) {
                return undefined;
            } else {
                this.put(key, createValue);
                this.createCount++;
                return createValue;
            }

        }

        public createDefault(key: K): V | undefined {
            return undefined;
        }

        private containsInner(key: K): V | undefined {
            this.hitCount++;
            let value: V | undefined = this.cache.get(key);
            this.cache.delete(key);
            this.cache.set(key, value as V);
            return value;
        }

        public contains(key: K): boolean {
            if (key == null) {
              throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${key} must be Object`);
            }
            if (this.cache.has(key)) {
                this.containsInner(key);
                return true;
            }
            this.missCount++;
            return false;
        }
    }

    export enum Type {
        BASIC,
        MIME,
        BASIC_URL_SAFE,
        MIME_URL_SAFE,
    }

    export class Base64Helper {
        private static BASE64_CHARS: string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        private static BASE64_URL_SAFE_CHARS: string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        private static readonly BYTE_SIZE = 8;
        private static readonly BASE64_GROUP_SIZE = 6;
        private static readonly BASE64_MASK = 0x3F;
        private static readonly LINE_BREAK_FREQUENCY = 76;

        constructor() { }

        /**
         * Encodes the given byte array into a Base64 string.
         * @param bytes The byte array to encode.
         * @param urlSafe If true, uses the URL-safe Base64 alphabet.
         * @param mime If true, inserts line breaks according to MIME specifications.
         * @returns The Base64 encoded string.
         */
        private static encodeBase64(bytes: Uint8Array, urlSafe: boolean, mime: boolean): string {
            let table = urlSafe ? Base64Helper.BASE64_URL_SAFE_CHARS : Base64Helper.BASE64_CHARS;
            let encoded: string = "";
            let buffer: int = 0;
            let bufferLength: int = 0;

            for (let i = 0; i < bytes.length; i++) {
                buffer = (buffer << Base64Helper.BYTE_SIZE) | bytes[i].toInt();
                bufferLength += Base64Helper.BYTE_SIZE;
                while (bufferLength >= Base64Helper.BASE64_GROUP_SIZE) {
                    bufferLength -= Base64Helper.BASE64_GROUP_SIZE;
                    encoded += table[(buffer >> bufferLength) & Base64Helper.BASE64_MASK];
                }
            }

            if (bufferLength > 0) {
                encoded += table[(buffer << (Base64Helper.BASE64_GROUP_SIZE - bufferLength)) & Base64Helper.BASE64_MASK];
            }

            while (encoded.length % 4 !== 0 && !urlSafe) {
                encoded += "=";
            }

            if (mime) {
                encoded = encoded.replace(new RegExp(`(.{${Base64Helper.LINE_BREAK_FREQUENCY}})`, "g"), "$1\r\n");
                if (!encoded.endsWith("\r\n")) {
                    encoded += "\r\n";
                }
            }
            return encoded;
        }

        /**
         * Decodes the given Base64 string into a byte array.
         * @param base64 The Base64 string to decode.
         * @param urlSafe If true, uses the URL-safe Base64 alphabet.
         * @returns A Uint8Array containing the decoded bytes.
         */
        private static decodeBase64(base64: string, urlSafe: boolean): Uint8Array {
            let table = urlSafe ? Base64Helper.BASE64_URL_SAFE_CHARS : Base64Helper.BASE64_CHARS;
            let allowedPattern = urlSafe ? "[^A-Za-z0-9\\-_=]" : "[^A-Za-z0-9+/=]";
            let cleaned: string = base64.replace(new RegExp(allowedPattern, "g"), "");
            let buffer: int = 0;
            let bufferLength: int = 0;
            let bytes: Array<number> = new Array<number>();

            for (let i = 0; i < cleaned.length; i++) {
                let ch = cleaned.charAt(i);
                if (ch == c'=') {
                    break;
                }
                let index: int = table.indexOf(ch).toInt();
                if (index == -1) {
                    continue;
                }
                buffer = (buffer << Base64Helper.BASE64_GROUP_SIZE) | index;
                bufferLength += Base64Helper.BASE64_GROUP_SIZE;
                if (bufferLength >= Base64Helper.BYTE_SIZE) {
                    bufferLength -= Base64Helper.BYTE_SIZE;
                    bytes.push((buffer >> bufferLength) & 0xFF);
                }
            }
            return new Uint8Array(bytes);
        }

        /**
         * Synchronously encodes the input byte array into a Base64 encoded Uint8Array.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Uint8Array containing the Base64 encoded string.
         */
        encodeSync(src: Uint8Array, options?: Type): Uint8Array {
            if (options === undefined) {
                options = Type.BASIC;
            }
            if (options != Type.BASIC && options != Type.BASIC_URL_SAFE){
                throw createBusinessError(TypeErrorCodeId,
                    `Parameter error. The target encoding type option must be BASIC or BASIC_URL_SAFE.`);
            }
            let encoded: string = Base64Helper.encodeBase64(src, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE, options === Type.MIME || options === Type.MIME_URL_SAFE);
            let result: Uint8Array = new Uint8Array(encoded.length);
            for (let i: int = 0; i < encoded.length; i++) {
                result[i] = encoded.charCodeAt(i);
            }
            return result;
        }

        /**
         * Synchronously encodes the input byte array into a Base64 encoded string.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A string containing the Base64 encoded result.
         */
        encodeToStringSync(src: Uint8Array, options?: Type): string {
            if (options === undefined) {
                options = Type.BASIC;
            }
            return Base64Helper.encodeBase64(src, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE, options === Type.MIME || options === Type.MIME_URL_SAFE);
        }

        /**
         * Synchronously decodes a Base64 encoded Uint8Array into a byte array.
         * @param src A Uint8Array containing the Base64 encoded string or the Base64 encoded string.
         * @param options The decoding options (Base64 type).
         * @returns A Uint8Array with the decoded bytes.
         */
        decodeSync(src: Uint8Array | string, options?: Type): Uint8Array {
            if (options === undefined) {
                options = Type.BASIC;
            }
            if (src instanceof Uint8Array) {
                return this.decodeSyncUint8Array(src, options);
            } else {
                return this.decodeSyncString(src, options);
            }
        }

        private decodeSyncUint8Array(src: Uint8Array, options: Type): Uint8Array {
            let base64String: string;
            let charArray: Array<string> = new Array<string>();
            for (let i: int = 0; i < src.length; i++) {
                charArray.push(String.fromCharCode(src[i]));
            }
            base64String = charArray.join("");
            if (options === Type.MIME || options === Type.MIME_URL_SAFE) {
                base64String = base64String.replace(new RegExp("[\\r\\n]", "g"), "");
            }
            return Base64Helper.decodeBase64(base64String, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE);
        }

        private decodeSyncString(src: string, options: Type): Uint8Array {
            let newString: string = src;
            if (options === Type.MIME || options === Type.MIME_URL_SAFE) {
                newString = src.replace(new RegExp("[\\r\\n]", "g"), "");
            }
            return Base64Helper.decodeBase64(newString, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE);
        }

        /**
         * Asynchronously encodes the input byte array into a Base64 encoded Uint8Array.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Promise that resolves to a Uint8Array containing the Base64 encoded string.
         */
        encode(src: Uint8Array, options?: Type): Promise<Uint8Array> {
            if (options === undefined) {
                options = Type.BASIC;
            }
            return Promise.resolve(this.encodeSync(src, options));
        }

        /**
         * Asynchronously decodes a Base64 encoded string into a byte array.
         * @param src The Base64 encoded string or a Uint8Array containing the Base64 encoded string.
         * @param options The decoding options (Base64 type).
         * @returns A Promise that resolves to a Uint8Array with the decoded bytes.
         */
        decode(src: Uint8Array | string, options?: Type): Promise<Uint8Array> {
            if (options === undefined) {
                options = Type.BASIC;
            }
            if (src instanceof Uint8Array) {
                return this.decodeUint8Array(src, options);
            } else {
                return this.decodeString(src, options);
            }
        }

        private decodeString(src: string, options: Type): Promise<Uint8Array> {
            return Promise.resolve(this.decodeSync(src, options));
        }

        private decodeUint8Array(src: Uint8Array, options: Type): Promise<Uint8Array> {
            return Promise.resolve(this.decodeSync(src, options));
        }

        /**
         * Asynchronously encodes the input byte array into a Base64 encoded string.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Promise that resolves to a string containing the Base64 encoded result.
         */
        encodeToString(src: Uint8Array, options?: Type): Promise<string> {
            if (options === undefined) {
                options = Type.BASIC;
            }
            return Promise.resolve(this.encodeToStringSync(src, options));
        }
    }

    /**
    * The util.format() method returns a formatted string using the first argument as a printf-like
    * format string which can contain zero or more format specifiers.
    * Each specifier is replaced with the converted value from the corresponding argument.
    * Supported specifiers are:
    * %s: String will be used to convert all values except BigInt, Object and -0. BigInt values will be represented
    *     with an n and Objects that have no user defined toString function are inspected using util.inspect()
    *     with options { depth: 0, colors: false, compact: 3 }.
    * %d: Number will be used to convert all values except BigInt and Char.
    * %i: parseInt(value, 10) is used for all values except BigInt and Char.
    * %f: parseFloat(value) is used for all values expect Char.
    * %j: JSON. Replaced with the string '[Circular]' if the argument contains circular references.
    * %o: Object. A string representation of an object with generic JavaScript object formatting.
    *     Similar to util.inspect() with options { showHidden: true, showProxy: true }.
    *     This will show the full object including non-enumerable properties and proxies.
    * %O: Object. A string representation of an object with generic JavaScript object formatting.
    *     Similar to util.inspect() without options. This will show the full object not including non-enumerable
    *     properties and proxies.
    * %c: [DEPRECATED] - this specifier is ignored with they argument
    * %%: single percent sign ('%'). This does not consume an argument.
    *
    * @param s: <string> A printf-like format string.
    *
    * @param args: <Object[]> format params
    *
    * @returns Returns: <string> The formatted string
    */
    export function format(s: string, ...args: Object[]): string {
        // arrays are invariant, need to copy
        let a = new Array<Any>(args.length)
        for (let i = 0; i < args.length; ++i) {
            a[i] = args[i]
        }
        return (new Formatter).format(s, a)
    }

    export function generateRandomUUID(entropyCache?: boolean): string {
        let s = UtilHelper.generateRandomUUID((entropyCache == undefined) ? false : entropyCache)
        return s
    }

    export interface ScopeComparable<T> {

        /**
        * The comparison function is used by the scope.
        *
        * @param { ScopeComparable } other - Other
        * @returns { boolean } Returns whether the current object is greater than or equal to the input object.
        */
        compareTo(other: T): boolean;
    }

    /**
    * A type used to denote ScopeComparable or number.
    *
    * @typedef { ScopeComparable }
    */
    export type ScopeType<T> = ScopeComparable<T>;

    export class ScopeHelper<T extends ScopeComparable<T>> {
        private readonly _lowerLimit: T
        private readonly _upperLimit: T

        /**
        * A constructor used to create a Scope instance with the lower and upper bounds specified.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        */
        constructor(lowerObj: T, upperObj: T) {
            if (lowerObj.compareTo(upperObj)) {
                throw new Error('lower limit must be less than upper limit');
            }
            this._lowerLimit = lowerObj
            this._upperLimit = upperObj
        }

        /**
        * Obtains a string representation of the current range.
        *
        * @returns { string } Returns a string representation of the current range object.
        */
        toString(): string {
            let strLower: string = this._lowerLimit.toString()
            let strUpper: string = this._upperLimit.toString()
            return '[' + strLower + ', ' + strUpper + ']'
        }

        /**
        * Returns the intersection of a given range and the current range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the intersection of a given range and the current range.
        */
        intersect(range: ScopeHelper<T>): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(range.getLower()) ? this._lowerLimit : range.getLower()
            let upperLimit = this._upperLimit.compareTo(range.getUpper()) ? range.getUpper() : this._upperLimit
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Returns the intersection of the current range and the range specified by the given lower and upper bounds.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        * @returns { ScopeHelper } Returns the intersection of the current range and the range specified by the given lower and upper bounds.
        */
        intersect(lowerObj: T, upperObj: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(lowerObj) ? this._lowerLimit : lowerObj
            let upperLimit = this._upperLimit.compareTo(upperObj) ? upperObj : this._upperLimit
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Obtains the upper bound of the current range.
        *
        * @returns { ScopeType } Returns the upper bound of the current range.
        */
        getUpper(): T {
            return this._upperLimit
        }

        /**
        * Obtains the lower bound of the current range.
        *
        * @returns { ScopeType } Returns the lower bound of the current range.
        */
        getLower(): T {
            return this._lowerLimit
        }

        /**
        * Creates the smallest range that includes the current range and the given lower and upper bounds.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and the given lower and upper bounds.
        */
        expand(lowerObj: T, upperObj: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(lowerObj) ? lowerObj : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(upperObj) ? this._upperLimit : upperObj
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Creates the smallest range that includes the current range and a given range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given range.
        */
        /**
        * Creates the smallest range that includes the current range and a given range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given range.
        */
        expand(range: ScopeHelper<T>): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(range.getLower()) ? range.getLower() : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(range.getUpper()) ? this._upperLimit : range.getUpper()
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Creates the smallest range that includes the current range and a given value.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given value.
        */
        expand(value: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(value) ? value : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(value) ? this._upperLimit : value
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Checks whether a given value is within the current range.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { boolean } If the value is within the current range return true,otherwise return false.
        */
        contains(value: T): boolean {
            let lowerRes = value.compareTo(this._lowerLimit)
            let upperRes = this._upperLimit.compareTo(value)
            return lowerRes && upperRes
        }

        /**
        * Checks whether a given range is within the current range.
        *
        * @param { ScopeHelper } range - A Scope range
        * @returns { boolean } If the current range is within the given range return true,otherwise return false.
        */
        contains(range: ScopeHelper<T>): boolean {
            let lowerRes = range.getLower().compareTo(this._lowerLimit)
            let upperRes = this._upperLimit.compareTo(range.getUpper())
            return lowerRes && upperRes
        }

        /**
        * Clamps a given value to the current range.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { ScopeType } Returns a ScopeType object that a given value is clamped to the current range.
        */
        clamp(value: T): T {
            if (!value.compareTo(this._lowerLimit)) {
                return this._lowerLimit
            } else if (value.compareTo(this._upperLimit)) {
                return this._upperLimit
            } else {
                return value
            }
        }
    }
    export interface TextDecoderOptions {
        fatal?: boolean;
        ignoreBOM?: boolean;
    }

    export interface DecodeToStringOptions {
        stream?: boolean;
    }
    export class TextDecoder {
        // Use static properties to manage Finalization Registry and avoid module level exports
        private static destroyRegistry = new FinalizationRegistry<long>(
            (nativePtr: long) => TextDecoder.nativeDestroy(nativePtr)
        );
        private static unregisterToken = new object();

        static { loadLibrary("ets_sdk_native") }

        private encoding_: string = "utf-8";
        private fatal_: boolean = false;
        ignoreBOM_: boolean = false;
        readonly CONVERTER_FLAGS_FATAL = 0x2;
        readonly CONVERTER_FLAGS_IGNORE_BOM = 0x4;
        private nativeDecoder_: long;

        constructor(encoding?: string, options?: TextDecoderOptions) {
            let flags = 0;
            if (encoding) {
                this.encoding_ = encoding;
            }
            if (options) {
                flags |= options.fatal ? this.CONVERTER_FLAGS_FATAL : 0;
                flags |= options.ignoreBOM ? this.CONVERTER_FLAGS_IGNORE_BOM : 0;
                this.fatal_ = options.fatal ?? false;
                this.ignoreBOM_ = options.ignoreBOM ?? false;
            }
            this.bindNativeDecoder(this.encoding_, flags);
            TextDecoder.destroyRegistry.register(this, this.nativeDecoder_, TextDecoder.unregisterToken);
        }

        static create(encoding?: string, options?: TextDecoderOptions): TextDecoder {
            return new TextDecoder(encoding, options);
        }

        public decodeToString(input: Uint8Array, options?: DecodeToStringOptions): string {
            if (input.length === 0) {
                return '';
            }
            return this.decode(input, options?.stream ?? false);
        }

        get encoding(): string {
            return this.encoding_;
        }

        get fatal(): boolean {
            return this.fatal_;
        }

        get ignoreBOM(): boolean {
            return this.ignoreBOM_;
        }

        private native bindNativeDecoder(encoding: string, flags: int): void;
        private native decode(input: Uint8Array, decodeWithStream: boolean): string;
        private static native nativeDestroy(ptr: long): void;
    }
    export class RationalNumber {
        private mnum: long = 0;
        private mden: long = 0;

        constructor() {}

        /**
         * Check whether the string is number.
         * @param { string } str a rationalNumber object of the string type.
         * @returns { boolean } when the str can be parsed into a finite number return true otherwise return false.
         */
        static isNumeric(str: string): boolean {
            let val = Double.parseFloat(str)
            return !isNaN(val) && isFinite(val);
        }

        /**
         * Used to create a RationalNumber instance with a given numerator and denominator.
         * @param { long } num represents the numerator.
         * @param { long } den represents the denominator.
         * @returns { RationalNumber } return a RationalNumber object.
         */
        static parseRationalNumber(num: long, den: long): RationalNumber {
            let sign = (num < 0) !== (den < 0) ? -1 : 1;
            num = Math.abs(num).toLong();
            den = Math.abs(den).toLong();
            let ratNum = new RationalNumber();
            if (den === 0) {
                if (num !== 0) {
                    ratNum.mnum = sign;
                    ratNum.mden = 0;
                } else {
                    ratNum.mnum = 0;
                    ratNum.mden = 0;
                }
            } else if (num === 0) {
                ratNum.mnum = 0;
                ratNum.mden = 1;
            } else {
                let gnum = RationalNumber.getCommonFactor(num, den);
                if (gnum !== 0) {
                    ratNum.mnum = (num / gnum) * sign;
                    ratNum.mden = den / gnum;
                }
            }
            return ratNum;
        }

        /**
         * Create a RationalNumber object based on a given string.
         * @param { string } str represents a string.
         * @returns { RationalNumber } return a RationalNumber object.
         */
        static createRationalFromString(str: string): RationalNumber {
            const regex = new RegExp('^(.*)([:\\/])(.*)$');
            const match = str.match(regex);
            if (!match) {
                throw createBusinessError(TypeErrorCodeId,
                    `Parameter error. The type of ${str} must be effective string`);
            }
            const str1 = match[1]!;
            const separator = match[2];
            const str2 = match[3]!;
            if (RationalNumber.isNumeric(str1) && RationalNumber.isNumeric(str2)) {
                const num1 = Number(str1);
                 const num2 = Number(str2);
                if (!Number.isInteger(num1) || !Number.isInteger(num2)) {
                    console.error('createRationalFromString: The type of Parameter must be integer string');
                }
                return RationalNumber.parseRationalNumber(num1.toLong(), num2.toLong());
            } else {
                throw createBusinessError(TypeErrorCodeId,
                    `Parameter error. The type of ${str} must be character string`);
            }
        }

        /**
         * Compares the current RationalNumber object with the target RationalNumber object and returns the comparison result.
         * @param { RationalNumber } other RationalNumber object.
         * @returns { number } if equal return 0, If the givens smaller than the current return 1, otherwise return -1.
         */
        public compare(other: RationalNumber): int {
            if (this.mnum === other.mnum && this.mden === other.mden) {
                return 0;
            } else if (this.isNaN()) {
                return 1;
            } else if (other.isNaN()) {
                return -1;
            } else if ((this.mden === 0 && this.mnum > 0) || (other.mden === 0 && other.mnum < 0)) {
                return 1;
            } else if ((this.mden === 0 && this.mnum < 0) || (other.mden === 0 && other.mnum > 0)) {
                return -1;
            }
            let thisnum: number = this.mnum * other.mden;
            let othernum: number = other.mnum * this.mden;
            if (thisnum < othernum) {
                return -1;
            } else if (thisnum > othernum) {
                return 1;
            } else {
                return 0;
            }
        }

        /**
         * Compare the current RationalNumber object to the given object equally.
         * @param { object } obj the given object.
         * @returns { boolean } if the given object is same as the current object return true otherwise return false.
         */
        public equals(obj: object): boolean {
            if (!(obj instanceof RationalNumber)) {
                return false;
            }
            return this.compare(obj) === 0;
        }

        /**
         * Gets the value of the current RationalNumber object as an integer or float number.
         *
         * @returns { double } returns the value of an integer or float number.
         */
        public valueOf(): double {
            if (this.mnum > 0 && this.mden === 0) {
                return Number.POSITIVE_INFINITY;
            } else if (this.mnum < 0 && this.mden === 0) {
                return Number.NEGATIVE_INFINITY;
            } else if (this.isNaN()) {
                return Number.NaN;
            } else {
                return Number(this.mnum) / Number(this.mden);
            }
        }

        /**
         * Gets the greatest common divisor of two specified integers.
         * @param { long } firNum the integer.
         * @param { long } SecNum the integer.
         * @returns { long } returns the greatest common divisor of firNum and SecNum.
         */
        static getCommonFactor(firNum: long, SecNum: long): long {
            if (firNum === 0 || SecNum === 0) {
                throw createBusinessError(TypeErrorCodeId,
                    `Parameter error. The Parameter cannot be zero`);
            }
            return RationalNumber.getGCD(firNum, SecNum);
        }

        private static getGCD(firNum: long, SecNum: long): long {
            let temp: long = 0;
            if (firNum < SecNum) {
                temp = firNum;
                firNum = SecNum;
                SecNum = temp;
            }
            while (firNum % SecNum !== 0) {
                temp = firNum % SecNum;
                firNum = SecNum;
                SecNum = temp;
            }
            return SecNum;
        }

        /**
         * Gets the denominator of the current RationalNumber object.
         *
         * @returns { long } returns the denominator.
         */
        public getDenominator(): long {
            return this.mden;
        }

        /**
         * Gets the numerator of the current RationalNumber object.
         *
         * @returns { long } returns the numerator.
         */
        public getNumerator(): long {
            return this.mnum;
        }

        /**
         * Check if the current RationalNumber object represents a finite value.
         *
         * @returns { boolean } if the denominator is not 0 return true otherwise return false.
         */
        public isFinite(): boolean {
            return this.mden !== 0;
        }

        /**
         * Check if the current RationalNumber object represents a finite value.
         *
         * @returns { boolean } if both the denominator and the numerator are 0 return true otherwise return false.
         */
        public isNaN(): boolean {
            return this.mnum === 0 && this.mden === 0;
        }

        /**
         * Check whether the current RationalNumber object is 0.
         *
         * @returns { boolean } if the value represented by the current object is 0 return true Otherwise return false.
         */
        public isZero(): boolean {
            return this.mnum === 0 && this.mden !== 0;
        }

        /**
         * Gets the string representation of the current RationalNumber object.
         *
         * @returns { string } returns a string in Numerator/Denominator format if the denominator is 0 return Infinity.
         * If both the numerator and denominator are 0 return NaN.
         */
        public toString(): string {
            let buf: string;
            if (this.isNaN()) {
                buf = 'NaN';
            } else if (this.mnum > 0 && this.mden === 0) {
                buf = 'Infinity';
            } else if (this.mnum < 0 && this.mden === 0) {
                buf = '-Infinity';
            } else {
                buf = String(this.mnum) + '/' + String(this.mden);
            }
            return buf;
        }
    }

    export interface EncodeIntoUint8ArrayInfo {
        read: int;
        written: int;
    }

    export class TextEncoder {

        static { loadLibrary("ets_sdk_native") }

        private encoding_: string = "utf-8";

        constructor(encoding?: string)
        {
            if (!encoding) {
                this.encoding_ = "utf-8";
                return;
            }
            let tempString: string = encoding.toLowerCase();
            if (!(TextEncoder.KNOWN_ENCODINGS.has(tempString))) {
                throw createBusinessError(TypeErrorCodeId, `Wrong encoding format, the current '${encoding}' format is not support.`)
            }
            this.encoding_ = encoding;
        }

        static create(encoding?: string): TextEncoder
        {
            return new TextEncoder(encoding);
        }

        encodeInto(input?: string): Uint8Array
        {
            if (!input) {
                return new Uint8Array(0);
            }
            return new Uint8Array(TextEncoder.doEncodeInto(input, this.encoding_));
        }

        encodeIntoUint8Array(input: string, dest: Uint8Array): EncodeIntoUint8ArrayInfo
        {
            let res: Array<int> = TextEncoder.doEncodeInfoUint8Array(input, this.encoding_, dest);
            let result: EncodeIntoUint8ArrayInfo = {read: 0, written: 0};
            result.read = res[0];
            result.written = res[1];
            return result;
        }

        get encoding(): string {
            return this.encoding_;
        }

        private static readonly KNOWN_ENCODINGS: Set<string> = new Set<string>([
            "utf-8", "utf-16be", "utf-16le", "gbk", "gb2312", "gb18030", "ibm866",
            "iso-8859-1", "iso-8859-2", "iso-8859-3", "iso-8859-4", "iso-8859-5", "iso-8859-6",
            "iso-8859-7", "iso-8859-8", "iso-8859-8-i", "iso-8859-10", "iso-8859-13",
            "iso-8859-14", "iso-8859-15", "koi8-r", "koi8-u", "macintosh",
            "windows-874", "windows-1250", "windows-1251", "windows-1252",
            "windows-1253", "windows-1254", "windows-1255", "windows-1256",
            "windows-1257", "windows-1258", "big5", "euc-jp", "iso-2022-jp",
            "shift_jis", "euc-kr", "x-mac-cyrillic"
        ]);

        private static native doEncodeInto(
            input: string, inputEncoding: string): ArrayBuffer;

        private static native doEncodeInfoUint8Array(
            input: string, inputEncoding: string, destArray: Uint8Array): Array<int>;
    }

    export class StringDecoder {
        static { loadLibrary("ets_sdk_native"); }

        private encoding_: string = 'utf-8';
        private nativeDecoder_: long;
        private static knownEncodings: FixedArray<string> = [
            "utf-8", "utf-16be", "utf-16le", "gbk", "gb2312", "gb18030", "ibm866",
            "iso-8859-1", "iso-8859-2", "iso-8859-3", "iso-8859-4", "iso-8859-5", "iso-8859-6",
            "iso-8859-7", "iso-8859-8", "iso-8859-8-i", "iso-8859-10", "iso-8859-13",
            "iso-8859-14", "iso-8859-15", "koi8-r", "koi8-u", "macintosh",
            "windows-874", "windows-1250", "windows-1251", "windows-1252",
            "windows-1253", "windows-1254", "windows-1255", "windows-1256",
            "windows-1257", "windows-1258", "big5", "euc-jp", "iso-2022-jp",
            "shift_jis", "euc-kr", "x-mac-cyrillic"
        ];

        constructor(encoding?: string)
        {
            if (!encoding) {
                this.encoding_ = 'utf-8';
            } else {
                let tempString: string = encoding.toLowerCase();
                let flag: boolean = StringDecoder.checkEncodingFormat(tempString);
                if (!flag) {
                    throw createBusinessError(TypeErrorCodeId,
                        `Wrong encoding format, the current '${encoding}' format is not support.`);
                }
                this.encoding_ = tempString;
            }
            this.bindNativeStringDecoder(this.encoding_);
        }

        write(chunk: string | Uint8Array): string
        {
            if (chunk instanceof string) {
                return chunk;
            }
            return this.doWrite(chunk);
        }

        end(chunk?: string | Uint8Array): string
        {
            if (chunk && chunk instanceof string) {
                return chunk;
            }
            return this.doEnd(chunk);
        }

        private static checkEncodingFormat(encoding: string): boolean
        {
            for (const enc of StringDecoder.knownEncodings) {
                if (enc == encoding) {
                    return true;
                }
            }
            return false;
        }

        private native bindNativeStringDecoder(encoding: string): void;

        private native doWrite(chunk: Uint8Array): string;

        private native doEnd(chunk?: Uint8Array): string;
    }

    export class UtilHelper {

        static {
            loadLibrary("ets_sdk_native")
        }

        static native generateRandomUUID(entropyCache: boolean): string

        static native generateRandomBinaryUUID(entropyCache: boolean): Uint8Array

        static native getErrorString(err: int): string

        private static objectToString(obj: Object, full: boolean = false): string {
            return (new UtilHelper).stringifyObject(obj, '\n', 1, full)
        }

        private static isNullObject(obj: Any): boolean {
            return (obj == undefined) || (obj == null)
        }

        private static isFunction(obj: Object): boolean {
            return (typeof obj === 'function')
        }

        private static isObject(obj: Object): boolean {
            return (typeof obj === 'object')
        }

        private static isString(obj: Object): boolean {
            return (typeof obj === 'string')
        }

        private static isArray(obj: Object): boolean {
            return obj instanceof Array
        }

        private stringifySubObject(delimiter: string, subObj: Any, subObjName: string, depth: number, full: boolean = false): string {

            if (UtilHelper.isNullObject(subObj)) {
                return UtilHelper.nullObjectDescription(subObjName) + ',' + delimiter
            }

            let strB = new StringBuilder
            subObj = subObj as object

            if (UtilHelper.isArray(subObj)) {
                strB.append(subObjName + ':' + delimiter + UtilHelper.arrayToString(delimiter + '  ', subObj as Array<Object>, depth, full))
            } else if (UtilHelper.isObject(subObj)) {
                strB.append(this.subObjectDescription(subObj, subObjName, this.isCircular(subObj), delimiter, depth))
            } else if (UtilHelper.isFunction(subObj)) {
                strB.append(UtilHelper.functionDescription(subObj, delimiter, true, full))
            } else {
                strB.append(UtilHelper.simpleObjectDescription(subObj, subObjName, true))
            }
            this.objects.add(subObj)

            strB.append(',' + delimiter)
            return strB.toString()
        }

        private objectDescription(obj: Object, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

            let strB = new StringBuilder('{ ')
            let isComplexObject: boolean = false

            for (const objEntry of Object.entries(obj)) {
                if (!objEntry) {
                    continue
                }
                isComplexObject = true
                strB.append(this.stringifySubObject(delimiter, objEntry[1], objEntry[0], depth, full))
            }

            if (!isComplexObject) {
                return JSON.stringify(obj)
            }

            return strB.toString().slice(0, - delimiter.length - 1) + ' }'
        }

        private subObjectDescription(obj: Object, objName: string, isCircular: boolean, delimiter: string, depth: number): string {
            if (isCircular) {
                return objName + ': [Circular]' + delimiter
            }
            return objName + ':' + delimiter + this.stringifyObject(obj, delimiter + '  ', depth + 1, true)
        }

        private objects = new Set<Object>

        private isCircular(obj: Any): boolean {
            if (obj == null) {
                return false
            }

            return this.objects.has(obj as object)
        }

        private static nullObjectDescription(objName: string = ''): string {
            return ((objName != '') ? objName + ': ' : '') + null
        }

        private static simpleObjectDescription(obj: Object, objName: string = '', isSub: boolean = false): string {
            let quotesStr = UtilHelper.isString(obj) ? '\'' : ''
            return (isSub ? objName + ': ' : '') + quotesStr + obj + quotesStr
        }

        private stringifyObject(obj: Any, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

            if (UtilHelper.isNullObject(obj)) {
                return UtilHelper.nullObjectDescription()
            }
            obj = obj as object
            this.objects.add(obj)
            let strB = new StringBuilder
            if (UtilHelper.isArray(obj)) {
                strB.append(UtilHelper.arrayToString(delimiter, obj as Array<Object>, depth, full))
            } else if (UtilHelper.isFunction(obj)) {
                strB.append(UtilHelper.functionDescription(obj, delimiter, false, full))
                if (!full) {
                    strB = (new StringBuilder).append('{ ').append(strB.toString()).append(' }')
                }
            } else if (UtilHelper.isObject(obj)) {
                strB.append(this.objectDescription(obj, delimiter, depth, full))
            } else {
                strB.append(UtilHelper.simpleObjectDescription(obj, ''))
            }
            return strB.toString()
        }

        private static arrayToString(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
            return ('[ ' + UtilHelper.arrayToStringImpl(delimiter, arr, depth, full) + (full ? ('[length]: ' + arr.length) : '') + ' ]')
        }

        private static functionDescription(obj: Object, delimiter: string = '\n', isSub: boolean = false, full: boolean = false, inArray: boolean = false): string {
            let strB = new StringBuilder
            let functionName = UtilHelper.getName(obj)
            strB.append((isSub && functionName !== '') ? (functionName + ':' + delimiter) : '');

            let baseFunctionStr = '[Function: ' + functionName + ']'

            if (full) {
                let end = delimiter + (inArray || isSub) ? '  ' : '';
                let length = UtilHelper.getLength(obj)
                strB.append('{ ' + baseFunctionStr + end +
                    '[length]: ' + (length ? length : 0) + ',' + end +
                    '[name] :\'' + functionName + '\',' + end +
                    '[prototype]: ' + functionName + ' { [constructor]: [Circular] } }')
            } else {
                strB.append(baseFunctionStr)
            }

            return strB.toString()
        }

        private static arrayToStringImpl(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
            if (!arr.length) {
                return ''
            }

            let arrayDelimiter: string = ', '

            for (let item of arr) {
                if (!UtilHelper.isNullObject(item) && (UtilHelper.isFunction(item) || UtilHelper.isObject(item) && depth <= 2)) {
                    arrayDelimiter += delimiter
                    break
                }
            }

            let strB = new StringBuilder
            for (let item of arr) {
                if (UtilHelper.isObject(item)) {
                    strB.append((new UtilHelper).stringifyObject(item, delimiter + '  ', depth + 1, full))
                } else if (UtilHelper.isFunction(item)) {
                    strB.append(UtilHelper.functionDescription(item, delimiter, false, full, true))
                } else {
                    strB.append(UtilHelper.simpleObjectDescription(item, full ? UtilHelper.getName(item) : ''))
                }
                strB.append(arrayDelimiter)
            }

            return strB.toString().slice(0, -arrayDelimiter.length)
        }

        private static getName(obj: Object): string {
            let subObj = UtilHelper.getValue("name", obj)
            return subObj ? subObj! as string : 'null'
        }

        private static getLength(obj: Object): int {
            let subObj = UtilHelper.getValue("length", obj)
            return subObj ? subObj! as int : 0
        }

        private static getValue(key: string, obj: Object): Any {
            if (!obj) {
                return null
            }
            for (const objEntry of Object.entries(obj)) {
                if (!objEntry) {
                    continue
                }
                if ((!objEntry[0]) && (objEntry[0] == key)) {
                    return objEntry[1]
                }
            }
            return null
        }

        /**
          * Remove html tag, css, js of html string to extract the contents
          * @param {String} html
          * @return {String} contents of the html
          */
        static extractContents(html: string): string {
            return html
                .replace(new RegExp("(\n|\r|\t)", "gm"), '') // remove linebreaks
                .replace(new RegExp("<(style|script|link|noscript).*?>.*?<\/(style|script|link|noscript)>", "g"), '') // remove css, js blocks
                .replace(new RegExp("<!--.*?-->", "g"), '') // remove comments
                .replace(new RegExp("<.*?>", "g"), '') // remove tags
                .replace(new RegExp("[\s\!\@\#\$\%\^\&\*\(\)\_\+\-\=\{\}\[\]\:\"\;\'\<\>\?\,\.\/\|\\\`\~]+", "g"), '') // remove Chars
                .trim()
        }

        static processArg(flag: string, arg: Object): string {
            // %c: CSS. This specifier is ignored and will skip any CSS passed in.
            if (flag == "c" && (arg instanceof String)) {
                return UtilHelper.extractContents(arg as string)
            } else if (flag == "s" && !(arg instanceof BigInt)) {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return arg as string
            } else if (flag == "d") {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + new Number(arg.toString())
            } else if (flag == "i") {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + parseInt(arg as string, 10)
            }
            else if (flag == "f") {
                if (arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + parseFloat(arg as string)
            } else if (flag == "j") {
                let res = ""
                try {
                    res = JSON.stringify(arg)
                }
                catch (error) {
                    if (typeof arg === "object") {
                        return UtilHelper.objectToString(arg)
                    } else {
                        throw new Error("error parse json")
                    }
                }

                return res
            } else if (flag == "o") {
                // Object. A string representation of an object
                // Similar to util.inspect() with options { showHidden: true, showProxy: true }
                return UtilHelper.objectToString(arg, true)
            } else if (flag == "O") {
                // Object. A string representation of an object
                // Similar to util.inspect() without options
                return UtilHelper.objectToString(arg)
            }

            return "%" + flag
        }

        static joinLastArgs(index: int, ...args: Object[]): string {
            let sb = new StringBuilder
            for (let i = index; i < args.length; ++i) {
                sb.append(" ")
                sb.append(args[i])
            }
            let res = sb.toString()
            return res
        }

    }  // class UtilHelper

    /**
     * Check the type of parameter.
     */
    export class types {

        /**
         * Check whether the entered value is of bigint64array array type.
         * @param { Object } value - A BigInt64Array value
         * @returns { boolean } Returns true if the value is a BigInt64Array instance.
         */
        isBigInt64Array(value: Object): boolean {
            return value instanceof BigInt64Array;
        }

        /**
         * Check whether the entered value is of biguint64array array array type.
         * @param { Object } value - A BigUint64Array value
         * @returns { boolean } Returns true if the value is a BigUint64Array instance.
         */
        isBigUint64Array(value: Object): boolean {
            return value instanceof BigUint64Array;
        }

        /**
         * Check whether the entered value is of arraybuffer type.
         * @param { Object } value - A ArrayBuffer value
         * @returns { boolean } Returns true if the value is a built-in ArrayBuffer instance.
         */
        isAnyArrayBuffer(value: Object): boolean {
            return this.isArrayBuffer(value);
        }

        /**
         * Check whether the type is included in the isAnyArrayBuffer.
         * @param { Object } value - A included in the isAnyArrayBuffer value
         * @returns { boolean } Returns true if the value is an instance of one of the ArrayBuffer views,
         * such as typed array objects or DataView.
         */
        isArrayBufferView(value: Object): boolean {
            return this.isDataView(value) || this.isTypedArray(value);
        }

        /**
         * Check whether the entered value is of arraybuffer type.
         * @param { Object } value - A arraybuffer value
         * @returns { boolean } Returns true if the value is a built-in ArrayBuffer instance. This does not include SharedArrayBuffer instances.
         */
        isArrayBuffer(value: Object): boolean {
            return value instanceof ArrayBuffer;
        }

        /**
         * Check whether the entered value is of DataView type.
         * @param { Object } value - A DataView value
         * @returns { boolean } Returns true if the value is a built-in DataView instance.
         */
        isDataView(value: Object): boolean {
            return value instanceof DataView;
        }

        /**
         * Check whether the entered value is of type date.
         * @param { Object } value - A Date value
         * @returns { boolean } Returns true if the value is a built-in Date instance.
         */
        isDate(value: Object): boolean {
            return value instanceof Date;
        }

        /**
         * Check whether the entered value is of float32array array type.
         * @param { Object } value - A Float32Array value
         * @returns { boolean } Returns true if the value is a built-in Float32Array instance.
         */
        isFloat32Array(value: Object): boolean {
            return value instanceof Float32Array;
        }

        /**
         * Check whether the entered value is of float64array array type.
         * @param { Object } value - A Float64Array value
         * @returns { boolean } Returns true if the value is a built-in Float64Array instance.
         */
        isFloat64Array(value: Object): boolean {
            return value instanceof Float64Array;
        }

        /**
         * Check whether the entered value is of int8array array type.
         * @param { Object } value - A Int8Array value
         * @returns { boolean } Returns true if the value is a built-in Int8Array instance.
         */
        isInt8Array(value: Object): boolean {
            return value instanceof Int8Array;
        }

        /**
         * Check whether the entered value is the int16array type.
         * @param { Object } value - A Int16Array value
         * @returns { boolean } Returns true if the value is a built-in Int16Array instance.
         */
        isInt16Array(value: Object): boolean {
            return value instanceof Int16Array;
        }

        /**
         * Check whether the entered value is the int32array array type.
         * @param { Object } value - A Int32Array value
         * @returns { boolean } Returns true if the value is a built-in Int32Array instance.
         */
        isInt32Array(value: Object): boolean {
            return value instanceof Int32Array;
        }

        /**
         * Check whether the entered value is of map type.
         * @param { Object } value - A Map value
         * @returns { boolean } Returns true if the value is a built-in Map instance.
         */
        isMap(value: Object): boolean {
            return value instanceof Map;
        }

        /**
         * Check whether the entered value is the iterator type of map.
         * @param { Object } value - A Map iterator value
         * @returns { boolean } Returns true if the value is an iterator returned for a built-in Map instance.
         */
        isMapIterator(value: Object): boolean {
            return value instanceof MapIterator;
        }

        /**
         * Check whether the value entered is of type error.
         * @param { Object } value - A Error value
         * @returns { boolean } Returns true if the value is an instance of a built-in Error type.
         */
        isNativeError(value: Object): boolean {
            return value instanceof Error;
        }

        /**
         * Check whether the entered value is of promise type.
         * @param { Object } value - A Promise value
         * @returns { boolean } Returns true if the value is a built-in Promise.
         */
        isPromise(value: Object): boolean {
            return value instanceof Promise;
        }

        /**
         * Check whether the entered value is of type regexp.
         * @param { Object } value - A regular expression object value
         * @returns { boolean } Returns true if the value is a regular expression object.
         */
        isRegExp(value: Object): boolean {
            return value instanceof RegExp;
        }

        /**
         * Check whether the entered value is of type set.
         * @param { Object } value - A Set instance value
         * @returns { boolean } Returns true if the value is a built-in Set instance.
         */
        isSet(value: Object): boolean {
            return value instanceof Set;
        }

        /**
         * Check whether the entered value is the iterator type of set.
         * @param { Object } value - A Set iterator value
         * @returns { boolean } Returns true if the value is an iterator returned for a built-in Set instance.
         *
         * Since the current interface cannot be correctly determined, it returns false by default
         */
        isSetIterator(value: Object): boolean {
            return value instanceof SetIterator
        }

        /**
         * Check whether the entered value is a type contained in typedarray.
         * @param { Object } value - A TypedArray instance value
         * @returns { boolean } Returns true if the value is a built-in TypedArray instance.
         */
        isTypedArray(value: Object): boolean {
            return this.isInt8Array(value) ||
                this.isUint8Array(value) ||
                this.isUint8ClampedArray(value) ||
                this.isInt16Array(value) ||
                this.isUint16Array(value) ||
                this.isInt32Array(value) ||
                this.isUint32Array(value) ||
                this.isFloat32Array(value) ||
                this.isFloat64Array(value) ||
                this.isBigInt64Array(value) ||
                this.isBigUint64Array(value);
        }

        /**
         * Check whether the entered value is the uint8array array type.
         * @param { Object } value - A Uint8Array value
         * @returns { boolean } Returns true if the value is a built-in Uint8Array instance.
         */
        isUint8Array(value: Object): boolean {
            return value instanceof Uint8Array;
        }

        /**
         * Check whether the entered value is the uint8clapedarray array type.
         * @param { Object } value - A Uint8ClampedArray value
         * @returns { boolean } Returns true if the value is a built-in Uint8ClampedArray instance.
         */
        isUint8ClampedArray(value: Object): boolean {
            return value instanceof Uint8ClampedArray;
        }

        /**
         * Check whether the entered value is the uint16array array array type.
         * @param { Object } value - A Uint16Array value
         * @returns { boolean } Returns true if the value is a built-in Uint16Array instance.
         */
        isUint16Array(value: Object): boolean {
            return value instanceof Uint16Array;
        }

        /**
         * Check whether the entered value is the uint32array array type.
         * @param { Object } value - A Uint32Array value
         * @returns { boolean } Returns true if the value is a built-in Uint32Array instance.
         */
        isUint32Array(value: Object): boolean {
            return value instanceof Uint32Array;
        }

        /**
         * Check whether the entered value is of type weakmap.
         * @param { Object } value - A WeakMap value
         * @returns { boolean } Returns true if the value is a built-in WeakMap instance.
         */
        isWeakMap(value: Object): boolean {
            return value instanceof WeakMap;
        }

        /**
         * Check whether the entered value is of type weakset.
         * @param { Object } value - A WeakSet value
         * @returns { boolean } Returns true if the value is a built-in WeakSet instance.
         */
        isWeakSet(value: Object): boolean {
            return value instanceof WeakSet;
        }

        /**
         * Check whether the entered value is of type asyncfunction.
         * @param { Object } value - A AsyncFunction value
         * @returns { boolean } Returns true if the value is a built-in AsyncFunction instance.
         *
         * Since the current interface cannot be correctly determined, it returns false by default
         */
        isAsyncFunction(value: Object): boolean {
            return false;
        }
    }

    export function promisify(original: Function): PromisifiedFunc {
        return (...args: FixedArray<Any>): Promise<Any> => {
            return new Promise<Any>((resolve, reject) => {
                let callback: Function = (err: Error | null, ...value: FixedArray<Any>) => {
                    if (err) {
                        reject(err);
                    } else {
                        resolve(value);
                    }
                };
                const newArgs: FixedArray<Any> = new FixedArray<Any>(args.length + 1);
                for (let i = 0; i < args.length; i++) {
                    newArgs[i] = args[i];
                }
                newArgs[args.length] = callback;
                original.unsafeCall(...newArgs);
            });
        };
    }

    export function callbackWrapper(original: Function): Function {
        return (...args: FixedArray<string | number | Function>): void => {
            if (typeof args[args.length - 1] != 'function') {
                throw new Error('maybe is not function');
            }
            const func: Function = args[args.length - 1] as Function;
            const newArgs: FixedArray<Any> = new FixedArray<Any>(args.length - 1);
            for (let i = 0; i < args.length - 1; i++) {
                newArgs[i] = args[i];
            }
            const funcReturn = original.unsafeCall(...newArgs);
            if (funcReturn instanceof Promise) {
                const func1 = funcReturn as Promise<Any>
                func1.then((result: Any) => {
                    func.unsafeCall(null, result);
                }).catch((err: Error) => {
                    func.unsafeCall(err, null);
                });
            } else {
                throw new Error('The function passed to callbackWrapper must return a Promise.');
            }
        }
    }

    /**
     * Get the hash code of an object.
     * @param { RecordData } [obj] - The object that need to get hash code.
     * @returns { long } Return a hash code of an object.
     */
    export function getHash(obj: RecordData): long {
        if (obj === undefined || obj === null) {
            throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of Parameter must be object.`);
        }
        let result: long = Runtime.getHashCodeByValue(obj!);
        return result;
    }

    /**
    * Generates a random binary UUID.
    * @param { boolean } entropyCache - Whether to use cached UUIDs for generation
    * @returns { Uint8Array | undefined } A Uint8Array containing the generated binary UUID, or undefined on failure.
    */
    export function generateRandomBinaryUUID(entropyCache?: boolean): Uint8Array | undefined {
        let arr = UtilHelper.generateRandomBinaryUUID((entropyCache == undefined) ? false : entropyCache)
        return arr;
    }

    /**
    * Parses a UUID string into a binary representation (Uint8Array).
    * @param { string } uuid - The UUID string to parse
    * @returns { Uint8Array } A Uint8Array containing the 16-byte binary representation of the UUID.
    */
    export function parseUUID(uuid: string): Uint8Array {
        const format = new RegExp("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
        if (!format.test(uuid)) {
            throw createBusinessError(syntaxErrorCode, `Syntax Error.Invalid ${uuid} string`);
        }
        const hex = uuid.replace(new RegExp('-', 'g'), '');
        const uuidArray = new Uint8Array(16);
        for (let i = 0; i < 16; i++) {
            uuidArray[i] = parseInt(hex.substr(i * 2, 2), 16);
        }
        return uuidArray;
    }

    /**
    * Obtain detailed information about the system error code.
    * @param { int } errno - Error codes generated by system errors
    * @returns { string } Details about the error code.
    */
    export function errnoToString(errno: int): string {
        let result = UtilHelper.getErrorString(errno);
        if (result == undefined) {
            result = `Unknown system error ${errno}`;
        }
        return result;
    }
}
