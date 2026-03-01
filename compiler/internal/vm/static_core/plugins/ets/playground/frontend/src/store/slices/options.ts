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

import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { IOption } from '../../models/options';

export interface IObj {
    [key: string]: boolean | number | string;
}

interface IState {
    isLoading: boolean
    options: IOption[] | null
    pickedOptions: IObj | null
}

const initialState: IState = {
    isLoading: false,
    options: null,
    pickedOptions: {},
};

const optionsSlice = createSlice({
    name: 'optionsState',
    initialState,
    reducers: {
        setOptionsLoading(state, action: PayloadAction<boolean>) {
            state.isLoading = action.payload;
        },
        setOptionsResponse(state, action: PayloadAction<IOption[]>) {
            state.options = action.payload;
        },
        setOptionsPicked(state, action: PayloadAction<IObj>) {
            state.pickedOptions = action.payload;
        }
    }
});

export const {
    setOptionsLoading,
    setOptionsResponse,
    setOptionsPicked,
} = optionsSlice.actions;

export default optionsSlice.reducer;
