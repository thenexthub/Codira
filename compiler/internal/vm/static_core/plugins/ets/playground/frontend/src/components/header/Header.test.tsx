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
import Header, {primaryColors} from './Header';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { ThemeProvider } from '../theme/ThemeContext';
import { AppDispatch } from '../../store';

const mockStore = configureMockStore<AppDispatch>();

describe('Header component', () => {
    let store: ReturnType<typeof mockStore>;

    beforeEach(() => {
        store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49' },
        });
        store.dispatch = jest.fn();
    });

    const renderWithProviders = (component: JSX.Element, storeOverrides = {}): ReturnType<typeof render> => {
        return render(
            <Provider store={store}>
                <ThemeProvider>{component}</ThemeProvider>
            </Provider>
        );
    };

    it('renders header with title "ArkTS playground"', () => {
        renderWithProviders(<Header />);
        expect(screen.getByText('ArkTS playground')).toBeInTheDocument();
    });

    it('opens and closes the settings popover on click', () => {
        renderWithProviders(<Header />);
        const settingsIcon = screen.getByTestId('settings-icon');
        fireEvent.click(settingsIcon);
        expect(screen.getByText('Dark mode')).toBeInTheDocument();
    });

    it('toggles theme when the switch is clicked', () => {
        renderWithProviders(<Header />);
        const settingsIcon = screen.getByTestId('settings-icon');
        fireEvent.click(settingsIcon);
        const darkModeSwitch = screen.getAllByTestId('mocked-switch');
        expect(darkModeSwitch[0]).toHaveTextContent('Off');
        fireEvent.click(darkModeSwitch[0]);
        expect(darkModeSwitch[0]).toHaveTextContent('On');
    });

    it('changes primary color when a color option is clicked', () => {
        renderWithProviders(<Header />);
        const settingsIcon = screen.getByTestId('settings-icon');
        fireEvent.click(settingsIcon);

        const firstColorOption = screen.getByTestId(primaryColors[0]);
        fireEvent.click(firstColorOption);

        expect(firstColorOption).toHaveStyle({ backgroundColor: primaryColors[0] });
    });
});
