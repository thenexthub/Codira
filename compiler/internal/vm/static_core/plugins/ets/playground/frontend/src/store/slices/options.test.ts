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

import clinicReducer, {
    setOptionsLoading,
    setOptionsResponse,
    setOptionsPicked,
} from './options';
import { IOption } from '../../models/options';

describe('clinicSlice', () => {
    const initialState = {
        isLoading: false,
        options: null,
        pickedOptions: {},
    };

    const mockOptions: IOption[] = [
        { flag: 'option1', values: [1, 2, 3], default: 1 },
        { flag: 'option2', values: ['A', 'B'], default: 'A' }
    ];

    const mockPickedOptions = { option1: 2, option2: 'B' };

    it('should handle setOptionsLoading action', () => {
        const action = setOptionsLoading(true);
        const state = clinicReducer(initialState, action);
        expect(state.isLoading).toBe(true);
    });

    it('should handle setOptionsResponse action', () => {
        const action = setOptionsResponse(mockOptions);
        const state = clinicReducer(initialState, action);
        expect(state.options).toEqual(mockOptions);
    });

    it('should handle setOptionsPicked action', () => {
        const action = setOptionsPicked(mockPickedOptions);
        const state = clinicReducer(initialState, action);
        expect(state.pickedOptions).toEqual(mockPickedOptions);
    });
});
