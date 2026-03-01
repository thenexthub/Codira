'use static'
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
export declare namespace hh {
    export namespace hhh {
        export namespace hhhh {
            export enum Support {
                COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
                COMMON_EVENT_SHUTDOWN = "SHUTDOWN"
            }
        }
        export enum Support {
            COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
            COMMON_EVENT_SHUTDOWN = "SHUTDOWN"
        }
    }
    export enum Support {
        COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
        COMMON_EVENT_SHUTDOWN = "SHUTDOWN"
    }
}
export declare enum Support1 {
    COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
    COMMON_EVENT_SHUTDOWN = "SHUTDOWN"
}
export declare class MyClass {
    static aa: string;
}
export declare class Test {
    static bb: hh.Support;
    static readonly cc: hh.Support;
    static readonly ee: Support1;
    static readonly ee1: hh.hhh.Support;
    static readonly ee2: hh.hhh.hhhh.Support;
    static readonly dd: string;
    static readonly gg: string;
    static ii: string;
    static ff: number;
}
