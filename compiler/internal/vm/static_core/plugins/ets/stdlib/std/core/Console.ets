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
 * The maximum depth of printing nested objects
 * Useful for cyclic linked objects
 */
const MAX_CONSOLE_PRINT_DEPTH: int = 10

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    LOG = 2,
    WARN = 3,
    ERROR = 4,
    PRINTLN = 5,
}

class ColumnMap {
    columnValue:Map<string, Array<string>> = new Map<string, Array<string>>();
    constructor() {}
}

/**
 * A Console class that provides access to standard output and error streams.
 * Supports printing various data types, timing operations, and indentation management.
 * @export
 * @final
 */
export final class Console {

    /**
     * @section Internal fields
     */
    private lvl2Buf: Map<LogLevel, StringBuilder>

    /**
     * Map to store named timers for performance measurements
     * @private
     */
    private timers: Map<string, long>

    /**
     * Current indentation level for formatted output
     * @private
     */
    private indentationLevel: int = 0;

    /**
     * Number of spaces used for each level of indentation
     * @private
     * @static
     * @readonly
     */
    private static readonly INDENT_SIZE: int = 2;

    /**
     * Represents the space creating the margin for indentation
     * @private
     * @static
     * @readonly
     */
    private static readonly INDENT_MARGIN: string = " ".repeat(Console.INDENT_SIZE)

    /**
     * Map to store named counters for counting operations
     * @private
     */
    private counters: Map<string, int>

    private static tableChars: Map<string, string> = new Map<string,string>([
        ["middleMiddle", "─"],
        ["rowMiddle", "┼"],
        ["topRight", "┐"],
        ["topLeft", "┌"],
        ["leftMiddle", "├"],
        ["topMiddle", "┬"],
        ["bottomRight", "┘"],
        ["bottomLeft", "└"],
        ["bottomMiddle", "┴"],
        ["rightMiddle", "┤"],
        ["left", "│ "],
        ["right", " │"],
        ["middle", " │ "],
    ])

    /**
     * Internal constructor for Console class
     * @internal
     */
    private constructor() {
        this.timers = new Map<string, long>
        this.counters = new Map<string, int>
        this.lvl2Buf = new Map<LogLevel, StringBuilder>
        this.lvl2Buf.set(LogLevel.DEBUG, new StringBuilder)
        this.lvl2Buf.set(LogLevel.INFO, new StringBuilder)
        this.lvl2Buf.set(LogLevel.LOG, new StringBuilder)
        this.lvl2Buf.set(LogLevel.WARN, new StringBuilder)
        this.lvl2Buf.set(LogLevel.ERROR, new StringBuilder)
        this.lvl2Buf.set(LogLevel.PRINTLN, new StringBuilder)
    }

    private static instance?: Console

    public static getInstance(): Console {
        if (Console.instance == undefined) {
            Console.instance = new Console()
        }
        return Console.instance!;
    }

    private native printString(s: String, lvl: int): void;

    private addToBuffer(s: String, level: LogLevel): void {
        if (s.length == 0) {
            return
        }
        let buf = this.lvl2Buf.get(level)!
        buf.append(s)
        // buffering strategies can be applied here based on buf size
        this.printString(buf.toString(), level.valueOf())
        this.lvl2Buf.set(level, new StringBuilder)
    }

    private get indent(): String {
        return Console.INDENT_MARGIN.repeat(this.indentationLevel)
    }

    private static AnyToPrint(o: Any): String {
        return DataFrame.toPrint(o, MAX_CONSOLE_PRINT_DEPTH)
    }

    /**
     * Implementations for printing primitive types
     * @param {string | boolean | byte | short | char | int | long | float | double | Any} i - The value to print
     * @returns {void}
     * @public
     * @deprecated Will be removed in future. Please use log instead
     */
    public print(i: boolean): void {
        this.addToBuffer(new Boolean(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: byte): void {
        this.addToBuffer(new Byte(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: short): void {
        this.addToBuffer(new Short(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: char): void {
        this.addToBuffer(new Char(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: int): void {
        this.addToBuffer(new Int(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: long): void {
        this.addToBuffer(new Long(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: float): void {
        this.addToBuffer(new Float(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: double): void {
        this.addToBuffer(new Double(i).toString(), LogLevel.PRINTLN)
    };
    public print(i: String): void {
        this.addToBuffer(i, LogLevel.PRINTLN)
    };
    public print(o: Any): void {
        this.print(Console.AnyToPrint(o))
    };

    /**
     * @section Println definitions
     */

    /**
     * Prints a newline
     * @returns {void}
     * @public
     * @deprecated Will be removed in future. Please use log instead
     */
    public println(): void {
        this.print(c'\n')
    }

    /**
     * Prints an object followed by a newline to stdout
     * @param {Any | string | boolean | byte | short | char | int | long | float | double} i - The value to print
     * @returns {void}
     * @public
     * @deprecated Will be removed in future. Please use log instead
     */
    public println(i: boolean): void {
        this.print(i)
        this.println()
    }
    public println(i: byte): void {
        this.print(i)
        this.println()
    }
    public println(i: short): void {
        this.print(i)
        this.println()
    }
    public println(i: char): void {
        this.print(i)
        this.println()
    }
    public println(i: int): void {
        this.print(i)
        this.println()
    }
    public println(i: long): void {
        this.print(i)
        this.println()
    }
    public println(i: float): void {
        this.print(i)
        this.println()
    }
    public println(i: double): void {
        this.print(i)
        this.println()
    }
    public println(i: String): void {
        this.print(i)
        this.println()
    }
    public println(i: Any): void {
        this.print(i)
        this.println()
    }

    /**
     * @section Console logging API
     */

    /**
     * Implementations for log primitive types to stdout
     * @param {string | boolean | byte | short | char | int | long | Any} i - The value to print to stdout
     * @returns {void}
     * @public
     */
    // NOTE(ivan-tyulyandin): overload are added for performance reasons
    // To optimize `debug`, `warn`, `info` and `error` the same technique can be applied
    public log(i: boolean): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: byte): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: short): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: char): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: int): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: long): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: float): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: double): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(i: String): void {
        this.addToBuffer(this.indent + i + c'\n', LogLevel.LOG)
    }
    public log(): void {
        this.addToBuffer("\n", LogLevel.LOG)
    }

    /**
     * @section Variadics
     */

    private printRest(level: LogLevel, ...vals: Any[]) {
        let sb = new StringBuilder()

        if (vals.length != 0) {
            sb.append(this.indent)
            const dFst = vals[0]
            const hasFmt = (dFst instanceof String) && (dFst as String).indexOf(c'%') != -1
            if (hasFmt) {
                vals.shift()
                sb.append((new Formatter).format(dFst as String, vals))
            } else {
                sb.append(Console.AnyToPrint(dFst))
                for (let i = 1; i < vals.length; ++i) {
                    sb.append(" " + Console.AnyToPrint(vals[i]))
                }
            }
        }

        sb.append(c'\n')
        this.addToBuffer(sb.toString(), level)
    }

    /**
     * Prints log-level messages
     * If first argument is a string it is treated as a format string
     * @param {...Any[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public log(...vals: Any[]): void {
        this.printRest(LogLevel.LOG, ...vals)
    }

    /**
     * Prints debug-level messages
     * If first argument is a string it is treated as a format string
     * @param {...Any[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public debug(...vals: Any[]): void {
        this.printRest(LogLevel.DEBUG, ...vals)
    }

    /**
     * Prints info-level messages
     * If first argument is a string it is treated as a format string
     * @param {...Any[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public info(...vals: Any[]): void {
        this.printRest(LogLevel.INFO, ...vals)
    }

    /**
     * Prints warn-level messages
     * If first argument is a string it is treated as a format string
     * @param {...Any[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public warn(...vals: Any[]): void {
        this.printRest(LogLevel.WARN, ...vals)
    }

    /**
     * Prints error-level messages
     * If first argument is a string it is treated as a format string
     * @param {...Any[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public error(...vals: Any[]): void {
        this.printRest(LogLevel.ERROR, ...vals)
    }

    /**
     * Conditionally prints an error message if the assertion condition is false
     * @param {...Any[]} vals - Values to be logged if condition is false.
     * Condition is the first value in vals (if exist)
     * @returns {void}
     * @public
     */
    public assert(...vals: Any[]): void {
        if (vals.length == 0) {
            return
        }
        if (!vals[0]) {
            // replace false evaluated condition with no `vals` length changes
            let sb = new StringBuilder("Assertion failed")
            if (vals.length > 1) {
                sb.append(c':')
            }
            vals[0] = sb
            this.error(...vals)
        }
    }


    /**
     * @section Console.count* API
     */

    /**
    * Counts the number of times this method has been called with a specific label
    * Prints the current count to stdout
    * @param {string} [label='default'] - The label to identify this counter
    * @returns {void}
    */
    public count(label?: string): void {
        const key = label ?? 'default'
        const current = this.counters.get(key) ?? 0
        this.counters.set(key, current + 1)
        this.log(`${key}: ${current + 1}`)
    }

    /**
    * Resets the counter for a specific label
    * @param {string} [label='default'] - The label of the counter to reset
    * @returns {void}
    */
    public countReset(label?: string): void {
        const key = label ?? 'default'
        this.counters.delete(key)
    }


    /**
     * @section Console.dir* API
     */


    /**
    * Prints a formatted representation of an object to stdout
    * Filters out properties containing 'field#' in their keys
    * @param {Any} obj - The object to inspect
    * @returns {void}
    */
    public dir(obj?: Any): void {
        if (obj == null || obj == undefined) {
            return
        }

        // NOTE (templin.konstantin): Internal Fields Filtering
        //
        //  In the current language implementation, objects have an internal representation
        //  where some fields are duplicated with a "field#" prefix. These fields are part
        //  of the internal implementation and should not be displayed when outputting objects.
        //
        //  Example of internal representation:
        //  {
        //     "name": "test",
        //     "field#n": "test",    // Internal field
        //     "age": 25,
        //     "field#m": 25         // Internal field
        //  }
        const filterer = (key: String, value: Any): Any => {
            if (key.includes("field#")) {
                return undefined;
            }
            return value;
        }

        this.log(JSON.stringify(obj, filterer))
    }

    /**
    * Prints an XML representation of an object to stdout
    * Currently outputs the object as-is
    * @param {Any} obj - The object to display as XML
    * @returns {void}
    */
    public dirxml(...obj: Any[]): void {
        this.log(...obj)
    }


    /**
     * @section Console.group* API
     */


    /**
    * Starts a new logging group with optional label
    * Increases indentation level for subsequent log messages
    * @param {string} [objs] - Data to be printed without additional indent
    * @returns {void}
    */
    public group(...objs: Any[]): void {
        if (objs.length != 0) {
            this.log(objs)
        }
        this.indentationLevel++
    }

    /**
    * Ends the current logging group
    * Decreases indentation level for subsequent log messages
    * @returns {void}
    */
    public groupEnd(): void {
        if (this.indentationLevel > 0) {
            this.indentationLevel--
        }
    }

    /**
    * Alias for group() method
    * Creates a collapsed group in environments that support it
    * @param {string} [objs] - Data to be printed without additional indent
    * @returns {void}
    * @see group
    */
    public groupCollapsed(...objs: Any[]): void {
        this.group(...objs);
    }

    /**
    * @section Console methods for tabular data display.
    */

    /**
    * Displays an array of objects in tabular format
    * Converts the data to a DataFrame and renders it
    * @param {Any[]} data - Array of objects to display as a table
    * @returns {void}
    */
    public table(...data: Any[]): void {
        if (data.length == 0) {
            return
        }

        // Note(ivan-tyulyandin): unwrap from Array, workaround for 25264
        // unwrap from Array
        let dat = data[0]!

        const isSimple = (dat == undefined) || (dat == null)
                || (dat instanceof String) || (Object.keys(dat as Object).length == 0)
        if (isSimple) {
            this.log(dat)
            return
        }

        let df: DataFrame | undefined = undefined
        if (dat instanceof ArrayLike) {
            dat = dat as ArrayLike<Any>
            let d = new Array<Any>(dat.length)
            for (let i = 0; i < d.length; ++i) {
                d[i] = dat[i]
            }
            DataFrame.fromObjects(...d)
        } else {
            Console.parseObject(dat)
        }
    }

    /**
    * @section Console methods for tabular data display.
    */


    /**
    * Starts a timer with an optional label
    * Used to track execution time between time() and timeEnd() calls
    * @param {string} [label='default'] - Label to identify the timer
    * @returns {void}
    * @see timeEnd
    * @see timeLog
    */
    public time(label?: string): void {
        const key = label ?? 'default'
        Console.isNullCharacter(key);
        if (this.timers.has(key)) {
            this.warn("⚠️ Warning: Label 'default' already exists for console.time()")
            return
        }
        const start = Date.now()
        this.timers.set(key, start)
    }

    /**
    * Logs the current duration of a running timer without stopping it
    * Prints a warning if the specified timer doesn't exist
    * @param {string} [label='default'] - Label of the timer to check
    * @returns {void}
    * @see time
    */
    public timeLog(label?: string, ...arguments: Object[]): void {
        const key = label ?? 'default'
        Console.isNullCharacter(key);
        const len = arguments.length;
        let result = "";
        const startTime = this.timers.get(key)
        if (startTime === undefined) {
            this.warn(`Timer '${key}' does not exist`)
            return
        }
        for (let i = 0; i < len; i++) {
            result += " ";
            result += arguments[i].toString();
        }
        const end = Date.now()
        const duration = end - startTime
        this.log(`${key}: ${duration}ms ${result}`)
    }

    /**
    * Stops a timer and logs its final duration
    * Removes the timer and prints a warning if it doesn't exist
    * @param {string} [label='default'] - Label of the timer to stop
    * @returns {void}
    * @see time
    */
    public timeEnd(label?: string): void {
        if (label != undefined) {
            Console.isNullCharacter(label);
        }
        this.timeLog(label)
        this.timers.delete(label ?? 'default')
    }

    /**
    * Prints the current stack trace with an optional label
    * Skips the first stack frame (the trace call itself)
    * @param {Any[]} ...data - args to prints
    * @returns {void}
    */
    public trace(...data: Any[]): void {
        this.log("Trace:", data)
        const stackT = StackTrace.provisionStackTrace();
        for (let i = 1; i < stackT.length; ++i) {
            this.log(stackT[i]);
        }
    }

    private static isNullCharacter(key: string) {
        if (key.length == 0 || (key[0] == '\0')) {
            throw new Error("Parameter error. The input parameters are invalid, Timer or Counter name must be not null.")
        }
    }

    private static parseObject(...data: Any[]): void {
        const obj = data[0];
        if (obj instanceof object) {
            let hasPrimitive = false;
            const columnMap = new ColumnMap();
            const fieldVals = DataFrame.getAllInstanceFieldValues(obj)
            const keyLength = fieldVals.length
            let keyArray = new Array<string>();
            let valuesKeyArray:Array<string> = new Array<string>(keyLength).fill("");
            for (let i = 0; i < keyLength; i++) {
                const fieldVal: [string, Any, Class] = fieldVals[i]

                const fieldName = fieldVal[0]
                keyArray.push(fieldName);

                let value = fieldVal[1]
                const fieldType = fieldVal[2]
                if (value === undefined || DataFrame.isElementaryType(fieldType)) {
                    valuesKeyArray[i] = DataFrame.primitiveToString(value)
                    hasPrimitive = true;
                } else {
                    Console.parseObjectInner(value, keyLength, columnMap, i);
                }
            }
            let outputKeysArray = Console.getKeyArray(columnMap);
            let outputValuesArray = Console.getValueArray(columnMap, keyLength, keyArray)
            if (hasPrimitive) {
                Console.setPrimitive(columnMap, keyLength, valuesKeyArray, outputKeysArray, outputValuesArray);
            }
            let tableResult = Console.graphTable(outputKeysArray, outputValuesArray, keyLength);
            Console.printTable(tableResult);
        } else {
            DataFrame.fromObjects(obj);
        }
    }

    private static parseObjectInner(value: Any, keyLength: int, columnMap: ColumnMap, index: int): void {
        let initialMap = new Map<string, boolean>();
        let keys:Array<string> = new Array<string>();
        let innerLength: int = 0;
        let innerValue: Array<string> = new Array<string>();
        if (value instanceof Array) {
            const innerValueStr = DataFrame.toPrint(value, MAX_CONSOLE_PRINT_DEPTH)
            Console.parseString(innerValueStr, keys, innerValue);
            innerLength = innerValue.length;
        } else {
            Console.parseClassType(value, keys, innerValue);
            innerLength = innerValue.length;
        }
        Console.setColumnMap(innerLength, initialMap, columnMap, innerValue, keys, keyLength.toInt(), index);
    }

    private static setColumnMap(innerLength: int, initialMap: Map<string, boolean>, columnMap: ColumnMap, innerValue: Array<string>, 
    keys:Array<string>, keyLength: int, i: int): void {
        for (let j = 0; j < innerLength; j++) {
            let innerKey = keys[j];
            if (!initialMap.has(innerKey)) {
                let mapArrayLen: int = innerLength > keyLength ? innerLength : keyLength
                let mapArray = new Array<string>(mapArrayLen).fill("");
                mapArray[i] = innerValue[j];
                columnMap.columnValue.set(innerKey, mapArray);
                initialMap.set(innerKey, true);
            } else {
                let mapTmp: Array<string> = columnMap.columnValue.get(innerKey)!;
                mapTmp[i] = innerValue[j];
                columnMap.columnValue.set(innerKey, mapTmp);
            }
        }
    }

    private static parseString(innerValueStr:string, keys:Array<string>, innerValue:Array<string>):void {
        let stringLen = innerValueStr.length;
        let index = 0;
        for (let i = 0; i < stringLen; ++i) {
            if (innerValueStr[i] <= '9' && innerValueStr[i] >= '0') {
                let num = parseInt(innerValueStr[i]);
                innerValue.push(num.toString());
                keys.push(index.toString());
                index++;
            }
        }
    }

    private static parseClassType(value: Any, keys: Array<string>, innerValue: Array<string>): void {
        if (DataFrame.isElementaryValue(value)) {
            return
        }
        const fieldVals = DataFrame.getAllInstanceFieldValues(value as object)
        for (const fieldVal: [string, Any, Class] of fieldVals) {
            const fieldName = fieldVal[0]
            keys.push(fieldName)

            const value = fieldVal[1]
            if (DataFrame.isElementaryValue(value)) {
                innerValue.push(DataFrame.primitiveToString(value))
            } else {
                innerValue.push(DataFrame.toPrint(value, MAX_CONSOLE_PRINT_DEPTH))
            }
        }
    }

    private static getKeyArray(columnMap: ColumnMap): Array<string> {
        let mapLen = columnMap.columnValue.size;
        let keysArray: Array<string> = new Array<string>();
        for (const key of columnMap.columnValue.keys()) {
            keysArray.push(key);
        }
        let keyLength = mapLen + 1;
        let outputKeysArray:Array<string> = new Array<string>(keyLength);
        outputKeysArray[0] = "(index)";
        for (let i = 0; i < mapLen; i++) {
            outputKeysArray[i + 1] = keysArray[i];
        }
        return outputKeysArray;
    }

    private static getValueArray(columnMap: ColumnMap, length: int, keyArray: Array<string>): Array<string> {
        let mapLen:int = columnMap.columnValue.size;
        let mapKeys: Array<string> = new Array<string>();
        for (const key of columnMap.columnValue.keys()) {
            mapKeys.push(key);
        }
        let keyLength: int = mapLen + 1;
        let valueLength: int = keyLength * length;
        let outputValuesArray:Array<string> = new Array<string>(valueLength);
        let valueIndex = 0;
        for (let i = 0; i < length; i++) {
            let key = keyArray[i];
            outputValuesArray[valueIndex++] = key;
        }
        for (let i = 0; i < mapLen; i++) {
            let innerKey = mapKeys[i];
            let valueArray:Array<string> = columnMap.columnValue.get(innerKey)!;
            for (let j = 0; j < length; j++) {
                if (valueArray[j] != null && valueArray[j] != undefined) {
                    outputValuesArray[valueIndex++] = valueArray[j] as string;
                }
            }
        }
        return outputValuesArray;
    }

    private static setPrimitive(columnMap: ColumnMap, length: int, valuesKeyArray: Array<string>, outputKeysArray: Array<string>, outputValuesArray: Array<string>): void {
        const mapKeys = new Array<string>();
        for (const key of columnMap.columnValue.keys()) {
            mapKeys.push(key);
        }
        outputKeysArray.push("Values");
        for (let i = 0; i < length; i++) {
            outputValuesArray.push(valuesKeyArray[i]);
        }
    }

    private static graphTable(head: Array<string>, columns: Array<string>, length: int): Array<string> {
        let columnLen = head.length;
        let columnWidths: Array<number> = new Array<number>(columnLen);
        let rowDivider: Array<string> = new Array<string>(columnLen);
        let tableResult: Array<string> = new Array<string>();
        for (let i = 0; i < columnLen; i++) {
            let stringLen = head[i].length;
            columnWidths[i] = stringLen;
        }
        for (let i = 0; i < columnLen; i++) {
            for (let j = 0; j < length; j++) {
                let element = columns[i * length + j];
                let stringLen = Console.getStringLen(element);
                columnWidths[i] = columnWidths[i] > stringLen ? columnWidths[i] : stringLen;
            }
            let middle: string = Console.tableChars.get("middleMiddle")!;
            rowDivider[i] = Console.stringRepeat(columnWidths[i] + 2, middle);
        }
        let indexRow1: string = Console.tableChars.get("topLeft")! + Console.arrayJoin(rowDivider, Console.tableChars.get("topMiddle")!) + Console.tableChars.get("topRight")!;
        let indexRow2: string = Console.renderHead(head, columnWidths);
        let indexRow3 = Console.tableChars.get("leftMiddle")! + Console.arrayJoin(rowDivider, Console.tableChars.get("rowMiddle")!) + Console.tableChars.get("rightMiddle")!;
        tableResult.push(indexRow1);
        tableResult.push(indexRow2);
        tableResult.push(indexRow3);
        let rows = Console.printRows(columns, columnWidths, length);
        for (let i = 0; i < rows.length; i++) {
            tableResult.push(rows[i]);
        }
        let endRow = Console.tableChars.get("bottomLeft")! + Console.arrayJoin(rowDivider, Console.tableChars.get("bottomMiddle")!) + Console.tableChars.get("bottomRight")!;
        tableResult.push(endRow);
        return tableResult;
    }

    private static getStringLen(str: string):number {
        let length = 0;
        for (let i =0; i < str.length; i++) {
            if (str[i] != "" && str[i] != undefined && str[i] != null) {
                length++;
            }
        }
        return length;
    }

    private static stringRepeat(number: number, tableChars: string): string {
        let divider: string = "";
        let length = number;
        for (let i = 0; i < length; i++) {
            divider += tableChars;
        }
        return divider;
    }

    private static arrayJoin(rowDivider: Array<string>, tableChars: string): string {
        let size = rowDivider.length;
        if (size == 0) {
            return "no rowDivider";
        }
        let result: string = rowDivider[0];
        for (let i = 1; i < size; i++) {
            result += tableChars;
            result += rowDivider[i];
        }
        return result;
    }

    private static renderHead(head: Array<string>, columnWidths: Array<number>) {
        let result = Console.tableChars.get("left")!;
        let length = columnWidths.length;
        for (let i = 0; i < length; i++) {
            let elemStr = head[i];
            let stringLen = elemStr.length;
            let left = (columnWidths[i] - stringLen) / 2;
            let right = columnWidths[i] - stringLen - left;
            result += Console.stringRepeat(left, " ") + elemStr + Console.stringRepeat(right, " ");
            if (i != length - 1) {
                result += Console.tableChars.get("middle")!;
            }
        }
        result += Console.tableChars.get("right")!;
        return result;
    }

    private static printRows(Rows: Array<string>, columnWidths: Array<number>, indexNum: int): Array<string> {
        let rows:Array<string> = new Array<string>();
        let length = columnWidths.length;
        for (let i = 0; i < indexNum; i++) {
            let result:string = Console.tableChars.get("left")!;
            for (let j = 0; j < length; j++) {
                let stringVal = Rows[j * indexNum + i];
                let stringLen = Console.getStringLen(stringVal);
                if (stringLen > 0) {
                    let left:int = ((columnWidths[j] - stringLen) / 2).toInt();
                    let right:int = (columnWidths[j] - stringLen - left).toInt();
                    result += Console.stringRepeat(left, " ") + stringVal + Console.stringRepeat(right, " ");
                } else {
                    result += Console.stringRepeat(columnWidths[j], " ");
                }
                if (j != length - 1) {
                    result += Console.tableChars.get("middle")!;
                }
            }
            result += Console.tableChars.get("right")!;
            rows.push(result);
        }
        return rows;
    }

    private static printTable(tableResult: Array<string>): void {
        let len = tableResult.length;
        for (let i = 0; i < len; i++) {
            console.log(tableResult[i]);
        }
    }
}

// initialized in _initializerBlock_.ets
export const console: Console;

/**
 * @section Dataframe helpers class for console.table API
 */

/** Column name type alias */
type ColumnName = string;

/** Column value type alias */
type ColumnValue = string;

/** Array of column values type alias */
type ColumnValues = Array<ColumnValue>;

/** Column width type alias */
type ColumnWidth = int;

/**
 * DataFrame class for handling and displaying tabular data
 * Provides functionality to create, manipulate, and render data in a table format
 */
class DataFrame {
    /** Map storing column names and their corresponding values */
    private tableModel: Map<ColumnName, ColumnValues>

    /** Map storing column names and their display widths */
    private columnWidths: Map<ColumnName, ColumnWidth>

    /** Number of rows in the DataFrame */
    private rowsCount: int

    /** Header text for the index column */
    private static readonly headerIndexRow: string = '│ (index) │'

    private static readonly GETTER_METHOD_PREFIX = "%%get-"

    /** Width of the index column based on header */
    private readonly _indexColumnWidth: int = Double.toInt(DataFrame.headerIndexRow.length);

    /**
     * Creates a new DataFrame with specified columns
     * @param {Array<ColumnName>} columns - Array of column names
     * @ensures All columns are initialized with empty value arrays
     * @ensures Column widths are initialized to the length of column names
     */
    constructor(columns: Array<ColumnName>) {
        this.tableModel = new Map<ColumnName, ColumnValues>();
        this.columnWidths = new Map<ColumnName, ColumnWidth>();
        for (let column of columns) {
            this.tableModel.set(column, new Array<string>());
            this.columnWidths.set(column, column.length);
        }
        this.rowsCount = 0;
    }

    /**
     * Inserts a new row of data into the DataFrame
     * @param {Map<ColumnName, ColumnValue>} row - Map of column names to values
     * @ensures Column count remains unchanged
     * @ensures Column widths are updated if new values are longer
     */
    public insertRow(row: Map<ColumnName, ColumnValue>) {
        for (let key of this.tableModel.keys()) {
            const value = row.get(key) ?? "-";
            this.tableModel.get(key)!.push(value);
            const currentWidth = this.columnWidths.get(key)!;
            const valueLength = value.length
            this.columnWidths.set(key,
                                currentWidth < valueLength
                                ? valueLength
                                : currentWidth);
        }
        this.rowsCount++;
    }

    /**
     * Builds a border row for the table
     * @param {string} char - String to use for the border line
     * @param {string} commonConnector - Character for column separators
     * @param {string} connectorLeft - Character for left border
     * @param {string} connectorRight - Character for right border
     * @returns {string} Formatted border row
     * @private
     */
    private buildBorderRow(
        ch: string,
        commonConnector: string,
        connectorLeft: string,
        connectorRight: string,
    ): string {
        const connLength: int =
            this._indexColumnWidth -
            connectorLeft.length.toInt() -
            connectorRight.length.toInt();
        const conn: string = ch.repeat(connLength);
        let border: string = `${connectorLeft}${conn}${commonConnector}`;
        const keys = Array.from(this.tableModel.keys());
        for (let i = 0; i < keys.length; i++) {
            const colWidth = this.columnWidths.get(keys[i])!;
            if (i != keys.length - 1) {
                border += ch.repeat(colWidth + 2) + commonConnector;
            } else {
                border += ch.repeat(colWidth + 2);
            }
        }
        return `${border}${connectorRight}`;
    }

    /**
     * Builds the header row containing column names
     * @returns {string} Formatted header row
     * @private
     */
    private buildHeaderRow(): string {
        let headerRow: string = DataFrame.headerIndexRow;
        for (const col of this.tableModel.keys()) {
            const colWidth = this.columnWidths.get(col)!;
            headerRow += ` ${col.padEnd(colWidth, ' ')} │`;
        }
        return headerRow;
    }

    /**
     * Builds a data row for the specified index
     * @param {number} rowIndex - Index of the row to build
     * @returns {string} Formatted data row
     * @private
     */
    private buildDataRow(rowIndex: int): string {
        const paddedIndex = `${rowIndex}`.padEnd(this._indexColumnWidth - 4);
        let row: string = `│ ${paddedIndex} │`;
        for (const col of this.tableModel.keys()) {
            const colWidth = this.columnWidths.get(col)!;
            const colContent = this.tableModel.get(col)![rowIndex];
            row += ` ${colContent.padEnd(colWidth)} │`;
        }
        return row;
    }

    /**
     * Renders the DataFrame as an array of strings
     * @returns {Array<String>} Array of formatted table rows
     */
    public render(): Array<String> {
        if (this.tableModel.size == 0) {
            return new Array<string>();
        }

        // row1
        const topBorder = this.buildBorderRow('─', '┬', '┌', '┐');
        // row2
        const separator = this.buildBorderRow('─', '┼', '├', '┤');
        // row3
        const bottomBorder = this.buildBorderRow('─', '┴', '└', '┘');
        // header has 2 rows and footer - 1

        const output = new Array<string>(4 + this.rowsCount);

        // |output| = 0
        output[0] = topBorder
        // |output| = 1
        output[1] = this.buildHeaderRow()
        // |output| = 2
        output[2] = separator
        // |output| = 3

        for (let i = 0; i < this.rowsCount; i++) {
            output[3 + i] = this.buildDataRow(i);
        }
        // |output| = 3 + rowsCount
        output[3 + this.rowsCount] = bottomBorder;
        // |output| = 4 + rowsCount
        return output;
    }

    /**
     * Displays the DataFrame to the console
     * @returns {void}
     */
    public display(): void {
        const rows = this.render();
        for (const row of rows) {
            console.log(row);
        }
    }

    public static isElementaryValue(val: Any): boolean {
        if (val === undefined || val === null) {
            return true
        }
        const valType = Class.of(val as object)
        return Types.isPrimitiveType(valType) || Types.isCharType(valType)
    }

    public static isElementaryType(type: Class): boolean {
        return Types.isPrimitiveType(type) || Types.isCharType(type) || type === Class.of(null)
    }

    /**
    * Converts an object into a map of string key-value pairs for table representation.
    * Handles various types including primitives, collections, arrays, classes, and functions.
    *
    * @param obj - The object to be converted into table info
    * @returns A Map containing string representations of object properties
    */
    private static getObjectInfoForTable(obj: Any): Map<string, string> {
        const mapping = new Map<string, string>();
        // Handle primitive types
        if (obj === null) {
            mapping.set("Values", "null");
        } else if (obj === undefined) {
            mapping.set("Values", "undefined");
        } else if (obj instanceof String) {
            mapping.set("Values", `'${obj}'`);
        }
        // Handle complex types
        else if (obj instanceof Map) {
            const entries = obj.entries();
            for (let entry of entries) {
                mapping.set(`${entry[0]}`, `${entry[1]}`);
            }
        } else if (obj instanceof Function) {
            const fnName = obj.name
            mapping.set("Values", (fnName == "") ? "[Function (anonymous)]" : `[Function: ${fnName}]`)
        } else if (obj instanceof Set) {
            const values = obj.values();
            let i = 0;
            for (let value of values) {
                mapping.set(`{i}?`, `${value}`);
                i++;
            }
        } else {
            if (DataFrame.isElementaryValue(obj)) {
                mapping.set("Values", `${obj}`);
            } else if (obj instanceof object) {
                const objType = Class.of(obj as object)
                if (objType.isFixedArray()) {
                    DataFrame.addArrayInfoToTableMapping(obj, mapping)
                } else {
                    const fieldVals = DataFrame.getAllInstanceFieldValues(obj)
                    for (let i = 0; i < fieldVals.length; i++) {
                        const fieldVal: [string, Any, Class] = fieldVals[i]
                        const fieldName = (obj instanceof Tuple) ? `${i}` : fieldVal[0]
                        mapping.set(fieldName, DataFrame.toPrint(fieldVal[1], MAX_CONSOLE_PRINT_DEPTH));
                    }
                }
            } else {
                mapping.set("Values", JSON.stringify(obj))
            }
        }

        return mapping;
    }

    private static addArrayInfoToTableMapping(arrObj: object, mapping: Map<string, string>): void {
        if (arrObj instanceof FixedArray<boolean>) {
            forEach(arrObj, (v: boolean, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<byte>) {
            forEach(arrObj, (v: byte, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<char>) {
            forEach(arrObj, (v: char, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<short>) {
            forEach(arrObj, (v: short, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<int>) {
            forEach(arrObj, (v: int, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<long>) {
            forEach(arrObj, (v: long, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<float>) {
            forEach(arrObj, (v: float, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else if (arrObj instanceof FixedArray<double>) {
            forEach(arrObj, (v: double, i: int) => {
                mapping.set(`${i}`, DataFrame.toPrint(v, MAX_CONSOLE_PRINT_DEPTH));
            })
        } else {
            const arr = arrObj as FixedArray<object>
            for (let i = 0; i < arr.length; i++) {
                mapping.set(`${i}`, DataFrame.toPrint(arr[i], MAX_CONSOLE_PRINT_DEPTH));
            }
        }
    }

    public static getAllInstanceFieldValues(obj: object): Array<[string, Any, Class]> {
        const fieldVals = new Array<[string, Any, Class]>()

        if (!reflect.isLiteralInitializedInterface(obj)) {
            let curType = Class.of(obj)
            const typeHierarchy = new Array<Class>()
            while (curType.getSuper() !== undefined) {
                typeHierarchy.push(curType)
                curType = curType.getSuper()!
            }

            for (let i = typeHierarchy.length - 1; i >= 0; i--) {
                const curType = typeHierarchy[i]
                const fields = curType.getInstanceFields()
                for (const field of fields) {
                    const fieldVal: [string, Any, Class] = [field.getName(), field.getValue(obj), field.getType()]
                    fieldVals.push(fieldVal)
                }
            }
        } else {
            const prefixLength = DataFrame.GETTER_METHOD_PREFIX.getLength()
            const objType = Class.of(obj)
            const objTypeMethods = objType.getInstanceMethods()
            for (const method of objTypeMethods) {
                let methodName = method.getName()
                if (methodName.startsWith(DataFrame.GETTER_METHOD_PREFIX, 0)) {
                    const value = method.invoke(obj)
                    fieldVals.push([methodName.substring(prefixLength), value, method.getReturnType()])
                }
            }
        }

        return fieldVals
    }

    public static toPrint(val: Any, depth: int): string {
        if (val === undefined) {
            return "undefined"
        } else if (val === null) {
            return "null"
        } else if (val instanceof Date) {
            return new StringBuilder("Date: ").append("\"").append(val.toISOString()).append("\"").toString()
        } else if (val instanceof ArrayBuffer) {
            if (depth == 0) {
                return "std.core.ArrayBuffer {...}"
            }

            const res = new StringBuilder("std.core.ArrayBuffer {")
            const byteLength = val.byteLength
            res.append("data: [")
            if (byteLength > 0) {
                for (let i = 0; i < byteLength - 1; i++) {
                    res.append(val.at(i)).append(", ")
                }
                res.append(val.at(byteLength - 1))
            }
            res.append("], byteLength: ").append(byteLength).append("}")
            return res.toString()
        } else if (val instanceof Function) {
            const fnName = val.name
            return (fnName == "") ? "[Function (anonymous)]" : `[Function: ${fnName}]`
        }

        const objVal = val as object
        const valType = Class.of(objVal)
        if (valType.isFixedArray()) {
            return DataFrame.fixedArrayToPrint(objVal, depth - 1)
        }

        if (depth == 0) {
            return valType.getName() + " {...}"
        }

        const valStr = objVal.toString()
        const objStr = new Object().toString()
        // val overrides toString() - so using it
        if (valStr != objStr) {
            return valStr
        }

        const fieldVals = DataFrame.getAllInstanceFieldValues(objVal)
        if (fieldVals.length == 0) {
            return valType.getName() + " {}"
        }

        const res = new StringBuilder(valType.getName())
        res.append(" {")

        const lastFieldIdx = fieldVals.length - 1
        for (let fieldIdx = 0; fieldIdx < lastFieldIdx; fieldIdx++) {
            const fieldVal: [string, Any, Class] = fieldVals[fieldIdx]
            res.append(fieldVal[0]).append(": ").append(DataFrame.toPrint(fieldVal[1], depth - 1))
            res.append(", ")
        }

        const lastFieldVal = fieldVals[lastFieldIdx]
        res.append(lastFieldVal[0]).append(": ").append(DataFrame.toPrint(lastFieldVal[1], depth - 1))
        res.append("}")
        return res.toString()
    }

    private static fixedArrayToPrint(arrObj: object, depth: int): string {
        if (depth == 0) {
            return "[...]"
        }

        if (arrObj instanceof FixedArray<Any>) {
            const arrLen = arrObj.length
            if (arrLen == 0) {
                return "[]"
            }

            const res = new StringBuilder("[")
            for (let i = 0; i < arrLen - 1; i++) {
                const elem = arrObj[i]
                res.append(DataFrame.toPrint(elem, depth - 1)).append(", ")
            }
            res.append(DataFrame.toPrint(arrObj[arrLen - 1], depth - 1)).append("]")
            return res.toString()
        }

        const res = new StringBuilder("[")
        if (arrObj instanceof FixedArray<boolean>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<byte>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<char>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<short>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<int>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<long>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<float>) {
            res.append(join(arrObj, ", "))
        } else if (arrObj instanceof FixedArray<double>) {
            res.append(join(arrObj, ", "))
        } else {
            const arrType = Class.of(arrObj)
            throw new AssertionError(`unexpected FixedArray type: ${arrType.getName()}`)
        }
        res.append("]")
        return res.toString()
    }

    /**
     * Creates a DataFrame from an array of objects
     * @param {Any[]} data - Array of objects to convert
     * @static
     */
    public static fromObjects(...data: Any[]): void {
        const columns = new Set<string>();
        const preparedInfo = new Array<Map<string, string>>(data.length);
        for (let i = 0; i < data.length; ++i){
            preparedInfo[i] = DataFrame.getObjectInfoForTable(data[i]);
            for (let key of preparedInfo[i].keys())
            {
                columns.add(key);
            }
        }
        const df = new DataFrame(Array.from(columns));
        for (let i = 0; i < data.length; ++i) {
            df.insertRow(preparedInfo[i]);
        }
        df.display();
    }

    public static primitiveToString(val: Any): string {
        if (!DataFrame.isElementaryValue(val)) {
            throw new IllegalArgumentError("illegal argument")
        }

        if (val === undefined) {
            return "undefined"
        } else if (val === null) {
            return "null"
        } else if (val instanceof object) {
            return val.toString()
        } else {
            throw new IllegalArgumentError("unsupported type: " + Class.ofAny(val)?.getName())
        }
    }
}
