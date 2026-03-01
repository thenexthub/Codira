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

package std.testing;

export namespace arktest {
    /**
    * Produce AssertionError if condition is false. Print an additinal message in this case
    * @param condition the checking condition
    * @param message Error message for false condition
    * @param description optional additional description for producing Error
    * @throws AssertionError if condition is false
    */
    function assertCommon(condition: boolean, message: string, description?: string): void {
        if (!condition) {
            failingAssertion(message, description)
        }
    }

    /**
    * Produce AssertionError unconditionally
    * @param message Error message
    * @param description optional Error description
    * @throws AssertionError unconditionally
    */
    function failingAssertion(message: string, description?: string): never {
        throw new AssertionError((description ? description + "\n" : "") + message)
    }

    /**
    * Assert that the supplied condition is true
    * @param condition supplied condition
    * @param comment optional comment that will be printed in case of failed assertion
    * @throws AssertionFailed if condition is false
    */
    export function assertTrue(condition: boolean, comment?: string): void {
        assertCommon(condition, "expected true but was false", comment)
    }

    /**
    * Assert that the supplied condition is false
    * @param condition supplied condition
    * @param comment optional comment that will be printed in case of failed assertion
    * @throws AssertionFailed if condition is true
    */
    export function assertFalse(condition: boolean, comment?: string): void {
        assertCommon(!condition, "expected false but was true", comment)
    }

    /**
    * Check two values for == and produce throw error if check failed
    * @param value1 first value for equality
    * @param value2 second value for equality
    * @param comment optional comment will be printed if equality failed
    * @throws AssertionError if equality failed
    */
    export function assertEQ<T1, T2>(value1: T1, value2: T2, comment?: string): void {
        assertCommon(value1 === value2, "expected equality failed: '" + value1 + "' === '" + value2 + "'", comment);
    }

    /**
    * Check two values for != and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertNE<T1, T2>(value1: T1, value2: T2, comment?: string): void {
        assertCommon(value1 !== value2, "expected non-equality failed: '" + value1 + "' !== '" + value2 + "'", comment);
    }

    /**
    * Check that the difference between two values of double type does not exceed the absolute error
    * @param value1 first value to check
    * @param value2 second value to check
    * @param absError absolute error
    * @param comment optional comment will be printed if equality failed
    * @throws AssertionError if the difference exceed the absolute error
    */
    export function assertDoubleEQ(value1: double, value2: double, absError: double, comment?: string): void {
        let difference = Math.abs(value1 - value2);
        assertCommon(difference <= absError, "The difference between '" + value1 + "' and '" + value2 +
                                            "' is '" + difference + "', but expected absolute error = '" + absError + "'",
                                            comment);
    }

    /**
    * Check two values of byte type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: byte, value2: byte, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of short type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: short, value2: short, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of int type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: int, value2: int, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of long type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: long, value2: long, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of float type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: float, value2: float, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of double type for < and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLT(value1: double, value2: double, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Check two values of byte type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: byte, value2: byte, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Check two values of short type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: short, value2: short, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Check two values of int type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: int, value2: int, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Check two values of long type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: long, value2: long, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Check two values of float type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: float, value2: float, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Check two values of double type for <= and produce throw error if check failed
    * @param value1 first value for non-equality
    * @param value2 second value for non-equality
    * @param comment optional comment will be printed if non-equality failed
    * @throws AssertionError if non-equality failed
    */
    export function assertLE(value1: double, value2: double, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Assert callback throws and test the thrown value
    * @param cb callback to be executed
    * @param test for a thrown value, acts as predicate or returns an error message
    * @throws AssertionError if no value thrown or value doesn't match the test predicate
    */
    export function expectThrow(
        cb: () => void,
        test?: (v: Error) => boolean | string) {
        try {
            cb()
        } catch (e) {
            e = (e instanceof Error) ? e as Error
                : failingAssertion("An arbitrary object instance was caught!")
            let res = test?.(e)
            if (res != undefined && res != true) {
                failingAssertion((typeof res == "string") ? res as string : ("Unexpected value thrown: " + e))
            }
            return
        }
        failingAssertion("Callback expected to throw");
    }

    /**
    * Execute callback and check that Error was occurred and it is the same as expected if it passed
    * @param fn executing callback
    * @param message optional string message or Error to be tested upon the thrown value
    * @throws AssertionError if an error was not occured or occured error is not same as expected error
    */
    export function expectError(fn: () => void, expect?: string | Error): void {
        expectThrow(fn, (e) => {
            if (!(e instanceof Error)) {
                return "Expected an instance of Error, got " + Class.of(e).getName()
            }
            if (expect === undefined) {
                return true
            } else if (typeof expect == "string") {
                return e.message == expect
            } else {
                const expectedError = expect as Error // broken smartcast
                const eType = Class.of(e)
                const expectType = Class.of(expectedError)
                if (eType !== expectType) {
                    return "Expected " + expectType.getName() + ", got " + eType.getName()
                }
                if (e.message != expectedError.message) {
                    return "Expected '" + expectedError.message + "', got '" + e.message + "'"
                }
                return true
            }
        })
    }

    /**
    * Execute callback and check that no Error occurred
    * @param fn executing callback
    * @throws AssertionError if an error occured
    */
    export function expectNoThrow(fn: () => void): void {
        try {
            fn();
        } catch (e) {
            assertCommon(false, "Error is not expected, but has ocurred: " + e);
        }
    }

    class TermColor {
        static readonly BLACK = new TermColor(0)
        static readonly RED = new TermColor(1)
        static readonly GREEN = new TermColor(2)

        readonly code: int

        private constructor(code: int) {
            this.code = code
        }

        toString(): string {
            return "" + this.code
        }
    }

    /**
    * @class ArkTest provides interface for running a test
    */
    export class ArkTest {
        /**
        * Create a test instance
        * @param name test name
        * @param callback callback with test body
        */
        constructor(name: string, callback: () => void) {
            this.name = name;
            this.callback = callback;
        }

        /**
        * Run the test
        * @returns true if test passed, false - otherwise
        */
        final run(): boolean {
            let testcaseName: string = this.getFullName();
            let startMs = Date.now();
            try {
                this.callback();
            } catch (e) {
                const failedMsg = this.addTermColor(TermColor.RED, "FAILED")
                const msgFail = "[ " + failedMsg + " ] " + testcaseName + " (" + (Date.now() - startMs) + " ms)";
                if (!(e instanceof AssertionError)) {
                    console.error("Unexpected error: " + e);
                } else {
                    // AssertionError already print message in constructor console.error,
                    // so set message to empty to avoid double printing
                    e.message = "";
                }
                if (e instanceof Error) {
                    if (e.stack) {
                        console.error(e.stack);
                    }
                }
                console.log(msgFail);
                return false;
            }
            const passedMsg = this.addTermColor(TermColor.GREEN, "PASSED")
            console.log("[ " + passedMsg + " ] " + testcaseName + " (" + (Date.now() - startMs) + " ms)");
            return true;
        }

        private addTermColor(color: TermColor, str: string): string {
            return "\x1b[38;5;" + color + "m" + str + "\x1b[0m"
        }

        /**
        * @returns Full test name (including testsuite)
        */
        final getFullName(): string {
            let name: string = "";
            if (this.suite) {
                name += this.suite!.name + ".";
            }
            return name + this.name;
        }

        public setSuite(suite: ArkTestsuite): void {
            this.suite = suite;
        }

        /// Name of the test
        readonly name: string;

        private readonly callback: () => void;
        private suite?: ArkTestsuite;
    }

    /**
    * @class ArkTestsuite provides interface for running a group of tests
    */
    export class ArkTestsuite {

        /**
        * Create a testsuite instance
        * @param name name of testsuite
        */
        constructor(name: string) {
            this.name = name;
            this.tests = new Array<ArkTest>();
        }

        /**
        * Run before each test. Can be overrided in derived testusute class
        */
        before(): void {}

        /**
        * Run after each test. Can be overrided in derived testusute class
        */
        after(): void {}

        /**
        * Add new test to the testsuite
        * @param name name of test
        * @param callback callback with test body
        */
        final addTest(name: string, callback: () => void): void {
            this.addTest(new ArkTest(name, callback));
        }

        /**
        * Add new async test to the testsuite
        * @param name name of async test
        * @param asyncCallback callback with async test body
        */
        final addAsyncTest(name: string, asyncCallback: () => Promise<void>): void {
            let callback = () => {
                waitForCompletion(asyncCallback);
            };
            this.addTest(new ArkTest(name, callback));
        }

        /**
        * Add new test to the testsuite
        * @param test test instance
        */
        final addTest(test: ArkTest): void {
            if (test.name == "") {
                throw new Error("Test with empty name is not allowed");
            }
            test.setSuite(this);
            this.tests.push(test);
        }

        /**
        * Run all tests in the testsuite
        * @returns count of failed tests
        */
        final run(): int {
            let startMs = Date.now();
            this.tests.forEach((test: ArkTest) => {
                this.before();
                if (test.run()) {
                    this.passedTests += 1;
                } else {
                    this.failedTests += 1;
                }
                this.after();
            });
            let execTimeMs: string = "(" + (Date.now() - startMs) + " ms)";
            let failedTestsCount = this.failedTests;
            if (failedTestsCount != 0) {
                console.log("Passed tests: " + this.passedTests);
                console.log("Failed tests: " + this.failedTests);
                console.log(this.name + " testsuite failed " + execTimeMs);
            } else {
                let testStr: string = this.passedTests + ((this.passedTests == 1) ? " test" : " tests");
                console.log(testStr + " from " +  this.name + " testsuite passed " + execTimeMs);
            }
            this.failedTests = 0;
            this.passedTests = 0;
            return failedTestsCount;
        }

        /// name of the testsuite
        readonly name: string;

        private tests: Array<ArkTest>;
        private passedTests: int = 0;
        private failedTests: int = 0;
    }

}
