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
import { Provider } from 'react-redux';
import { configureStore } from '@reduxjs/toolkit';
import LogsView from './LogsView';
import logsReducer from '../../store/slices/logs';
import { ELogType } from '../../models/logs';

const createMockStore = (): ReturnType<typeof configureStore> => {
    return configureStore({
        reducer: {
            logs: logsReducer
        }
    });
};

describe('LogsView Filters', () => {
    const mockLogs = [
        {
            message: [{ message: 'Compile output message', source: 'output' as const }],
            isRead: false,
            from: ELogType.COMPILE_OUT,
            exit_code: 0
        },
        {
            message: [{ message: 'Compile error message', source: 'error' as const }],
            isRead: false,
            from: ELogType.COMPILE_ERR,
            exit_code: 1
        },
        {
            message: [{ message: 'Runtime output message', source: 'output' as const }],
            isRead: false,
            from: ELogType.RUN_OUT,
            exit_code: 0
        },
        {
            message: [{ message: 'Runtime error message', source: 'error' as const }],
            isRead: false,
            from: ELogType.RUN_ERR,
            exit_code: 1
        }
    ];

    const renderWithStore = (logs = mockLogs): ReturnType<typeof render> => {
        const store = createMockStore();
        return render(
            <Provider store={store}>
                <LogsView
                    logArr={logs}
                    clearFilters={jest.fn()}
                    logType='compilation'
                />
            </Provider>
        );
    };

    it('shows both Out and Err checkboxes', () => {
        renderWithStore();

        const checkboxes = screen.getAllByRole('checkbox');
        expect(checkboxes).toHaveLength(2);

        const outCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Out');
        const errCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Err');

        expect(outCheckbox).toBeInTheDocument();
        expect(errCheckbox).toBeInTheDocument();
    });

    it('both checkboxes are checked by default', () => {
        renderWithStore();

        const checkboxes = screen.getAllByRole('checkbox');
        const outCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Out') as HTMLInputElement;
        const errCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Err') as HTMLInputElement;

        expect(outCheckbox.checked).toBe(true);
        expect(errCheckbox.checked).toBe(true);
    });

    it('displays all logs when both checkboxes are checked', () => {
        renderWithStore();

        expect(screen.getByText('Compile output message')).toBeInTheDocument();
        expect(screen.getByText('Compile error message')).toBeInTheDocument();
        expect(screen.getByText('Runtime output message')).toBeInTheDocument();
        expect(screen.getByText('Runtime error message')).toBeInTheDocument();
    });

    it('filters out error logs when Err checkbox is unchecked', () => {
        renderWithStore();

        const checkboxes = screen.getAllByRole('checkbox');
        const errCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Err');
        if (errCheckbox) {
            fireEvent.click(errCheckbox);
        }

        expect(screen.getByText('Compile output message')).toBeInTheDocument();
        expect(screen.getByText('Runtime output message')).toBeInTheDocument();
        expect(screen.queryByText('Compile error message')).not.toBeInTheDocument();
        expect(screen.queryByText('Runtime error message')).not.toBeInTheDocument();
    });

    it('filters out output logs when Out checkbox is unchecked', () => {
        renderWithStore();

        const checkboxes = screen.getAllByRole('checkbox');
        const outCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Out');
        if (outCheckbox) {
            fireEvent.click(outCheckbox);
        }

        expect(screen.queryByText('Compile output message')).not.toBeInTheDocument();
        expect(screen.queryByText('Runtime output message')).not.toBeInTheDocument();
        expect(screen.getByText('Compile error message')).toBeInTheDocument();
        expect(screen.getByText('Runtime error message')).toBeInTheDocument();
    });

    it('filters logs by text input', () => {
        renderWithStore();

        const filterInput = screen.getByPlaceholderText('Filter') as HTMLInputElement;
        fireEvent.change(filterInput, { target: { value: 'Compile' } });

        expect(screen.getByText('Compile output message')).toBeInTheDocument();
        expect(screen.getByText('Compile error message')).toBeInTheDocument();
        expect(screen.queryByText('Runtime output message')).not.toBeInTheDocument();
        expect(screen.queryByText('Runtime error message')).not.toBeInTheDocument();
    });

    it('combines checkbox and text filters', () => {
        renderWithStore();

        const checkboxes = screen.getAllByRole('checkbox');
        const errCheckbox = checkboxes.find((cb) => cb.getAttribute('label') === 'Err');
        const filterInput = screen.getByPlaceholderText('Filter') as HTMLInputElement;

        if (errCheckbox) {
            fireEvent.click(errCheckbox); // Uncheck Err
        }
        fireEvent.change(filterInput, { target: { value: 'Compile' } });

        expect(screen.getByText('Compile output message')).toBeInTheDocument();
        expect(screen.queryByText('Compile error message')).not.toBeInTheDocument();
        expect(screen.queryByText('Runtime output message')).not.toBeInTheDocument();
        expect(screen.queryByText('Runtime error message')).not.toBeInTheDocument();
    });
});
