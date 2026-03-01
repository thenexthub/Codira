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
type ErrorCode = number;

enum EventType {
    DefaultEvent
}

enum ListenerStatusNumeric {
    on,
    off
}
 
enum ListenerStatusString {
    on = "ON",
    off = "OFF"
}

interface TestListener {
    "onStart"?: () => void;
    "onDestroy"?: () => void;
    onError?: (code: ErrorCode, msg: string) => void;
    onTouch?: () => void;
    onEvent?: (e: EventType) => void;
}

interface MyListener {
    on(key: string, cb: (r: Record<string, string>) => void);
    off(key: string, cb: (r: Record<string, string>, t:number) => void);
}
 
interface MyListener2 {
    on(key: string, cb: (r: ListenerStatusNumeric) => void);
}
 
interface MyListener3 {
    on(key: string, cb: (r: ListenerStatusString) => void);
}
 
class MyListener4 {
    static on(key: string, cb: (r: ListenerStatusString) => void);
}
 
interface MyListener5 {
    on(key: string, cb: Callback<number>)
}
 
export function on(key: string, cb: (r: Record<string, string>, option, t?:number) => void);
 
export function off(key: string, cb?: (r: Record<string, string>, t:number) => void);
