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

import { selectOptions, selectPickedOptions } from './options';
import { RootState } from '..';
import {mockAllState} from './appState.test';

describe('Option Selectors', () => {
    let mockState: RootState;

    beforeEach(() => {
        mockState = {...mockAllState,
            options: {
                isLoading: false,
                options: [
                    { flag: 'opt1', values: [true, false], default: 'true', isSelected: 'true' },
                    { flag: 'opt2', values: [1, 2, 3], default: '1', isSelected: '2' },
                ],
                pickedOptions: {
                    opt1: 'true',
                    opt2: '2',
                },
            },
        };
    });

    it('should select available options', () => {
        expect(selectOptions(mockState)).toEqual([
            { flag: 'opt1', values: [true, false], default: 'true', isSelected: 'true' },
            { flag: 'opt2', values: [1, 2, 3], default: '1', isSelected: '2' },
        ]);
    });

    it('should select picked options', () => {
        expect(selectPickedOptions(mockState)).toEqual({
            opt1: 'true',
            opt2: '2',
        });
    });
});
