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

const SyntaxErrorCodeId: int = 10200002;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

/**
* The url module provides utilities for URL resolution and parsing.
*
* @namespace url
*/
export namespace url {

    export type UrlCbFn = (value: string, key: string, searchParams: URLParams) => void;

    /**
    * Store some constants and arrays
    * @class Helper
    */
    class Helper {
        static ASCII_32: number = 32;
        static ASCII_127: int = 127;
        static specialCharForBit: Array<number> = new Array<number>(128);
        static head: Map<string, number> = new Map<string, number>();
        static doubleSegment = new Array<string>("..", ".%2e", ".%2E", "%2e.", "%2E.",
            "%2e%2e", "%2E%2E", "%2e%2E", "%2E%2e");

        static singlesegment = new Array<string>('.', "%2e", "%2E");

        static spesicalSymbols: Record<string, string> =
            { "@": "%40", "#": "%23", "=": "%3D", ":": "%3A", "/": "%2F", ";": "%3B", "?": "%3F" };

        static specialcharacter = new Array<string>('\0', '\t', '\n', '\r', ' ', '#', '%', '/', ':', '?', '@', '[', '\\', ']');

        static hexTable: Array<string> = new Array<string>(256);

        static {
            Helper.specialCharForBit.fill(0);
            let specialSymbolsTmp = ['#', '%', '/', ':', '?', '@', '[', '\\', ']', '<', '>', '^', '|'];
            for (let i = 0; i <= Helper.ASCII_32; i++) {
                Helper.specialCharForBit[i] = 1;
            }
            for (let i = 0; i < specialSymbolsTmp.length; i++) {
                let specialChar = specialSymbolsTmp[i].charCodeAt(0).toInt();
                Helper.specialCharForBit[specialChar] = 1;
            }
            Helper.specialCharForBit[Helper.ASCII_127] = 1;

            Helper.head.set("ftp:", 21);
            Helper.head.set("file:", -1);
            Helper.head.set("gopher:", 70);
            Helper.head.set("http:", 80);
            Helper.head.set("https:", 443);
            Helper.head.set("ws:", 80);
            Helper.head.set("wss:", 443);
            const hexNum = 16;
            for (let i = 0; i < 256; ++i) {
                Helper.hexTable[i] = "%" +
                    ((i < hexNum ? "0" : "") + Double.toString(i, hexNum))  // Convert to hexadecimal and handle leading zeros
                        .toUpperCase();
            }
        }

        // hostname, host
        static URL_ENCODED_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // username, password {'%25':'%'}
        static USERINFO_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // path {'%7C': '|', '%5B': '[', '%5D': ']', '%25': '%'} %5C '\\'
        static PATH_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, // 0x50 - 0x5F  // todo 5c
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // hash, fragment {'%7C': '|', '%5B': '[', '%5D': ']', '%7B': '{', '%7D': '}', '%60': '`', '%25': '%'}
        static FRAGMENT_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x50 - 0x5F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 0x70 - 0x7F
        ]);

        // query {'%7C': '|', '%5B': '[', '%5D': ']', '%7B': '{', '%7D': '}', '%60': '`', '%5E': '^', '%25': '%'}
        static QUERY_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 0x50 - 0x5F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 0x70 - 0x7F
        ]);

        // params tostring encode
        static PARAMS_TOSTRING_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0x30 - 0x3F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 0x70 - 0x7F
        ]);

        static unhexTable = new Int8Array([
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0 - 15
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16 - 31
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32 - 47
            +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, -1, -1, -1, -1, -1, -1, // 48 - 63
            -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64 - 79
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80 - 95
            -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96 - 111
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 112 - 127
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128 ...
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // ... 255
        ]);
    }

    function checkCharacter(input: string) {
        let inputLen = input.length;
        for (let i = 0; i < inputLen; i++) {
            let charPos: int = input.charCodeAt(i).toInt();
            if (charPos >= 0 && charPos < 128) {
                if (Helper.specialCharForBit[charPos]) {
                    return false;
                }
            }
        }
        return true;
    }

    function deleteC0OrSpace(input: string): string {
        const codeC0 = '\0'.charCodeAt(0);
        const codeSpace = ' '.charCodeAt(0);
        let left = 0;
        let right = input.length - 1;
        while (left <= right &&
            input.charCodeAt(left) <= codeSpace &&
            input.charCodeAt(left) >= codeC0) {
            left++;
        }
        while (right >= left &&
            input.charCodeAt(right) <= codeSpace &&
            input.charCodeAt(right) >= codeC0) {
            right--;
        }
        return left > right ? "" : input.slice(left, right + 1);
    }

    function deleteTabOrNewline(input: string): string {
        let reg = new RegExp('\\t|\\r|\\n', 'g');
        return input.replaceAll(reg, '');
    }

    function isAlphaNum(ch: char): boolean {
        return Char.isLetter(ch) || Char.isDecDigit(ch);
    }

    function isSpecialScheme(scheme: string) {
        return Helper.head.has(scheme);
    }

    function isFileNotHost(input: string) {
        return Char.isLetter(input.charAt(0)) && (input.charAt(1) == c':' || input.charAt(1) == c'|');
    }

    function DecodeSpecialChars(input: string): string {
        let temp: string = input;
        let len: number = temp.length;
        if (input.length == 0) {
            return temp;
        }
        let pos: int = temp.indexOf("%");
        while ((pos != -1) && (pos < len - 2)) {
            if (Char.isHexDigit(temp.charAt(pos + 1)) && Char.isHexDigit(temp.charAt(pos + 2))) {
                let subStr = temp.substring(pos + 1, pos + 3);
                let octNum: number = parseInt(subStr, 16);
                if (isNaN(octNum)) {
                    return temp;
                }

                let convertedChar = String.fromCharCode(octNum.toInt());
                temp = temp.slice(0, pos) + convertedChar + temp.slice(pos + 3);
                len = len - 2;
            }
            pos = temp.indexOf("%", pos + 1);
        }
        return temp;
    }

    function dealIPv4(input: string): string {
        const hexSegments = new Array<string>();
        const lastColonPos = input.lastIndexOf(':');
        let currentPosition = lastColonPos + 1;
        while (currentPosition < input.length) {
            const dotPos = input.indexOf('.', currentPosition);
            const segmentEnd = dotPos === -1 ? input.length : dotPos;

            const segmentValue = input.substring(currentPosition, segmentEnd);
            const numericValue = parseInt(segmentValue, 10);

            if (isNaN(numericValue) || numericValue > 255 || numericValue < 0) {
                return segmentValue;
            }

            const hexValue = numericValue.toString(16).padStart(2, '0');
            hexSegments.push(hexValue);
            currentPosition = segmentEnd + 1;
        }
        const prefix = input.substring(0, lastColonPos);
        return `${prefix}:${hexSegments[0]}${hexSegments[1]}:${hexSegments[2]}${hexSegments[3]}`;
    }

    function formatIPv6(input: string): string {
        let pos: int = input.indexOf("::");
        let index: int = pos;
        if (pos != -1) {
            let left: int = 0;
            let count: int = 0;
            while ((pos = input.indexOf(":", left)) != -1) {
                count += 1;
                left = pos + 1;
            }
            let size = 7 - (count - 2);
            let temp = "";
            for (let i = 0; i < size - 1; i++) {
                temp += ":0";
            }
            temp += ":";
            input = input.slice(0, index) + temp + input.substring(index + 2);
            if (index == 0) {
                input = "0" + input;
            }
        }
        return input;
    }

    function zeroCompression(inputs: Array<string>): string {
        let maxIndex: number = 0;
        let maxSize: number = 0;
        let index: number = 0;
        let size: number = 0;
        let needZeroCompression: boolean = false;
        let inputSize: number = inputs.length;
        for (let i = 0; i < inputSize; i++) {
            index = i;
            size = 0;
            while (i < inputSize && inputs[i] == "0") {
                needZeroCompression = true;
                size++;
                i++;
            }
            if (maxSize < size) {
                maxSize = size;
                maxIndex = index;
            }
        }
        let res: string = "";
        for (let i = 0; i < inputSize; i++) {
            if (needZeroCompression && i == maxIndex) {
                if (maxIndex == 0) {
                    res += "::";
                } else {
                    res += ":";
                }
                i += maxSize - 1;
                continue;
            }
            res += inputs[i];
            i != (inputSize - 1) ? res += ":" : "";
        }
        return res;
    }

    function compress(input: string): string {
        let array = Array.from<string, string>(
            input.split(':'),
            (it) => it.trimLeft([c'0'])
        )
        let res: string = zeroCompression(array);
        return res.toLowerCase();
    }

    function isRadix(input: string, radix: string): boolean {
        let len = input.length;
        for (let i = 0; i < len; i++) {
            if (radix.indexOf(input.charAt(i)) == -1) {
                return false;
            }
        }
        return true;
    }

    function isNumber(num: string, convension: number[]): boolean {
        let len: number = num.length;
        if (len >= 2 && num.charAt(0) == c'0' && (num.charAt(1) == c'x' || num.charAt(1) == c'X')) {
            convension[0] = 16;
            let subStr = num.substring(2);
            if (subStr.length == 0) {
                return true;
            }
            return isRadix(subStr, "0123456789abcdefABCDEF");
        } else if (len >= 1 && num.charAt(0) == c'0') {
            convension[0] = 8;
            let subStr = num.substring(1);
            if (subStr.length == 0) {
                return true;
            }
            return isRadix(subStr, "01234567");
        } else if (isRadix(num, "0123456789")) {
            convension[0] = 10;
            return true;
        }
        return false;
    }

    function BinaryConversion(num: string, radix: number): string {
        let val: number = 0;
        if (radix == 16) {
            if (num.substring(2).length == 0) {
                return "0";
            }
            val = parseInt(num, 16);
            if (isNaN(val)) {
                return num;
            }
            return Number.toString(val);
        } else if (radix == 8) {
            if (num.substring(1).length == 0) {
                return "0";
            }
            val = parseInt(num, 8);
            if (isNaN(val)) {
                return num;
            }
            return Number.toString(val);
        }
        return num;
    }

    function getFormatIPv4Index(nums: Array<string>): int {
        let len = nums.length;
        for (let i = 0; i < len; i++) {
            if (nums[i].length > 8) {
                return i;
            }
            if (nums[i].length > 0 && (parseInt(nums[i]) > 255)) {
                return i;
            }
        }
        return -1;
    }

    function splitNum(num: string, storeNumber: number[]): string {
        if (num.length > 8) {
            storeNumber[0] = num.length;
            return num;
        }
        let val: number = parseInt(num);
        let nums = new Array<string>();
        let res: string = "";
        while (val > 0) {
            let numConvert: number = val % 256;
            nums.push(Number.toString(numConvert));
            val = Math.floor(val / 256);
        }
        for (let i = nums.length - 1; i >= 0; i--) {
            res += nums[i] + ".";
        }

        storeNumber[0] = nums.length;
        return res.slice(0, res.length - 1);
    }

    const INDEX_INVALID = 0;
    const INDEX_SPECIAL = 1;
    const INDEX_USERNAME = 2;
    const INDEX_PASSWORD = 3;
    const INDEX_HOST = 4;
    const INDEX_PORT = 5;
    const INDEX_PATH = 6;
    const INDEX_QUERY = 7;
    const INDEX_FRAGMENT = 8;
    const INDEX_INFOPATH = 9;
    const INDEX_IPV6 = 10;
    const INDEX_DOUBLE_SLASH = 11;


    /**
    * Handle the specific URL logic
    * @class InnerURL
    */
    class InnerURL {
        flags: boolean[] = new boolean[12];
        cScheme: string = "";
        cFragment: string = "";
        cQuery: string = "";
        cPath: Array<string> = new Array<string>();
        cHost: string = "";
        cIsSpecialPath: boolean = false;
        cUsername: string = "";
        cPassword: string = "";
        cPort: number = -1;

        private ipv4Array: Array<string> = new Array<string>();

        analysizeScheme(input: string): boolean {
            if (!Char.isLetter(input.charAt(0))) {
                this.flags[INDEX_INVALID] = true;
                return false;
            }
            let sizeLen: number = input.length;
            for (let i = 1; i < sizeLen - 1; i++) {
                if (!isAlphaNum(input.charAt(i)) && input.charAt(i) != c'+' && input.charAt(i) != c'-' && input.charAt(i) != c'.') {
                    this.flags[INDEX_INVALID] = true;
                    return false;
                }
            }
            this.cScheme = input.toLowerCase();
            if (isSpecialScheme(this.cScheme)) {
                this.flags[INDEX_SPECIAL] = true;
            }
            return true;
        }

        private analysizeFragment(fragment: string) {
            this.cFragment = fragment;
            this.flags[INDEX_FRAGMENT] = true;
        }

        private analysizeQuery(query: string) {
            this.cQuery = query;
            this.flags[INDEX_QUERY] = true;
        }

        private analysizeFilePath(input: string) {
            let temp = new Array<string>();
            let pos: int = 0;
            while ((pos = input.indexOf('/')) != -1 || (pos = input.indexOf('\\')) != -1) {
                temp.push(input.substring(0, pos));
                input = input.substring(pos + 1);
            }
            temp.push(input);
            let tempLength: number = temp.length;
            for (let i = 0; i < tempLength; i++) {
                if (Helper.doubleSegment.includes(temp[i])) {
                    if (this.cPath.length == 1 && isFileNotHost(this.cPath[0]) && this.cPath[0].length == 2) {
                        this.cPath[0] = this.cPath[0].charAt(0) + ':';
                    } else if (this.cPath.length > 0) {
                        this.cPath.pop();
                    }
                    if (i == temp.length - 1) {
                        this.cPath.push("");
                    }
                    continue;
                }

                if (Helper.singlesegment.includes(temp[i])) {
                    if (i == temp.length - 1) {
                        this.cPath.push("");
                    }
                    continue;
                }
                this.cPath.push(temp[i]);
                this.flags[INDEX_PATH] = true;
            }

            let it = this.cPath[0];
            if (Char.isLetter(it.charAt(0)) && (it.charAt(1) == c':' || it.charAt(1) == c'|')) {
                if (it.length == 2) {
                    this.cPath[0] = it[0] + ':';
                    this.flags[INDEX_HOST] = false;
                    this.cHost = "";
                }
            }
        }

        private analysizeSpecialFile(input: string, pos: int) {
            let strHost: string = input.substring(0, pos);
            let strPath: string = input.substring(pos + 1);
            let special: boolean = true;
            let isFile = isFileNotHost(strHost);
            if (!isFile) {
                this.analysizeHost(strHost, special);
            } else if (!isFile && this.flags[INDEX_INVALID]) {
                return;
            }
            if (!isFile) {
                this.analysizeFilePath(strPath);
            } else {
                this.analysizeFile(input);
            }
        }

        private analysizeIPv6Host(input: string) {
            let regex: RegExp = new RegExp("(::|(:((:[0-9A-Fa-f]{1,4}){1,7}))|(([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|:))|(((:(:[0-9A-Fa-f]{1,4}){0,5}:)|(([0-9A-Fa-f]{1,4}:){1}(:[0-9A-Fa-f]{1,4}){0,4}:)|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){0,3}:)|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}:)|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4})?:)|(([0-9A-Fa-f]{1,4}:){5}:)|(([0-9A-Fa-f]{1,4}:){6}))((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)))(%[a-zA-Z0-9._]+)?")
            if (!regex.test(input)) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            let pos: int = input.indexOf('.');
            if (pos != -1) {
                input = dealIPv4(input);
            }
            input = formatIPv6(input);
            input = compress(input);
            this.cHost = "[" + input + "]";
            this.flags[INDEX_HOST] = true;
            this.flags[INDEX_IPV6] = true
        }

        private removalIPv4(inputStr: string): boolean {
            let pos: int = 0;
            let left: int = 0;
            pos = inputStr.indexOf(".", left);
            while (pos != -1) {
                this.ipv4Array.push(inputStr.substring(left, pos));
                left = pos + 1;
                pos = inputStr.indexOf(".", left);
            }

            if (left != inputStr.length) {
                this.ipv4Array.push(inputStr.substring(left));
            }

            let inputSize: number = this.ipv4Array.length;
            let res = new Array<string>();
            for (let i = 0; i < inputSize; i++) {
                let conversion: number[] = [0];
                if (isNumber(this.ipv4Array[i], conversion)) {
                    res.push(BinaryConversion(this.ipv4Array[i], conversion[0]));
                } else {
                    return false;
                }
            }
            this.ipv4Array = res;
            let isIPv4 = true;
            for (let i = 0; i < inputSize; i++) {
                if (res[i] == '') {
                    isIPv4 = false;
                    this.flags[INDEX_INVALID] = true;
                    if (i == inputSize - 1) {
                        res.push('');
                        this.flags[INDEX_INVALID] = false;
                    }
                }
            }
            return isIPv4;
        }

        private formatIPv4(nums: Array<string>) {
            let len: int = nums.length;
            let index: int = getFormatIPv4Index(nums);
            let res: string = "";
            if (index == -1) {
                for (let i = 0; i < len - 1; i++) {
                    res += nums[i] + '.';
                }
                for (let i = 0; i < (4 - len); i++) {
                    res += "0.";
                }
                res += nums[len - 1];
                this.cHost = res;
                this.flags[INDEX_HOST] = true;
            } else if (index == (len - 1)) {
                for (let i = 0; i < (len - 1); i++) {
                    res += nums[i] + ".";
                }
                let storeNumber: number[] = [0];
                let temp: string = splitNum(nums[index], storeNumber);
                if (storeNumber[0] + (len - 1) > 4) {
                    this.flags[INDEX_INVALID] = true;
                    return;
                }
                for (let i = 0; i < (4 - (len - 1 + storeNumber[0])); i++) {
                    temp = "0." + temp;
                }
                this.cHost = res + temp;
                this.flags[INDEX_HOST] = true;
            } else {
                this.flags[INDEX_INVALID] = true;
            }
        }

        private analysizeIPv4(input: string) {
            let isIPv4: boolean = false;
            isIPv4 = this.removalIPv4(input);
            let temp = this.ipv4Array;
            let tempLen: int = temp.length;
            let lastSize: number = temp[tempLen - 1].length;
            if (isIPv4 && lastSize > 8) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            let res: string = "";
            for (let i = 0; i < tempLen; i++) {
                res += temp[i];
                if (i != (tempLen - 1)) {
                    res += '.';
                }
            }
            if (isIPv4) {
                if (tempLen > 4) {
                    res = res.toLowerCase();
                    this.cHost = res;
                    this.flags[INDEX_HOST] = true;
                } else if (tempLen == 4) {
                    if (getFormatIPv4Index(temp) == -1) {
                        this.cHost = res;
                        this.flags[INDEX_HOST] = true;
                    } else {
                        this.flags[INDEX_INVALID] = true;
                    }
                } else {
                    this.formatIPv4(temp);
                }
            } else {
                res = res.toLowerCase();
                this.cHost = res;
                this.flags[INDEX_HOST] = true;
            }
        }

        private analysizeOpaqueHost(input: string) {
            let inputSize: number = input.length;
            for (let i = 0; i < inputSize; i++) {
                let ch = new Char(input.charAt(i));
                let result = Helper.specialcharacter.indexOf(ch.toString());
                if ((ch != c'%') && (result != -1)) {
                    this.flags[INDEX_INVALID] = true;
                    return;
                }
            }
            this.cHost = input;
            this.flags[INDEX_HOST] = true;
        }

        analysizeHost(input: string, special: boolean) {
            if (input.length == 0) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            let inputLength = input.length;
            if (input.charAt(0) == c'[') {
                if (input.charAt(inputLength.toInt() - 1) == c']') {
                    input = input.substring(1, inputLength - 1);
                    this.analysizeIPv6Host(input);
                    return;
                }
                this.flags[INDEX_INVALID] = true;
                return;
            }
            if (!special) {
                this.analysizeOpaqueHost(input);
                return;
            }
            let decodedInput = DecodeSpecialChars(input);
            if (!checkCharacter(decodedInput)) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            this.analysizeIPv4(decodedInput);
        }

        private analysizeFile(input: string) {
            let special: boolean = true;
            const firstChar = input.length > 0 ? input.charAt(0) : '';
            const secondChar = input.length > 1 ? input.charAt(1) : '';
            if ((firstChar == c'/' || firstChar == c'\\') &&
                (secondChar == c'/' || secondChar == c'\\'))
            {
                let temp: string = input.substring(2);
                let pos: int = 0;
                if (((pos = temp.indexOf('/')) != -1 || (pos = temp.indexOf('\\')) != -1) && pos == 0) {
                    temp = temp.substring(1);
                    this.analysizeFilePath(temp);
                } else if (((pos = temp.indexOf('/')) != -1 || (pos = temp.indexOf('\\')) != -1) && pos != 0) {
                    this.analysizeSpecialFile(temp, pos);
                } else {
                    if (temp.length != 0 && this.flags[INDEX_INVALID]) {
                        this.analysizeHost(temp, special);
                    } else if (temp.length != 0 && !this.flags[INDEX_INVALID]) {
                        this.analysizeHost(temp, special);
                        return;
                    }
                }
            } else {
                if (firstChar == c'/' || firstChar == c'\\') {
                    input = input.substring(1);
                }
                this.analysizeFilePath(input);
            }
        }

        private analysizeUsernameAndPasswd(strHost: string): string {
            let pos: int = strHost.length - 1;
            for (; pos >= 0; pos--) {
                if (strHost.charAt(pos) == c'@') {
                    break;
                }
            }
            let userAndPasswd = strHost.substring(0, pos);
            strHost = strHost.substring(pos + 1);
            if (userAndPasswd.length == 0) {
                return strHost;
            }
            if (userAndPasswd.indexOf('@') != -1) {
                while (true) {
                    let posTemp = userAndPasswd.indexOf('@');
                    if (posTemp != -1) {
                        userAndPasswd = userAndPasswd.replace('@', "%40");
                    } else {
                        break;
                    }
                }
            }
            if (userAndPasswd.indexOf(':') != -1) {
                let position: int = userAndPasswd.indexOf(':');
                let user: string = userAndPasswd.substring(0, position);
                let keyWord: string = userAndPasswd.substring(position + 1);
                if (user.length != 0) {
                    this.cUsername = user;
                    this.flags[INDEX_USERNAME] = true;
                }
                if (keyWord.length != 0) {
                    this.cPassword = keyWord;
                    this.flags[INDEX_PASSWORD] = true;
                }
            } else {
                this.cUsername = userAndPasswd;
                this.flags[INDEX_USERNAME] = true;
            }
            return strHost;
        }

        analysizePort(port: string) {
            if (port.length == 0) {
                return;
            }
            for (let i = 0; i < port.length; i++) {
                if (!Char.isDecDigit(port.charAt(i))) {
                    this.flags[INDEX_INVALID] = true;
                    return;
                }
            }
            if (port.length >= 6) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            let it: number = parseInt(port);
            const maxPort: number = 65535;
            if (it > maxPort) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            this.flags[INDEX_PORT] = true;
            for (let key of Helper.head.keys()) {
                let value = Helper.head.get(key);
                if (key == this.cScheme && value == it) {
                    this.cPort = -1;
                    this.flags[INDEX_PORT] = false;
                    return;
                }
            }
            this.cPort = it;
        }

        analysizePath(path: string, special: boolean) {
            let temp = new Array<string>();
            let pos: int = 0;
            while ((pos = path.indexOf('/')) != -1 || ((pos = path.indexOf('\\')) != -1 && special)) {
                temp.push(path.substring(0, pos));
                path = path.substring(pos + 1);
            }
            temp.push(path);
            let tempLen: number = temp.length;
            for (let it = 0; it < tempLen; it++) {
                let result: number = Helper.doubleSegment.indexOf(temp[it]);
                if (result != -1) {
                    if (this.cPath.length == 0 && it == (tempLen - 1)) {
                        this.cPath.push("");
                        this.flags[INDEX_PATH] = true;
                    }
                    if (this.cPath.length == 0) {
                        continue;
                    }
                    this.cPath.pop();
                    if (it == (tempLen - 1)) {
                        this.cPath.push("");
                        this.flags[INDEX_PATH] = true;
                    }
                    continue;
                }

                result = Helper.singlesegment.indexOf(temp[it]);
                if (result != -1 && it == (tempLen - 1)) {
                    this.cPath.push("");
                    this.flags[INDEX_PATH] = true;
                    continue;
                }
                if (result == -1) {
                    this.cPath.push(temp[it]);
                    this.flags[INDEX_PATH] = true;
                }
            }
        }

        private parsingHostAndPath(input: string, pos: int) {
            let special: boolean = true;
            let inputLen: int = input.length;
            for (pos = 0; pos < inputLen; pos++) {
                if (input.charAt(pos) == c'/' || input.charAt(pos) == c'\\') {
                    break;
                }
            }
            let strHost: string = input.substring(0, pos);
            let strPath: string = input.substring(pos + 1);
            if (strHost.indexOf('@') != -1) {
                strHost = this.analysizeUsernameAndPasswd(strHost);
            }
            if (strHost.length == 0) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            if (strHost.charAt(strHost.length - 1) != c']' && (strHost.lastIndexOf(':') != -1)) {
                pos = strHost.lastIndexOf(':');
                if (pos != -1) {
                    let port: string = strHost.substring(pos + 1);
                    strHost = strHost.substring(0, pos);
                    this.analysizePort(port);
                }
            }
            if (strHost.charAt(strHost.length - 1) != c']' && strHost.lastIndexOf(':') != -1 &&
                this.flags[INDEX_INVALID]) 
            {
                return;
            }
            this.analysizeHost(strHost, special);
            this.analysizePath(strPath, special);
        }

        private analysizeOnlyHost(input: string, pos: int) {
            let strHost: string = input;
            if (strHost.indexOf('@') != -1) {
                strHost = this.analysizeUsernameAndPasswd(strHost);
            }
            if (strHost.length == 0) {
                this.flags[INDEX_INVALID] = true;
                return;
            }
            if (strHost.charAt(strHost.length - 1) != c']') {
                if ((pos = strHost.lastIndexOf(':')) != -1) {
                    let port: string = strHost.substring(pos + 1);
                    strHost = strHost.substring(0, pos);
                    this.analysizePort(port);
                }
                if ((pos = strHost.lastIndexOf(':')) != -1 && this.flags[INDEX_INVALID]) {
                    return;
                }
            }
            this.analysizeHost(strHost, true);
        }

        private analysizeFilescheme(input: string) {
            let strPath: string = this.cScheme + input;
            this.cScheme = "file:";
            this.flags[INDEX_SPECIAL] = true;
            this.analysizeFilePath(strPath);
        }

        private analysizeStrHost(input: string): string {
            if (input.indexOf('@') != -1) {
                input = this.analysizeUsernameAndPasswd(input);
            }
            if (input.length == 0) {
                this.flags[INDEX_INVALID] = true;
            }
            return input;
        }

        private analysizeHostPath(input: string): string {
            let pos: int = 0;
            if (input.charAt(input.length - 1) != c']' && (pos = input.lastIndexOf(':')) != -1) {
                let port: string = input.substring(pos + 1);
                input = input.substring(0, pos);
                this.analysizePort(port);
                if (this.flags[INDEX_INVALID]) {
                    return input;
                }
            }
            return input;
        }

        private analysizeInfoPath(input: string) {
            this.flags[INDEX_INFOPATH] = true;
            if (this.cPath.length == 0) {
                this.cPath.push("");
            }
            this.cPath[0] = input;
            this.flags[INDEX_PATH] = true;
            return;
        }

        private analysizeNoDefaultProtocol(input: string) {
            if (this.cScheme.length == 2) {
                this.analysizeFilescheme(input);
                return;
            }
            if (input.charAt(0) == c'/' && input.charAt(1) == c'/' && input.charAt(2) != c'/') {
                let hostAndPath: string = input.substring(2);
                if (hostAndPath.length == 0) {
                    return;
                }
                let i: int = 0;
                let special: boolean = false;
                let strHost: string = "";
                if (hostAndPath.indexOf('/') != -1) {
                    i = hostAndPath.indexOf('/');
                    strHost = hostAndPath.substring(0, i);
                    let strPath: string = hostAndPath.substring(i + 1);
                    if (strHost.indexOf('@') != -1) {
                        strHost = this.analysizeUsernameAndPasswd(strHost);
                    }
                    if (strHost.length == 0) {
                        this.flags[INDEX_INVALID] = true;
                        return;
                    }
                    let pos: int = 0;
                    if (strHost.charAt(strHost.length - 1) != c']' && (pos = strHost.lastIndexOf(':')) != -1) {
                        let port: string = strHost.substring(pos + 1);
                        strHost = strHost.substring(0, pos);
                        this.analysizePort(port);
                    }
                    if (strHost.charAt(strHost.length - 1) != c']' && (pos = strHost.lastIndexOf(':')) != -1 &&
                        this.flags[INDEX_INVALID])
                    {
                        return;
                    }
                    this.analysizeHost(strHost, special);
                    this.analysizePath(strPath, special);
                } else {
                    strHost = hostAndPath;
                    strHost = this.analysizeStrHost(strHost);
                    strHost = this.analysizeHostPath(strHost);
                    this.analysizeHost(strHost, special);
                }
            } else if (input.charAt(0) == c'/' && input.charAt(1) == c'/') {
                let strOfPath = input.substring(1);
                this.analysizePath(strOfPath, false);
            } else {
                this.analysizeInfoPath(input);
            }
        }

        private analysizeHostAndPath(input: string) {
            if (this.flags[INDEX_SPECIAL]) {
                let pos: int = 0;
                let inputLen: int = input.length;
                while (pos < inputLen) {
                    if (input.charAt(pos) == c'/' || input.charAt(pos) == c'\\') {
                        pos++;
                        continue;
                    }
                    break;
                }
                input = input.substring(pos);
                if (input.length == 0) {
                    this.flags[INDEX_INVALID] = true;
                    return;
                } else if (input.indexOf('/') != -1 || input.indexOf('\\') != -1) {
                    this.parsingHostAndPath(input, pos);
                } else if (input.length != 0 && input.indexOf('/') == -1 && input.indexOf('\\') == -1) {
                    this.analysizeOnlyHost(input, pos);
                }
            } else {
                let inputLen: number = input.length;
                if (inputLen > 0) {
                    this.cIsSpecialPath = input.charAt(0) != c'/';
                }
                if (this.flags[INDEX_DOUBLE_SLASH] && inputLen == 2) {
                    return;
                }
                this.analysizeNoDefaultProtocol(input);
            }
        }

        initOnlyInput(input: string): string {
            if (input.length == 0) {
                this.flags[INDEX_INVALID] = true;
                return input;
            }
            let tempInput = input;
            if (tempInput.indexOf(':') != -1) {
                let pos = tempInput.indexOf(':')
                pos++;
                this.flags[INDEX_DOUBLE_SLASH] = ((pos + 1 < tempInput.length) && (input.charAt(pos) == c'/') && (input.charAt(pos + 1) == c'/'));
                let scheme: string = tempInput.substring(0, pos);
                if (!this.analysizeScheme(scheme)) {
                    return tempInput;
                }
                if (tempInput.indexOf('#') != -1) {
                    let posTemp = tempInput.indexOf('#');
                    let fragment: string = tempInput.substring(posTemp);
                    this.analysizeFragment(fragment);
                    tempInput = tempInput.substring(0, posTemp);
                }
                if (tempInput.indexOf('?') != -1) {
                    let posTemp = tempInput.indexOf('?');
                    let query: string = tempInput.substring(posTemp);
                    this.analysizeQuery(query);
                    tempInput = tempInput.substring(0, posTemp);
                }
                let restInput: string = tempInput.substring(pos);
                if (this.cScheme == "file:") {
                    this.analysizeFile(restInput);
                } else {
                    this.analysizeHostAndPath(restInput);
                }
            } else {
                this.flags[INDEX_INVALID] = true;
            }

            return tempInput;
        }

        analysizeInput(input: string): string {
            let pos: int = 0;
            if (input.indexOf('#') != -1) {
                pos = input.indexOf('#');
                let fragment: string = input.substring(pos);
                this.analysizeFragment(fragment);
                input = input.substring(0, pos);
            }
            if (input.indexOf('?') != -1) {
                pos = input.indexOf('?');
                let query: string = input.substring(pos);
                this.analysizeQuery(query);
                input = input.substring(0, pos);
            }
            let special: boolean = this.flags[INDEX_SPECIAL];
            this.analysizePath(input, special);
            return input;
        }

        toolHasBase(input: string, strInput: string): string {
            if (input.length != 0 && input.charAt(0) == c'/') {
                strInput = input.substring(1);
                return this.analysizeInput(strInput);
            } else if (input.length != 0 && input.charAt(0) != c'/') {
                return this.analysizeInput(strInput);
            }
            return strInput;
        }

        parseSingleUrl(inputUrl: string): string {
            inputUrl = deleteC0OrSpace(inputUrl);
            inputUrl = deleteTabOrNewline(inputUrl);
            return this.initOnlyInput(inputUrl);
        }

        constructor() {}

        getHostName(): string {
            let temp = "";
            if (this.flags[INDEX_HOST]) {
                if (!this.flags[INDEX_IPV6]) {
                    temp = encodePercentEncoding(this.cHost, Helper.URL_ENCODED_PERCENT_SIGN_CHARS);
                } else {
                    temp = this.cHost;
                }
            }
            return temp;
        }

        setHostName(input: string) {
            if (this.flags[INDEX_INFOPATH]) {
                return;
            }
            let strHost: string = input;
            let len = strHost.length
            for (let pos = 0; pos < len; pos++) {
                if ((strHost.charAt(pos) == c':') || (strHost.charAt(pos) == c'?') ||
                    (strHost.charAt(pos) == c'#') ||
                    (strHost.charAt(pos) == c'/' || strHost.charAt(pos) == c'\\'))
                {
                    strHost = strHost.substring(0, pos);
                    break;
                }
            }
            if (strHost.length == 0) {
                return;
            }
            let special: boolean = Helper.head.has(this.cScheme);
            let tempUrl: InnerURL = new InnerURL();
            tempUrl.analysizeHost(strHost, special);
            if (tempUrl.flags[INDEX_HOST]) {
                if (this.cScheme == "file:" && (tempUrl.cHost == "localhost")) {
                    tempUrl.cHost = "";
                }
                this.cHost = tempUrl.cHost;
                this.flags[INDEX_HOST] = true;
            }
        }

        setUserName(input: string) {
            if (input.length == 0) {
                this.cUsername = "";
                this.flags[INDEX_USERNAME] = false;
            } else {
                let username: string = encodePercentEncoding(input, Helper.USERINFO_PERCENT_SIGN_CHARS);
                Helper.spesicalSymbols.forEach((value: string, key: string) => {
                    username = username.replaceAll(key, value);
                });
                this.cUsername = username;
                this.flags[INDEX_USERNAME] = true;
            }
        }

        setPassword(input: string) {
            if (input.length == 0) {
                this.cPassword = "";
                this.flags[INDEX_PASSWORD] = false;
            } else {
                let keyWord: string = encodePercentEncoding(input, Helper.USERINFO_PERCENT_SIGN_CHARS);
                Helper.spesicalSymbols.forEach((value: string, key: string) => {
                    keyWord = keyWord.replaceAll(key, value);
                });
                this.cPassword = keyWord;
                this.flags[INDEX_PASSWORD] = true;
            }
        }

        setScheme(input: string) {
            let strInput: string = input;
            let special: boolean = Helper.head.has(this.cScheme);
            let inputIsSpecial: boolean = Helper.head.has(input);
            if ((special != inputIsSpecial) || ((input == "file") && (this.flags[INDEX_USERNAME] || this.flags[INDEX_PASSWORD] || this.flags[INDEX_PORT]))) {
                return;
            }
            let tempUrl: InnerURL = new InnerURL();
            if (tempUrl.analysizeScheme(strInput)) {
                if (tempUrl.flags[INDEX_SPECIAL]) {
                    this.flags[INDEX_SPECIAL] = true;
                }
                this.cScheme = tempUrl.cScheme;
            }
        }

        setFragment(input: string) {
            let temp: string = "";
            if (input.length == 0) {
                this.cFragment = "";
                this.flags[INDEX_FRAGMENT] = false;
            } else {
                if (input.charAt(0) != c'#') {
                    temp = '#';
                    temp += encodePercentEncoding(input, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
                } else {
                    temp = encodePercentEncoding(input, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
                }
                this.analysizeFragment(temp);
            }
        }

        setSearch(input: string) {
            let temp: string = "";
            if (input.length == 0) {
                this.cQuery = "";
                this.flags[INDEX_QUERY] = false;
            } else {
                if (input.charAt(0) != c'?') {
                    temp = "?";
                    temp += input;
                } else {
                    temp = input;
                }
                let oldStr = "#";
                let newStr = "%23";
                temp = temp.replaceAll(oldStr, newStr);
                this.analysizeQuery(temp);
            }
        }

        setEncodeSearch(input: string) {
            this.setSearch(encodePercentEncoding(input, Helper.QUERY_PERCENT_SIGN_CHARS));
        }

        setHost(input: string) {
            if (input.length == 0 || this.flags[INDEX_INFOPATH]) {
                return;
            }
            let strHost: string = input;
            let port: string = "";
            // splitString
            let strLen = input.length
            for (let pos = 0; pos < strLen; pos++) {
                if ((input.charAt(pos) == c':') || (input.charAt(pos) == c'?') ||
                    (input.charAt(pos) == c'#') || (input.charAt(pos) == c'/') || (input.charAt(pos) == c'\\'))
                {
                    strHost = input.substring(0, pos);
                    if (input.charAt(pos) == c':') {
                        pos++;
                        port = input.substring(pos);
                    }
                    break;
                }
            }

            if (strHost.length == 0) {
                return;
            }
            let special: boolean = Helper.head.has(this.cScheme);
            let tempUrl: InnerURL = new InnerURL();
            tempUrl.analysizeHost(strHost, special);
            if (tempUrl.flags[INDEX_HOST]) {
                if (this.cScheme == "file:" && tempUrl.cHost == "localhost") {
                    tempUrl.cHost = "";
                }
                this.cHost = tempUrl.cHost;
                this.flags[INDEX_HOST] = true;
            } else {
                return;
            }
            if (port.length > 0) {
                let portLen = port.length
                for (let pos = 0; pos < portLen; pos++) {
                    if (port.charAt(pos) == c'?' || port.charAt(pos) == c'#' ||
                        port.charAt(pos) == c'/' || port.charAt(pos) == c'\\')
                    {
                        port = port.substring(0, pos);
                        break;
                    }
                }
                if (port.length > 0) {
                    let tempPortUrl: InnerURL = new InnerURL();
                    tempPortUrl.analysizePort(port);
                    if (tempPortUrl.flags[INDEX_PORT]) {
                        this.flags[INDEX_PORT] = true;
                        this.cPort = tempPortUrl.cPort;
                    }
                }
            }
        }

        setPort(input: string) {
            let port: string = input;
            let portLen = port.length
            for (let pos = 0; pos < portLen; pos++) {
                if ((port.charAt(pos) == c'?') || (port.charAt(pos) == c'#') ||
                    (port.charAt(pos) == c'/') || (port.charAt(pos) == c'\\'))
                {
                    port = port.substring(0, pos);
                    break;
                }
            }
            if (port.length > 0) {
                let tempUrl: InnerURL = new InnerURL();
                tempUrl.analysizePort(port);
                if (tempUrl.flags[INDEX_PORT]) {
                    this.flags[INDEX_PORT] = true;
                    this.cPort = tempUrl.cPort;
                }
            }
        }

        setHref(input: string) {
            let str: string = input;
            str = deleteC0OrSpace(str);
            let newUrl: InnerURL = new InnerURL();
            newUrl.initOnlyInput(str);
            this.flags = newUrl.flags;
            if (!newUrl.flags[INDEX_INVALID]) {
                this.flags = newUrl.flags;
                this.cFragment = newUrl.cFragment;
                this.cScheme = newUrl.cScheme;
                this.cQuery = newUrl.cQuery;
                this.cPath = newUrl.cPath;
                this.cHost = newUrl.cHost;
                this.cIsSpecialPath = newUrl.cIsSpecialPath;
                this.cUsername = newUrl.cUsername;
                this.cPassword = newUrl.cPassword;
                this.cPort = newUrl.cPort;
            }
        }

        setPath(input: string) {
            let strPath: string = encodePercentEncoding(input, Helper.PATH_PERCENT_SIGN_CHARS);
            if (this.flags[INDEX_INFOPATH] || strPath.length == 0) {
                return;
            }
            let oldStr: string = "%3A";
            let newStr: string = ":";
            strPath = strPath.replaceAll(oldStr, newStr);
            let special: boolean = Helper.head.has(this.cScheme);
            if (this.cScheme == "file:") {
                let tempUrl: InnerURL = new InnerURL();
                if ((strPath.charAt(0) == c'/') || (strPath.charAt(0) == c'\\' && this.flags[INDEX_SPECIAL])) {
                    strPath = strPath.substring(1);
                }
                tempUrl.analysizeFilePath(strPath);
                if (tempUrl.flags[INDEX_PATH]) {
                    this.cPath = tempUrl.cPath;
                    this.flags[INDEX_PATH] = true;
                }
            } else {
                let tempUrl: InnerURL = new InnerURL();
                if ((strPath.charAt(0) == c'/') || (strPath.charAt(0) == c'\\' && this.flags[INDEX_SPECIAL])) {
                    strPath = strPath.substring(1);
                }
                tempUrl.analysizePath(strPath, special);
                if (tempUrl.flags[INDEX_PATH]) {
                    this.cPath = tempUrl.cPath;
                    this.flags[INDEX_PATH] = true;
                }
            }
        }

        getSearch(): string {
            if (this.flags[INDEX_QUERY] && (this.cQuery.length != 1)) {
                return this.cQuery;
            }
            return "";
        }

        getEncodeSearch(): string {
            if (this.flags[INDEX_QUERY] && (this.cQuery.length != 1)) {
                return encodePercentEncoding(this.cQuery, Helper.QUERY_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getUsername(): string {
            if (this.flags[INDEX_USERNAME]) {
                return encodePercentEncoding(this.cUsername, Helper.USERINFO_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getPassword(): string {
            if (this.flags[INDEX_PASSWORD]) {
                return encodePercentEncoding(this.cPassword, Helper.USERINFO_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getFragment(): string {
            if (this.flags[INDEX_FRAGMENT] && (this.cFragment.length != 1)) {
                return encodePercentEncoding(this.cFragment, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getScheme(): string {
            if (this.cScheme.length != 0) {
                return this.cScheme;
            }
            return "";
        }

        getPath(): string {
            let temp: string = "/";
            if (this.cIsSpecialPath) {
                temp = "";
            }
            if (this.flags[INDEX_PATH]) {
                let len: number = this.cPath.length;
                for (let i = 0; i < len; i++) {
                    if (i < len - 1) {
                        temp += this.cPath[i] + "/";
                    } else {
                        temp += this.cPath[i];
                    }
                }
            } else {
                let special: boolean = Helper.head.has(this.cScheme);
                if (!special) {
                    temp = "";
                }
            }
            temp = encodePercentEncoding(temp, Helper.PATH_PERCENT_SIGN_CHARS);
            return temp;
        }

        getPort(): string {
            if (this.flags[INDEX_PORT]) {
                return Number.toString(this.cPort);
            }
            return "";
        }

        getHost(): string {
            let temp: string = this.cHost;
            if (this.flags[INDEX_PORT]) {
                temp += ":";
                temp += Number.toString(this.cPort);
            }
            if (!this.flags[INDEX_IPV6]) {
                temp = encodePercentEncoding(temp, Helper.URL_ENCODED_PERCENT_SIGN_CHARS);
            }
            return temp;
        }

        getIsIpv6(): boolean {
            return this.flags[INDEX_IPV6];
        }

        hasDoubleSlash(): boolean {
            return this.flags[INDEX_DOUBLE_SLASH];
        }

    }

    function baseIntoUrl(base: InnerURL, res: InnerURL, inputIsEmpty: boolean) {
        res.cScheme = base.cScheme;
        res.flags[INDEX_SPECIAL] = base.flags[INDEX_SPECIAL];

        res.cHost = base.cHost;
        res.flags[INDEX_HOST] = true

        res.cUsername = base.cUsername;
        res.flags[INDEX_USERNAME] = base.flags[INDEX_USERNAME];

        res.cPassword = base.cPassword;
        res.flags[INDEX_PASSWORD] = base.flags[INDEX_PASSWORD];

        res.cPort = base.cPort;
        res.flags[INDEX_PORT] = base.flags[INDEX_PORT];

        if (inputIsEmpty) {
            res.cPath = base.cPath;
            res.flags[INDEX_PATH] = base.flags[INDEX_PATH];

            res.cQuery = base.cQuery;
            res.flags[INDEX_QUERY] = base.flags[INDEX_QUERY];

            res.cFragment = base.cFragment;
            res.flags[INDEX_FRAGMENT] = base.flags[INDEX_FRAGMENT];
        }
        res.flags[INDEX_INFOPATH] = base.flags[INDEX_INFOPATH];
        res.flags[INDEX_IPV6] = base.flags[INDEX_IPV6];
    }

    function shorteningPath(resUrl: InnerURL, baseUrl: InnerURL, isFile: boolean) {
        if (baseUrl.cPath.length == 0) {
            return;
        }
        if (resUrl.cPath.length == 1 && resUrl.cPath[0].length == 0) {
            resUrl.cPath.pop();
            return;
        }

        let ch: char = baseUrl.cPath[0].charAt(0);
        let ch1: char = baseUrl.cPath[0].charAt(1);
        if ((baseUrl.cPath.length == 1) && isFile && Char.isLetter(ch) && ch1 == c':') {
            return;
        }
        baseUrl.cPath.pop();
    }

    function basePathToString(base: InnerURL): string {
        let temp: string = "";
        let len: number = base.cPath.length;
        for (let i = 0; i < len; i++) {
            if (i < len - 1) {
                temp += base.cPath[i] + "/";
            } else {
                temp += base.cPath[i];
            }
        }
        return temp;
    }

    function setEncodedData(dst: URL, src: InnerURL) {
        dst.cSearch = src.getEncodeSearch();
        dst.cUsername = src.getUsername();
        dst.cPassword = src.getPassword();
        dst.cHostname = src.getHostName();
        dst.cHost = src.getHost();
        dst.cHash = src.getFragment();
        dst.cProtocol = src.getScheme();
        dst.cPathname = src.getPath();
        dst.cPort = src.getPort();
        dst.cOrigin = src.getScheme() + '//' + src.getHost();
        dst.cParams = new URLParams(src.getEncodeSearch());
        dst.cParams.parentUrl = dst;
        dst.cHasDoubleSlash = src.hasDoubleSlash();
        dst.setHref();
    }

    function containIllegalCode(str: string): Boolean {
        const regex = new RegExp("(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])");
        return regex.test(str);
    }

    function fixIllegalString(str: string): string {
        if (containIllegalCode(str)) {
            return str.toWellFormed();
        } else {
            return str;
        }
    }

    /**
    * The interface of URL is used to parse, construct, normalize, and encode URLs.
    *
    * @class URL
    */
    export class URL implements JsonSerializable {

        urlInner: InnerURL = new InnerURL();

        cHref: string = '';
        cSearch: string = '';
        cOrigin: string = '';
        cUsername: string = '';
        cPassword: string = '';
        cHostname: string = '';
        cHost: string = '';
        cHash: string = '';
        cProtocol: string = '';
        cPathname: string = '';
        cPort: string = '';
        cHasDoubleSlash: boolean = false;
        cParams: URLParams = new URLParams();
        /**
        * URL constructor, which is used to instantiate a URL object.
        *
        */
        constructor() {}

        /**
        * Replaces the original constructor to process arguments and return a url object.
        *
        * @param { string } inputUrl -  Absolute or relative input URL to resolve. 
        * Base is required if input is relative.
        * If input is an absolute value, base ignores the value.
        * @param { string | URL } [base] - Base URL to parse if input is not absolute.
        * @returns { URL }
        * @throws { BusinessError } 10200002 - Invalid url string.
        */
        static parseURL(inputUrl: string, inputBase?: string | URL): URL {
            let nativeUrl: URL;
            if (inputBase == undefined) {
                nativeUrl = URL.parseSingleInput(fixIllegalString(inputUrl));
            } else {
                if (inputBase instanceof string) {
                    nativeUrl = URL.parseBaseInput(fixIllegalString(inputUrl), fixIllegalString(inputBase));
                }
                if (inputBase instanceof URL) {
                    nativeUrl = URL.parseBaseUrl(fixIllegalString(inputUrl), inputBase);
                }
            }
            if (nativeUrl!.urlInner.flags[INDEX_INVALID]) {
                throw createBusinessError(SyntaxErrorCodeId, 'Syntax Error. Invalid Url string.');
            }
            setEncodedData(nativeUrl!, nativeUrl!.urlInner);
            return nativeUrl!;
        }

        /**
        * Returns the serialized URL as a string.
        *
        * @returns { string } Returns the serialized URL as a string.
        */
        toString(): string {
            return this.cHref;
        }

        /**
        * Gets the protocol portion of the URL.
        *
        * @return  {string} Returns the protocol portion of the URL.
        */
        get protocol(): string {
            return this.cProtocol;
        }

        /**
        * Sets the protocol portion of the URL.
        * 
        * @param { string } scheme - protocol portion of the URL.
        */
        set protocol(scheme: string) {
            if (scheme.length == 0) {
                return;
            }
            if (this.cProtocol == "file:" && (this.cHost == "" || this.cHost == null)) {
                return;
            }
            this.urlInner.setScheme(scheme);
            this.cProtocol = this.urlInner.getScheme();
            this.cHost = this.urlInner.getHost();
            this.cPort = this.urlInner.getPort();
            this.setHref();
        }

        /**
        * Gets the origin portion of the URL.
        * 
        * @return  {string} Returns the origin portion of the URL.
        */
        get origin(): string {
            let kOpaqueOrigin: string = 'null';
            switch (this.cProtocol) {
                case 'ftp:':
                case 'gopher:':
                case 'http:':
                case 'https:':
                case 'ws:':
                case 'wss:':
                    return this.cProtocol + '//' + this.cHost;
            }
            return kOpaqueOrigin;
        }

        /**
        * Gets the username portion of the URL.
        * 
        * @return  {string} Returns the username portion of the URL.
        * @type { string }
        */
        get username(): string {
            return this.cUsername;
        }

        /**
        * Sets the username portion of the URL.
        * 
        * @param { string }  input - The username portion of the URL.
        */
        set username(input: string) {
            if (this.cHost == null || this.cHost == '' || this.cProtocol == 'file:') {
                return;
            }
            this.urlInner.setUserName(fixIllegalString(input));
            this.cUsername = this.urlInner.getUsername();
            this.setHref();
        }

        /**
        * Gets the password portion of the URL.
        * 
        * @return  {string} Returns the password portion of the URL.
        */
        get password(): string {
            return this.cPassword;
        }

        /**
        * Sets the password portion of the URL.
        * 
        * @param { string } input - The password portion of the URL.
        */
        set password(input: string) {
            if (this.cHost == null || this.cHost == '' || this.cProtocol == 'file:') {
                return;
            }
            this.urlInner.setPassword(fixIllegalString(input));
            this.cPassword = this.urlInner.getPassword();
            this.setHref();
        }

        /**
        * Gets the serialized URL as a string.
        * 
        * @return  {string} Returns the serialized URL as a string.
        */
        get hash(): string {
            return this.cHash;
        }

        /**
        * Sets the hash portion of the URL.
        * 
        * @param { string } fragment - The hash portion of the URL.
        */
        set hash(fragment: string) {
            this.urlInner.setFragment(fixIllegalString(fragment));
            this.cHash = this.urlInner.getFragment();
            this.setHref();
        }

        /**
        * Gets the search portion of the URL.
        * 
        * @return  {string} Returns the search portion of the URL.
        */
        get search(): string {
            return this.cSearch;
        }

        /**
        * Sets the search portion of the URL.
        * 
        * @param { string }  query - The search portion of the URL.
        */
        set search(query: string) {
            this.urlInner.setEncodeSearch(fixIllegalString(query));
            this.cSearch = this.urlInner.getEncodeSearch();
            this.cParams.updateParams(this.cSearch);
            this.setHref();
        }

        /**
        * Gets the hostname portion of the URL.
        * 
        * @return  {string} Returns the hostname portion of the URL.
        */
        get hostname(): string {
            return this.cHostname;
        }

        /**
        * Sets the hostname portion of the URL.
        * 
        * @param { string } hostname - The hostname portion of the URL.
        */
        set hostname(hostname: string) {
            this.urlInner.setHostName(hostname);
            this.cHostname = this.urlInner.getHostName();
            this.cHost = this.urlInner.getHost();
            this.setHref();
        }

        /**
        * Gets the host portion of the URL.
        * 
        * @return  {string} Returns the host portion of the URL.
        */
        get host(): string {
            return this.cHost;
        }

        /**
        * Sets the host portion of the URL.
        * 
        * @param { string } host - The host portion of the URL.
        */
        set host(host: string) {
            this.urlInner.setHost(host);
            this.cHost = this.urlInner.getHost();
            this.cHostname = this.urlInner.getHostName();
            this.cPort = this.urlInner.getPort();
            this.setHref();
        }

        /**
        * Gets the port portion of the URL.
        * 
        * @return  {string} Returns the port portion of the URL.
        */
        get port(): string {
            return this.cPort;
        }

        /**
        * Sets the port portion of the URL.
        * 
        * @param { string }  port - The port portion of the URL.
        */
        set port(port: string) {
            if (this.cHost == '' || this.cProtocol == 'file:' || port == '') {
                return;
            }
            this.urlInner.setPort(port);
            this.cPort = this.urlInner.getPort();
            this.cHost = this.urlInner.getHost();
            this.setHref();
        }

        /**
        * Gets the pathname portion of the URL.
        * 
        * @return  {string} Returns the pathname portion of the URL.
        */
        get pathname(): string {
            return this.cPathname;
        }

        /**
        * Sets the pathname portion of the URL.
        * 
        * @param { string }  path - The pathname portion of the URL.
        */
        set pathname(path: string) {
            this.urlInner.setPath(fixIllegalString(path));
            this.cPathname = this.urlInner.getPath();
            this.setHref();
        }

        /**
        * Gets the params portion of the URL.
        * 
        * @return  {URLParams} Returns the params portion of the URL.
        */
        get params(): URLParams {
            return this.cParams;
        }

        /**
        * Returns the serialized URL as a string.
        *
        * @returns { string } Returns the serialized URL as a string.
        */
        override toJSON(): string {
            return this.cHref;
        }

        /**
        * Gets the href portion of the URL.
        * 
        * @return  {string} Returns the href portion of the URL.
        */
        get href(): string {
            return this.cHref;
        }

        /**
        * Sets the href portion of the URL.
        * 
        * @param { string } href - The href portion of the URL.
        */
        set href(href: string) {
            this.urlInner.setHref(href);
            if (!this.urlInner.flags[INDEX_INVALID]) {
                setEncodedData(this, this.urlInner);
            } else {
                throw createBusinessError(SyntaxErrorCodeId, 'Syntax Error. Invalid Url string.');
            }
        }

        setHref(): void {
            let temp: string = this.cProtocol;
            if (this.cHostname != "" || this.cProtocol == "file:" || this.cHasDoubleSlash) {
                temp += "//";
            }
            if (this.cHostname != "") {
                if (this.cPassword != "" || this.cUsername != "") {
                    if (this.cUsername != "") {
                        temp += this.cUsername;
                    }
                    if (this.cPassword != "") {
                        temp += ":";
                        temp += this.cPassword;
                    }
                    temp += "@";
                }
                temp += this.cHostname;
                if (this.cPort != "") {
                    temp += ":";
                    temp += this.cPort;
                }
            }
            temp += this.cPathname;
            if (this.cSearch) {
                temp += this.cSearch;
            }
            if (this.cHash) {
                temp += this.cHash;
            }
            this.cHref = temp;
        }

        static parseSingleInput(input: string): URL {
            let result = new URL();
            let url = new InnerURL();
            url.parseSingleUrl(input);
            result.urlInner = url;

            return result;
        }

        static parseBaseInput(input: string, base: string): URL {
            if (base.length == 0) {
                throw createBusinessError(SyntaxErrorCodeId, 'Parameter error. The type of  must be string')
            }
            let result = new URL();
            let possibleBaseURL = new InnerURL();
            let resUrl = new InnerURL();
            let strInput = input;
            possibleBaseURL.parseSingleUrl(base);
            if (possibleBaseURL.flags[INDEX_INVALID]) {
                resUrl.flags[INDEX_INVALID] = true
                result.urlInner = resUrl;
                return result;
            }
            strInput = resUrl.parseSingleUrl(strInput);
            if (!resUrl.flags[INDEX_INVALID]) {
                result.urlInner = resUrl;
                return result;
            }
            let needReturn = false;
            const baseResults = URL.parseBaseInner(input, possibleBaseURL, resUrl, result, needReturn);
	        let firstChar = baseResults[0];
	        needReturn = baseResults[1];
            if (needReturn) {
                return result;
            }
            if (!possibleBaseURL.flags[INDEX_INFOPATH]) {
                resUrl.flags[INDEX_INVALID] = false
                baseIntoUrl(possibleBaseURL, resUrl, input.length == 0);
                strInput = resUrl.toolHasBase(input, strInput);
                if (input.length != 0 && input.charAt(0) != c'/' && resUrl.cPath.length == 0) {
                    resUrl.cPath = possibleBaseURL.cPath;
                    resUrl.flags[INDEX_PATH] = possibleBaseURL.flags[INDEX_PATH];
                }
                if (input.length != 0 && input.charAt(0) != c'/' && resUrl.cPath.length != 0) {
                    let isFile: boolean = resUrl.cScheme == "file:";
                    shorteningPath(resUrl, possibleBaseURL, isFile);
                    let basePathStr: string = basePathToString(possibleBaseURL);
                    basePathStr == "" ? basePathStr = strInput : basePathStr += '/' + strInput;
                    resUrl.cPath = new Array<string>();
                    resUrl.analysizeInput(basePathStr);
                    resUrl.flags[INDEX_PATH] = true
                }
            } else if (possibleBaseURL.flags[INDEX_INFOPATH]) {
                resUrl.flags[INDEX_INVALID] = true
                result.urlInner = resUrl;
                return result;
            }
            result.urlInner = resUrl;
            return result;
        }

        static parseBaseUrl(input: string, baseUrl: URL): URL {
            let base: InnerURL = baseUrl.urlInner;
            let result: URL = new URL();
            let resUrl: InnerURL = new InnerURL();
            let strInput: string = input;
            resUrl.parseSingleUrl(strInput);
            if (!resUrl.flags[INDEX_INVALID]) {
                result.urlInner = resUrl;
                return result;
            }
            let needReturn = false;
            const baseResults = URL.parseBaseInner(input, base, resUrl, result, needReturn);
	        let firstChar = baseResults[0];
	        needReturn = baseResults[1];
            if (needReturn) {
                return result;
            }
            if (!base.flags[INDEX_INFOPATH]) {
                resUrl.flags[INDEX_INVALID] = false
                baseIntoUrl(base, resUrl, input.length == 0);
                strInput = resUrl.toolHasBase(input, strInput);
                if ((input.length > 0) && firstChar != c'/' && resUrl.cPath.length == 0) {
                    resUrl.cPath = base.cPath;
                    resUrl.flags[INDEX_PATH] = base.flags[INDEX_PATH];
                }
                if ((input.length > 0) && firstChar != c'/' && resUrl.cPath.length != 0) {
                    let isFile: boolean = resUrl.cScheme == "file:";
                    shorteningPath(resUrl, base, isFile);
                    let basePathStr: string = basePathToString(base);
                    basePathStr == "" ? basePathStr = strInput : basePathStr += "/" + strInput;
                    resUrl.cPath = new Array<string>();
                    resUrl.analysizeInput(basePathStr);
                    resUrl.flags[INDEX_PATH] = true
                }
            } else if (base.flags[INDEX_INFOPATH]) {
                resUrl.flags[INDEX_INVALID] = true
                result.urlInner = resUrl;
                return result;
            }
            result.urlInner = resUrl;
            return result;
        }

        private static parseBaseInner(input: string, base: InnerURL, resUrl: InnerURL, result: URL, needReturn: Boolean): [Char | '' , Boolean] {
            const firstChar = input.length > 0 ? input.charAt(0) : '';
            const secondChar = input.length > 1 ? input.charAt(1) : '';
            if ((input.length > 0) && (firstChar == c'/') && (secondChar == c'/' || (secondChar == c'\\' && base.flags[INDEX_SPECIAL]))) {
                let newInput: string = base.cScheme + input;
                resUrl.flags[INDEX_INVALID] = false;
                resUrl.initOnlyInput(newInput);
                result.urlInner = resUrl;
                needReturn = true;
            }
            return [firstChar, needReturn];
        }
    }

    function removeKeyValuePairs(str: string, key: string): string {
        const regexStr = `\\b${key}=[^&]*&?`
        let result = str.replaceAll(regexStr, "");
        if (result.endsWith("&")) {
            result = result.slice(0, -1);
        }
        return result;
    }

    class ParamNode {
        element: [string, string];
        prev?: ParamNode;
        next?: ParamNode;

        constructor(element: [string, string]) {
            this.element = element;
        }
    }

    /**
    * The URLParams interface defines some practical methods to process URL query strings.
    *
    * @class URLParams
    */
    export class URLParams {
        parentUrl: URL | null = null;
        private head: ParamNode | undefined;
        private tail: ParamNode | undefined;
        private cacheMap: Map<string, int>;

        /**
        * A parameterized constructor used to create an URLParams instance.
        * As the input parameter of the constructor function, init supports four types.
        * The input parameter is a character string two-dimensional array.
        * The input parameter is the object list.
        * The input parameter is a character string.
        * The input parameter is the URLParams object.
        *
        * @param { string[][] | Record<string, string> | string | URLParams }
        * @throws { BusinessError } 10200002 - Parameter error. Parameter verification failed.
        */
        constructor(input?: [string, string][] | Record<string, string> | string | URLParams) {
            this.cacheMap = new Map<string, int>();
            this.parameterProcess(input);
        }

        /**
        * Appends a specified key/value pair as a new search parameter.
        *
        * @param { string } name - name name Key name of the search parameter to be inserted.
        * @param { string } value - value value Values of search parameters to be inserted.
        */
        append(params1: string, params2: string): void {
            let element: [string, string] = [params1, params2];
            let tempNode: ParamNode = new ParamNode(element);
            this.addToCache(params1);
            if (this.head == undefined) {
                this.head = tempNode;
                this.tail = tempNode;
                return;
            }
            this.tail!.next = tempNode;
            tempNode.prev = this.tail;
            this.tail = tempNode;
            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }

        /**
        * Sets the value associated with a given search parameter to the
        * given value. If there were several matching values, this method
        * deletes the others. If the search parameter doesn't exist, this
        * method creates it.
        *
        * @param { string } name - name name Key name of the parameter to be set.
        * @param { string } value - value value Indicates the parameter value to be set.
        */
        set(setName: string, setValues: string): void {
            let hasFound = false;
            let current = this.head;
            while(current != undefined) {
                if (current!.element[0] == setName) {
                    if (!hasFound) {
                        current!.element[1] = setValues;
                        hasFound = true;
                        current = current!.next;
                        continue;
                    } else if (current == this.tail) {
                        this.tail = current!.prev;
                        this.tail!.next = undefined;
                        current = undefined;
                        let val = this.cacheMap.get(setName);
                        this.cacheMap.set(setName, val! - 1);
                        continue;
                    } else {
                        current!.prev!.next = current!.next;
                        current!.next!.prev = current!.prev;
                        let val = this.cacheMap.get(setName);
                        this.cacheMap.set(setName, val! - 1);
                    }
                }
                current = current!.next;
            }

            if (!hasFound) {
                this.append(setName, setValues);
            }

            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }

        /**
        * Sort all key/value pairs contained in this object in place and return undefined.
        *
        */
        sort(): void {
            this.head = this.doMergeSort(this.head);
            if (this.head) {
		        let current = this.head;
		        while (current!.next) {
			        current = current!.next;
		        }
                this.tail = current;
	        } 
            else {
		        this.tail = undefined;
	        }
            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }

        /**
        * Returns a Boolean that indicates whether a parameter with the specified name exists.
        *
        * @param { string } name - name name Specifies the name of a key-value pair.
        * @returns { boolean } Returns a Boolean value that indicates whether a found
        */
        has(hasname: string): boolean {
            let current = this.head;
            while (current != undefined) {
                if (current!.element[0] == hasname) {
                    return true;
                }
                current = current!.next;
            }
            return false;
        }

        /**
        * Returns a query string suitable for use in a URL.
        *
        * @returns { string } Returns a search parameter serialized as a string, percent-encoded if necessary.
        */
        toString(): string {
            let resultArray = new Array<string>();
            let current = this.head;
            let key: string = '';
            let value: string = '';
            const regStr = '%20';
            while(current != undefined) {
                key = encodePercentEncoding(fixUSVstring(current!.element[0]), Helper.PARAMS_TOSTRING_SIGN_CHARS).replaceAll(regStr, '+');
                value = encodePercentEncoding(fixUSVstring(current!.element[1]), Helper.PARAMS_TOSTRING_SIGN_CHARS).replaceAll(regStr, '+');
                resultArray.push(`${current != this.head ? '&' : ''}${key}=${value}`);
                current = current!.next;
            }
            return resultArray.join('');
        }

        /**
        * Returns an iterator allowing to go through all keys contained in this object.
        *
        * @returns { IterableIterator<string> } Returns an ES6 Iterator over the names of each name-value pair.
        */
        keys(): IterableIterator<string> {
            let current = this.head;
            let res = new Array<string>();
            while(current != undefined) {
                res.push(current!.element[0]);
                current = current!.next;
            }
            return res.$_iterator();
        }

        /**
        * Returns an iterator allowing to go through all keys contained in this object.
        *
        * @returns { IterableIterator<string> } Returns an ES6 Iterator over the names of each name-value pair.
        */
        values(): IterableIterator<string> {
            let current = this.head;
            let res = new Array<string>();
            while(current != undefined) {
                res.push(current!.element[1]);
                current = current!.next;
            }
            return res.$_iterator();
        }

        /**
        * Returns all key-value pairs associated with a given search parameter as an array.
        *
        * @param { string } name - Specifies the name of a key value.
        * @returns { string[] } string[] Returns all key-value pairs with the specified name.
        */
        getAll(getAllname: string): string[] {
            let res = new Array<string>();
            let current = this.head;
            while(current != undefined) {
                if (current!.element[0] == getAllname) {
                    res.push(current!.element[1]);
                }
                current = current!.next;
            }
            return res;
        }

        /**
        * Returns the first value associated to the given search parameter.
        *
        * @param { string } name - Specifies the name of a key-value pair.
        * @returns { string | undefined } Returns the first value found by name. If no value is found, undefined is returned.
        * If no value is found, undefined is returned.
        */
        get(getname: string): string | undefined {
            let current = this.head;
            while(current != undefined) {
                if (current!.element[0] == getname) {
                    return current!.element[1];
                }
                current = current!.next;
            }
            return undefined;
        }

        /**
        * Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
        * The first item of Array is name, and the second item of Array is value.
        *
        * @returns { IterableIterator<[string,string]> } Returns an iterator over all values.
        */
        entries(): IterableIterator<[string, string]> {
            let result = new Array<[string, string]>();
            let current = this.head;
            while(current != undefined) {
                let arr: [string, string] = current!.element;
                result.push(arr);
                current = current!.next;
            }
            return result.$_iterator();
        }

        /**
        * Deletes the given search parameter and its associated value,from the list of all search parameters.
        *
        * @param { string } name - name name Name of the key-value pair to be deleted.
        */
        delete(deleteName: string): void {
            let val: int | undefined = this.cacheMap.get(deleteName);
            if (val == undefined || val == 0) {
                return;
            }
            let current = this.head;
            while (current != undefined) {
                if (current!.element[0] == deleteName) {
                    this.cacheMap.set(deleteName, val - 1);
                    if (current == this.head && current == this.tail) {
                        this.head = undefined;
                        this.tail = undefined;
                        current = undefined;
                        if ((val - 1) == 0) {
                            return;
                        } else {
                           continue;
                        }
                    } else if (current == this.head && current != this.tail) {
                        this.head = current!.next;
                        this.head!.prev = undefined;
                        current = this.head;
                        if ((val - 1) == 0) {
                            return;
                        } else {
                           continue;
                        }
                    } else if (current == this.tail) {
                        this.tail = current!.prev;
                        this.tail!.next = undefined;
                        current = undefined;
                        if ((val - 1) == 0) {
                            return;
                        } else {
                           continue;
                        }
                    } else {
                        current!.prev!.next = current!.next;
                        current!.next!.prev = current!.prev;
                        if ((val - 1) == 0) {
                            return;
                        }
                    }
                }
                current = current!.next;
            }
            if (this.parentUrl != null) {
                let searchStr: string = removeKeyValuePairs(this.parentUrl!.urlInner.getSearch(), deleteName);
                this.parentUrl!.urlInner.setSearch(searchStr);
                this.parentUrl!.cSearch = searchStr;
                this.parentUrl!.setHref();
            }
        }

        /**
        * Callback functions are used to traverse key-value pairs on the URLParams instance object.
        *
        * @param { function } callbackFn - callbackFn value Current traversal key value,
        * key Indicates the name of the key that is traversed.
        */
        forEach(objfun: UrlCbFn) {
            let current = this.head;
            if (current == undefined) {
                return;
            }
            while(current != undefined) {
                let node = current!.element;
                objfun(node[1], node[0], this);
                current = current!.next;
            }
        }

        /**
        * Returns an iterator allowing to go through all key/value
        * pairs contained in this object.
        *
        * @returns { IterableIterator<[string, string]> } Returns an ES6 iterator.
        * Each item of the iterator is a JavaScript Array.
        * The first item of Array is name, and the second item of Array is value.
        */
        $_iterator(): IterableIterator<[string, string]> {
            return this.entries();
        }

        /**
        * Helper function for sort
        *
        * @returns { ParamNode | undefined } Returns ParamNode or undefined.
        */
        private sortSplit(head: ParamNode | undefined) : ParamNode | undefined {
          let fast = head;
          let slow = head;

          while (fast!.next && fast!.next!.next) {
              fast = fast!.next!.next;
              slow = slow!.next;
          }

          let temp = slow!.next;
          slow!.next = undefined;
          if (temp != undefined) {
              temp!.prev = undefined;
          }

          return temp;
        }

        /**
        * Helper function for sort
        *
        * @returns { ParamNode | undefined } Returns ParamNode or undefined.
        */
        private sortMerge(first: ParamNode | undefined, second: ParamNode | undefined) {
            if (first == undefined) return second;

            if (second == undefined) return first;

            if (first!.element[0] <= second!.element[0]) {
                first!.next = this.sortMerge(first!.next, second);
                if (first!.next) first!.next!.prev = first;
                first!.prev = undefined;
                return first;
            } else {
                second!.next = this.sortMerge(first, second!.next);
                if (second!.next) second!.next!.prev = second;
                second!.prev = undefined;
                return second;
            }
        }

        /**
        * Helper function for sort
        *
        * @returns { ParamNode | undefined } Returns ParamNode or undefined.
        */
        private doMergeSort(head: ParamNode | undefined): ParamNode | undefined {
            if (head == undefined || head.next == undefined) {
                return head;
            }
            let second = this.sortSplit(head);

            head = this.doMergeSort(head);
            second = this.doMergeSort(second);

            return this.sortMerge(head, second);
        }

        /**
        * Update an URLParams instance by a string input.
        *
        * @param { string } input - input string.
        */
        updateParams(input: string): void {
            this.parameterProcess(input);
        }

        private addToCache(key: string) {
            let val: int | undefined = this.cacheMap.get(key);
            if (val == undefined) {
                this.cacheMap.set(key, 1);
            } else {
                this.cacheMap.set(key, (val!) + 1);
            }
        }

        private parameterProcess(input?: [string, string][] | Record<string, string> | string | URLParams): void {
            // Clear the cacheMap because updateParams can call this method.
            this.cacheMap.clear();
            if (input == undefined || input == null || input == '') {
                this.head = undefined;
                this.tail = undefined;
            } else if (typeof input == 'string') {
                this.parseParamFromString(input as string);
            } else if (input instanceof URLParams) {
                this.head = input.head;
                this.tail = input.tail;
                this.cacheMap = input.cacheMap;
            } else if (input instanceof Array) {
                this.parseParamFromArray(input as [string, string][]);
            } else if (input instanceof Record) {
                this.parseParamFromRecord(input as Record<string, string>)
            } else {
                throw createBusinessError(SyntaxErrorCodeId, 'Syntax Error. Invalid input type.');
            }
        }

        private parseParamFromString(input: string): void {
            if (input.charAt(0) == c'?') {
                input = input.slice(1);
            }
            let strVal = input.replaceAll('+', ' ');
            let searchParamsArr: Array<string> = stringParmas(strVal);
            let current = this.head;
            for (let i: int = 0; i <= (searchParamsArr.length - 2); i += 2) {
                let key: string = decodeStringParmas(searchParamsArr[i]);
                let ele: [string, string] = [key, decodeStringParmas(searchParamsArr[i + 1])];
                let tempNode: ParamNode = new ParamNode(ele);
                this.addToCache(key);
                if (i == 0) {
                    this.head = tempNode;
                    current = this.head;
                    continue;
                }
                current!.next = tempNode;
                tempNode.prev = current;
                current = tempNode;
            }
            this.tail = current;
        }

        private parseParamFromArray(input: [string, string][]): void {
            if (input.length == 0) {
                this.head = undefined;
                this.tail = undefined;
            } else if (input[0][0] == undefined || input[0][1] == undefined) {
                throw createBusinessError(SyntaxErrorCodeId, 'Parameter error. The type of input must be string[][].');
            }
            let current = this.head;
            for (let it of input) {
                let element: [string, string] = [it[0], it[1]];
                let temp = new ParamNode(element);
                this.addToCache(it[0]);
                if (this.head == undefined) {
                    this.head = temp;
                    current = this.head;
                } else {
                    temp.prev = current;
                    current!.next = temp;
                    current = temp;
                }
            }
            this.tail = current;
        }

        private parseParamFromRecord(input: Record<string, string>): void {
            let current = this.head;
            for (let k of input.keys()) {
                let element: [string, string] = [k, input.get(k)!];
                let temp = new ParamNode(element);
                this.addToCache(k);
                if (this.head == undefined) {
                    this.head = temp;
                    current = this.head;
                } else {
                    temp.prev = current;
                    current!.next = temp;
                    current = temp;
                }
            }
            this.tail = current;
        }
    }

    function utf8Decode(uint8Array: Uint8Array) {
        let result: string = '';
        let i = 0;
        const UNICODE_0x80 = 0x80;
        const UNICODE_0xC0 = 0xC0;
        const UNICODE_0x3F = 0x3F;
        const UNICODE_0xF0 = 0xF0;
        const UNICODE_0xE0 = 0xE0;
        const UNICODE_0x07 = 0x07;
        const UNICODE_0x0F = 0x0F;
        const UNICODE_0XFFFD = 0XFFFD;
        const UNICODE_0x1F = 0x1F;
        while (i < uint8Array.length) {
            const byte1 = uint8Array[i];
            if ((byte1 & UNICODE_0x80) === 0) {
                //  (0xxxxxxx)
                result += String.fromCharCode(byte1);
                i++;
            } else if ((byte1 & UNICODE_0xE0) === UNICODE_0xC0) {
                if (i + 1 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }
                //  (110xxxxx 10xxxxxx)
                const byte2 = uint8Array[i + 1];
                const codePoint = ((byte1 & UNICODE_0x1F) << 6) | (byte2 & UNICODE_0x3F)
                result += String.fromCharCode(codePoint)
                i += 2;
            } else if ((byte1 & UNICODE_0xF0) === UNICODE_0xE0) {
                if (i + 2 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }
                //  (1110xxxx 10xxxxxx 10xxxxxx)
                const byte2 = uint8Array[i + 1];
                const byte3 = uint8Array[i + 2];
                const codePoint = ((byte1 & UNICODE_0x0F) << 12) | ((byte2 & UNICODE_0x3F) << 6) | (byte3 & UNICODE_0x3F)
                result += String.fromCharCode(codePoint)
                i += 3;
            } else if ((byte1 & 0xF8) === UNICODE_0xF0) {
                // (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                if (i + 3 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }

                const byte2 = uint8Array[i + 1];
                const byte3 = uint8Array[i + 2];
                const byte4 = uint8Array[i + 3];
                const codePoint = ((byte1 & UNICODE_0x07) << 18) | ((byte2 & UNICODE_0x3F) << 12) | ((byte3 & UNICODE_0x3F) << 6) | (byte4 & UNICODE_0x3F)
                result += String.fromCodePoint(codePoint.toInt());
                i += 4;
            } else {
                result += String.fromCharCode(UNICODE_0XFFFD);
                i++;
            }
        }
        return result;
    }

    function unescapeBuffer(s: string): Uint8Array {
        const outBuffer = new Uint8Array(s.length * 4);
        let outIndex = 0;
        let srcIndex = 0;

        // UTF-8 encoding function
        const encodeUtf8 = (codePoint: number) => {
            if (codePoint <= 0x7F) {
                outBuffer[outIndex++] = codePoint;
            } else if (codePoint <= 0x7FF) {
                outBuffer[outIndex++] = 0xC0 | (codePoint >> 6);
                outBuffer[outIndex++] = 0x80 | (codePoint & 0x3F);
            } else if (codePoint <= 0xFFFF) {
                outBuffer[outIndex++] = 0xE0 | (codePoint >> 12);
                outBuffer[outIndex++] = 0x80 | ((codePoint >> 6) & 0x3F);
                outBuffer[outIndex++] = 0x80 | (codePoint & 0x3F);
            } else if (codePoint <= 0x10FFFF) {
                outBuffer[outIndex++] = 0xF0 | (codePoint >> 18);
                outBuffer[outIndex++] = 0x80 | ((codePoint >> 12) & 0x3F);
                outBuffer[outIndex++] = 0x80 | ((codePoint >> 6) & 0x3F);
                outBuffer[outIndex++] = 0x80 | (codePoint & 0x3F);
            } else {
                // Invalid code point
                outBuffer[outIndex++] = 0xEF;
                outBuffer[outIndex++] = 0xBF;
                outBuffer[outIndex++] = 0xBD; // U+FFFD
            }
        };

        // Process '%' encoding
        const handlePercentEncoding = () => {
            if (srcIndex + 2 < s.length) {
                const hex1 = Helper.unhexTable[s.charCodeAt(srcIndex + 1).toInt()];
                const hex2 = Helper.unhexTable[s.charCodeAt(srcIndex + 2).toInt()];

                if (hex1 !== -1 && hex2 !== -1) {
                    outBuffer[outIndex++] = hex1 * 16 + hex2;
                    // skip %XX
                    srcIndex += 3;
                    return true;
                }
            }
            return false;
        };

        while (srcIndex < s.length) {
            const code = s.charCodeAt(srcIndex);

            // Process '%' encoding
            if (code === 0x25) { // '%'
                if (handlePercentEncoding()) {
                    //Successfully processed '%' encoding, proceed to next char
                    continue;
                }
            }

            // Handling bit agents
            if (isHighSurrogate(code)) {
                if (srcIndex + 1 < s.length) {
                    const nextCode = s.charCodeAt(srcIndex + 1);
                    if (isLowSurrogate(nextCode)) {
                        const codePoint = combineSurrogates(code, nextCode);
                        encodeUtf8(codePoint);
                        //Skip high-bit agents and low-bit agents
                        srcIndex += 2;
                        continue;
                    }
                }
                // No matching low-bit agent
                encodeUtf8(0xFFFD);
            } else if (isLowSurrogate(code)) {
                // Isolated low-bit agents
                encodeUtf8(0xFFFD);
            } else {
                encodeUtf8(code);
            }
            srcIndex++;
        }
        return outBuffer.slice(0, outIndex);
    }

    function isHighSurrogate(code: number): boolean {
        return code >= 0xD800 && code <= 0xDBFF;
    }

    function isLowSurrogate(code: number): boolean {
        return code >= 0xDC00 && code <= 0xDFFF;
    }

    function combineSurrogates(high: number, low: number): number {
        return (high - 0xD800) * 0x400 + (low - 0xDC00) + 0x10000;
    }

    function decodeStringParmas(input: string): string {
        let result = '';
        try {
            result = decodeURIComponent(input);
        } catch (e) {
            result = utf8Decode(unescapeBuffer(input));
        }
        return result;
    }

    function stringParmas(stringParm: string): Array<string> {
        let seachParasVec = new Array<string>();
        let strStartPos = 0;
        let strLastPos = 0;
        let isHasSpace = false;
        let buf = "";
        let i = 0;
        for (; i < stringParm.length; i++) {
            let code = stringParm.charAt(i);
            switch (code) {
                case c'&':
                    {
                        if (strStartPos == i) {
                            strLastPos = i + 1;
                            strStartPos = i + 1;
                        }
                        if (strLastPos < i) {
                            buf += stringParm.substring(strLastPos, i);
                        }
                        seachParasVec.push(buf);
                        if (!isHasSpace) {
                            seachParasVec.push("");
                        }
                        isHasSpace = false;
                        buf = "";
                        strLastPos = i + 1;
                        strStartPos = i + 1;
                        break;

                    }
                case c'=':
                    {
                        if (isHasSpace) {
                            break;
                        }
                        if (strLastPos < i) {
                            buf += stringParm.substring(strLastPos, i);
                        }
                        seachParasVec.push(buf);
                        buf = "";
                        strLastPos = i + 1;
                        isHasSpace = true;
                        break;
                    }
                default: break;
            }
        }
        if (strStartPos == i) {
            return seachParasVec;
        }
        if (strLastPos < i) {
            buf += stringParm.substring(strLastPos, i);
        }
        seachParasVec.push(buf);
        if (!isHasSpace) {
            seachParasVec.push("");
        }

        return seachParasVec;
    }

    function fixUSVstring(str: string) {
        return str.toWellFormed();
    }

    function encodePercentEncoding(str: string, noEscapeTable: Int8Array) {
        const len = str.length;
        if (len == 0)
            return '';

        let outVal = '';
        let lastPos = 0;
        let i = 0;
        const UNICODE_0x80 = 0x80;
        const UNICODE_0x800 = 0x800;
        const UNICODE_0xD800 = 0xD800;
        const UNICODE_0xE000 = 0xE000;
        const UNICODE_0xC0 = 0xC0;
        const UNICODE_0x3F = 0x3F;
        const UNICODE_0xF0 = 0xF0;
        const UNICODE_0xE0 = 0xE0;
        const UNICODE_0x3FF = 0x3FF;
        const UNICODE_0x10000 = 0x10000;
        outer:
        for (; i < len; i++) {
            let c = str.charCodeAt(i).toInt();

            // ASCII
            while (c < UNICODE_0x80) {
                if (noEscapeTable[c] !== 1) {
                    if (lastPos < i)
                        outVal += str.slice(lastPos, i);
                    lastPos = i + 1;
                    outVal += Helper.hexTable[c];
                }

                if (++i == len)
                    break outer;

                c = str.charCodeAt(i).toInt();
            }

            if (lastPos < i)
                outVal += str.slice(lastPos, i);

            // Multi-byte characters ...
            if (c < UNICODE_0x800) {
                lastPos = i + 1;
                outVal += Helper.hexTable[UNICODE_0xC0 | (c >> 6)] +
                    Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
                continue;
            }
            if (c < UNICODE_0xD800 || c >= UNICODE_0xE000) {
                lastPos = i + 1;
                outVal += Helper.hexTable[UNICODE_0xE0 | (c >> 12)] +
                    Helper.hexTable[UNICODE_0x80 | ((c >> 6) & UNICODE_0x3F)] +
                    Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
                continue;
            }
            // Surrogate pair
            ++i;

            const c2 = str.charCodeAt(i).toInt() & UNICODE_0x3FF;

            lastPos = i + 1;
            c = UNICODE_0x10000 + (((c & UNICODE_0x3FF) << 10) | c2);
            outVal += Helper.hexTable[UNICODE_0xF0 | (c >> 18)] +
                Helper.hexTable[UNICODE_0x80 | ((c >> 12) & UNICODE_0x3F)] +
                Helper.hexTable[UNICODE_0x80 | ((c >> 6) & UNICODE_0x3F)] +
                Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
        }
        if (lastPos == 0) {
            return str;
        }
        if (lastPos < len) {
            return outVal + str.slice(lastPos);
        }
        return outVal;
    }
}
