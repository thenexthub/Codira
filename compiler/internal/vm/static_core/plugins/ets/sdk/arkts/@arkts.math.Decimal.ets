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

export type Rounding = int
export type Modulo = int
export type Value = Decimal | double | string;

const RANGE_ERROR_CODE: int = 10200001;
const TYPE_ERROR_CODE: int = 401;
const PRECISION_LIMIT_EXCEEDED_ERROR_CODE: int = 10200060;
const CRYPTO_UNAVAILABLE_ERROR_CODE: int = 10200061;

const EXP_LIMIT: double = 9e15;
const MAX_DIGITS: double = 1e9;
const NUMERALS: string = '0123456789abcdef';

const LN10 = ('2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298341'
    + '9677840422862486334095254650828067566662873690987816894829072083255546808437998948262331985283935053089653777326'
    + '2884616336622228769821988674654366747440424327436515504893431493939147961940440022210510171417480036880840126470'
    + '8068556774321622835522011480466371565912137345074785694768346361679210180644507064800027750268491674655058685693'
    + '5673420670581136429224554405758925724208241314695689016758940256776311356919292033376587141660230105703089634572'
    + '0754403708474699401682692828084811842893148485249486448719278096762712757753970276686059524967166741834857044225'
    + '0719796500471495105049221477656763693866297697952211071826454973477266242570942932258279850258550978526538320760'
    + '6726317164309505995087807523710333101197857547331541421808427543863591778117054309827482385045648019095610299291'
    + '8243182375253577097505395651876975103749708886921802051893395072385392051446341972652872869651108625714921988499'
    + '78748873771345686209167058');

const PI = ('3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214'
    + '8086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933'
    + '4461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209'
    + '2096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179'
    + '3105118548074462379962749567351885752724891227938183011949129833673362440656643086021394946395224737190702179860'
    + '9437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249'
    + '5343014654958537105079227968925892354201995611212902196086403441815981362977477130996051870721134999999837297804'
    + '9951059731732816096318595024459455346908302642522308253344685035261931188171010003137838752886587533208381420617'
    + '1776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989'
    + '380952572010654858632789');

const DEFAULTS_PRECISION = 20;
const DEFAULTS_ROUNDING = 4;
const DEFAULTS_MODULO = 1;
const DEFAULTS_TOEXPNEG = -7;
const DEFAULTS_TOEXPPOS = 21;

let external: boolean = true;
let inexact: boolean = false;
const tag: string = '[object Decimal]';

const isBinary: RegExp = new RegExp("^0b([01]+(\\.[01]*)?|\\.[01]+)(p[+-]?\\d+)?$", "i");
const isHex: RegExp = new RegExp("^0x([0-9a-f]+(\\.[0-9a-f]*)?|\\.[0-9a-f]+)(p[+-]?\\d+)?$", "i");
const isOctal: RegExp = new RegExp("^0o([0-7]+(\\.[0-7]*)?|\\.[0-7]+)(p[+-]?\\d+)?$", "i");
const isDecimal: RegExp = new RegExp("^(\\d+(\\.\\d*)?|\\.\\d+)(e[+-]?\\d+)?$", "i");

const BASE: double = 1e7;
const LOG_BASE: int = 7;
const MAX_SAFE_INTEGER: double = 9007199254740991;

const LN10_PRECISION = LN10.length - 1;
const PI_PRECISION = PI.length - 1;

export interface DecimalConfig {
    precision?: double;
    rounding?: Rounding;
    toExpNeg?: double;
    toExpPos?: double;
    minE?: double;
    maxE?: double;
    crypto?: boolean;
    modulo?: Modulo;
    defaults?: boolean;
}

export class Decimal {
    public digits: Array<double> | null = new Array<double>();
    public exponent: double;
    private sign: double;
    public readonly toStringTag: string = '[object Decimal]';
    private static quadrant: int = 0;

    public static maxE: double = EXP_LIMIT;
    public static minE: double = -EXP_LIMIT;
    public static precision: double = DEFAULTS_PRECISION;
    public static rounding: Rounding = DEFAULTS_ROUNDING;
    public static toExpNeg: double = DEFAULTS_TOEXPNEG;
    public static toExpPos: double = DEFAULTS_TOEXPPOS;
    public static modulo: Modulo = DEFAULTS_MODULO;
    public static crypto: boolean = false;

    // defined in d.ets, See "@arkts.math.Decimal.d.ets" for details.
    public static readonly ROUND_UP: int = 0;
    public static readonly ROUND_DOWN: int = 1;
    public static readonly ROUND_CEILING: int = 2;
    public static readonly ROUND_FLOOR: int = 3;
    public static readonly ROUND_HALF_UP: int = 4;
    public static readonly ROUND_HALF_DOWN: int = 5;
    public static readonly ROUND_HALF_EVEN: int = 6;
    public static readonly ROUND_HALF_CEILING: int = 7;
    public static readonly ROUND_HALF_FLOOR: int = 8;
    public static readonly EUCLIDEAN: int = 9;

    get d(): Array<double> | null {
        return this.digits;
    }

    get e(): double {
        return this.exponent;
    }

    get s(): double {
        return this.sign;
    }

    /**
     * Return a new Decimal whose value is the absolute value of this Decimal.
     *
     * @param { Value } n {double | string | Decimal}
     */
    constructor(n: Value) {
        if (Utils.isDecimalInstance(n)) {
            this.initializeByDecimal(n as Decimal);
        } else if (typeof n === 'number') {
            this.initializeByNumber(n as double);
        } else if (typeof n === 'string') {
            this.initializeByString(n as string);
        } else {
            throw Utils.createBusinessError(TYPE_ERROR_CODE,
                `The type of "index" must be String. Received value is: ${n}`);
        }
    }

    /**
     * Return a new Decimal whose value is the absolute value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public abs(): Decimal {
        let x = new Decimal(this);
        if (x.sign < 0) {
            x.sign = 1;
        }
        return Utils.finaliseExternal(x);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a whole number in the
     * direction of negative Infinity.
     *
     * @returns { Decimal } the Decimal type
     */
    public floor(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_FLOOR);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a whole number in the
     * direction of positive Infinity.
     *
     * @returns { Decimal } the Decimal type
     */
    public ceil(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_CEILING);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal truncated to a whole number.
     *
     * @returns { Decimal } the Decimal type
     */
    public trunc(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_DOWN);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal clamped to the range
     * delineated by `min` and `max`.
     *
     * @param { Value } min {double | string | Decimal}
     * @param { Value } max {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `min` is out of range.
     */
    public clamp(min: Value, max: Value): Decimal {
        let minDecimal = new Decimal(min);
        let maxDecimal = new Decimal(max);
        if (!minDecimal.s || !maxDecimal.s) {
            return new Decimal(NaN);
        }
        if (minDecimal.greaterThan(maxDecimal)) {
            throw Utils.createBusinessError(RANGE_ERROR_CODE,
                `The value of min is out of range. It must be <= ${maxDecimal}. Received value is: ${minDecimal}`);
        }
        let k = this.comparedTo(minDecimal);
        return k < 0 ? minDecimal : this.comparedTo(maxDecimal) > 0 ? maxDecimal : new Decimal(this);
    }

    /**
     * Return true if the value of this Decimal is a finite number, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isFinite(): boolean {
        return !!this.digits;
    }

    /**
     * Return true if the value of this Decimal is an integer, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isInteger(): boolean {
        return !!this.digits && Math.floor(this.exponent / LOG_BASE) > this.digits!.length - 2;
    }

    /**
     * Return true if the value of this Decimal is NaN, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isNaN(): boolean {
        return !this.sign;
    }

    /**
     * Return true if the value of this Decimal is negative, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isNegative(): boolean {
        return this.sign < 0;
    }

    /**
     * Return true if the value of this Decimal is positive, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isPositive(): boolean {
        return this.sign > 0;
    }

    /**
     * Return true if the value of this Decimal is 0 or -0, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isZero(): boolean {
        return !!this.digits && this.digits!.at(0) === 0;
    }

    /**
     * Return a new Decimal whose value is the integer part of dividing the value of this Decimal
     * by the value of `n`, rounded to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public dividedToIntegerBy(n: Value): Decimal {
        return Utils.finalise(Utils.divide(this, new Decimal(n), 0, 1, true), Decimal.precision, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal negated, i.e. as if multiplied by -1.
     *
     * @returns { Decimal } the Decimal type
     */
    public negate(): Decimal {
        let x = new Decimal(this);
        x.sign = -x.sign;
        return Utils.finaliseExternal(x);
    }

    /**
     * Return a string representing the value of this Decimal in base 2.
     *
     * @returns { string } the string type
     */
    public toBinary(): string {
        return this.toStringBinary(2);
    }

    /**
     * Return a string representing the value of this Decimal in base 2, round to `significantDigits`
     * significant digits.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toBinary(significantDigits: double): string {
        return this.toStringBinary(2, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 2, round to `significantDigits`
     * significant digits using rounding mode `rounding`.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toBinary(significantDigits: double, rounding: Rounding): string {
        return this.toStringBinary(2, significantDigits, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in base 8.
     *
     * @returns { string } the string type
     */
    public toOctal(): string {
        return this.toStringBinary(8);
    }

    /**
     * Return a string representing the value of this Decimal in base 8, round to `significantDigits` significant.
     *
     * @param { double } significantDigits {double | string | Decimal}
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toOctal(significantDigits: double): string {
        return this.toStringBinary(8, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 8, round to `significantDigits` significant
     * digits using rounding mode `rounding`.
     *
     * @param { double } significantDigits {double | string | Decimal}
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toOctal(significantDigits: double, rounding: Rounding): string {
        return this.toStringBinary(8, significantDigits, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in base 16
     *
     * @returns { string } the string type
     */
    public toHexadecimal(): string {
        return this.toStringBinary(16);
    }

    /**
     * Return a string representing the value of this Decimal in base 16, round to `significantDigits` significant.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toHexadecimal(significantDigits: double): string {
        return this.toStringBinary(16, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 16, round to `significantDigits` significant
     * digits using rounding mode `rounding`.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toHexadecimal(significantDigits: double, rounding: Rounding): string {
        return this.toStringBinary(16, significantDigits, rounding);
    }

    /**
     * Return an array representing the value of this Decimal as a simple fraction with an integer
     * numerator and an integer denominator.
     *
     * @returns { Decimal[] } the Decimal[] type
     */
    public toFraction(): Decimal[] {
        return this.fraction();
    }

    /**
     * Return an array representing the value of this Decimal as a simple fraction with an integer
     * numerator and an integer denominator. The denominator will be a positive non-zero value
     * less than or equal to `max_denominator`.
     *
     * @param { Value } maxDenominator {double | string | Decimal}
     * @returns { Decimal[] } the Decimal[] type
     */
    public toFraction(maxDenominator: Value): Decimal[] {
        return this.fraction(maxDenominator);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(): Decimal {
        return new Decimal(this);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `decimalPlaces`
     * decimal places.
     *
     * @param { double } decimalPlaces Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(decimalPlaces: double): Decimal {
        return this.toDecimalPlaces(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `decimalPlaces`
     * decimal places using rounding mode `rounding`.
     *
     * @param { double } decimalPlaces Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(decimalPlaces: double, rounding: Rounding): Decimal {
        let decimal = this.toDecimalPlaces();
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        return Utils.finalise(decimal, decimalPlaces + decimal.e + 1, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation.
     *
     * @returns { string } the string type
     */
    public toExponential(): string {
        let str = this.finiteToString(true);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation rounded to
     * `decimalPlaces` fixed decimal places.
     *
     * @param { double } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces` is out of range.
     */
    public toExponential(decimalPlaces: double): string {
        return this.toExponential(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation rounded to
     * `decimalPlaces` fixed decimal places using rounding mode `rounding`.
     *
     * @param { double } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces | rounding` is out of range.
     */
    public toExponential(decimalPlaces: double, rounding: Rounding): string {
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let x = Utils.finalise(new Decimal(this), decimalPlaces + 1, rounding);
        let str = x.finiteToString(true, decimalPlaces + 1);
        return x.isNegative() && !x.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point).
     *
     * @returns { string } the string type
     */
    public toFixed(): string {
        let str = this.finiteToString(false);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point) notation to
     * `decimalPlaces` fixed decimal places.
     *
     * @param { double } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces` is out of range.
     */
    public toFixed(decimalPlaces: double): string {
        return this.toFixed(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point) notation to
     * `decimalPlaces` fixed decimal places and rounded using rounding mode `rounding`.
     *
     * @param { double } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces | rounding` is out of range.
     */
    public toFixed(decimalPlaces: double, rounding: Rounding): string {
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let y = Utils.finalise(new Decimal(this), decimalPlaces + this.e + 1, rounding);
        let str = y.finiteToString(false, decimalPlaces + y.e + 1);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Returns a new Decimal whose value is the nearest multiple of `n`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public toNearest(n: Value): Decimal {
        return this.toNearest(n, Decimal.rounding);
    }

    /**
     * Returns a new Decimal whose value is the nearest multiple of `n` in the direction of rounding
     * mode `rounding`, to the value of this Decimal.
     *
     * @param { Value } n {double | string | Decimal}
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `rounding` is out of range.
     */
    public toNearest(n: Value, rounding: Rounding): Decimal {
        let x = new Decimal(this);
        let y = new Decimal(n);
        Utils.checkInt32(rounding, 0, 8);

        // If x is not finite, return x if y is not NaN, else NaN.
        if (!x.d) {
            return y.s ? x : y;
        }

        // If y is not finite, return Infinity with the sign of x if y is Infinity, else NaN.
        if (!y.d) {
            if (y.s) {
                y.sign = x.s;
            }
            return y;
        }

        // If y is not zero, calculate the nearest multiple of y to x.
        if (y.d![0]) {
            external = false;
            x = Utils.divide(x, y, 0, rounding, true).mul(y);
            external = true;
            Utils.finalise(x, undefined, undefined);
        } else {
            // If y is zero, return zero with the sign of x.
            y.sign = x.s;
            x = y;
        }

        return x;
    }

    /**
     * Return a string representing the value of this Decimal.
     *
     * @returns { string } the string type
     */
    public toPrecision(): string {
        let str = this.finiteToString(this.e <= Decimal.toExpNeg || this.e >= Decimal.toExpPos);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal rounded to `significantDigits` significant digits.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toPrecision(significantDigits: double): string {
        return this.toPrecision(significantDigits, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal rounded to `significantDigits` significant digits
     * using rounding mode `rounding`.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toPrecision(significantDigits: double, rounding: Rounding): string {
        Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let x = Utils.finalise(new Decimal(this), significantDigits, rounding);
        let str = x.finiteToString(significantDigits <= x.e || x.e <= Decimal.toExpNeg, significantDigits);
        return x.isNegative() && !x.isZero() ? '-' + str : str;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public toSignificantDigits(): Decimal {
        return Utils.finalise(new Decimal(this), Decimal.precision, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `significantDigits`
     * significant digits.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toSignificantDigits(significantDigits: double): Decimal {
        return this.toSignificantDigits(significantDigits, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `significantDigits`
     * significant digits using rounding mode `rounding`.
     *
     * @param { double } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toSignificantDigits(significantDigits: double, rounding: Rounding): Decimal {
        Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        return Utils.finalise(new Decimal(this), significantDigits, rounding);
    }

    /**
     * Return the value of this Decimal converted to a number primitive. Zero keeps its sign.
     *
     * @returns { double } the number type
     */
    public toNumber(): double {
        return Utils.toNumber(this.valueOf());
    }

    /**
     * Return a string representing the value of this Decimal.
     * Return exponential notation if this Decimal has a positive exponent equal to or greater than
     * `toExpPos`, or a negative exponent equal to or less than `toExpNeg`.
     *
     * @returns { string } the string type
     */
    public toString(): string {
        let str = this.finiteToString(this.exponent <= Decimal.toExpNeg || this.exponent >= Decimal.toExpPos);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return the number of decimal places of the value of this Decimal.
     *
     * @returns { double } the number type
     */
    public decimalPlaces(): double {
        let digits = this.digits;
        let result: double = NaN;
        if (digits) {
            let len = digits!.length - 1;
            result = (len - Math.floor(this.exponent / LOG_BASE)) * LOG_BASE;
            let value = digits!.at(len);
            if (value) {
                let v: double = value! 
                for (; v % 10 == 0; v /= 10) {
                    result--;
                }
            }
            if (result < 0) {
                result = 0;
            }
        }
        return result;
    }

    /**
     * Return a string representing the value of this Decimal.
     * Unlike `toString`, negative zero will include the minus sign.
     *
     * @returns { string } the string type
     */
    public valueOf(): string {
        let str = this.finiteToString(this.exponent <= Decimal.toExpNeg || this.exponent >= Decimal.toExpPos);
        return this.isNegative() ? '-' + str : str;
    }

    /**
     * Return the number of significant digits of the value of this Decimal.
     *
     * @returns { double } the number type
     */
    public precision(): double {
        let result: double;
        if (this.digits) {
            result = Utils.getPrecision(this.digits!);
        } else {
            result = NaN;
        }
        return result;
    }

    /**
     * Return the number of significant digits of the value of this Decimal, whether to count
     * integer-part trailing zeros.
     *
     * @param { boolean | int } includeZeros Whether to count integer-part trailing zeros: true, false,
     * 1 or 0.
     * @returns { double } the number type
     * @throws { BusinessError } 10200001 - The value of `includeZeros` is out of range.
     */
    public precision(includeZeros: boolean | int): double {
        let result: double;
        if (includeZeros != undefined && includeZeros !== !!includeZeros && includeZeros !== 1 && includeZeros !== 0) {
            throw Utils.createBusinessError(RANGE_ERROR_CODE,
                `The value of includeZeros is out of range. It must be 0 or 1. Received value is: ${includeZeros}`);
        }

        if (this.digits) {
            result = Utils.getPrecision(this.digits!);
            if (includeZeros && this.exponent + 1 > result) {
                result = this.exponent + 1;
            }
        } else {
            result = NaN;
        }
        return result;
    }

    /**
     * Return a new Decimal whose value is the absolute value of `n`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static abs(n: Value): Decimal {
        return new Decimal(n).abs();
    }

    /**
     * Return a new Decimal whose value is `n` round to an integer using `ROUND_FLOOR`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static floor(n: Value): Decimal {
        return new Decimal(n).floor();
    }

    /**
     * Return a new Decimal whose value is `n` rounded to an integer using `ROUND_CEIL`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static ceil(n: Value): Decimal {
        return new Decimal(n).ceil();
    }

    /**
     * Return a new Decimal whose value is `n` truncated to an integer.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static trunc(n: Value): Decimal {
        return new Decimal(n).trunc();
    }

    /**
     * Return a new Decimal whose value is `n` clamped to the range delineated by `min` and `max`.
     *
     * @param { Value } n {double | string | Decimal}
     * @param { Value } min {double | string | Decimal}
     * @param { Value } max {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static clamp(n: Value, min: Value, max: Value): Decimal {
        return new Decimal(n).clamp(min, max);
    }

    /**
     * Return a new Decimal whose value is the sum of `x` and `y`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } x {double | string | Decimal}
     * @param { Value } y {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static add(x: Value, y: Value): Decimal {
        return new Decimal(x).add(y);
    }

    /**
     * Return a new Decimal whose value is the sum of the arguments, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * Only the result is rounded, not the intermediate calculations.
     *
     * @param { Value[] } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sum(...values: Value[]): Decimal {
        let result = new Decimal(values[0]);
        external = false;
        for (let i = 1; i < values.length; i++) {
            if (result.isNaN()) {
                break;
            }
            result = result.add(values[i]);
        }
        external = true;
        return Utils.finalise(result, Decimal.precision, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is `x` minus `y`, rounded to `precision` significant digits
     * using rounding mode `rounding`.
     *
     * @param { Value } x {double | string | Decimal}
     * @param { Value } y {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sub(x: Value, y: Value): Decimal {
        return new Decimal(x).sub(y);
    }

    /**
     * Return a new Decimal whose value is `x` multiplied by `y`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } x {double | string | Decimal}
     * @param { Value } y {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static mul(x: Value, y: Value): Decimal {
        return new Decimal(x).mul(y);
    }

    /**
     * Return a new Decimal whose value is `x` divided by `y`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } x {double | string | Decimal}
     * @param { Value } y {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static div(x: Value, y: Value): Decimal {
        return new Decimal(x).div(y);
    }

    /**
     * Return a new Decimal whose value is `x` modulo `y`, rounded to `precision` significant digits
     * using rounding mode `rounding`.
     *
     * @param { Value } x {double | string | Decimal}
     * @param { Value } y {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static mod(x: Value, y: Value): Decimal {
        return new Decimal(x).mod(y);
    }

    /**
     * Return a new Decimal whose value is the square root of `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sqrt(n: Value): Decimal {
        return new Decimal(n).sqrt();
    }

    /**
     * Return a new Decimal whose value is the cube root of `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static cbrt(n: Value): Decimal {
        return new Decimal(n).cbrt();
    }

    /**
     * Return a new Decimal whose value is `base` raised to the power `exponent`, rounded to precision
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } base {double | string | Decimal} The base.
     * @param { Value } exponent {double | string | Decimal} The exponent.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static pow(base: Value, exponent: Value): Decimal {
        return new Decimal(base).pow(exponent);
    }

    /**
     * Return a new Decimal whose value is the natural exponential of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static exp(n: Value): Decimal {
        return new Decimal(n).exp();
    }

    /**
     * Return a new Decimal whose value is the log of `n` to the base `base`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @param { Value } base {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static log(n: Value, base: Value): Decimal {
        return new Decimal(n).log(base);
    }

    /**
     * Return a new Decimal whose value is the natural logarithm of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static ln(n: Value): Decimal {
        return new Decimal(n).ln();
    }

    /**
     * Return a new Decimal whose value is the base 2 logarithm of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static log2(n: Value): Decimal {
        return new Decimal(n).log(2);
    }

    /**
     * Return a new Decimal whose value is the base 10 logarithm of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static log10(n: Value): Decimal {
        return new Decimal(n).log(10);
    }

    /**
     * Return a new Decimal whose value is the cosine of `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     */
    static cos(n: Value): Decimal {
        return new Decimal(n).cos();
    }

    /**
     * Return a new Decimal whose value is the sine of `n`, rounded to `precision` significant digits
     * using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     */
    static sin(n: Value): Decimal {
        return new Decimal(n).sin();
    }

    /**
     * Return a new Decimal whose value is the tangent of `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     */
    static tan(n: Value): Decimal {
        return new Decimal(n).tan();
    }

    /**
     * Return a new Decimal whose value is the hyperbolic cosine of `n`, rounded to precision
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     */
    static cosh(n: Value): Decimal {
        return new Decimal(n).cosh();
    }

    /**
     * Return a new Decimal whose value is the hyperbolic sine of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sinh(n: Value): Decimal {
        return new Decimal(n).sinh();
    }

    /**
     * Return a new Decimal whose value is the hyperbolic tangent of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     */
    static tanh(n: Value): Decimal {
        return new Decimal(n).tanh();
    }

    /**
     * Return a new Decimal whose value is the arccosine in radians of `n`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static acos(n: Value): Decimal {
        return new Decimal(n).acos();
    }

    /**
     * Return a new Decimal whose value is the arcsine in radians of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static asin(n: Value): Decimal {
        return new Decimal(n).asin();
    }

    /**
     * Return a new Decimal whose value is the arctangent in radians of `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static atan(n: Value): Decimal {
        return new Decimal(n).atan();
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic cosine of `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static acosh(n: Value): Decimal {
        return new Decimal(n).acosh();
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic sine of `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.

     */
    static asinh(n: Value): Decimal {
        return new Decimal(n).asinh();
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic tangent of `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal} A value in radians.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static atanh(n: Value): Decimal {
        return new Decimal(n).atanh();
    }

    /**
     * Return a new Decimal whose value is the arctangent in radians of `y/x` in the range -pi to pi
     * (inclusive), rounded to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } y {double | string | Decimal} The y-coordinate.
     * @param { Value } x {double | string | Decimal} The x-coordinate.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    static atan2(y: Value, x: Value): Decimal {
        y = new Decimal(y);
        x = new Decimal(x);
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        let wpr = pr + 4;
        let result: Decimal;
        // Either NaN
        if (!y.s || !x.s) {
            result = new Decimal(NaN);
        } else if (!y.d && !x.d) { // Both ±Infinity
            result = Utils.getPi(wpr, 1).mul(x.s > 0 ? 0.25 : 0.75);
            result.sign = y.s;
        } else if (!x.d || y.isZero()) { // x is ±Infinity or y is ±0
            result = x.s < 0 ? Utils.getPi(pr, rm) : new Decimal(0);
            result.sign = y.s;
        } else if (!y.d || x.isZero()) { // y is ±Infinity or x is ±0
            result = Utils.getPi(wpr, 1).mul(0.5);
            result.sign = y.s;
        } else if (x.s < 0) { // Both non-zero and finite
            Decimal.precision = wpr;
            Decimal.rounding = 1;
            result = Decimal.atan(Utils.divide(y, x, wpr, 1));
            x = Utils.getPi(wpr, 1);
            Decimal.precision = pr;
            Decimal.rounding = rm;
            result = y.s < 0 ? result.sub(x) : result.add(x);
        } else {
            result = Decimal.atan(Utils.divide(y, x, wpr, 1));
        }

        return result;
    }

    /**
     * Return a new Decimal whose value is the square root of the sum of the squares of the arguments,
     * rounded to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value[] } n {double | string | Decimal} Decimal
     * @returns { Decimal } the Decimal type
     */
    static hypot(...n: Value[]): Decimal {
        let t = new Decimal(0);
        external = false;
        for (let i = 0; i < n.length;) {
            let v = new Decimal(n[i++]);
            if (!v.d) {
                if (v.s) {
                    external = true;
                    return new Decimal(Infinity);
                }
                t = v;
            } else if (t.d) {
                t = t.add(v.mul(v));
            }
        }
        external = true;
        return t.sqrt();
    }

    /**
     * Return a new Decimal whose value is the maximum of the arguments.
     *
     * @param { Value[] } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static max(...values: Value[]): Decimal {
        let result = new Decimal(values[0]);
        for (let i = 1; i < values.length; i++) {
            let other = new Decimal(values[i]);
            if (other.isNaN()) {
                result = other;
                break;
            }
            if (Utils.lessThan(result, other)) {
                result = other;
            }
        }
        return result;
    }

    /**
     * Return a new Decimal whose value is the minimum of the arguments.
     *
     * @param { Value[] } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static min(...values: Value[]): Decimal {
        let result = new Decimal(values[0]);
        for (let i = 1; i < values.length; i++) {
            let other = new Decimal(values[i]);
            if (other.isNaN()) {
                result = other;
                break;
            }

            if (Utils.greaterThan(result, other)) {
                result = other;
            }
        }
        return result;
    }

    /**
     * Returns a new Decimal with a random value equal to or greater than 0 and less than 1.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static random(): Decimal {
        return Utils.random();
    }

    /**
     * Returns a new Decimal with a random value equal to or greater than 0 and less than 1, and with
     * `significantDigits` significant digits (or less if trailing zeros are produced).
     *
     * @param { double } significantDigits Significant digits. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static random(significantDigits: double): Decimal {
        return Utils.random(significantDigits);
    }

    /**
     * Return the sign of the passed value to the method.
     *   1    if x > 0,
     *  -1    if x < 0,
     *   0    if x is 0,
     *  -0    if x is -0,
     *   NaN  otherwise
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sign(n: Value): double {
        let x = new Decimal(n);
        return x.d ? (x.d![0] ? x.s : 0 * x.s) : x.s || NaN;
    }

    /**
     * Return a new Decimal whose value is `n` rounded to an integer using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static round(n: Value): Decimal {
        let x = new Decimal(n)
        return Utils.finalise(x, x.e + 1, Decimal.rounding);
    }

    /**
     * Configures the 'global' settings for this particular Decimal constructor.
     *
     * @param { DecimalConfig } config object with one or more of the following properties,
     *   precision  {double}
     *   rounding   {Rounding}
     *   toExpNeg   {double}
     *   toExpPos   {double}
     *   maxE       {double}
     *   minE       {double}
     *   modulo     {Modulo}
     *   crypto     {boolean}
     *   defaults   {true}
     * @returns { void }
     * @throws { BusinessError } 10200001 - The value of `DecimalConfig.properties` is out of range.
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static set(config: DecimalConfig): void {
        let useDefaults = (config.defaults != undefined && config.defaults === true);
        Decimal.precision =
            Utils.checkRange(config.precision, 1, MAX_DIGITS, useDefaults, [DEFAULTS_PRECISION, Decimal.precision]);
        Decimal.rounding = Utils.checkRange(config.rounding, 0, 8, useDefaults, [DEFAULTS_ROUNDING, Decimal.rounding]);
        Decimal.toExpNeg =
            Utils.checkRange(config.toExpNeg, -EXP_LIMIT, 0, useDefaults, [DEFAULTS_TOEXPNEG, Decimal.toExpNeg]);
        Decimal.toExpPos =
            Utils.checkRange(config.toExpPos, 0, EXP_LIMIT, useDefaults, [DEFAULTS_TOEXPPOS, Decimal.toExpPos]);
        Decimal.maxE = Utils.checkRange(config.maxE, 0, EXP_LIMIT, useDefaults, [EXP_LIMIT, Decimal.maxE]);
        Decimal.minE = Utils.checkRange(config.minE, -EXP_LIMIT, 0, useDefaults, [-EXP_LIMIT, Decimal.minE]);
        Decimal.modulo = Utils.checkRange(config.modulo, 0, 9, useDefaults, [DEFAULTS_MODULO, Decimal.modulo]);

        if (useDefaults) {
            Decimal.crypto = false;
        }
        if (config.crypto != undefined) {
            if (config.crypto!) {
                throw Utils.createBusinessError(CRYPTO_UNAVAILABLE_ERROR_CODE, `Crypto unavailable`);
            } else {
                Decimal.crypto = false;
            }
        }
    }

    /**
     * Return a new Decimal whose value is the square root of this Decimal, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     */
    public sqrt(): Decimal {
        let x = this;
        let d = x.d;
        let s: double = x.s;

        if (s !== 1 || !d || !d![0]) {
            return new Decimal(!s || s < 0 && (!d || d![0]) ? NaN : d ? x : Infinity);
        }

        external = false;
        s = Math.sqrt(+(x.toNumber()));
        let n: string = '';
        let e: double = x.e;
        let r: Decimal;
        if (s == 0 || s == Infinity) {
            n = Utils.digitsToString(d);
            if ((n.length + e) % 2 == 0) {
                n += '0';
            }
            s = Math.sqrt(Utils.toNumber(n));
            e = Math.floor((e + 1) / 2) - (e < 0 ? 1 : e % 2 );
            if (s == Infinity) {
                n = '5e' + e;
            } else {
                n = Utils.toExponential(s);
                n = n.slice(0, n.indexOf('e') + 1) + e;
            }
            r = new Decimal(n);
        } else {
            r = new Decimal(s);
        }

        let sd: int = (e = Decimal.precision).toInt() + 3;
        let rep: int = 0;
        let m: boolean = false;
        while (true) {
            let t = r;
            r = t.add(Utils.divide(x, t, sd + 2, 1)).mul(0.5);
            if (this.isDigitsEqual(t.d!, r.d!, 0, sd)) {
                n = n.slice(sd - 3, sd + 1);
                if (n == '9999' || !rep && n == '4999') {
                    if (!rep) {
                        Utils.finalise(t, e + 1, 0);
                        if (t.mul(t).equals(x)) {
                            r = t;
                            break;
                        }
                    }
                    sd += 4;
                    rep = 1;
                } else {
                    if (!Utils.toNumber(n) || !Utils.toNumber(n.slice(1)) && (n.charAt(0) == c'5')) {
                        Utils.finalise(r, e + 1, 1);
                        m = !r.mul(r).equals(x);
                    }
                    break;
                }
            }
        }
        external = true;
        return Utils.finalise(r, e, Decimal.rounding, m);
    }

    /**
     * Return a new Decimal whose value is the cube root of the value of this Decimal, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     */
    public cbrt(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite() || x.isZero()) {
            return new Decimal(x);
        }
        external = false;

        // Initial estimate.
        let s: double = x.s * Math.pow(x.s * x.toNumber(), 1 / 3);

        // Math.cbrt underflow/overflow?
        // Pass x to Math.pow as integer, then adjust the exponent of the result.
        let n: string = '';
        let e: double = 0;
        let r: Decimal;
        if (!s || Math.abs(s) == Infinity) {
            n = Utils.digitsToString(x.d!);
            e = x.e;

            // Adjust n exponent so it is a multiple of 3 away from x exponent.
            if (s = (e - n.length + 1) % 3) {
                n += (s == 1 || s == -2 ? '0' : '00');
            }
            s = Math.pow(Utils.toNumber(n), 1 / 3);

            // Rarely, e may be one less than the result exponent value.
            e = Math.floor((e + 1) / 3) - ((e % 3 == (e < 0 ? -1 : 2)) ? 1 : 0);

            if (s == Infinity) {
                n = '5e' + e;
            } else {
                n = Utils.toExponential(s);
                n = n.slice(0, n.indexOf('e') + 1) + e;
            }

            r = new Decimal(n);
            r.sign = x.s;
        } else {
            r = new Decimal(s);
        }

        let sd: int = (e = Decimal.precision).toInt() + 3;

        // Halley's method. Compare Newton's method.
        let rep: int = 0;
        let m: boolean = false;
        while (true) {
            let t = r;
            let t3 = t.mul(t).mul(t);
            let t3plusx = t3.add(x);
            r = Utils.divide(t3plusx.add(x).mul(t), t3plusx.add(t3), sd + 2, 1);
            // Replace with for-loop and checkRoundingDigits.
            if (this.isDigitsEqual(t.d!, r.d!, 0, sd)) {
                n = Utils.digitsToString(r.d!).slice(0, sd)
                n = n.slice(sd - 3, sd + 1);
                // The 4th rounding digit may be in error by -1 so if the 4 rounding digits are 9999 or 4999
                // , i.e. approaching a rounding boundary, continue the iteration.
                if (n == '9999' || !rep && n == '4999') {
                    // On the first iteration only, check to see if rounding up gives the exact result as the
                    // nines may infinitely repeat.
                    if (!rep) {
                        Utils.finalise(t, e + 1, 0);
                        if (t.mul(t).mul(t).equals(x)) {
                            r = t;
                            break;
                        }
                    }
                    sd += 4;
                    rep = 1;
                } else {
                    // If the rounding digits are null, 0{0,4} or 50{0,3}, check for an exact result.
                    // If not, then there are further digits and m will be truthy.
                    if (!+Utils.toNumber(n) || !+Utils.toNumber(n.slice(1)) && (n.charAt(0) == c'5')) {
                        // Truncate to the first rounding digit.
                        Utils.finalise(r, e + 1, 1);
                        m = !r.mul(r).mul(r).equals(x);
                    }
                    break;
                }
            }
        }

        external = true;
        return Utils.finalise(r, e, Decimal.rounding, m);
    }

    /**
     * Return true if the value of this Decimal is equal to the value of `n`, otherwise return false.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public equals(n: Value): boolean {
        return this.comparedTo(n) === 0;
    }

    /**
     * Return
     *   1    if the value of this Decimal is greater than the value of `n`,
     *  -1    if the value of this Decimal is less than the value of `n`,
     *   0    if they have the same value,
     *   NaN  if the value of either Decimal is NaN.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { double } the number type
     */
    public comparedTo(n: Value): double {
        let y = new Decimal(n);
        let xd = this.d;
        let yd = y.d;
        let xs = this.s;
        let ys = y.s;

        if (!xd || !yd) {
            return (!xs || !ys) ? NaN : (xs !== ys) ? xs : (xd === yd) ? 0 : (!xd ^ xs < 0) ? 1 : -1;
        }

        if (!xd![0] || !yd![0]) {
            return xd![0] ? xs : yd![0] ? -ys : 0;
        }

        if (xs !== ys) {
            return xs;
        }

        if (this.e !== y.e) {
            return this.e > y.e ^ xs < 0 ? 1 : -1;
        }

        let xdL: number = xd.length;
        let ydL: number = yd.length;

        for (let i = 0, j = xdL < ydL ? xdL : ydL; i < j; ++i) {
            if (xd![i] !== yd![i]) {
                return xd![i] > yd![i] ^ xs < 0 ? 1 : -1;
            }
        }
        return xdL === ydL ? 0 : xdL > ydL ^ xs < 0 ? 1 : -1;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal divided by `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public div(n: Value): Decimal {
        return Utils.divide(this, new Decimal(n));
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal modulo `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal }the Decimal type
     */
    public mod(n: Value): Decimal {
        let y = new Decimal(n);
        if (!this.d || !y.s || y.d && !y.d![0]) {
            return new Decimal(NaN);
        }

        if (!y.d || this.d && !this.d![0]) {
            return Utils.finalise(new Decimal(this), Decimal.precision, Decimal.rounding);
        }

        external = false;
        let q: Decimal;
        if (Decimal.modulo == 9) {
            q = Utils.divide(this, y.abs(), 0, 3, true);
            q.sign *= y.s;
        } else {
            q = Utils.divide(this, y, 0, Decimal.modulo, true);
        }

        q = q.mul(y);
        external = true;
        return this.sub(q);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal raised to the power `n`, rounded
     * to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public pow(n: Value): Decimal {
        let y = new Decimal(n);
        let yn = +y.toNumber();
        if (!this.d || !y.d || !this.d![0] || !y.d![0]) {
            return new Decimal(Math.pow(+this.toNumber(), yn));
        }

        let x = new Decimal(this);
        if (x.equals(1)) {
            return x;
        }

        let pr = Decimal.precision.toInt()
        let rm = Decimal.rounding;
        if (y.equals(1)) {
            return Utils.finalise(x, pr, rm);
        }
        let e = Math.floor(y.e / LOG_BASE);
        let k: double;
        if (e >= y.d!.length - 1 && (k = yn < 0 ? -yn : yn) <= MAX_SAFE_INTEGER) {
            let r = this.intPow(x, k, pr);
            return y.s < 0 ? new Decimal(1).div(r) : Utils.finalise(r, pr, rm);
        }

        let s: double = x.s;
        if (s < 0) {
            if (e < y.d!.length - 1) {
                return new Decimal(NaN);
            }
            if ((e < y.d!.length ? y.d![e.toInt()] & 1 : 0) == 0) {
                s = 1;
            }
            if (x.e == 0 && x.d![0] == 1 && x.d!.length == 1) {
                x.sign = s;
                return x;
            }
        }

        k = Math.pow(+x.toNumber(), yn);
        e = k == 0 || !isFinite(k)
            ? Math.floor(yn * (Math.log(Utils.toNumber('0.' + Utils.digitsToString(x.d!))) / Utils.toNumber(LN10) + x.e + 1))
            : new Decimal(k + '').e;

        if (e > Decimal.maxE + 1 || e < Decimal.minE - 1) {
            return new Decimal(e > 0 ? s * Infinity : 0);
        }

        external = false;
        x.sign = 1;
        Decimal.rounding = 1;
        // Estimate the extra guard digits needed to ensure five correct rounding digits from
        // naturalLogarithm(x). Example of failure without these extra digits (precision: 10):
        // new Decimal(2.32456).pow('2087987436534566.46411')
        // should be 1.162377823e+764914905173815, but is 1.162355823e+764914905173815
        k = Math.min(12, (e + '').length);
        let r = this.naturalExponential(y.mul(this.naturalLogarithm(x, pr + k)), pr);
        if (r.d) {
            r = Utils.finalise(r, pr + 5, 1);
            if (Utils.checkRoundingDigits(r.d!, pr, rm)) {
                e = pr + 10;
                r = Utils.finalise(this.naturalExponential(y.mul(this.naturalLogarithm(x, e + k)), e), e + 5, 1);
                if (this.check14Nines(r.d!, pr)) {
                    r = Utils.finalise(r, pr + 1, 0);
                }
            }
        }
        r.sign = s;
        external = true;
        Decimal.rounding = rm;
        return Utils.finalise(r, pr, rm);
    }

    /**
     * Return a new Decimal whose value is the natural exponential of the value of this Decimal,
     * i.e. the base e raised to the power the value of this Decimal, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     */
    public exp(): Decimal {
        return this.naturalExponential(this);
    }

   /**
    * Check for 14 nines from the 2nd rounding digit, as the first may be 4.
    */
    private check14Nines(d: Array<double>, k: int) {
        return (Utils.toNumber(Utils.digitsToString(d).slice(k + 1, k + 15)) + 1 == 1e14)
    }

    /**
     * Return the logarithm of the value of this Decimal to the specified base, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public log(n: Value): Decimal {
        let base = new Decimal(n);
        let d = base.d;

        // Return NaN if base is negative, or non-finite, or is 0 or 1.
        if (base.s < 0 || !d || !d[0] || base.equals(1)) {
            return new Decimal(NaN);
        }

        let isBase10 = base.equals(10);
        d = this.d;
        // Is arg negative, non-finite, 0 or 1?
        if (this.s < 0 || !d || !d[0] || this.equals(1)) {
            return new Decimal(d && !d[0] ? -Infinity : this.s != 1 ? NaN : d ? 0 : Infinity);
        }

        // The result will have a non-terminating decimal expansion if base is 10 and arg is not an
        // integer power of 10.
        let inf: boolean = false;
        let k: double;
        if (isBase10) {
            if (d.length > 1) {
                inf = true;
            } else {
                for (k = d[0]; k % 10 === 0;) {
                    k /= 10;
                }
                inf = k !== 1;
            }
        }

        external = false;
        let guard = 5;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        let sd = pr + guard;
        let num = this.naturalLogarithm(this, sd);
        let denominator = isBase10 ? this.getLn10(sd + 10) : this.naturalLogarithm(base, sd);

        // The result will have 5 rounding digits.
        let r = Utils.divide(num, denominator, sd, 1);

        // If at a rounding boundary, i.e. the result's rounding digits are [49]9999 or [50]0000,
        // calculate 10 further digits.
        //
        // If the result is known to have an infinite decimal expansion, repeat this until it is clear
        // that the result is above or below the boundary. Otherwise, if after calculating the 10
        // further digits, the last 14 are nines, round up and assume the result is exact.
        // Also assume the result is exact if the last 14 are zero.
        //
        // Example of a result that will be incorrectly rounded:
        // log[1048576](4503599627370502) = 2.60000000000000009610279511444746...
        // The above result correctly rounded using ROUND_CEILING to 1 decimal place should be 2.7, but it
        // will be given as 2.6 as there are 15 zeros immediately after the requested decimal place, so
        // the exact result would be assumed to be 2.6, which rounded using ROUND_CEILING to 1 decimal
        // place is still 2.6.
        if (Utils.checkRoundingDigits(r.d!, k = pr, rm)) {
            do {
                sd += 10;
                num = this.naturalLogarithm(this, sd);
                denominator = isBase10 ? this.getLn10(sd + 10) : this.naturalLogarithm(base, sd);
                r = Utils.divide(num, denominator, sd, 1);

                if (!inf) {
                    if (this.check14Nines(r.d!, k.toInt())) {
                        r = Utils.finalise(r, pr + 1, 0);
                    }
                    break;
                }
            } while (Utils.checkRoundingDigits(r.d!, k += 10, rm));
        }
        external = true;

        return Utils.finalise(r, pr, rm);
    }

    /**
     * Return a new Decimal whose value is the natural logarithm of the value of this Decimal,
     * rounded to `precision` significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public ln(): Decimal {
        return this.naturalLogarithm(this);
    }

    /**
     * Return a new Decimal whose value is the cosine of the value in radians of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public cos(): Decimal {
        let x: Decimal = this;
        if (!x.d) {
            return new Decimal(NaN);
        }

        if (!x.d![0]) {
            return new Decimal(1);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + Math.max(this.e, x.precision()) + LOG_BASE;
        Decimal.rounding = 1;

        x = this.cosine(this.toLessThanHalfPi(x));
        Decimal.precision = pr;
        Decimal.rounding = rm;

        return Utils.finalise(Decimal.quadrant == 2 || Decimal.quadrant == 3 ? x.negate() : x, pr, rm, true);
    }

    /**
     * Return a new Decimal whose value is the sine of the value in radians of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public sin(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite()) {
            return new Decimal(NaN);
        }
        if (x.isZero()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + Math.max(x.e, x.precision()) + LOG_BASE;
        Decimal.rounding = 1;

        x = this.sine(this.toLessThanHalfPi(x));

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return Utils.finalise(Decimal.quadrant > 2 ? x.negate() : x, pr, rm, true);
    }

    /**
     * Return a new Decimal whose value is the tangent of the value in radians of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public tan(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite()) {
            return new Decimal(NaN);
        }
        if (x.isZero()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + 10;
        Decimal.rounding = 1;

        x = x.sin();
        x.sign = 1;
        x = Utils.divide(x, new Decimal(1).sub(x.mul(x)).sqrt(), pr + 10, 0);

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return Utils.finalise(Decimal.quadrant == 2 || Decimal.quadrant == 4 ? x.negate() : x, pr, rm, true);
    }

    /**
     * Return a new Decimal whose value is the hyperbolic cosine of the value in radians of this
     * Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public cosh(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite()) {
            return new Decimal(x.s ? Infinity : NaN);
        }
        let one = new Decimal(1);
        if (x.isZero()) {
            return one;
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + Math.max(x.e, x.precision()) + 4;
        Decimal.rounding = 1;
        let len: number = x.d!.length;

        // Argument reduction: cos(4x) = 1 - 8cos^2(x) + 8cos^4(x) + 1
        // i.e. cos(x) = 1 - cos^2(x/4)(8 - 8cos^2(x/4))

        // Estimate the optimum number of times to use the argument reduction.
        let k: double = 0;
        let n: string = '';
        if (len < 32) {
            k = Math.ceil(len / 3);
            n = Utils.toString((1 / Utils.tinyPow(4, k)));
        } else {
            k = 16;
            n = '2.3283064365386962890625e-10';
        }

        x = this.taylorSeries(1, x.mul(n), new Decimal(1), true);

        // Reverse argument reduction
        let i = k;
        let d8 = new Decimal(8);
        for (; i--;) {
            let cosh2_x = x.mul(x);
            x = one.sub(cosh2_x.mul(d8.sub(cosh2_x.mul(d8))));
        }

        return Utils.finalise(x, Decimal.precision = pr, Decimal.rounding = rm, true);
    }

    /**
     * Return a new Decimal whose value is the hyperbolic sine of the value in radians of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public sinh(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite() || x.isZero()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + Math.max(x.e, x.precision()) + 4;
        Decimal.rounding = 1;
        let len: number = x.d!.length;

        if (len < 3) {
            x = this.taylorSeries(2, x, x, true);
        } else {
            // Alternative argument reduction: sinh(3x) = sinh(x)(3 + 4sinh^2(x))
            // i.e. sinh(x) = sinh(x/3)(3 + 4sinh^2(x/3))
            // 3 multiplications and 1 addition
            // Argument reduction: sinh(5x) = sinh(x)(5 + sinh^2(x)(20 + 16sinh^2(x)))
            // i.e. sinh(x) = sinh(x/5)(5 + sinh^2(x/5)(20 + 16sinh^2(x/5)))
            // 4 multiplications and 2 additions
            // Estimate the optimum number of times to use the argument reduction.
            let k = 1.4 * Math.sqrt(len);
            k = k > 16 ? 16 : k | 0;
            x = x.mul(1 / Utils.tinyPow(5, k));
            x = this.taylorSeries(2, x, x, true);

            // Reverse argument reduction
            let d5 = new Decimal(5);
            let d16 = new Decimal(16);
            let d20 = new Decimal(20);
            for (; k--;) {
                let sinh2_x = x.mul(x);
                x = x.mul(d5.add(sinh2_x.mul(d16.mul(sinh2_x).add(d20))));
            }
        }

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return Utils.finalise(x, pr, rm, true);
    }

    /**
     * Return a new Decimal whose value is the hyperbolic tangent of the value in radians of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public tanh(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite()) {
            return new Decimal(x.s);
        }
        if (x.isZero()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + 7;
        Decimal.rounding = 1;

        return Utils.divide(x.sinh(), x.cosh(), Decimal.precision = pr, Decimal.rounding = rm);
    }

    /**
     * Return true if the value of this Decimal is greater than the value of `n`, otherwise return false.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public greaterThan(n: Value): boolean {
        return this.comparedTo(n) > 0;
    }

    /**
     * Return true if the value of this Decimal is greater than or equal to the value of `n`,
     * otherwise return false.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public greaterThanOrEqualTo(n: Value): boolean {
        let k = this.comparedTo(n);
        return k == 1 || k === 0;
    }

    /**
     * Return true if the value of this Decimal is less than `n`, otherwise return false.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public lessThan(n: Value): boolean {
        return this.comparedTo(n) < 0;
    }

    /**
     * Return true if the value of this Decimal is less than or equal to `n`, otherwise return false.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public lessThanOrEqualTo(n: Value): boolean {
        return this.comparedTo(n) <= 0;
    }

    /**
     * Return a new Decimal whose value is the arccosine (inverse cosine) in radians of the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public acos(): Decimal {
        let x: Decimal = this;
        let k = x.abs().comparedTo(1);
        let pr = Decimal.precision;
        let rm = Decimal.rounding;

        if (k != -1) {
            return k == 0
                // |x| is 1
                ? x.isNegative() ? Utils.getPi(pr, rm) : new Decimal(0)
                // |x| > 1 or x is NaN
                : new Decimal(NaN);
        }

        if (x.isZero()) {
            return Utils.getPi(pr + 4, rm).mul(0.5);
        }

        Decimal.precision = pr + 6;
        Decimal.rounding = 1;

        x = (new Decimal(1)).sub(x).div(x.add(1)).sqrt().atan();

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return x.mul(2);
    }

    /**
     * Return a new Decimal whose value is the arcsine (inverse sine) in radians of the value of this
     * Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public asin(): Decimal {
        let x: Decimal = this;
        if (x.isZero()) {
            return new Decimal(x);
        }

        let k = x.abs().comparedTo(1);
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        if (k != -1) {
            // |x| is 1
            if (k == 0) {
                let halfPi = Utils.getPi(pr + 4, rm).mul(0.5);
                halfPi.sign = x.s;
                return halfPi;
            }
            // |x| > 1 or x is NaN
            return new Decimal(NaN);
        }

        Decimal.precision = pr + 6;
        Decimal.rounding = 1;

        x = x.div(new Decimal(1).sub(x.mul(x)).sqrt().add(1)).atan();

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return x.mul(2);
    }

    /**
     * Return a new Decimal whose value is the arctangent (inverse tangent) in radians of the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public atan(): Decimal {
        let x: Decimal = this;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        let r: Decimal;
        if (!x.isFinite()) {
            if (!x.s) {
                return new Decimal(NaN);
            }
            if (pr + 4 <= PI_PRECISION) {
                r = Utils.getPi(pr + 4, rm).mul(0.5);
                r.sign = x.s;
                return r;
            }
        } else if (x.isZero()) {
            return new Decimal(x);
        } else if (x.abs().equals(1) && pr + 4 <= PI_PRECISION) {
            r = Utils.getPi(pr + 4, rm).mul(0.25);
            r.sign = x.s;
            return r;
        }

        let wpr: double = pr + 10;
        Decimal.precision = wpr;
        Decimal.rounding = 1;

        // Argument reduction
        // Ensure |x| < 0.42
        // atan(x) = 2 * atan(x / (1 + sqrt(1 + x^2)))
        let k = Math.min(28, wpr / LOG_BASE + 2 | 0);
        let i: int = 0;
        for (i = k.toInt(); i; --i) {
            x = x.div(x.mul(x).add(1).sqrt().add(1));
        }
        external = false;

        let j = Math.ceil(wpr / LOG_BASE).toInt();
        let n = 1;
        let x2 = x.mul(x);
        r = new Decimal(x);
        let px = x;

        // atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
        for (; i != -1;) {
            px = px.mul(x2);
            let t = r.sub(px.div(n += 2));
            px = px.mul(x2);
            r = t.add(px.div(n += 2));
            if (j < r.d!.length && r.d![j] != undefined) {
                for (i = j; r.d![i] == (i < t.d!.length ? t.d![i] : undefined) && i--;);
            }
        }

        if (k) {
            r = r.mul(2 << (k - 1));
        }

        external = true;

        return Utils.finalise(r, Decimal.precision = pr, Decimal.rounding = rm, true);
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic cosine in radians of the
     * value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public acosh(): Decimal {
        let x: Decimal = this;
        if (x.comparedTo(1) == -1) {
            return new Decimal(x.equals(1) ? 0 : NaN);
        }
        if (!x.isFinite()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + Math.max(Math.abs(x.e), x.precision()) + 4;
        Decimal.rounding = 1;
        external = false;

        x = x.mul(x).sub(1).sqrt().add(x);

        external = true;
        Decimal.precision = pr;
        Decimal.rounding = rm;

        return x.ln();
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic sine in radians of the value
     * of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public asinh(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite() || x.isZero()) {
            return new Decimal(x);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        Decimal.precision = pr + 2 * Math.max(Math.abs(x.e), x.precision()) + 6;
        Decimal.rounding = 1;
        external = false;

        x = x.mul(x).add(1).sqrt().add(x);

        external = true;
        Decimal.precision = pr;
        Decimal.rounding = rm;

        return x.ln();
    }

    /**
     * Return a new Decimal whose value is the inverse of the hyperbolic tangent in radians of the
     * value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200060 - Precision limit exceeded.
     */
    public atanh(): Decimal {
        let x: Decimal = this;
        if (!x.isFinite()) {
            return new Decimal(NaN);
        }
        if (x.e >= 0) {
            return new Decimal(x.abs().equals(1) ? x.s * Infinity : x.isZero() ? x : NaN);
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        let xsd = x.precision();

        if (Math.max(xsd, pr) < 2 * -x.e - 1) {
            return Utils.finalise(new Decimal(x), pr, rm, true);
        }

        let wpr = xsd - x.e;
        Decimal.precision = wpr;

        x = Utils.divide(x.add(1), new Decimal(1).sub(x), wpr + pr, 1);

        Decimal.precision = pr + 4;
        Decimal.rounding = 1;

        x = x.ln();

        Decimal.precision = pr;
        Decimal.rounding = rm;

        return x.mul(0.5);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal plus `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public add(n: Value): Decimal {
        let y = new Decimal(n);
        if (!this.d || !y.d) {
            if (!this.s || !y.s) {
                y = new Decimal(NaN);
            } else if (!this.d) {
                y = new Decimal(y.d || this.s === y.s ? this : NaN);
            }
            return y;
        }

        if (this.s != y.s) {
            y.sign = -y.s;
            return this.sub(y);
        }
        let xd = this.d!;
        let yd = y.d!;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;

        if (!xd[0] || !yd[0]) {
            if (!yd[0]) {
                y = new Decimal(this);
            }
            return external ? Utils.finalise(y, pr, rm) : y;
        }
        let k = Math.floor(this.e / LOG_BASE);
        let e = Math.floor(y.e / LOG_BASE);
        xd = xd!.slice();
        let i = (k - e).toInt();
        let len: int;
        let d: Array<double>;
        if (i) {
            if (i < 0) {
                d = xd;
                if (i == Int.MIN_VALUE) {
                    i = Int.MAX_VALUE;
                } else {
                    i = -i;
                }
                len = yd.length;
            } else {
                d = yd;
                e = k;
                len = xd.length;
            }
            k = Math.ceil(pr / LOG_BASE);
            len = (k > len ? k + 1 : len + 1).toInt();
            if (i > len) {
                i = len;
                d.length = 1;
            }
            d.reverse();
            for (; i--;) {
                d.push(0);
            }
            d.reverse();
        }
        len = xd.length;
        i = yd.length;
        if (len - i < 0) {
            i = len;
            d = yd;
            yd = xd;
            xd = d;
        }
        let carry: Long = 0;
        for (carry = 0; i;) {
            --i;
            xd[i] = xd[i] + yd[i] + carry;
            carry = (xd[i] / BASE) | 0;
            xd[i] = xd[i] % BASE;
        }

        if (carry) {
            xd.unshift(carry);
            ++e;
        }

        for (len = xd.length; xd[--len] == 0;) {
            xd.pop();
        }
        y.digits = xd;
        y.exponent = this.getBase10Exponent(xd, e);
        return external ? Utils.finalise(y, pr, rm) : y;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal minus `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public sub(n: Value): Decimal {
        let y: Decimal = new Decimal(n);
        let x = this;
        if (!x.d || !y.d) {
            if (!x.s || !y.s) {
                y = new Decimal(NaN);
            } else if (x.d) {
                y.sign = -y.s;
            } else {
                y = new Decimal(y.d || x.s !== y.s ? x : NaN);
            }
            return y;
        }

        if (x.s != y.s) {
            y.sign = -y.s;
            return x.add(y);
        }

        let xd = x.d!;
        let yd = y.d!;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        if (!xd[0] || !yd[0]) {
            if (yd[0]) {
                y.sign = -y.s;
            } else if (xd[0]) {
                y = new Decimal(x);
            } else {
                return new Decimal(rm === 3 ? -0.0 : 0);
            }
            return external ? Utils.finalise(y, pr, rm) : y;
        }

        // Calculate base 1e7 exponents.
        let yExp = Math.floor(y.e / LOG_BASE);
        let xExp = Math.floor(x.e / LOG_BASE);
        xd = xd.slice();
        let expDiff = xExp - yExp;
        let len: int;
        let i: int;
        let xLessThanY: boolean = false;
        if (expDiff) {
            xLessThanY = expDiff < 0;
            let d: Array<double>;
            if (xLessThanY) {
                d = xd;
                expDiff = -expDiff;
                len = yd.length;
            } else {
                d = yd;
                yExp = xExp;
                len = xd.length;
            }
            let i = Math.max(Math.ceil(pr / LOG_BASE), len) + 2;
            if (expDiff > i) {
                expDiff = i;
                d.length = 1;
            }
            d.reverse();
            for (i = expDiff; i--;) {
                d.push(0);
            }
            d.reverse();
        } else {
            let i = xd.length;
            len = yd.length;
            xLessThanY = i < len;
            if (xLessThanY) {
                len = i;
            }
            for (i = 0; i < len; i++) {
                if (xd[i] != yd[i]) {
                    xLessThanY = xd[i] < yd[i];
                    break;
                }
            }
            expDiff = 0;
        }

        if (xLessThanY) {
            let d = xd;
            xd = yd;
            yd = d;
            y.sign = -y.s;
        }
        len = xd.length;
        for (i = yd.length - len; i > 0; --i) {
            xd.push(0);
            len++
        }
        let j: int;
        for (i = yd.length; i > expDiff && i > 0;) {
            if (xd[--i] < yd[i]) {
                for (j = i; j && xd[--j] === 0;) {
                    xd[j] = BASE - 1;
                }
                --xd[j];
                xd[i] = xd[i] + BASE;
            }
            xd[i] -= yd[i];
        }

        for (; len > 0 && xd[--len] === 0;) {
            xd.pop();
        }

        for (; xd.length > 0 && xd[0] === 0; xd.shift()) {
            --yExp;
        }

        if (xd.length == 0 || (xd.length > 0 && !xd[0])) {
            return new Decimal(rm === 3 ? -0.0 : 0);
        }
        y.digits = xd;
        y.exponent = this.getBase10Exponent(xd, yExp);
        return external ? Utils.finalise(y, pr, rm) : y;
    }

    /**
     * Return a new Decimal whose value is this Decimal times `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { Value } n {double | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public mul(n: Value): Decimal {
        let xd = this.digits;
        let y = new Decimal(n);
        let yd = y.d;
        y.sign *= this.sign;
        if (!xd || !(xd!.at(0)) || !yd || !(yd!.at(0))) {
            return new Decimal((!y.s || xd && !(xd!.at(0)) && !yd || yd && !(yd!.at(0)) && !xd)
                ? NaN
                : (!xd || !yd) ? y.s / 0 : y.s * 0.0);
        }
        let e = Math.floor(this.exponent / LOG_BASE) + Math.floor(y.e / LOG_BASE);
        let xdL: int = xd!.length;
        let ydL: int = yd!.length;
        let r: Array<double> | null;
        let rL: int;
        if (xdL < ydL) {
          r = xd;
          xd = yd;
          yd = r;
          rL = xdL;
          xdL = ydL;
          ydL = rL;
        }
        r = new Array<double>();
        rL = xdL + ydL;
        let i: int;
        for (i = rL; i--;) {
            r.push(0);
        }

        let carry: Long = 0;
        let k: int;
        let t: double;
        for (i = ydL; --i >= 0;) {
          carry = 0;
          for (k = xdL + i; k > i;) {
            t = r[k] + yd!.at(i)! * xd!.at(k - i - 1)! + carry;
            r[k--] = t % BASE | 0;
            carry = t / BASE | 0;
          }
          r[k] = (r[k] + carry) % BASE | 0;
        }

        for (; !r[--rL];) {
            r.pop();
        }
        if (carry) {
            ++e;
        } else {
            r.shift();
        }
        y.digits = r;
        y.exponent = this.getBase10Exponent(r, e);
        return external ? Utils.finalise(y, Decimal.precision, Decimal.rounding) : y;
    }

    private initializeByDecimal(v: Decimal) {
        this.sign = v.s;
        if (external) {
            if (!v.d || v.e > Decimal.maxE) {
                this.exponent = NaN;
                this.digits = null;
            } else if (v.e < Decimal.minE) {
                this.exponent = 0;
                this.digits = Utils.updateDigits(0);
            } else {
                this.exponent = v.e;
                this.digits = v.d!.slice();
            }
        } else {
            this.exponent = v.e;
            this.digits = v.d ? v.d!.slice() : v.d;
        }
    }

    private initializeByNumber(v: double) {
        if (v == 0) {
            this.sign = 1 / v < 0 ? -1 : 1;
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
            return;
        }
        let tv = v;
        if (v < 0) {
            tv = -v;
            this.sign = -1;
        } else {
            this.sign = 1;
        }

        if (tv === ~~tv && tv < 1e7) {
            let e: double;
            let i: double;
            for (e = 0, i = tv; i >= 10; i /= 10) {
                e++;
            }
            if (external) {
                if (e > Decimal.maxE) {
                  this.exponent = NaN;
                  this.digits = null;
                } else if (e < Decimal.minE) {
                    this.exponent = 0;
                    this.digits = Utils.updateDigits(0);
                } else {
                    this.exponent = e;
                    this.digits = Utils.updateDigits(tv);
                }
            } else {
                this.exponent = e;
                this.digits = Utils.updateDigits(tv);
            }
            return;
        } else if (tv * 0 !== 0) {
            if (!tv) {
                this.sign = NaN;
            }
            this.exponent = NaN;
            this.digits = null;
            return;
        }
        this.parseDecimal(Utils.toString(tv));
    }

    private initializeByString(v: string) {
        let i: int;
        let str: string = v;
        if ((i = v.charCodeAt(0).toInt()) === 45) {
            str = v.slice(1);
            this.sign = -1;
        } else {
            if (i === 43) {
                str = v.slice(1);
            }
            this.sign = 1;
        }
        if (isDecimal.test(str)) {
            this.parseDecimal(str);
        } else {
            this.parseOther(str);
        }
    }

    private parseDecimal(str: String): void {
        let e: double;
        let i: int;
        let len: int;
        if ((e = str.indexOf('.')) > -1) {
            str = str.replace('.', '');
        }

        if ((i = str.search(new RegExp("e", "i"))) > 0) {
            if (e < 0) {
                e = i;
            }
            e += Utils.toNumber(str.slice(i + 1));
            str = str.substring(0, i);
        } else if (e < 0) {
            e = str.length;
        }

        for (i = 0; i < str.length && str.charCodeAt(i) === 48; i++);

        for (len = str.length; len > 0 && str.charCodeAt(len - 1) === 48; --len);
        str = str.slice(i, len);
        if (str) {
            len -= i;
            this.exponent = e = e - i - 1;
            this.digits = new Array<double>();
            i = ((e + 1) % LOG_BASE).toInt()
            if (e < 0) {
                i += LOG_BASE;
            }
            if (i < len) {
                if (i) {
                    this.digits!.push(Utils.toNumber(str.slice(0, i)));
                }
                for (len -= LOG_BASE; i < len;) {
                    this.digits!.push(Utils.toNumber(str.slice(i, i += LOG_BASE)));
                }
                str = str.slice(i);
                i = LOG_BASE - str.length;
            } else {
                i -= len;
            }

            for (; i--;) {
                str += '0';
            }
            this.digits!.push(Utils.toNumber(str));
            if (external) {
                if (this.exponent > Decimal.maxE) {
                    this.digits = null;
                    this.exponent = NaN;
                } else if (this.exponent < Decimal.minE) {
                    this.exponent = 0;
                    this.digits = Utils.updateDigits(0);
                }
            }
        } else {
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
        }
        return;
    }

    private parseOther(str: string): void {
        if (str.indexOf('_') > -1) {
            str = str.replace(new RegExp("(\\d)_(?=\\d)", "g"), '$1');
            if (isDecimal.test(str)) {
                this.parseDecimal(str);
                return;
            }
        } else if (str === 'Infinity' || str === 'NaN') {
            if (!Utils.toNumber(str)) {
                this.sign = NaN;
            }
            this.exponent = NaN;
            this.digits = null;
            return;
        }

        let base: int;
        if (isHex.test(str))  {
            base = 16;
            str = str.toLowerCase();
        } else if (isBinary.test(str))  {
            base = 2;
        } else if (isOctal.test(str))  {
            base = 8;
        }  else {
            throw Utils.createBusinessError(TYPE_ERROR_CODE,
                `The type of "test(str)" must be Hex/Binary/Octal. Received value is: ${str}`);
        }

        let i = str.search(new RegExp("p", "i")).toInt();
        let p: double = 0;
        if (i > 0) {
            p = Utils.toNumber(str.slice(i + 1));
            str = str.substring(2, i);
        } else {
            str = str.slice(2);
        }

        i = str.indexOf('.').toInt();
        let isFloat: boolean = i >= 0;
        let len: int = 0;
        let divisor: Decimal = new Decimal(0);
        if (isFloat) {
            str = str.replace('.', '');
            len = str.length.toInt();
            i = len - i;
            divisor = this.intPow(new Decimal(base), i, i * 2);
        }
        let xd = this.convertBase(str, base, BASE);
        let xe = xd.length - 1;
        for (i = xe; i >= 0 && xd[i] === 0; --i) {
            xd.pop();
        }
        if (i < 0) {
            this.sign = this.sign < 0 ? -1 : 1;
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
            return;
        }
        this.exponent = this.getBase10Exponent(xd, xe);
        this.digits = xd;
        external = false;
        if (isFloat) {
            let x = Utils.divide(this, divisor, len * 4);
            this.initializeByDecimal(x);
        }

        if (p) {
            let x = this.mul(Math.abs(p) < 54 ? Math.pow(2, p) : Utils.pow(2, p));
            this.initializeByDecimal(x)
        }
        external = true;
        return;
    }

    private intPow(x: Decimal, n: double, pr: double): Decimal {
        let r = new Decimal(1);
        let k = Math.ceil(pr / LOG_BASE + 4);
        external = false;
        let isTruncated: boolean = false;
        while (true) {
            if (n % 2) {
                r = r.mul(x);
                if (this.truncate(r.d!, k)) {
                    isTruncated = true;
                }
            }

            n = Math.floor(n / 2);
            if (n === 0) {
                n = r.d!.length - 1;
                if (isTruncated && r.d![n.toInt()] === 0) {
                    ++r.d![n.toInt()];
                }
                break;
            }
            x = x.mul(x);
            this.truncate(x.d!, k);
        }
        external = true;
        return r;
    }

    private getBase10Exponent(digits: Array<double>, e: double): double {
        let w = digits[0];
        for (e *= LOG_BASE; w >= 10; w /= 10) {
            e++;
        }
        return e;
    }

    private truncate(arr: Array<double>, len: double): boolean {
        if (arr.length > len) {
            arr.length = len.toInt();
            return true;
        }
        return false;
    }

    private convertBase(str: string, baseIn: int, baseOut: double): Array<double> {
        let arrL: int;
        let arr: Array<double> = Utils.updateDigits(0);
        for (let i = 0; i < str.length;) {
            for (arrL = arr.length; arrL--;) {
               arr[arrL] = arr[arrL] * baseIn;
            }
            arr[0] = arr[0] + NUMERALS.indexOf(str.charAt(i++));
            for (let j = 0; j < arr.length; j++) {
               if (arr[j] > baseOut - 1) {
                    if ((j + 1) == arr.length) {
                        arr.push(0);
                    }
                    arr[j + 1] = arr[j + 1] + (arr[j] / baseOut | 0);
                    arr[j] = arr[j] % baseOut;
                }
            }
        }
        return arr.reverse();
    }

    private finiteToString(isExp: boolean, sd?: double): string {
        if (!this.isFinite()) {
            return this.nonFiniteToString();
        }
        let k: int;
        let e = this.exponent;
        let str: string = Utils.digitsToString(this.d!);
        let len: int = str.length;
        const kIfSd = (sd != undefined) ? (sd! - len).toInt() : 0
        if (isExp) {
          if (sd && (k = kIfSd) > 0) {
            str = str.charAt(0) + '.' + str.slice(1) + Utils.getZeroString(k);
          } else if (len > 1) {
            str = str.charAt(0) + '.' + str.slice(1);
          }
          str = str + (this.e < 0 ? 'e' : 'e+') + this.e;
        } else if (e < 0) {
          str = '0.' + Utils.getZeroString(-e - 1) + str;
          if (sd && (k = kIfSd) > 0) {
            str += Utils.getZeroString(k);
          }
        } else if (e >= len) {
          str += Utils.getZeroString(e + 1 - len);
          if (sd && (k = (sd - e - 1).toInt()) > 0) {
            str = str + '.' + Utils.getZeroString(k);
          }
        } else {
          if ((k = (e + 1).toInt()) < len) {
            str = str.slice(0, k) + '.' + str.slice(k);
          }
          if (sd && (k = kIfSd) > 0) {
            if (e + 1 === len) str += '.';
            str += Utils.getZeroString(k);
          }
        }
        return str;
    }

    private nonFiniteToString(): String {
        return String(Number.isNaN(this.sign) ? NaN : this.sign * this.sign * Infinity);
    }

    private isDigitsEqual(digits1: Array<double>, digits2: Array<double>, start: int, end: int) {
        return (Utils.digitsToString(digits1).slice(start, end) === Utils.digitsToString(digits2).slice(start, end))
    }

    private naturalExponential(x: Decimal, sd?: double): Decimal {
        let rm = Decimal.rounding;
        let pr = Decimal.precision;
        if (!x.d || !x.d![0] || x.e > 17) {
            return new Decimal(x.d
                ? !x.d![0] ? 1 as double : x.s < 0 ? 0 as double : Infinity
                : x.s ? x.s < 0 ? 0 as double : x : NaN);
        }

        let wpr: double;
        if (sd == undefined) {
            external = false;
            wpr = pr;
        } else {
            wpr = sd;
        }
        let t = new Decimal(0.03125);
        let k: double = 0;
        while (x.e > -2) {
            x = x.mul(t);
            k += 5;
        }

        // Use 2 * log10(2^k) + 5 (empirically derived) to estimate the increase in precision
        // necessary to ensure the first 4 rounding digits are correct.
        let guard = Math.log(Math.pow(2, k)) / Utils.toNumber(LN10) * 2 + 5 | 0;
        wpr += guard;
        let denominator = new Decimal(1);
        let pow = new Decimal(1);
        let sum = new Decimal(1);
        Decimal.precision = wpr;
        let i = 0;
        let rep: int = 0;
        while (true) {
            pow = Utils.finalise(pow.mul(x), wpr, 1);
            denominator = denominator.mul(++i);
            t = sum.add(Utils.divide(pow, denominator, wpr, 1));
            if (this.isDigitsEqual(t.d!, sum.d!, 0, wpr.toInt())) {
                let j = k;
                while (j--) {
                    sum = Utils.finalise(sum.mul(sum), wpr, 1);
                }

                if (sd == undefined) {
                    if (rep < 3 && Utils.checkRoundingDigits(sum.d!, wpr - guard, rm, rep)) {
                        Decimal.precision = wpr += 10;
                        denominator = pow = t = new Decimal(1);
                        i = 0;
                        rep++;
                    } else {
                        return Utils.finalise(sum, Decimal.precision = pr, rm, external = true);
                    }
                } else {
                    Decimal.precision = pr;
                    return sum;
                }
            }
            sum = t;
        }
    }

    private naturalLogarithm(y: Decimal, sd?: double): Decimal {
        let x = y;
        let xd = x.d;
        let rm = Decimal.rounding;
        if (x.s < 0 || !xd || !xd![0] || !x.e && xd![0] == 1 && xd!.length == 1) {
            return new Decimal(xd && !xd![0] ? -Infinity : x.s != 1 ? NaN : xd ? 0 as double : x);
        }
        let wpr: double;
        let pr = Decimal.precision;
        if (sd == undefined) {
            external = false;
            wpr = pr;
        } else {
            wpr = sd;
        }
        let guard: int = 10;
        Decimal.precision = wpr += guard;
        let c = Utils.digitsToString(xd!);
        let ch = new Char(c.charAt(0));
        let c0 = Utils.toNumber(ch.toString());
        let e: double = 0;
        let n: double = 1;
        if (Math.abs(e = x.e) < 1.5e15) {
            while (c0 < 7 && c0 != 1 ||
                c0 == 1 && (c.length > 1 ? Utils.toNumber((new Char(c.charAt(1))).toString()) : NaN) > 3) {
                x = x.mul(y);
                c = Utils.digitsToString(x.d!);
                c0 = Utils.toNumber((new Char(c.charAt(0))).toString());
                n++;
            }
            e = x.e;
            if (c0 > 1) {
                x = new Decimal('0.' + c);
                e++;
            } else {
                x = new Decimal(c0 + '.' + c.slice(1));
            }
        } else {
            let t = this.getLn10(wpr + 2, pr).mul(e + '');
            x = this.naturalLogarithm(new Decimal(c0 + '.' + c.slice(1)), wpr - guard).add(t);
            Decimal.precision = pr;
            return sd == undefined ? Utils.finalise(x, pr, rm, external = true) : x;
        }
        let x1 = x;
        let numerator: Decimal;
        let sum = numerator = x = Utils.divide(x.sub(1), x.add(1), wpr, 1);
        let x2 = Utils.finalise(x.mul(x), wpr, 1);
        let denominator: double = 3;
        let rep: int = 0;
        while (true) {
            numerator = Utils.finalise(numerator.mul(x2), wpr, 1);
            let t = sum.add(Utils.divide(numerator, new Decimal(denominator), wpr, 1));
            if (this.isDigitsEqual(t.d!, sum.d!, 0, wpr.toInt())) {
                sum = sum.mul(2);
                if (e !== 0) {
                    sum = sum.add(this.getLn10(wpr + 2, pr).mul(e + ''));
                }
                sum = Utils.divide(sum, new Decimal(n), wpr, 1);
                if (sd == undefined) {
                    if (Utils.checkRoundingDigits(sum.d!, wpr - guard, rm, rep)) {
                        Decimal.precision = wpr += guard;
                        t = numerator = x = Utils.divide(x1.sub(1), x1.add(1), wpr, 1);
                        x2 = Utils.finalise(x.mul(x), wpr, 1);
                        denominator = rep = 1;
                    } else {
                        return Utils.finalise(sum, Decimal.precision = pr, rm, external = true);
                    }
                } else {
                    Decimal.precision = pr;
                    return sum;
                }
          }
          sum = t;
          denominator += 2;
        }
    }

    private getLn10(sd: double, pr?: double): Decimal {
        if (sd > LN10_PRECISION) {
            external = true;
            if (pr != undefined) {
                Decimal.precision = pr;
            }
            throw Utils.createBusinessError(PRECISION_LIMIT_EXCEEDED_ERROR_CODE,
                `Precision limit exceeded, "sd" must be <= LN10_PRECISION`);
        }
        return Utils.finalise(new Decimal(LN10), sd, 1, true);
    }

    private toLessThanHalfPi(x: Decimal): Decimal {
        let isNeg = x.s < 0;
        let pi = Utils.getPi(Decimal.precision, 1);
        let halfPi = pi.mul(0.5);
        x = x.abs();
        if (x.comparedTo(halfPi) < 1) {
            Decimal.quadrant = isNeg ? 4 : 1;
            return x;
        }
        let t = x.dividedToIntegerBy(pi);
        if (t.isZero()) {
            Decimal.quadrant = isNeg ? 3 : 2;
        } else {
            x = x.sub(t.mul(pi));
            // 0 <= x < pi
            if (x.comparedTo(halfPi) < 1) {
                Decimal.quadrant = Utils.isOdd(t) ? (isNeg ? 2 : 3) : (isNeg ? 4 : 1);
                return x;
            }
            Decimal.quadrant = Utils.isOdd(t) ? (isNeg ? 1 : 4) : (isNeg ? 3 : 2);
        }
        return x.sub(pi).abs();
    }

    private cosine(x: Decimal): Decimal {
        if (x.isZero()) {
            return x;
        }

        // Argument reduction: cos(4x) = 8*(cos^4(x) - cos^2(x)) + 1
        // i.e. cos(x) = 8*(cos^4(x/4) - cos^2(x/4)) + 1
        // Estimate the optimum number of times to use the argument reduction.
        let len: double = x.d!.length;
        let k: double = 0;
        let y: string = '';
        if (len < 32) {
            k = Math.ceil(len / 3);
            y = Utils.toString((1 / Utils.tinyPow(4, k)));
        } else {
            k = 16;
            y = '2.3283064365386962890625e-10';
        }

        Decimal.precision += k;

        x = this.taylorSeries(1, x.mul(y), new Decimal(1));

        // Reverse argument reduction
        for (let i = k; i--;) {
            let cos2x = x.mul(x);
            x = cos2x.mul(cos2x).sub(cos2x).mul(8).add(1);
        }
        Decimal.precision -= k;
        return x;
    }

    private sine(x: Decimal): Decimal {
        let len: number = x.d!.length;
        if (len < 3) {
          return x.isZero() ? x : this.taylorSeries(2, x, x);
        }

        // Argument reduction: sin(5x) = 16*sin^5(x) - 20*sin^3(x) + 5*sin(x)
        // i.e. sin(x) = 16*sin^5(x/5) - 20*sin^3(x/5) + 5*sin(x/5)
        // and  sin(x) = sin(x/5)(5 + sin^2(x/5)(16sin^2(x/5) - 20))
        // Estimate the optimum number of times to use the argument reduction.
        let k = 1.4 * Math.sqrt(len);
        k = k > 16 ? 16 : k | 0;

        x = x.mul(1 / Utils.tinyPow(5, k));
        x = this.taylorSeries(2, x, x);

        // Reverse argument reduction
        let d5 = new Decimal(5);
        let d16 = new Decimal(16);
        let d20 = new Decimal(20);
        for (; k--;) {
          let sin2_x = x.mul(x);
          x = x.mul(d5.add(sin2_x.mul(d16.mul(sin2_x).sub(d20))));
        }

        return x;
    }

    private taylorSeries(n: int, x: Decimal, y: Decimal, isHyperbolic?: boolean): Decimal {
        let pr = Decimal.precision;
        let k = Math.ceil(pr / LOG_BASE).toInt();

        external = false;
        let x2 = x.mul(x);
        let u = new Decimal(y);
        let i: int = 1;
        let t: Decimal = new Decimal(0);
        let j: int = 0;
        while(true) {
            t = Utils.divide(u.mul(x2), new Decimal(n++ * n++), pr, 1);
            u = isHyperbolic ? y.add(t) : y.sub(t);
            y = Utils.divide(t.mul(x2), new Decimal(n++ * n++), pr, 1);
            t = u.add(y);
            if (k < t.d!.length && t.d![k] != undefined) {
                for (j = k; (j >= 0 && t.d![j] == (j < u.d!.length ? u.d![j] : undefined)) && j--;);
                if (j == -1) {
                    break;
                }
            }
            let t1 = u;
            u = y;
            y = t;
            t = t1;
            i++;
        }
        external = true;
        t.d!.length = k + 1;
        return t;
    }

    private fraction(maxD?: Value): Decimal[] {
        let x: Decimal = this;
        let xd = x.d;
        if (!xd) {
            return [new Decimal(x)];
        }

        let d0 = new Decimal(1);
        let n0 = new Decimal(0);
        let n1 = d0;
        let d1 = n0;

        let d = new Decimal(d1);
        let e = d.exponent = Utils.getPrecision(xd!) - x.e - 1;
        let k = e % LOG_BASE;
        d.d![0] = Math.pow(10, k < 0 ? LOG_BASE + k : k);

        let n: Decimal;
        if (maxD == undefined) {
            // d is 10**e, the minimum max-denominator needed.
            maxD = e > 0 ? d : n1;
        } else {
            n = new Decimal(maxD);
            if (!n.isInteger() || n.comparedTo(n1) == -1) {
                throw Utils.createBusinessError(TYPE_ERROR_CODE,
                    `The type of "Ctor(maxD)" must be Integer. Received value is: ${n}`);
            }
            maxD = n.comparedTo(d) == 1 ? (e > 0 ? d : n1) : n;
        }

        external = false;
        n = new Decimal(Utils.digitsToString(xd!));
        let pr = Decimal.precision;
        Decimal.precision = e = xd.length * LOG_BASE * 2;
        let d2: Decimal;
        while(true)  {
            let q = Utils.divide(n, d, 0, 1, true);
            d2 = d0.add(q.mul(d1));
            if (d2.comparedTo(maxD) == 1) {
                break;
            }
            d0 = d1;
            d1 = d2;
            d2 = n1;
            n1 = n0.add(q.mul(d2));
            n0 = d2;
            d2 = d;
            d = n.sub(q.mul(d2));
            n = d2;
        }

        d2 = Utils.divide(maxD.sub(d0), d1, 0, 1, true);
        n0 = n0.add(d2.mul(n1));
        d0 = d0.add(d2.mul(d1));
        n0.sign = n1.sign = x.s;

        // Determine which fraction is closer to x, n0/d0 or n1/d1?
        let r = Utils.divide(n1, d1, e, 1).sub(x).abs().comparedTo(Utils.divide(n0, d0, e, 1).sub(x).abs()) < 1
            ? [n1, d1] : [n0, d0];

        Decimal.precision = pr;
        external = true;

        return r;
    }

    private getDigitsValidIndex(digits: Array<double>): int {
        let len: int = 0;
        for (len = digits.length; !digits[len - 1]; --len);
        return len;
    }

    private digitsToString(digits: Array<double>, start: int, orgStr: string, length?: int): string {
        let len: int;
        let str: string = orgStr;
        if (length != undefined) {
            len = length;
        } else {
            len = this.getDigitsValidIndex(digits);
        }

        for (let i = start; i < len; i++) {
            str += NUMERALS.charAt(digits[i].toInt());
        }
        return str;
    }

    private completeStringWithZero(isExp: boolean, len: double, baseOut: int,
        orgStr: string, base: int, exp: double): string {
        let str = orgStr;
        if (isExp) {
            if (len > 1) {
                if (baseOut == 16 || baseOut == 8) {
                    let i = baseOut == 16 ? 4 : 3;
                    for (let j = len - 1; j % i; j++) {
                        str += '0';
                    }
                    let xd = this.convertBase(str, base, baseOut);
                    str = this.digitsToString(xd, 1, '1.');
                } else {
                    str = str.charAt(0) + '.' + str.slice(1);
                }
            }
            str =  str + (exp < 0 ? 'p' : 'p+') + exp;
        } else if (exp < 0) {
            while (++exp != 0) {
                str = '0' + str;
            }
            str = '0.' + str;
        } else {
            if (++exp > len) {
                for (exp -= len; exp != 0; exp--) {
                    str += '0';
                }
            } else if (exp < len) {
                str = str.slice(0, exp.toInt()) + '.' + str.slice(exp.toInt())
            }
        }
        return str;
    }

    private toStringBinary(baseOut: int, sd?: double, rm?: int) {
        let isExp = (sd != undefined);
        let significantDigits: int;
        if (isExp) {
            significantDigits = sd!.toInt()
            Utils.checkInt32(sd!, 1, MAX_DIGITS);
            if (rm == undefined) {
                rm = Decimal.rounding;
            } else {
                Utils.checkInt32(rm!, 0, 8);
            }
        } else {
            significantDigits = Decimal.precision.toInt();
            rm = Decimal.rounding;
        }

        if (!this.isFinite()) {
            let str = this.nonFiniteToString();
            return this.sign < 0 ? '-' + str : str;
        } else {
            let str = this.finiteToString(false);
            let i: number = str.indexOf('.');
            let base: int = baseOut;
            if (isExp) {
                base = 2;
                if (baseOut == 16) {
                    significantDigits = significantDigits * 4 - 3;
                } else if (baseOut == 8) {
                    significantDigits = significantDigits * 3 - 2;
                }
            }

            let x: Decimal = new Decimal(this);
            let y: Decimal = new Decimal(1);
            if (i >= 0) {
                str = str.replace('.', '');
                y.exponent = str.length - i;
                y.digits = this.convertBase(y.finiteToString(false), 10, base);
                y.exponent = y.d!.length;
            }
            let xd = this.convertBase(str, 10, base);
            let e: number = xd.length;
            let len: int = xd.length;
            for (; len >= 1 && xd[--len] == 0;) {
                xd.pop();
            }

            let roundUp: boolean = false;
            if (xd.length == 0 || !xd[0]) {
                str = isExp ? '0p+0' : '0';
            } else {
                if (i < 0) {
                    e--;
                } else {
                    x.digits = xd;
                    x.exponent = e;
                    x = Utils.divide(x, y, significantDigits, rm, false, base);
                    xd = x.d!;
                    e = x.e;
                    roundUp = inexact;
                }

                i = significantDigits < xd.length ? xd[significantDigits] : -1;
                let k = base / 2;
                roundUp = roundUp || (significantDigits + 1 < xd.length ? xd[significantDigits + 1] != undefined : false);
                roundUp = rm != undefined && rm < 4
                    ? roundUp && (rm === 0 || rm === (x.s < 0 ? 3 : 2))
                    : i > k || i === k && (rm === 4 || roundUp || rm === 6 && (xd[significantDigits - 1] & 1) > 0 ||
                        rm === (x.s < 0 ? 8 : 7));

                // ArkTs1.2's Array can't change length to bigger or negative.
                if (significantDigits <= xd.length) {
                    xd.length = significantDigits;
                } else {
                    xd.extendTo(significantDigits, 0);
                }

                if (roundUp) {
                    significantDigits--;
                    xd[significantDigits] += 1
                    for (; xd[significantDigits] > (base - 1);) {
                        xd[significantDigits] = 0;
                        if (!significantDigits) {
                            ++e;
                            xd.unshift(1);
                        }
                        significantDigits--;
                        if (significantDigits < 0) {
                            break;
                        }
                        xd[significantDigits] += 1
                    }
                }

                len = this.getDigitsValidIndex(xd);
                str = this.digitsToString(xd, 0, '', len);
                str = this.completeStringWithZero(isExp, len, baseOut, str, base, e);
            }
            str = (baseOut == 16 ? '0x' : baseOut == 2 ? '0b' : baseOut == 8 ? '0o' : '') + str;
            return x.s < 0 ? '-' + str : str;
        }
    }
}

class Utils {
    public static isDecimalInstance(obj: Object): boolean {
        return obj instanceof Decimal
            || (obj.hasOwnProperty('toStringTag') && (obj as Decimal).toStringTag === tag) || false;
    }

    public static getZeroString(k: double): string {
        let zs: string = '';
        for (; k--;) {
            zs += '0';
        }
        return zs;
    }

    public static digitsToString(d: Array<double>): string {
        let indexOfLastWord: double = d.length - 1;
        let str: string = '';
        let w: double = d[0];
        if (indexOfLastWord > 0) {
            str += w;
            let i: int;
            let k: double;
            let ws: string;
            for (i = 1; i < indexOfLastWord; i++) {
                ws = d[i] + '';
                k = LOG_BASE - ws.length;
                if (k) {
                    str += Utils.getZeroString(k);
                }
                str += ws;
            }
            w = d[i];
            ws = w + '';
            k = LOG_BASE - ws.length;
            if (k) {
                str += Utils.getZeroString(k);
            }
        } else if (w === 0) {
            return '0';
        }
        for (; w % 10 === 0;) {
            w /= 10;
        }
        return str + w;
    }

    public static checkInt32(i: double, min: int, max: double): void {
        if (i !== ~~i || i < min || i > max) {
            throw Utils.createBusinessError(RANGE_ERROR_CODE,
                `The value of "${i}" is out of range. It must be >= ${min} && <= ${max} . Received value is: ${i}`);
        }
    }

    public static multiplyInteger(x: Array<double>, k: double, base: double): Array<double> {
        let temp: double = 0;
        let carry: Long = 0;
        let i: int = x.length;
        for (x = x.slice(); i--;) {
            temp = x[i] * k + carry;
            x[i] = temp % base | 0;
            carry = temp / base | 0;
        }
        if (carry) {
            x.unshift(carry);
        }
        return x;
    }

    public static compare(a: Array<double>, b: Array<double>, aL: double, bL: double): double {
        let r: int = 0;
        if (aL != bL) {
            r = aL > bL ? 1 : -1;
        } else {
          for (let i = r = 0; i < aL; i++) {
            if (a[i] != b[i]) {
                r = a[i] > b[i] ? 1 : -1;
                break;
            }
          }
        }
        return r;
    }

    public static subtract(a: Array<double>, b: Array<double>, aL: double, base: double): void {
        let i: int = 0;
        let j: int = aL.toInt();
        for (; j--;) {
            a[j] = a[j] - i;
            i = a[j] < b[j] ? 1 : 0;
            a[j] = i * base + a[j] - b[j];
        }
        for (; !a[0] && a.length > 1;) {
            a.shift();
        }
    }

    public static pow(x: double, y: double): Decimal {
        return new Decimal(x).pow(y);
    }

    /**
     * Check 5 rounding digits if `repeating` is undefined, 4 otherwise.
     * `repeating == undefined` if caller is `log` or `pow`,
     * `repeating != undefined` if caller is `naturalLogarithm` or `naturalExponential`.
     */
    public static checkRoundingDigits(d: Array<double>, i: double, rm: int, repeating?: int): boolean {
        let k: double;
        // Get the length of the first word of the array d.
        for (k = d[0]; k >= 10; k /= 10) {
            --i;
        }
        let di: int;
        // Is the rounding digit in the first word of d?
        if (--i < 0) {
            i += LOG_BASE;
            di = 0;
        } else {
            di = Math.ceil((i + 1) / LOG_BASE).toInt();
            i %= LOG_BASE;
        }
        // i is the index (0 - 6) of the rounding digit.
        // E.g. if within the word 3487563 the first rounding digit is 5,
        // then i = 4, k = 1000, rd = 3487563 % 1000 = 563
        k = Math.pow(10, LOG_BASE - i);
        let rd = di < d.length ? d[di] % k | 0 : 0;
        let r: boolean = false;
        if (repeating == undefined) {
            if (i < 3) {
                if (i == 0) {
                    rd = rd / 100 | 0;
                } else if (i == 1) {
                    rd = rd / 10 | 0;
                }
                // Is the rounding digits are 0, 99999, 49999 or 50000
                r = rm < 4 && rd == 99999 || rm > 3 && rd == 49999 || rd == 50000 || rd == 0;
            } else {
                r = (rm < 4 && rd + 1 == k || rm > 3 && rd + 1 == k / 2) &&
                    ((di + 1 < d.length) ? (d[di + 1] / k / 100 | 0) : 0) == Math.pow(10, i - 2) - 1 ||
                    (rd == k / 2 || rd == 0) && ((di + 1 < d.length) ? (d[di + 1] / k / 100 | 0) : 0) == 0;
            }
        } else {
            if (i < 4) {
                if (i == 0) {
                    rd = rd / 1000 | 0;
                } else if (i == 1) {
                    rd = rd / 100 | 0;
                } else if (i == 2) {
                    rd = rd / 10 | 0;
                }
                r = (repeating > 0 || rm < 4) && rd == 9999 || !repeating && rm > 3 && rd == 4999;
            } else {
                r = ((repeating > 0 || rm < 4) && rd + 1 == k ||
                    (!repeating && rm > 3) && rd + 1 == k / 2) &&
                    (d[di + 1] / k / 1000 | 0) == Math.pow(10, i - 3) - 1;
            }
        }
        return r;
    }

    public static updateDigits(v?: double): Array<double> {
        let res = new Array<double>();
        if (v != undefined) {
            res.push(v!);
        }
        return res;
    }

    public static toNumber(s: string): double {
        return +Number.parseFloat(s);
    }

    public static toExponential(n: double): String {
        return new Double(n).toExponential();
    }

    public static toString(n: double): String {
        return new String(n);
    }

    public static checkRange(v: double | undefined, min: double, max: double,
        useDef: boolean, def: FixedArray<double>): double {
        if (v == undefined && useDef) {
            return def[0];
        }
        if (v != undefined) {
            if (Math.floor(v) === v && v >= min && v <= max) {
                return v!;
            } else {
                throw Utils.createBusinessError(RANGE_ERROR_CODE,
                    `The value of "${v!}" is out of range. It must be >= ${min} && <= ${max}. Received value is: ${v!}`
                );
            }
        }
        return def[1];
    }

    public static checkRange(v: int | undefined, min: double, max: double, useDef: boolean, def: FixedArray<int>): int {
        if (v == undefined && useDef) {
            return def[0];
        }
        if (v != undefined) {
            if (Math.floor(v) === v && v >= min && v <= max) {
                return v!;
            } else {
                throw Utils.createBusinessError(RANGE_ERROR_CODE,
                    `The value of "${v!}" is out of range. It must be >= ${min} && <= ${max}. Received value is: ${v!}`
                );
            }
        }
        return def[1];
    }

    public static getPrecision(digits: Array<double>) {
        let w: double = digits.length - 1;
        let len = w * LOG_BASE + 1;

        w = digits[w.toInt()];
        if (w) {
            for (; w % 10 == 0; w /= 10) {
                len--;
            }

            for (w = digits[0]; w >= 10; w /= 10) {
                len++;
            }
        }
        return len;
    }

    public static isOdd(n: Decimal): boolean {
        return (n.d![n.d!.length - 1] & 1) > 0;
    }

    public static tinyPow(b: int, e: double): double {
        let n = b;
        while (--e) {
            n *= b;
        }
        return n;
    }

    public static getPi(sd: double, rm: int): Decimal {
        if (sd > PI_PRECISION) {
            throw Utils.createBusinessError(PRECISION_LIMIT_EXCEEDED_ERROR_CODE,
                `Precision limit exceeded, "sd" must be <= PI_PRECISION`);
        }
        return Utils.finalise(new Decimal(PI), sd, rm, true);
    }

    /**
     * Round `x` to `sd` significant digits using rounding mode `rm`.
     * Check for over/under-flow.
     */
    public static finalise(x: Decimal, sd: double | undefined, rm: int | undefined,
        isTruncated: boolean = false): Decimal {
        // Don't round if sd is undefined.
        if (sd != undefined) {
            let xd = x.d;
            // Infinity/NaN.
            if (!xd) {
                return x;
            }

            // digits: the number of digits of w.
            let digits: double;
            let k: double;
            for (digits = 1, k = xd![0]; k >= 10; k /= 10) {
                digits++;
            }
            let signDigits: double = sd!;
            // i: what would be the index of rd within w if all the numbers were 7 digits long (i.e. if
            // they had leading zeros)
            let i = sd - digits;
            // j: if > 0, the actual index of rd within w (if < 0, rd is a leading zero).
            let j: double;
            // w: the word of xd containing rd, a base 1e7 number.
            let w: double;
            // xdi: the index of w within xd.
            let xdi: int = 0;
            // rd: the rounding digit, i.e. the digit after the digit that may be rounded up.
            let rd: double;
            if (i < 0) {
                i += LOG_BASE;
                j = signDigits;
                w = xd[xdi];
                // Get the rounding digit at index j of w.
                rd = w / Math.pow(10, digits - j - 1) % 10 | 0;
            } else {
                xdi = Math.ceil((i + 1) / LOG_BASE).toInt();
                k = xd.length;
                if (xdi >= k) {
                    if (isTruncated) {
                        for (; k++ <= xdi;) {
                            xd!.push(0);
                        }
                        w = rd = 0;
                        digits = 1;
                        i %= LOG_BASE;
                        j = i - LOG_BASE + 1;
                    } else {
                        return Utils.finaliseExternal(x);
                    }
                } else {
                    w = k = xd[xdi];
                    for (digits = 1; k >= 10; k /= 10) {
                        digits++;
                    }

                    i %= LOG_BASE;
                    j = i - LOG_BASE + digits;
                    rd = j < 0 ? 0 : w / Math.pow(10, digits - j - 1) % 10 | 0;
                }
            }

            // Are there any non-zero digits after the rounding digit?
            isTruncated = isTruncated || signDigits < 0
                || ((xdi + 1) < xd!.length && xd![xdi + 1] !== undefined)
                || (j < 0 ? w : w % Math.pow(10, digits - j - 1)) > 0;

            let roundUp = (rm != undefined && rm! < 4)
                ? (rd || isTruncated) && (rm == 0 || rm == (x.s < 0 ? 3 : 2))
                : rd > 5 || rd == 5 && (rm == 4 || isTruncated || rm == 6 &&
                ((i > 0 ? j > 0 ? w / Math.pow(10, digits - j) : 0 : xdi - 1 >= 0 ? xd![xdi - 1] : NaN) % 10) & 1 ||
                rm == (x.s < 0 ? 8 : 7));

            if (signDigits < 1 || !(xd![0])) {
                xd!.length = 0;
                if (roundUp) {
                    signDigits -= x.e + 1;
                    xd!.push(Math.pow(10, (LOG_BASE - signDigits % LOG_BASE) % LOG_BASE));
                    x.exponent = -signDigits || 0;
                } else {
                    xd!.push(x.exponent = 0);
                }
                return x;
            }

            if (i == 0) {
                xd!.length = xdi;
                k = 1;
                xdi--;
            } else {
               xd!.length = xdi + 1;
               k = Math.pow(10, LOG_BASE - i);
               xd![xdi] = j > 0 ? (w / Math.pow(10, digits - j) % Math.pow(10, j) | 0) * k : 0;
            }

            if (roundUp) {
                while (true) {
                    if (xdi == 0) {
                        for (i = 1, j = xd![0]; j >= 10; j /= 10) {
                            i++;
                        }
                        xd![0] = xd![0] + k;
                        j = xd![0];
                        for (k = 1; j >= 10; j /= 10) {
                            k++;
                        }
                        if (i != k) {
                            x.exponent++;
                            if (xd![0] == BASE) {
                                xd![0] = 1;
                            }
                        }
                        break;
                    } else {
                        xd![xdi] = xd![xdi] + k;
                        if (xd![xdi] != BASE) {
                            break;
                        }
                        xd![xdi--] = 0;
                        k = 1;
                    }
                }
            }
            for (i = xd.length; xd![(--i).toInt()] === 0;) {
                xd!.pop();
            }
        }
        return Utils.finaliseExternal(x);
    }

    public static finaliseExternal(x: Decimal): Decimal {
        if (external) {
            if (x.e > Decimal.maxE) {
                x.digits = null;
                x.exponent = NaN;
            } else if (x.e < Decimal.minE) {
                x.exponent = 0;
                x.digits = Utils.updateDigits(0);
            }
        }
        return x;
    }

    /**
     * Return a new Decimal whose value is `x` divided by `y`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Decimal } x dividend
     * @param { Decimal } y divisor
     * @param { double } pr rounding precision
     * @param { int } rm rounding mode
     * @param { boolean } dp the flag of decimal places
     * @param { double } base division in the specified base, {2 | 8 | 10 | 16}
     * @returns { Decimal } the Decimal type
     */
    public static divide(x: Decimal, y: Decimal, pr?: double, rm?: int, dp?: boolean, base?: double): Decimal {
        let sign = x.s == y.s ? 1 : -1;
        let xd = x.d;
        let yd = y.d;
        if (!xd || !xd![0] || !yd || !yd![0]) {
            return new Decimal(
                !x.s || !y.s || (xd ? yd && xd![0] == yd![0] : !yd) ? NaN :
                xd && xd![0] == 0 || !yd ? sign * 0.0 : sign * Infinity);
        }
        xd = x.d!;
        yd = y.d!;
        let logBase: double;
        let e: double;
        if (base) {
            logBase = 1;
            e = x.e - y.e;
        } else {
            base = BASE;
            logBase = LOG_BASE;
            e = Math.floor(x.e / logBase) - Math.floor(y.e / logBase);
        }
        let yL: int = yd.length;
        let xL: int = xd.length;
        let q = new Decimal(sign);
        let qd = new Array<double>();
        q.digits = qd;
        let i: int;
        for (i = 0; i < yL && yd[i] == (i < xL ? xd[i] || 0 : 0); i++);

        if (i < yL && yd[i] > (i < xL ? xd[i] || 0 : 0)) {
            e--;
        }
        let sd: double;
        if (pr == undefined) {
            sd = pr = Decimal.precision;
            rm = Decimal.rounding;
        } else if (dp) {
            sd = pr + (x.exponent - y.e) + 1;
        } else {
            sd = pr;
        }
        let more: boolean;
        let k: double;
        if (sd < 0) {
            qd.push(1);
            more = true;
        } else {
            sd = sd / logBase + 2 | 0;
            i = 0;
            if (yL == 1) {
                k = 0;
                let ydv = yd[0];
                sd++;
                let t: double;
                for (; (i < xL || k) && sd--; i++) {
                    t = k * base + (i < xd.length ? (xd[i] || 0) : 0);
                    qd.push(t / ydv | 0);
                    k = t % ydv | 0;
                }
                more = (k || i.toDouble()) < xL;
            } else {
                k = base / (yd[0] + 1) | 0;
                if (k > 1) {
                    yd = Utils.multiplyInteger(yd, k, base);
                    xd = Utils.multiplyInteger(xd, k, base);
                    yL = yd.length;
                    xL = xd.length;
                }
                let xi = yL;
                let rem: Array<double> | undefined = xd.slice(0, yL);
                let remL: int = rem.length;
                for (; remL < yL;) {
                    if (remL == rem.length) {
                        rem.push(0);
                        remL++;
                    } else {
                        rem[remL++] = 0;
                    }
                }
                let yz = yd.slice();
                yz.unshift(0);
                let yd0 = yd[0];
                if (yd[1] >= base / 2) {
                    ++yd0;
                }
                do {
                    k = 0;
                    let cmp = Utils.compare(yd, rem!, yL, remL);
                    if (cmp < 0) {
                        let rem0 = rem![0];
                        if (yL != remL) {
                            rem0 = rem0 * base + (rem![1] || 0);
                        }
                        k = rem0 / yd0 | 0;
                        let prod: Array<double>;
                        if (k > 1) {
                            if (k >= base) {
                                k = base - 1;
                            }
                            prod = Utils.multiplyInteger(yd, k, base);
                            let prodL: number = prod.length;
                            remL = rem!.length;
                            cmp = Utils.compare(prod, rem!, prodL, remL);
                            if (cmp == 1) {
                                k--;
                                Utils.subtract(prod, yL < prodL ? yz : yd, prodL, base);
                            }
                        } else {
                            if (k == 0) {
                                cmp = k = 1;
                            }
                            prod = yd.slice();
                        }
                        let prodL = prod.length;
                        if (prodL < remL) {
                            prod.unshift(0);
                        }
                        Utils.subtract(rem!, prod, remL, base);
                        if (cmp == -1) {
                            remL = rem!.length;
                            cmp = Utils.compare(yd, rem!, yL, remL);
                            if (cmp < 1) {
                                k++;
                                Utils.subtract(rem!, yL < remL ? yz : yd, remL, base);
                            }
                        }
                        remL = rem!.length;
                    } else if (cmp === 0) {
                        k++;
                        rem = Utils.updateDigits(0);
                    }
                    qd.push(k);
                    i++;
                    if (cmp && rem![0]) {
                        rem!.push(xi < xL ? (xd[xi] || 0) : 0);
                        remL++;
                    } else {
                        rem = xi < xd.length ? Utils.updateDigits(xd[xi]) : undefined;
                        remL = 1;
                    }
                } while ((xi++ < xL || rem !== undefined) && sd--);
                more = rem !== undefined;
            }
            if (!qd![0]) {
                qd!.shift();
            }
        }
        if (logBase == 1) {
            q.exponent = e;
            inexact = more;
        } else {
            for (i = 1, k = qd![0]; k >= 10; k /= 10) {
                i++;
            }
            q.exponent = i + e * logBase - 1;
            let sd: double = dp ? pr + q.e + 1 : pr;
            Utils.finalise(q, sd, rm, more);
        }
        return q;
    }

    public static random(significantDigits?: double): Decimal {
        let result = new Decimal(1);
        if (significantDigits == undefined) {
            significantDigits = Decimal.precision;
        } else {
            Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        }

        // calculate the exponent based on 1e7.
        let exp = Math.ceil(significantDigits / LOG_BASE);
        let i: int = 0;
        let resultDigits: Array<double> = new Array<double>();
        if (!Decimal.crypto) {
            for (; i < exp;) {
                resultDigits.push(Math.random() * 1e7 | 0);
                i++;
            }
        } else {
            throw Utils.createBusinessError(CRYPTO_UNAVAILABLE_ERROR_CODE, `Crypto unavailable`);
        }

        let digitsValue = resultDigits[--i];
        significantDigits = significantDigits! % LOG_BASE;

        // Convert trailing digits to zeros according to significant digits.
        if (digitsValue && significantDigits) {
            let value = Math.pow(10, LOG_BASE - significantDigits);
            if (i < resultDigits.length) {
                resultDigits[i] = (digitsValue / value | 0) * value;
            } else {
                resultDigits.push((digitsValue / value | 0) * value);
            }
        }

        // Remove trailing words which are zero.
        for (; i >= 0 && resultDigits[i] === 0; i--) {
            resultDigits.pop();
        }

        // Zero?
        let resultExponent: double = 0;
        if (i < 0) {
            resultExponent = 0;
            resultDigits = new Array<double>();
            resultDigits.push(0);
        } else {
            resultExponent = -1;
            // Remove leading words which are zero and adjust exponent accordingly.
            for (; resultDigits[0] === 0; resultExponent -= LOG_BASE) {
                resultDigits.shift();
            }

            // Count the digits of the first word of result digits to determine leading zeros.
            let exponent: double = 0;
            let firstVal: double = 0;
            for (exponent = 1, firstVal = resultDigits[0]; firstVal >= 10; firstVal /= 10) {
                exponent++;
            }

            // Adjust the exponent for leading zeros of the first word of result digits.
            if (exponent < LOG_BASE) {
                resultExponent -= LOG_BASE - exponent;
            }
        }
        result.exponent = resultExponent;
        result.digits = resultDigits;

        return result;
    }

    public static createBusinessError(code: int, message: string) {
        return new BusinessError(code, new Error('BusinessError', message, undefined));
    }

    public static greaterThan(left: Decimal, right: Decimal): boolean {
        let result = left.comparedTo(right);
        return result > 0 || result === 0 && left.s > right.s;
    }

    public static lessThan(left: Decimal, right: Decimal): boolean {
        let result = left.comparedTo(right);
        return result < 0 || result === 0 && left.s < right.s;
    }
}
