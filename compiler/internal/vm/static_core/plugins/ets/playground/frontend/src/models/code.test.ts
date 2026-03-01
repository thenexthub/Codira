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

import { codeModel } from './code';
import { ICodeReq, IRunReq } from './code';

describe('codeModel', () => {
    describe('fromApiCompile', () => {
        it('should correctly map compile and disassembly fields from API response', () => {
            const mockData: ICodeReq = {
                compile: { output: 'compile output', error: 'compile error', exit_code: 1 },
                disassembly: { output: 'disasm output', code: 'disasm code', error: 'disasm error', exit_code: 2 },
                verifier: { output: 'verifier output', error: 'verifier error', exit_code: 2 },
            };

            const result = codeModel.fromApiCompile(mockData);

            expect(result).toEqual({
                compile: { output: 'compile output', error: 'compile error', exit_code: 1 },
                disassembly: { output: 'disasm output', code: 'disasm code', error: 'disasm error', exit_code: 2 },
                verifier: { output: 'verifier output', error: 'verifier error', exit_code: 2 },
            });
        });

        it('should use default values when fields are missing in API response', () => {
            const mockData = {} as ICodeReq; // Empty data to test defaults
            const result = codeModel.fromApiCompile(mockData);

            expect(result).toEqual({
                compile: { output: '', error: '', exit_code: undefined },
                disassembly: { output: '', code: '', error: '', exit_code: undefined },
                verifier: { output: '', error: '', exit_code: undefined },
            });
        });
    });

    describe('fromApiRun', () => {
        it('should correctly map compile, disassembly, and run fields from API response', () => {
            const mockData: IRunReq = {
                compile: { output: 'compile output', error: 'compile error', exit_code: 1 },
                disassembly: { output: 'disasm output', code: 'disasm code', error: 'disasm error', exit_code: 2 },
                run: { output: 'run output', error: 'run error', exit_code: 3 },
                verifier: { output: '', error: '', exit_code: 0 }
            };

            const result = codeModel.fromApiRun(mockData);

            expect(result).toEqual({
                compile: { output: 'compile output', error: 'compile error', exit_code: 1 },
                disassembly: { output: 'disasm output', code: 'disasm code', error: 'disasm error', exit_code: 2 },
                run: { output: 'run output', error: 'run error', exit_code: 3 },
                verifier: { output: '', error: '', exit_code: 0 }
            });
        });

        it('should use default values when fields are missing in API response', () => {
            const mockData = {} as IRunReq; // Empty data to test defaults
            const result = codeModel.fromApiRun(mockData);

            expect(result).toEqual({
                compile: { output: '', error: '', exit_code: undefined },
                disassembly: { output: '', code: '', error: '', exit_code: undefined },
                run: { output: '', error: '', exit_code: undefined },
                verifier: { output: '', error: '', exit_code: undefined }
            });
        });
    });
});
