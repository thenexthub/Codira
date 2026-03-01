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

import { RootState } from '..';
import { IrDumpOptions, Theme, Versions, LogTab } from '../slices/appState';
import { VerificationMode } from '../../models/code';

export const theme = (state: RootState): Theme => state.appState.theme;
export const primaryColor = (state: RootState): string => state.appState.primaryColor;
export const withDisasm = (state: RootState): boolean => state.appState.disasm;
export const withVerifier = (state: RootState): boolean => state.appState.verifier;
export const withAstView = (state: RootState): boolean => state.appState.astView;
export const selectVerificationMode = (state: RootState): VerificationMode => state.appState.verificationMode;
export const withAotMode = (state: RootState): boolean => state.appState.aotMode;
export const versions = (state: RootState): Versions => state.appState.versions;
export const isVersionsLoading = (state: RootState): boolean => state.appState.versionsLoading;
export const clearLogsEachRun = (state: RootState): boolean => state.appState.clearLogsEachRun;
export const selectActiveLogTab = (state: RootState): LogTab => state.appState.activeLogTab;
export const withIrDump = (state: RootState): IrDumpOptions => state.appState.irDump;
export const withIrDumpEnabled = (state: RootState): boolean =>
    state.appState.irDump.compilerDump || state.appState.irDump.disasmDump;
