/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export declare const globalVar : import('typescript').SyntaxKind;
export declare function func1(): import('some').Type;
export declare function func2(param: import('foo').Foo): void;
export declare function func3(param1: string, param2: import('typescript').SyntaxKind): import('typescript').SourceFile;
export declare interface A {
    f1(): import('typescript').SourceFile;
    f2(sourceFile: import('typescript').SyntaxKind): string;
    f3(sourceFile: import('typescript').SyntaxKind): import('typescript').SyntaxKind;
}
export declare class B {
    f1(a: string): import('typescript').SourceFile;
    f2(sourceFile: import('typescript').SourceFile): string;
    f3(sourceFile: import('typescript').SyntaxKind): import('typescript').SyntaxKind;
}