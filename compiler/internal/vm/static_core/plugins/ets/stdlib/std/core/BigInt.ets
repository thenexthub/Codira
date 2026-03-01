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

/**
 * `bigint` is an alias for type `BigInt`
 */
export type bigint = BigInt

type ToStringHandlerMap = Record<int, (instance: BigInt) => string>

// initialized in _initializerBlock_.ets
const MAX_REGULAR_INTEGER: BigInt;
// NOTE(@aendrewpolyakov, #19830): Rewrite using object literal of record type
// initialized in _initializerBlock_.ets
const FAST_TO_STRING_HANDLER_MAP: ToStringHandlerMap;

export interface BigIntToLocaleStringOptions {
    compactDisplay?: string;
    currency?: string;
    currencyDisplay?: string;
    localeMatcher?: string;
    maximumFractionDigits?: number;
    maximumSignificantDigits?: number;
    minimumFractionDigits?: number;
    minimumIntegerDigits?: number;
    minimumSignificantDigits?: number;
    notation?: string;
    numberingSystem?: string;
    style?: string;
    unit?: string;
    unitDisplay?: string;
    useGrouping?: boolean;
}

/**
 * BigInt class stub
 */
export final class BigInt {

    private readonly bytes: FixedArray<int>
    private readonly sign: int

    private static readonly shiftSys: long = 32 as long
    private static readonly baseSys: long = (1 as long) << BigInt.shiftSys
    private static readonly maskSys: long = BigInt.baseSys - (1 as long)
    private static readonly INVALID_RADIX_MESSAGE: string = "toString() radix argument must be between 2 and 36"
    private static readonly BITS_PER_1024_DEC = 3402
    private static readonly BIGINT64_MASK = new BigInt("0xffffffffffffffff")

    private static resetBytes(b: FixedArray<int>):void {
        for (let i = 0; i<b.length; i++) {
            b[i] = 0
        }
    }

    private static fromLongHelper(val: long, bitLen: int, sign: int): FixedArray<int> {
        // get required array of Ints' size to store whole value
        // split by suported mode (baseSys)
        let size: int = ((bitLen + BigInt.shiftSys - 1) / BigInt.shiftSys).toInt()

        let digits: FixedArray<int> = new FixedArray<int>(size)
        for (let i = 0; i < size; i++) {
            let d: long = val & BigInt.maskSys
            val = val >> BigInt.shiftSys
            digits[i] = d.toInt()
        }

        if (sign < 0) {
            digits = BigInt.to2sComplement(digits)
        }

        // remove leading zeroes
        return BigInt.stripLeadingZeros(digits)
    }

    constructor() {
        this([], 0)
    }

    constructor(d: byte) {
        this.sign = d < 0 ? -1 : (d == 0 ? 0 : 1)
        this.bytes = this.sign == 0 ? [] as FixedArray<int> : BigInt.fromLongHelper(d.toLong(), 8, this.sign)
    }

    constructor(d: short) {
        this.sign = d < 0 ? -1 : (d == 0 ? 0 : 1)
        this.bytes = this.sign == 0 ? [] as FixedArray<int> : BigInt.fromLongHelper(d.toLong(), 16, this.sign)
    }

    constructor(d: int) {
        this.sign = d < 0 ? -1 : (d == 0 ? 0 : 1)
        this.bytes = this.sign == 0 ? [] as FixedArray<int> : BigInt.fromLongHelper(d.toLong(), 32, this.sign)
    }

    constructor(d: long) {
        this.sign = d < 0 ? -1 : (d == 0 ? 0 : 1)
        this.bytes = this.sign == 0 ? [] as FixedArray<int> : BigInt.fromLongHelper(d, 64, this.sign)
    }

    constructor(d: double) {
        // should we cast it to long at then use internal helper?
        // i.e. like this: this.bytes = BigInt.fromLongHelper(d as long, 64)
        if (!d.isInteger()) {
            throw new RangeError(`the Double value ${d} cannot be converted to a BigInt because it is not an integer`)
        }
        if (!d.isSafeInteger()) {
            let a = BigInt.fromDoubleInteger(d);
            this.bytes = a.bytes;
            this.sign = a.sign;
            return
        }
        this.sign = d < 0 ? -1 : (d == 0 ? 0 : 1)
        this.bytes = this.sign == 0 ? [] as FixedArray<int> : BigInt.fromLongHelper(d.toLong(), 64, this.sign)
    }

    constructor(d: string) {
        let trimmed = d.trim()
        let a = BigInt.parseFromString(trimmed)
        this.bytes = a[0] as FixedArray<int>
        this.sign = a[1] as int
    }

    constructor(d: boolean) {
        const oz = d ? 1 : 0
        this.sign = oz
        this.bytes = this.sign == 0
            ? [] as FixedArray<int>
            : BigInt.fromLongHelper(oz.toLong(), 1, this.sign)
    }

    constructor(d: BigInt) {
        this(d.bytes, d.sign)
    }

    constructor(v: FixedArray<int>, sign: int) {
        this.bytes = BigInt.stripLeadingZeros(v)
        this.sign = this.bytes.length == 0 ? 0 : sign
    }

    public static fromULong(val: long): BigInt {
        let ret = val == 0 ? new BigInt([], 0) : new BigInt(BigInt.fromLongHelper(val, 64, 1), 1)
        return ret
    }

    public equals(to: BigInt): boolean {
        if (this.bytes.length != to.bytes.length) {
            return false
        }
        if (this.sign != to.sign) {
            return false
        }
        for (let i = 0; i < this.bytes.length; i++) {
            if (this.bytes[i] != to.bytes[i]) {
                return false
            }
        }
        return true
    }

    equals(other: Any): boolean {
        if (!(other instanceof BigInt)) {
            return false
        }

        return this.equals(other as BigInt)
    }

    /**
     * Converts a number to a string by using current locale
     *
     * @returns string representation
     */
    public toLocaleString(): string {
        return this.toLocaleString(undefined)
    }

    /**
     * Converts a number to a string by using the current or specified locale.
     *
     * @param locales
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales?: string | Intl.Locale | (string | Intl.Locale)[], options?: BigIntToLocaleStringOptions): string {
        const numFmtOptions = BigInt.toNumberFormatOptions(options)
        const formatter = new Intl.NumberFormat(Intl.intlLocalesToLanguageTags(locales), numFmtOptions)
        return formatter.format(this)
    }

    private static toNumberFormatOptions(options?: BigIntToLocaleStringOptions): Intl.NumberFormatOptions | undefined {
        if (!options) {
            return undefined
        }

        const numFmtOptions: Intl.NumberFormatOptions = {}

        if (options.compactDisplay) {
            if (!(options.compactDisplay == "short" || options.compactDisplay == "long")) {
                throw new RangeError(`invalid value "${options.compactDisplay}" for option compactDisplay`)
            }
            numFmtOptions.compactDisplay = options.compactDisplay as "short" | "long"
        }

        numFmtOptions.currency = options.currency

        if (options.currencyDisplay) {
            const currencyDisplay = options.currencyDisplay
            if (!(currencyDisplay == "code" || currencyDisplay == "symbol" || currencyDisplay == "narrowSymbol" || currencyDisplay == "name")) {
                throw new RangeError(`invalid value "${currencyDisplay}" for option currencyDisplay`)
            }
            numFmtOptions.currencyDisplay = currencyDisplay as "code" | "symbol" | "narrowSymbol" | "name"
        }

        if (options.localeMatcher) {
            const localeMatcher = options.localeMatcher
            if (!(localeMatcher == "lookup" || localeMatcher == "best fit")) {
                throw new RangeError(`invalid value "${localeMatcher}" for option localeMatcher`)
            }
            numFmtOptions.localeMatcher = localeMatcher as "lookup" | "best fit"
        }

        numFmtOptions.maximumFractionDigits = options.maximumFractionDigits
        numFmtOptions.maximumSignificantDigits = options.maximumSignificantDigits
        numFmtOptions.minimumFractionDigits = options.minimumFractionDigits
        numFmtOptions.minimumIntegerDigits = options.minimumIntegerDigits
        numFmtOptions.minimumSignificantDigits = options.minimumSignificantDigits

        if (options.notation) {
            const notation = options.notation
            if (!(notation == "standard" || notation == "scientific" || notation == "engineering" || notation == "compact")) {
                throw new RangeError(`invalid value "${notation}" for option notation`)
            }
            numFmtOptions.notation = notation as "standard" | "scientific" | "engineering" | "compact"
        }

        // numFmtOptions.numberingSystem = options.numberingSystem

        if (options.style) {
            const style = options.style
            if (!(style == "decimal" || style == "percent" || style == "currency" || style == "unit")) {
                throw new RangeError(`invalid value "${style}" for option style`)
            }
            numFmtOptions.style = style as "decimal" | "percent" | "currency" | "unit"
        }

        numFmtOptions.unit = options.unit

        if (options.unitDisplay) {
            const unitDisplay = options.unitDisplay
            if (unitDisplay == "short" || unitDisplay == "long" || unitDisplay == "narrow") {
                throw new RangeError(`invalid value "${unitDisplay}" for option unitDisplay`)
            }
            numFmtOptions.unitDisplay = unitDisplay as "short" | "long" | "narrow"
        }

        numFmtOptions.useGrouping = options.useGrouping

        return numFmtOptions
    }

    public override toString(): string {
        return this.toString(10)
    }

    public toString(radix: int): string {
        if (radix < 2 || radix > 36) {
            throw new RangeError(BigInt.INVALID_RADIX_MESSAGE)
        }
        if (this.isZero()) {
            return "0"
        }
        let fastToStringHandler = FAST_TO_STRING_HANDLER_MAP[radix]
        // NOTE(@aendrewpolyakov, #19643): Rewrite concat() with '+' operator once it got fixed for string operands
        return (this.negative() ? "-" : "").concat((fastToStringHandler != undefined) ? fastToStringHandler!(this) : this.toStringAsGenericRadix(radix))
    }

    public toString(radix?: double): string {
        if (radix == undefined) {
            return this.toString(10)
        }
        if (radix == -Infinity || radix == +Infinity || radix == NaN) {
            throw new RangeError(BigInt.INVALID_RADIX_MESSAGE)
        }
        return this.toString(radix.toInt())
    }

    public toStringAsBinary(): string {
        let reversedInts = BigInt.reverse(this.bytes)
        let intToBinaryString = (value: Long) => { return value.toString(2) }
        const str = (i: int) => { return intToBinaryString((reversedInts[i]!).toLong() & BigInt.maskSys) }
        let result = new StringBuilder(str(0))
        for (let i = 1; i < reversedInts.length; ++i) {
            result = result.append(str(i).padStart(BigInt.shiftSys.toInt(), '0'))
        }
        return result.toString()
    }

    public toStringAsDecimal(): string {
        let digits = this.bytesToDigits(this.bytes)
        let result = new StringBuilder
        for (let i = 0; i < digits.length; i++) {
            result = result.append(digits[i])
        }
        return result.toString()
    }

    private toStringAsGenericRadix(radix: int): string {
        const radixBigInt = new BigInt(radix)
        let result: string = ""
        let dividend = this.abs()
        while (dividend > 0n) {
            if (dividend < MAX_REGULAR_INTEGER) {
                result = StringBuilder.concatStrings(Long.toDouble(dividend.getLong()).toString(radix), result)
                break
            }
            // NOTE(@aendrewpolyakov, #18313): Rewrite once destructuring assignment got support
            let divisionResult = dividend.operatorDivideInternal(radixBigInt)
            let quotent = divisionResult[0], remainder = divisionResult[1]
            result = StringBuilder.concatStrings(Long.toDouble(remainder.getLong()).toString(radix), result)
            dividend = quotent
        }
        return result.toString()
    }

    public toStringAsPowerOfTwoRadix(radix: int): string {
        let chunkSize: int = Math.ceil(Math.log2(radix)).toInt()
        if (radix < 2) {
            throw new Error("BigInt::toStringAsPowerOfTwoRadix(): invalid radix: " + radix)
        }
        let result = new StringBuilder
        let bitChunks = this.sliceToBinaryChunks(chunkSize)
        for (let bitChunk of bitChunks) {
            result = result.append(parseInt(bitChunk!, 2).toString(radix))
        }
        return result.toString()
    }

    private sliceToBinaryChunks(chunkSize: int): FixedArray<string> {
        let binaryRepresentation: string = this.toStringAsBinary()
        if (binaryRepresentation.length % chunkSize != 0) {
            const requiredLength: int = binaryRepresentation.length + (chunkSize - binaryRepresentation.length % chunkSize)
            binaryRepresentation = binaryRepresentation.padStart(requiredLength, '0')
        }
        let chunksCount: int = (binaryRepresentation.length / chunkSize).toInt()
        let result: FixedArray<string> = new FixedArray<string>(chunksCount)
        for (let index: int = 0, offset: int = 0; index < chunksCount; ++index, offset += chunkSize) {
            result[index] = binaryRepresentation.substring(offset, offset + chunkSize)
        }
        return result
    }

    public valueOf(): BigInt {
        return this
    }

    public static asIntN(bits: long, num: BigInt): BigInt {
        if (bits == 0 || num.isZero()) {
            return 0n
        }
        if (bits < 0 || bits > Double.MAX_SAFE_INTEGER) {
            throw new RangeError('bits < 0 or bits > Double.MAX_SAFE_INTEGER')
        }
        if (bits > num.length() * BigInt.shiftSys) {
            return num
        }
        // operatorLeftShift temporary has limitations to 255 on rhs
        // in this case must be bits <= 255
        // please check `operatorLeftShift` implementation

        // 2^(bits-1)
        const half = 1n << new BigInt(bits-1)
        // 2^(bits)-1 = 2 * half - 1
        let mask = 2n * half - 1n
        const masked = num & mask
        // maxRepresentable = 2^(bits) = mask + 1
        // if masked > 2^(bits-1) : masked - maxRepresentableValue : masked
        return masked >= half ? masked - (mask + 1n) : masked
    }

    public static asIntN(bits: Number, num: BigInt): BigInt {
        return BigInt.asIntN(bits.toLong(), num)
    }

    public static asUintN(bits: long, num: BigInt): BigInt {
        if (bits == 0 || num.isZero()) {
            return 0n
        }
        if (bits < 0 || bits > Double.MAX_SAFE_INTEGER) {
            throw new RangeError('bits < 0 or bits > Double.MAX_SAFE_INTEGER')
        }
        if (bits >= num.length() * BigInt.shiftSys && num.positive()) {
            return num
        }
        // operatorLeftShift temporary has limitations to 255 on rhs
        // in this case must be bits <= 255
        // please check `operatorLeftShift` implementation
        return num & ((1n << (new BigInt(bits))) - 1n)
    }

    public static asUintN(bits: Number, num: BigInt): BigInt {
        return BigInt.asUintN(bits.toLong(), num)
    }

    public negative(): boolean {
        return this.sign < 0
    }

    public positive(): boolean {
        return this.sign >= 0
    }

    private getSign(): boolean {
        return this.positive()
    }

    private length(): int {
        return this.bytes.length as int
    }

    private isZero(): boolean {
         return this.sign == 0
    }

    private clone(): BigInt {
        return new BigInt(this.asBytes(), this.sign)
    }

    // remove leading 0x0
    private static stripLeadingZeros(d: FixedArray<int>): FixedArray<int> {
        let i = d.length - 1

        if (i < 0 || d[i] != 0) {
           return d
        }

        for (; i >= 0 && d[i] == 0; --i) {}

        let result: FixedArray<int> = new FixedArray<int>(i+1)

        for (; i >= 0; --i) {
            result[i] = d[i]
        }

        return result
    }

    public operatorAdd(other: BigInt): BigInt {
        let max_len = max(this.length(), other.length()) as int + 1
        let lhs: FixedArray<int> = this.bytes
        let rhs: FixedArray<int> = other.bytes

        if (this.negative() != other.negative()) {
           if (this.negative()) {
              lhs = this.as2sComplementBytes(max_len - this.length())
           }
           else {
              rhs = other.as2sComplementBytes(max_len - other.length())
           }
        }

        let carry: long = 0
        let sum: long = 0
        let res: FixedArray<int> = new FixedArray<int>(max_len)
        for (let i = 0; i < max_len; i++) {
            sum = carry
            carry = 0
            if (i < lhs.length) {
                sum += lhs[i].toLong() & BigInt.maskSys
            }
            if (i < rhs.length) {
                sum += rhs[i].toLong() & BigInt.maskSys
            }
            carry = sum >> BigInt.shiftSys
            res[i] = (sum & BigInt.maskSys).toInt()
        }

        let sign: int = 1

        if (this.negative() != other.negative()) {
           if (res[res.length - 1] == -1) {
              res = BigInt.to2sComplement(res)
              sign = -1
           }
        } else {
           if (this.negative()) {
              sign = -1
           }
        }

        return new BigInt(res, sign)
    }

    public operatorSubtract(other: BigInt): BigInt {
        let rhs = other.negate()
        return this.operatorAdd(rhs)
    }

    public operatorMultiply(other: BigInt): BigInt {
        if (this.isZero() || other.isZero()) {
            return new BigInt()
        }

        let result = new BigInt(0)
        let lhs = this
        let rhs = other

        // we're little endian, means digits at our vector
        // are stored as little-endian manner
        let product: FixedArray<int> = new FixedArray<int>(lhs.bytes.length + rhs.bytes.length + 1)
        for (let i = 0; i < lhs.bytes.length; i++) {
            BigInt.resetBytes(product)
            let div: long = 0
            let l: long = lhs.bytes[i].toLong() & BigInt.maskSys
            for (let j = 0; j < rhs.bytes.length; j++) {
                let r: long = rhs.bytes[j].toLong() & BigInt.maskSys
                let mul: long = l * r + div

                product[i + j] = (mul & BigInt.maskSys).toInt()

                div = mul >>> BigInt.shiftSys
            }

            if (div > 0) {
                product[i + rhs.bytes.length] = div.toInt()
            }

            result = result + new BigInt(product, 1)
        }

        if (this.negative() != other.negative()) {
            result = result.negate()
        }

        return result
    }

    private static getCnt(from: long, to:long): long {
        return (from + ((to - from)/(2 as long)))
    }

    private getNearestDivider(divident: BigInt, divisor: BigInt): [Long, BigInt] {
        let sub = divident - divisor
        if (sub.negative()) {
            // i.e. divisor is greater than divident, so the latter is a remainder
            return [0, divident]
        }
        if (divident < MAX_REGULAR_INTEGER) {
            let dividentLong: long = divident.getLong()
            let divisorLong: long = divisor.getLong()
            let quotent: long = dividentLong / divisorLong
            let remainder: long = dividentLong % divisorLong
            return [quotent, new BigInt(remainder)]
        }
        let from: long = 1
        let to: long = BigInt.baseSys - (1 as long)
        let cnt: long = BigInt.getCnt(from, to)
        let res2: BigInt

        let done = false
        do {
            let res1 = divident - ((new BigInt(cnt)) * divisor)
            let b1: boolean = res1.negative()
            if (res1.isZero())
            {
                return [cnt, res1]
            }
            res2 = divident - ((new BigInt(cnt - 1)) * divisor)
            let b2: boolean = res2.positive()
            if (res2.isZero())
            {
                cnt--
                done = true
                return [cnt, res2]
            }

            if (b1 && b2)
            {
                cnt--
                done = true
            }
            if (!b1) {
                from = cnt
                cnt = BigInt.getCnt(from, to)

            }
            if (!b2) {
                to = cnt
                cnt = BigInt.getCnt(from, to)
            }
        } while (!done)

        return [cnt, res2]
    }

    private doDivide(other: BigInt): [FixedArray<int>, FixedArray<int>] {
        let divisor = other.negative() ? other.negate() : other

        let bytes: FixedArray<int> = new FixedArray<int>(divisor.bytes.length)
        let idx = this.bytes.length - 1
        let res: FixedArray<int> = new FixedArray<int>(this.bytes.length)
        for (let i=0; i < divisor.bytes.length && idx >= 0; i++, idx--) {
            bytes[divisor.bytes.length - i - 1] = this.bytes[idx]
        }

        let div = new BigInt(bytes, 1)

        let position = 0

        do {
            let ret: [Long, BigInt]
            ret = this.getNearestDivider(div, divisor)
            res[position++] = ret[0].toInt()
            if (idx >= 0) {
                let size = (ret[1] as BigInt).bytes.length + 1
                bytes = new FixedArray<int>(size)
                bytes[0] = this.bytes[idx]
                for (let i=0; i < (ret[1] as BigInt).bytes.length; i++) {
                    bytes[i + 1] = (ret[1] as BigInt).bytes[i]
                }
                div = new BigInt(bytes, 1)
            } else {
               div = ret[1] as BigInt
            }
        } while(idx-- >= 0)

        let result: FixedArray<int> = new FixedArray<int>(position)
        for (let i = 0; i < position; i++) {
           result[i] = res[position - i - 1]
        }

        return [result, div.bytes]
    }

    private operatorDivideInternal(other: BigInt): [BigInt, BigInt] {
        if (other.isZero()) {
            throw new Error("BigInt: division by zero")
        }

        if (this.isZero()) {
            return [new BigInt(0), new BigInt(0)]
        }

        let div_res = this.doDivide(other)
        let quot: FixedArray<int> = div_res[0]
        let rem: FixedArray<int> = div_res[1]

        let quotent: BigInt = new BigInt(quot, this.negative() != other.negative() ? -1 : 1)
        let remainder: BigInt = new BigInt(rem, this.negative() ? -1 : 1)

        return [quotent, remainder]
    }

    public operatorDivide(other: BigInt): BigInt {
        return (this.operatorDivideInternal(other))[0]
    }

    public operatorModule(other: BigInt): BigInt {
        return (this.operatorDivideInternal(other))[1]
    }

    private bitwiseHelper(other:BigInt, fn:(a:int, b:int)=>int): BigInt {
        let max_len = max(this.length(), other.length())
        let lhs: FixedArray<int> = this.as2sComplementBytes(max_len - this.length())
        let rhs: FixedArray<int> = other.as2sComplementBytes(max_len - other.length())

        let res: FixedArray<int> = new FixedArray<int>(max_len + 1)

        for (let i = 0; i <= max_len; i++) {
            res[i] = fn(lhs[i], rhs[i])
        }

        let sign: int = 1

        if (res[res.length - 1] == -1) {
            sign = -1
            res = BigInt.to2sComplement(res)
        }

        return new BigInt(res, sign)
    }

    public operatorBitwiseOr(other: BigInt): BigInt {
        return this.bitwiseHelper(other, (a:int, b:int) => {return (a | b)})
    }

    public operatorBitwiseAnd(other: BigInt): BigInt {
        return this.bitwiseHelper(other, (a:int, b:int) => {return (a & b)})
    }

    public operatorBitwiseXor(other: BigInt): BigInt {
        return this.bitwiseHelper(other, (a:int, b:int) => {return (a ^ b)})
    }

    public operatorGreaterThan(other: BigInt): boolean {
        if (this.positive() && other.negative()) {
            return true
        }

        if (this.negative() && other.positive()) {
            return false
        }

        let sub = this.operatorSubtract(other)

        if (sub.negative() || sub.isZero()) {
            return false
        }

        return true
    }

    public operatorLessThan(other: BigInt): boolean {
        return other > this
    }

    public operatorGreaterThanEqual(other: BigInt): boolean {
        return !(other > this)
    }

    public operatorLessThanEqual(other: BigInt): boolean {
        return !(other < this)
    }

    public operatorLeftShift(other: BigInt): BigInt {
        if (other.negative()) {
            return this.operatorRightShift(other.negate())
        }

        // we can't shift more than container can hold
        let max_shift = new BigInt(BigInt.baseSys - 1)
        if (other.abs() > max_shift) {
            throw new Error("BigInt: too many digits in integer")
        }

        let lhs = this
        if (other.isZero()) {
            return lhs
        }

        let limit:long = other.getLong()

        let wordSize:int = 32

        let wordShift: int = ((limit / wordSize).toInt());
        let bitShift: int = ((limit % wordSize).toInt());

        let oldLength: int = this.bytes.length;
        let newLength: int = oldLength + wordShift + (bitShift > 0 ? 1 : 0);
        let newBytes: FixedArray<int> = new FixedArray<int>(newLength)

        if (bitShift > 0) {
            let carry: int = 0;
            let rightShift = wordSize - bitShift;
            for (let i: int = 0; i < oldLength; i++) {
                let currentWord: int = this.bytes[i];
                let newWord: int = (((currentWord << bitShift) | carry) & 0xFFFFFFFF).toInt();
                carry = (currentWord >>> rightShift);
                newBytes[i + wordShift] = newWord;
            }
            if (carry > 0) {
                newBytes[newLength - 1] = carry;
            }
        } else {
            copyTo(this.bytes, newBytes, wordShift, 0, oldLength);
        }

        return new BigInt(newBytes, this.sign);

    }

    public operatorRightShift(other: BigInt): BigInt {
        if (other.negative()) {
            return this.operatorLeftShift(other.negate())
        }

        if (other.isZero()) {
            return this
        }

        let lhs = this.as2sComplementBytes()

        // consider shifting limits, as we can not shift more
        // than stored at the BigInt object
        let bit: long = (1 << (BigInt.shiftSys - 1))
        // it is equal to  len * 8 or len * log2(baseSys)
        let limit: long = this.length() * BigInt.shiftSys
        let lval: long = other.getLong()
        limit = limit >= lval ? lval : limit

        let last: long = 0x0
        for (let i: long = 0; i < limit; i++) {

            let first: long = this.negative() ? 1 : 0
            for (let j: int = (lhs.length as int) - 1; j>=0; j--) {

                let a: long = lhs[j].toLong() & BigInt.maskSys
                last = a & 0x1
                a = (a >> 1) & BigInt.maskSys
                if (first) {
                    a = a | bit
                }

                lhs[j] = (a & BigInt.maskSys).toInt()
                first = last
            }
        }

        let sign = 1

        if (lhs[lhs.length - 1] == -1) {
            lhs = BigInt.to2sComplement(lhs)
            sign = -1
        }

        return new BigInt(lhs, sign)
    }

    public operatorIncrement(): BigInt {
        return new BigInt("1") + this
    }

    public operatorDecrement(): BigInt {
        return this - new BigInt("1")
    }

    private static invertBytes(bytes: FixedArray<int>): void{
        for (let i = 0; i < bytes.length; i++) {
            bytes[i] = (~bytes[i]) & BigInt.maskSys.toInt()
        }
    }

    public operatorBitwiseNot(): BigInt {
        let len = this.length() + 1
        let res: FixedArray<int> = new FixedArray<int>(len)

        for (let i = 0; i<len; i++) {
            res[i] = i < this.length() ? this.bytes[i] : 0
        }

        if (this.negative()) {
           res = BigInt.to2sComplement(res)
        }

        BigInt.invertBytes(res)

        let sign = 1

        if (res[res.length - 1] == -1) {
           res = BigInt.to2sComplement(res)
           sign = -1
        }

        return new BigInt(res, sign)
    }

    public pow(exponent: BigInt): BigInt {
        if (exponent.negative()) {
            throw new RangeError("Exponent must be non-negative")
        }
        if (exponent.isZero()) {
            return 1n
        }
        if (this.isZero()) {
            return 0n
        }

        const one = 1n
        if (exponent === one) {
            return new BigInt(this)
        }

        let trailingZeros = 0n
        let temp = new BigInt(exponent)
        while((temp & one).isZero()) {
            trailingZeros++
            temp = temp >> one
        }

        const shiftedExp = exponent >> trailingZeros

        let result: BigInt = 1n
        let base: BigInt = this
        let exp: BigInt = shiftedExp

        while (!exp.isZero()) {
            if ((exp & one) === one) {
                result = result * base
                if (exp === one) {
                    break;
                }
            }
            base = base * base
            exp = exp >> one
        }

        for (let i = 0; i < trailingZeros; i++) {
            result = result * result
        }

        return result
    }

    public negate(): BigInt {
        return new BigInt(this.bytes, this.sign * -1)
    }

    private abs(): BigInt {
        return new BigInt(this.bytes, 1)
    }

    private asBytes(): FixedArray<int> {
        let bytes: FixedArray<int> = new FixedArray<int>(this.length())
        for (let i = 0; i < this.length(); i++) {
            bytes[i] = this.bytes[i]
        }

        return bytes
    }

    private as2sComplementBytes(): FixedArray<int> {
        return this.as2sComplementBytes(0)
    }

    private as2sComplementBytes(pad: int): FixedArray<int> {
        let size = this.length() + pad + 1
        let bytes: FixedArray<int> = new FixedArray<int>(size)

        for (let i = 0; i < size; i++) {
            bytes[i] = i<this.length() ? this.bytes[i] : 0
        }

        return this.negative() ? BigInt.to2sComplement(bytes) : bytes
    }

    private bytesToDigits(input: FixedArray<int>): FixedArray<int> {

        let rbytes = BigInt.reverse(input)
        let digits: FixedArray<int> = []
        let mask: FixedArray<int> = []
        if (BigInt.shiftSys == 8) {
            mask = [2, 5, 6]
        } else if (BigInt.shiftSys == 16) {
            mask = [6, 5, 5, 3, 6]
        } else if (BigInt.shiftSys == 24){
            mask = [1, 6, 7, 7, 7, 2, 1, 6]
        } else if (BigInt.shiftSys == 30) {
            mask = [1, 0, 7, 3, 7, 4, 1, 8, 2, 4]
        } else if (BigInt.shiftSys == 32) {
            mask = [4, 2, 9, 4, 9, 6, 7, 2, 9, 6]
        }
        for (let i = 0; i < rbytes.length; i++) {
            digits = BigInt.multiplyDigits(digits, mask)
            let b = ((rbytes[i].toLong() & BigInt.maskSys) as Long).toString()
            let temp: FixedArray<int> = new FixedArray<int>(b.length.toInt())
            for (let j = 0; j < b.length; j++) {
                temp[j] = b.charAt(j).toInt() - c'0'.toInt();
            }

            digits = BigInt.addDigits(digits, temp)
        }

        return digits
    }

    private static processRadixPrefix(s: string): [string, int] {
        if (s.length >= 2 && s.charAt(0) == c'0') {
            const secondChar = s.charAt(1).toLowerCase()
            switch (secondChar) {
                case c'b': 
                    return [s.substring(2), 2]
                case c'o': 
                    return [s.substring(2), 8]
                case c'x': 
                    return [s.substring(2), 16]
            }
        }
        return [s, 10]
    }

    private static multiplyDecimalArrayWithDigit(a: Array<int>, digit: int): Array<int> {
        const lenA: int = a.length
        const result: Array<int> = Array.create<int>(lenA + 1, 0)
        let carry: int = 0 
        
        for (let i = lenA - 1; i >= 0; i--) {
            const product: int = a[i] * digit + carry
            result[i + 1] = product % 10
            carry = product / 10
        }
        
        if (carry > 0) {
            result[0] = carry
            return result
        } 
        return result.slice(1)
        
    }

    private static addDecimalArrayWithDigit(a: Array<int>, digit: int): Array<int> {
        if (digit === 0) return a.slice()        
        const result: Array<int> = a.slice()
        let carry = digit
        let i: int = result.length - 1
        
        while (i >= 0 && carry > 0) {
            const sum: int = result[i] + carry
            result[i] = sum % 10
            carry = sum / 10
            i--
        }
        if (carry > 0) {
            result.unshift(carry)
        }
        return result
    }

    private static convertToDecimalArray(s: string, radix: int): Array<int> {
        if (s.isEmpty()) {
            return [0]
        }
        
        let result: Array<int> = [0]      
        for (let i = 0; i < s.length; i++) {
            let currentChar: char = s.charAt(i)           
            if (currentChar == c'_') {
                continue  
            }
            
            let digitValue: int
            let charCode = currentChar.toInt()
            
            if (radix == 16) {
                if (charCode >= c'0'.toInt() && charCode <= c'9'.toInt()) {
                    digitValue = charCode - c'0'.toInt()
                } else if (charCode >= c'a'.toInt() && charCode <= c'f'.toInt()) {
                    digitValue = charCode - c'a'.toInt() + 10
                } else {
                    digitValue = charCode - c'A'.toInt() + 10
                }
            } else if (radix == 8) {
                digitValue = charCode - c'0'.toInt()
            } else if (radix == 2) {
                digitValue = charCode - c'0'.toInt()
            } else {
                throw new IllegalArgumentError()
            }
            
            result = BigInt.multiplyDecimalArrayWithDigit(result, radix)
            result = BigInt.addDecimalArrayWithDigit(result, digitValue)
        }
        
        return result
    }

    private static parseFromString(s: string): [FixedArray<int>, int] {
        if (s.isEmpty()) {
            // Return 0 when provided string is empty
            s = "0"
        }

        const processedStringRadix = BigInt.processRadixPrefix(s)
        s = processedStringRadix[0]
        let radix = processedStringRadix[1]

        if (s.isEmpty() || !BigInt.validate(s,radix)) {
            throw new Error("BigInt.parseFromString(): provided string is invalid")
        }

        let firstChar = s.charAt(0)
        let negative: boolean = (firstChar == c'-')
        if (firstChar == c'-' || firstChar == c'+') {
            s = s.substring(1)
        }

        if(radix != 10) {
            s = BigInt.convertToDecimalArray(s,radix).join('')
        }
        let len: int = s.length.toInt()
        let shift: int = 0
        let expPosition: int = s.indexOf(c'e')
        if (expPosition > 0) {
            shift = BigInt.parseInt(s, expPosition + 1, len)
            len = expPosition
        }
        let digits = new Array<Int>(0)
        for (let i = 0; i < len; i++) {
            if (s.charAt(i) == c'_') {
                continue
            }

            if (s.charAt(i) == c'.') {
                shift -= len - i - 1; // Length of fraction part.
                continue
            }

            if (i >= len + shift) { // Check the rest is zeros for negative shift.
                if (s.charAt(i) != c'0') {
                    throw new Error("Can't convert number " + s + " to BigInt because it's not an integer")
                }
            } else {
                digits.push(s.charAt(i).toInt() - c'0'.toInt())
            }
        }

        // In case of unnormalized float, e.g. 0e-3, append at least one 0.
        for (let i = 0; i < shift || digits.length == 0; i++) {
            digits.push(0)
        }

        let bytes = BigInt.stripLeadingZeros(BigInt.toBytes(digits))
        let sign = bytes.length == 0 ? 0 : negative ? -1 : 1

        return [bytes, sign]
    }

    private static parseInt(s: string, start: int, end: int): int {
        let value: int = 0
        let sign: int = 1
        let firstChar = s.charAt(start)
        if (firstChar == c'-' || firstChar == c'+') {
            start += 1
            if (firstChar == c'-') {
                sign = -1
            }
        }

        for (let i = start; i < end; ++i) {
            value = value * 10 + (s.charAt(i).toInt() - c'0'.toInt())
        }

        return sign * value
    }

    private static toBytes(digits: Array<Int>): FixedArray<int> {
        // 3402 bits per each 1024 symbols for decimal number in string
        let chunkSize = (((digits.length * BigInt.BITS_PER_1024_DEC) >>> 10) + BigInt.shiftSys) >>> 5
        let bytes: FixedArray<int> = new FixedArray<int>(chunkSize.toInt())
        let position = 0
        while (!(digits.length == 0)) {
            let carry: long = 0
            for (let i = 0; i < digits.length; i++) {
                let current: long = digits[i].toLong() + carry * (10 as long)
                if (current < BigInt.baseSys) {
                    carry = current
                    digits[i] = 0
                    continue
                }

                digits[i] = (current >>> BigInt.shiftSys).toInt()
                carry = current & BigInt.maskSys
            }

            bytes[position++] = carry.toInt()
            while (!(digits.length == 0)) {
                if (digits[0] == 0) {
                    digits.shift()
                } else {
                    break
                }
            }
        }

        return bytes
    }

    private static validate(s: string, radix: int = 10): boolean {
        let expPosition = -1
        let isDouble = false
        for (let i = 0; i < s.length; i++) {
            let c = s.charAt(i)
            if (BigInt.isValidDigitForRadix(c, radix)) {
            } else if ((c == c'_') && (i > 0) && (i < s.length - 1)) {
                // Underscore, allowed only between digits
            } else if ((i == 0 || i == expPosition) && (c == c'-' || c == c'+')) {
                // Minus or plus sign, allowed at the beginning of the string
                // or at the beggining of the scientific notation
            } else if (c == c'.' && !isDouble && expPosition < 0 && radix == 10) {
                // Fraction is not allowed in exponential part
                isDouble = true
            } else if ((c == c'e') && expPosition < 0) {
                expPosition = i + 1
            } else {
                // Invalid character
                return false
            }
        }

        return true
    }

    private static isValidDigitForRadix(judgeChar: char, radix: int): boolean {
        switch (radix) {
            case 2: 
                return judgeChar == c'0' || judgeChar == c'1'
                
            case 8: 
                return judgeChar >= c'0' && judgeChar <= c'7'
                
            case 10: 
                return judgeChar >= c'0' && judgeChar <= c'9'
                
            case 16: 
                return (judgeChar >= c'0' && judgeChar <= c'9') || 
                       (judgeChar >= c'A' && judgeChar <= c'F') ||
                       (judgeChar >= c'a' && judgeChar <= c'f')
            default:
                return false
        }
    }

    private static multiplyDigits(lhs: FixedArray<int>, rhs: FixedArray<int>): FixedArray<int> {
        if ((lhs.length == 0) || (rhs.length == 0)) {
            let r: FixedArray<int> = new FixedArray<int>(1)
            r[0] = 0
            return r
        }

        if (lhs.length == 1 && lhs[0] == 0) {
            let r: FixedArray<int> = new FixedArray<int>(1)
            r[0] = 0
            return r
        }

        if (rhs.length == 0 && rhs[0] == 0) {
            let r: FixedArray<int> = new FixedArray<int>(1)
            r[0] = 0
            return r
        }

        let len1: int = lhs.length as int
        let len2: int = rhs.length as int
        let product: FixedArray<int> = new FixedArray<int>(len1 + len2)
        BigInt.resetBytes(product)

        for (let i = len1 - 1; i >= 0; i--) {
            let mulflag: long = 0
            let addflag: long = 0
            for (let j = len2 - 1; j >= 0; j--) {
                let l: long = lhs[i].toLong() & BigInt.maskSys
                let r: long = rhs[j].toLong() & BigInt.maskSys
                let temp1: long = l * r + mulflag

                mulflag = temp1 / 10
                temp1 = temp1 % 10
                let temp2: long = product[i + j + 1] + temp1 + addflag
                product[i + j + 1] = (temp2 % 10).toInt()
                addflag = temp2 / 10
            }

            product[i] = (product[i] + mulflag + addflag).toInt()
        }

        if (!(product.length == 0)) {
            if (product[0] == 0) {
               let res: FixedArray<int> = new FixedArray<int>(product.length - 1)
               for (let i = 1; i < product.length; i++ ) {
                   res[i - 1] = product[i]
               }
               product = res
            }
        }

        return product
    }

    private static reverse(input: FixedArray<int>): FixedArray<int> {
        let result: FixedArray<int> = new FixedArray<int>(input.length)
        let j = 0
        for (let i = input.length - 1; i >= 0; --i) {
            result[j++] = input[i]
        }

        return result
    }

    private static to2sComplement(input: FixedArray<int>): FixedArray<int> {
        // Perform 2's complement for negative numbers.
        let carry: long = 1
        for (let i = 0; i < input.length; i++) {
            let sum: long = (~input[i]).toLong() & BigInt.maskSys
            sum += carry
            input[i] = sum.toInt()
            carry = sum >>> BigInt.shiftSys
        }
        return input
    }

    private static addDigits(lhs: FixedArray<int>, rhs: FixedArray<int>): FixedArray<int> {
        let max_len = max(lhs.length, rhs.length) as int
        let result: FixedArray<int> = new FixedArray<int>(max_len)

        let carry: int = 0
        for (let i: int = 0; i < max_len; i++) {
            let l: int = i < lhs.length ? lhs[lhs.length - i - 1] : 0
            let r: int = i < rhs.length ? rhs[rhs.length - i - 1] : 0
            let sum: int = l + r + carry
            carry = sum / 10
            result[result.length - i - 1] = ((sum % 10) as int)
        }

        if (carry > 0) {
            let tmp = result
            result = new FixedArray<int>(max_len + 1)
            result[0] = carry
            for (let i = 1; i <= max_len; i++) {
                result[i] = tmp[i - 1]
            }
        }

        return result
    }

    /**
     * Returns the value of an object as a double
     *
     * @returns object's value
     */
    public doubleValue(): double {
        let buff = 0 as number
        const bytes = this.bytes
        for (let i = 0; i < bytes.length; ++i) {
            buff += (bytes[i].toLong() & BigInt.maskSys).toDouble() * Math.pow(BigInt.baseSys, i)
        }
        return this.sign * buff
    }

    /**
     * Returns the value of an object as a long
     *
     * @returns object's value
     */
    public getLong(): long {
        let buff = 0 as long
        const bytes = this.bytes
        const num = Math.min(2, bytes.length)
        for (let i = 0; i < num; ++i) {
            buff += (bytes[i].toLong() & BigInt.maskSys) << i * BigInt.shiftSys
        }
        return this.sign * buff
    }

    /**
     * Return clipped bigint num to 64 bits
     */
    private static asUint64(num: BigInt): BigInt {
        return num & BigInt.BIGINT64_MASK
    }

    /**
     * Return current value clipped to unsigned long
     */
    public getULong(): long {
        return BigInt.asUint64(this).getLong()
    }

     /**
     * Creates a new instance of BigInt from existing BigInt number
     *
     * @param value BigInt value
     *
     * @returns A new BigInt instance from existing BigInt
     */
    static $_invoke(value: BigInt): BigInt {
        return new BigInt(value)
    }

    /**
     * Creates a new instance of BigInt from number instance
     *
     * @param value number value
     *
     * @returns A new BigInt instance from number
     */
    static $_invoke(value: number): BigInt {
        return new BigInt(value)
    }

    /**
     * Creates a new instance of BigInt from string
     *
     * @param value string value
     *
     * @returns A new BigInt instance from string
     */
    static $_invoke(value: string): BigInt {
        return new BigInt(value)
    }

    /**
     * Creates a new instance of BigInt from boolean
     *
     * @param value boolean value
     *
     * @returns A new BigInt instance from boolean
     */
    static $_invoke(value: boolean): BigInt {
        return new BigInt(value)
    }

    /**
     * Creates a new instance of BigInt from union of bigint/number/string/boolean
     *
     * @param value source value
     *
     * @returns A new BigInt instance from union of bigint/number/string/boolean
     */
    static $_invoke(value: BigInt | Number | String | Boolean): BigInt {
        if (value instanceof BigInt) {
            return new BigInt(value as BigInt)
        } else if (value instanceof Number) {
            return new BigInt(value as Number)
        } else if (value instanceof String) {
            return new BigInt(value as String)
        } else if (value instanceof Boolean) {
            return new BigInt(value as Boolean)
        }
        return new BigInt(0)
    }

    private static fromDoubleInteger(d: double): BigInt {
        if (!Number.isFinite(d)) {
            throw new RangeError(`the Double value ${d} cannot be converted to a BigInt because it is not finite`);
        }

        if (d == 0) {
            return new BigInt(0);
        }

        const buffer = new ArrayBuffer(8);
        const view = new DataView(buffer);

        // big-endian
        view.setFloat64(0, d, false);

        const hi = view.getUint32(0);
        const lo = view.getUint32(4);

        // IEEE-754
        const signBit = hi >>> 31;
        const exponentBits = (hi >>> 20) & 0x7FF;
        const significandHi = hi & 0xFFFFF;
        const significandLo = lo;

        // Mantissa as integer val can hold upto 52 bit
        let mantissaNum = significandHi * Math.pow(2, 32) + significandLo;

        let exponent: int;
        if (exponentBits === 0) {
            // exponent = 1 - bias - 52
            exponent = (1 - 1023 - 52) as int;
        } else {
            // (1.xxx... * 2^(exponentBits - bias))
            mantissaNum += Math.pow(2, 52);  // hidden bit
            exponent = (exponentBits - 1023 - 52).toInt();
        }

        // mantissaNum is integer <= 2^53-1
        let result = new BigInt(mantissaNum as double);
        if (exponent > 0) {
            result = result.operatorLeftShift(new BigInt(exponent));
        } else if (exponent < 0) {
            result = result.operatorRightShift(new BigInt(-exponent));
        }
        if (signBit) {
            result = result.negate();
        }
        return result;
    }

}
