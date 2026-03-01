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
import { render, screen, fireEvent } from '@testing-library/react';
import CompileOptions from './CompileOptions';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { IOption, TObj } from '../../models/options';
import {AppDispatch} from '../../store';
import {ThemeProvider} from '../../components/theme/ThemeContext';

const mockStore = configureMockStore<AppDispatch>();

describe('CompileOptions component', () => {
    let store: ReturnType<typeof mockStore>;
    const availableOptions: IOption[] = [
        { flag: '--option1', values: ['1', '2', '3'], isSelected: '1', default: '1' },
        { flag: '--option2', values: ['A', 'B'], isSelected: 'A', default: 'A' },
    ];
    const pickedOptions: TObj = {
        '--option1': '2',
        '--option2': 'B',
    };
    beforeEach(() => {
        store = mockStore({
            // @ts-ignore
            appState: { disasm: false, theme: 'light', primaryColor: '#e32b49' },
            options: { options: availableOptions, pickedOptions: pickedOptions, isLoading: false },
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

    const mockOnClose = jest.fn();

    it('renders available options correctly', () => {
        renderWithProviders(<CompileOptions onClose={mockOnClose} />);

        expect(screen.getByText('--option1')).toBeInTheDocument();
        expect(screen.getByText('--option2')).toBeInTheDocument();
        expect(screen.getByLabelText('1')).toBeInTheDocument();
        expect(screen.getByLabelText('2')).toBeInTheDocument();
        expect(screen.getByLabelText('3')).toBeInTheDocument();
        expect(screen.getByLabelText('A')).toBeInTheDocument();
        expect(screen.getByLabelText('B')).toBeInTheDocument();
    });

    it('handles option selection correctly', () => {
        renderWithProviders(<CompileOptions onClose={mockOnClose} />);

        const option1Value2 = screen.getByTestId('radio-2');
        fireEvent.click(option1Value2);
        expect(option1Value2).toBeChecked();

        const option2ValueB = screen.getByTestId('radio-B');
        fireEvent.click(option2ValueB);
        expect(option2ValueB).toBeChecked();
    });

    it('dispatches pickOptions and calls onClose on Save button click', () => {
        renderWithProviders(<CompileOptions onClose={mockOnClose} />);

        fireEvent.click(screen.getByText('Save'));

        expect(store.dispatch).toHaveBeenCalled();
        expect(mockOnClose).toHaveBeenCalled();
    });

    it('resets options on Reset button click', () => {
        renderWithProviders(<CompileOptions onClose={mockOnClose} />);

        fireEvent.click(screen.getByText('Reset'));
        expect(mockOnClose).toHaveBeenCalled();
    });
});
