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

import { store, RootState } from '../index';
import { setTheme } from '../slices/appState';
import { setOptionsLoading } from '../slices/options';
import { setSyntaxLoading } from '../slices/syntax';

describe('Redux store structure and functionality', () => {
    it('should initialize with the correct default state', () => {
        const initialState: RootState = store.getState();
        expect(initialState.appState.theme).toBe('light'); // Assuming default as 'light'
        expect(initialState.options.isLoading).toBe(false);
        expect(initialState.syntax.isLoading).toBe(false);
    });

    it('should handle appState actions correctly', () => {
        store.dispatch(setTheme('dark'));
        const state = store.getState();
        expect(state.appState.theme).toBe('dark');
    });

    it('should handle options loading state change', () => {
        store.dispatch(setOptionsLoading(true));
        const state = store.getState();
        expect(state.options.isLoading).toBe(true);
    });

    it('should handle syntax loading state change', () => {
        store.dispatch(setSyntaxLoading(true));
        const state = store.getState();
        expect(state.syntax.isLoading).toBe(true);
    });
});
