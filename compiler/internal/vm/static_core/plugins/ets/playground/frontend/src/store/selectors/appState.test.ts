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

import { theme, primaryColor, withDisasm } from './appState';
import { RootState } from '..';

export const mockAllState: RootState = {
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
        out: [],
        err: [],
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
describe('Redux Selectors', () => {
    let mockState: RootState;

    beforeEach(() => {
        mockState = mockAllState;
    });

    it('should select the theme from appState', () => {
        expect(theme(mockState)).toBe('light');
        mockState.appState.theme = 'dark';
        expect(theme(mockState)).toBe('dark');
    });

    it('should select the primaryColor from appState', () => {
        expect(primaryColor(mockState)).toBe('#e32b49');
        mockState.appState.primaryColor = '#000000';
        expect(primaryColor(mockState)).toBe('#000000');
    });

    it('should select the disasm value from appState', () => {
        expect(withDisasm(mockState)).toBe(false);
        mockState.appState.disasm = true;
        expect(withDisasm(mockState)).toBe(true);
    });
});
