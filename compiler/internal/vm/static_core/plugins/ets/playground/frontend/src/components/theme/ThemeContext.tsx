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

import React, {createContext, useContext, useState, ReactNode, useEffect, useCallback} from 'react';
import {useDispatch} from 'react-redux';
import {AppDispatch} from '../../store';
import {setPrimaryColorAction, setThemeAction} from '../../store/actions/appState';

type Theme = 'light' | 'dark';

interface ThemeContextProps {
    theme: Theme;
    toggleTheme: () => void;
    togglePrimaryColor: (val: string) => void;
}

const ThemeContext = createContext<ThemeContextProps | undefined>(undefined);

export const ThemeProvider = ({ children }: { children: ReactNode }): JSX.Element => {
    const [theme, setTheme] = useState<Theme>((): Theme => {
        const savedTheme = localStorage.getItem('theme');
        if (savedTheme) {
            return savedTheme as Theme;
        }

        return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
    });
    const [primaryColor, setPrimaryColor] = useState((): string => {
        const savedPrimaryColor = localStorage.getItem('primaryColor');
        if (savedPrimaryColor) {
            return savedPrimaryColor;
        }
        return '#e32b49';
    });
    const dispatch = useDispatch<AppDispatch>();

    const toggleTheme = useCallback(() => {
        const nextTheme = theme === 'dark' ? 'light' : 'dark';
        setTheme(nextTheme);
        dispatch(setThemeAction(nextTheme));
    }, [dispatch, theme]);

    const togglePrimaryColor = useCallback((color: string) => {
        setPrimaryColor(color);
        dispatch(setPrimaryColorAction(color || '#e32b49'));
    }, [dispatch]);

    useEffect(() => {
        localStorage.setItem('theme', theme);
        localStorage.setItem('primaryColor', primaryColor || '');
        document.body.classList.remove('light-theme', 'dark-theme');
        document.body.classList.add(`${theme}-theme`);
        if (theme === 'dark') {
            document.documentElement.style.setProperty('--background-color', 'rgb(24, 24, 24)');
            document.documentElement.style.setProperty('--background-sub-color', '#2f343c');
            document.documentElement.style.setProperty('--text-color', '#d4d4d4');
            document.documentElement.style.setProperty('--blue', '#0d47a1');
            document.documentElement.style.setProperty('--divider', '#3c3c3c');
            document.documentElement.style.setProperty('--primary1', primaryColor || '#e32b49');
        } else {
            document.documentElement.style.setProperty('--background-color', '#f3f3f3');
            document.documentElement.style.setProperty('--background-sub-color', '#8f99a84d');
            document.documentElement.style.setProperty('--text-color', '#000000');
            document.documentElement.style.setProperty('--blue', '#1e90ff');
            document.documentElement.style.setProperty('--divider', '#f3f3f3');
            document.documentElement.style.setProperty('--primary1', primaryColor || '#e32b49');
        }

    }, [theme, primaryColor]);

    return (
        <ThemeContext.Provider value={{ theme, toggleTheme, togglePrimaryColor }}>
            {children}
        </ThemeContext.Provider>
    );
};

export const useTheme = (): ThemeContextProps => {
    const context = useContext(ThemeContext);
    if (!context) {
        throw new Error('useTheme must be used within a ThemeProvider');
    }
    return context;
};
