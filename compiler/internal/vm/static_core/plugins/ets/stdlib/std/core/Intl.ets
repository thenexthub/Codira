/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

export namespace Intl {
    export type BCP47LanguageTag = string
    export type DisplayNamesFallback = "code" | "none"
    export type DisplayNamesLanguageDisplay = "dialect" | "standard"
    export type DisplayNamesType = "language" | "region" | "script" | "calendar" | "dateTimeField" | "currency"
    export type ES2018NumberFormatPartType = "literal" | "nan" | "infinity" | "percent" | "integer" | "group" | "decimal" | "fraction" | "plusSign" | "minusSign" | "percentSign" | "currency" | "code" | "symbol" | "name"
    export type ES2020NumberFormatPartType = "compact" | "exponentInteger" | "exponentMinusSign" | "exponentSeparator" | "unit" | "unknown"
    export type LDMLPluralRule = "zero" | "one" | "two" | "few" | "many" | "other"
    export type ListFormatLocaleMatcher = "lookup" | "best fit"
    export type ListFormatStyle = "long" | "short" | "narrow"
    export type ListFormatType = "conjunction" | "disjunction" | "unit"
    export type LocaleCollationCaseFirst = "upper" | "lower" | "false"
    export type LocaleHourCycleKey = "h12" | "h23" | "h11" | "h24"
    export type NumberFormatPartTypes = ES2018NumberFormatPartType | ES2020NumberFormatPartType;
    export type PluralRuleType = "cardinal" | "ordinal"
    export type RelativeTimeFormatLocaleMatcher = "lookup" | "best fit"
    export type RelativeTimeFormatNumeric = "always" | "auto"
    export type RelativeTimeFormatStyle = "long" | "short" | "narrow";
    export type RelativeTimeFormatUnit = "year" | "years" | "quarter" | "quarters" | "month" | "months" | "week" | "weeks" | "day" | "days" | "hour" | "hours" | "minute" | "minutes"| "second" | "seconds"
    export type RelativeTimeFormatUnitSingular =  "year" | "quarter" | "month" | "week" | "day" | "hour" | "minute" | "second"
    export type UnicodeBCP47LocaleIdentifier = string;
    export type DateTimeFormatPartTypes = "day" | "dayPeriod" | "era" | "fractionalSecond" | "hour" | "literal" | "minute" | "month" | "second" | "timeZoneName" | "unknown" | "weekday" | "year";
    export type DateTimeRangeFormatPartSource = "startRange" | "endRange" | "shared"
    export type LocalesArgument = UnicodeBCP47LocaleIdentifier | Locale | readonly (UnicodeBCP47LocaleIdentifier | Locale)[] | undefined

    class LocaleMatch {
        static native bestFitLocale(locale: string[]): string
        static native lookupLocale(locale: string[]): string
        static native bestFitLocales(locale: string[]): string[]
        static native lookupLocales(locale: string[]): string[]
    }

    class LocaleHelper {
        static native verifyBCP47LanguageTag(locale: string): void
    }

    export function intlLookUpLocale(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string {
        const tagList = locale instanceof Intl.BCP47LanguageTag ? [locale] : locale
        return LocaleMatch.lookupLocale(tagList)
    }

    export function intlBestFitLocale(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string {
        const tagList = locale instanceof Intl.BCP47LanguageTag ? [locale] : locale
        return LocaleMatch.bestFitLocale(tagList)
    }

    export function intlLookUpLocales(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string[] {
        const tagList = locale instanceof Intl.BCP47LanguageTag ? [locale] : locale
        return LocaleMatch.lookupLocales(tagList)
    }

    export function intlBestFitLocales(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string[] {
        const tagList = locale instanceof Intl.BCP47LanguageTag ? [locale] : locale
        return LocaleMatch.bestFitLocales(tagList)
    }

    export function intlLocalesToLanguageTags(locales: string | Intl.Locale | ReadonlyArray<string | Intl.Locale> | undefined): string[] {
        if (locales === undefined) {
            return []
        }

        if (locales instanceof string) {
            const locale = locales as string
            LocaleHelper.verifyBCP47LanguageTag(locale)
            return [locales]
        } else if (locales instanceof Intl.Locale) {
            return [locales.toString()]
        } else if (locales instanceof ReadonlyArray) {
            if (locales.length == 0) {
                return []
            }

            const langTags = new string[locales.length]
            for (let i = 0; i < locales.length; i++) {
                const locale = locales[i] as string | Intl.Locale
                langTags[i] = locale instanceof string ? locale : locale.toString()
            }
            return langTags
        } else {
            throw new TypeError("'locales' parameter has invalid type")
        }
    }

    // Intl.Collator

    export interface ResolvedCollatorOptions {
        caseFirst: string
        collation: string
        ignorePunctuation: boolean
        locale: string
        numeric: boolean
        sensitivity: string
        usage: string
    }

    export interface CollatorOptions {
        caseFirst?: string | undefined
        collation?: string | undefined
        ignorePunctuation?: boolean | undefined
        localeMatcher?: string | undefined
        numeric?: boolean | undefined
        sensitivity?: string | undefined
        usage?: string | undefined
    }

    export class Collator {
        /**
        * Compares 2 input strings x and y
        *
        * @param x input string to compare
        * @param y input string to compare
        * @return 1 if x "more" than y
        * @return -1 if y "more" than x
        * @return 0 if x equal y
        */
        public compare(x: string, y: string): number {
            if (x == y) {
                return 0
            }

            if (this.options.numeric) {
                let xNumber = new Number(x.replaceAll(',', '.'))
                let yNumber = new Number(y.replaceAll(',', '.'))

                if (!isNaN(xNumber) && !isNaN(yNumber)) {
                    if (xNumber == yNumber) {
                        return 0
                    }

                    // Intl.Collator comapres negative numbers by abs but numbers with different signs correct
                    // https://stackoverflow.com/questions/73939121/why-does-intl-collator-sort-negative-numbers-in-descending-order
                    if (xNumber < 0 && yNumber < 0) {
                        xNumber = abs(xNumber)
                        yNumber = abs(yNumber)
                    }
                    return xNumber > yNumber ? 1 : -1;
                }
            }

            let xToCompare : string = x
            let yToCompare : string = y

            if (this.options.ignorePunctuation) {
                xToCompare = this.removePunctuation(xToCompare)
                yToCompare = this.removePunctuation(yToCompare)
            }

            switch (this.options.sensitivity) {
                case "base": {
                    xToCompare = xToCompare.toLowerCase()
                    yToCompare = yToCompare.toLowerCase()

                    xToCompare = this.removeAccents(xToCompare)
                    yToCompare = this.removeAccents(yToCompare)
                }
                break;
                case "accent": {
                    xToCompare = xToCompare.toLowerCase()
                    yToCompare = yToCompare.toLowerCase()
                }
                break;
                case "case": {
                    xToCompare = this.removeAccents(xToCompare)
                    yToCompare = this.removeAccents(yToCompare)
                }
                break;
                case "variant":
                break;
            }

            if (xToCompare == yToCompare) {
                return 0
            }

            if (this.options.usage == "search") {
                return 1
            }

            const IgnoreSensitivityValues : ReadonlyArray<string> = Array.of<string>("base",  "accent")
            return this.compareByCollation(this.options.collation, this.options.locale, xToCompare, yToCompare,
                                           this.options.caseFirst)
        }

        /**
        * The Intl.Collator() constructor creates Intl.Collator objects.
        *
        * @param locales locales
        * @param options options
        */
        public constructor(locales?: string | string[], options?: CollatorOptions) {
            this.options = {caseFirst: "false", collation: "default", ignorePunctuation: false,
            locale: Locale.defaultTag(),
            numeric: false,
            sensitivity: "variant",
            usage: "sort"}

            this.options.collation = "default"
            try {
                let l = Collator.supportedLocalesOf((locales == undefined) ? "" : locales, options)
                if (l.length > 0) {
                    this.options.locale = l[0]
                    let locale = new Intl.Locale(l[0])
                    this.options.collation = locale.collation != undefined ? locale.collation! : this.options.collation
                    this.options.numeric = locale.numeric != undefined ? locale.numeric! : this.options.numeric
                    this.options.caseFirst = locale.caseFirst != undefined ? locale.caseFirst! : this.options.caseFirst
                }
            }
            catch(e) {
                throw new RangeError("Incorrect locales " + locales);
            }

            if (options != undefined) {
                Collator.checkOptions(options!)
                this.options.caseFirst = (options.caseFirst == undefined) ? this.options.caseFirst : options.caseFirst!
                this.options.ignorePunctuation = (options.ignorePunctuation == undefined) ? false : options.ignorePunctuation!
                this.options.numeric = (options.numeric == undefined) ? this.options.numeric : options.numeric!
                this.options.sensitivity = (options.sensitivity == undefined) ? "variant" : options.sensitivity!
                this.options.usage = (options.usage == undefined) ? "sort" : options.usage!
                this.options.collation = (options.collation == undefined) ? this.options.collation : options.collation!
            }
        }

        /**
        * Returns resolved options
        * @return resolved options
        */
        public resolvedOptions(): ResolvedCollatorOptions {
            return this.options;
        }

        /**
        * Returns supported locales
        * @param locales locales
        * @param options options
        * @return supported locales
        */
        static supportedLocalesOf(locales: string | string[], options?: CollatorOptions): string[] {
            if (options != undefined) {
                Collator.checkOptions(options!)
            }
            let lookup = (options != undefined) ? options.localeMatcher == "lookup" : false
            try {
                return lookup ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
            }
            catch(e) {
                if (e instanceof Error) {
                    throw e
                }
                else {
                    throw new RangeError("Incorrect locales " + locales)
                }
            }
        }


        private static errorMessage(name: string, value: string) : string {
            return "Value \"" + value + "\" out of range for Intl.Collator options property \"" + name + "\""
        }

        private static checkOptions(options: CollatorOptions) {
            if ((options.caseFirst != undefined) && !Collator.CaseFirstValues.includes(options.caseFirst!)) {
                throw new RangeError(Collator.errorMessage("caseFirst", options.caseFirst!))
            }

            if ((options.sensitivity != undefined) && !Collator.SensitivityValues.includes(options.sensitivity!)) {
                throw new RangeError(Collator.errorMessage("sensitivity", options.sensitivity!))
            }

            if ((options.usage != undefined) && !Collator.UsageValues.includes(options.usage!)) {
                throw new RangeError(Collator.errorMessage("usage", options.usage!))
            }

            if ((options.localeMatcher != undefined) && !Collator.LocaleMatcherValues.includes(options.localeMatcher!)) {
                throw new RangeError(Collator.errorMessage("localeMatcher", options.localeMatcher!))
            }
        }

        private readonly options : ResolvedCollatorOptions

        private native removePunctuation(text: string) : string
        private native removeAccents(text: string) : string

        /**
        * Returns icu-collator compare result for 2 input strings  x and y in case with non-default collation for given lang
        * @param data string array contains collation, lang, x, y
        * @return icu-collator 2 strings compare result: 1 (x "more" than y), -1 (y "more" than x) or 0 (equals)
        */
        private native compareByCollation(collation : string, lang: string, str1: string, str2: string, caseFirst: string) : double

        private static readonly SensitivityValues : ReadonlyArray<string> = Array.of<string>("base",  "accent", "case", "variant")
        private static readonly UsageValues : ReadonlyArray<string> = Array.of<string>("sort", "search")
        private static readonly CaseFirstValues : ReadonlyArray<string> = Array.of<string>("upper", "lower", "false")
        private static readonly LocaleMatcherValues : ReadonlyArray<string>  = Array.of<string>("lookup", "best fit")
    }

    // Intl.DateTimeFormat

    export interface ResolvedDateTimeFormatOptions {
        calendar: string;
        dateStyle?: "full" | "long" | "medium" | "short";
        day?: string;
        dayPeriod?: "narrow" | "short" | "long";
        era?: string;
        formatMatcher?: "basic" | "best fit" | "best fit";
        fractionalSecondDigits?: number;
        hour12?: boolean;
        hour?: string;
        hourCycle?: "h11" | "h12" | "h23" | "h24";
        locale: string;
        minute?: string;
        month?: string;
        numberingSystem: string;
        second?: string;
        timeStyle?: "full" | "long" | "medium" | "short";
        timeZone: string;
        timeZoneName?: string;
        weekday?: string;
        year?: string;
    }

    export interface DateTimeFormatPart {
        type: DateTimeFormatPartTypes;
        value: string;
    }

    export interface DateTimeRangeFormatPart extends DateTimeFormatPart {
        source: DateTimeRangeFormatPartSource
    }

    type HourCycle = "h11" | "h12" | "h23" | "h24"
    type DateStyle = "full" | "long" | "medium" | "short"
    type TimeStyle = "full" | "long" | "medium" | "short"
    type TimeZoneName = "short" | "long" | "shortOffset" | "longOffset" | "shortGeneric" | "longGeneric"
    type DayPeriod = "narrow" | "short" | "long"
    type FormatMatcher = "basic" | "best fit"

    export interface DateTimeFormatOptions {
        calendar?: string
        localeMatcher?: "lookup" | "best fit"
        dateStyle?: DateStyle
        day?: "numeric" | "2-digit"
        dayPeriod?: "narrow" | "short" | "long"
        era?: "long" | "short" | "narrow"
        formatMatcher?: "basic" | "best fit"
        // NOTE(cheezzario) 'number' literal types are not supported #23963
        // fractionalSecondDigits?: 1 | 2 | 3
        fractionalSecondDigits?: number
        hour12?: boolean
        hour?: "numeric" | "2-digit"
        hourCycle?: "h11" | "h12" | "h23" | "h24"
        minute?: "numeric" | "2-digit"
        month?: "numeric" | "2-digit" | "long" | "short" | "narrow"
        numberingSystem?: string
        second?: "numeric" | "2-digit"
        timeStyle?: TimeStyle
        timeZone?: string
        timeZoneName?: TimeZoneName
        weekday?: "long" | "short" | "narrow"
        year?: "numeric" | "2-digit"
    }

    class DateTimeFormatOptionsImpl implements DateTimeFormatOptions {
        set calendar(val: string | undefined) {this.calendar_ = val}
        get calendar(): string | undefined { return this.calendar_ }
        set localeMatcher(val: "lookup" | "best fit" | undefined) { this.localeMatcher_ = val}
        get localeMatcher(): "lookup" | "best fit" | undefined { return this.localeMatcher_}
        set dateStyle(val: DateStyle | undefined) { this.dateStyle_ = val }
        get dateStyle(): DateStyle | undefined { return this.dateStyle_}
        set day(val: "numeric" | "2-digit" | undefined) { this.day_ = val }
        get day(): "numeric" | "2-digit" | undefined { return this.day_ }
        set dayPeriod(val: "narrow" | "short" | "long" | undefined) { this.dayPeriod_ = val}
        get dayPeriod() : "narrow" | "short" | "long" | undefined { return this.dayPeriod_}
        set era(val: "long" | "short" | "narrow" | undefined) { this.era_ = val}
        get era(): "long" | "short" | "narrow" | undefined { return this.era_}
        set formatMatcher(val : "basic" | "best fit" | undefined) { this.formatMatcher_ = val}
        get formatMatcher(): "basic" | "best fit" | undefined { return this.formatMatcher_}
        // NOTE(cheezzario) 'number' literal types are not supported #23963
        // fractionalSecondDigits?: 1 | 2 | 3
        set fractionalSecondDigits(val: number | undefined) {this.fractionalSecondDigits_ = val}
        get fractionalSecondDigits(): number | undefined { return this.fractionalSecondDigits_}
        set hour12(val: boolean | undefined) {this.hour12_ = val}
        get hour12(): boolean | undefined { return this.hour12_} 
        set hour(val: "numeric" | "2-digit" | undefined) { this.hour_ = val}
        get hour(): "numeric" | "2-digit" | undefined { return this.hour_}
        set hourCycle(val: "h11" | "h12" | "h23" | "h24" | undefined) { this.hourCycle_ = val}
        get hourCycle(): "h11" | "h12" | "h23" | "h24" | undefined { return this.hourCycle_}
        set minute(val: "numeric" | "2-digit" | undefined) { this.minute_ = val}
        get minute(): "numeric" | "2-digit" | undefined { return this.minute_}
        set month(val: "numeric" | "2-digit" | "long" | "short" | "narrow" | undefined) { this.month_ = val}
        get month(): "numeric" | "2-digit" | "long" | "short" | "narrow" | undefined { return this.month_}
        set numberingSystem(val: string | undefined) { this.numberingSystem_ = val}
        get numberingSystem(): string | undefined { return this.numberingSystem_}
        set second(val: "numeric" | "2-digit" | undefined) { this.second_ = val}
        get second(): "numeric" | "2-digit" | undefined { return this.second_}
        set timeStyle(val: TimeStyle | undefined) { this.timeStyle_ = val}
        get timeStyle(): TimeStyle | undefined {return this.timeStyle_}
        set timeZone(val: string | undefined) { this.timeZone_ = val}
        get timeZone(): string | undefined { return this.timeZone_}
        set timeZoneName(val: TimeZoneName | undefined) { this.timeZoneName_ = val}
        get timeZoneName(): TimeZoneName | undefined { return this.timeZoneName_}
        set weekday(val: "long" | "short" | "narrow" | undefined) { this.weekday_ = val}
        get weekday(): "long" | "short" | "narrow" | undefined { return this.weekday_}
        set year(val: "numeric" | "2-digit" | undefined) { this.year_ = val}
        get year(): "numeric" | "2-digit" | undefined { return this.year_}

        private calendar_?: string
        private localeMatcher_?: "lookup" | "best fit"
        private dateStyle_?: DateStyle
        private day_?: "numeric" | "2-digit"
        private dayPeriod_?: "narrow" | "short" | "long"
        private era_?: "long" | "short" | "narrow"
        private formatMatcher_?: "basic" | "best fit"
        private fractionalSecondDigits_?: number
        private hour12_?: boolean
        private hour_?: "numeric" | "2-digit"
        private hourCycle_?: "h11" | "h12" | "h23" | "h24"
        private minute_?: "numeric" | "2-digit"
        private month_?: "numeric" | "2-digit" | "long" | "short" | "narrow"
        private numberingSystem_?: string
        private second_?: "numeric" | "2-digit"
        private timeStyle_?: TimeStyle
        private timeZone_?: string
        private timeZoneName_?: TimeZoneName
        private weekday_?: "long" | "short" | "narrow"
        private year_?: "numeric" | "2-digit"

        constructor() {}

        constructor(options: DateTimeFormatOptions) {
            this.calendar = options.calendar
            this.localeMatcher = options.localeMatcher
            this.dateStyle = options.dateStyle
            this.day = options.day
            this.dayPeriod = options.dayPeriod
            this.era = options.era
            this.formatMatcher = options.formatMatcher
            this.fractionalSecondDigits = options.fractionalSecondDigits
            this.hour12 = options.hour12
            this.hour = options.hour
            this.hourCycle = options.hourCycle
            this.minute = options.minute
            this.month = options.month
            this.numberingSystem = options.numberingSystem
            this.second = options.second
            this.timeStyle = options.timeStyle
            this.timeZone = options.timeZone
            this.timeZoneName = options.timeZoneName
            this.weekday = options.weekday
            this.year = options.year
        }
    }

    class DateTimeFormatPartImpl implements DateTimeFormatPart {
        private _type: DateTimeFormatPartTypes
        private _value: String

        constructor() {
            this._type = "literal"
            this._value = ""
        }

        constructor(type: DateTimeFormatPartTypes, value: string) {
            this._type = type
            this._value = value
        }

        get type(): DateTimeFormatPartTypes {
            return this._type
        }

        set type(value: DateTimeFormatPartTypes) {
            this._type = value
        }

        get value(): string {
            return this._value
        }

        set value(value: string) {
            this._value = value
        }
    }

    class DateTimeRangeFormatPartImpl extends DateTimeFormatPartImpl implements DateTimeRangeFormatPart {
        private  _source: DateTimeRangeFormatPartSource

        constructor() {
            this._source = "shared"
        }

        constructor(type: DateTimeFormatPartTypes, value: string, source: DateTimeRangeFormatPartSource) {
            super(type, value)
            this._source = source
        }

        get source(): DateTimeRangeFormatPartSource {
            return this._source
        }

        set source(source: DateTimeRangeFormatPartSource) {
            this._source = source
        }
    }

    class ResolvedDateTimeFormatOptionsImpl implements ResolvedDateTimeFormatOptions {
        private _calendar: string
        private _dateStyle?: DateStyle
        private _day?: string
        private _dayPeriod?: DayPeriod
        private _era?: string
        private _formatMatcher?: FormatMatcher
        private _fractionalSecondDigits?: number
        private _hour12?: boolean
        private _hour?: string
        private _hourCycle?: HourCycle
        private _locale: string
        private _minute?: string
        private _month?: string
        private _numberingSystem: string
        private _second?: string
        private _timeStyle?: TimeStyle
        private _timeZone: string
        private _timeZoneName?: string
        private _weekday?: string
        private _year?: string

        constructor(locale: string, calendar: string, numberingSystem: string, timeZone: string) {
            this._locale = locale
            this._calendar = calendar
            this._numberingSystem = numberingSystem
            this._timeZone = timeZone
        }

        get calendar(): string {
            return this._calendar
        }
        set calendar(calendar: string) {
            this._calendar = calendar
        }

        get dateStyle(): DateStyle | undefined {
            return this._dateStyle
        }
        set dateStyle(dateStyle: DateStyle | undefined) {
            this._dateStyle = dateStyle
        }

        get day(): string | undefined {
            return this._day
        }
        set day(day: string | undefined) {
            this._day = day
        }

        get dayPeriod(): DayPeriod | undefined {
            return this._dayPeriod
        }
        set dayPeriod(dayPeriod: DayPeriod | undefined) {
            this._dayPeriod = dayPeriod
        }

        get era(): string | undefined {
            return this._era
        }
        set era(era: string | undefined) {
            this._era = era
        }

        get formatMatcher(): FormatMatcher | undefined {
            return this._formatMatcher
        }
        set formatMatcher(formatMatcher: FormatMatcher | undefined) {
            this._formatMatcher = formatMatcher
        }

        get fractionalSecondDigits(): number | undefined {
            return this._fractionalSecondDigits
        }
        set fractionalSecondDigits(fractionalSecondDigits: number | undefined) {
            this._fractionalSecondDigits = fractionalSecondDigits
        }

        get hour12(): boolean | undefined {
            return this._hour12
        }
        set hour12(hour12: boolean | undefined) {
            this._hour12 = hour12
        }

        get hour(): string | undefined {
            return this._hour
        }
        set hour(hour: string | undefined) {
            this._hour = hour
        }

        get hourCycle(): HourCycle | undefined {
            return this._hourCycle
        }
        set hourCycle(hourCycle: HourCycle | undefined) {
            this._hourCycle = hourCycle
        }

        get locale(): string {
            return this._locale
        }
        set locale(locale: string) {
            this._locale = locale
        }

        get minute(): string | undefined {
            return this._minute
        }
        set minute(minute: string | undefined) {
            this._minute = minute
        }

        get month(): string | undefined {
            return this._month
        }
        set month(month: string | undefined) {
            this._month = month
        }

        get numberingSystem(): string {
            return this._numberingSystem
        }
        set numberingSystem(numberingSystem: string) {   
            this._numberingSystem = numberingSystem
        }

        get second(): string | undefined {
            return this._second
        }
        set second(second: string | undefined) {
            this._second = second
        }

        get timeStyle(): TimeStyle | undefined {
            return this._timeStyle
        }
        set timeStyle(timeStyle: TimeStyle | undefined) {
            this._timeStyle = timeStyle
        }

        get timeZone(): string {
            return this._timeZone
        }
        set timeZone(timeZone: string) {   
            this._timeZone = timeZone
        }

        get timeZoneName(): string | undefined {
            return this._timeZoneName
        }
        set timeZoneName(timeZoneName: string | undefined) {
            this._timeZoneName = timeZoneName
        }

        get weekday(): string | undefined {
            return this._weekday
        }
        set weekday(weekday: string | undefined) {
            this._weekday = weekday
        }

        get year(): string | undefined {
            return this._year
        }
        set year(year: string | undefined) {
            this._year = year
        }
    }

export class DateTimeFormat {
        private static isEmpty(options: DateTimeFormatOptions): boolean {
            return options.localeMatcher === undefined 
                    && options.dateStyle === undefined 
                    && options.day === undefined 
                    && options.dayPeriod === undefined 
                    && options.era === undefined
                    && options.formatMatcher === undefined 
                    && options.fractionalSecondDigits === undefined 
                    && options.hour12 === undefined 
                    && options.hour === undefined 
                    && options.hourCycle === undefined 
                    && options.minute === undefined 
                    && options.month === undefined 
                    && options.numberingSystem === undefined 
                    && options.second === undefined 
                    && options.timeStyle === undefined 
                    && options.timeZone === undefined 
                    && options.timeZoneName === undefined 
                    && options.weekday === undefined 
                    && options.year === undefined
        }

        private readonly locale: string
        private readonly options: DateTimeFormatOptionsImpl
        private readonly pattern: string
        private readonly cacheKey: string

        public constructor(locales?: string | string[], options?: DateTimeFormatOptions) {
            if (locales === undefined) {
                this.locale = Locale.defaultTag()
            } else {
                const lookupLocale = options?.localeMatcher == "lookup"
                this.locale = lookupLocale ? intlLookUpLocale(locales!) : intlBestFitLocale(locales!)
            }

            if (options === undefined || DateTimeFormat.isEmpty(options)) {
                this.options = { year: "numeric", month: "numeric", day: "numeric" } as DateTimeFormatOptionsImpl
            } else {
                this.options = this.getResolvedOptions(options!) as DateTimeFormatOptionsImpl
            }

            this.pattern = this.getPattern()
            this.cacheKey = this.generateCacheKey();
        }

        public static $_invoke(locales?: string | string[], options?: DateTimeFormatOptions): DateTimeFormat {
            return new DateTimeFormat(locales, options)
        }

        public format(date?: Date | number): string {
            if (date === undefined) {
                return this.formatImpl(Date.now(), this.cacheKey)
            }
            if (date instanceof Date) {
                return this.formatImpl(date.getTime(), this.cacheKey)
            }
            return this.formatImpl(date!, this.cacheKey)
        }

        public formatToParts(date?: Date | number): DateTimeFormatPart[] {
            if (date === undefined) {
                return this.formatToPartsImpl(Date.now(), this.cacheKey)
            }
            if (date instanceof Date) {
                return this.formatToPartsImpl(date.getTime(), this.cacheKey)
            }
            return this.formatToPartsImpl(date!, this.cacheKey)
        }

        public formatRange(startDate: Date | number, endDate: Date | number): string {
            const startDateTime = this.getDateTime(startDate)
            const endDateTime = this.getDateTime(endDate)
            return this.formatRangeImpl(startDateTime, endDateTime, this.cacheKey)
        }

        public formatRangeToParts(startDate: Date | number, endDate: Date | number): DateTimeRangeFormatPart[] {
            const startDateTime = this.getDateTime(startDate)
            const endDateTime = this.getDateTime(endDate)
            return this.formatRangeToPartsImpl(startDateTime, endDateTime, this.cacheKey)
        }

        public resolvedOptions(): ResolvedDateTimeFormatOptions {
            const resolvedOptions = this.resolvedOptionsImpl(this.cacheKey)

            if (resolvedOptions.hourCycle === undefined) {
                const hourCycleOpt = this.options.hourCycle
                resolvedOptions.hourCycle = hourCycleOpt ? hourCycleOpt : this.getLocaleHourCycle()
            }

            resolvedOptions.locale = this.locale

            resolvedOptions.hour12 = resolvedOptions.hourCycle == "h11" || resolvedOptions.hourCycle == "h12"

            resolvedOptions.dateStyle = this.options.dateStyle
            resolvedOptions.day = this.options.day
            resolvedOptions.dayPeriod = this.options.dayPeriod
            resolvedOptions.era = this.options.era
            resolvedOptions.fractionalSecondDigits = this.options.fractionalSecondDigits
            resolvedOptions.hour = this.options.hour
            resolvedOptions.minute = this.options.minute
            resolvedOptions.month = this.options.month
            resolvedOptions.second = this.options.second
            resolvedOptions.timeStyle = this.options.timeStyle
            resolvedOptions.timeZoneName = this.options.timeZoneName
            resolvedOptions.weekday = this.options.weekday
            resolvedOptions.year = this.options.year

            return resolvedOptions
        }

        public static supportedLocalesOf(locales: string | Locale | ReadonlyArray<string | Locale>, options?: DateTimeFormatOptions): string[] {
            const langTags = intlLocalesToLanguageTags(locales)

            if (options) {
                return (options.localeMatcher == "best fit")
                        ? intlBestFitLocales(langTags)
                        : intlLookUpLocales(langTags)
            }

            return intlBestFitLocales(langTags)
        }

        private native formatImpl(timestamp: number, cacheKey: string): string

        private native formatToPartsImpl(timestamp: number, cacheKey: string): Intl.DateTimeFormatPart[]

        private native formatRangeImpl(start: number, end: number, cacheKey: string): string

        private native formatRangeToPartsImpl(start: number, end: number, cacheKey: string):
                                              Intl.DateTimeRangeFormatPart[]

        private native getLocaleHourCycle(): HourCycle | undefined

        private native resolvedOptionsImpl(cacheKey: string): ResolvedDateTimeFormatOptions

        private getDateTime(date: Date | number): number {
            return date instanceof Date ? date.getTime() : date
        }

        private getResolvedOptions(options: DateTimeFormatOptions): DateTimeFormatOptionsImpl {
            let ret = new DateTimeFormatOptionsImpl(options)
            if (options.hour12 !== undefined) {
                let hourCycle = this.getLocaleHourCycle()

                const hour12: boolean = options.hour12!
                if (hour12) {
                    if (hourCycle == "h23") {
                        hourCycle = "h11"
                    } else if (hourCycle == "h24") {
                        hourCycle = "h12"
                    }
                } else {
                    hourCycle = "h23"
                }

                ret.hourCycle = hourCycle
            }

            return ret
        }

        private getHourSymbol(): string {
            const hourCycle = this.options.hourCycle
            if (hourCycle) {
                switch (hourCycle) {
                    case "h11":
                        return "K"
                    case "h12":
                        return "h"
                    case "h23":
                        return "H"
                    case "h24":
                        return "k"
                    default:
                        throw new RangeError("Invalid hour cycle: " + hourCycle)
                }
            } else {
                return "j"
            }
        }

        private toDefaultDateTimeSkeleton(): string {
            let needDateDefaults:boolean = true
            const skeleton = new StringBuilder()
            if (this.options.year !== undefined || this.options.month !== undefined || this.options.weekday !== undefined || this.options.day !== undefined) {
                needDateDefaults = false
            }
            if (this.options.hour !== undefined || this.options.minute !== undefined || this.options.second !== undefined) {
                needDateDefaults = false
            }
            if (this.options.dateStyle !== undefined || this.options.timeStyle !== undefined) {
                needDateDefaults = false
            }

            const hcDefault = this.getLocaleHourCycle()
            const hourCycle = this.options.hourCycle 
            const hc = hourCycle == undefined ? hcDefault : hourCycle
            if (needDateDefaults) {
                skeleton.append("yMd")
                switch (hc) {
                    case "h11":
                        skeleton.append("Kms")
                        break
                    case "h12":
                        skeleton.append("hms")
                        break
                    case "h23":
                    case undefined:
                        skeleton.append("Hms")
                        break
                    case "h24":
                        skeleton.append("kms")
                        break
                    default:
                        break
                }

            }
            return skeleton.toString()
        }

        private getPattern(): string {
            const datePattern = this.getDatePattern()
            const timePattern = this.getTimePattern()
            const dtfSkeleton = this.toDefaultDateTimeSkeleton()
            return datePattern + timePattern + dtfSkeleton
        }

        private getDatePattern(): string {
            const datePattern = new StringBuilder()

            this.appendYearPattern(datePattern)
            this.appendMonthPattern(datePattern)
            this.appendWeekdayPattern(datePattern)
            this.appendDayPattern(datePattern)
            this.appendDayPeriodPattern(datePattern)
            this.appendEraPattern(datePattern)

            return datePattern.toString()
        }

        private appendYearPattern(pattern: StringBuilder): void {
            const year = this.options.year
            if (year) {
                switch (year) {
                    case "numeric":
                        pattern.append("yyyy")
                        break
                    case "2-digit":
                        pattern.append("yy")
                        break
                }
            }
        }

        private appendMonthPattern(pattern: StringBuilder): void {
            const month = this.options.month
            if (month) {
                switch (month) {
                    case "numeric":
                        pattern.append("M")
                        break
                    case "narrow":
                        pattern.append("MMMMM")
                        break
                    case "long":
                        pattern.append("MMMM")
                        break
                    case "short":
                        pattern.append("MMM")
                        break
                    case "2-digit":
                        pattern.append("MM")
                        break
                }
            }
        }

        private appendWeekdayPattern(pattern: StringBuilder): void {
            const weekday = this.options.weekday
            if (weekday) {
                switch (weekday) {
                    case "long":
                        pattern.append("EEEE")
                        break
                    case "short":
                        pattern.append("EEE")
                        break
                    case "narrow":
                        pattern.append("EEEEE")
                        break
                }
            }
        }

        private appendDayPattern(pattern: StringBuilder): void {
            const day = this.options.day
            if (day) {
                switch (day) {
                    case "numeric":
                        pattern.append("d")
                        break
                    case "2-digit":
                        pattern.append("dd")
                        break
                }
            }
        }

        private appendDayPeriodPattern(pattern: StringBuilder): void {
            const dayPeriod = this.options.dayPeriod
            if (dayPeriod) {
                pattern.append("B")
            }
        }

        private appendEraPattern(pattern: StringBuilder): void {
            const optEra = this.options.era
            if (optEra) {
                switch (optEra) {
                    case "long":
                        pattern.append("GGGG")
                        break
                    case "short":
                        pattern.append("GGG")
                        break
                    case "narrow":
                        pattern.append("GGGGG")
                        break
                }
            }
        }

        private getTimePattern(): string {
            const timePattern = new StringBuilder()

            this.appendHourPattern(timePattern)
            this.appendMinutePattern(timePattern)
            this.appendSecondPattern(timePattern)
            this.appendFractionalSecondPattern(timePattern)
            this.appendTimeZoneNamePattern(timePattern)
            
            return timePattern.toString()
        }

        private appendHourPattern(pattern: StringBuilder): void {
            const optHour = this.options.hour
            if (optHour) {
                const hourSymbol = this.getHourSymbol()
                switch (optHour) {
                    case "2-digit":
                        pattern.append(hourSymbol.repeat(2))
                        break
                    case "numeric":
                        pattern.append(hourSymbol)
                        break
                }
            }
        }

        private appendMinutePattern(pattern: StringBuilder): void {
            const optMinute = this.options.minute
            if (optMinute) {
                switch (optMinute) {
                    case "2-digit":
                        pattern.append("mm")
                        break
                    case "numeric":
                        pattern.append("m")
                        break
                }
            }
        }

        private appendSecondPattern(pattern: StringBuilder): void {
            const optSecond = this.options.second
            if (optSecond) {
                switch (optSecond) {
                    case "2-digit":
                        pattern.append("ss")
                        break
                    case "numeric":
                        pattern.append("s")
                        break
                }
            }
        }

        private appendFractionalSecondPattern(pattern: StringBuilder): void {
            const optFractionalSecond = this.options.fractionalSecondDigits
            if (optFractionalSecond) {
                if (optFractionalSecond < 1 || optFractionalSecond > 3) {
                    throw new RangeError("invalid digits value: " + optFractionalSecond)
                }
                pattern.append("S".repeat(optFractionalSecond.toInt()))
            }
        }

        private appendTimeZoneNamePattern(pattern: StringBuilder): void {
            const optTimeZoneName = this.options.timeZoneName
            if (optTimeZoneName) {
                switch (optTimeZoneName) {
                    case "short":
                        pattern.append("z");
                        break
                    case "long":
                        pattern.append("zzzz")
                        break
                    case "shortGeneric":
                        pattern.append("v")
                        break
                    case "shortOffset":
                        pattern.append("O")
                        break
                    case "longGeneric":
                        pattern.append("vvvv")
                        break
                    case "longOffset":
                        pattern.append("OOOO")
                        break
                }
            }
        }

        private generateCacheKey(): string {
            const sb = new StringBuilder();
            sb.append(this.locale ?? "").append(";");
            if (this.pattern !== undefined && this.pattern.length > 0) {
                sb.append(this.pattern).append(";");
            } else {
                const dateStyle = this.options.dateStyle ?? "";
                const timeStyle = this.options.timeStyle ?? "";
                sb.append("style/").append(dateStyle).append("/").append(timeStyle).append(";");
            }

            sb.append(this.options.calendar ?? "").append(";");
            sb.append(this.options.hourCycle ?? "").append(";");
            sb.append(this.options.numberingSystem ?? "").append(";");
            sb.append(this.options.timeZone ?? "").append(";");
            sb.append(this.options.timeZoneName ?? "").append(";");
            sb.append(this.options.formatMatcher ?? "");

            return sb.toString();
        }
    }

    // Intl.DisplayNames

    /**
     * Options for creating a DisplayNames object
     */
    export interface DisplayNamesOptions {
        /** How to handle code fallbacks when no display name is available */
        fallback?: DisplayNamesFallback;
        /** How to display language names (only applies when type is "language") */
        languageDisplay?: DisplayNamesLanguageDisplay;
        /** Strategy for locale matching */
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
        /** Display style for names (narrow, short, or long) */
        style?: RelativeTimeFormatStyle;
        /** Type of code to be displayed (required) */
        type: DisplayNamesType;
    }

    /**
     * Options for locale matching when using supportedLocalesOf
     */
    export interface DisplayNamesLocaleMatcherOptions {
        /** Strategy for locale matching */
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
    }

    /**
     * Resolved options returned by the resolvedOptions method
     */
    export interface ResolvedDisplayNamesOptions {
        /** Current fallback setting */
        fallback: DisplayNamesFallback;
        /** Current language display setting (only present when type is "language") */
        languageDisplay?: DisplayNamesLanguageDisplay;
        /** Resolved locale identifier */
        locale: UnicodeBCP47LocaleIdentifier;
        /** Current display style */
        style: RelativeTimeFormatStyle;
        /** Current type of codes being displayed */
        type: DisplayNamesType;
    }

    /**
     * Format and display locale-sensitive names such as languages, regions, scripts, etc.
     */
    export class DisplayNames {
        /** The resolved locale for this instance */
        private locale: string;
        /** The display style (narrow, short, or long) */
        private style: RelativeTimeFormatStyle;
        /** The type of code to display */
        private type: DisplayNamesType;
        /** How to handle fallbacks */
        private fallback: DisplayNamesFallback;
        /** How to display language names */
        private languageDisplay?: DisplayNamesLanguageDisplay;


        private native static ofNative(locale: string, type: string, code: string, style: string,
                               fallback: string, languageDisplay?: string): string;

        /**
         * Creates a new DisplayNames object
         *
         * @param locales The locale or array of locales to use
         * @param options Configuration options
         *
         * @throws TypeError If options is undefined or type is not specified
         * @throws RangeError If an invalid locale or option value is provided
         */
        public constructor(
            locales?: BCP47LanguageTag | BCP47LanguageTag[],
            options?: DisplayNamesOptions
        ) {
            if (options === undefined) {
                throw new TypeError("Options parameter is required");
            }

            if (options.type === undefined) {
                throw new TypeError("Option 'type' is required");
            }

            // Set default values and process options
            const localeMatcher = options.localeMatcher ?? 'best fit';
            this.style = options.style ?? 'long';
            this.type = options.type;
            this.fallback = options.fallback ?? 'code';

            // Only set languageDisplay if type is 'language'
            if (this.type === 'language') {
                this.languageDisplay = options.languageDisplay ?? 'dialect';
            }

            if (locales !== undefined) {
                const lookupLocale = options != null && options!.localeMatcher == "lookup"
                this.locale = lookupLocale ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = Locale.defaultTag();
            }
        }

        /**
         * Returns the localized display name for the provided code
         *
         * @param code The code to get the display name for
         * @returns The localized display name, the code itself (if fallback is 'code'), or undefined
         *
         * @throws RangeError If the code is invalid for the current type
         */
        public of(code: string): string | undefined {
            if (code.isEmpty()) {
                throw new RangeError("Code cannot be empty");
            }
            try {
                const result = DisplayNames.ofNative(
                    this.locale,
                    this.type,
                    code,
                    this.style,
                    this.fallback,
                    this.languageDisplay
                );

                return result;
            } catch (e) {
                if (e instanceof RangeError) throw e;
                throw new RangeError(`Invalid code "${code}" for type "${this.type}"`);
            }
        }

        /**
         * Returns the resolved options for this DisplayNames instance
         *
         * @returns The resolved options object
         */
        public resolvedOptions(): ResolvedDisplayNamesOptions {
            const result: ResolvedDisplayNamesOptions = {
                locale: this.locale,
                style: this.style,
                type: this.type,
                fallback: this.fallback
            };

            // Only include languageDisplay if type is 'language'
            if (this.type === 'language' && this.languageDisplay !== undefined) {
                result.languageDisplay = this.languageDisplay;
            }

            return result;
        }

        /**
        * Returns an array of supported locales for text segmentation
        *
        * @param locales An array of BCP 47 language tags to check
        * @param options Optional configuration for locale matching
        * @returns An array of supported locales from the input
        */
        public static supportedLocalesOf(locales: string | Locale | FixedArray<string | Locale>): string[] {
            const supportedLocales = new Array<string>()

            if (locales instanceof FixedArray<string>) {
                return DisplayNames.bestFitLocales(Array.from(locales))
            } else if (locales instanceof FixedArray<Locale>) {
                return DisplayNames.bestFitLocales(Array.from(locales, (loc: Locale) => loc.toString()))
            } else if (locales instanceof string) {
                return DisplayNames.bestFitLocales(Array.of(locales))
            } else if (locales instanceof Locale) {
                return DisplayNames.bestFitLocales(Array.of(locales).map((loc: Locale) => loc.toString()))
            } else {
                throw new TypeError("'locales' parameter has invalid type")
            }
        }

        private static bestFitLocales(locales: Array<string>): string[] {
            return intlBestFitLocales(locales)
        }
    }

    // Intl.ListFormat

    export interface ListFormatOptions {
        localeMatcher?: ListFormatLocaleMatcher | undefined
        style?: ListFormatStyle | undefined
        type?: ListFormatType | undefined
    }

    export interface FormatToPartsResult {
        type: "element" | "literal"
        value: string
    }
    
    export class ListFormat {
        private locale: string
        private style: ListFormatStyle
        private type: ListFormatType

        public constructor(locales?: string | string[], options?: ListFormatOptions) {
            this.style = options?.style ?? "long"
            this.type = options?.type ?? "conjunction"
            this.locale = this.resolveLocale(locales, options?.localeMatcher)
        }

        public format(list: Iterable<string>): string {
            const parts = this.formatToParts(list)
            return parts.map((part: FormatToPartsResult) => part.value).join('')
        }

        public formatToParts(list: Iterable<string>): Array<FormatToPartsResult> {
            const arr = ListFormat.iterableToArray(list)
            if (arr.length == 0) {
                return new Array<FormatToPartsResult>()
            }
            const squashed = ListFormat.formatToPartsNative(arr, this.locale, this.style, this.type) as string[]
            if (squashed.length % 2 != 0) {
                throw Error("ListFormat.formatToPartsNative incorrect result")
            }
            const result = new Array<FormatToPartsResult>(squashed.length / 2)
            for (let i = 0; i < result.length; i++) {
                result[i] = {
                    type: squashed[i * 2] as "element" | "literal",
                    value: squashed[i * 2 + 1]
                } as FormatToPartsResult
            }
            return result
        }

        public static supportedLocalesOf(locales: string | string[], options?: ListFormatLocaleMatcher): string[] {
            return options === "lookup" ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
        }

        private resolveLocale(locales?: string | string[], localeMatcher?: ListFormatLocaleMatcher): string {
            if (locales === undefined) {
                return Locale.defaultTag()
            }
            return localeMatcher === "lookup" ? intlLookUpLocale(locales!) : intlBestFitLocale(locales!)
        }

        private static iterableToArray(list: Iterable<string>): string[] {
            const arr = new Array<string>()

            const it = list.$_iterator()
            for (let itRes = it.next(); !itRes.done; itRes = it.next()) {
                const item = itRes.value
                if (item !== undefined) {
                    arr.push(item)
                }
            }
            
            const result = new string[arr.length]
            for (let i = 0; i < arr.length; i++) {
                result[i] = arr[i]
            }
            return result
        }

        private static native formatToPartsNative(
            list: string[], locale: string, style: string, type: string): Object
    }

    // Intl.Locale

    export interface LocaleOptions {
        baseName?: string
        language?: string
        calendar?: string
        caseFirst?: string // LocaleCollationCaseFirst
        numeric?: boolean
        collation?: string
        hourCycle?: string  // LocaleHourCycleKey
        numberingSystem?: string
        region?: string
        script?: string
    }

    enum LocaleInfoKeys {
        LANG,
        SCRIPT,
        COUNTRY,
        U_CA,
        U_KF,
        U_CO,
        U_HC,
        U_NU,
        U_KN,
        COUNT
    }

    final class ParsedLocaleData {
        language: string | undefined
        script: string | undefined
        region: string | undefined
        calendar: string | undefined
        collation: string | undefined
        numberingSystem: string | undefined
        hourCycle: string | undefined
        numeric: boolean | undefined
        caseFirst: string | undefined
        baseName: string | undefined
        
        constructor(
            language: string | undefined,
            script: string | undefined,
            region: string | undefined,
            calendar: string | undefined,
            collation: string | undefined,
            numberingSystem: string | undefined,
            hourCycle: string | undefined,
            numeric: boolean | undefined,
            caseFirst: string | undefined,
            baseName: string | undefined
        ) {
            this.language = language
            this.script = script
            this.region = region
            this.calendar = calendar
            this.collation = collation
            this.numberingSystem = numberingSystem
            this.hourCycle = hourCycle
            this.numeric = numeric
            this.caseFirst = caseFirst
            this.baseName = baseName
        }
    }

    export class Locale implements LocaleOptions {
        // Single-entry cache for parseTag to avoid re-parsing the same tag
        // This is effective for common patterns like repeated locale creation with the same tag
        // Using single entry keeps memory footprint minimal while covering most use cases
        private static lastParsedTag: string | undefined = undefined;
        private static lastParsedResult: ParsedLocaleData | undefined = undefined;
        
        /**
         * Get cached locale data for a given tag
         * @param tag The BCP47 language tag to lookup
         * @returns Cached ParsedLocaleData or undefined if not found
         */
        private static getCachedLocale(tag: string): ParsedLocaleData | undefined {
            if (Locale.lastParsedTag == tag && Locale.lastParsedResult != undefined) {
                return Locale.lastParsedResult
            }
            return undefined
        }
        
        /**
         * Cache locale data for a given tag
         * @param tag The BCP47 language tag to use as cache key
         * @param data The parsed locale data to cache
         */
        private static setCachedLocale(tag: string, data: ParsedLocaleData): void {
            Locale.lastParsedTag = tag
            Locale.lastParsedResult = data
        }
        
        private _baseName?: string;
        private _language?: string;
        private _calendar?: string;
        private _caseFirst?: string;
        private _numeric?: boolean;
        private _collation?: string;
        private _hourCycle?: string;
        private _numberingSystem?: string;
        private _region?: string;
        private _script?: string;

        set baseName(value: string | undefined) {
            this._baseName = value
        }

        get baseName(): string | undefined {
            return this._baseName
        }

        set language(value: string | undefined) {
            this._language = value
        }

        get language(): string | undefined {
            return this._language
        }

        set calendar(value: string | undefined) {
            this._calendar = value
        }

        get calendar(): string | undefined {
            return this._calendar
        }

        set caseFirst(value: string | undefined) {
            this._caseFirst = value
        }

        get caseFirst(): string | undefined {
            return this._caseFirst
        }

        set numeric(value: boolean | undefined) {
            this._numeric = value
        }

        get numeric(): boolean | undefined {
            return this._numeric
        }

        set collation(value: string | undefined) {
            this._collation = value
        }

        get collation(): string | undefined {
            return this._collation
        }

        set hourCycle(value: string | undefined) {
            this._hourCycle = value
        }

        get hourCycle(): string | undefined {
            return this._hourCycle
        }

        set numberingSystem(value: string | undefined) {
            this._numberingSystem = value
        }

        get numberingSystem(): string | undefined {
            return this._numberingSystem
        }

        set region(value: string | undefined) {
            this._region = value
        }

        get region(): string | undefined {
            return this._region
        }

        set script(value: string | undefined) {
            this._script = value
        }

        get script(): string | undefined {
            return this._script
        }

        /**
        * The Intl.Locale() constructor creates Intl.Locale objects.
        *
        * @param tag BCP47LanguageTag or other Locale
        * @param options options
        */
        public constructor(tag: BCP47LanguageTag | Locale, options?: LocaleOptions) {
            // Step 1: Initialize locale
            this.initLocale()
            
            // Use local variable for default language to avoid multiple calls within same constructor
            let defaultLanguage: string | undefined = undefined
            let canSkipValidation: boolean = false
            
            // Step 2: Process tag parameter
            if (tag instanceof BCP47LanguageTag) {
                this.tag = tag
                canSkipValidation = this.parseTag()
            } else {
                // Get default language only once if needed for either language or baseName
                if (tag.language == undefined || tag.baseName == undefined) {
                    defaultLanguage = this.defaultLang()
                }
                
                this.language = (tag.language != undefined) ? tag.language : defaultLanguage
                this.baseName = (tag.baseName != undefined) ? tag.baseName : defaultLanguage
                this.calendar = (tag.calendar != undefined) ? tag.calendar : ""
                this.caseFirst = (tag.caseFirst != undefined) ? tag.caseFirst : undefined
                this.numeric = (tag.numeric != undefined) ? tag.numeric : false
                this.collation = tag.collation
                this.hourCycle = tag.hourCycle
                this.numberingSystem = tag.numberingSystem
                this.region = tag.region
                this.script = tag.script
                this.tag = tag.tag
            }
            
            // Step 3: Process options parameter
            if (options != undefined) {
                canSkipValidation = false  // Options modify the locale, so validation is required
                this.checkOptions(options)
                
                // Only fetch defaultLang if we need it and haven't already cached it in local variable
                if ((options.language == undefined || options.baseName == undefined) && defaultLanguage == undefined) {
                    defaultLanguage = this.defaultLang()
                }
                
                // For options overrides, use options value if provided, else keep current value or use default
                if (options.language != undefined) {
                    this.language = options.language
                } else if (this.language == undefined && defaultLanguage != undefined) {
                    this.language = defaultLanguage
                }
                if (options.baseName != undefined) {
                    this.baseName = options.baseName
                } else if (this.baseName == undefined && defaultLanguage != undefined) {
                    this.baseName = defaultLanguage
                }
                this.calendar = (options.calendar != undefined) ? options.calendar : ""
                this.caseFirst = (options.caseFirst != undefined) ? options.caseFirst : undefined
                this.numeric = (options.numeric != undefined) ? options.numeric : false
                this.collation = (options.collation != undefined) ? options.collation : this.collation
                this.hourCycle = (options.hourCycle != undefined) ? options.hourCycle : this.hourCycle
                this.numberingSystem = (options.numberingSystem != undefined) ? options.numberingSystem : this.numberingSystem
                this.region = (options.region != undefined) ? (options.region as string).toUpperCase() : this.region
                this.script = (options.script != undefined) ? options.script : this.script
            }

            // Step 4: Validate region and script (skip if we used cached data), then create tag (always)
            if (!canSkipValidation) {
                // Validate region
                if (this.region != undefined) {
                    let regionUpper: string = (this.region as string).toUpperCase()
                    if (!this.regionList().includes(regionUpper)) {
                        throw new RangeError(this.errMsg)
                    }
                }

                // Validate script
                if (this.script != undefined && !this.scriptList().includes(this.script as string)) {
                    throw new RangeError(this.errMsg)
                }
            }
            this.tag = this.createTag()
        }

        native langList() : string
        native regionList() : string
        native scriptList() : string
        native numberingSystemList() : string
        native initLocale() : void
        native maximizeInfo(lang: string) : string
        native isTagValid(tag: string) : int
        native parseTagImpl(tag:string) : string
        private native defaultLang() : string

        static native defaultTag(): string

        private fillBaseName() {
            let sb = new StringBuilder()
            sb.append(this.language as string)
            if (this.script != undefined && this.script != "" && this.script != " ") {
                sb.append("-")
                sb.append(this.script as string)
            }
            if (this.region != undefined && this.region != "" && this.region != " ") {
                sb.append("-")
                sb.append(this.region as string)
            }
            this.baseName = sb.toString()
        }

        /**
        * The maximize() method of Intl.Locale instances gets the most likely values for the language,
        * script, and region of this locale based on existing values.
        * @return A Intl.Locale instance whose baseName property returns the result of the
        * Add Likely Subtags algorithm executed against locale.baseName.
        */
        public maximize() : Locale {
            let res = new Locale(this)
            let info = this.maximizeInfo(this.language as string).split(",")
            res.region = info[0]
            res.script = info.length > 1 ? info[1] : res.script
            this.fillBaseName()
            res.tag = res.createTag()
            return res
        }

        /**
        * The minimize() attempts to remove information about this locale that would be added by calling maximize().
        * @return A Intl.Locale instance whose baseName property returns the result of the
        * Remove Likely Subtags algorithm executed against locale.baseName.
        */
        public minimize() : Locale {
            let res = new Locale(this)
            res.region = undefined
            res.script = undefined
            res.baseName = res.language
            res.tag = res.createTag()
            return res
        }

        /**
        * The toString() method of Intl.Locale instances returns this Locale's full locale identifier string.
        * @return The locale's Unicode locale identifier string.
        */
        public toString() : BCP47LanguageTag {
            return this.tag
        }

        private tag: BCP47LanguageTag = ""

        private uFlags() : BCP47LanguageTag {
            let sb = new StringBuilder()
            if ((this.numeric != undefined && this.numeric as boolean == true)) {
                sb.append("-u-kn")
            }
            if (this.caseFirst != undefined && this.caseFirst != "") {
                sb.append("-u-kf-" + this.caseFirst as string)
            }
            if (this.calendar != undefined && this.calendar != "") {
                sb.append("-u-ca-" + this.calendar as string)
            }
            if (this.collation != undefined && this.collation != "") {
                sb.append("-u-co-" + this.collation as string)
            }
            if (this.hourCycle != undefined && this.hourCycle != "" && (this.hourCycle instanceof LocaleHourCycleKey)) {
                sb.append("-u-hc-" + this.hourCycle as string)
            }
            if (this.numberingSystem != undefined && this.numberingSystem != "") {
                sb.append("-u-nu-" + this.numberingSystem as string)
            }
            return sb.toString()
        }

        private createTag() : BCP47LanguageTag {
            let sb = new StringBuilder()
            this.fillBaseName()
            sb.append(this.baseName as string)
            sb.append(this.uFlags())
            return sb.toString()
        }

        private errOptionMessage(val : string, name : string) : string {
            return "Value " + val + " out of range for locale options property " + name
        }

        private errMsg = "Incorrect locale information provided"
        private checkOptions(options: LocaleOptions) {
            if  ((options.calendar != undefined && !Locale.calendarValues.includes(options.calendar!)) ||
                (options.caseFirst != undefined && !Locale.caseFirstValues.includes(options.caseFirst!)) ||
                (options.collation != undefined && !Locale.collationValues.includes(options.collation!)) ||
                (options.hourCycle != undefined && !Locale.hourCycleValues.includes(options.hourCycle!)) ||
                (options.numberingSystem != undefined && (options.numberingSystem! == "" ||
                                !this.numberingSystemList().includes(options.numberingSystem!))) ||
                (options.region != undefined && (options.region! == "" || !this.regionList().includes((options.region! as String).toUpperCase()))) ||
                (options.script != undefined && (options.script! == "" || !this.scriptList().includes(options.script!)))
            ) {
                throw new RangeError(this.errMsg)
            }
        }

        private parseTag(): boolean {
            if (this.tag == "") {
                throw new RangeError("First argument to Intl.Locale constructor can't be empty or missing")
            }

            // Check if we have a cached result for this tag
            let cached = Locale.getCachedLocale(this.tag)
            if (cached != undefined) {
                // Use cached values (avoids expensive validation and parsing)
                this.language = cached.language
                this.script = cached.script
                this.region = cached.region
                this.calendar = cached.calendar
                this.collation = cached.collation
                this.numberingSystem = cached.numberingSystem
                this.hourCycle = cached.hourCycle
                this.numeric = cached.numeric
                this.caseFirst = cached.caseFirst
                this.baseName = cached.baseName
                return true  // Cache hit
            }
            
            if (this.isTagValid(this.tag as string) == 1) {
                throw new RangeError(this.errMsg)
            }

            let argsStr : string = this.parseTagImpl(this.tag)
            let args: string[] = argsStr.split(" ")
            const getValue = (val: string): string | undefined => (val === "") ? undefined : val
            this.language = args[LocaleInfoKeys.LANG]
            this.script = getValue(args[LocaleInfoKeys.SCRIPT])
            this.region = getValue(args[LocaleInfoKeys.COUNTRY])
            this.calendar = getValue(args[LocaleInfoKeys.U_CA])
            this.collation = getValue(args[LocaleInfoKeys.U_CO])
            this.numberingSystem = getValue(args[LocaleInfoKeys.U_NU])
            this.hourCycle = getValue(args[LocaleInfoKeys.U_HC])
            this.numeric = (args[LocaleInfoKeys.U_KN] == "true") ? true : false
            this.caseFirst = getValue(args[LocaleInfoKeys.U_KF])

            if (this.tag.includes("-u")) {
               if (this.language != undefined && !this.langList().includes(this.language as string)) {
                    throw new RangeError("Incorrect locale information provided")
               }
            }
            this.fillBaseName()
            
            // Cache the parsed result to avoid redundant parsing for the same tag
            Locale.setCachedLocale(this.tag, new ParsedLocaleData(
                this.language,
                this.script,
                this.region,
                this.calendar,
                this.collation,
                this.numberingSystem,
                this.hourCycle,
                this.numeric,
                this.caseFirst,
                this.baseName
            ))
            
            return false  // Cache miss
        }

        private fillScriptOrRegion(val : string) {
            if (val.length <= 3) {
                this.region = val.toUpperCase()
            }
            else {
                this.script = val
            }
        }

        private static readonly calendarValues: ReadonlyArray<string> = Array.of<string> (
            "buddhist",
            "chinese",
            "coptic",
            "dangi",
            "ethioaa",
            "ethiopic",
            "gregory",
            "hebrew",
            "indian",
            "islamic",
            "islamic-umalqura",
            "islamic-tbla",
            "islamic-civil",
            "islamic-rgsa",
            "iso8601",
            "japanese",
            "persian",
            "roc",
            "islamicc"
        )

        private static readonly caseFirstValues: ReadonlyArray<string> = Array.of<string>("upper", "lower", "false")

        private static readonly hourCycleValues: ReadonlyArray<string> = Array.of<string>("h12", "h23", "h11", "h24")

        private static readonly collationValues: ReadonlyArray<string> = Array.of<string>(
            "big5han",
            "compat",
            "dict",
            "direct",
            "ducet",
            "emoji",
            "eor",
            "gb2312",
            "phonebk",
            "phonetic",
            "pinyin",
            "reformed",
            "search",
            "searchjl",
            "standard",
            "stroke",
            "trad",
            "unihan",
            "zhuyin"
        )
    }

    // Intl.NumberFormat

    export class NumberRangeFormatPart {
        type: NumberFormatPartTypes;
        value: string;
        source: "startRange" | "endRange" | "shared"

        public constructor(
            type: NumberFormatPartTypes,
            value: string,
            source: "startRange" | "endRange" | "shared")
        {
            this.type = type;
            this.value = value;
            this.source = source;
        }

    }

    export class NumberFormatPart {
        // No default value in TS/JS, because of this class
        //  is only used as returned result for
        //  function Intl.NumberFormat.formatToParts(..)
        type: NumberFormatPartTypes;
        value: string;

        public constructor(type: NumberFormatPartTypes, value: string)
        {
            this.type = type;
            this.value = value;
        }
    }

    export interface NumberFormatOptions {
        compactDisplay?: "short" | "long";
        currency?: string;
        currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name"
        currencySign?: "standard" | "accounting"
        localeMatcher?: "lookup" | "best fit"
        maximumFractionDigits?: number
        maximumSignificantDigits?: number
        minimumFractionDigits?: number
        minimumIntegerDigits?: number
        minimumSignificantDigits?: number
        notation?: "standard" | "scientific" | "engineering" | "compact"
        signDisplay?: "auto" | "never" | "always" | "exceptZero"
        style?: "decimal" | "percent" | "currency" | "unit"
        unit?: string
        unitDisplay?: "short" | "long" | "narrow"
        useGrouping?: boolean;
    }

    export interface ResolvedNumberFormatOptions {
        compactDisplay?: "short" | "long"
        currencySign?: "standard" | "accounting"
        currency?: string
        currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name"
        locale: Intl.BCP47LanguageTag;
        maximumFractionDigits: number
        maximumSignificantDigits?: number
        minimumFractionDigits: number
        minimumIntegerDigits: number
        minimumSignificantDigits?: number
        notation?: "standard" | "scientific" | "engineering" | "compact"
        numberingSystem: string
        signDisplay?: "auto" | "never" | "always" | "exceptZero"
        style: "decimal" | "percent" | "currency" | "unit"
        unit?: string
        unitDisplay?: "short" | "long" | "narrow"
        useGrouping: boolean
    }

    class ResolvedNumberFormatOptionsImpl implements ResolvedNumberFormatOptions {
        public minFracStr: string;
        public maxFracStr: string;
        public minSignStr: string;
        public maxSignStr: string;
        public minIntStr: string;
        public useGroupingStr: string;
        public isCompact: boolean;

        public _compactDisplay?: "short" | "long";
        public _currencySign?: "standard" | "accounting";
        public _currency?: string;
        public _currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name";
        public _locale: Intl.BCP47LanguageTag;
        public _maximumFractionDigits: number;
        public _maximumSignificantDigits?: number;
        public _minimumFractionDigits: number;
        public _minimumIntegerDigits: number;
        public _minimumSignificantDigits?: number;
        public _notation?: "standard" | "scientific" | "engineering" | "compact";
        public _numberingSystem: string;
        public _signDisplay?: "auto" | "never" | "always" | "exceptZero";
        public _style: "decimal" | "percent" | "currency" | "unit";
        public _unit?: string;
        public _unitDisplay?: "short" | "long" | "narrow";
        public _useGrouping: boolean;

        set compactDisplay(value: "short" | "long" | undefined) {
            this._compactDisplay = value;
        }

        get compactDisplay(): "short" | "long" | undefined {
            return this._compactDisplay;
        }

        set currencySign(value: "standard" | "accounting" | undefined) {
            this._currencySign = value;
        }

        get currencySign(): "standard" | "accounting" | undefined {
            return this._currencySign;
        }

        set currency(value: string | undefined) {
            this._currency = value;
        }

        get currency(): string | undefined {
            return this._currency;
        }

        set currencyDisplay(value: "code" | "symbol" | "narrowSymbol" | "name" | undefined) {
            this._currencyDisplay = value;
        }

        get currencyDisplay(): "code" | "symbol" | "narrowSymbol" | "name" | undefined {
            return this._currencyDisplay;
        }

        set locale(value: Intl.BCP47LanguageTag) {
            this._locale = value;
        }

        get locale(): Intl.BCP47LanguageTag {
            return this._locale;
        }

        set maximumFractionDigits(value: number) {
            this._maximumFractionDigits = value;
        }

        get maximumFractionDigits(): number {
            return this._maximumFractionDigits;
        }

        set maximumSignificantDigits(value: number | undefined) {
            this._maximumSignificantDigits = value;
        }

        get maximumSignificantDigits(): number | undefined {
            return this._maximumSignificantDigits;
        }

        set minimumFractionDigits(value: number) {
            this._minimumFractionDigits = value;
        }

        get minimumFractionDigits(): number {
            return this._minimumFractionDigits;
        }

        set minimumIntegerDigits(value: number) {
            this._minimumIntegerDigits = value;
        }

        get minimumIntegerDigits(): number {
            return this._minimumIntegerDigits;
        }

        set minimumSignificantDigits(value: number | undefined) {
            this._minimumSignificantDigits = value;
        }

        get minimumSignificantDigits(): number | undefined {
            return this._minimumSignificantDigits;
        }

        set notation(value: "standard" | "scientific" | "engineering" | "compact" | undefined) {
            this._notation = value;
        }

        get notation(): "standard" | "scientific" | "engineering" | "compact" | undefined {
            return this._notation;
        }

        set numberingSystem(value: string) {
            this._numberingSystem = value;
        }

        get numberingSystem(): string {
            return this._numberingSystem;
        }

        set signDisplay(value: "auto" | "never" | "always" | "exceptZero" | undefined) {
            this._signDisplay = value;
        }

        get signDisplay(): "auto" | "never" | "always" | "exceptZero" | undefined {
            return this._signDisplay;
        }

        set style(value: "decimal" | "percent" | "currency" | "unit") {
            this._style = value;
        }

        get style(): "decimal" | "percent" | "currency" | "unit" {
            return this._style;
        }

        set unit(value: string | undefined) {
            this._unit = value;
        }

        get unit(): string | undefined {
            return this._unit;
        }

        set unitDisplay(value: "short" | "long" | "narrow" | undefined) {
            this._unitDisplay = value;
        }

        get unitDisplay(): "short" | "long" | "narrow" | undefined {
            return this._unitDisplay;
        }

        set useGrouping(value: boolean) {
            this._useGrouping = value;
        }

        get useGrouping(): boolean {
            return this._useGrouping;
        }

        public toString(): string {
            return JSON.stringify(this)
        }

        public resolvedNumberFormatOptions(): ResolvedNumberFormatOptions {
            let newOptions: ResolvedNumberFormatOptions = {
                compactDisplay : this._compactDisplay,
                currencySign : this._currencySign,
                currency : this._currency,
                currencyDisplay : this._currencyDisplay,
                locale : this._locale,
                maximumFractionDigits : this._maximumFractionDigits,
                maximumSignificantDigits : this._maximumSignificantDigits,
                minimumFractionDigits : this._minimumFractionDigits,
                minimumIntegerDigits : this._minimumIntegerDigits,
                minimumSignificantDigits : this._minimumSignificantDigits,
                notation : this._notation,
                numberingSystem : this._numberingSystem,
                signDisplay : this._signDisplay,
                style : this._style,
                unit : this._unit,
                unitDisplay : this._unitDisplay,
                useGrouping : this._useGrouping
            }
            return newOptions;
        }
    }

    const Currencies: ReadonlyArray<string> = Array.of<string>("AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD",
        "AWG", "AZN", "BAM", "BBD", "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL", "BSD", "BTN", "BWP", "BYN",
        "BZD", "CAD", "CDF", "CHF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP", "CVE", "CZK", "DJF", "DKK", "DOP", "DZD",
        "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP", "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD", "HNL",
        "HRK", "HTG", "HUF", "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD", "JPY", "KES", "KGS", "KHR", "KMF",
        "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP", "LKR", "LRD", "LSL", "LYD", "MAD", "MDL", "MGA", "MKD", "MMK",
        "MNT", "MOP", "MRU", "MUR", "MVR", "MWK", "MXN", "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR", "NZD", "OMR",
        "PAB", "PEN", "PGK", "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF", "SAR", "SBD", "SCR", "SDG",
        "SEK", "SGD", "SHP", "SLL", "SOS", "SRD", "SSP", "STN", "SVC", "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP",
        "TRY", "TTD", "TWD", "TZS", "UAH", "UGX", "USD", "UYU", "UZS", "VES", "VND", "VUV", "WST", "XAF", "XCD", "XDR",
        "XOF", "XPF", "XSU", "YER", "ZAR", "ZMW", "ZWL")
    const RefCurrenciesMap = new Map<string, number>()
    Currencies.forEach((value, index) => { RefCurrenciesMap.set(value, index) })

    export class NumberFormat {

        /**
        * The Intl.NumberFormat() constructor creates Intl.NumberFormat objects.
        *
        * @param locales locale(s) from which to load formats and symbols for number formatting
        *
        * @param options options
        */
        public constructor(locales?: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[], options?: NumberFormatOptions) {
            // Set new options or default decimal style
            this.options = NumberFormat.assignOptionsDefault(options)

            if (locales == undefined) {
                if (options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            } else {
                let loc: Array<string>
                if (locales! instanceof string) {
                    loc = Array.of<string>(locales! as string)
                } else {
                    loc = Array.from<string>(locales! as string[])
                }
                if (loc.includes("") && options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            }

            let locMatcher = options?.localeMatcher ?? "best fit"
            this.options.locale = locMatcher == "lookup" ? intlLookUpLocale(locales ?? "") : intlBestFitLocale(locales ?? "")
            if (options != undefined) {
                const localOptions: Intl.NumberFormatOptions = options!; // It's used to reduce amount of calls options!
                NumberFormat.checkOptions(localOptions)
                if (localOptions.style != undefined) {
                    this.options.style = localOptions.style!

                    // Handle percent style
                    if (this.options.style == "percent") {
                        if (localOptions.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = 0
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 0
                        }
                        if(this.options.maximumFractionDigits < this.options.minimumFractionDigits) {
                            this.options.maximumFractionDigits = this.options.minimumFractionDigits
                        }
                    }

                    // Handle currency style
                    if (this.options.style == "currency") {
                        if (localOptions.currencySign == undefined) {
                            this.options.currencySign = "standard";
                        }
                        if (localOptions.currencyDisplay == undefined) {
                            this.options.currencyDisplay = "symbol";
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                           this.options.minimumFractionDigits = this.currencyDigits(localOptions.currency);
                        }
                        if (localOptions.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = this.currencyDigits(localOptions.currency);
                        }
                    }

                    // Handle unit style
                    if (this.options.style == "unit") {
                        if (localOptions.unitDisplay == undefined) {
                            this.options.unitDisplay = "short";
                        }
                    }
                }

                // Handle notation compact
                // roundingPriority is "auto" by default
                const minFrac = localOptions.minimumFractionDigits
                const maxFrac = localOptions.maximumFractionDigits
                const minSign = localOptions.minimumSignificantDigits
                const maxSign = localOptions.maximumSignificantDigits
                if (this.options.isCompact) {
                    if (minFrac == undefined && maxFrac == undefined && minSign == undefined && maxSign == undefined) {
                        this.options.minimumFractionDigits = 0
                        this.options.maximumFractionDigits = 0
                        this.options.minimumSignificantDigits = 1
                        this.options.maximumSignificantDigits = 2
                    } else if (minFrac != undefined || maxFrac != undefined) {
                        // FractionDigits options apply, other ignored
                        this.options.minimumSignificantDigits = undefined
                        this.options.maximumSignificantDigits = undefined
                    }
                } else {
                    if (minSign != undefined || maxSign != undefined) {
                        // SignificantDigits options apply, other ignored
                        this.options.minimumFractionDigits = 0
                        this.options.maximumFractionDigits = 0
                    } else {
                        // FractionDigits options apply, other ignored
                        this.options.minimumSignificantDigits = undefined
                        this.options.maximumSignificantDigits = undefined
                    }
                }

                // Additional handle useGrouping (begin)
                if (localOptions.useGrouping == undefined) {
                    this.options.useGroupingStr = this.options.isCompact ? "min2" : ""
                } else if (this.options.useGrouping) {
                    this.options.useGroupingStr = "true"
                } else if (!this.options.useGrouping) {
                    this.options.useGroupingStr = "false"
                }
            } else {
                // Additional handle useGrouping (end)
                this.options.useGroupingStr = ""
            }

            // Handle numberingSystem
            this.options.numberingSystem = NumberFormat.getNumberingSystem(this.options.locale)

            this.options.minFracStr = Number.toString(this.options.minimumFractionDigits)
            this.options.maxFracStr = Number.toString(this.options.maximumFractionDigits)
            this.options.minSignStr = this.options.minimumSignificantDigits != undefined ? Number.toString(this.options.minimumSignificantDigits!) : ""
            this.options.maxSignStr = this.options.maximumSignificantDigits != undefined ? Number.toString(this.options.maximumSignificantDigits!) : ""
            this.options.minIntStr = Number.toString(this.options.minimumIntegerDigits)
        }

        private static checkOptions(options: NumberFormatOptions) {
            if (options.currency != undefined) {
                if (!RefCurrenciesMap.has(options.currency!.toUpperCase())) {
                    throw new RangeError("Invalid currency")
                }
            }

            if (options.unit != undefined) {
                if (!NumberFormat.isUnitCorrect(options.unit!)) {
                    throw new RangeError("Invalid unit")
                }
            }

            if (options.style != undefined) {
                if ((options.style! == "unit") && (options.unit == undefined)) {
                    throw new RangeError("Style is unit but unit is undefined")
                }
                if ((options.style! == "currency") && (options.currency == undefined)) {
                    throw new RangeError("Style is currency but currency is undefined")
                }
            }

            if (options.maximumFractionDigits != undefined) {
                if (!(options.maximumFractionDigits! >= 0 && options.maximumFractionDigits! <= 100)) {
                    throw new RangeError("Invalid maximumFractionDigits")
                }
                if (options.minimumFractionDigits != undefined) {
                    if (options.maximumFractionDigits! < options.minimumFractionDigits!) {
                        throw new RangeError("Invalid maximumFractionDigits")
                    }
                }
            }
            if ((options.minimumFractionDigits != undefined)
                && !(options.minimumFractionDigits! >= 0 && options.minimumFractionDigits! <= 100)) {
                throw new RangeError("Invalid minimumFractionDigits")
            }

            if (options.maximumSignificantDigits != undefined) {
                if (!(options.maximumSignificantDigits! >= 1 && options.maximumSignificantDigits! <= 21)) {
                    throw new RangeError("Invalid maximumSignificantDigits")
                }
                if (options.minimumFractionDigits != undefined) {
                    if ((options.minimumSignificantDigits != undefined) && options.maximumSignificantDigits! < options.minimumSignificantDigits!) {
                        throw new RangeError("Invalid maximumSignificantDigits")
                    }
                }
            }
            if ((options.minimumSignificantDigits != undefined) && !(options.minimumSignificantDigits! >= 1 && options.minimumSignificantDigits! <= 21)) {
                throw new RangeError("Invalid minimumSignificantDigits")
            }

            if ((options.minimumIntegerDigits != undefined) && !(options.minimumIntegerDigits! >= 1 && options.minimumIntegerDigits! <= 21)) {
                throw new RangeError("Invalid minimumIntegerDigits")
            }

            if (options.notation == "compact" && options.compactDisplay == undefined) {
                throw new RangeError("Notaion is compact but compactDisplay is undefined")
            }
        }

        /**
        * Assign new options or default decimal style and values
        *
        * @param options options
        *
        * @returns boolean true means notation is "compact". Otherwise, false
        */
        private static assignOptionsDefault(options?: NumberFormatOptions): ResolvedNumberFormatOptionsImpl {
            let newOptions = new ResolvedNumberFormatOptionsImpl
            // Set new options or default decimal style and values
            newOptions._currencySign = options?.currencySign ?? undefined
            newOptions._currency = options?.currency ?? undefined
            newOptions._currencyDisplay = options?.currencyDisplay ?? undefined
            newOptions._locale = Locale.defaultTag()
            newOptions._maximumFractionDigits = options?.maximumFractionDigits ?? 3
            newOptions._minimumFractionDigits = options?.minimumFractionDigits ?? 0
            if (newOptions._maximumFractionDigits < newOptions._minimumFractionDigits) {
                newOptions._maximumFractionDigits = newOptions._minimumFractionDigits
            }
            newOptions._minimumIntegerDigits = options?.minimumIntegerDigits ?? 1
            newOptions._maximumSignificantDigits = options?.maximumSignificantDigits ?? undefined
            newOptions._minimumSignificantDigits = options?.minimumSignificantDigits ?? undefined
            if (newOptions._minimumSignificantDigits != undefined) {
                if (newOptions._maximumSignificantDigits == undefined) {
                    newOptions._maximumSignificantDigits = 21
                }
            }
            if (newOptions._maximumSignificantDigits != undefined) {
                if (newOptions._minimumSignificantDigits == undefined) {
                    newOptions._minimumSignificantDigits = 1
                }
            }
            newOptions._notation = options?.notation ?? "standard"
            if (newOptions._notation == "compact") {
                newOptions._compactDisplay = options?.compactDisplay ?? "short"
                newOptions.isCompact = true
            } else {
                newOptions.isCompact = false
            }
            newOptions._numberingSystem = "latn"
            newOptions._signDisplay = options?.signDisplay ?? "auto"
            newOptions._style = options?.style ?? "decimal"
            newOptions._unit = options?.unit ?? undefined
            newOptions._unitDisplay = options?.unitDisplay ?? undefined
            newOptions._useGrouping = (options == undefined) ? true : (options!.useGrouping ?? true)
            return newOptions
        }

        /**
        * Creates a new instance of an Intl.NumberFormat with the specified locale and options
        *
        * @param locales locale(s) from which to load formats and symbols for number formatting
        *
        * @param options options
        *
        * @returns A new Array instance with the specified length
        */
        public static $_invoke(locales?: string | string[], options?: NumberFormatOptions): NumberFormat {
            return new NumberFormat(locales, options)
        }

        /**
        * Formats a number according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param value A long to format.
        *
        * @returns string representing the given number formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public format(value: long): string {
            return this.formatLong(value);
        }

        /**
        * Formats a number according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param value A number to format.
        *
        * @returns string representing the given number formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public format(value: number): string {
            return this.formatDouble(value);
        }

        /**
        * Formats a number according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param value A number or BigInt or long to format
        *
        * @returns A string representing the given number formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public format(value: number | bigint | long): string {
            if (value instanceof BigInt) {
                return this.formatDecStr(value.toString());
            } else if (value instanceof Long) {
                return this.formatLong(value);
            }
            return this.formatDouble(value);
        }

        /**
        * Formats a range of numbers according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param start Start range to format a number or BigInt
        *
        * @param end End range to format a number or BigInt
        *
        * @throws TypeError if start or end is NaN
        *
        * @returns A string representing the given range of numbers formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public formatRange(start: number | bigint, end: number | bigint): string {
            let isStartBigInt: boolean = false;
            if (start instanceof BigInt) {
                isStartBigInt = true;
            } else if (isNaN(start as number)) {
                throw new TypeError("Invalid start: NaN");
            }
            let isEndBigInt: boolean = false;
            if (end instanceof BigInt) {
                isEndBigInt = true;
            } else if (isNaN(end as number)) {
                throw new TypeError("Invalid end: NaN");
            }

            if (!isStartBigInt && !isEndBigInt) {
                return this.formatRangeDoubleDouble(start as double, end as double);
            } else if (!isStartBigInt && isEndBigInt) {
                return this.formatRangeDoubleDecStr(start as double, (end as BigInt).toString());
            } else if (isStartBigInt && !isEndBigInt) {
                return this.formatRangeDecStrDouble((start as BigInt).toString(), end as double);
            }
            return this.formatRangeDecStrDecStr((start as BigInt).toString(), (end as BigInt).toString());
        }

        /**
        * Returns an Array of objects representing the range of numbers in parts that can be used for custom locale-aware formatting.
        *
        * @param start Start range to format a number or BigInt
        * @param end End range to format a number or BigInt
        *
        * @throws TypeError if start or end is NaN
        *
        * @returns An Array of objects containing the formatted range in parts
        */
        public formatRangeToParts(start: number | bigint, end: number | bigint): NumberRangeFormatPart[] {
            let isStartBigInt: boolean = false;
            if (start instanceof BigInt) {
                isStartBigInt = true;
            } else if (isNaN(start as number)) {
                throw new TypeError("Invalid start: NaN");
            }
            let isEndBigInt: boolean = false;
            if (end instanceof BigInt) {
                isEndBigInt = true;
            } else if (isNaN(end as number)) {
                throw new TypeError("Invalid end: NaN");
            }

            if (!isStartBigInt && !isEndBigInt) {
                return this.formatToRangePartsDoubleDouble(start as double, end as double);
            } else if (!isStartBigInt && isEndBigInt) {
                return this.formatToRangePartsDoubleDecStr(start as double, (end as BigInt).toString());
            } else if (isStartBigInt && !isEndBigInt) {
                return this.formatToRangePartsDecStrDouble((start as BigInt).toString(), end as double);
            }
            return this.formatToRangePartsDecStrDecStr((start as BigInt).toString(), (end as BigInt).toString());
        }

        /**
        * Returns an Array of objects representing the number string in parts that can be used for custom locale-aware formatting.
        *
        * @param value A number or BigInt to format
        *
        * @returns An Array of objects containing the formatted number in parts
        */
        public formatToParts(value: number | bigint): NumberFormatPart[] {
            if (value instanceof BigInt) {
                return this.formatToPartsDecStr(value.toString());
            }
            return this.formatToPartsDouble(value);
        }

        /**
        *
        * Returns resolved options
        *
        * @return resolved options
        */
        public resolvedOptions(): ResolvedNumberFormatOptions {
            return this.options.resolvedNumberFormatOptions()
        }

        /**
        * Returns supported locales
        *
        * @param locales locales
        *
        * @param options options
        *
        * @return supported locales
        */
        static supportedLocalesOf(locales: string | string[], options?: NumberFormatOptions): string[] {
            if (options != undefined) {
                NumberFormat.checkOptions(options!)
            }
            let locMatcher = options?.localeMatcher ?? "best fit"
            let loc: Array<string>
            if (locales! instanceof string)
                loc = Array.of<string>(locales! as string)
            else {
                loc = Array.from<string>(locales! as string[])
            }
            if (loc.includes("")) {
                throw new Error("Incorrect locale information provided")
            }
            return locMatcher == "best fit" ? intlBestFitLocales(loc) : intlLookUpLocales(loc)
        }

        private readonly options: ResolvedNumberFormatOptionsImpl

        private native static isUnitCorrect(unit: string): boolean;
        private native static getNumberingSystem(locale: string): string;
        private native formatDouble(value: double): string;
        private native formatLong(value: long): string;
        private native formatDecStr(value: string): string;
        private native formatRangeDoubleDouble(start: number, end: number): string;
        private native formatRangeDoubleDecStr(start: number, end: string): string;
        private native formatRangeDecStrDouble(start: string, end: number): string;
        private native formatRangeDecStrDecStr(start: string, end: string): string;
        private native formatToPartsDouble(value: double): NumberFormatPart[];
        private native formatToPartsDecStr(value: string): NumberFormatPart[];
        private native formatToRangePartsDoubleDouble(start: number, end: number): NumberRangeFormatPart[];
        private native formatToRangePartsDoubleDecStr(start: number, end: string): NumberRangeFormatPart[];
        private native formatToRangePartsDecStrDouble(start: string, end: number): NumberRangeFormatPart[];
        private native formatToRangePartsDecStrDecStr(start: string, end: string): NumberRangeFormatPart[];
        private native currencyDigits(currency: string | undefined): number;
    }

    // Intl.PluralRules

    export interface PluralRulesOptions {
        localeMatcher?: "lookup" | "best fit" | undefined
        type?: PluralRuleType | undefined
        minimumIntegerDigits?: number | undefined
        minimumFractionDigits?: number | undefined
        maximumFractionDigits?: number | undefined
        minimumSignificantDigits?: number | undefined
        maximumSignificantDigits?: number | undefined
    }

    export interface ResolvedPluralRulesOptions {
        locale: string
        localeMatcher: "lookup" | "best fit"
        type: PluralRuleType
        minimumIntegerDigits: number
        minimumFractionDigits: number
        maximumFractionDigits: number
        minimumSignificantDigits?: number
        maximumSignificantDigits?: number
        pluralCategories: FixedArray<LDMLPluralRule>
    }

    export interface SupportedLocalesOfOptions {
        localeMatcher?: "lookup" | "best fit" | undefined
    }

    export interface PluralRulesSelectOptions {
        locale: string
        type: string
        minimumIntegerDigits: number
        minimumFractionDigits: number
        maximumFractionDigits: number
        minimumSignificantDigits: number
        maximumSignificantDigits: number
    }

    class PluralRulesSelectOptionsImpl implements PluralRulesSelectOptions {
        set locale(val: string) { this.locale_ = val }
        get locale(): string { return this.locale_ }
        set type(val: string) { this.type_ = val}
        get type(): string { return this.type_ }
        set minimumIntegerDigits(val: number) { this.minimumIntegerDigits_ = val}
        get minimumIntegerDigits(): number { return this.minimumIntegerDigits_ }
        get minimumFractionDigits(): number { return this.minimumFractionDigits_ }
        set minimumFractionDigits(val: number) { this.minimumFractionDigits_ = val}
        get maximumFractionDigits(): number { return this.maximumFractionDigits_ }
        set maximumFractionDigits(val: number) { this.maximumFractionDigits_ = val}
        get minimumSignificantDigits(): number { return this.minimumSignificantDigits_ }
        set minimumSignificantDigits(val: number) { this.minimumSignificantDigits_ = val}
        get maximumSignificantDigits(): number { return this.maximumSignificantDigits_ }
        set maximumSignificantDigits(val: number) { this.maximumSignificantDigits_ = val}
        
        private locale_: string = ""
        private type_: string = ""
        private minimumIntegerDigits_: number = 0
        private minimumFractionDigits_: number = 0
        private maximumFractionDigits_: number = 0
        private minimumSignificantDigits_: number = 0
        private maximumSignificantDigits_: number = 0
    };

    export class PluralRules {
        private options : ResolvedPluralRulesOptions

        public constructor(locales?: string | FixedArray<string>, options?: PluralRulesOptions) {
            const localeMatcher = options?.localeMatcher ?? "best fit"
            const locale = this.resolveLocale(localeMatcher, locales)
            const type: PluralRuleType = options?.type ?? "cardinal"
            const categories = PluralRules.getPluralCategories(locale, type) as FixedArray<LDMLPluralRule>
            this.options = {
                locale: locale,
                localeMatcher: localeMatcher,
                minimumIntegerDigits: options?.minimumIntegerDigits ?? 1,
                minimumFractionDigits: options?.minimumFractionDigits ?? 0,
                maximumFractionDigits: options?.maximumFractionDigits ?? 3,
                minimumSignificantDigits: options?.minimumSignificantDigits,
                maximumSignificantDigits: options?.maximumSignificantDigits,
                pluralCategories: categories,
                type: type,
            } as ResolvedPluralRulesOptions
            if (this.options.maximumFractionDigits < this.options.minimumFractionDigits) {
                this.options.maximumFractionDigits = this.options.minimumFractionDigits
            }
        }

        private resolveLocale(localeMatcher: "lookup" | "best fit", locales?: string | FixedArray<string>): string {
            if (locales === undefined) {
                return Locale.defaultTag()
            }
            const arr = locales instanceof FixedArray<string> ? Array.from(locales) : [locales]
            return localeMatcher === "lookup" ? intlLookUpLocale(arr) : intlBestFitLocale(arr)
        }

        public resolvedOptions(): ResolvedPluralRulesOptions {
            return this.options
        }

        public select(value: number): LDMLPluralRule {
            const options = {
                locale: this.options.locale,
                type: this.options.type,
                minimumIntegerDigits: this.options.minimumIntegerDigits,
                minimumFractionDigits: this.options.minimumFractionDigits,
                maximumFractionDigits: this.options.maximumFractionDigits,
                minimumSignificantDigits: this.options.minimumSignificantDigits ?? 0,
                maximumSignificantDigits: this.options.maximumSignificantDigits ?? 0
            } as PluralRulesSelectOptionsImpl
            return PluralRules.selectDouble(value, options) as LDMLPluralRule
        }

        public static supportedLocalesOf(locales: string | FixedArray<string> | Array<string>, options?: SupportedLocalesOfOptions | PluralRulesOptions): string[] {
            const arr = PluralRules.toArray(locales)
            return options?.localeMatcher === "lookup" ? intlLookUpLocales(arr) : intlBestFitLocales(arr)
        }

        private static toArray(locales: string | FixedArray<string> | Array<string>): string[] {
            if (locales instanceof FixedArray<string>) {
                return Array.from(locales)
            } else if (locales instanceof string) {
                return [locales]
            } else {
                return locales
            }
        }

        private static native selectDouble(value: double, options: Object): string

        private static native getPluralCategories(locale: string, type: string): Object
    }

    // Intl.RelativeTimeFormat

    export interface ResolvedRelativeTimeFormatOptions {
        locale: string;
        numberingSystem: string;
        numeric: RelativeTimeFormatNumeric;
        style: RelativeTimeFormatStyle;
    }

    export interface RelativeTimeFormatOptions {
        numeric?: RelativeTimeFormatNumeric
        style?: RelativeTimeFormatStyle
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
    }

    class ResolvedRelativeTimeFormatOptionsImpl implements ResolvedRelativeTimeFormatOptions {
        private _locale: string;
        private _numberingSystem: string;
        private _numeric: RelativeTimeFormatNumeric;
        private _style: RelativeTimeFormatStyle;

        constructor(locale: string, numberingSystem: string) {
            this._locale = locale;
            this._numberingSystem = numberingSystem;
            this._numeric = 'always';
            this._style = 'long';
        }

        get locale(): string {
            return this._locale;
        }

        set locale(value: string) {
            this._locale = value;
        }

        get numberingSystem(): string {
            return this._numberingSystem;
        }

        set numberingSystem(value: string) {
            this._numberingSystem = value;
        }

        get numeric(): RelativeTimeFormatNumeric {
            return this._numeric;
        }

        set numeric(value: RelativeTimeFormatNumeric) {
            this._numeric = value;
        }

        get style(): RelativeTimeFormatStyle {
            return this._style;
        }

        set style(value: RelativeTimeFormatStyle) {
            this._style = value;
        }
    }

    export interface RelativeTimeFormatPart {
        type: string;
        unit?: RelativeTimeFormatUnit;
        value: string;
    }

    export class RelativeTimeFormat {
        private readonly locale: string | undefined
        private readonly options: ResolvedRelativeTimeFormatOptions

        constructor(locales?: string | string[], options?: RelativeTimeFormatOptions) {
            if (locales !== undefined) {
                const useLookup = options?.localeMatcher === 'lookup'
                this.locale = useLookup ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = undefined
            }
            this.options = new ResolvedRelativeTimeFormatOptionsImpl(this.getLocale(), "latn")
            this.options.numeric = options?.numeric ?? "always"
            this.options.style = options?.style ?? "long"
        }

        public static $_invoke(locales?: string | string[], options?: RelativeTimeFormatOptions): RelativeTimeFormat {
            return new RelativeTimeFormat(locales, options)
        }

        format(value: number, unit: RelativeTimeFormatUnit): string {
            return this.formatImpl(value, unit)
        }

        formatToParts(value: number, unit: RelativeTimeFormatUnit): RelativeTimeFormatPart[] {
            let val = this.formatImpl(value, unit)
            let part: RelativeTimeFormatPart = {type: 'literal', value: val}
            return [part]
        }

        resolvedOptions(): ResolvedRelativeTimeFormatOptions {
            return this.options
        }

        static supportedLocalesOf(locales: string | string[], options?: RelativeTimeFormatOptions): string[] {
            return RelativeTimeFormat.supportedLocalesOfImpl(
                (locales instanceof string) ? [locales] : locales, 
                options ?? { numeric: "always", style: "long" } as RelativeTimeFormatOptions
            )
        }

        private native formatImpl(value: number, unit: string): string
        private native getLocale(): string

        private static supportedLocalesOfImpl(locales: string[], options: RelativeTimeFormatOptions): string[] {
            return options.localeMatcher === "lookup" ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
        }
    }

    // Intl.Segmenter

    type SegmenterGranularity = 'grapheme' | 'word' | 'sentence';

    // used in Intl native methods
    class Cluster {
        /** The text content of the cluster */
        cluster: string;

        /** Indicates whether the cluster is word-like */
        isWordLike: boolean;

        /** Starts from index */
        index: int;

        /**
        * Default constructor initializing with empty values
        */
        public constructor() {
            this.cluster = "";
            this.isWordLike = false;
            this.index = 0;
        }
    }

    /**
    * Interface defining the structure of a text segment
    */
    export interface SegmentData {
        /** The index of the segment in the original text */
        index: number;

        /** The original input text */
        input: string;

        /** Indicates if the segment is word-like (optional) */
        isWordLike: boolean | undefined;

        /** The text content of the segment */
        segment: string;
    }

    /**
    * Implementation of SegmentData with private fields and pub getters/setters
    */
    class SegmentDataImpl implements SegmentData {
        // Private backing fields
        private _index: number = 0;
        private _input: string = "";
        private _isWordLike?: boolean;
        private _segment: string = "";

        /**
        * Constructor with optional parameters
        * @param index Segment index
        * @param input Original input text
        * @param segment Segment text
        * @param isWordLike Word-like status
        */
        constructor(index?: number, input?: string, segment?: string, isWordLike?: boolean) {
            this._index = index ?? 0;
            this._input = input ?? "";
            this._segment = segment ?? "";
            this._isWordLike = isWordLike;
        }

        // Getters and setters for each property
        public set index(i: number) { this._index = i; }
        public get index(): number { return this._index; }

        public set input(inp: string) { this._input = inp; }
        public get input(): string { return this._input; }

        public set isWordLike(iwl: boolean | undefined) { this._isWordLike = iwl; }
        public get isWordLike(): boolean | undefined { return this._isWordLike; }

        public set segment(seg: string) { this._segment = seg; }
        public get segment(): string { return this._segment; }
    }

    /**
    * Options for text segmentation
    */
    export interface SegmenterOptions {
        /** Locale matching strategy */
        localeMatcher?: 'best fit' | 'lookup';
        /** Type of segmentation to perform */
        granularity?: SegmenterGranularity;
    }

    interface PickLocaleMatchSegmenterOptions {
        localeMatcher?: 'best fit' | 'lookup'
    }

    /**
    * Fully resolved segmentation options
    */
    export interface ResolvedSegmenterOptions {
        locale: string;
        granularity: SegmenterGranularity;
    }

    /**
    * Segments iterator class
    */
    export class Segments implements IterableIterator<SegmentData> {
        /** Base parent segments array */
        private parent: SegmentDataImpl[]
        private idx: int = 0

        constructor(parent: SegmentDataImpl[]) {
            this.parent = parent
        }

        public override $_iterator(): IterableIterator<SegmentData>
        {
            return this
        }

        override next(): IteratorResult<SegmentData>
        {
            if (this.idx < 0 || this.idx >= this.parent.length) {
                return new IteratorResult<SegmentData>()
            }
            return new IteratorResult<SegmentData>(false, this.parent[this.idx++])
        }
    }

    /**
    * A specialized class for text segmentation across different linguistic granularities
    */
    export class Segmenter {
        /**
        * Native static methods for cluster generation across different segmentation types
        */
        private native static graphemeClusters(str: string, locale: string): Cluster[]
        private native static wordClusters(str: string, locale: string): Cluster[]
        private native static sentenceClusters(str: string, locale: string): Cluster[]

        private _locale: string;
        private _granularity: SegmenterGranularity;

        /**
        * Creates a new Segmenter instance with specified locales and options
        *
        * @param locales Optional locale or array of locales for segmentation
        * @param options Optional segmentation configuration
        *
        * @remarks
        * - Supports single or multiple locales
        * - Automatically resolves the best-fit locale
        * - Defaults to grapheme granularity if not specified
        */
        public constructor(
            locales?: BCP47LanguageTag | BCP47LanguageTag[],
            options?: SegmenterOptions
        ) {
            const picked = {localeMatcher: options?.localeMatcher}
                as PickLocaleMatchSegmenterOptions;
            const supported = Segmenter.supportedLocalesOf(locales ?? "", picked);
            this._locale = supported.length > 0 ? supported[0] : Locale.defaultTag();
            this._granularity = options?.granularity ?? 'grapheme';
        }

        /**
        * Segments a document into clusters based on the configured granularity
        *
        * @param doc The input text to segment
        * @returns A Segments iterator containing segmented data
        */
        public segment(doc: string): Segments {
            let result = this.segmentWith(doc, this._granularity)
            let segments = new SegmentDataImpl[result.length];

            for (let i = 0; i < result.length; ++i) {
                segments[i] = Segmenter.tryToSegmentData(result[i], this._granularity, doc);
            }

            return new Segments(segments);
        }

        /**
        * Internal method to perform segmentation based on granularity
        *
        * @param doc The input text to segment
        * @param granularity The type of segmentation to perform
        * @returns An array of Cluster representing the segments
        *
        * @privateRemarks
        * Defaults to grapheme segmentation if an unknown granularity is provided
        */
        private segmentWith(doc: string, granularity: 'grapheme' | 'word' | 'sentence'): Cluster[] {
            switch (granularity) {
            case "word":
                return Array.from<Cluster>(Segmenter.wordClusters(doc, this._locale))
            case "sentence":
                return Array.from<Cluster>(Segmenter.sentenceClusters(doc, this._locale))
            case "grapheme":
            default:
                return Array.from<Cluster>(Segmenter.graphemeClusters(doc, this._locale))
            }
        }

        /**
        * Retrieves the resolved options for the current Segmenter instance
        *
        * @returns The fully resolved segmentation options
        */
        public resolvedOptions(): ResolvedSegmenterOptions {
            return {
                locale: this._locale,
                granularity: this._granularity
            }
        }

        /**
        * Returns an array of supported locales for text segmentation
        *
        * @param locales An array of BCP 47 language tags to check
        * @param options Optional configuration for locale matching
        * @returns An array of supported locales from the input
        *
        * @remarks
        * - Validates which of the provided locales are supported by the Segmenter
        * - Uses the specified locale matching strategy
        * - Helps developers determine locale compatibility before creating a Segmenter instance
        */
        public static supportedLocalesOf(
            locales: BCP47LanguageTag | BCP47LanguageTag[],
            options?: PickLocaleMatchSegmenterOptions,
        ): BCP47LanguageTag[]
        {
            const matchingStrategy = options?.localeMatcher ?? 'best fit';
            try {
                return matchingStrategy == 'lookup'
                    ? intlLookUpLocales(locales) 
                    : intlBestFitLocales(locales)
            }
            catch(e) {
                if (e instanceof Error) throw e
                throw new RangeError(`Incorrect locales ${locales}`)
            }
        }

        /**
        * Converts an Cluster to a SegmentDataImpl
        * @param clusterData The cluster to convert
        * @param granularity The segmentation granularity
        * @param index The segment index
        * @param input The original input text
        * @returns A SegmentDataImpl instance
        */
        private static tryToSegmentData(
            clusterData: Cluster,
            granularity: SegmenterGranularity,
            input: string
        ): SegmentDataImpl
        {
            return new SegmentDataImpl(
                clusterData.index,
                input,
                clusterData.cluster,
                granularity == "word"
                            ? clusterData.isWordLike
                            : undefined
            )
        }
    }
}
