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

export const asmSyntax = {
    // Instructions, registers, operators, and keywords
    instructions: [
        'add', 'sub', 'mul', 'div', 'mov', 'jmp', 'call', 'ret', 'push', 'pop',
        'cmp', 'jne', 'je', 'jg', 'jl', 'jge', 'jle', 'inc', 'dec', 'nop'
    ],
    registers: ['r0', 'r1', 'r2', 'r3', 'sp', 'fp', 'pc'],
    operators: ['=', '>', '<', '!', '~', '?', ':', '==', '<=', '>=', '!=', '&&', '||', '+', '-', '*', '/', '&', '|', '^', '%'],

    // Data types
    types: [
        'u1', 'u8', 'i8', 'u16', 'i16', 'u32', 'i32', 'i64', 'f32', 'f64', 'any'
    ],

    // Keywords for structures and functions
    keywords: ['.record', '.function', '.ctor'],

    symbols: /[=><!~?:&|+\-*\/\^%]+/,

    tokenizer: {
        root: [
            // Structure and function definitions
            [/\.(record|function|ctor)\b/, 'keyword'],

            // Metadata for functions and records
            [/<.*?>/, 'annotation'],

            // Data types
            [/\b(u1|u8|i8|u16|i16|u32|i32|i64|f32|f64|any)\b/, 'type'],

            // Instructions and keywords
            [/[a-z_$][\w$]*/, { cases: { '@instructions': 'keyword', '@types': 'type', '@default': 'identifier' } }],

            // Registers
            [/\b(r[0-9]+|sp|fp|pc)\b/, 'variable.predefined'],

            // Labels
            [/^[a-zA-Z_]\w*:/, 'type.identifier'],

            // Operators and delimiters
            [/@symbols/, { cases: { '@operators': 'operator', '@default': '' } }],
            [/[{}()\[\]]/, '@brackets'],
            [/[;,.]/, 'delimiter'],

            // Numeric values
            [/#-?\d+/, 'number'],
            [/#0x[0-9a-fA-F]+/, 'number.hex'],

            // Comments
            [/;.*/, 'comment'],

            // Strings
            [/"([^"\\]|\\.)*$/, 'string.invalid'], // non-terminated string
            [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
        ],

        string: [
            [/[^\\"]+/, 'string'],
            [/\\./, 'string.escape'],
            [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
        ],

        whitespace: [
            [/[ \t\r\n]+/, 'white'],
            [/;.*/, 'comment'],
        ]
    }
};
