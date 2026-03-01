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
import {ILog} from '../../models/logs';

interface IState {
    out: ILog[]
    err: ILog[]
    highlightErrors: HighlightError[];
    jumpTo: JumpTo | null;
}
export type JumpTo = { line: number; column: number; nonce?: number };

export interface HighlightError {
    message: string;
    line: number;
    column: number;
    type?: string
}

const initialState: IState = {
    out: [],
    err: [],
    highlightErrors: [],
    jumpTo: null
};

const logsSlice = createSlice({
    name: 'logsState',
    initialState,
    reducers: {
        setOutLogs(state, action: PayloadAction<ILog[]>) {
            state.out = action.payload;
        },
        setErrLogs(state, action: PayloadAction<ILog[]>) {
            state.err = action.payload;
        },
        setHighlightErrors: (state, action) => {
            state.highlightErrors = state.highlightErrors.concat(action.payload);
        },
        setClearHighLightErrs: (state) => {
            state.highlightErrors = [];
        },
        setJumpTo(state, action: PayloadAction<JumpTo | null>) {
            state.jumpTo = action.payload;
        },
    }
});

export const {
    setOutLogs,
    setErrLogs,
    setHighlightErrors,
    setClearHighLightErrs,
    setJumpTo,
} = logsSlice.actions;

export default logsSlice.reducer;
