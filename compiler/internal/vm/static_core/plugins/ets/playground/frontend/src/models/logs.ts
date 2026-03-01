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

import { createAsyncThunk } from '@reduxjs/toolkit';
import {
    setErrLogs,
    setHighlightErrors,
    setOutLogs
} from '../store/slices/logs';
import { RootState } from '../store';
import { IAstReq } from './ast';

export enum ELogType {
    COMPILE_OUT = 'compileOut',
    COMPILE_ERR = 'compileErr',
    RUN_OUT = 'runOut',
    RUN_ERR = 'runErr',
    RUN_AOT_OUT = 'runAotOut',
    RUN_AOT_ERR = 'runAotErr',
    DISASM_OUT = 'disasmOut',
    DISASM_ERR = 'disasmErr',
    VERIFIER_OUT = 'verifierOut',
    VERIFIER_ERR = 'verifierErr',
    AST_OUT = 'ASTOut',
    AST_ERR = 'ASTErr',
}

export interface ILog {
    message: { message: string, line?: number, column?: number, source?: 'output' | 'error' }[];
    isRead?: boolean;
    from?: ELogType;
    exit_code?: number;
}

export interface ICompileData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IRunData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IDisassemblyData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IVerifierData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IApiResponse {
    data: {
        compile?: ICompileData;
        run?: IRunData;
        run_aot?: IRunData;
        disassembly?: IDisassemblyData;
        verifier?: IVerifierData;
    };
}

type HighlightItem = { message: string; line?: number; column?: number; type?: string; source?: 'output' | 'error' };

const collectHighlights = (str?: string, source?: 'output' | 'error'): HighlightItem[] => {
    if (!str) {
        return [];
    }

    // keep in sync with DiagnosticType in ets_frontend/ets2panda/util/diagnostic/diagnostic.h.erb
    const diag_types = [
        'ArkTS config error',
        'Error',
        'Fatal',
        'Semantic error',
        'SUGGESTION',
        'Syntax error',
        'Warning',
    ];

    // emulate Python's rf strings to make regex more readable
    const r = String.raw;
    const diag_type = r`(?:${diag_types.join('|')})`;
    const loc = r`\[(?<file>[^\[\]]+?):(?<line>\d+):(?<col>\d+)\]`;
    const shortid = r`(?:[A-Za-z]+\d+)`

    const re_str = r`(?:${loc}\s*)(?<type>${diag_type})\s*(?<shortid>${shortid})\s*:\s*(?<msg>.*?)(?<newline>\n|$)`;
    const re = new RegExp(re_str, 'g');

    const out: HighlightItem[] = [];

    const pushPlainSegments = (text: string): void => {
        const parts = text.split('\n')
        for (const p of parts) {
            if (p === '') {
                continue;
            }
            out.push({ message: p, source });
        }
    };

    let last = 0;
    for (const m of str.matchAll(re)) {
        const start = m.index ?? 0;
        const full = m[0] ?? '';
        const end = start + full.length;

        if (start > last) {
            pushPlainSegments(str.slice(last, start));
        }
        const { type, msg, file, line, col, shortid, newline } = (m.groups ?? {}) as {
            type?: string; msg?: string; file?: string; line?: string; col?: string; shortid?: string; newline?: string;
        };

        out.push({
            message: `${type} ${shortid ?? ''}: ${(msg ?? '').trim()} [${file ?? ''}:${line ?? ''}:${col ?? ''}]${newline ?? ''}`,
            line: line ? Number(line) : undefined,
            column: col ? Number(col) : undefined,
            type,
            source,
        });
        last = end;
    }

    if (last < str.length) {
        pushPlainSegments(str.slice(last));
    }
    return out;
};

export const handleResponseLogs = createAsyncThunk(
    'logs/compileLogs',
    async (response: IApiResponse, thunkAPI) => {
        const handleLog = (
            data: ICompileData | IRunData | IDisassemblyData | IAstReq | undefined,
            logTypeOut: ELogType,
            logTypeErr: ELogType,
            successMessage: string
        ): void => {
            const state: RootState = thunkAPI.getState() as RootState;
            const logsState = state.logs;

            if (!data) {
                return;
            }

            if (data.output) {
                const outputMessages = collectHighlights(data.output, 'output');
                if (outputMessages.length > 0) {
                    thunkAPI.dispatch(setOutLogs(
                        logsState.out.concat({
                            message: outputMessages,
                            isRead: false,
                            from: logTypeOut,
                            exit_code: data.exit_code
                        })
                    ));
                }
            }

            if (data.error) {
                const errorMessages = collectHighlights(data.error, 'error');
                if (errorMessages.length > 0) {
                    thunkAPI.dispatch(setErrLogs(
                        logsState.err.concat({
                            message: errorMessages,
                            isRead: false,
                            from: logTypeErr,
                            exit_code: data.exit_code
                        })
                    ));
                }
            }

            if (data.exit_code === 0 && !data.error && !data.output && successMessage) {
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: [{ message: successMessage, source: 'output' }],
                        isRead: false,
                        from: logTypeOut,
                        exit_code: data.exit_code
                    })
                ));
            }

            const outHighlight = collectHighlights(data.output, 'output').filter(h => h.line !== undefined);
            const errHighlight = collectHighlights(data.error, 'error').filter(h => h.line !== undefined);
            thunkAPI.dispatch(setHighlightErrors([...outHighlight, ...errHighlight]));
        };

        handleLog(
            response.data.compile,
            ELogType.COMPILE_OUT,
            ELogType.COMPILE_ERR,
            'Compile successful!'
        );

        handleLog(
            response.data.run,
            ELogType.RUN_OUT,
            ELogType.RUN_ERR,
            'Run successful!'
        );

        handleLog(
            response.data.run_aot,
            ELogType.RUN_AOT_OUT,
            ELogType.RUN_AOT_ERR,
            'AOT Run successful!'
        );

        handleLog(
            response.data.disassembly,
            ELogType.DISASM_OUT,
            ELogType.DISASM_ERR,
            'Disassembly successful!'
        );

        if (response.data.verifier?.error || response.data.verifier?.output) {
            handleLog(
                response.data.verifier,
                ELogType.VERIFIER_OUT,
                ELogType.VERIFIER_ERR,
                ''
            );
        }
    }
);

export const handleASTLogs = createAsyncThunk(
    'logs/compileLogs',
    async (data: IAstReq, thunkAPI) => {
            const state: RootState = thunkAPI.getState() as RootState;
            const logsState = state.logs;
            if (!data) {
                return;
            }
            if (data.error) {
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: collectHighlights(data.error, 'error'),
                        isRead: false,
                        from: ELogType.AST_ERR,
                        exit_code: data.exit_code
                    })
                ));
            }
    }
);