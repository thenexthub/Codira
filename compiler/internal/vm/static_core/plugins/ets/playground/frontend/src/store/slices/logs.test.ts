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

import codeReducer, {
    setRunLoading,
    setCompileLoading,
    setCode,
    setCompileRes,
    setRunRes
} from './code';
import { ICodeReq } from '../../models/code';

describe('codeSlice', () => {
    const initialState = {
        isRunLoading: false,
        isCompileLoading: false,
        isShareLoading: false,
        code: '',
        compileRes: null,
        runRes: null,
        verifierRes: null
    };

    it('should update isRunLoading state when setRunLoading is called', () => {
        const action = setRunLoading(true);
        const state = codeReducer(initialState, action);
        expect(state.isRunLoading).toBe(true);
    });

    it('should update isCompileLoading state when setCompileLoading is called', () => {
        const action = setCompileLoading(true);
        const state = codeReducer(initialState, action);
        expect(state.isCompileLoading).toBe(true);
    });

    it('should update code state when setCode is called', () => {
        const action = setCode('new code');
        const state = codeReducer(initialState, action);
        expect(state.code).toBe('new code');
    });

    it('should update compileRes state when setCompileRes is called', () => {
        const compileRes: ICodeReq = {
            compile: { output: 'Compile output', error: '', exit_code: 0 },
            disassembly: { output: '', code: '', error: '', exit_code: 0 },
            verifier: { output: '', error: '', exit_code: 0 }
        };
        const action = setCompileRes(compileRes);
        const state = codeReducer(initialState, action);
        expect(state.compileRes).toEqual(compileRes);
    });

    it('should update runRes state when setRunRes is called', () => {
        const runRes: ICodeReq = {
            compile: { output: 'Compile output', error: '', exit_code: 0 },
            disassembly: { output: '', code: '', error: '', exit_code: 0 },
            verifier: { output: '', error: '', exit_code: 0 }
        };
        const action = setRunRes(runRes);
        const state = codeReducer(initialState, action);
        expect(state.runRes).toEqual(runRes);
    });
});
