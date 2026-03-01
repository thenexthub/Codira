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

import { store } from './index';
import { setTheme } from './slices/appState';
import { setOptionsLoading, setOptionsResponse } from './slices/options';
import { setSyntaxLoading, setSyntaxResponse } from './slices/syntax';
import { setCode, setRunLoading, setCompileLoading } from './slices/code';
import { setOutLogs, setErrLogs } from './slices/logs';

describe('Redux Store Configuration', () => {
    it('should initialize with the correct default state', () => {
        const state = store.getState();

        expect(state.appState.theme).toBeDefined();
        expect(state.options.isLoading).toBe(false);
        expect(state.syntax.isLoading).toBe(false);
        expect(state.code.isCompileLoading).toBe(false);
        expect(state.logs.out).toEqual([]);
    });

    it('should handle appState actions', () => {
        store.dispatch(setTheme('dark'));
        expect(store.getState().appState.theme).toBe('dark');
    });

    it('should handle options actions', () => {
        store.dispatch(setOptionsLoading(true));
        expect(store.getState().options.isLoading).toBe(true);

        const mockOptions = [{ flag: 'option1', values: [1, 2], default: 1 }];
        store.dispatch(setOptionsResponse(mockOptions));
        expect(store.getState().options.options).toEqual(mockOptions);
    });

    it('should handle syntax actions', () => {
        store.dispatch(setSyntaxLoading(true));
        expect(store.getState().syntax.isLoading).toBe(true);

        const mockSyntax = { tokenizer: { root: [[/\b(keyword)\b/, 'keyword']] } };
        store.dispatch(setSyntaxResponse(mockSyntax));
        expect(store.getState().syntax.syntax).toEqual(mockSyntax);
    });

    it('should handle code actions', () => {
        store.dispatch(setRunLoading(true));
        expect(store.getState().code.isRunLoading).toBe(true);

        store.dispatch(setCompileLoading(true));
        expect(store.getState().code.isCompileLoading).toBe(true);

        store.dispatch(setCode('new code'));
        expect(store.getState().code.code).toBe('new code');
    });

    it('should handle logs actions', () => {
        const mockOutLogs = [{ message: [{message: 'Compile success'}], isRead: false }];
        store.dispatch(setOutLogs(mockOutLogs));
        expect(store.getState().logs.out).toEqual(mockOutLogs);

        const mockErrLogs = [{ message: [{message: 'Run error'}], isRead: false }];
        store.dispatch(setErrLogs(mockErrLogs));
        expect(store.getState().logs.err).toEqual(mockErrLogs);
    });
});
