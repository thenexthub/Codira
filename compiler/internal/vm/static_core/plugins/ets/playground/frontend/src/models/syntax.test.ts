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

import { syntaxModel, ISyntax } from './syntax';

describe('syntaxModel', () => {
    describe('fromApi', () => {
        it('should convert regex strings to RegExp objects', () => {
            const apiData: ISyntax = {
                tokenizer: {
                    root: [
                        ['/(test)/', 'identifier'],
                        ['not a regex', 'string'],
                    ]
                }
            };

            const result = syntaxModel.fromApi(apiData);

            expect(result.tokenizer.root[0][0]).toEqual(/(test)/);
            expect(result.tokenizer.root[1][0]).toBe('not a regex');
        });

        it('should handle deeply nested regex strings', () => {
            const apiData: ISyntax = {
                tokenizer: {
                    root: [
                        ['/deepRegex/', 'keyword'],
                        ['regular string', 'identifier']
                    ]
                }
            };

            const result = syntaxModel.fromApi(apiData);

            expect(result.tokenizer.root[0][0]).toEqual(/deepRegex/);
            expect(result.tokenizer.root[1][0]).toBe('regular string');
        });

        it('should return an identical structure if no regex strings are present', () => {
            const apiData: ISyntax = {
                tokenizer: {
                    root: [
                        ['no regex here', 'tokenType'],
                        ['another string', 'anotherType']
                    ]
                }
            };

            const result = syntaxModel.fromApi(apiData);

            expect(result).toEqual(apiData);
        });

        it('should ignore invalid regex format strings', () => {
            const apiData: ISyntax = {
                tokenizer: {
                    root: [
                        ['/unclosed regex', 'tokenType']
                    ]
                }
            };

            const result = syntaxModel.fromApi(apiData);

            expect(result.tokenizer.root[0][0]).toBe('/unclosed regex');
        });
    });
});
