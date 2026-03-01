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
import {
    setClearHighLightErrs,
    setErrLogs,
    setOutLogs
} from '../slices/logs';
import { RootState } from '..';
import { ELogType } from '../../models/logs';

export const clearOutLogs = createAsyncThunk(
    '@logs/clearOut',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setOutLogs([]));
        thunkAPI.dispatch(setClearHighLightErrs());
    },
);

export const clearErrLogs = createAsyncThunk(
    '@logs/clearErr',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setErrLogs([]));
        thunkAPI.dispatch(setClearHighLightErrs());
    },
);

export const clearCompilationLogs = createAsyncThunk(
    '@logs/clearCompilation',
    async (_, thunkAPI) => {
        const state = thunkAPI.getState() as RootState;
        const filteredOut = state.logs.out.filter(log =>
            log.from !== ELogType.COMPILE_OUT
        );
        const filteredErr = state.logs.err.filter(log =>
            log.from !== ELogType.COMPILE_ERR
        );
        thunkAPI.dispatch(setOutLogs(filteredOut));
        thunkAPI.dispatch(setErrLogs(filteredErr));
        thunkAPI.dispatch(setClearHighLightErrs());
    },
);

export const clearRuntimeLogs = createAsyncThunk(
    '@logs/clearRuntime',
    async (_, thunkAPI) => {
        const state = thunkAPI.getState() as RootState;
        const filteredOut = state.logs.out.filter(log =>
            log.from !== ELogType.RUN_OUT
        );
        const filteredErr = state.logs.err.filter(log =>
            log.from !== ELogType.RUN_ERR
        );
        thunkAPI.dispatch(setOutLogs(filteredOut));
        thunkAPI.dispatch(setErrLogs(filteredErr));
        thunkAPI.dispatch(setClearHighLightErrs());
    },
);
