/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import configureMockStore from 'redux-mock-store';
import { handleResponseLogs, IApiResponse } from './logs';
import { RootState } from '../store';
import { thunk } from 'redux-thunk';

const middlewares = [thunk as never];
const mockStore = configureMockStore<RootState>(middlewares);

interface MockAction {
    type: string;
    payload?: {
        message: Array<{
            message: string;
            line?: number;
            column?: number;
            source?: 'output' | 'error';
        }>;
    }[];
}

describe('handleResponseLogs thunk', () => {
    let store: ReturnType<typeof mockStore>;
    const initialState: RootState = {
        appState: {
            theme: 'light',
            primaryColor: '#e32b49',
            disasm: false,
            verifier: true,
            astView: false,
            verificationMode: 'disabled',
            aotMode: false,
            versions: {},
            versionsLoading: false,
            clearLogsEachRun: false,
            activeLogTab: 'compilation',
            irDump: {
                compilerDump: false,
                disasmDump: false,
            },
        },
        options: {
            isLoading: false,
            options: [],
            pickedOptions: {},
        },
        syntax: {
            isLoading: false,
            syntax: { tokenizer: { root: [] } },
        },
        code: {
            isRunLoading: false,
            isCompileLoading: false,
            isShareLoading: false,
            code: 'initial code',
            compileRes: null,
            runRes: null,
            verifierRes: null
        },
        logs: {
            err: [],
            out: [],
            highlightErrors: [],
            jumpTo: null
        },
        ast: {
            isAstLoading: false,
            astRes: null
        },
        features: { astMode: 'manual' },
        notifications: { messages: []}
    };

    beforeEach(() => {
        store = mockStore(initialState);
        jest.clearAllMocks();
    });

    it('dispatches compile error log correctly', async () => {
        const apiResponse: IApiResponse = {
            data: {
                compile: {
                    exit_code: 1,
                    error: 'Compile error occurred'
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions();

        expect(actions.length).toBeGreaterThan(0);
    });

    it('preserves newlines in error messages', async () => {
        const errorWithNewlines = '[test.ets:1:5] Error ESE0123: First error\n[test.ets:2:10] Error ESE0456: Second error\nSome output';

        const apiResponse: IApiResponse = {
            data: {
                compile: {
                    exit_code: 1,
                    error: errorWithNewlines
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions() as MockAction[];

        // Find setErrLogs action
        const setErrLogsAction = actions.find(action => action.type === 'logsState/setErrLogs');
        expect(setErrLogsAction).toBeDefined();

        if (!setErrLogsAction?.payload) {
            throw new Error('Payload is undefined');
        }

        const logMessages = setErrLogsAction.payload[0].message;

        // Check that we have multiple log entries (parsed by newlines)
        expect(logMessages.length).toBeGreaterThan(1);

        // Check that error messages contain newlines where expected
        const firstError = logMessages.find(m => m.message.includes('ESE0123'));
        expect(firstError).toBeDefined();
        expect(firstError?.message).toMatch(/\n$/); // Should end with newline

        const secondError = logMessages.find(m => m.message.includes('ESE0456'));
        expect(secondError).toBeDefined();
        expect(secondError?.message).toMatch(/\n$/); // Should end with newline
    });

    it('handles multiline output correctly', async () => {
        const multilineOutput = 'Line 1\nLine 2\nLine 3';

        const apiResponse: IApiResponse = {
            data: {
                run: {
                    exit_code: 0,
                    output: multilineOutput
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions() as MockAction[];

        const setOutLogsAction = actions.find(action => action.type === 'logsState/setOutLogs');
        expect(setOutLogsAction).toBeDefined();

        if (!setOutLogsAction?.payload) {
            throw new Error('Payload is undefined');
        }

        const logMessages = setOutLogsAction.payload[0].message;

        // Should split into 3 separate messages
        expect(logMessages.length).toBe(3);
        expect(logMessages[0].message).toBe('Line 1');
        expect(logMessages[1].message).toBe('Line 2');
        expect(logMessages[2].message).toBe('Line 3');
    });

    it('preserves newline at end of formatted error', async () => {
        const formattedError = '[test.ets:5:3] Syntax error ESY0229: Unexpected token\n';

        const apiResponse: IApiResponse = {
            data: {
                compile: {
                    exit_code: 1,
                    error: formattedError
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions() as MockAction[];

        const setErrLogsAction = actions.find(action => action.type === 'logsState/setErrLogs');

        if (!setErrLogsAction?.payload) {
            throw new Error('Payload is undefined');
        }

        const logMessages = setErrLogsAction.payload[0].message;

        // The formatted error should preserve the newline
        expect(logMessages[0].message).toMatch(/\n$/);
    });

    it('handles errors without newlines correctly', async () => {
        const errorNoNewline = '[test.ets:10:1] Error ESE0001: Missing semicolon';

        const apiResponse: IApiResponse = {
            data: {
                compile: {
                    exit_code: 1,
                    error: errorNoNewline
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions() as MockAction[];

        const setErrLogsAction = actions.find(action => action.type === 'logsState/setErrLogs');

        if (!setErrLogsAction?.payload) {
            throw new Error('Payload is undefined');
        }

        const logMessages = setErrLogsAction.payload[0].message;

        // Should still parse correctly, just without trailing newline
        expect(logMessages[0].message).toContain('ESE0001');
        expect(logMessages[0].line).toBe(10);
        expect(logMessages[0].column).toBe(1);
    });

    it('does not lose empty lines between errors', async () => {
        const errorWithEmptyLine = '[test.ets:1:1] Error ESE0123: First\n\n[test.ets:3:1] Error ESE0456: Second';

        const apiResponse: IApiResponse = {
            data: {
                compile: {
                    exit_code: 1,
                    error: errorWithEmptyLine
                }
            }
        };

        await store.dispatch(handleResponseLogs(apiResponse) as never);
        const actions = store.getActions() as MockAction[];

        const setErrLogsAction = actions.find(action => action.type === 'logsState/setErrLogs');

        if (!setErrLogsAction?.payload) {
            throw new Error('Payload is undefined');
        }

        const logMessages = setErrLogsAction.payload[0].message;

        const errorMessages = logMessages.filter(m => m.line !== undefined);
        expect(errorMessages.length).toBe(2);
    });
});