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

import { selectCompileLoading, selectRunLoading, selectCode, selectCompileRes, selectRunRes } from './code';
import { RootState } from '..';
import { ICodeReq } from '../../models/code';
import {mockAllState} from './appState.test';

describe('Code Selectors', () => {
    let mockState: RootState;

    beforeEach(() => {
        mockState = mockAllState;
    });

    it('should select the compile loading state from code', () => {
        expect(selectCompileLoading(mockState)).toBe(false);
        mockState.code.isCompileLoading = true;
        expect(selectCompileLoading(mockState)).toBe(true);
    });

    it('should select the run loading state from code', () => {
        expect(selectRunLoading(mockState)).toBe(false);
        mockState.code.isRunLoading = true;
        expect(selectRunLoading(mockState)).toBe(true);
    });

    it('should select the current code from code state', () => {
        expect(selectCode(mockState)).toBe('initial code');
        mockState.code.code = 'updated code';
        expect(selectCode(mockState)).toBe('updated code');
    });

    it('should select the compile result from code state', () => {
        expect(selectCompileRes(mockState)).toBeNull();
        const compileRes: ICodeReq = {
            compile: {
                output: 'compile output',
                error: '',
                exit_code: 0,
            },
            disassembly: {
                output: '',
                code: '',
                error: '',
                exit_code: 0,
            },
            verifier: {
                output: '',
                error: '',
                exit_code: 0,
            }
        };
        mockState.code.compileRes = compileRes;
        expect(selectCompileRes(mockState)).toEqual(compileRes);
    });

    it('should select the run result from code state', () => {
        expect(selectRunRes(mockState)).toBeNull();
        const runRes: ICodeReq = {
            compile: {
                output: 'compile output',
                error: '',
                exit_code: 0,
            },
            disassembly: {
                output: '',
                code: '',
                error: '',
                exit_code: 0,
            },
            verifier: {
                output: '',
                error: '',
                exit_code: 0,
            }
        };
        mockState.code.runRes = runRes;
        expect(selectRunRes(mockState)).toEqual(runRes);
    });
});
