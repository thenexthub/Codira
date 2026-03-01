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

const MAX_CODE_POINT = 0x10FFFF // Unicode maximum allowed code points
const MAX_CODE_UNIT = 0xFFFF
const REPLACEMENT_CHARACTER = '\uFFFD'; // Unicode character replacement

// High Surrogate Range (UTF-16)
const HIGH_SURROGATE_MIN = 0xD800;   // Start of the high surrogate range (U+D800)
const HIGH_SURROGATE_MAX = 0xDBFF;   // End of the high surrogate range (U+DBFF)

// Low Surrogate Range
const LOW_SURROGATE_MIN = 0xDC00;    // Start of the low surrogate range (U+DC00)
const LOW_SURROGATE_MAX = 0xDFFF;    // End of the low surrogate range (U+DFFF)

// Unicode proxy for calculating related offsets
const SURROGATE_OFFSET = 0x400;      // Offset between high and low surrogates (0x10000 >> 10)
const SURROGATE_BASE = 0x10000;      // Base value for supplementary Unicode planes (e.g., U+10000)

type StringOrRegExp = String | RegExp

/**
 * Unicode string
 */
export final class String extends Object implements Comparable<String>, Iterable<String> {

    // The constructors below are implemented via initobj instruction
    /**
     * Constructs an empty String
     */
    public native constructor()

    /**
     * Constructs String from chars array initializer
     *
     * @param data initializer
     */
    public native constructor(data: FixedArray<char>)
    // let a = new char[1]
    // let b = new String(a)
    /**
     * Constructs String from another String
     *
     * @param otherStr initializer
     */
    public native constructor(otherStr: String)

    /**
     * Constructs String from Any
     *
     * @param object initializer
     */
    public native constructor(object: Any)

    /**
     * Creates a new instance of a String
     *
     * @returns A new String instance
     */
    static $_invoke(): string {
        return new String();
    }

    /**
     * Creates a new instance of a String from boolean
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: boolean): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from byte
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: byte): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from char
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: char): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from short
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: short): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from int
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: int): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from long
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: long): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from float
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: float): string {
        return new String(value);
    }

    /**
     * Creates a new instance of a String from double
     *
     * @param value The value to be converted to a string.
     *
     * @returns A new String instance
     */
    static $_invoke(value: double): string {
        return new String(value);
    }


    /**
     * Creates a new instance of a String
     *
     * @param value The value to be converted to a string. Can be an Object or null.
     *
     * @returns A new String instance
     */
    static $_invoke(value: Any): string {
        return new String(value);
    }

    /**
     * Returns an instance of string at passed index.
     *
     * @param index index to look at
     *
     * @returns String value at index
     *
     * @throws StringIndexOutOfBoundsError if index is negative or >= length
     */
    public native $_get(index: int): String;

    /**
     * Checks equality of this string and another Object as String
     *
     * @param to another object to compare
     *
     * @returns true if strings are equal and false otherwise
     *
     * @remarks
     * Implemented as native function,  @see `equals()` intrinsic.
     */
    public native equals(to: Any): boolean;

    /**
     * Length of this string
     *
     * @returns length of this string
     *
     * @remarks
     * Implemented as native function,  @see `getLength()` intrinsic.
     */
    public native getLength(): int;

    /**
     * Length of this string
     *
     * @returns length of this string
     */
    public get length(): int {
        return this.getLength()
    }

    /**
     * Getter for char at some index
     *
     * @param index index in char array inside String
     *
     * @returns char value at index
     *
     * @throws StringIndexOutOfBoundsError if index is negative or >= length
     *
     * @remarks
     * Implemented as native function,  @see `charAt()` intrinsic.
     */
    public native charAt(index: int): char;

    /**
     * Checks if this string is empty
     *
     * @returns true if empty and false otherwise
     * @remarks
     * Implemented as native function,  @see `length()` intrinsic.
     */
    public native isEmpty(): boolean;

    /**
     * Gets the codepoint at the specified index in this string.
     * Is similar to charAt(index), but if the character at the
     * index is the beginning of a surrogate pair, the int
     * representation for this codepoint is returned. Otherwise,
     * the character at the index is returned.
     *
     * @param index index of the potential surrogate pair
     *
     * @returns the codepoint at the specified index
     */
    public codePointAt(index: int): int | undefined {
        if (index < 0 || index >= this.getLength() || this.getLength() == 0) {
            return undefined
        }
        let highValue: char = this.charAt(index);
        if (!Char.isHighSurrogate(highValue) || ++index == this.getLength()) {
            return highValue.toInt();
        }
        let lowValue: char = this.charAt(index);
        if (!Char.isLowSurrogate(lowValue)) {
            return highValue.toInt();
        }
        return Char.charsToCodePoint(highValue, lowValue);
    }

    /**
     * Returns the amount of full codepoints between begin and end
     * indexes. Characters outside the range are not counted, even if
     * the range ends in the middle of a surrogate pair.
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsError if begin is negative or >= length
     * @throws StringIndexOutOfBoundsError if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns the amount of completed codepoints
     */
    public codePointCount(begin: int, end: int): int {
        if (begin > end) {
            throw new AssertionError('Begin idx must be less than end idx')
        }
        let counter: int = 0;
        for (let i: int = begin; i < end; i++) {
            ++counter;
            if (Char.isHighSurrogate(this.charAt(i)) &&
                (i + 1 < end) &&
                Char.isLowSurrogate(this.charAt(i + 1))) {
                ++i;
            }
        }
        return counter;
    }

    /**
     * Returns the amount of full codepoints between begin and end
     * indexes. Characters outside the range are not counted, even if
     * the range ends in the middle of a surrogate pair.
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsError if begin is negative or >= length
     * @throws StringIndexOutOfBoundsError if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns the amount of completed codepoints
     */
    public codePointCount(begin: number, end: number): number {
        return this.codePointCount(begin.toInt(), end.toInt())
    }

    /**
     * Gets the full char sequence that is representing
     * this string.
     *
     * @returns char[] array
     */
    public getChars(): char[] {
        return this.getChars(0, this.getLength());
    }

    /**
     * Gets the char sequence that is representing the part of
     * this string between begin and end indexes. The range is
     * a half-interview [begin, end).
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsError if begin > end
     * @throws StringIndexOutOfBoundsError if begin is negative or >= length
     * @throws StringIndexOutOfBoundsError if end > length
     *
     * @returns char[] array
     *
     * @remark
     * Implemented as native function,  @see `getChars()` intrinsic.
     */
    public getChars(begin: int, end: int): char[] {
        // #24515: Need to remove the following function call after primitive type refactoring
        let ret : FixedArray<char> = this.getCharsImpl(begin, end);
        let tmpForceBox : Array<char> = new Array<char>(ret.length);
        for (let i = 0; i < tmpForceBox.length; i++) {
            tmpForceBox[i] = ret[i]
        }
        return tmpForceBox;
    }

    private native getCharsImpl(begin: int, end: int): FixedArray<char>;

    /**
     * Gets the byte sequence that is representing the part of
     * this string between begin and end indexes. The range is
     * a half-interview [begin, end).
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsError if begin is negative or >= length
     * @throws StringIndexOutOfBoundsError if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns byte[] array
     */
    public getBytes(begin: int, end: int): byte[] {
        // #24515: Need to remove the following function call after primitive type refactoring
        let ret : FixedArray<byte> = this.getBytesImpl(begin, end);
        let tmpForceBox: Array<byte> = new Array<byte>(ret.length);
        for (let i = 0; i < tmpForceBox.length; i++) {
            tmpForceBox[i] = ret[i]
        }
        return tmpForceBox;
    }

    private native getBytesImpl(begin: int, end: int): FixedArray<byte>;

    /**
     * Compares the given StringBuilder to this String. The
     * result is true if the StringBuilder has the same content
     * as this String at this moment and false otherwise.
     *
     * @param sb StringBuilder to compare to
     *
     * @throws NullPointerError if sb param is null
     *
     * @returns true if StringBuilder has the same String
     */
    public contentEquals(sb: StringBuilder): boolean {
        return this.equals(sb.toString());
    }

    /**
     * Lexicographical comparison between this String and another one.
     * The result is less than 0 if this string sorts before the other,
     * 0 if they are equal, and greater than 0 otherwise.
     *
     * @param other String to compare with
     *
     * @throws NullPointerError if to param is null
     *
     * @returns the comparison result
     */
    public native override compareTo(other: String): int;

    /**
     * Comparison between this String and another one based on locale and options.
     * The result is -1 if this string sorts before the another string,
     * 0 if they are equal, and 1 otherwise.
     *
     * @param that String to compare with
     * @param locale String representing the BCP47 language tag
     * @param options Intl.CollatorOptions contains comparison options
     *
     * @throws RangeError if the locale tag is invalid or not found
     * @throws NullPointerError if another or locale is undefined
     *
     * @returns the comparison result
     */
    public localeCompare(that: String, locale?: string | string[], options?: Intl.CollatorOptions): int {
        return new Intl.Collator(locale, options).compare(this, that).toInt()
    }

    /**
     * Checks that the substring of this string that starts from
     * the specified index starts with the specified prefix.
     * Negative `fromIndex` is treated as 0. Result is always true
     * if `prefix` is empty.
     *
     * @param prefix prefix string
     * @param fromIndex index to start from
     *
     * @returns true if the substring begins with prefix
     */
    private native startsWithImpl(prefix: String, fromIndex: int): boolean;
    public startsWith(prefix: String, fromIndex?: int): boolean {
        if (fromIndex == undefined) {
            return this.startsWithImpl(prefix, 0)
        }
        return this.startsWithImpl(prefix, fromIndex!)
    }

    /**
     * Checks that this string ends with the specified suffix.
     * Result is always true if `suffix` is empty.
     *
     * @param suffix suffix string
     *
     * @param endPosition at which suffix is expected to be found. Defaults to str.length.
     *
     * @returns true if this string ends with suffix
     */
    private native endsWithImpl(suffix: String, endPosition: int): boolean;
    public endsWith(suffix: String, endPosition?: int): boolean {
        return this.endsWithImpl(suffix, endPosition ?? this.getLength());
    }

    /**
     * Finds the first occurrence of a character in this String.
     *
     * @param ch to find
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public native indexOf(ch: char): int;

    /**
     * Finds the first occurrence of a character in this String at position >= fromIndex.
     * Negative fromIndex is equivalent to 0, and fromIndex >= length implies no match.
     *
     * @param ch to find
     * @param fromIndex to start searching from
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public native indexOf(ch: char, fromIndex: int): int;

    /**
     * Finds the first occurrence of another String in this String
     *
     * @param str to find
     *
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public indexOf(str: String, fromIndex?: int): int {
        return this.indexOfImpl(str, fromIndex ?? 0)
    }

    /**
     * Finds the first occurrence of another String in this String at position >= fromIndex.
     * Negative fromIndex is equivalent to fromIndex = 0, and fromIndex >= length implies no match.
     *
     * @param str to find
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public native indexOfImpl(str: String, fromIndex: int): int;

    /**
     * Finds the last occurrence of a character in this String at position <= fromIndex.
     * All values of fromIndex >= length are equivalent, and negative fromIndex implies no match.
     *
     * @param ch to find
     * @param fromIndex to start searching from
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public native lastIndexOf(ch: char, fromIndex: int): int;

    /**
     * Finds the last occurrence of a character in this String.
     *
     * @param ch to find
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public lastIndexOf(ch: char): int {
        return this.lastIndexOf(ch, this.getLength());
    }

    /**
     * Finds the last occurrence of another String in this String.
     *
     * @param str to find
     *
     * @param fromIndex to start searching from
    *
     * @throws NullPointerError if str param is null
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public lastIndexOf(str: String, fromIndex?: int): int {
        if (fromIndex == undefined) {
            return this.lastIndexOfImpl(str, Int.MAX_VALUE as int)
        }
        return this.lastIndexOfImpl(str, fromIndex)
    }

    /**
     * Finds the last occurrence of another String in this String at position <= fromIndex.
     * All values of fromIndex >= length are equivalent, and negative fromIndex implies no match.
     *
     * @param str to find
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    private native lastIndexOfImpl(str: String, fromIndex: int): int;

    /**
     * Selects a substring of this String, starting at a specified index
     * and ending at index before another specified index.
     *
     * If 'begin' < 0, then 'begin' is assumed to be equal to zero.
     * If 'begin' > this.length, then 'begin' is assumed to be equal to the this.length.
     * If 'end' < 0, then 'end' is assumed to be equal to zero.
     * If 'end' > this.length, then 'end' is assumed to be equal to this.length.
     * If 'begin' > 'end', then these are swapped.
     * If 'begin' == 'end', then an empty string is returned.
     *
     * @param begin to start substring
     * @param end to end before at
     *
     * @returns new String which is a substring of this String
     */
    // NOTE(zaytsevaekaterina): fix num-> int & intrinsic tests #31034
    public native substring(begin: int, end: int): String;

    /**
     * Selects a substring of this String, starting at a specified index
     * and ending at index before another specified index.
     *
     * @param begin to start substring
     * @param end to end before at
     *
     * @returns new String which is a substring of this String
     */
    // NOTE(zaytsevaekaterina): fix num-> int & intrinsic tests #31034
    public substring(begin: number, end?: int): String {
        return this.substring(begin.toInt(), end ?? this.getLength())
    }

    /**
     * Concatenation of this and array of strings.
     *
     * @param strings strings to concat with
     *
     * @returns new String which is a concatenation of this + strings[0] + ... + strings[string.length - 1]
     */
    public concat(s: String): String {
        return String.concat2(this, s);
    }
    public concat(s1: String, s2: String): String {
        return String.concat3(this, s1, s2);
    }
    public concat(s1: String, s2: String, s3: String): String {
        return String.concat4(this, s1, s2, s3);
    }

    public concat(...strings: String[]): String {
        let length = strings.length
        if (length == 0) {
            return this
        }
        let res = new StringBuilder(this);
        for (let i = 0; i < length; i++) {
            res = res.append(strings[i])
        }
        return res.toString();
    }

    /**
     * Concatenation of two, three of four strings
     *
     * @param strings strings to concat
     *
     * @returns new String is the concatenation of the
     * arguments passed to the function:
     *
     * str1 + str2 - for concat2
     * str1 + str3 + str3 - for concat3
     * str1 + str2 + str3 + str4 - for concat4
     */
    private static native concat2(str1: String, str2: String): String;
    private static native concat3(str1: String, str2: String, str3: String): String;
    private static native concat4(str1: String, str2: String, str3: String, str4: String): String;

    /*
     * Creates a String using substitutions
     *
     * @param d a template
     *
     * @param substs strings to be substituted
     *
     * @returns string with respected data being substituted
     */
    // TODO(ivan-tyulyandin): uncomment when #12735 will be fixed
    // public static raw(d: String, substs: ...String): String {
    //     throw new Error('String.raw: not implemented')
    // }

    /**
     * Replaces all occurrences of the specified character
     * with another specified character. If no specified
     * character in this string is found then the original
     * string will be returned
     *
     * @param oldCh character which occurrences will be replaced
     * @param newCh character to replace
     *
     * @returns new String with replaced characters
     */
    public replaceChar(oldCh: char, newCh: char): String {
        let newChars: FixedArray<char> = this.getCharsImpl(0, this.getLength());
        for (let i: int = 0; i < newChars.length; i++) {
            if (newChars[i] == oldCh) {
                newChars[i] = newCh;
            }
        }
        return new String(newChars);
    }

    /**
     * Checks if this String contains the specified string.
     * The search starts from specified index.
     *
     * @param str string to search
     * @param fromIndex index to start search from
     *
     * @throws NullPointerError if str param is null
     * @throws StringIndexOutOfBoundsError if fromIndex param is negative or >= length
     *
     * @returns true if this String contains str and false otherwise
     */
    public contains(str: String, fromIndex: int): boolean {
        return this.indexOf(str, fromIndex) != -1;
    }

    /**
     * Checks if this String contains the specified string.
     * The search starts from specified index (negative fromIndex is equivalent to fromIndex = 0, and fromIndex >= length implies no match).
     *
     * @param str string to search
     * @param fromIndex index to start search from
     *
     * @throws NullPointerError if str param is null
     *
     * @returns true if this String contains str and false otherwise
     */
    public contains(str: String, fromIndex: number): boolean {
        return this.contains(str, fromIndex.toInt());
    }

    /**
     * Splits this String by a single char and returns an ordered array of substrings.
     * The order of the resulted array corresponds to the order of the passage of
     * this String from beginning to end. The pattern is excluded from substrings.
     * The array is limited by some specified value.
     *
     * @param separator - Char
     * @param limit - max length of the returned array. Int.MAX_VALUE if it's negative.
     *
     * @returns string array contains substrings from this String
     */
    private split(separator: Char, limit: Int): String[] {
        if (limit == 0) {
            return new String[0];
        }
        let strLen = this.getLength();
        if (strLen == 0) {
            return [(this)];
        }
        if (limit < 0) {
            limit = Int.MAX_VALUE;
        }

        let out: String[] = new String[0];
        let from: int = 0;
        while (true) {
            let index: int = this.indexOf(separator, from);
            if (index == -1) {
                out.pushOne(this.substring(from, this.getLength()));
                break;
            }
            if (out.pushOne(this.substring(from, index)) == limit) {
                break;
            }
            from = index + 1;
        }
        return out;
    }

    private splitImpl(separator: String, limit: Int): String[] {
        if (limit == 0) {
            return new String[0];
        }
        let strLen = this.getLength();
        let sepLen: int = separator.getLength();
        if (strLen == 0) {
            if (sepLen == 0) {
                return new String[0];
            }
            return [(this)];
        }
        if (limit < 0) {
            limit = Int.MAX_VALUE;
        }
        let out: String[] = new String[0];
        if (sepLen == 0) {
            // The string is split by UTF-16 code units and this destroys surrogate pairs (if any).
            if (strLen > limit) {
                strLen = limit;
            }
            for (let i = 0; i < strLen; ++i) {
                out.pushOne(String.fromCharCode(this.charAt(i).toDouble()));
            }
            return out;
        }
        let from: int = 0;
        while (true) {
            let index: int = this.indexOf(separator, from);
            if (index == -1) {
                out.pushOne(this.substring(from, this.getLength()));
                break;
            }
            if (out.pushOne(this.substring(from, index)) == limit) {
                break;
            }
            from = index + sepLen;
        }
        return out;
    }

    /**
     * Splits this String by separator and returns an ordered array of substrings.
     * The order of the resulted array corresponds to the order of the passage of
     * this String from beginning to end. The pattern is excluded from substrings.
     * The array is limited by some specified value.
     *
     * @param separator - String or RegExp to split by
     * @param limit - max length of the returned array. Int.MAX_VALUE if it's negative.
     *
     * @throws NullPointerError if pattern param is null
     *
     * @returns string array contains substrings from this String
     */
    public split(separator: String, limit?: int): String[] {
        let maxSize = limit == undefined ? Int.MAX_VALUE : limit.toInt();
        if (separator.getLength() != 1) {
            return this.splitImpl(separator, maxSize);
        }
        return this.split(separator.charAt(0), maxSize);
    }

    public split(separator: RegExp, limit?: int): String[] {
        return separator.split(this, limit);
    }

    public split(separator: StringOrRegExp, limit?: int): String[] {
        if (separator instanceof RegExp) {
            return (separator as RegExp).split(this, limit);
        }
        return this.split(separator as String, limit);
    }

    /**
     * Concatenates the specified string array by inserting
     * the specified separator between all elements.
     *
     * @param strings string array
     * @param delim separator between all elements
     *
     * @throws NullPointerError if strings param is null
     * @throws NullPointerError if delim param is null
     *
     * @returns newly created string from string array and delimiter
     */
    public static join(strings: String[], delim: String): String {
        return String.join(strings, delim, '', '');
    }

    /**
     * Concatenates the specified string array by inserting
     * the specified prefix before each element, the specified
     * suffix after each element, and the specified separator
     * between all elements.
     *
     * @param strings string array
     * @param delim separator between all elements
     * @param prefix prefix before each element
     * @param suffix suffix after each element
     *
     * @throws NullPointerError if strings param is null
     * @throws NullPointerError if delim param is null
     * @throws NullPointerError if prefix param is null
     * @throws NullPointerError if suffix param is null
     *
     * @returns newly created string from string array, prefix, suffix and delimiter
     */
    public static join(strings: String[], delim: String, prefix: String, suffix: String): String {
        let resStr: String = '';
        for (let i: int = 0; i < strings.length; i++) {
            resStr += prefix + strings[i] + suffix;
            if (i != strings.length - 1) {
                resStr += delim;
            }
        }
        return resStr;
    }

    /**
     * Creates new string similar to this String but with
     * all characters in lower case.
     *
     * @returns new string with all characters in lower case
     */
    public native toLowerCase(): String;

    /**
     * Creates new string similar to this String but with
     * all characters in upper case.
     *
     * @returns new string with all characters in upper case
     */
    public native toUpperCase(): String;

    /**
     * Trims all whitespaces from the beginning and end of this String.
     *
     * @returns new trimmed string
     */
    public native trim(): String;

    /**
     * Trims all whitespaces from the beginning of this String.
     *
     * @returns new left trimmed string
     */
    public native trimLeft(): String;

    /**
     * Trims all whitespaces from the end of this String.
     *
     * @returns new right trimmed string
     */
    public native trimRight(): String;

    /**
     * Checks whether the specified char is in the specified
     * char array or not.
     *
     * @param ch character to search
     * @param data char array to search in
     *
     * @throws NullPointerError if data param is null
     *
     * @returns true if ch is in the data and false otherwise
     */
    private static isCharOneOf(ch: char, data: char[]): boolean {
        for (let i: int = 0; i < data.length; i++) {
            if (ch == data[i]) {
                return true;
            }
        }
        return false;
    }

    /**
     * Trims all specified characters from the beginning and
     * end of this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerError if remove param is null
     *
     * @returns new trimmed string
     */
    public trim(remove: char[]): String {
        if (this.isEmpty()) {
            return this;
        }
        let right: int = this.getLength() - 1;
        let left: int = 0;
        if (String.isCharOneOf(this.charAt(right), remove)) {
            right--;
            while (left <= right && String.isCharOneOf(this.charAt(right), remove)) {
                right--;
            }
        } else {
            if (right != 0) {
                if (!String.isCharOneOf(this.charAt(left), remove)) {
                    return this;
                } else {
                    left++;
                }
            } else {
                return this;
            }
        }

        while (left <= right && String.isCharOneOf(this.charAt(left), remove)) {
            left++;
        }

        return this.substring(left, right + 1);
    }

    /**
     * Trims all specified characters from the beginning this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerError if remove param is null
     *
     * @returns new left trimmed string
     */
    public trimLeft(remove: char[]): String {
        if (this.isEmpty() || !String.isCharOneOf(this.charAt(0), remove)) {
            return this;
        }
        let firstNotSpecCharIdx: int = 0;
        for (let i: int = 1; i < this.getLength(); i++) {
            if (!String.isCharOneOf(this.charAt(i), remove)) {
                firstNotSpecCharIdx = i;
                break;
            }
        }
        return this.substring(firstNotSpecCharIdx, this.getLength());
    }

    /**
     * Trims all specified characters from the end of this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerError if remove param is null
     *
     * @returns new right trimmed string
     */
    public trimRight(remove: char[]): String {
        let last: int = this.getLength() - 1;
        if (this.isEmpty() || !String.isCharOneOf(this.charAt(last), remove)) {
            return this;
        }
        let lastNotSpecCharIdx: int = 0;
        for (let i: int = last - 1; i >= 0; i--) {
            if (!String.isCharOneOf(this.charAt(i), remove)) {
                lastNotSpecCharIdx = i;
                break;
            }
        }
        return this.substring(0, lastNotSpecCharIdx + 1);
    }

    /**
     * Creates a new string of a specified length in which
     * the beginning of this String is padded with a
     * specified character. `padStart` is an alias of this method,
     * except the parameter order.
     *
     * @param pad to repeat
     * @param count of characters in the resulting string
     *
     * @returns new string with padding at the beginning
     */
    public padLeft(pad: char, count: int): String {
        return this.padStart(count, pad)
    }

    /**
     * Creates a new string of a specified length in which
     * the end of this String is padded with a specified
     * character. `padEnd` is an alias of this method,
     * except the parameter order.
     *
     * @param pad to repeat
     * @param count of characters in the resulting string
     *
     * @returns new string with padding at the end
     */
    public padRight(pad: char, count: int): String {
        return this.padEnd(count, pad)
    }

    /**
     * Repeats this string count times, i.e.
     *    a = 'A',
     *    a.repeat(2) == 'AA'
    *
    * @param count number of repetitions of this String
    *
    * @throws ArgumentOutOfRangeError if count < 0
    *
    * @returns this string that is repeated count times
    */
    public native repeat(count: int): String;

    /**
     * The `toString()` method returns the string representation of the given String
     * in the form of a copy of the original object.
     *
     * @returns a copy of the original String
     */
    public override toString(): String {
        return this;
    }

    /**
     * The `toString()` method returns the string representation of the given String
     * in the form of a copy of the original object.
     *
     * @returns a copy of the original String
     */
    public override toLocaleString(): String {
        return this;
    }

    /**
     * The at() method takes an integer value and returns a new String consisting of the single UTF-16 code unit located
     * at the specified offset. This method allows for positive and negative integers. Negative integers count back from the last string character.
     *
     * @returns A String consisting of the single UTF-16 code unit located at the specified position.
     * Returns undefined if the given index can not be found.
     */
    public at(index: int): string | undefined {
        const n = this.getLength();
        if (index < 0) {
            index += n;
        }
        if (index < 0 || index >= n) {
            return undefined;
        }
        return new String(this.charAt(index).toString())
    }

    private CreateHTMLString(tag: String, param: String): String{
        return '<' + tag + param + '>' + (this) + '</' + tag + '>'
    }

    /**
     * The anchor() method creates a string that embeds a string in an <a> element with a name (<a name="...">str</a>)
     *
     * @returns A string beginning with an <a name="name"> start tag (double quotes in name are replaced with &quot;),
     * then the text str, and then an </a> end tag.
     */
    public anchor(name: String): String {
        return this.CreateHTMLString('a', ' name=\"' + name + '\"')
    }

    /*
     * The big() method creates a string that embeds a string in a <big> element (<big>str</big>), which causes a string to be displayed in a big font.
     */
    public big(): String{
        return this.CreateHTMLString('big', '')
    }


    /*
     * The small() method creates a string that embeds a string in a <small> element (<small>str</small>), which causes a string to be displayed in a big font.
     */
    public small(): String{
        return this.CreateHTMLString('small', '')
    }

    /*
     * The blink() method creates a string that embeds a string in a <blink> element (<blink>str</blink>), which causes a string to be displayed in a big font.
     */
    public blink(): String{
        return this.CreateHTMLString('blink', '')
    }

    /*
     * The bold() method creates a string that embeds a string in a <bold> element (<b>str</b>), which causes a string to be displayed in a big font.
     */
    public bold(): String{
        return this.CreateHTMLString('b', '')
    }

    /*
     * The italics() method creates a string that embeds a string in a <i> element (<i>str</i>), which causes a string to be displayed in a big font.
     */
    public italics(): String{
        return this.CreateHTMLString('i', '')
    }

    /*
     * The strike() method creates a string that embeds a string in a <strike> element (<strike>str</strike>), which causes a string to be displayed in a big font.
     */
    public strike(): String{
        return this.CreateHTMLString('strike', '')
    }

    /*
     * The sub() method creates a string that embeds a string in a <sub> element (<sub>str</sub>), which causes a string to be displayed in a big font.
     */
    public sub(): String{
        return this.CreateHTMLString('sub', '')
    }

    /*
     * The sup() method creates a string that embeds a string in a <sup> element (<sup>str</sup>), which causes a string to be displayed in a big font.
     */
    public sup(): String{
        return this.CreateHTMLString('sup', '')
    }

    /*
     * The fixed() method creates a string that embeds a string in a <tt> element (<tt>str</tt>), which causes a string to be displayed in a big font.
     */
    public fixed(): String{
        return this.CreateHTMLString('tt', '')
    }

    /*
     * The fontcolor() method creates a string that embeds a string in a <font> element (<font color="...">str</font>), which causes a string to be displayed in the specified font color.
     */
    public fontcolor(color: String): String{
        return this.CreateHTMLString('font', ' color=\"' + color + '\"')
    }

    /*
     * The fontsize() method creates a string that embeds a string in a <font> element (<font size="...">str</font>), which causes a string to be displayed in the specified font size.
     */
    public fontsize(size: int): String{
        return this.CreateHTMLString('font', ' size=\"' + size + '\"')
    }

    /*
     * The fontsize() method creates a string that embeds a string in a <font> element (<font size="...">str</font>), which causes a string to be displayed in the specified font size.
     */
    public fontsize(size: String): String {
        return this.CreateHTMLString('font', ' size=\"' + size.replaceAll('\"', '&quot;') + '\"')
    }

    /*
     * The link() method creates a string that embeds a string in an <a> element (<a href="...">str</a>), to be used as a hypertext link to another URL.
     */
    public link(link: String): String{
        return this.CreateHTMLString('a', ' href=\"' + link + '\"')
    }

    /*
     * The charCodeAt() method returns an integer between 0 and 65535 representing the UTF-16 code unit at the given index.
     */
    public charCodeAt(index: int): int {
        if (index < 0 || index >= this.getLength() || this.getLength() == 0) {
            throw new RangeError(`Invalid index: ${index}`);
        }
        return this.charAt(index).toInt()
    }

    // NOTE(zaytsevaekaterina): fix num-> int & intrinsic tests #31034
    public static native fromCharCodeImpl(codes: Array<number>): String;

    /**
     * The String.fromCharCode() static method returns a string created from the specified UTF-16 code unit
     *
     * @param code is a number between 0 and 65535 (0xFFFF) representing a UTF-16 code unit or NaN
     *
     * @returns string consisting of the specified UTF-16 code unit.
     *
     */
    // NOTE(zaytsevaekaterina): fix num-> int & intrinsic tests #31034
    public static native fromCharCode(code: number): String;

    /**
     * The String.fromCharCode() static method returns a string created from the specified sequence of UTF-16 code units
     *
     * @param codes are numbers between 0 and 65535 (0xFFFF) representing a UTF-16 code unit or NaN
     *
     * @returns string consisting of the specified UTF-16 code units.
     *
     */
    public static fromCharCode(...codes: number[]): String {
        if (codes.length < 0) {
            throw new NegativeArraySizeError("The value must be non negative")
        }
        return String.fromCharCodeImpl(codes)
    }

    /*
     * The valueOf() method returns the primitive value of a String object.
     */
    public valueOf(): String {
        return this;
    }

    /**
     * The includes() method performs a case-sensitive search to determine whether one string may
     * be found within another string, returning true or false as appropriate.
     *
     * @param searchString to be searched
     *
     * @param position within the string at which to begin searching for searchString
     *
     * @returns true if the search string is found anywhere within the given string,
     *  including when searchString is an empty string; otherwise, false
     */
    public includes(searchString: String, position?: int): boolean {
        return this.indexOf(searchString, position) != -1
    }

    public padEnd(end: int): String {
        return this.padEnd(end, ' ')
    }

    /*
     * The padEnd() method pads the current string with a given string (repeated, if needed)
     * so that the resulting string reaches a given length.
     * The padding is applied from the end of the current string.
     */
    public padEnd(end: int, ch: char): String {
        if (this.getLength() >= end) {
            return this;
        }
        let arr : FixedArray<char> = new FixedArray<char>(end)
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[i] = this.charAt(i)
        }
        for(let i = this.getLength(); i < end; i++){
            arr[i] = ch
        }
        return new String(arr)
    }

    /**
     * The padEnd() method pads the current string with a given string (repeated, if needed)
     * so that the resulting string reaches a given length.
     * The padding is applied from the end of the current string.
     *
     * @param end of the resulting string once the current str has been padded
     *
     * @param fillString to pad the current str with
     *
     * @returns string with the padString applied at the end of the current str
     */
    public padEnd(end: int, fillString?: String): String {
        const str = fillString ?? '\u0020'
        if (this.getLength() >= end || str.getLength() == 0) {
            return this;
        }
        let arr : FixedArray<char> = new FixedArray<char>(end);
        let length = this.getLength();
        let strLength = str.getLength()
        for (let i: int = 0; i < length; i++) {
            arr[i] = this.charAt(i);
        }
        for (let i = 0; i < strLength && length + i < end; i++) {
            arr[length + i] = str.charAt(i);
        }
        for (let i = this.getLength() + strLength; i < end; i++) {
            arr[i] = arr[i - strLength];
        }
        return new String(arr);
    }

    /*
     * The padStart() method pads the current string with another string (multiple times, if needed)
     * until the resulting string reaches the given length.
     * The padding is applied from the start of the current string.
     */
    public padStart(end: int, ch: char): String {
        let padLen = end - this.getLength();
        if (padLen <= 0) {
            return this;
        }
        let arr: FixedArray<char> = new FixedArray<char>(end)
        for(let i = 0; i < padLen; i++){
            arr[i] = ch
        }
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[padLen + i] = this.charAt(i);
        }
        return new String(arr)
    }

    public padStart(end: int, fillString?: String): String {
        const str = (fillString == undefined) ? '\u0020' : fillString
        if (this.getLength() >= end || str.getLength() == 0) {
            return this;
        }
        let arr: FixedArray<char> = new FixedArray<char>(end);
        let padLen: int = end - this.getLength();
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[padLen + i] = this.charAt(i);
        }
        let strLength = str.getLength()
        for (let i = 0; i < strLength && i < padLen; i++) {
            arr[i] = str.charAt(i);
        }
        for (let i = strLength; i < padLen; i++) {
            arr[i] = arr[i - strLength];
        }
        return new String(arr);
    }

    /**
     * The substr() method returns a portion of the string, starting at the specified
     * index and extending for a given number of characters afterwards.
     *
     * @param begin is index of the first character to include in the returned substring
     *
     * @param length is number of characters to extract
     *
     * @returns new string containing the specified part of the given string
     */
    public substr(begin: int, length?: int): String {
        begin = normalizeIndex(begin, this.getLength());
        length = length ?? this.getLength()
        let end = begin + length
        if (length > this.getLength() - begin) {
            end = this.getLength()
        }
        if (end <= begin) {
            return '';
        }
        return this.substring(begin, end);
    }


    /*
     * The trimEnd() method removes whitespace from the end of a string and returns a new string,
     * without modifying the original string. trimRight() is an alias of this method.
     */
    public trimEnd(): String {
        return this.trimRight()
    }

    /*
     * The trimStart() method removes whitespace from the beginning of a string and returns a new string,
     * without modifying the original string. trimLeft() is an alias of this method.
     */
    public trimStart(): String {
        return this.trimLeft()
    }

    /*
     * The slice() method extracts a section of a string and returns it as a new string,
     * without modifying the original string.
     */
    public slice(begin?: int, end?: int): String {
        begin = normalizeIndex(begin ?? 0, this.getLength())
        end = normalizeIndex(end ?? this.getLength(), this.getLength())
        if (end <= begin) {
            return '';
        }
        return this.substring(begin, end)
    }

    static getSubstitution(matched: String, str: String, position: int, captures: String[], namedCaptures: Record<String, String | undefined> | undefined, replacement: String) {
        let matchLength = matched.getLength()
        let stringLength = str.getLength()
        if (position < 0 || position > stringLength) {
            throw new AssertionError('Position ' + position + ' has to be in range 0..' + stringLength)
        }
        let tailPos = position + matchLength
        let m = captures.length
        let result = ''
        let doubleCapture = true;
        for (let i: int = 0; i < replacement.getLength();) {
            if (i + 1 < replacement.getLength()
                        && replacement.charAt(i) == c'$'
                        && replacement.charAt(i + 1) == c'$') {
                result += c'$'
                i += 2
            } else if (i + 1 < replacement.getLength()
                        && replacement.charAt(i) == c'$'
                        && replacement.charAt(i + 1) == c'&') {
                result += matched
                i += 2
            } else if (i + 1 < replacement.getLength()
                        && replacement.charAt(i) == c'$'
                        && replacement.charAt(i + 1) == c'`') {
                if (position != 0) {
                    result += str.substring(0, position)
                }
                i += 2
            } else if (i + 1 < replacement.getLength()
                        && replacement.charAt(i) == c'$'
                        && replacement.charAt(i + 1) == c'\'') {
                if (tailPos < stringLength) {
                    result += str.substring(tailPos, stringLength)
                }
                i += 2
            } else if (i + 2 < replacement.getLength()
                            && doubleCapture
                            && replacement.charAt(i) == c'$'
                            && Char.isDecDigit(replacement.charAt(i + 1))
                            && Char.isDecDigit(replacement.charAt(i + 2))) {
                let firstDigit = replacement.charAt(i + 1).toInt() - Char.toInt(c'0');
                let secondDigit = replacement.charAt(i + 2).toInt() - Char.toInt(c'0');
                let digit = firstDigit * 10 + secondDigit;
                if (digit == 0 || digit > m) {
                    doubleCapture = false;
                } else {
                    result += captures[digit - 1];
                    i += 3
                }
            } else if (i + 2 < replacement.getLength()
                            && replacement.charAt(i) == c'$'
                            && Char.isDecDigit(replacement.charAt(i + 1))
                            && !Char.isDecDigit(replacement.charAt(i + 2))
                        || i + 1 < replacement.getLength()
                            && replacement.charAt(i) == c'$'
                            && Char.isDecDigit(replacement.charAt(i + 1))) {
                let digit = replacement.charAt(i + 1).toInt() - Char.toInt(c'0');
                if (digit == 0 || digit > m) {
                    result += replacement.substring(i, i + 2)
                } else {
                    result += captures[digit - 1];
                }
                doubleCapture = true;
                i += 2
            } else if (i + 1 < replacement.getLength()
                            && replacement.charAt(i) == c'$'
                            && replacement.charAt(i + 1) == c'<') {
                if (namedCaptures == undefined) {
                    result += '$<';
                    i += 2;
                } else {
                    let j = i + 2;
                    for (; j < replacement.getLength() && replacement.charAt(j) != c'>'; ++j) {}
                    if (j < replacement.getLength()) {
                        let groupName = replacement.substring(i + 2, j);
                        let capture = namedCaptures[groupName];
                        if (capture != undefined) {
                            result += capture
                        }
                        i = j + 1;
                    } else {
                        result += '$<';
                        i += 2;
                    }
                }
            } else {
                result += replacement.charAt(i);
                i += 1;
            }
        }
        return result
    }

    /**
     * Returns a new string with one, some, or all matches of a pattern replaced by a replacement
     *
     * @param searchValue is pattern which can be a String or RegExp
     *
     * @param replaceValue is replacement String
     *
     * @returns a new replaced string
     */
    public replace(searchValue: RegExp, replaceValue: String) : String {
        return searchValue.replace(this, replaceValue)
    }

    public replace(searchValue: String, replaceValue: String) : String {
        let searchLength = searchValue.getLength()
        let position: int = this.indexOf(searchValue)
        if (position == -1) {
            return this
        }
        let preceding = this.substring(0, position)
        let following = this.substring(position + searchLength)
        let replacement = String.getSubstitution(searchValue, this, position, new String[0], undefined, replaceValue)
        return preceding + replacement + following
    }

    public replace(searchValue: StringOrRegExp, replaceValue: String) : String {
        if (searchValue instanceof RegExp) {
            return (searchValue as RegExp).replace(this, replaceValue)
        }
        return this.replace(searchValue as String, replaceValue)
    }

    /**
     * Returns a new string with one, some, or all matches of a pattern replaced by a replacement
     *
     * @param searchValue is pattern which can be a String or RegExp
     *
     * @param replacer is replacement function
     *
     * @returns a new replaced string
     */
    public replace(searchValue: RegExp, replacer: (substr: String, args: Object[]) => String): String {
        return searchValue.replace(this, replacer)
    }

    public replace(searchValue: String, replacer: (substr: String, args: Object[]) => String): String {
        let searchLength = searchValue.getLength()
        let position = this.indexOf(searchValue)
        if (position == -1) {
            return this
        }
        let preceding = this.substring(0, position)
        let following = this.substring(position + searchLength)
        let replacement = replacer(searchValue, [position.toDouble() as Object, this as Object] as Object[])
        return preceding + replacement + following
    }

    public replace(searchValue: StringOrRegExp, replacer: (substr: String, args: Object[]) => String): String {
        if (searchValue instanceof RegExp) {
            return (searchValue as RegExp).replace(this, replacer)
        }
        return this.replace(searchValue as String, replacer)
    }

     /**
     * Returns a new string with all matches of a pattern replaced by a replacement
     *
     * @param searchValue is pattern which can be a String or RegExp
     *
     * @param replaceValue is replacement String
     *
     * @returns a new replaced string
     */
    public replaceAll(searchValue: RegExp, replaceValue: String): String {
        if (!searchValue.global) {
            throw new Error('Global flag expected for regexp')
        }
        return searchValue.replace(this, replaceValue)
    }

    public replaceAll(searchValue: String, replaceValue: String): String {
        let searchLength = searchValue.getLength()
        let advanceBy = max(1, searchLength)
        let matchPositions = new containers.ArrayAsListInt();
        let position = this.indexOf(searchValue, 0)
        while (position != -1) {
            matchPositions.pushBack(position)
            if (position == this.getLength()) {
                break;
            }
            position = this.indexOf(searchValue, position + advanceBy)
        }
        let endOfLastMatch = 0
        let result = ''
        let arrayMatchPositions = matchPositions.toArray()
        for (let i = 0; i < arrayMatchPositions.length; ++i) {
            let p = arrayMatchPositions[i]
            let preserved = this.substring(endOfLastMatch, p)
            let replacement = String.getSubstitution(searchValue, this, p, new String[0], undefined, replaceValue)
            result = result + preserved + replacement
            endOfLastMatch = p + searchLength
        }
        if (endOfLastMatch < this.getLength()) {
            result += this.substring(endOfLastMatch)
        }
        return result
    }

    public replaceAll(searchValue: StringOrRegExp, replaceValue: String): String {
        if (searchValue instanceof RegExp) {
            const re = searchValue as RegExp
            if (!re.global) {
                throw new Error('Global flag expected for regexp')
            }
            return re.replace(this, replaceValue)
        }
        return this.replaceAll(searchValue as String, replaceValue)
    }

    /**
     * Returns a new string with all matches of a pattern replaced by a replacement
     *
     * @param searchValue is pattern which can be a String or RegExp
     *
     * @param replacer is replacement function
     *
     * @returns a new replaced string
     */
    public replaceAll(searchValue: RegExp, replacer: (substr: String, args: Object[]) => String): String {
        if (!searchValue.global) {
            throw new Error('Global flag expected for regexp')
        }
        return searchValue.replace(this, replacer)
    }

    public replaceAll(searchValue: String, replacer: (substr: String, args: Object[]) => String): String {
        let searchLength = searchValue.getLength()
        let advanceBy = max(1, searchLength)
        let matchPositions = new containers.ArrayAsListInt();
        let position = this.indexOf(searchValue, 0)
        while (position != -1) {
            matchPositions.pushBack(position)
            position = this.indexOf(searchValue, position + advanceBy)
        }
        let endOfLastMatch = 0
        let result = ''
        let arrayMatchPositions = matchPositions.toArray()
        for (let i = 0; i < arrayMatchPositions.length; ++i) {
            let p = arrayMatchPositions[i]
            let preserved = this.substring(endOfLastMatch, p)
            let args = new containers.UndefinableObjectArray(2)
            args.pushBack(Double.valueOf(p))
            args.pushBack(this)
            let replacement = replacer(searchValue, args.toArray() as Object[])
            result = result + preserved + replacement
            endOfLastMatch = p + searchLength
        }
        if (endOfLastMatch < this.getLength()) {
            result += this.substring(endOfLastMatch)
        }
        return result
    }

    public replaceAll(searchValue: StringOrRegExp, replacer: (substr: String, args: Object[]) => String): String {
        if (searchValue instanceof RegExp) {
            const re = searchValue as RegExp
            if (!re.global) {
                throw new Error('Global flag expected for regexp')
            }
            return re.replace(this, replacer)
        }
        return this.replaceAll(searchValue as String, replacer)
    }

      /**
     * Executes a search for a match between a regular expression and this String object.
     *
     * @param regexp a regular expression object or implicit regular expression
     *
     * @returns the index of the first match between the regular expression and the given string,
     * or -1 if no match was found.
     */
    public search(implicitRegExp: String): int {
        return new RegExp(implicitRegExp).search(this)
    }

    public search(regexp: RegExp): int {
        return regexp.search(this)
    }

    public search(regexp: StringOrRegExp): int {
        if (regexp instanceof String) {
            return new RegExp(regexp as String).search(this)
        }
        return (regexp as RegExp).search(this)
    }

    /*
     * The toLocaleLowerCase() method returns the calling string value converted to lower case,
     * according to any locale-specific case mappings.
     */
    public native toLocaleLowerCase(locale: String): String;

    public toLocaleLowerCase(locale: String[]): String {
        if (locale.length != 0) {
            return this.toLocaleLowerCase(locale[0]);
        }
        return this.toLocaleLowerCase('');
    }

    public toLocaleLowerCase(): String {
        return this.toLocaleLowerCase('');
    }

    /*
     * The toLocaleUpperCase() method returns the calling string value converted to upper case,
     * according to any locale-specific case mappings.
     */
    public native toLocaleUpperCase(locale: String): String;

    public toLocaleUpperCase(locale: String[]): String {
        if (locale.length != 0) {
            return this.toLocaleUpperCase(locale[0]);
        }
        return this.toLocaleUpperCase('');
    }

    public toLocaleUpperCase(): String {
        return this.toLocaleUpperCase('');
    }
    /**
     * Retrieves the result of matching a string against a regular expression
     *
     * @param regexp a regular expression object
     *
     * @returns
     * If the regexp.global is true, all results matching the complete regular expression will be returned,
     * but capturing groups are not included
     * Otherwise, only the first complete match and its related capturing groups are returned
     */
    public match(implicitRegExp: String): RegExpMatchArray | null {
        return new RegExp(implicitRegExp).match(this)
    }

    public match(regexp: RegExp): RegExpMatchArray | null {
        return regexp.match(this)
    }

    public match(regexp: StringOrRegExp): RegExpMatchArray | null {
        if (regexp instanceof String) {
            return new RegExp(regexp as String).match(this)
        }
        return (regexp as RegExp).match(this)
    }


    /**
     * Returns an iterator of all results matching a string against a regular expression,
     * including capturing groups
     *
     * @param regexp a regular expression object
     */
    public matchAll(reg: RegExp): IterableIterator<RegExpMatchArray> {
        let flags = reg.flags;
        if (!reg.global) {
            throw new Error('matchAll must be called with a global RegExp')
        }
        return reg.matchAll(this)
    }

    private native normalizeNFC(): String;

    private native normalizeNFD(): String;

    private native normalizeNFKC(): String;

    private native normalizeNFKD(): String;

    /**
     * The normalize() method of String values returns the Unicode Normalization Form of this string
     *
     * @param form is 'NFC' or 'NFD' or 'NFKC' or 'NFKD'
     *
     * @throws RangeError if form is not 'NFC' or 'NFD' or 'NFKC' or 'NFKD'
     *
     * @returns the Unicode Normalization Form of the string
     */
    public normalize(form?: String): String {
        const f = (form == undefined) ? 'NFC' : form
        switch (f) {
        case 'NFC':
            return this.normalizeNFC()
        case 'NFD':
            return this.normalizeNFD()
        case 'NFKC':
            return this.normalizeNFKC()
        case 'NFKD':
            return this.normalizeNFKD()
        default:
            throw new RangeError('The normalization form should be one of NFC, NFD, NFKC, NFKD.')
        }
    }

    /**
     * Check if low order proxy items are paired when the current character is a high proxy pair
     *
     * @param currentIndex Index of the current high-level proxy item
     * @param highSurrogateCode Symbol value of high-level proxy term
     * @param totalLength Total length of string
     *
     * @returns If pairing is successful, return valid characters; otherwise, return null
     */
    private processHighSurrogatePair(currentIndex: int, highSurrogateCode: int, totalLength: int): string | null {
        // Check if low level proxy items are paired
        if (currentIndex + 1 >= totalLength) {
            return null;
        }
        const nextCodeUnit = this.charCodeAt(currentIndex + 1);
        if (nextCodeUnit < LOW_SURROGATE_MIN || nextCodeUnit > LOW_SURROGATE_MAX) {
            return null;
        }
        // Calculate joint code points
        const codePoint = SURROGATE_BASE +
            (highSurrogateCode - HIGH_SURROGATE_MIN) * SURROGATE_OFFSET +
            (nextCodeUnit - LOW_SURROGATE_MIN);
        // Verify whether the joint code points are legal
        return Char.isValidCodePoint(codePoint.toInt()) ? String.fromCodePoint(codePoint) : null;
    }

    /**
     * The toWellFormed() method of String values returns a string where all lone surrogates of
     * this string are replaced with the Unicode replacement character U+FFFD.
     */
    public toWellFormed(): string {
        let res = new StringBuilder();
        let length = this.getLength();
        for (let i = 0; i < length; i++) {
            let codeUnit = this.charCodeAt(i);
            // Currently, it is a high-level proxy item
            if (codeUnit >= HIGH_SURROGATE_MIN && codeUnit <= HIGH_SURROGATE_MAX) {
                // Check if low level proxy items are paired
                const processedChar = this.processHighSurrogatePair(i, codeUnit, length);
                if (processedChar !== null) {
                    res.append(processedChar);
                    i++; // Skip processed low-level proxy items
                    continue;
                }
                res.append(REPLACEMENT_CHARACTER);
            // Currently, it is a low-level proxy item
            } else if (codeUnit >= LOW_SURROGATE_MIN && codeUnit <= LOW_SURROGATE_MAX) {
                res.append(REPLACEMENT_CHARACTER);
            // Regular Characters
            } else if (Char.isValidCodePoint(codeUnit)) {
                res.append(this.charAt(i).toChar());
            } else {
                res.append(REPLACEMENT_CHARACTER);
            }
        }
        return res.toString();
    }

    private static native codePointToChar(cp: int): int

    /**
     * The String.fromCodePoint() static method returns a string created by using the specified sequence of code points
     *
     * @param codePoints are integers between 0 and 0x10FFFF (inclusive) representing a Unicode code point
     *
     * @throws RangeError if codePoints[i] is less than 0, or is greater than 0x10FFFF
     *
     * @returns string created by using the specified sequence of code points
     */
    public static fromCodePoint(...codePoints: int[]): String {
        let res = new StringBuilder();
        for (const cp of codePoints) {
            if (cp < 0 || cp > MAX_CODE_POINT) {
                throw new RangeError('Invalid code point: ' + new Number(cp).toString())
            }
            let chrs = String.codePointToChar(cp.toInt());
            res.append((chrs & 0xffff).toChar())
            chrs = (chrs >> 16) & 0xffff
            if (chrs > 0) {
                res.append(chrs.toChar());
            }
        }
        return res.toString()
    }

    /*
     * The isWellFormed() method of String values returns a boolean indicating whether this string contains any lone surrogates.
     */
    public native isWellFormed(): boolean;

    /**
     * Check if a string is compressed
     *
     * @param s string to be checked
     *
     * @returns true - if s is compressed, false - otherwise
     */
    public native isCompressed(): boolean;

    public override $_iterator(): IterableIterator<String> {
        return new StringIterator(this)
    }
}

class StringIterator implements IterableIterator<String> {
    private s: String
    private idx: int = 0

    constructor(s: String) {
        this.s = s
    }

    public override next(): IteratorResult<String> {
        if (this.idx == this.s.getLength()) {
            return new IteratorResult<String>()
        }
        let highValue: char = this.s.charAt(this.idx);
        this.idx += 1
        if (!Char.isHighSurrogate(highValue) || this.idx == this.s.getLength()) {
            return new IteratorResult<String>(String.fromCharCode(highValue.toInt()));
        }
        let lowValue: char = this.s.charAt(this.idx);
        if (!Char.isLowSurrogate(lowValue)) {
            return new IteratorResult<String>(String.fromCharCode(highValue.toInt()));
        }
        this.idx += 1
        return new IteratorResult<String>(String.fromCharCode(highValue.toInt(), lowValue.toInt()));
    }
}

/**
 * `string` is an alias for type `String`
 */
export type string = String;
