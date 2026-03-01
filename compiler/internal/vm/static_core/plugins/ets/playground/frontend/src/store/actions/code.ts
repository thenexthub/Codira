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
import {setCode, setCompileLoading, setCompileRes, setRunLoading, setRunRes, setShareLoading} from '../slices/code';
import {codeService} from '../../services/code';
import {handleResponseLogs} from '../../models/logs';
import { RootState } from '..';
import { clearErrLogs, clearOutLogs } from './logs';
import { setOptionsPicked } from '../slices/options';
import { setClearHighLightErrs } from '../slices/logs';
import { encodeShareData, copyToClipboard, buildShareUrl } from '../../utils/shareUtils';
import { showMessage } from './notification';

export const fetchCompileCode = createAsyncThunk(
    '@code/compileCode',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setCompileLoading(true));
        thunkAPI.dispatch(setCompileRes(null));
        thunkAPI.dispatch(setRunRes(null));
        const state: RootState = thunkAPI.getState() as RootState;
        const codeState = state.code || '';
        const appState = state?.appState || false;
        const optionsState = state.options || {};
        thunkAPI.dispatch(setClearHighLightErrs());
        if (appState.clearLogsEachRun) {
            thunkAPI.dispatch(clearOutLogs());
            thunkAPI.dispatch(clearErrLogs());
        }
        const response = await codeService.fetchCompile({
            code: codeState?.code,
            options: optionsState?.pickedOptions,
            disassemble: appState.disasm,
            verifier: appState.verifier,
            ir_dump: (appState.irDump.compilerDump || appState.irDump.disasmDump) ? {
                compiler_dump: appState.irDump.compilerDump,
                disasm_dump: appState.irDump.disasmDump,
            } : null,
        });
        if (response.error) {
            console.error(response.error);
            thunkAPI.dispatch(setCompileLoading(false));
            return;
        }
        thunkAPI.dispatch(handleResponseLogs(response));
        thunkAPI.dispatch(setCompileRes(response.data));
        thunkAPI.dispatch(setCompileLoading(false));
    },
);
export const fetchRunCode = createAsyncThunk(
    '@code/runCode',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setRunLoading(true));
        thunkAPI.dispatch(setCompileRes(null));
        thunkAPI.dispatch(setRunRes(null));
        const state: RootState = thunkAPI.getState() as RootState;
        const codeState = state.code || '';
        const appState = state?.appState || false;
        const optionsState = state?.options || {};
        thunkAPI.dispatch(setClearHighLightErrs());
        if (appState.clearLogsEachRun) {
            thunkAPI.dispatch(clearOutLogs());
            thunkAPI.dispatch(clearErrLogs());
        }
        const response = await codeService.fetchRun({
            code: codeState?.code,
            options: optionsState?.pickedOptions,
            disassemble: appState.disasm,
            verifier: appState.verifier,
            verification_mode: appState.verificationMode,
            aot_mode: appState.aotMode,
            ir_dump: (appState.irDump.compilerDump || appState.irDump.disasmDump) ? {
                compiler_dump: appState.irDump.compilerDump,
                disasm_dump: appState.irDump.disasmDump,
            } : null,
        });
        if (response.error) {
            console.error(response.error);
            thunkAPI.dispatch(setRunLoading(false));
            return;
        }
        thunkAPI.dispatch(handleResponseLogs(response));
        thunkAPI.dispatch(setRunRes(response.data));
        thunkAPI.dispatch(setRunLoading(false));
    },
);

export const setCodeAction = createAsyncThunk(
    '@code/setCode',
    (data: string, thunkAPI) => {
        thunkAPI.dispatch(setCode(data));
    },
);

export const flushPendingCodeUpdate = (): void => {
    window.dispatchEvent(new Event('flushPendingCodeUpdate'));
};

export const fetchShareCode = createAsyncThunk(
    '@code/shareCode',
    async (_, thunkAPI): Promise<string> => {
        const state: RootState = thunkAPI.getState() as RootState;
        const codeState = state.code || '';
        const optionsState = state.options || {};

        const response = await codeService.shareCode({
            code: codeState.code,
            options: optionsState.pickedOptions,
        });

        if (response.error) {
            console.error(response.error);
            return '';
        }

        const shareUrl = `${window.location.origin}/${response.data.uuid}`;
        return shareUrl;
    },
);

export const fetchCodeByUuid = createAsyncThunk(
    '@code/fetchCodeByUuid',
    async (uuid: string, thunkAPI) => {
        const response = await codeService.fetchShareCode(uuid);

        if (response.error) {
            console.error(response.error);
            return;
        }

        thunkAPI.dispatch(setCode(response.data.code));

        if (response.data.options) {
            thunkAPI.dispatch(setOptionsPicked(response.data.options));
        }
    },
);

export const shareCodeLocally = createAsyncThunk(
    '@code/shareCodeLocally',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setShareLoading(true));

        try {
            const state: RootState = thunkAPI.getState() as RootState;
            const codeState = state.code || '';
            const optionsState = state.options || {};
            const encoded = encodeShareData(codeState.code, optionsState.pickedOptions);
            const shareUrl = buildShareUrl(encoded);
            window.history.pushState({}, '', shareUrl);
            const copied = await copyToClipboard(shareUrl);

            if (copied) {
                thunkAPI.dispatch(showMessage({
                    type: 'success',
                    title: 'Share',
                    message: 'Link copied to clipboard!',
                    duration: 3000
                }));
            } else {
                thunkAPI.dispatch(showMessage({
                    type: 'info',
                    title: 'Share',
                    message: 'URL updated. Please copy it manually from the address bar.',
                    duration: 5000
                }));
            }

            thunkAPI.dispatch(setShareLoading(false));
        } catch (error) {
            console.error('Error sharing code:', error);

            thunkAPI.dispatch(showMessage({
                type: 'error',
                title: 'Share Error',
                message: 'Failed to create share link. Please try again.',
                duration: 5000
            }));

            thunkAPI.dispatch(setShareLoading(false));
        }
    },
);