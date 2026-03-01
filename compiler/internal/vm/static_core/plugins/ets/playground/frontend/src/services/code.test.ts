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

import { codeService, EActionsEndpoints } from './code';
import { fetchPostEntity } from './common';
import { codeModel, ICodeFetch } from '../models/code';

jest.mock('./common', () => ({
    fetchPostEntity: jest.fn(),
}));

describe('codeService', () => {
    const mockPayload: ICodeFetch = {
        code: 'some code',
        disassemble: true,
        verifier: true,
        options: { flag: true },
    };

    afterEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchCompile', () => {
        it('should call fetchPostEntity with correct arguments and transform response using codeModel.fromApiCompile', async () => {
            const mockResponse = {
                data: {
                    compile: { output: 'Compiled output', error: '', exit_code: 0 },
                    disassembly: { output: '', code: '', error: '', exit_code: 0 }
                },
            };

            (fetchPostEntity as jest.Mock).mockResolvedValue(mockResponse);

            const result = await codeService.fetchCompile(mockPayload);

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                EActionsEndpoints.COMPILE,
                null,
                codeModel.fromApiCompile
            );
            expect(result).toEqual(mockResponse);
        });
    });

    describe('fetchRun', () => {
        it('should call fetchPostEntity with correct arguments and transform response using codeModel.fromApiRun', async () => {
            const mockResponse = {
                data: {
                    compile: { output: 'Compiled output', error: '', exit_code: 0 },
                    disassembly: { output: '', code: '', error: '', exit_code: 0 },
                    run: { output: 'Run output', error: '', exit_code: 0 },
                },
            };

            (fetchPostEntity as jest.Mock).mockResolvedValue(mockResponse);

            const result = await codeService.fetchRun(mockPayload);

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                EActionsEndpoints.RUN,
                null,
                codeModel.fromApiRun
            );
            expect(result).toEqual(mockResponse);
        });
    });
});
