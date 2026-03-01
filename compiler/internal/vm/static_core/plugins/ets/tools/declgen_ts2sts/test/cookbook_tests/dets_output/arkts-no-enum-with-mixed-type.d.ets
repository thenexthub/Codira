'use static'
/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
declare function enumReturnFunc(): Color;
declare function enumParmFunc(role: UserRole): boolean;
declare function enumXAsParam(param: Any): string;
declare function enumXAsReturn(): Any;
declare enum Color {
    Red = 0,
    Green = 1,
    Blue = 2
}
declare enum UserRole {
    Admin = "admin",
    Editor = "editor",
    Viewer = "viewer",
    Guest = "guest"
}
export declare const X: Any;
declare enum T {
    a = 1,
    b = 2,
    c = 2
}
declare enum Z {
    a = -1,
    b = -2,
    c = 0
}
