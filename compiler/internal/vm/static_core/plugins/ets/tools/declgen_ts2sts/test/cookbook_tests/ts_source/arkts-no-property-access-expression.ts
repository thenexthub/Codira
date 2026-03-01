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

export namespace hh{
    export declare enum Support {
        COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
        COMMON_EVENT_SHUTDOWN = "SHUTDOWN",
    }
    export namespace hhh {
        export declare enum Support {
            COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
            COMMON_EVENT_SHUTDOWN = "SHUTDOWN",
        }
        export namespace hhhh {
            export declare enum Support {
                COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
                COMMON_EVENT_SHUTDOWN = "SHUTDOWN",
            }
    
        }
    }
}

export enum Support1 {
    COMMON_EVENT_BOOT_COMPLETED = "BOOT_COMPLETED",
    COMMON_EVENT_SHUTDOWN = "SHUTDOWN",
}

export class MyClass {
    static aa="hello";
}

export class Test {
    static bb: hh.Support;
    static readonly cc = hh.Support.COMMON_EVENT_SHUTDOWN;
    static readonly ee = Support1.COMMON_EVENT_SHUTDOWN;
    static readonly ee1 =hh.hhh.Support.COMMON_EVENT_SHUTDOWN;
    static readonly ee2 =hh.hhh.hhhh.Support.COMMON_EVENT_SHUTDOWN;
    static readonly dd = "hello";
    static readonly gg=MyClass.aa;
    static ii=MyClass.aa;
    static ff: number;
}
