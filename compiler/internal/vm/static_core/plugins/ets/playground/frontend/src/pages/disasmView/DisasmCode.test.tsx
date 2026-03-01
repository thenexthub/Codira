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

import React from 'react';
import { render, screen } from '@testing-library/react';
import DisasmCode from './DisasmCode';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { ThemeProvider } from '../../components/theme/ThemeContext';
import { AppDispatch } from '../../store';
import { syntax } from '../codeEditor/ArkTSEditor.test';

jest.mock('lodash.debounce', () => jest.fn(fn => fn));

const mockStore = configureMockStore<AppDispatch>();

describe('DisasmView component', () => {
    let store: ReturnType<typeof mockStore>;

    beforeEach(() => {
        store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49', disasm: false },
            code: { compileLoading: false, runLoading: false, code: '' },
            options: { isLoading: false, options: [], pickedOptions: [] },
            syntax: { isLoading: false, syntax }
        });
        store.dispatch = jest.fn();
    });

    const renderWithProviders = (component: JSX.Element): ReturnType<typeof render> => {
        return render(
            <Provider store={store}>
                <ThemeProvider>{component}</ThemeProvider>
            </Provider>
        );
    };

    it('renders the editor component', () => {
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor')).toBeInTheDocument();
    });

    it('applies correct theme based on context', () => {
        localStorage.setItem('theme', 'dark');
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor')).toHaveAttribute('data-theme', 'vs-dark');
    });
    it('applies correct theme based on context light', () => {
        localStorage.setItem('theme', 'light');
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor')).toHaveAttribute('data-theme', 'light');
    });
    it('applies correct run loading', () => {
        store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49', disasm: true },
            code: { isCompileLoading: false, isRunLoading: true, code: '' },
            options: { isLoading: false, options: [], pickedOptions: [] },
            syntax: { isLoading: false, syntax }
        });
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor').textContent).toContain('Loading...');
    });
    it('applies correct compile loading', () => {
        store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49', disasm: true },
            code: { isCompileLoading: true, isRunLoading: false, code: '' },
            options: { isLoading: false, options: [], pickedOptions: [] },
            syntax: { isLoading: false, syntax }
        });
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor').textContent).toContain('Loading...');
    });
    it('applies correct disasm code', () => {
        const testCode = 'disasm code';
        store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49', disasm: true },
            code: { isCompileLoading: false, isRunLoading: false, code: '', runRes: { disassembly: { code: testCode}} },
            options: { isLoading: false, options: [], pickedOptions: [] },
            syntax: { isLoading: false, syntax }
        });
        renderWithProviders(<DisasmCode />);
        expect(screen.getByTestId('editor').textContent).toContain(testCode);
    });
});
