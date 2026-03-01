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
import {ICodeReq} from '../../models/code';

interface IState {
    isRunLoading: boolean
    isCompileLoading: boolean
    isShareLoading: boolean
    code: string
    compileRes: ICodeReq | null
    runRes: ICodeReq | null
    verifierRes: ICodeReq | null
}

const initialState: IState = {
    isRunLoading: false,
    isCompileLoading: false,
    isShareLoading: false,
    code: 'console.log("Hello, ArkTS!");',
    compileRes: null,
    runRes: null,
    verifierRes: null
};

const codeSlice = createSlice({
    name: 'codeState',
    initialState,
    reducers: {
        setRunLoading(state, action: PayloadAction<boolean>) {
            state.isRunLoading = action.payload;
        },
        setCompileLoading(state, action: PayloadAction<boolean>) {
            state.isCompileLoading = action.payload;
        },
        setShareLoading(state, action: PayloadAction<boolean>) {
            state.isShareLoading = action.payload;
        },
        setCode(state, action: PayloadAction<string>) {
            state.code = action.payload;
        },
        setCompileRes(state, action: PayloadAction<ICodeReq | null>) {
            state.compileRes = action.payload;
        },
        setRunRes(state, action: PayloadAction<ICodeReq | null>) {
            state.runRes = action.payload;
        },
    }
});

export const {
    setRunLoading,
    setCompileLoading,
    setShareLoading,
    setCode,
    setCompileRes,
    setRunRes
} = codeSlice.actions;

export default codeSlice.reducer;
