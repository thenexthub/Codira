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

import {
    selectOutLogs,
    selectErrLogs,
    selectHighlightErrors,
    selectCompilationLogs,
    selectRuntimeLogs
} from './logs';
import { RootState } from '..';
import {mockAllState} from './appState.test';
import { ELogType } from '../../models/logs';

describe('Log Selectors', () => {
    let mockState: RootState;

    beforeEach(() => {
        mockState = {...mockAllState,
            logs: {
                out: [
                    { message: [{message: 'compile output', source: 'output' as const}], isRead: true, from: ELogType.COMPILE_OUT },
                    { message: [{message: 'run output', source: 'output' as const}], isRead: true, from: ELogType.RUN_OUT }
                ],
                err: [
                    { message: [{message: 'compile error', source: 'error' as const}], isRead: false, from: ELogType.COMPILE_ERR },
                    { message: [{message: 'run error', source: 'error' as const}], isRead: false, from: ELogType.RUN_ERR }
                ],
                highlightErrors: [],
                jumpTo: null
            },
        };
    });

    it('should select general output logs', () => {
        expect(selectOutLogs(mockState)).toHaveLength(2);
        expect(selectOutLogs(mockState)[0].message[0].message).toBe('compile output');
    });

    it('should select general error logs', () => {
        expect(selectErrLogs(mockState)).toHaveLength(2);
        expect(selectErrLogs(mockState)[0].message[0].message).toBe('compile error');
    });

    it('should select compilation logs (compile out + err)', () => {
        const compilationLogs = selectCompilationLogs(mockState);
        expect(compilationLogs).toHaveLength(2);
        expect(compilationLogs.some(log => log.from === ELogType.COMPILE_OUT)).toBe(true);
        expect(compilationLogs.some(log => log.from === ELogType.COMPILE_ERR)).toBe(true);
    });

    it('should select runtime logs (run out + err)', () => {
        const runtimeLogs = selectRuntimeLogs(mockState);
        expect(runtimeLogs).toHaveLength(2);
        expect(runtimeLogs.some(log => log.from === ELogType.RUN_OUT)).toBe(true);
        expect(runtimeLogs.some(log => log.from === ELogType.RUN_ERR)).toBe(true);
    });

    it('should select highlight errors', () => {
        mockState.logs.highlightErrors = [
            { line: 1, column: 2, message: 'Syntax error' },
        ];
        expect(selectHighlightErrors(mockState)).toEqual([
            { line: 1, column: 2, message: 'Syntax error' },
        ]);
    });
});
