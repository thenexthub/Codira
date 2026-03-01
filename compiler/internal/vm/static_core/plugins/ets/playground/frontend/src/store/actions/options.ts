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

import { createAsyncThunk } from '@reduxjs/toolkit';
import {setOptionsLoading, setOptionsPicked, setOptionsResponse} from '../slices/options';
import {optionsService} from '../../services/options';
import {TObj} from '../../models/options';
import { RootState } from '..';

export const fetchOptions = createAsyncThunk(
    '@options/fetch',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setOptionsLoading(true));
        const response = await optionsService.fetchGetOptions();
        if (response.error) {
            thunkAPI.dispatch(setOptionsLoading(false));
            return;
        }
        thunkAPI.dispatch(setOptionsResponse(response.data));

        const state: RootState = thunkAPI.getState() as RootState;
        const currentPickedOptions = state.options?.pickedOptions || {};

        const hasExistingOptions = Object.keys(currentPickedOptions).length > 0;

        if (!hasExistingOptions) {
            const picked: TObj = {};
            response.data?.forEach((el) => {
                if (el.isSelected?.toString()) {
                    picked[el.flag] = el.isSelected;
                }
            });
            thunkAPI.dispatch(pickOptions(picked));
        }

        thunkAPI.dispatch(setOptionsLoading(false));
    },
);

export const pickOptions = createAsyncThunk(
    '@options/select',
    async (opt: TObj, thunkAPI) => {
        thunkAPI.dispatch(setOptionsPicked(opt));
    },
);
