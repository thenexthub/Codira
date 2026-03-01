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

import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { VerificationMode } from '../../models/code';

export type Theme = 'light' | 'dark';
export type LogTab = 'compilation' | 'runtime';

export interface IrDumpOptions {
    compilerDump: boolean;
    disasmDump: boolean;
}

interface IState {
    theme: Theme;
    primaryColor: string;
    disasm: boolean;
    verifier: boolean;
    astView: boolean;
    versions: Versions;
    versionsLoading: boolean;
    clearLogsEachRun: boolean;
    verificationMode: VerificationMode;
    activeLogTab: LogTab;
    aotMode: boolean;
    irDump: IrDumpOptions;
}

export interface Versions {
    frontend?: string;
    backendVersion?: string;
    arktsVersion?: string;
    es2pandaVersion?: string;
}

const initialState: IState = {
    theme: (localStorage.getItem('theme') as Theme) || (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light'),
    disasm: false,
    verifier: true,
    verificationMode: 'ahead-of-time',
    aotMode: false,
    astView: false,
    primaryColor: '#e32b49',
    versions: {
        frontend: '',
        backendVersion: '',
        arktsVersion: '',
        es2pandaVersion: ''
    },
    versionsLoading: false,
    clearLogsEachRun: localStorage.getItem('clearLogsEachRun') === 'false' ? false : true,
    activeLogTab: 'compilation',
    irDump: {
        compilerDump: false,
        disasmDump: false,
    },
};

const appStateSlice = createSlice({
    name: 'appState',
    initialState,
    reducers: {
        setTheme: (state, action: PayloadAction<'light' | 'dark'>) => {
            state.theme = action.payload;
        },
        setPrimaryColor: (state, action: PayloadAction<string>) => {
            state.primaryColor = action.payload;
        },
        setDisasm: (state, action: PayloadAction<boolean>) => {
            state.disasm = action.payload;
        },
        setAstView: (state, action: PayloadAction<boolean>) => {
            state.astView = action.payload;
        },
        setVerifier: (state, action: PayloadAction<boolean>) => {
            state.verifier = action.payload;
        },
        setVerificationMode: (state, action: PayloadAction<VerificationMode>) => {
            state.verificationMode = action.payload;
        },
        setAotMode: (state, action: PayloadAction<boolean>) => {
            state.aotMode = action.payload;
        },
        setVersions(state, action: PayloadAction<Versions>) {
            state.versions = action.payload;
        },
        setVersionsLoading(state, action: PayloadAction<boolean>) {
            state.versionsLoading = action.payload;
        },
        setClearLogsEachRun(state, action: PayloadAction<boolean>) {
            state.clearLogsEachRun = action.payload;
            localStorage.setItem('clearLogsEachRun', String(action.payload));
        },
        setActiveLogTab(state, action: PayloadAction<LogTab>) {
            state.activeLogTab = action.payload;
        },
        setIrDump(state, action: PayloadAction<IrDumpOptions>) {
            state.irDump = action.payload;
        },
        setIrDumpCompilerDump(state, action: PayloadAction<boolean>) {
            state.irDump.compilerDump = action.payload;
        },
        setIrDumpDisasmDump(state, action: PayloadAction<boolean>) {
            state.irDump.disasmDump = action.payload;
        },
    },
});

export const {
    setTheme,
    setDisasm,
    setVerifier,
    setAstView,
    setPrimaryColor,
    setVersions,
    setVersionsLoading,
    setClearLogsEachRun,
    setVerificationMode,
    setActiveLogTab,
    setAotMode,
    setIrDump,
    setIrDumpCompilerDump,
    setIrDumpDisasmDump,
} = appStateSlice.actions;

export default appStateSlice.reducer;
