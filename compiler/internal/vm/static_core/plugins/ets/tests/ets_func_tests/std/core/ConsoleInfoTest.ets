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
class Config {
    port: number;
    environment: string;
    database: string;
    cache: string;

    constructor(port: number, environment: string, database: string, cache: string) {
        this.port = port;
        this.environment = environment;
        this.database = database;
        this.cache = cache;
    }

    toString(): string {
        return `Config(port=${this.port}, env=${this.environment}, db=${this.database}, cache=${this.cache})`;
    }
}

class ApplicationLogger {
    private static features: FixedArray<string> = [
        "authentication",
        "authorization",
        "caching",
        "logging"
    ];

    static logStartup(): void {
        console.info("Application started");
        console.info("Version 1.0.0");
    }

    static logServerStatus(port: number): void {
        console.info("Server status:", "Running", "Port:", port);
    }

    static logConfiguration(config: Config): void {
        console.info("Server configuration:", config);
    }

    static logActiveFeatures(): void {
        console.info("Active features:", ApplicationLogger.features);
    }

    static logSystemInformation(): void {
        console.info(
            "%c System Information",
            "color: green;"
        );
    }

}

function main() {
    // Basic info messages
    ApplicationLogger.logStartup();

    // Info with multiple arguments
    ApplicationLogger.logServerStatus(3000);

    // Info with objects
    const serverConfig = new Config(
        3000,
        "production",
        "mongodb",
        "redis"
    );
    ApplicationLogger.logConfiguration(serverConfig);

    // Info with arrays
    ApplicationLogger.logActiveFeatures();

    // Info with styled output
    ApplicationLogger.logSystemInformation();

    console.info()

    // Formatted strings
    console.info("%d")
    console.info("%d", 1)
    console.info("%d", 1, 2)

    console.info("%s %s", "37", 37)
    console.info("%s %s", "19.5", 19.5)
}
