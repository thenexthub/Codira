/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

class C {
    public a = 1
    b = 2
    static c = ""
    public static d = "1"

    protected e = new Map()
    protected static f = new Map<string, string>()

    private g = 1
    private static h = ""

    #i = 1

    public j() {
        return 1
    }
    k(): number {
        return 2
    }
    static l() {
        return ""
    }
    public static m(): string {
        return "1"
    }

    protected o() {
        return new Map();
    }
    protected static p() {
        return new Map<string, string>()
    }

    private q() {
        return 1
    }
    private static r() {
        return ""
    }

    #s() {
        return
    }
}

class Singleton {
    private static instance: Singleton | null = null;

    private constructor() {}

    public static getInstance(): Singleton {
        if (Singleton.instance === null) {
            Singleton.instance = new Singleton();
        }
        return Singleton.instance;
    }

    public someMethod(): void {
        console.log("This is a method of the Singleton class.");
    }

    private test(){
        console.log("test");
    }
}
