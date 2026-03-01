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

class RequestHeaders {
    Authorization: string;

    constructor(Authorization: string) {
        this.Authorization = Authorization;
    }
}

class User {
    id: number;
    name: string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

class ConsoleLogger {
    static logUserDetails(): void {
        console.group("User Details");
        console.log("Name: John");
        console.log("Age: 30");
        console.log("Email: john@example.com");
        console.groupEnd();
    }

    static logApplicationInfo(): void {
        console.group("Application Info");
        console.log("Version: 1.0.0");

        console.group("System Details");
        console.log("OS: Windows");
        console.log("Browser: Chrome");
        console.groupEnd();

        console.group("Performance");
        console.log("Memory: 4GB");
        console.log("CPU: 2.4GHz");
        console.groupEnd();
        console.groupEnd();
    }

    static logDebugInfo(): void {
        console.groupCollapsed("Debug Information");
        console.log("Debug Level: Verbose");
        console.log("Stack Trace: Enabled");
        console.log("Logging: Enabled");
        console.groupEnd();
    }

    static logApiCalls(): void {
        console.group("API Calls");
        console.log("GET /users");

        console.groupCollapsed("Request Details");
        const headers = new RequestHeaders("Bearer token");
        console.log("Headers:", headers);
        console.groupEnd();

        console.groupCollapsed("Response");
        console.log("Status: 200");
        console.log("Data:", [new User(1, "John")] as FixedArray<User>);
        console.groupEnd();
        console.groupEnd();
    }
}

function main() {
    // Basic group
    ConsoleLogger.logUserDetails();

    // Nested groups
    ConsoleLogger.logApplicationInfo();

    // Collapsed group
    ConsoleLogger.logDebugInfo();

    // Mix of normal and collapsed groups
    ConsoleLogger.logApiCalls();

    // Group
    console.log("outter");
    console.group();
    console.log("level 1");
    console.group("in level 1");
    console.log("level 2");

    // Reset for next test
    console.groupEnd()
    console.groupEnd()

    // Collapsed
    console.log("outter Collapsed");
    console.groupCollapsed();
    console.log("level 1 Collapsed");
    console.groupCollapsed("in level 1 Collapsed");
    console.log("level 2 Collapsed");

    // Reset for next test
    console.groupEnd()
    console.groupEnd()

    let num = 1
    let str = "str"
    console.groupCollapsed(num, str)
    console.groupCollapsed(str)
    console.groupCollapsed('count %d')
    console.groupCollapsed()
    console.groupCollapsed(num, str, "sdsf")
}
