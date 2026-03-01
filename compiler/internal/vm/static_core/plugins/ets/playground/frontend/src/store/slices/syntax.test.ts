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

import syntaxReducer, {
    setSyntaxLoading,
    setSyntaxResponse,
} from './syntax';
import { ISyntax } from '../../models/syntax';

describe('syntaxSlice', () => {
    const initialState = {
        isLoading: false,
        syntax: {
            tokenizer: {
                root: [],
            }
        }
    };

    const mockSyntax: ISyntax = {
        tokenizer: {
            root: [
                [/\b(class|const|let|var|function)\b/, 'keyword'],
            ],
        },
    };

    it('should handle setSyntaxLoading action', () => {
        const action = setSyntaxLoading(true);
        const state = syntaxReducer(initialState, action);
        expect(state.isLoading).toBe(true);
    });

    it('should handle setSyntaxResponse action', () => {
        const action = setSyntaxResponse(mockSyntax);
        const state = syntaxReducer(initialState, action);
        expect(state.syntax).toEqual(mockSyntax);
    });
});