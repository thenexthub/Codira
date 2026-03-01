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
import { ThemeProvider, useTheme } from './ThemeContext';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { AppDispatch } from '../../store';
import { primaryColors } from '../header/Header';

const mockStore = configureMockStore<AppDispatch>();

const ThemeConsumerComponent = (): JSX.Element => {
    const { theme, toggleTheme, togglePrimaryColor } = useTheme();
    return (
        <div>
            <button onClick={toggleTheme}>Toggle Theme</button>
            <button onClick={(): void => togglePrimaryColor(primaryColors[3])}>Set Color</button>
            <span data-testid="theme-value">{theme}</span>
        </div>
    );
};

describe('ThemeProvider component', () => {
    let store: ReturnType<typeof mockStore>;

    beforeEach(() => {
        // @ts-ignore
        store = mockStore({});
        store.dispatch = jest.fn();
        localStorage.clear();
        document.body.className = '';
    });

    const renderWithProviders = (component: JSX.Element): ReturnType<typeof render> => {
        return render(
            <Provider store={store}>
                <ThemeProvider>{component}</ThemeProvider>
            </Provider>
        );
    };

    it('renders with initial theme based on system preference', () => {
        renderWithProviders(<ThemeConsumerComponent />);
        const themeValue = screen.getByTestId('theme-value');

        const systemPrefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
        const expectedTheme = systemPrefersDark ? 'dark' : 'light';

        expect(themeValue.textContent).toBe(expectedTheme);
        expect(document.body.classList.contains(`${expectedTheme}-theme`)).toBe(true);
    });

    it('renders with initial theme based on localStorage', () => {
        const theme = 'dark';
        localStorage.setItem('theme', theme);
        renderWithProviders(<ThemeConsumerComponent />);
        const themeValue = screen.getByTestId('theme-value');

        expect(themeValue.textContent).toBe(theme);
        expect(document.body.classList.contains(`${theme}-theme`)).toBe(true);
    });

    it('toggles theme on button click and dispatches action', () => {
        const dark = 'dark';
        const light = 'light';
        localStorage.setItem('theme', dark);
        renderWithProviders(<ThemeConsumerComponent />);
        const themeValue = screen.getByTestId('theme-value');
        expect(themeValue.textContent).toBe(dark);

        const toggleButton = screen.getByText('Toggle Theme');
        fireEvent.click(toggleButton);

        expect(store.dispatch).toHaveBeenCalled();
        expect(themeValue.textContent).toBe(light);
    });


    it('sets primary color on button click and dispatches action', () => {
        renderWithProviders(<ThemeConsumerComponent />);
        expect(document.documentElement.style.getPropertyValue('--primary1')).toBe(primaryColors[0]);
        const setColorButton = screen.getByText('Set Color');

        fireEvent.click(setColorButton);

        expect(document.documentElement.style.getPropertyValue('--primary1')).toBe(primaryColors[3]);
        expect(store.dispatch).toHaveBeenCalled();
    });
});
