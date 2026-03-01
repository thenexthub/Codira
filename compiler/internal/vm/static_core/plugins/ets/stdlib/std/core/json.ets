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

package std.core

const BigIntType: Class;

export interface JsonReplacer {
    jsonReplacer(): Record<String, Any>
}

// NOTE (chengzhihao): $Partial is not generated for exported interfaces.issue #26111
// now use this class to throw the error.
export class JsonReplacerTemp implements JsonReplacer {
    jsonReplacer(): Record<String, Any> {
        throw new Error("frontend bug");
    }
}

export @interface JSONRename {
    newName: string
}

export @interface JSONStringifyIgnore {
}

export @interface JSONParseIgnore {
}

function searchInstanceFieldInHierarchy(cls: Class, name: string): reflect.InstanceField | undefined {
    let currClass: Class | undefined = cls
    while(currClass != undefined) {
        const candidate = currClass.getInstanceField(name)
        if (candidate != undefined) {
            return candidate
        }
        currClass = currClass.getSuper()
    }
    return undefined
}

function getClassInstanceFieldsAll(cls: Class): Array<reflect.InstanceField> {
    const baseClasses = new Array<Class>()
    while (cls.getSuper() !== undefined) {
        baseClasses.push(cls)
        cls = cls.getSuper()!
    }

    const allFields = new Array<reflect.InstanceField>()
    const baseClassesCount = baseClasses.length
    for (let i = baseClassesCount - 1; i >= 0; i--) {
        const currentCls = baseClasses[i]
        const clsFields = currentCls.getInstanceFields()
        for (const field of clsFields) {
            allFields.push(field)
        }
    }

    return allFields
}

class JSONAPI {
    native static getJSONStringifyIgnore(field: reflect.InstanceField): boolean

    native static getJSONParseIgnore(field: reflect.InstanceField): boolean

    native static getJSONRename(field: reflect.InstanceField): string | undefined
}

// JsonSerializable provides a interface for external classes to override toJSON
export interface JsonSerializable {
    toJSON(): string
}

export class JSON {

    /**
     * Converts byte to JSON format
     *
     * @param d: byte - byte to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: byte): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts char to JSON format
     *
     * @param d: char - char to be converted to a JSON as a String
     *
     * @returns String - JSON representation of char
     */
    public static stringify(d: char): String {
        let sb = new StringBuilder("\"")
        sb.append(d)
        sb.append("\"")
        return sb.toString()
    }

    /**
     * Converts short to JSON format
     *
     * @param d: short - short to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of short
     */
    public static stringify(d: short): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts int to JSON format
     *
     * @param d: int - int to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of int
     */
    public static stringify(d: int): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts long to JSON format
     *
     * @param d: long - long to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of long
     */
    public static stringify(d: long): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts float to JSON format
     *
     * @param d: float - float to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of float
     */
    public static stringify(d: float): String {
        if (Float.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return "null"
        }
    }

    /**
     * Converts double to JSON format
     *
     * @param d: double - double to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of double
     */
    public static stringify(d: double): String {
        if (Double.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return "null"
        }
    }

    /**
     * Converts bigint to JSON format
     *
     * @param d: bigint - bigint to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of bigint
     */
    public static stringify(d: bigint): String {
        return d.toString()
    }

    /**
     * Converts boolean to JSON format
     *
     * @param d: boolean - boolean to be converted to a JSON as a Boolean literal
     *
     * @returns String - JSON representation of boolean
     */
    public static stringify(d: boolean): String {
        if (d) {
            return "true"
        }
        return "false"
    }

    private static readonly ESCAPED_CHARS: FixedArray<char> = [c'\"', c'\\', c'\b', c'\f', c'\n', c'\r', c'\t']
    private static readonly NON_ESCAPED_CHARS: FixedArray<char> = [c'"', c'\\', c'b', c'f', c'n', c'r', c't']
    private static readonly JSON_SPACE_INDENT_LIMIT = 10

    /**
     * Converts String to JSON format
     *
     * @param d: String - byte to be converted to a JSON as a String
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: String): String {
        let sb = new StringBuilder([c'\"'] as FixedArray<char>)
        let len = d.getLength()

        let prevChar: char = c'\0'
        for (let i = 0; i < len; ++i) {
            let currChar = d.charAt(i)

            if (Char.isHighSurrogate(currChar)) {
                if (i != 0 && Char.isHighSurrogate(prevChar)) {
                    JSON.appendCharHex(sb, prevChar)
                }
            } else if (Char.isLowSurrogate(currChar)) {
                if (i != 0 && Char.isHighSurrogate(prevChar)) {
                    sb.append(prevChar)
                    sb.append(currChar)
                } else {
                    JSON.appendCharHex(sb, currChar)
                }
            } else {
                if (i != 0 && Char.isHighSurrogate(prevChar)) {
                    JSON.appendCharHex(sb, prevChar)
                }

                let escapedCharIndex = lastIndexOf(JSON.ESCAPED_CHARS, currChar, JSON.ESCAPED_CHARS.length)
                if (escapedCharIndex != -1) {
                    sb.append(c'\\')
                    sb.append(JSON.NON_ESCAPED_CHARS[escapedCharIndex])
                } else if ((currChar.compareTo(c'\x1f') <= 0) && (currChar.compareTo(c'\x00') >= 0)) {
                    if (currChar.compareTo(c'\x0f') <= 0) {
                        sb.append('\\u000')
                    } else {
                        sb.append('\\u00')
                    }
                    sb.append(currChar.toInt().toString(16))
                } else {
                    sb.append(currChar)
                }
            }

            prevChar = currChar
        }

        if (len > 0) {
            if (Char.isHighSurrogate(prevChar)) {
                JSON.appendCharHex(sb, prevChar)
            }
        }

        sb.append(c'\"')
        return sb.toString()
    }

    private static appendCharHex(buffer: StringBuilder, chr: char): StringBuilder {
        buffer.append("\\u").append(chr.toInt().toString(16))
        return buffer
    }

    //--------------------------
    //          arrays
    //--------------------------

    /**
     * Converts bytes array to JSON format
     *
     * @param d: byte[] - bytes array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of bytes array
     */
    public static stringify(d: FixedArray<byte>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts chars array to JSON format
     *
     * @param d: FixedArray<char> - chars array  to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of chars array
     */
    public static stringify(d: FixedArray<char>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append("\"")
            s.append(d[i])
            s.append("\",")
        }
        if (d.length > 0) {
            s.append("\"")
            s.append(d[last_elem])
            s.append("\"")
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts shorts array to JSON format
     *
     * @param d: FixedArray<short> - shorts array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of shorts array
     */
    public static stringify(d: FixedArray<short>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts ints array to JSON format
     *
     * @param d: FixedArray<int> - ints array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of ints array
     */
    public static stringify(d: FixedArray<int>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts longs array to JSON format
     *
     * @param d: FixedArray<long> - longs array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of longs array
     */
    public static stringify(d: FixedArray<long>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts array of bytes to JSON format
     *
     * @param d: FixedArray<byte> - array of byte to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of array of bytes
     */
    public static stringify(d: FixedArray<float>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]))
            s.append(',')
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]))
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts doubles array to JSON format
     *
     * @param d: FixedArray<double> - doubles array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of doubles array
     */
    public static stringify(d: FixedArray<double>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]))
            s.append(',')
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]))
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts booleans array to JSON format
     *
     * @param d: FixedArray<boolean> - booleans array to be converted to a JSON as an Array of Boolean literals
     *
     * @returns String - JSON representation of booleans array
     */
    public static stringify(d: FixedArray<boolean>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            if (d[i]) {
                s.append("true,")
            } else {
                s.append("false,")
            }
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: Array<Number>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]));
            s.append(",");
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append(']');
        return s.toString()
    }

    public static stringify(d: ArrayLike<Number>): String {
        let s = new StringBuilder("{")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append("\"");
            s.append(JSON.stringify(i));
            s.append("\":");
            s.append(JSON.stringify(d[i]));
            s.append(",");
        }
        if (d.length > 0) {
            s.append("\"");
            s.append(JSON.stringify(last_elem));
            s.append("\":");
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append('}');
        return s.toString()
    }

    public static stringify(obj: JsonReplacer): String {
        const record = obj.jsonReplacer();
        return JSON.stringify(record);
    }

    private static native stringifyJSValue(d: JSValue): String;

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(obj: Any): String {
        if (obj === undefined) {
            return "undefined"
        }
        if (obj instanceof JSValue) {
            return JSON.stringifyJSValue(obj as JSValue)
        }
        const json = JSON.stringifyFast(obj)
        if (json !== undefined) {
            return json
        }
        return new JSONWriter().write(obj)
    }

    private static native stringifyFast(obj: Any): string

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @param replacer A function that transforms the results.
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(
                obj: Any,
                replacer: ((key: string, value: Any) => Any) | undefined | null,
                space?: string | int): string {
        const replacerOpt: ((key: string, value: Any) => Any) | undefined = (replacer === null) ? undefined : replacer

        if (space instanceof String || space === undefined) {
            return new JSONWriter(replacerOpt, space).write(obj)
        } else {
            return new JSONWriter(replacerOpt, JSON.spaceFromNumber(space as int)).write(obj)
        }
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @param replacer An array with elements indicating names of the properties in the object
     * that should be included in the resulting JSON string
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(obj: Any, replacer: FixedArray<double | string>, space?: int | string): string {
        const filter : FixedArray<string> = new FixedArray<string>(replacer.length)
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    public static stringify(obj: Any, replacer: Array<double | string> | Array<string> | Array<double>, space?: int | string): string {
        const filter : FixedArray<string> = new FixedArray<string>(replacer.length)
        let index: int = 0
        for (let element of replacer) {
            filter[index++] = `${element}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    private static spaceFromNumber(space: int): String {
        let spacesCount = 0
        if (space > 0)  {
            spacesCount = (space > JSON.JSON_SPACE_INDENT_LIMIT) ? JSON.JSON_SPACE_INDENT_LIMIT : space
        }

        return " ".repeat(spacesCount)
    }

    /**
      * Converts a JsonElementSerializable to a JSON string.
     * @param elem - The JsonElementDeserializable to stringify
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElementSerializable): string {
        return JSON.stringifyJsonElement(elem.toJSON())
    }

    /**
      * Converts a JsonElementSerializable to a JSON string with optional formatting.
     * @param elem - The JsonElementDeserializable to stringify
     * @param replacer - Array of keys to include (currently not implemented)
     * @param space - String or number of spaces for indentation
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElementSerializable, replacer?: (double | string)[], space?: int | string): string {
        return JSON.stringifyJsonElement(elem.toJSON(), replacer, space)
    }

    /**
      * Converts a JsonElement to a JSON string.
     * @param elem - The JsonElement to stringify
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElement): string {
        return stringifyElementRecursively(elem, undefined, undefined, 0)
    }

    /**
      * Converts a JsonElement to a JSON string with optional formatting.
     * @param elem - The JsonElement to stringify
     * @param replacer - Array of keys to include (currently not implemented)
     * @param space - String or number of spaces for indentation
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElement, replacer?: (double | string)[], space?: int | string): string {
        return stringifyElementRecursively(elem, replacer, space, 0)
    }

    // TODO(kirill-mitkin): Map<Long, Long> blocked by internal issue
    private static checkType(type: Class, used: Map<Class, TypeColor>): void {
        const tc = used.get(type)
        if (tc !== undefined) {
            if (tc.color == TypeColor.VISITED) {
                return
            }
            throw new Error(`Incorrect type: ${type.getName()}`)
        }
        if (type === BigIntType ||
                type === TypeColor.STRING_CLASS ||
                Types.isPrimitiveType(type) ||
                type === TypeColor.NULL_CLASS) {
            return
        }
        if (type.isFixedArray()) {
            JSON.checkType(type.getFixedArrayComponentType()!, used)
        } else if (type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (type.isSubtypeOf(TypeColor.OBJECT_CLASS)) {
            const newTc = new TypeColor(type, TypeColor.AT_STACK)
            used.set(type, newTc)
            if (!type.isEnum()) {
                if (!JSON.classHasDefaultCtor(type)) {
                    throw new Error(`Incorrect type: '${type.getName()}' doesn't provide default public constructor`)
                }
                for (const field of getClassInstanceFieldsAll(type)) {
                    if (JSONAPI.getJSONParseIgnore(field)) {
                        continue
                    }
                    try {
                        JSON.checkType(field.getType(), used)
                    } catch (e) {
                        throw new Error(`Incorrect field: '${type.getName()}.${field.getName()}'. ${e.message}`)
                    }
                }
            }
            newTc.color = TypeColor.VISITED
        } else {
            throw new Error(`Incorrect type: ${type.getName()}`)
        }
    }

    private static classHasDefaultCtor(cls: Class): boolean {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return true
            }
        }
        return false
    }

    public static parse<T>(text: String, type: Type): T | null | undefined {
        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, undefined, cls)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T>(text: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Type, options?: jsonx.ParseOptions): T | null | undefined {
        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, reviver, cls, options)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T>(json: string, type: Class): T | null | undefined {
        return JSON.parse<T>(json, undefined, type)
    }

    public static parse<T>(json: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Class, options?: jsonx.ParseOptions): T | null | undefined {
        JSON.checkType(type, new Map<Class, TypeColor>())

        let bigIntOpt: int = options ? options.bigIntMode : 0
        if (bigIntOpt > 2 || bigIntOpt < 0) {
            throw new Error("bigIntMode value should be from 0 to 2, current value is" + bigIntOpt)
        }

        let jsonValue = JSONParser.parse(json)
        const parsingResult = new JSONValueParser(reviver, bigIntOpt).parse(jsonValue, type) as T

        if (reviver !== undefined) {
            return reviver("", parsingResult) as T
        } else {
            return parsingResult
        }
    }

    /**
      * Parses a JSON string and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      */
    static parseJsonElement(text: string, options?: jsonx.ParseOptions): jsonx.JsonElement {
        return new JsonElementParserOptimized(text, options).parse()
    }

    /**
      * Parses a JSON string with a reviver function and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @param {(key: string, value: JsonElement) => JsonElement} reviver - Function to transform values
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      */
    static parseJsonElement(text: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement, options?: jsonx.ParseOptions): jsonx.JsonElement {
        return new JsonElementParserOptimized(text, reviver, options).parse()
    }
}

interface ArrayHandle {
    readonly length: int
    readonly array: object
    get(i: int): Any
}

class BoolArrayHandle implements ArrayHandle {
    private arr: FixedArray<boolean>

    constructor(arr: FixedArray<boolean>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class ByteArrayHandle implements ArrayHandle {
    private arr: FixedArray<byte>

    constructor(arr: FixedArray<byte>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class CharArrayHandle implements ArrayHandle {
    private arr: FixedArray<char>

    constructor(arr: FixedArray<char>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class ShortArrayHandle implements ArrayHandle {
    private arr: FixedArray<short>

    constructor(arr: FixedArray<short>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class IntArrayHandle implements ArrayHandle {
    private arr: FixedArray<int>

    constructor(arr: FixedArray<int>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class LongArrayHandle implements ArrayHandle {
    private arr: FixedArray<long>

    constructor(arr: FixedArray<long>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class FloatArrayHandle implements ArrayHandle {
    private arr: FixedArray<float>

    constructor(arr: FixedArray<float>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class DoubleArrayHandle implements ArrayHandle {
    private arr: FixedArray<double>

    constructor(arr: FixedArray<double>) {
        this.arr = arr
    }

    get length(): int {
        return this.arr.length
    }

    get array(): object {
        return this.arr
    }

    get(i: int): Any {
        return this.arr[i]
    }
}

class JSONWriter {
    private static readonly STD_CORE_INTEROP_JSVALUE_CLASS = Class.from<JSValue>()
    private static readonly GETTER_METHOD_PREFIX = "%%get-"
    private static readonly maxSpaceLength = 10

    private readonly replacer: ((key: String, value: Any) => Any) | undefined
    private readonly fieldsFilter: FixedArray<string>
    private readonly space: String | undefined

    private readonly useReplacer: boolean
    private readonly useFieldsFilter: boolean
    private readonly useIndent: boolean

    private indentLevel: int = 0

    private readonly path = new Set<Object>()
    private buffer = new StringBuilder()

    constructor(replacer?: (key: String, value: Any) => Any, space?: String) {
        this.replacer = replacer
        this.fieldsFilter = []
        if (space !== undefined && space.length > JSONWriter.maxSpaceLength) {
            this.space = space.substring(0, JSONWriter.maxSpaceLength)
        } else {
            this.space = space
        }

        this.useReplacer = this.replacer != undefined
        this.useFieldsFilter = false
        this.useIndent = !(this.space == undefined || this.space == "")
    }

    constructor(filter: FixedArray<string>, space?: String) {
        this.replacer = undefined
        this.fieldsFilter = filter
        if (space !== undefined && space.length > JSONWriter.maxSpaceLength) {
            this.space = space.substring(0, JSONWriter.maxSpaceLength)
        } else {
            this.space = space
        }

        this.useReplacer = false
        this.useFieldsFilter = true
        this.useIndent = !(this.space == undefined || this.space == "")
    }

    write(obj: Any): String {
        if (this.useReplacer) {
            this.writeObject(this.replacer!("", obj))
        } else {
            this.writeObject(obj)
        }

        return this.buffer.toString()
    }

    private writeObject(obj: Any): void {
        if (obj === null) {
            this.buffer.append("null")
        } else if (obj === undefined) {
            this.buffer.append("undefined")
        } else if (obj instanceof String) {
            this.buffer.append(JSON.stringify(obj as String))
        } else if(obj instanceof Tuple) {
            this.writeTupleValue(obj);
        } else if (obj instanceof Record) {
            this.writeESCompatRecord(obj)
        } else if (obj instanceof JsonReplacer) {
            let temp: Record<String, Any> = obj.jsonReplacer()
            this.writeESCompatRecord(temp)
        } else if (obj instanceof ESValue) {
            this.writeInteropESValue(obj)
        } else if (obj instanceof JsonSerializable) {
            this.buffer.append(obj.toJSON())
        } else if (obj instanceof jsonx.JsonElementSerializable) {
            this.buffer.append(JSON.stringifyJsonElement(obj))
        } else if (obj instanceof ArrayBuffer) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Function) {
            this.buffer.append("undefined")
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else if (obj instanceof Date) {
            this.writeObject(obj.toJSON())
        } else if (obj instanceof Array) {
            this.writeEscompatArray(obj)
        } else if(obj instanceof Int8Array || obj instanceof Int16Array || obj instanceof Int32Array || obj instanceof Uint8Array || obj instanceof BigInt64Array
                || obj instanceof Uint16Array || obj instanceof Uint32Array || obj instanceof Float32Array || obj instanceof Float64Array
                || obj instanceof BigUint64Array || obj instanceof Uint8ClampedArray) {
            this.writeTypedArray(obj)
        } else if (obj instanceof Set || obj instanceof Promise || obj instanceof Map) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Object) {
            const objCls = Class.of(obj)
            if (objCls === JSONWriter.STD_CORE_INTEROP_JSVALUE_CLASS) {
                this.buffer.append(JSON.stringify(obj as JSValue))
            } else if (objCls.isFixedArray()) {
                if (obj instanceof FixedArray<boolean>) {
                    this.writeArrayHandleValue(new BoolArrayHandle(obj))
                } else if (obj instanceof FixedArray<byte>) {
                    this.writeArrayHandleValue(new ByteArrayHandle(obj))
                } else if (obj instanceof FixedArray<char>) {
                    this.writeArrayHandleValue(new CharArrayHandle(obj))
                } else if (obj instanceof FixedArray<short>) {
                    this.writeArrayHandleValue(new ShortArrayHandle(obj))
                } else if (obj instanceof FixedArray<int>) {
                    this.writeArrayHandleValue(new IntArrayHandle(obj))
                } else if (obj instanceof FixedArray<long>) {
                    this.writeArrayHandleValue(new LongArrayHandle(obj))
                } else if (obj instanceof FixedArray<float>) {
                    this.writeArrayHandleValue(new FloatArrayHandle(obj))
                } else if (obj instanceof FixedArray<double>) {
                    this.writeArrayHandleValue(new DoubleArrayHandle(obj))
                } else {
                    this.writeArrayValue(Types.asFixedArray(obj))
                }
            } else if (objCls.isEnum()) {
                this.writeEnumValue(obj, objCls)
            } else {
                this.writeClassValue(obj, objCls)
            }
        } else {
            throw new AssertionError("Unsupported object type: " + typeof obj)
        }
    }

    private writeEnumValue(obj: Object, cls: Class) {
        let valMethod = cls.getInstanceMethod('valueOf')
        if (valMethod!.getReturnType() == Class.from<String>()) {
            this.buffer.append('\"' + obj + '\"')
        } else {
            this.buffer.append(obj)
        }
    }

    private writeTupleValue(obj: Object) {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const objClass = Class.of(obj)

        const writableFields = this.getWritableFields(objClass, obj)

        let fieldDumped = false
        if (writableFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeTupleElements(writableFields)
            this.path.delete(obj)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("]")

        if (fieldDumped) {
            currentBuffer.append(this.buffer.toString())
        } else {
            currentBuffer.append("[]")
        }

        this.buffer = currentBuffer
    }

    private writeTupleElements(tupleFields: Array<[string, Any]>): boolean {
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < tupleFields.length; fieldIdx++) {
            const objToDump = tupleFields[fieldIdx][FIELD_VALUE]
            if (fieldDumped) {
                this.buffer.append(",")
                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            } else {
                fieldDumped = true
            }

            this.writePadding()

            if (this.useIndent) {
                this.buffer.append(" ")
            }

            if (objToDump != null && !(objToDump instanceof Function)) {
                this.writeObject(objToDump)
            } else {
                this.buffer.append("null")
            }
        }

        return fieldDumped
    }

    private writeESCompatRecord<K extends Numeric | string, V>(record: Record<K, V>): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        this.path.add(record)

        let entryDumped = false
        if (this.useFieldsFilter) {
            const writableEntries = this.getRecordWritableEntries(record)

            const ENTRY_KEY = 0
            const ENTRY_VAL = 1
            for (const entry of writableEntries) {
                const entryKey = entry[ENTRY_KEY]
                const entryVal = entry[ENTRY_VAL]
                entryDumped = this.writeRecordEntry(entryKey, entryVal, entryDumped)
            }
        } else {
            record.forEach((entryVal: V, entryKey: K) => {
                entryDumped = this.writeRecordEntry(entryKey, entryVal, entryDumped)
            })
        }

        this.path.delete(record)

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("}")

        currentBuffer.append(entryDumped ? this.buffer.toString() : JSONObject.EMPTY)

        this.buffer = currentBuffer
    }

    private getRecordWritableEntries<K extends Numeric | string, V>(record: Record<K, V>): Array<[K, V]> {
        // 'entry name' -> output position index
        const entryNamePos = new Map<string, int>()
        for (let idx = 0; idx < this.fieldsFilter.length; idx++) {
            const entryName = this.fieldsFilter[idx]
            entryNamePos.set(entryName, idx)
        }

        // 'entry key' -> output position index
        const entryKeyPos = new Array<[K, int]>
        for (const entryKey of record.keys()) {
            const entryName = (entryKey as object).toString()
            const entryPos = entryNamePos.get(entryName)
            if (entryPos === undefined) {
                continue
            }

            const entryKeyPosPair: [K, int] = [entryKey, entryPos]
            entryKeyPos.push(entryKeyPosPair)
        }

        // sorting 'filtered in' entries by position
        const ENTRY_POS = 1
        entryKeyPos.sort((pos1: [K, int], pos2: [K, int]) => {
            const entry1Pos = pos1[ENTRY_POS]
            const entry2Pos = pos2[ENTRY_POS]
            return (entry1Pos < entry2Pos) ? -1 : ((entry1Pos > entry2Pos) ? 1 : 0)
        })

        const writableEntries = new Array<[K, V]>()

        const ENTRY_KEY = 0
        for (const entry of entryKeyPos) {
            const entryKey = entry[ENTRY_KEY]
            const entryVal = record.get(entryKey) as V

            const writableEntry: [K, V] = [entryKey, entryVal]
            writableEntries.push(writableEntry)
        }

        return writableEntries
    }

    private writeRecordEntry<K extends Numeric | string, V>(entryKey: K, entryVal: V, entryDumped: boolean): boolean {
        const entryName = (entryKey as object).toString()
        const replacedVal = this.useReplacer ? this.replacer!(entryName, entryVal) : entryVal
        return this.writeField(entryName, replacedVal, entryDumped)
    }

    private writeInteropESValue(obj: ESValue): void {
        try {
            const json = JSON.stringify(obj.unwrap())
            this.buffer.append(json.toString())
        } catch (e) {
            if (e instanceof TypeError) {
                throw new TypeError("cyclic object value")
            } else {
                throw e
            }
        }
    }

    private collectInterfaceFields(obj: Object, objCls: Class, writableFields: Array<[string, Any]>): Array<[string, Any]> {
        const interfaceFields = new Array<[string, Any]>()
        let currentClass: Class | undefined = objCls
        while (currentClass != undefined) {
            const interfaces = currentClass.getInterfaces()
            for (let i = 0; i < interfaces.length; i++) {
                const iface = interfaces[i]
                const gettersValues = JSONWriter.getGettersKeyValuePairs(iface, obj, this.useFieldsFilter, this.fieldsFilter)
                for (let j = 0; j < gettersValues.length; j++) {
                    const pair = gettersValues[j]
                    const name = pair[0]
                    let isRedundant = false
                    for (let k = 0; k < writableFields.length; k++) {
                        if (writableFields[k][0] == name) {
                            isRedundant = true
                            break
                        }
                    }
                    if (!isRedundant) {
                        interfaceFields.push(pair)
                    }
                }
            }
            currentClass = currentClass.getSuper()
        }
        return interfaceFields
    }

    private writeClassValue(obj: Object, objCls: Class): void {
        const currentBuffer = this.buffer
        // we need this local buffer to discard empty object prolog: '{','\n',<space> and replace it with '{}'
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        let fieldDumped = false
        const writableFields = this.getWritableFields(objCls, obj)
        if (writableFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeClassFields(writableFields, fieldDumped)
            this.path.delete(obj)
        }

        if (reflect.isLiteralInitializedInterface(obj)) {
            const gettersValues = JSONWriter.getGettersKeyValuePairs(objCls, obj, this.useFieldsFilter, this.fieldsFilter)
            if (gettersValues.length > 0) {
                this.path.add(obj)
                fieldDumped |= this.writeClassFields(gettersValues, fieldDumped)
                this.path.delete(obj)
            }
            for (let i = 0; i < gettersValues.length; i++) {
                    writableFields.push(gettersValues[i])
            }
        }

        const interfaceFields = this.collectInterfaceFields(obj, objCls, writableFields)
        if (interfaceFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeClassFields(interfaceFields, fieldDumped)
            this.path.delete(obj)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("}")

        currentBuffer.append(fieldDumped ? this.buffer.toString() : JSONObject.EMPTY)

        this.buffer = currentBuffer
    }

    private writeField(fieldName: string, fieldVal: Any, fieldDumped: boolean): boolean {
        if (fieldVal === undefined) {
            return fieldDumped
        }

        if (fieldVal instanceof Function) {
            return fieldDumped
        }

        if (fieldDumped) {
            this.buffer.append(",")
            if (this.useIndent) {
                this.buffer.append("\n")
            }
        } else {
            fieldDumped = true
        }

        this.checkReferencesCycle(fieldVal)
        this.writePadding()

        this.buffer.append('"')
        this.buffer.append(fieldName)
        this.buffer.append('":')

        if (this.useIndent) {
            this.buffer.append(" ")
        }

        this.writeObject(fieldVal)

        return fieldDumped
    }

    private writeClassFields(writableFields: Array<[string, Any]>, fieldDumped: boolean): boolean {
        const FIELD_NAME = 0
        const FIELD_VALUE = 1

        for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
            const fieldTypeValuePair = writableFields[fieldIdx]

            const objFieldName = fieldTypeValuePair[FIELD_NAME]
            const objFieldValue = fieldTypeValuePair[FIELD_VALUE]

            const objToDump = this.useReplacer ? this.replacer!(objFieldName, objFieldValue) : objFieldValue
            fieldDumped = this.writeField(objFieldName, objToDump, fieldDumped)
        }

        return fieldDumped
    }

    private checkSameRename(fieldName: string, fields: Array<[string, Any]>): boolean {
        return fields.some((writableField) => writableField[0] == fieldName);
    }

    private findKeyIndex(fieldName: string, fields: Array<[string, Any]>): int {
        return fields.findIndex((writableField) => writableField[0] == fieldName);
    }

    private getWritableFields(cls: Class, obj: object): Array<[string, Any]> {
        const writableFields = new Array<[string, Any]>()
        let hasJsonIgnore: boolean = false
        let fieldRename : string | undefined = undefined

        if (this.useFieldsFilter) {
            for (let fieldIdx = 0; fieldIdx < this.fieldsFilter.length; fieldIdx++) {
                const fieldName = this.fieldsFilter[fieldIdx]
                let writableFieldName = fieldName
                const field = searchInstanceFieldInHierarchy(cls, fieldName)
                if (field === undefined) {
                    continue
                }

                hasJsonIgnore = JSONAPI.getJSONStringifyIgnore(field);
                if (hasJsonIgnore) {
                    continue
                }
                fieldRename = JSONAPI.getJSONRename(field);
                if (fieldRename != undefined) {
                    if (searchInstanceFieldInHierarchy(cls, fieldRename) !== undefined) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + cls.getName())
                    } else {
                        writableFieldName = fieldRename
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error("Cannot double rename " + writableFieldName + " in keys of " + cls.getName())
                    }
                }

                const fieldNameValuePair: [string, Any] = [writableFieldName, field.getValue(obj)]
                writableFields.push(fieldNameValuePair)
            }
        } else {
            for (const field of getClassInstanceFieldsAll(cls)) {
                let writableFieldName = field.getName()
                hasJsonIgnore = JSONAPI.getJSONStringifyIgnore(field);
                if (hasJsonIgnore) {
                    continue
                }
                const index: int = this.findKeyIndex(field.getName(), writableFields)
                fieldRename = JSONAPI.getJSONRename(field);
                if (fieldRename !== undefined) {
                    if (searchInstanceFieldInHierarchy(field.getOwner(), fieldRename) !== undefined) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + cls.getName())
                    } else {
                        writableFieldName = fieldRename
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error("Cannot double rename " + writableFieldName + " in keys of " + cls.getName())
                    }
                }

                const fieldNameValuePair: [string, Any] = [writableFieldName, field.getValue(obj)]
                if (fieldRename != undefined || index == -1) {
                    writableFields.push(fieldNameValuePair)
                } else {
                    writableFields[index] = fieldNameValuePair
                }
            }
        }

        return writableFields
    }

    /**
    * @brief Returns values of public getters of a class.
    * Searches only within direct methods of the class. Does not apply fields filter.
    */
    private static getGettersKeyValuePairs(cls: Class, obj: Object, useFieldsFilter: boolean, fieldsFilter: FixedArray<string>) {
        const namesAndValues = new Array<[string, Any]>()
        const prefixLength = JSONWriter.GETTER_METHOD_PREFIX.getLength()
        const clsMethods = cls.getInstanceMethods()
        for (const method of clsMethods) {
            let name = method.getName()
            if (name.startsWith(JSONWriter.GETTER_METHOD_PREFIX, 0)) {
                name = name.substring(prefixLength)
                if (useFieldsFilter && !JSONWriter.searchField(fieldsFilter, name)) {
                    continue
                }
                const value = method.invoke(obj)
                namesAndValues.push([name, value])
            }
        }
        return namesAndValues
    }

    private static searchField(fieldsFilter: FixedArray<string>, name: string): boolean {
        for (let i = 0; i < fieldsFilter.length; i++) {
            if (fieldsFilter[i] == name) {
                return true
            }
        }
        return false
    }

    private writeArrayValue<E>(array: FixedArray<E>): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const arrayLength = array.length
        if (arrayLength > 0) {
            this.path.add(array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(array[idx], idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeArrayElementValue(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
    }

    private writeArrayHandleValue(handle: ArrayHandle): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const arrayLength = handle.length
        if (arrayLength > 0) {
            this.path.add(handle.array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(handle.get(idx), idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeArrayElementValue(handle.get(lastElementIndex), lastElementIndex)

            this.path.delete(handle.array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
    }

    private writeArrayElementValue(elemValue: Any, elemIndex: int): void {
        this.writeObjectsArrayElement(elemValue, elemIndex)
    }

    private writeValueTypeWrapper(obj: Any): boolean {
        if (obj instanceof Boolean) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Byte) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Char) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Short) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Int) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Long) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Float) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Double) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof BigInt) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else {
            return false
        }
    }

    private writeTypedArray(array: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array |
        Uint32Array | Float32Array | Float64Array | BigInt64Array | BigUint64Array | Uint8ClampedArray): void {
        this.buffer.append("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        if (array.length > 0) {
            this.path.add(array)

            const lastElementIndex: int = array.length.toInt() - 1

            for (let idx: int = 0; idx < lastElementIndex; idx++) {
                this.buffer.append("\"")
                this.buffer.append(idx)
                this.buffer.append("\"")
                this.buffer.append(":")

                if (array instanceof BigInt64Array | BigUint64Array) {
                    this.writeObjectsArrayElement((array as ArrayLike<bigint>)[idx], idx)
                } else {
                    this.writeObjectsArrayElement((array as ArrayLike<Double>)[idx], idx)
                }
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.buffer.append("\"")
            this.buffer.append(lastElementIndex)
            this.buffer.append("\"")
            this.buffer.append(":")

            if (array instanceof BigInt64Array | BigUint64Array) {
                this.writeObjectsArrayElement((array as ArrayLike<bigint>)[lastElementIndex], lastElementIndex)
            } else {
                this.writeObjectsArrayElement((array as ArrayLike<Double>)[lastElementIndex], lastElementIndex)
            }

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("}")
    }

    private writeEscompatArray<T>(array: Array<T>): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        if (array.length > 0) {
            this.path.add(array)

            const lastElementIndex: int = array.length - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeObjectsArrayElement(array[idx], idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeObjectsArrayElement(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
    }

    private writeObjectsArrayElement(arrayElement: Any, elementIndex: int): void {
        this.writePadding()

        const replacedElem = this.useReplacer ? this.replacer!("" + elementIndex, arrayElement) : arrayElement

        if (replacedElem == null || replacedElem instanceof Function) {
            this.buffer.append("null")
        } else {
            this.checkReferencesCycle(replacedElem)
            this.writeObject(replacedElem)
        }
    }

    private checkReferencesCycle(obj: Any): void {
        if (obj == null) {
            return
        }

        const cycleDetected = this.path.has(obj as Object)
        if (cycleDetected) {
            throw new TypeError("cyclic object value")
        }
    }

    private writePadding() {
        if (this.indentLevel > 0) {
            const indent = this.space!.repeat(this.indentLevel)
            this.buffer.append(indent)
        }
    }
}

/**
* Internal method to stringify a JsonElement recursively.
* @param {JsonElement} elem - The JsonElement to stringify
* @param {(double | string)[] | undefined} replacer - Array of keys to include (currently not implemented)
* @param {int | string | undefined} space - String or number of spaces for indentation
* @param {number} depth - Current nesting depth for indentation
* @returns {string} The JSON string representation
* @private
*/
function stringifyElementRecursively(elem: jsonx.JsonElement, replacer?: (double | string)[], space?: int | string, depth: int = 0): string {
    const indentStr = getIndentation(space, depth)
    const nextIndentStr = getIndentation(space, depth + 1)
    const hasIndentation = space != undefined && nextIndentStr.length > 0

    switch (elem.jsonType) {
        case jsonx.JsonType.JsonNull:
            return "null"

        case jsonx.JsonType.JsonUndefined:
            return "undefined"

        case jsonx.JsonType.JsonTrue:
            return "true"

        case jsonx.JsonType.JsonFalse:
            return "false"

        case jsonx.JsonType.JsonString:
            return EscapeStringBuilder.process(elem.asString())

        case jsonx.JsonType.JsonNumber:
            const longValue = elem.tryAsLong()
            if (longValue != undefined) {
                return longValue.toString()
            }
            const doubleValue = elem.tryAsDouble()
            if (doubleValue != undefined) {
                return doubleValue.toString()
            }
            const bigintValue = elem.tryAsBigInt()
            if (bigintValue != undefined) {
                return bigintValue.toString()
            }
            return "null"

        case jsonx.JsonType.JsonArray:
            const array = elem.asArray()
            if (array.length == 0) {
                return "[]"
            }
            let arrayResult: string = "["
            if (hasIndentation) {
                arrayResult = arrayResult.concat("\n")
            }
            for (let i = 0; i < array.length; i++) {
                if (hasIndentation) {
                    arrayResult = arrayResult.concat(nextIndentStr)
                }
                arrayResult = arrayResult.concat(stringifyElementRecursively(array[i], replacer, space, depth + 1))
                if (i < array.length - 1) {
                    arrayResult = arrayResult.concat(",")
                }
                if (hasIndentation) {
                    arrayResult = arrayResult.concat("\n")
                }
            }
            if (hasIndentation) {
                arrayResult = arrayResult.concat(indentStr)
            }
            arrayResult = arrayResult.concat("]")
            return arrayResult

        case jsonx.JsonType.JsonObject:
            const it = elem.$_iterator()
            const entries = Array.from(it)
            if (entries.length == 0) {
                return "{}"
            }
            let objectResult: string = "{"
            if (hasIndentation) {
                objectResult = objectResult.concat("\n")
            }
            for (let i = 0; i < entries.length; i++) {
                const key = entries[i][0]
                const value = entries[i][1]
                if (value.jsonType == jsonx.JsonType.JsonUndefined) {
                    continue
                }
                if (hasIndentation) {
                    objectResult = objectResult.concat(nextIndentStr)
                }
                objectResult = objectResult.concat(EscapeStringBuilder.process(key))
                objectResult = objectResult.concat(":")
                if (hasIndentation) {
                    objectResult = objectResult.concat(" ")
                }
                objectResult = objectResult.concat(stringifyElementRecursively(value, replacer, space, depth + 1))
                if (i < entries.length - 1) {
                    objectResult = objectResult.concat(",")
                }
                if (hasIndentation) {
                    objectResult = objectResult.concat("\n")
                }
            }
            if (hasIndentation) {
                objectResult = objectResult.concat(indentStr)
            }
            objectResult = objectResult.concat("}")
            return objectResult

        default:
            return "null"
    }
}

class EscapeStringBuilder {
    static escapeTable: FixedArray<string> = EscapeStringBuilder.createEscapeTable()
    static controlCharsRange: int = 32

    static createEscapeTable(): FixedArray<string> {
        const asciiRange: int = 128
        const table: FixedArray<string> = new FixedArray<string>(asciiRange)
        for (let code = 0; code < asciiRange; code++) {
            switch (code.toChar()) {
                case c'\"':
                    table[code] = "\\\""
                    break
                case c'\\':
                    table[code] = "\\\\"
                    break
                case c'\b':
                    table[code] = "\\b"
                    break
                case c'\f':
                    table[code] = "\\f"
                    break
                case c'\n':
                    table[code] = "\\n"
                    break
                case c'\r':
                    table[code] = "\\r"
                    break
                case c'\t':
                    table[code] = "\\t"
                    break
                default:
                    if (code < EscapeStringBuilder.controlCharsRange) {
                        table[code] = "\\u".concat(code.toString(16).padStart(4, "0"))
                    }
                    break
            }
        }        
        return table
    }

    public static process(str: string): string {
        let result: string = "\""
        let begin: int = 0
        for (let i = 0; i < str.length; i++) {
            const ch = str.charAt(i)
            const code = ch.toInt()
            if (code >= EscapeStringBuilder.controlCharsRange && ch != c'\"' && ch != c'\\') {
                continue
            }
            if (begin < i) {
                result = result.concat(str.substring(begin, i))
            }
            result = result.concat(EscapeStringBuilder.escapeTable[code])
            begin = i + 1
        }
        if (begin < str.length) {
            result = result.concat(str.substring(begin))
        }
        return result.concat("\"")
    }
}

function getIndentation(space?: int | string, depth: int = 0): string {
    if (space == undefined) {
        return ""
    }
    if (space! instanceof string)
    {
        let unit = (space! as string).substring(0, 10)
        return unit.repeat(depth)
    }
    let lower = (space! as int) > 0
        ? (space! as int)
        : 0
    let aligned = lower > 10
        ? 10
        : lower
    return " ".repeat(aligned).repeat(depth)
}


class TypeColor {
    readonly static VISITED : int = 1
    readonly static AT_STACK : int = 2

    readonly static STRING_CLASS = Class.of("")

    readonly static OBJECT_CLASS = Class.from<Object>()
    readonly static NULL_CLASS = Class.from<null>()
    readonly static ARRAY_CLASS = Class.from<Array<object>>()

    type: Class
    color: int
    constructor(type: Class, color: int) {
        this.type = type
        this.color = color
    }

}

class Position {
    row: int = 1
    col: int = 1
    index: int = -1

    makeCopy(): Position {
        let c = new Position()
        c.row = this.row
        c.col = this.col
        c.index = this.index
        return c
    }

    restoreFromCopy(c: Position) {
        this.row = c.row
        this.col = c.col
        this.index = c.index
    }
}

export class JSONParser {
    private json: String
    private curPos: Position
    private curChar: char
    private escapeWhitespaces: boolean
    // TODO(ivan-tyulyandin): replace the hardcode with proper use of Array<T>/Map<T> in JSONValue inheritors

    constructor(json: String) {
        this.json = json
        this.escapeWhitespaces = true
        this.curPos = new Position()
    }

    private getCurPosDescr(): String {
        return "" + this.curPos.row + ":" + this.curPos.col
    }

    private getNextChar(): boolean {
        do {
            ++this.curPos.index
            if (this.curPos.index >= this.json.getLength()) {
                this.curChar = c'\0'
                return false
            }
            this.curChar = this.json.charAt(this.curPos.index)
            ++this.curPos.col
            if (this.curChar == c'\n') {
                ++this.curPos.row
                this.curPos.col = 1
            }
        } while (this.escapeWhitespaces && (this.curChar == c' ' || this.curChar == c'\t' ||
            this.curChar == c'\n' || this.curChar == c'\r'))
        return true
    }

    /**
     * Parses JSON into JSONValue
     *
     * @param json: String - a string with JSON
     *
     * @return JSONValue - JSON representation
     *
     * @throws SyntaxError if JSON is invalid
     *
     */
    private parse(res: JSONNumber): JSONNumber {
        let number = new StringBuilder()
        let posCopy = this.curPos.makeCopy()
        do {
            number.append(this.curChar)
            posCopy = this.curPos.makeCopy()
        } while (this.getNextChar() && (Char.isDecDigit(this.curChar) ||
                                               this.curChar == c'.' ||
                                               this.curChar == c'e' ||
                                               this.curChar == c'-' ||
                                               this.curChar == c'+' ))
        res.value = Double.parseFloat(number.toString())
        try {
            res.bigintValue = new BigInt(number.toString())
        } catch (e) {
            res.bigintValue = new BigInt()
        }
        this.curPos.restoreFromCopy(posCopy)
        return res
    }

    private parse(res: JSONString): JSONString {
        let resBuilder: StringBuilder = new StringBuilder()
        this.escapeWhitespaces = false
        let begin: int = this.curPos.index + 1
        while (this.getNextChar() && this.curChar != JSONString.END_CHAR) {
            if (this.curChar != c'\\') {
                continue
            }
            if (begin < this.curPos.index) {
                resBuilder.append(this.json.substring(begin, this.curPos.index))
            }
            if (!this.getNextChar()) {
                throw new SyntaxError(`Unexpected end of String at ${this.getCurPosDescr()}`)
            }
            switch (this.curChar) {
                case c'"':
                    resBuilder.append("\"")
                    break
                case c'\\':
                    resBuilder.append("\\")
                    break
                case c'/':
                    resBuilder.append("/")
                    break
                case c'b':
                    resBuilder.append("\b")
                    break
                case c'f':
                    resBuilder.append("\f")
                    break
                case c'n':
                    resBuilder.append("\n")
                    break
                case c'r':
                    resBuilder.append("\r")
                    break
                case c't':
                    resBuilder.append("\t")
                    break
                case c'u':
                    resBuilder.append(this.readUnicodeEscape())
                    break
                default:
                    throw new SyntaxError(`Bad escape sequence \\${this.curChar} at ${this.getCurPosDescr()}`)
            }
            begin = this.curPos.index + 1
        }
        if (this.curChar == JSONString.END_CHAR) {
            if (begin < this.curPos.index) {
                resBuilder.append(this.json.substring(begin, this.curPos.index))
            }
            res.value = resBuilder.toString()
        } else {
            throw new SyntaxError("Unexpected end of String at " + this.getCurPosDescr())
        }
        this.escapeWhitespaces = true
        return res
    }

    private readUnicodeEscape(): string {
        let sb = new StringBuilder()
        for (let i = 0; i < 4; i++) {
            if (!this.getNextChar() || this.curChar == JSONString.END_CHAR) {
                throw new SyntaxError(`Incomplete Unicode escape sequence at ${this.getCurPosDescr()}`)
            }
            if (!Char.isHexDigit(this.curChar)) {
                throw new SyntaxError(`Invalid hexadecimal digit '${this.curChar}' at ${this.getCurPosDescr()}`)
            }
            sb.append(this.curChar)
        }
        return String.fromCharCode(Double.parseInt(sb.toString(), 16))
    }

    static parse(json: String): JSONValue {
        let parser = new JSONParser(json)
        let res: JSONValue = new JSONValue()
        const val = parser.parse(res)
        if (parser.getNextChar()) {
            throw new SyntaxError(`Unexpected char "${parser.curChar}"  at ${parser.getCurPosDescr()}`)
        }
        return val
    }

    private parse(res: JSONTrue): JSONTrue {
        let metTrue = this.getNextChar() && this.curChar == c'r'
                    && this.getNextChar() && this.curChar == c'u'
                    && this.getNextChar() && this.curChar == c'e'
        if (!metTrue) {
            throw new SyntaxError("Expected true at " + this.getCurPosDescr())
        }
        return res
    }

    private parse(res: JSONFalse): JSONFalse {
        let metFalse = this.getNextChar() && this.curChar == c'a'
                    && this.getNextChar() && this.curChar == c'l'
                    && this.getNextChar() && this.curChar == c's'
                    && this.getNextChar() && this.curChar == c'e'
        if (!metFalse) {
            throw new SyntaxError("Expected false at " + this.getCurPosDescr())
        }
        return res
    }

    private parse(res: JSONNull): JSONNull {
        let metNull = this.getNextChar() && this.curChar == c'u'
                    && this.getNextChar() && this.curChar == c'l'
                    && this.getNextChar() && this.curChar == c'l'
        if (!metNull) {
            throw new SyntaxError("Expected null at " + this.getCurPosDescr())
        }
        return res
    }

    private parse(res: JSONArray): JSONArray {
        let posCopy = this.curPos.makeCopy()
        this.getNextChar()
        if (this.curChar == JSONArray.END_CHAR) {
            return res
        }
        this.curPos.restoreFromCopy(posCopy)

        res.values.push(this.parse(new JSONValue()))
        // Parse JSONArray until ]
        while (this.getNextChar()) {
            switch (this.curChar) {
                case c',':
                    res.values.push(this.parse(new JSONValue()))
                    break
                // TODO(ivan-tyulyandin): replace by JSONArray.END_CHAR, frontend bug
                case c']':
                    return res
                default:
                    throw new SyntaxError("Expected , or " + JSONArray.END_CHAR.toString() + " at " + this.getCurPosDescr() + ", got " + this.curChar.toString())
            }
        }
        return res
    }

    private parseKeyValue(res: JSONObject): JSONObject {
        res.keys_.push(this.parse(new JSONValue()) as JSONString)
        this.getNextChar()
        if (this.curChar  != c':') {
            throw new SyntaxError("Expected : \",\" at " + this.getCurPosDescr() + " got \"" + this.curChar + "\"")
        }
        res.values.push(this.parse(new JSONValue()))
        return res
    }

    private parse(res: JSONObject): JSONObject {
        let posCopy = this.curPos.makeCopy()
        this.getNextChar()
        if (this.curChar == JSONObject.END_CHAR) {
            return res
        }
        this.curPos.restoreFromCopy(posCopy)

        res = this.parseKeyValue(res)
        // Parse JSONObject until }
        while (this.getNextChar()) {
            switch (this.curChar) {
                case c',':
                    res = this.parseKeyValue(res)
                    break
                // TODO(ivan-tyulyandin): replace by JSONObject.END_CHAR, frontend bug
                case c'}':
                    return res
                default:
                    throw new SyntaxError("Expected \",\" or \"" + JSONObject.END_CHAR.toString() + "\" at " + this.getCurPosDescr() + ", got \"" + this.curChar.toString() + "\"")
            }
        }
        return res
    }

    private parse(res: JSONValue): JSONValue {
        // Fetch next symbol and call related parse method
        // TODO(ivan-tyulyandin): replace with switch, does not work due to frontend bug with switch over static fields with equal names
        this.getNextChar()
        if (this.curChar == JSONObject.START_CHAR) {
            let obj = new JSONObject()
            this.parse(obj)
            res = obj
        } else if (this.curChar  == JSONArray.START_CHAR) {
            let arr = new JSONArray()
            this.parse(arr)
            res = arr
        } else if (this.curChar == JSONString.START_CHAR) {
            let str = new JSONString()
            this.parse(str)
            res = str
        } else if (this.curChar == JSONTrue.START_CHAR) {
            let tr = new JSONTrue()
            this.parse(tr)
            res = tr
        } else if (this.curChar == JSONFalse.START_CHAR) {
            let fls = new JSONFalse()
            this.parse(fls)
            res = fls
        } else if (this.curChar == JSONNull.START_CHAR) {
            let nl = new JSONNull()
            this.parse(nl)
            res = nl
        } else if (Char.isDecDigit(this.curChar) || this.curChar == c'-') {
            let n = new JSONNumber()
            this.parse(n)
            res = n
        } else {
            throw new SyntaxError("Unexpected char \"" + this.curChar + "\" at " + this.getCurPosDescr())
        }
        return res
    }
}

type Types = reflect.internals.Types

class JSONValueParser {
    private reviver: ((key: string, value: Any) => Any) | undefined
    private bigIntMode: int

    private static readonly SETTER_PREFIX = "%%set-"

    constructor(reviver: ((key: string, value: Any) => Any) | undefined, bigIntMode: int) {
        this.reviver = reviver
        this.bigIntMode = bigIntMode
    }

    private createNumber(value: JSONNumber, type: Class): Any {
        if (type === BigIntType && (this.bigIntMode === 2 || (!Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new BigInt(value.bigintValue)
        } else if ((Types.isDoubleType(type) || type.isEnum()) && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value)
        } else if ((Types.isIntType(type) || type.isEnum()) && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value).toInt()
        } else if ((Types.isLongType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return new Long().createFromJSONValue(value)
        } else if ((Types.isShortType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return new Short().createFromJSONValue(value)
        } else if ((Types.isFloatType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return new Float().createFromJSONValue(value)
        } else if ((Types.isByteType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return new Byte().createFromJSONValue(value)
        } else {
            throw new Error(type.getName() + " is expected, but get " + Class.of(value.value).getName() + " with bigIntMode=" + this.bigIntMode)
        }
    }

    parse(value: JSONValue, type: Class): Any {
        if (value instanceof JSONObject && type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (value instanceof JSONArray && type.isFixedArray()) {
            return this.parseArray(value as JSONArray, type)
        } else if (value instanceof JSONNumber) {
            const num = this.createNumber(value as JSONNumber, type)
            if (type.isEnum()) {
                let valArray = type.getStaticMethod('values')!.invoke() as FixedArray<Any>
                for (let idx = 0; idx < valArray.length; idx++) {
                    if (valArray[idx] == num) {
                        return valArray[idx]
                    }
                }
            }
            return num
        } else if (value instanceof JSONString && (type === TypeColor.STRING_CLASS || type.isEnum())) {
            const str = (value as JSONString).value
            if (type.isEnum()) {
                let strArray = type.getStaticMethod('values')!.invoke() as FixedArray<Any>
                for (let idx = 0; idx < strArray.length; idx++) {
                    if (strArray[idx] == str) {
                        return strArray[idx]
                    }
                }
            }
            return str
        } else if (value instanceof JSONTrue && Types.isBooleanType(type)) {
            return new Boolean(true)
        } else if (value instanceof JSONFalse && Types.isBooleanType(type)) {
            return new Boolean(false)
        } else if (value instanceof JSONNull && type === TypeColor.NULL_CLASS) {
            return null
        } else if (value instanceof JSONObject && this.isParseableObjectType(type)) {
            return this.parseObject(value as JSONObject, type)
        } else {
            const valueType = Class.of(value)
            throw new Error(type.getName() + " is expected, but get " + valueType.getName())
        }
    }

    private isParseableObjectType(type: Class): boolean {
        return !(type === TypeColor.STRING_CLASS ||
                Types.isPrimitiveType(type) ||
                Types.isCharType(type) ||
                type === TypeColor.NULL_CLASS ||
                type.isSubtypeOf(TypeColor.ARRAY_CLASS) ||
                type.isFixedArray() ||
                type.isInterface())
    }

    private parseObject(jsonObj: JSONObject, classType: Class): Object {
        let setClassFieldFn = (obj: object, fieldName: string, fieldVal: Any) => {
            const field = searchInstanceFieldInHierarchy(Class.of(obj), fieldName)!
            field.setValue(obj, fieldVal)
        }

        if (this.reviver != undefined) {
            setClassFieldFn = (obj: object, fieldName: string, fieldVal: Any): void => {
                const revivedVal = this.reviver!(fieldName, fieldVal)

                const field = searchInstanceFieldInHierarchy(Class.of(obj), fieldName)!
                field.setValue(obj, revivedVal)
            }
        }

        const obj = JSONValueParser.classGetDefaultCtor(classType).createInstance() as object
        let hasJsonParseIgnore: boolean = false
        let hasJsonStringifyIgnore: boolean = false
        const jsonObjFields = jsonObj.getFields()

        for (const classField of getClassInstanceFieldsAll(classType)) {
            hasJsonParseIgnore = JSONAPI.getJSONParseIgnore(classField);
            if (hasJsonParseIgnore) {
                continue
            }

            let jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                hasJsonStringifyIgnore = JSONAPI.getJSONStringifyIgnore(classField);
                if (hasJsonStringifyIgnore) {
                    continue
                }
                let fieldRename = JSONAPI.getJSONRename(classField);
                if (fieldRename == undefined) {
                    throw new Error("Cannot find " + classField.getName() + " in keys of " + classType.getName())
                }
                jsonFieldVal = jsonObjFields.get(fieldRename)
                if (jsonFieldVal === undefined) {
                    throw new Error("Cannot find rename " + fieldRename + " in keys of " + classType.getName())
                }
            }

            const classFieldVal = this.parse(jsonFieldVal, classField.getType())
            setClassFieldFn(obj, classField.getName(), classFieldVal)
        }

        for (const method of classType.getInstanceMethods()) {
            const methodName = method.getName()
            if (methodName.startsWith(JSONValueParser.SETTER_PREFIX)) {
                const propName = methodName.substring(JSONValueParser.SETTER_PREFIX.length)
                const jsonFieldVal = jsonObjFields.get(propName)
                if (jsonFieldVal !== undefined) {
                    const propType = method.getParameterType(0)!
                    const classFieldVal = this.parse(jsonFieldVal, propType)

                    if (this.reviver !== undefined) {
                        const revivedVal = this.reviver!(propName, classFieldVal)
                        const setterArgs: FixedArray<Any> = [revivedVal]
                        method.invoke(obj, setterArgs)
                    } else {
                        const setterArgs: FixedArray<Any> = [classFieldVal]
                        method.invoke(obj, setterArgs)
                    }
                }
            }
        }

        return obj
    }

    private static classGetDefaultCtor(cls: Class): reflect.Constructor {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return ctor
            }
        }
        throw new Error(`class ${cls.getName()} doesn't have default ctor`)
    }

    private parseArray(jVal: JSONArray, aType: Class): Object | null {
        const len = jVal.values.length
        const componentType = aType.getFixedArrayComponentType()!
        const arr = reflect.internals.createFixedArray(componentType, len) as FixedArray<Any>

        let setArrayElementFn = (a: FixedArray<Any>, i: int, v: Any) => { a[i] = v }

        if (this.reviver !== undefined) {
            setArrayElementFn = (array: FixedArray<Any>, index: int, elemVal: Any): void => {
                const revivedVal = this.reviver!(`${index}`, elemVal)
                array[index] = revivedVal !== undefined ? revivedVal : null
            }
        }

        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i) as JSONValue, aType.getFixedArrayComponentType()!)
            setArrayElementFn(arr, i, elem)
        }
        return arr
    }
}

export enum JsonTokenKind
{
    CurlyOpen    = 0,
    CurlyClose   = 1,
    SquareOpen   = 2,
    SquareClose  = 3,
    Colon        = 4,
    Comma        = 5,
    String       = 6,
    Number       = 7,
    True         = 8,
    False        = 9,
    Null         = 10
}

export class JsonTokenSpan
{
    start: int
    end: int
}

export class JsonToken
{
    kind: JsonTokenKind
    span: JsonTokenSpan
    value: string
    constructor(kind: JsonTokenKind, span: JsonTokenSpan, value: string) {
        this.kind = kind
        this.span = span
        this.value = value
    }
}

// Grammar:
// =============================================================
// start  ::= value
// value  ::= object | array | double | string | true | false | null
// object ::= "{" [pair ("," pair)*] "}"
// array  ::= "[" [value ("," value)*] "]"
// pair   ::= string ":" value
// string ::= ESCAPED_STRING
// number ::= SIGNED_NUMBER
// true   ::= "true"
// false  ::= "false"
// null   ::= "null"

/**
 * Error thrown when parsing JSON fails.
 */
export class JsonParseError extends Error {
    public constructor(msg: string, start_offset?: int, end_offset?: int) {
        super(`${msg} at ${start_offset}..${end_offset}`)
    }
}

class JsonElementParser
{
    source: string
    tokens: Array<JsonToken>
    position: int
    reviver?: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement
    bigIntOpt: jsonx.BigIntMode = jsonx.BigIntMode.DEFAULT

    constructor(source: string, options?: jsonx.ParseOptions) {
        this.source = source
        this.tokens = JsonLexer.fromString(source).tokenize()
        this.position = 0
        this.bigIntOpt = options ? options.bigIntMode : jsonx.BigIntMode.DEFAULT
    }

    constructor(source: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement, options?: jsonx.ParseOptions) {
        this.source = source
        this.tokens = JsonLexer.fromString(source).tokenize()
        this.position = 0
        this.reviver = reviver
        this.bigIntOpt = options ? options.bigIntMode : jsonx.BigIntMode.DEFAULT
    }

    public lookahead(offset: int): JsonToken | undefined
    {
        if (this.position + offset >= this.tokens.length) {
            return undefined
        }
        return this.tokens[this.position + offset]
    }

    public expect(kind: JsonTokenKind): JsonToken
    {
        let token = this.lookahead(0)
        if (token == undefined || token.kind != kind) {
            let start = token?.span.start ?? 0;
            let end = token?.span.end ?? 0;
            throw new JsonParseError(`Expected ${kind} but got ${token?.kind}`, start, end)
        }
        this.position++
        return token
    }

    public parse(): jsonx.JsonElement
    {
        let result = this.parseValue()
        if (this.reviver != undefined) {
            return this.reviver!("", result)
        }
        return result
    }

    private parseValue(): jsonx.JsonElement
    {
        let maybe_token = this.lookahead(0)
        if (maybe_token == undefined) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        if (maybe_token.kind == JsonTokenKind.String) {
            return this.parseString()
        }
        if (maybe_token.kind == JsonTokenKind.Number) {
            return this.parseNumber()
        }
        if (maybe_token.kind == JsonTokenKind.True) {
            return this.parseBool()
        }
        if (maybe_token.kind == JsonTokenKind.False) {
            return this.parseBool()
        }
        if (maybe_token.kind == JsonTokenKind.Null) {
            return this.parseNull()
        }
        if (maybe_token.kind == JsonTokenKind.CurlyOpen) {
            return this.parseObject()
        }
        if (maybe_token.kind == JsonTokenKind.SquareOpen) {
            return this.parseArray()
        }
        throw new JsonParseError("Expected string, number, true, false, null, object, or array", maybe_token.span.start, maybe_token.span.end)
    }

    private parseArray(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.SquareOpen)
        let array = new Array<jsonx.JsonElement>()
        let index = 0
        while (this.lookahead(0)?.kind != JsonTokenKind.SquareClose) {
            let value = this.parseValue()
            if (this.reviver != undefined) {
                value = this.reviver!(`${index}`, value)
            }
            array.push(value)
            index++
            if (this.lookahead(0)?.kind == JsonTokenKind.Comma) {
                this.expect(JsonTokenKind.Comma)
            }
        }
        this.expect(JsonTokenKind.SquareClose)
        return jsonx.JsonElement.createArray(array)
    }

    private parseObject(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.CurlyOpen)
        let object = new Map<string, jsonx.JsonElement>()
        while (this.lookahead(0)?.kind != JsonTokenKind.CurlyClose) {
            let pair = this.parsePair()
            let key = pair[0]
            let value = pair[1]
            if (this.reviver != undefined) {
                value = this.reviver!(key, value)
            }
            object.set(key, value)
            if (this.lookahead(0)?.kind == JsonTokenKind.Comma) {
                this.expect(JsonTokenKind.Comma)
            }
        }
        this.expect(JsonTokenKind.CurlyClose)
        return jsonx.JsonElement.createObject(object)
    }

    private parsePair(): [string, jsonx.JsonElement]
    {
        let key = this.parseString().asString()
        this.expect(JsonTokenKind.Colon)
        let value = this.parseValue()
        return [key, value] as [string, jsonx.JsonElement]
    }

    private parseString(): jsonx.JsonElement
    {
        let token = this.expect(JsonTokenKind.String)
        return jsonx.JsonElement.createString(token.value)
    }

    private parseNumber(): jsonx.JsonElement
    {
        // can be extended with bigints later
        let token = this.expect(JsonTokenKind.Number)
        if (token.value.includes(".")) {
            return jsonx.JsonElement.createDouble(Double.parseFloat(token.value))
        }
        if (this.bigIntOpt == jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT) {
            return jsonx.JsonElement.createBigInt(BigInt(token.value))
        }
        if (this.bigIntOpt == jsonx.BigIntMode.PARSE_AS_BIGINT) {
            try {
                return jsonx.JsonElement.createLong(Long.parseInt(token.value, 10))
            } catch (e) {
                if (e instanceof FormatError) {
                    return jsonx.JsonElement.createBigInt(BigInt(token.value))
                }
                throw e
            }
        }
        return jsonx.JsonElement.createLong(Long.parseInt(token.value, 10))
    }

    private parseBool(): jsonx.JsonElement
    {
        let maybe_token = this.lookahead(0)
        if (maybe_token == undefined) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        if (maybe_token.kind == JsonTokenKind.True) {
            this.expect(JsonTokenKind.True)
            return jsonx.JsonElement.createBoolean(true)
        }
        if (maybe_token.kind == JsonTokenKind.False) {
            this.expect(JsonTokenKind.False)
            return jsonx.JsonElement.createBoolean(false)
        }
        throw new JsonParseError("Expected true or false", maybe_token.span.start, maybe_token.span.end)
    }

    private parseNull(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.Null)
        return jsonx.JsonElement.createNull()
    }
}

/**
 * Optimized JSON parser using streaming parsing (no intermediate token array).
 * Implements:
 * - Scheme 1: Stream parsing (single pass, no token array)
 * - Scheme 2: Zero-copy string parsing (substring without escape)
 * - Scheme 3: Fast path for common literals (null, true, false)
 */
class JsonElementParserOptimized {
    private source: string
    private index: int
    private length: int
    private reviver?: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement
    private bigIntOpt: jsonx.BigIntMode = jsonx.BigIntMode.DEFAULT

    constructor(source: string, options?: jsonx.ParseOptions) {
        this.source = source
        this.index = 0
        this.length = source.length
        this.bigIntOpt = options ? options.bigIntMode : jsonx.BigIntMode.DEFAULT
    }

    constructor(source: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement, options?: jsonx.ParseOptions) {
        this.source = source
        this.index = 0
        this.length = source.length
        this.reviver = reviver
        this.bigIntOpt = options ? options.bigIntMode : jsonx.BigIntMode.DEFAULT
    }

    public parse(): jsonx.JsonElement {
        this.skipWhitespace()
        if (this.isAtEnd()) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        let result = this.parseValue()
        if (this.reviver != undefined) {
            return this.reviver!("", result)
        }
        return result
    }

    private peek(): char {
        if (this.index >= this.length) {
            return c'\0'
        }
        return this.source.charAt(this.index)
    }

    private advance(): char {
        if (this.index >= this.length) {
            return c'\0'
        }
        return this.source.charAt(this.index++)
    }

    private isAtEnd(): boolean {
        return this.index >= this.length
    }

    private parseValue(): jsonx.JsonElement {
        let ch = this.peek()

        // Fast path for common literals (Scheme 3)
        // If fast path fails, immediately throw error to avoid unnecessary checks
        if (ch == c'n') {
            // Check "null" directly
            if (this.index + 4 <= this.length &&
                this.source.charAt(this.index + 1) == c'u' &&
                this.source.charAt(this.index + 2) == c'l' &&
                this.source.charAt(this.index + 3) == c'l') {
                this.index += 4
                return jsonx.JsonElement.createNull()
            }
            throw new JsonParseError("Unexpected character, expect null", this.index, this.index + 3)
        }

        if (ch == c't') {
            // Check "true" directly
            if (this.index + 4 <= this.length &&
                this.source.charAt(this.index + 1) == c'r' &&
                this.source.charAt(this.index + 2) == c'u' &&
                this.source.charAt(this.index + 3) == c'e') {
                this.index += 4
                return jsonx.JsonElement.createBoolean(true)
            }
            throw new JsonParseError("Unexpected character, expect true", this.index, this.index + 3)
        }

        if (ch == c'f') {
            // Check "false" directly
            if (this.index + 5 <= this.length &&
                this.source.charAt(this.index + 1) == c'a' &&
                this.source.charAt(this.index + 2) == c'l' &&
                this.source.charAt(this.index + 3) == c's' &&
                this.source.charAt(this.index + 4) == c'e') {
                this.index += 5
                return jsonx.JsonElement.createBoolean(false)
            }
            throw new JsonParseError("Unexpected character, expect false", this.index, this.index + 4)
        }

        // General path
        switch (ch) {
            case c'"':
                return this.parseString()
            case c'{':
                return this.parseObject()
            case c'[':
                return this.parseArray()
            case c'-':
            case c'0': case c'1': case c'2': case c'3': case c'4':
            case c'5': case c'6': case c'7': case c'8': case c'9':
                return this.parseNumber()
            default:
                throw new JsonParseError("Unexpected character", this.index, this.index)
        }
    }

    // Zero-copy string parsing (Scheme 2)
    private parseString(): jsonx.JsonElement {
        this.advance()  // Skip opening quote
        let start = this.index

        // First pass: check for escape sequences
        while (this.index < this.length) {
            let ch = this.source.charAt(this.index)
            if (ch == c'\\') {
                // Has escape, need to process
                return this.parseStringWithEscape(start)
            }
            if (ch == c'"') {
                // No escape, zero-copy substring
                let result = this.source.substring(start, this.index)
                this.index++  // Skip closing quote
                return jsonx.JsonElement.createString(result)
            }
            this.index++
        }

        throw new JsonParseError("Unterminated string", start, this.index)
    }

    private parseStringWithEscape(start: int): jsonx.JsonElement {
        // Direct string concatenation
        let result = ""
        let lastEnd = start

        // Process escape sequences
        while (this.index < this.length) {
            let ch = this.source.charAt(this.index)
            if (ch == c'\0') {
                throw new JsonParseError("Unterminated string", start, this.index)
            }
            this.index++

            if (ch == c'\\') {
                // Flush content before escape using concat
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                result = result.concat(this.parseEscape())
                lastEnd = this.index
            } else if (ch == c'"') {
                // Flush remaining content
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                return jsonx.JsonElement.createString(result)
            }
            // Normal character - continue, will be flushed with next escape or end
        }

        throw new JsonParseError("Unterminated string", start, this.index)
    }

    private parseEscape(): string {
        if (this.index >= this.length) {
            throw new JsonParseError("Incomplete escape sequence", this.index, this.index)
        }

        let ch = this.advance()
        switch (ch) {
            case c'"':
                return "\""
            case c'\\':
                return "\\"
            case c'/':
                return "/"
            case c'b':
                return "\b"
            case c'f':
                return "\f"
            case c'n':
                return "\n"
            case c'r':
                return "\r"
            case c't':
                return "\t"
            case c'u':
                return this.parseUnicodeEscape()
            default:
                throw new JsonParseError(`Invalid escape sequence '\\${ch}'`, this.index - 1, this.index)
        }
    }

    private parseUnicodeEscape(): string {
        if (this.index + 4 > this.length) {
            throw new JsonParseError("Incomplete Unicode escape sequence", this.index, this.index)
        }

        // Direct substring for 4 hex digits
        let hexStr = this.source.substring(this.index, this.index + 4)
        this.index += 4

        let codePoint = Double.parseInt(hexStr, 16).toInt()
        return String.fromCharCode(codePoint.toDouble())
    }

    // Optimized number parsing (Scheme 1 - direct from source)
    private parseNumber(): jsonx.JsonElement {
        let start = this.index
        let isFloat = false

        // Optional minus sign
        if (this.peek() == c'-') {
            this.index++
        }

        // Integer part
        this.parseIntegerPart(start)

        // Fractional part
        isFloat = this.parseFractionalPart(start)

        // Exponent part
        isFloat = this.parseExponentPart(start) || isFloat

        // Single substring operation
        let numStr = this.source.substring(start, this.index)

        if (isFloat) {
            return jsonx.JsonElement.createDouble(Double.parseFloat(numStr))
        } else {
            return this.parseInterger(numStr)
        }
    }

    private parseIntegerPart(start: int): void {
        if (this.peek() == c'0') {
            this.index++
        } else if (Char.isDecDigit(this.peek())) {
            while (Char.isDecDigit(this.peek())) {
                this.index++
            }
        } else {
            throw new JsonParseError("Invalid number format", start, this.index)
        }
    }

    private parseFractionalPart(start: int): boolean {
        if (this.peek() == c'.') {
            this.index++
            if (!Char.isDecDigit(this.peek())) {
                throw new JsonParseError("Invalid number format", start, this.index)
            }
            while (Char.isDecDigit(this.peek())) {
                this.index++
            }
            return true
        }
        return false
    }

    private parseExponentPart(start: int): boolean {
        let ch = this.peek()
        if (ch == c'e' || ch == c'E') {
            this.index++
            ch = this.peek()
            if (ch == c'+' || ch == c'-') {
                this.index++
            }
            if (!Char.isDecDigit(this.peek())) {
                throw new JsonParseError("Invalid number format", start, this.index)
            }
            while (Char.isDecDigit(this.peek())) {
                this.index++
            }
            return true
        }
        return false
    }

    private parseInterger(numStr: string): jsonx.JsonElement {
        if (this.bigIntOpt == jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT) {
            return jsonx.JsonElement.createBigInt(BigInt(numStr))
        }
        if (this.bigIntOpt == jsonx.BigIntMode.PARSE_AS_BIGINT) {
            try {
                return jsonx.JsonElement.createLong(Long.parseInt(numStr, 10))
            } catch (e) {
                if (e instanceof FormatError) {
                    return jsonx.JsonElement.createBigInt(BigInt(numStr))
                }
                throw e
            }
        }
        return jsonx.JsonElement.createLong(Long.parseInt(numStr, 10))
    }

    private parseArray(): jsonx.JsonElement {
        this.advance()  // '['
        this.skipWhitespace()

        let array: Array<jsonx.JsonElement> = new Array<jsonx.JsonElement>()
        let index = 0
        let hasReviver = this.reviver != undefined  // Cache for performance

        while (this.peek() != c']') {
            this.skipWhitespace()
            let value = this.parseValue()

            if (hasReviver) {
                value = this.reviver!(`${index}`, value)
            }

            array.push(value)
            index++

            this.skipWhitespace()
            if (this.peek() == c',') {
                this.advance()
            }
        }

        this.advance()  // ']'
        return jsonx.JsonElement.createArray(array)
    }

    private parseObject(): jsonx.JsonElement {
        this.advance()  // '{'
        this.skipWhitespace()

        // Directly create JsonElement and use setElement to avoid Map copy
        let result = new jsonx.JsonElement()
        let hasReviver = this.reviver != undefined  // Cache for performance
        let isEmpty = true

        while (this.peek() != c'}') {
            this.skipWhitespace()

            // Parse key
            let key = this.parseString().asString()

            this.skipWhitespace()
            if (this.advance() != c':') {
                throw new JsonParseError("Expected ':'", this.index - 1, this.index)
            }
            this.skipWhitespace()

            // Parse value
            let value = this.parseValue()

            if (hasReviver) {
                value = this.reviver!(key, value)
            }

            // Directly set element, avoids intermediate Map and copy
            result.setElement(key, value)
            isEmpty = false

            this.skipWhitespace()
            if (this.peek() == c',') {
                this.advance()
            }
        }

        this.advance()  // '}'
        if (!isEmpty) {
            return result
        } else {
            return jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>())
        }
    }

    private skipWhitespace(): void {
        while (this.index < this.length) {
            let ch = this.source.charAt(this.index)
            if (!Char.isWhiteSpace(ch)) {
                break
            }
            this.index++
        }
    }
}

export class JsonLexer
{
    chars: string
    index: int
    constructor(chars: string) {
        this.chars = chars
        this.index = 0
    }
    public static fromString(json: string): JsonLexer {
        return new JsonLexer(json)
    }

    /**
     * Tokenizes the JSON input and returns an array of tokens.
     * @returns Array of JsonToken objects representing the tokenized input
     * @throws JsonError if invalid JSON syntax is encountered
     */
    public tokenize(): JsonToken[] {
        let tokens = new Array<JsonToken>()

        while (!this.isAtEnd()) {
            this.skipWhitespace()
            if (this.isAtEnd()) break

            let token = this.nextToken()
            if (token != null) {
                tokens.push(token)
            }
        }

        return tokens
    }

    /**
     * Reads the next token from the input
     * @returns JsonToken or null if no valid token found
     * @throws JsonError if invalid syntax is encountered
     */
    private nextToken(): JsonToken | null {
        let start = this.index
        let ch = this.advance()

        switch (ch) {
            case c'{':
                return this.makeToken(JsonTokenKind.CurlyOpen, start, "{")
            case c'}':
                return this.makeToken(JsonTokenKind.CurlyClose, start, "}")
            case c'[':
                return this.makeToken(JsonTokenKind.SquareOpen, start, "[")
            case c']':
                return this.makeToken(JsonTokenKind.SquareClose, start, "]")
            case c':':
                return this.makeToken(JsonTokenKind.Colon, start, ":")
            case c',':
                return this.makeToken(JsonTokenKind.Comma, start, ",")
            case c'"':
                return this.readString(start)
            case c't':
                return this.readKeyword(start, "true", JsonTokenKind.True)
            case c'f':
                return this.readKeyword(start, "false", JsonTokenKind.False)
            case c'n':
                return this.readKeyword(start, "null", JsonTokenKind.Null)
            default:
                if (this.isDigit(ch) || ch == c'-') {
                    this.index-- // Back up to re-read the character
                    return this.readNumber(start)
                }
                throw new JsonParseError(`Unexpected character '${ch}' at position ${start}`, start, start)
        }
    }

    /**
     * Reads a JSON string token, handling escape sequences
     * @param start Starting position of the string
     * @returns JsonToken representing the string
     * @throws JsonError if string is malformed
     */
    private readString(start: int): JsonToken {
        let value: string = ""
        let begin = this.index

        while (!this.isAtEnd() && this.peek() != c'"') {
            let ch = this.advance()
            if (ch != c'\\') {
                continue
            }
            if (this.isAtEnd()) {
                throw new JsonParseError("Unterminated string escape at end of input", start, this.index)
            }
            if (begin < this.index - 1) {
                value = value.concat(this.chars.substring(begin, this.index - 1))
            }
            let escaped = this.advance()
            switch (escaped) {
                case c'"':
                    value = value.concat("\"")
                    break
                case c'\\':
                    value = value.concat("\\")
                    break
                case c'/':
                    value = value.concat("/")
                    break
                case c'b':
                    value = value.concat("\b")
                    break
                case c'f':
                    value = value.concat("\f")
                    break
                case c'n':
                    value = value.concat("\n")
                    break
                case c'r':
                    value = value.concat("\r")
                    break
                case c't':
                    value = value.concat("\t")
                    break
                case c'u':
                    value = value.concat(this.readUnicodeEscape())
                    break
                default:
                    throw new JsonParseError(`Invalid escape sequence '\\${escaped}' in string`, start, this.index)
            }
            begin = this.index
        }
        if (begin < this.index) {
            value = value.concat(this.chars.substring(begin, this.index))
        }

        if (this.isAtEnd()) {
            throw new JsonParseError("Unterminated string", start, this.index)
        }

        this.advance() // Consume closing quote
        return this.makeToken(JsonTokenKind.String, start, value)
    }

    /**
     * Reads a Unicode escape sequence (\uXXXX)
     * @returns The Unicode character as a string
     * @throws JsonError if escape sequence is malformed
     */
    private readUnicodeEscape(): string {
        let codeStr = new StringBuilder()
        for (let i = 0; i < 4; i++) {
            if (this.isAtEnd()) {
                throw new JsonParseError("Incomplete Unicode escape sequence")
            }
            let ch = this.advance()
            if (!this.isHexDigit(ch)) {
                throw new JsonParseError(`Invalid hexadecimal digit '${ch}' in Unicode escape`, this.index)
            }
            codeStr.append(ch)
        }

        let codePoint = Double.parseInt(codeStr.toString(), 16).toInt()
        return String.fromCharCode(codePoint)
    }

    /**
     * Reads a JSON number token
     * @param start Starting position of the number
     * @returns JsonToken representing the number
     * @throws JsonError if number format is invalid
     */
    private readNumber(start: int): JsonToken {
        let value = new StringBuilder()

        // Handle optional minus sign
        if (this.peek() == c'-') {
            value.append(this.advance())
        }

        // Read integer part
        if (this.peek() == c'0') {
            value.append(this.advance())
        } else if (this.isDigit(this.peek())) {
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value.append(this.advance())
            }
        } else {
            throw new JsonParseError("Invalid number format", start, this.index)
        }

        // Read decimal part if present
        if (!this.isAtEnd() && this.peek() == c'.') {
            value.append(this.advance())
            if (this.isAtEnd() || !this.isDigit(this.peek())) {
                throw new JsonParseError("Invalid number format: decimal point must be followed by digits", start, this.index)
            }
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value.append(this.advance())
            }
        }

        // Read exponent part if present
        if (!this.isAtEnd() && (this.peek() == c'e' || this.peek() == c'E')) {
            value.append(this.advance())
            if (!this.isAtEnd() && (this.peek() == c'+' || this.peek() == c'-')) {
                value.append(this.advance())
            }
            if (this.isAtEnd() || !this.isDigit(this.peek())) {
                throw new JsonParseError("Invalid number format: exponent must contain digits", start, this.index)
            }
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value.append(this.advance())
            }
        }

        return this.makeToken(JsonTokenKind.Number, start, value.toString())
    }

    /**
     * Reads a JSON keyword (true, false, null)
     * @param start Starting position of the keyword
     * @param expected The expected keyword string
     * @param kind The token kind for this keyword
     * @returns JsonToken representing the keyword
     * @throws JsonError if keyword doesn't match
     */
    private readKeyword(start: int, expected: string, kind: JsonTokenKind): JsonToken {
        this.index-- // Back up to re-read first character

        for (let i = 0; i < expected.length; i++) {
            if (this.isAtEnd() || this.advance() != expected.charAt(i)) {
                throw new JsonParseError(`Invalid keyword at position ${start}`, start, this.index)
            }
        }

        return this.makeToken(kind, start, expected)
    }

    /**
     * Creates a JsonToken with the given parameters
     * @param kind The token kind
     * @param start Starting position
     * @param value String value of the token
     * @returns New JsonToken instance
     */
    private makeToken(kind: JsonTokenKind, start: int, value: string): JsonToken {
        let span = new JsonTokenSpan()
        span.start = start
        span.end = this.index
        return new JsonToken(kind, span, value)
    }

    /**
     * Skips whitespace characters
     */
    private skipWhitespace(): void {
        while (!this.isAtEnd()) {
            let ch = this.peek()
            if (ch == c' ' || ch == c'\t' || ch == c'\n' || ch == c'\r') {
                this.advance()
            } else {
                break
            }
        }
    }

    /**
     * Advances to the next character and returns it
     * @returns The current character before advancing
     */
    private advance(): char {
        if (this.isAtEnd()) return c'\0'
        return this.chars.charAt(this.index++)
    }

    /**
     * Returns the current character without advancing
     * @returns The current character
     */
    private peek(): char {
        if (this.isAtEnd()) return c'\0'
        return this.chars.charAt(this.index)
    }

    /**
     * Checks if we've reached the end of input
     * @returns true if at end of input
     */
    private isAtEnd(): boolean {
        return this.index >= this.chars.length
    }

    /**
     * Checks if a character is a decimal digit
     * @param ch Character to check
     * @returns true if character is a digit
     */
    private isDigit(ch: char): boolean {
        return (ch.compareTo(c'0') >= 0) && (ch.compareTo(c'9') <= 0)
    }

    /**
     * Checks if a character is a hexadecimal digit
     * @param ch Character to check
     * @returns true if character is a hex digit
     */
    private isHexDigit(ch: char): boolean {
        return ((ch.compareTo(c'0') >= 0) && (ch.compareTo(c'9') <= 0)) ||
                ((ch.compareTo(c'A') >= 0) && (ch.compareTo(c'F') <= 0)) ||
                ((ch.compareTo(c'a') >= 0) && (ch.compareTo(c'f') <= 0))
    }
}

export class JSONValue extends Object {}

export class JSONObject extends JSONValue {
    keys_: Array<JSONString> = new Array<JSONString>()
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'{'
    readonly static END_CHAR = c'}'
    readonly static SEPARATOR = c':'
    readonly static DELIMETER = c','
    readonly static EMPTY = "{}"

    getFields(): Map<String, JSONValue> {
        const propsCount = this.keys_.length

        const result = new Map<String, JSONValue>()

        for (let i = 0; i < propsCount; i++) {
            const key = this.keys_[i]
            const val = this.values[i]

            result.set(key.value, val)
        }

        return result
    }

    public override toString(): String {
        let res = new StringBuilder([JSONObject.START_CHAR] as FixedArray<char>)
        for (let i = 0; i < this.keys_.length - 1; ++i) {
            res.append("" + this.keys_.at(i) + JSONObject.SEPARATOR + this.values.at(i) + JSONObject.DELIMETER)
        }
        if (this.keys_.length > 0) {
            res.append("" + this.keys_.at(this.keys_.length - 1) + JSONObject.SEPARATOR + this.values.at(this.keys_.length - 1))
        }
        res.append(JSONObject.END_CHAR)
        return res.toString()
    }
}

export class JSONArray extends JSONValue {
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'['
    readonly static END_CHAR = c']'
    readonly static SEPARATOR = c','

    public override toString(): String {
        let res = new StringBuilder([JSONArray.START_CHAR] as FixedArray<char>)
        for (let i = 0; i < this.values.length - 1; ++i) {
            res.append("" + this.values.at(i) + JSONArray.SEPARATOR)
        }
        if (this.values.length > 0) {
            res.append("" + this.values.at(this.values.length - 1))
        }
        res.append(JSONArray.END_CHAR)
        return res.toString()
    }
}

export class JSONNumber extends JSONValue {
    value: double
    bigintValue: bigint = new BigInt()

    public override toString(): String {
        return (new Double(this.value)).toString()
    }
}

export class JSONString extends JSONValue {
    value: String = ""
    readonly static START_CHAR = c'"'
    readonly static END_CHAR = c'"'

    public override toString(): String {
        return this.value
    }
}

export class JSONTrue extends JSONValue {
    readonly static value = "true"
    readonly static START_CHAR = c't'

    public override toString(): String {
        return JSONTrue.value
    }
}

export class JSONFalse extends JSONValue {
    readonly static value = "false"
    readonly static START_CHAR = c'f'

    public override toString(): String {
        return JSONFalse.value
    }
}

export class JSONNull extends JSONValue {
    readonly static value = "null"
    readonly static START_CHAR = c'n'

    public override toString(): String {
        return JSONNull.value
    }
}
