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

package escompat;

// initialized in _initializerBlock_.ets
const hexChars: char[];

function fillInCommonURI(): boolean[] {
    const arr = new boolean[128]
    for (let i = Char.toInt(c'a'); i <= Char.toInt(c'z'); i++) {
        arr[i] = true
    }
    for (let i = Char.toInt(c'A'); i <= Char.toInt(c'Z'); i++) {
        arr[i] = true
    }
    for (let i = Char.toInt(c'0'); i <= Char.toInt(c'9'); i++) {
        arr[i] = true
    }
    arr[Char.toInt(c'-')] = true
    arr[Char.toInt(c'_')] = true
    arr[Char.toInt(c'.')] = true
    arr[Char.toInt(c'!')] = true
    arr[Char.toInt(c'~')] = true
    arr[Char.toInt(c'*')] = true
    arr[Char.toInt(c'\'')] = true
    arr[Char.toInt(c'(')] = true
    arr[Char.toInt(c')')] = true
    return arr
}

// initialized in _initializerBlock_.ets
const uriArray: boolean[];

function fillInURISpecial(arr: boolean[]) {
    arr[Char.toInt(c';')] = true
    arr[Char.toInt(c'/')] = true
    arr[Char.toInt(c'?')] = true
    arr[Char.toInt(c':')] = true
    arr[Char.toInt(c'@')] = true
    arr[Char.toInt(c'&')] = true
    arr[Char.toInt(c'=')] = true
    arr[Char.toInt(c'+')] = true
    arr[Char.toInt(c'$')] = true
    arr[Char.toInt(c',')] = true
    arr[Char.toInt(c'#')] = true
}

function createAndFillInURISpecial(): boolean[] {
    const arr = new boolean[128]
    fillInURISpecial(arr)
    return arr
}

// initialized in _initializerBlock_.ets
const uriSpecialChars: boolean[];

function fillInURINotComponent(): boolean[] {
    const arr = fillInCommonURI()
    fillInURISpecial(arr)
    return arr
}

// initialized in _initializerBlock_.ets
const uriArrayComponent: boolean[];

/**
 * The decodeURI() function decodes a Uniform Resource Identifier
 * (URI) previously created by encodeURI() or a similar routine.
 */
export function decodeURI(uri: string): string {
    return unescapeImpl(
        uri,
        (str: string, i: int, ret: StringBuilder, unit: int): void => {
            if (unit < 128 && uriSpecialChars[unit]) {
                ret.append(str.slice(i, i + 3))
            } else {
                ret.append(unit.toChar())
            }
        }
    )
}

function putByteAsHex(ret: StringBuilder, c: int): void {
    c &= 0xff
    ret.append(c'%')
    ret.append(hexChars[c / 16])
    ret.append(hexChars[c % 16])
}

function encodeCharAsHexUtf8(ret: StringBuilder, c: int): void {
    if (c <= 0x7f) {
        putByteAsHex(ret, c)
    } else if (c <= 0x7ff) {
        putByteAsHex(ret, c >> 6 | 0xc0)
        putByteAsHex(ret, c & 0x3f | 0x80)
    } else if (c <= 0xffff) {
        putByteAsHex(ret, c >> 12 | 0xe0)
        putByteAsHex(ret, (c >> 6) & 0x3f | 0x80)
        putByteAsHex(ret, (c >> 0) & 0x3f | 0x80)
    } else {
        putByteAsHex(ret, c >> 18 | 0xf0)
        putByteAsHex(ret, (c >> 12) & 0x3f | 0x80)
        putByteAsHex(ret, (c >> 6) & 0x3f | 0x80)
        putByteAsHex(ret, (c >> 0) & 0x3f | 0x80)
    }
}

function encodeCharAsHex(ret: StringBuilder, c: int): void {
    if (c < 256) {
        ret.append(c'%')
        ret.append(hexChars[c / 16])
        ret.append(hexChars[c % 16])
    } else {
        ret.append('%u')
        for (let j = 3; j >= 0; j--) {
            ret.append(hexChars[(c >> (4 * j)) & 0xf])
        }
    }
}

function encodeURIImpl(uri: string, ok: boolean[]): string {
    let ret = new StringBuilder()
    let i = 0
    while (i < uri.getLength()) {
        let d: int
        try {
            d = uri.codePointAt(i)!
        } catch (e) {
            throw new URIError("malformed URI sequence")
        }
        const c: char = uri.charAt(i)
        if (d != c) {
            encodeCharAsHexUtf8(ret, d)
            i += 2
            continue
        }
        if (Char.isLowSurrogate(c) || Char.isHighSurrogate(c)) {
            throw new URIError("malformed URI sequence")
        }
        if (Char.toInt(c) < 128 && ok[Char.toInt(c)]) {
            ret.append(c)
        } else {
            encodeCharAsHexUtf8(ret, d)
        }
        i += 1
    }
    return ret.toString()
}

/**
 * The encodeURI() function encodes a URI by replacing each instance
 * of certain characters by one, two, three, or four escape sequences
 * representing the UTF-8 encoding of the character (will only be four
 * escape sequences for characters composed of two surrogate
 * characters). Compared to encodeURIComponent(), this function
 * encodes fewer characters, preserving those that are part of the
 * URI syntax.
 */
export function encodeURI(uri: string): string {
    return encodeURIImpl(uri, uriArray)
}

/**
 * The decodeURIComponent() function decodes a Uniform Resource
 * Identifier (URI) component previously created
 * by encodeURIComponent() or by a similar routine.
 */
export function decodeURIComponent(uriComponent: string): string {
    return unescapeImpl(
        uriComponent,
        (str: string, i: int, ret: StringBuilder, unit: int): void => {
            ret.append(unit.toChar())
        }
    )
}

/**
 * The encodeURIComponent() function encodes a URI by replacing each
 * instance of certain characters by one, two, three, or four escape
 * sequences representing the UTF-8 encoding of the character (will
 * only be four escape sequences for characters composed of two
 * surrogate characters). Compared to encodeURI(), this function
 * encodes more characters, including those that are part of the URI
 * syntax.
 *
 * @param uriComponent a string to be encoded as a URI component
 * (a path, query string, fragment, etc.). Other values are converted to strings.
 *
 * @returns A new string representing the provided uriComponent encoded as a URI component.
 */
export function encodeURIComponent(uriComponent: String | Number | Boolean): string {
    if (uriComponent instanceof String) {
        return encodeURIImpl(uriComponent as String, uriArrayComponent);
    } else if (uriComponent instanceof Number) {
        return encodeURIImpl((uriComponent as Number).toString(), uriArrayComponent)
    } else {
        return encodeURIImpl((uriComponent as Boolean).toString(), uriArrayComponent)
    }
}

/**
 * @deprecated This feature is no longer recommended. Though some
 * browsers might still support it, it may have already been removed
 * from the relevant web standards, may be in the process of being
 * dropped, or may only be kept for compatibility purposes. Avoid
 * using it, and update existing code if possible; see the
 * compatibility table at the bottom of this page to guide your
 * decision. Be aware that this feature may cease to work at any time.
 *
 * @note escape() is a non-standard function implemented by browsers
 * and was only standardized for cross-engine compatibility. It is not
 * required to be implemented by all JavaScript engines and may not
 * work everywhere. Use encodeURIComponent() or encodeURI() if
 * possible.
 *
 * The escape() function computes a new string in which certain
 * characters have been replaced by hexadecimal escape sequences.
 */
export function escape(str: string): string {
    let ret = new StringBuilder()
    for (let i = 0; i < str.getLength(); i++) {
        const d = str.charAt(i)
        const c = d.toInt() & 0xffff
        if (c >= Char.toInt(c'a') && c <= Char.toInt(c'z') || c >= Char.toInt(c'A') && c <= Char.toInt(c'Z') || c >= Char.toInt(c'0') && c <= Char.toInt(c'9') || c >= 42 && c <= 47 && c != 44 || c == Char.toInt(c'_') || c == Char.toInt(c'@')) {
            ret.append(c.toChar())
        } else {
            encodeCharAsHex(ret, c)
        }
    }
    return ret.toString();
}

function unhex(c: char): int {
    if ((c.compareTo(c'0') >= 0) && (c.compareTo(c'9') <= 0)) {
        return c.toInt() - c'0'.toInt()
    }
    if ((c.compareTo(c'A') >= 0) && (c.compareTo(c'F') <= 0)) {
        return c.toInt() - c'A'.toInt() + 10
    }
    if ((c.compareTo(c'a') >= 0) && (c.compareTo(c'f') <= 0)) {
        return c.toInt() - c'a'.toInt() + 10
    }
    throw new Error("invalid codepoint")
}

function encodedURIHexAt(str: string, i: int): int {
    try {
        if (str.charAt(i) !== c'%') {
            throw new AssertionError("Char at " + i + "equals c'%'")
        }
        return unhex(str.charAt(i + 1)) << 4 | unhex(str.charAt(i + 2))
    } catch (e) {
        throw new URIError("malformed URI sequence")
    }
}

function unescapeImpl(str: string, push: (str: string, i: int, ret: StringBuilder, unit: int) => void): string {
    let ret = new StringBuilder();
    for (let i = 0; i < str.getLength(); i++) {
        const d = str.charAt(i)
        if (d != c'%') {
            ret.append(d)
            continue
        }
        if (str.charAt(i) != c'%') {
            ret.append(d)
            continue
        }
        let chr0 = encodedURIHexAt(str, i);
        let chrsCnt: int
        if ((chr0 & 0x80) == 0) {
            push(str, i, ret, chr0)
            i += 2
            continue
        } else if ((chr0 & 0xe0) == 0xc0) {
            chrsCnt = 2
            chr0 &= ~0xe0
        } else if ((chr0 & 0xf0) == 0xe0) {
            chrsCnt = 3
            chr0 &= 0xf
        } else if ((chr0 & 0xf8) == 0xf0) {
            chrsCnt = 4
            chr0 &= 0x7
        } else {
            throw new URIError("malformed URI sequence")
        }

        for (let j = 1; j < chrsCnt; j++) {
            let nxtChr = encodedURIHexAt(str, i + j * 3)
            if ((nxtChr & 0xc0) != 0x80) {
                throw new URIError("malformed URI sequence")
            }
            chr0 = chr0 << 6 | nxtChr & 0x3f
        }
        i += chrsCnt * 3 - 1

        if (chr0 >= 0 && chr0 <= 0xd7ff || chr0 >= 0xe000 && chr0 <= 0xffff) {
            push(str, i, ret, chr0)
        } else {
            ret.append(Char.getHighSurrogate(chr0))
            ret.append(Char.getLowSurrogate(chr0))
        }
    }
    return ret.toString();
}

/**
 * @deprecated This feature is no longer recommended. Though some
 * browsers might still support it, it may have already been removed
 * from the relevant web standards, may be in the process of being
 * dropped, or may only be kept for compatibility purposes. Avoid
 * using it, and update existing code if possible; see the
 * compatibility table at the bottom of this page to guide your
 * decision. Be aware that this feature may cease to work at any time.
 *
 * @note unescape() is a non-standard function implemented by browsers
 * and was only standardized for cross-engine compatibility. It is not
 * required to be implemented by all JavaScript engines and may not
 * work everywhere. Use decodeURIComponent() or decodeURI() if
 * possible.
 *
 * The unescape() function computes a new string in which hexadecimal
 * escape sequences are replaced with the characters that they
 * represent. The escape sequences might be introduced by a function
 * like escape().
 */
export function unescape(str: string): string {
    let ret = new StringBuilder();
    for (let i = 0; i < str.getLength(); i++) {
        const d = str.charAt(i)
        if (d != c'%') {
            ret.append(d)
            continue
        }
        let unit: int = 0
        if (i + 5 < str.getLength() && str.charAt(i + 1) == c'u') {
            try {
                unit = unhex(str.charAt(i + 2)) << 12 | unhex(str.charAt(i + 3)) << 8 | unhex(str.charAt(i + 4)) << 4 | unhex(str.charAt(i + 5))
            } catch (e) {
                ret.append(d)
                continue
            }
            ret.append(unit.toChar())
            i += 5
            continue
        }
        if (i + 2 < str.getLength()) {
            try {
                unit = unhex(str.charAt(i + 1)) << 4 | unhex(str.charAt(i + 2))
            } catch (e) {
                ret.append(d)
                continue
            }
            ret.append(unit.toChar())
            i += 2
            continue
        }
        ret.append(d)
    }
    return ret.toString();
}
