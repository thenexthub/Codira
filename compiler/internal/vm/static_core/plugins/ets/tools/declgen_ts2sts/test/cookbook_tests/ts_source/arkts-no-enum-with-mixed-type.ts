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

enum Color {
    Red,
    Green,
    Blue
}
enum UserRole {
    Admin = 'admin',
    Editor = 'editor',
    Viewer = 'viewer',
    Guest = 'guest'
}
enum X {
    A = 1,
    b = '2'
}
enum T {
    a = 1,
    b = 1 + 1,
    c = T.b
}
enum Z {
    a = -1,
    b = -2,
    c = 0
}
function enumReturnFunc(): Color {
    return Color.Red;
}
function enumParmFunc(role: UserRole): boolean {
    return role == 'admin';
}

function enumXAsParam(param: X): string {
    if (param === X.A) {
        return 'The value is X.A';
    } else if (param === X.b) {
        return 'The value is X.b';
    }
    return 'Unknown value';
}

function enumXAsReturn(): X {
    return X.A;
}