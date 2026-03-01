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

type RegExpGroupsContainer = Record<String, String | undefined>

/**
 * Regular expression result descriptor
 */
export class RegExpResultArray extends Array<String | undefined> {
    /** true if match present */
    readonly isCorrect: boolean

    private resultRaw_: String = ""
    /** The 0-based index of the match in the string */
    protected index_: int = 0
    /** The original string that was matched against */
    protected input_: String = ""

    private endIndex: int = 0
    private wasProcessed: boolean = false

    // NOTE(shumilov-petr): Make nullable when #14813 will be fixed
    protected indices_: Array<Array<int>> = []

    /** named groups names and indeces */
    protected groupsRaw_: String = ""

    /** named groups names and indeces */
    public groups: RegExpGroupsContainer | undefined = undefined

    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(0)
        this.isCorrect = true
        this.index_ = index
        this.input_ = input
        this.indices_ = indices

        this.extendTo(result.length, undefined as (String | undefined))
        for (let i: int = 0; i < result.length; i++) {
            this.$_set(i, result.$_get(i))
        }
    }

    protected constructor() {
        super(0)
        this.isCorrect = false
    }

    public get result(): Array<String | undefined> {
        return this
    }

    public postExecProcessing(res: RegExpResultArray, input: String, index: int, hasIndices: boolean): void {
        // NOTE(ekaterinazaytseva): temp solution - will be fixed in #20259
        this.updateGroups()
        this.updateIndicesAndResult(hasIndices)
        this.wasProcessed = true
    }

    /**
     * Returns the first match if "0" is given and the first match exists
     *
     * @param index
     *
     * @returns the first match as string or null
     */
    public $_get(index: String): String {
        if (index == "0") {
            const v = this.$_get(0)
            if (v !== undefined) return v
            throw new Error("result[0] is undefined")
        }
        throw new Error("unsupported field name")
    }

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public $_get(index: int): String | undefined {
        return super.$_get(index)
    }

    public $_set(index: int, val: String | undefined): void {
        super.$_set(index, val)
    }

    public get indices(): Array<Array<int>> {
        return this.indices_
    }

    /**
     * Returns a string representing the match result
     *
     * @returns a string representing the match result
     */
    public toString(): String {
        if (this.length > 0) {
            return super.toString();
        }
        throw new Error("result is empty")
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected updateGroups(): void {
        throw new Error("not overridden")
    }

    // NOTE(mgroshev): remove it as part of #20259
    public getWasProcessed(): boolean {
        return this.wasProcessed
    }

    private updateIndicesAndResult(hasIndices: boolean): void {
        const data = this.resultRaw_.split(',')
        const n: int = (data.length / 2)

        if (n > this.length) {
            this.extendTo(n, undefined as (String | undefined))
        } else if (n < this.length) {
            this.shrinkTo(n)
        }

        if (hasIndices) {
            this.indices_ = new Array<Array<int>>(n)
            for (let i = 0; i < data.length; i += 2) {
                const index: int = Number.parseInt(data[i]).toInt()
                const endIndex: int = Number.parseInt(data[i + 1]).toInt()
                const pos: int = (i / 2);
                if (index == -1 || endIndex == -1) {
                    this.indices_[pos] = [0, 0]
                } else {
                    this.indices_[pos] = [index, endIndex]
                    const str = this.input_.substring(index, endIndex)
                    this.$_set(pos, str)
                }
            }
        } else {
            this.indices_ = []
            for (let i = 0; i < data.length; i += 2) {
                const index = Number.parseInt(data[i]).toInt()
                const endIndex = Number.parseInt(data[i + 1]).toInt()
                const pos: int = (i / 2);
                if (index != -1 && endIndex != -1) {
                    const str = this.input_.substring(index, endIndex)
                    this.$_set(pos, str)
                }
            }
        }
    }
}

export class RegExpMatchArray extends RegExpResultArray {
    private hasIndex_: boolean = true
    private hasInput_: boolean = true

    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(index, input, result, indices)
    }

    constructor(index: int, input: String, result: Array<String | undefined>) {
        this(index, input, result, [])
    }

    protected constructor() {
        super()
    }

    public get index(): int | undefined {
        if (this.hasIndex_) {
            return this.index_.toInt()
        }
        return undefined
    }

    public get input(): String | undefined {
        if (this.hasInput_) {
            return this.input_
        }
        return undefined
    }

    public set index(val: int | undefined) {
        if (val == undefined) {
            this.hasIndex_ = false
            return
        }
        this.hasIndex_ = true
        this.index_ = val
    }

    public set input(val: String | undefined) {
        if (val == undefined) {
            this.hasInput_ = false
            return
        }
        this.hasInput_ = true
        this.input_ = val
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected override updateGroups(): void {
        if (this.groupsRaw_ == "") {
            this.groups = undefined
            return
        }
        let result = new RegExpGroupsContainer()

        if (!this.hasInput_) {
            return
        }

        for (let groupData of this.groupsRaw_.split(";")) {
            if (groupData.length == 0) {
                continue
            }
            let unitData = groupData.split(",")
            if (unitData.length != 3) {
                throw new Error("internal error: invalid group data")
            }
            let index = Double.parseInt(unitData[1]).toInt()
            let endIndex = Double.parseInt(unitData[2]).toInt()
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index > endIndex) {
                    throw new Error("internal error: incorrect substring indexes")
                }
                result.set(unitData[0], this.input!.substring(index, endIndex))
            } else {
                result.set(unitData[0], undefined)
            }
        }
        if (result.size == 0) {
            this.groups = undefined
            return
        }
        this.groups = result
    }
}

export class RegExpExecArray extends RegExpResultArray {
    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(index, input, result, indices)
    }

    constructor(index: int, input: String, result: Array<String | undefined>) {
        this(index, input, result, [])
    }

    protected constructor() {
        super()
    }

    public get index(): int {
        return this.index_.toInt()
    }

    public get input(): String {
        return this.input_
    }

    public set index(val: int) {
        this.index_ = val
    }

    public set input(val: String) {
        this.input_ = val
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected override updateGroups(): void {
        if (this.groupsRaw_ == "") {
            this.groups = undefined
            return
        }
        let result = new RegExpGroupsContainer()

        for (let groupData of this.groupsRaw_.split(";")) {
            if (groupData.length == 0) {
                continue
            }
            let unitData = groupData.split(",")
            if (unitData.length != 3) {
                throw new Error("internal error: invalid group data")
            }
            let index = Double.parseInt(unitData[1]).toInt()
            let endIndex = Double.parseInt(unitData[2]).toInt()
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index > endIndex) {
                    throw new Error("internal error: incorrect substring indexes index = " + index + " endIndex = " + endIndex + 
                    " length = " + this.input_.length)
                }
                result.set(unitData[0], this.input.substring(index, endIndex))
            } else {
                result.set(unitData[0], undefined)
            }
        }
        if (result.size == 0) {
            this.groups = undefined
            return
        }
        this.groups = result
    }
}

class GlobalRegExpStringIterator implements IterableIterator<RegExpMatchArray> {
    private regexp: RegExp
    private string: String
    private returned: boolean

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
        this.returned = false
    }

    override next(): IteratorResult<RegExpMatchArray> {
        if (this.returned) {
            return new IteratorResult<RegExpMatchArray>();
        }
        let match = this.regexp.matchInternal(this.string)
        if (match == null) {
            this.returned = true;
            return new IteratorResult<RegExpMatchArray>()
        }
        let matchStr = match.$_get(0)
        if (matchStr == "") {
            this.regexp.lastIndex = RegExp.advanceStringIndex(this.string, this.regexp.lastIndex, this.regexp.unicode)
        }
        return new IteratorResult<RegExpMatchArray>(match)
    }

    override $_iterator(): IterableIterator<RegExpMatchArray> {
        return this
    }
}

class RegExpStringIterator implements IterableIterator<RegExpMatchArray> {
    private regexp: RegExp
    private string: String
    private returned: boolean

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
        this.returned = false
    }

    override next(): IteratorResult<RegExpMatchArray> {
        let match = this.regexp.matchInternal(this.string)
        if (match == null || this.returned) {
            return new IteratorResult<RegExpMatchArray>()
        }
        this.returned = true
        return new IteratorResult<RegExpMatchArray>(match)
    }

    override $_iterator(): IterableIterator<RegExpMatchArray> {
        return this
    }
}

/**
 * Regular expression
 */
export class RegExp extends Object {
    private pattern_: String = ""
    public lastIndex: int = 0
    private source_: String = ""
    private flags_: String = ""

    /** Flags */
    get flags(): String {
        return this.flags_
    }

    /**
     * Has the value true if the regular expression should be tested against
     * all possible matches in a string
     */
    get global(): boolean {
        return this.flags_.contains("g", 0)
    }

    /**
     * Has the value true if the dot special character (.) should additionally match
     * the line terminator characters in a string
     */
    get dotAll(): boolean {
        return this.flags_.contains("s", 0)
    }

    /**
     * Has the value true if the result of a regular expression match should contain
     * the start and end indices of the substrings of each capture group
     */
    get hasIndices(): boolean {
        return this.flags_.contains("d", 0)
    }

    /**
     * Has the value true if case should be ignored while attempting a match in a string
     */
    get ignoreCase(): boolean {
        return this.flags_.contains("i", 0)
    }

    /**
     * Has the value true if a multiline input string should be treated as multiple lines
     */
    get multiline(): boolean {
        return this.flags_.contains("m", 0)
    }

    /**
     * Has the value true if the regex attempts to match the target string only
     * from the index indicated by the lastIndex property
     */
    get sticky(): boolean {
        return this.flags_.contains("y", 0)
    }

    /**
     * Has the value true if 'u' flag is used
     */
    get unicode(): boolean {
        return this.flags_.contains("u", 0)
    }

    /**
     * Has the value true if 'v' flag is used
     */
    get unicodeSets(): boolean {
        return this.flags_.contains("v", 0)
    }

    /**
     * Returns a string containing the source text of this regular expression
     */
    get source(): String {
        return this.source_
    }

    /**
     * Constructs a new RegExp using pattern and flags
     *
     * @param pattern description of a pattern
     *
     * @param flags description of flags to be used
     */
    public constructor(pattern: String, flags?: String) {
        this.init(pattern, flags)
    }

    /**
     * Constructs a new RegExp by other RegExp. It uses other RegExp's flags if flags aren't provided
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    public constructor(regexp: RegExp, flags?: String) {
        this.init(regexp.pattern_, flags === undefined ? regexp.flags : flags!)
    }

    /**
     * Constructs a new RegExp by RegExp or string
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    public constructor(regexp: RegExp | String, flags?: String) {
        if (regexp instanceof String) {
            this.init(regexp as String, flags)
        } else {
            this.init(regexp as RegExp, flags)
        }
    }

    /**
     * Compiles a regular expression
     */
    public native compile(): RegExp;

    /**
     * Parse a regular expression
     */
    private native static parse(pattern: String, flags: String): void

    /**
     * Recompiles a regular expression with new source and flags after the RegExp object has already been created
     *
     * @param pattern the text of the regular expression
     *
     * @param flags any combination of flag values
     */
    public compile(pattern: String, flags?: String): RegExp {
        this.pattern_ = pattern
        this.flags_ = flags === undefined ? "" : flags!
        this.source_ = RegExp.escapePattern(pattern)
        return this.compile()
    }

    private native execImpl(pattern: String, flags: String, str: String, patternLength: int, strLength: int, lastIndex: int, hasSlashU: boolean): RegExpExecArray;

    private native matchImpl(pattern: String, flags: String, str: String, patternLength: int, strLength: int, lastIndex: int, hasSlashU: boolean): RegExpMatchArray;

    public exec(str: String, index: int): RegExpExecArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.execImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            if (!res.getWasProcessed() && res.isCorrect) {
                res.postExecProcessing(res, str, res.index, this.hasIndices)
            }
            return res
        } else {
            return null
        }
    }

    /**
     * Executes a search for a match in a specified string and returns a result array
     *
     * @param str the string against which to match the regular expression
     *
     * @returns RegExp result
     *
     * @see RegExpExecArray
     */
    public exec(str: String): RegExpExecArray | null {
        return this.exec(str, this.lastIndex)
    }

    private matchInternal(str: String, index: int): RegExpMatchArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.matchImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            if (!res.getWasProcessed() && res.isCorrect) {
                res.postExecProcessing(res, str, res.index!, this.hasIndices)
            }
            return res
        } else {
            return null
        }
    }

    public matchInternal(str: String): RegExpMatchArray | null {
        return this.matchInternal(str, this.lastIndex)
    }

    public search(str: String): int {
        let previousLastIndex: int = this.lastIndex
        this.lastIndex = 0
        let result = this.exec(str)
        this.lastIndex = previousLastIndex
        if (result == null) {
            return -1
        } else {
            return result.index.toInt()
        }
    }

    public replace(str: String, replaceValue: String): String {
        if (this.global) {
            this.lastIndex = 0
        }
        let results = new Array<RegExpExecArray>()
        let done = false
        while (!done) {
            let result = this.exec(str)
            if (result == null) {
                break;
            }
            results.push(result)
            if (!this.global) {
                break;
            }
            let matchStr = result.$_get(0)
            if (matchStr == "") {
                this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex.toInt(), this.unicode)
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = ""
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = max(min(result.index, lengthS), 0).toInt();
            let captures = new containers.UndefinableStringArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                captures.pushBack(capN)
            }
            let replacement = String.getSubstitution(matched, str, position as int, captures.toArray(), result.groups, replaceValue)
            if (position >= nextSourcePosition) {
                accumulatedResult += str.substring(nextSourcePosition, position) + replacement
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult
        }
        return accumulatedResult + str.substring(nextSourcePosition)
    }

    public replace(str: String, replacer: (substr: String, args: Object[]) => String): String {
        if (this.global) {
            this.lastIndex = 0
        }
        let results = new Array<RegExpExecArray>()
        let done = false
        while (!done) {
            let result = this.exec(str)
            if (result == null) {
                break;
            }
            results.push(result)
            if (!this.global) {
                break;
            }
            let matchStr = result.$_get(0)
            if (matchStr == "") {
                this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex.toInt(), this.unicode)
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = ""
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = (max(min(result.index, lengthS), 0)).toInt();
            let args = new containers.UndefinableObjectArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                args.pushBack(capN)
            }
            let namedCaptures: Object | undefined = result.groups;
            args.pushBack(position.toDouble());
            args.pushBack(str)
            if (namedCaptures != undefined) {
                args.pushBack(namedCaptures)
            }
            let replacement = replacer(matched, args.toArray());
            if (position >= nextSourcePosition) {
                accumulatedResult += str.substring(nextSourcePosition, position) + replacement
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult
        }
        return accumulatedResult + str.substring(nextSourcePosition)
    }

    public split(str: String, limit: Int | undefined): String[] {
        let unicodeMatching = this.unicode
        let newFlags = this.sticky ? this.flags : this.flags + "y";
        let splitter = new RegExp(this.pattern_, newFlags)
        let lim: long;
        if (limit == undefined) {
            lim = (1 << 32) -1
        } else if (limit == 0) {
            return new String[0]
        } else {
            lim = limit!.toLong()
        }
        let size = str.getLength()
        if (size == 0) {
            let z = splitter.exec(str)
            if (z != null) {
                return new String[0]
            }
            return [str]
        }
        let splittedStrings = new containers.UndefinableStringArray()
        let lastStart = 0
        for (let lastEnd: int = 0; lastEnd < size;) {
            splitter.lastIndex = lastEnd
            let z = splitter.exec(str)
            if (z == null) {
                lastEnd = RegExp.advanceStringIndex(str, lastEnd, unicodeMatching)
            } else {
                let separatorRight = (splitter.lastIndex < size) ? splitter.lastIndex : size
                if (separatorRight == lastStart) {
                    lastEnd = RegExp.advanceStringIndex(str, lastEnd, unicodeMatching)
                } else {
                    let substr = str.substring(lastStart, lastEnd)
                    splittedStrings.pushBack(substr)
                    if (splittedStrings.size() == lim) {
                        return splittedStrings.toArray() as String[]
                    }
                    lastStart = separatorRight
                    let numberOfCaptures = z.length;
                    for (let i = 1; i < numberOfCaptures; ++i) {
                        let nextCapture = z.$_get(i)
                        splittedStrings.pushBack(nextCapture)
                        if (splittedStrings.size() == lim) {
                            return splittedStrings.toArray() as String[]
                        }
                    }
                    lastEnd = lastStart
                }
            }
        }
        let substr = str.substring(lastStart, size)
        splittedStrings.pushBack(substr)
        let res = splittedStrings.toArray() as String[]
        return res
    }

    public match(str: String): RegExpMatchArray | null {
        if (!this.global) {
            return this.matchInternal(str)
        }
        this.lastIndex = 0;
        let matches = new containers.UndefinableStringArray();
        let indices = new Array<Array<int>>();
        let isUndefined = new Array<boolean>();
        let n = 0
        while (true) {
            let result = this.matchInternal(str)
            if (result == null) {
                if (n == 0) {
                    return null
                }
                let matchArray = new RegExpMatchArray(-1, "", matches.toUndefinableArray(), indices)
                matchArray.index = undefined
                matchArray.input = undefined
                return matchArray
            }
            else {
                let matchStr = result.$_get(0)
                matches.pushBack(matchStr)
                indices = indices.concat(result.indices)
                if (matchStr == "") {
                    this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex, this.unicode)
                }
                n++
            }
        }
    }

    public matchAll(str: String): IterableIterator<RegExpMatchArray> {
        let matcher = new RegExp(this)
        matcher.lastIndex = this.lastIndex
        if (matcher.global) {
            return new GlobalRegExpStringIterator(matcher, str)
        } else {
            return new RegExpStringIterator(matcher, str)
        }
    }

    /**
     * Executes a search for a match between a regular expression and specified string
     *
     * @param str the string against which to match the regular expression
     *
     * @returns true if match was found
     */
    public test(str: String): boolean {
        let res = this.exec(str)
        if (!res) {
            return false
        }
        return res.isCorrect
    }

    /**
     * Returns a string representing the given object
     *
     * @returns a string representing the given object
     */
    public override toString(): String {
        return "/" + this.source + "/" + this.flags_
    }

    /**
     * Returns next index from a passed one
     *
     * @param s
     *
     * @param index start position
     *
     * @param unicode true if unicode is used
     *
     * @returns new index
     */
    public static advanceStringIndex(s: String, index: int, unicode: boolean): int {
        const HIGH_SURROGATE_START = 0xD800;
        const HIGH_SURROGATE_END   = 0xDBFF;
        const LOW_SURROGATE_START  = 0xDC00;
        const LOW_SURROGATE_END    = 0xDFFF;
        const length = s.getLength();
        if (!unicode) {
            return index + 1;
        }
        if (index + 1 >= length) {
            return index + 1;
        }

        const hi = s.charCodeAt(index);
        if (hi < HIGH_SURROGATE_START || hi > HIGH_SURROGATE_END) {
            return index + 1;
        }
        
        const lo = s.charCodeAt(index + 1);
        if (lo >= LOW_SURROGATE_START && lo <= LOW_SURROGATE_END) {
            return index + 2;
        }
        return index + 1;
    }

    public static $_invoke(pattern: String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    public static $_invoke(pattern: RegExp | String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    public static $_invoke(pattern: RegExp | String) : RegExp {
        return new RegExp(pattern)
    }

    private static escapePattern(pattern: String): String {
        if (pattern == "") {
            return "(?:)"
        }
        let s = pattern.replaceAll("/", "\\/")
        return s.replaceAll("\\", "\\")
    }

    /**
     * Inits regexp from pattern and flags
     *
     * @param pattern description of a pattern
     *
     * @param flags description of flags to be used
     */
    private init(pattern: String, flags?: String) {
        this.pattern_ = pattern
        this.flags_ = flags === undefined ? "" : flags!
        if (!this.pattern_.isWellFormed()) {
            this.flags_ = this.flags_.replaceAll('u', '');
        }
        this.source_ = RegExp.escapePattern(pattern)
        this.flags_ = Array.from(this.flags_.split("")).sort().join("")
        RegExp.parse(pattern, this.flags_)
    }

    /**
     * Inits regexp from RegExp. It uses regexp's flags if flags aren't provided
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    private init(regexp: RegExp, flags?: String) {
        this.init(regexp.pattern_, flags === undefined ? regexp.flags : flags!)
    }
}
