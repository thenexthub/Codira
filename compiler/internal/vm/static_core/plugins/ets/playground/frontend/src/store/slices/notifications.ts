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
import { INotification } from '../../models/notifications';

interface INotificationsState {
    messages: INotification[]
}

const initialState: INotificationsState = {
    messages: []
};

const notificationsSlide = createSlice({
    name: 'notificationsState',
    initialState,
    reducers: {
        setMessages: (state, action: PayloadAction<INotification[]>) => {
            state.messages = action.payload;
        },
        addMessage: (state, action: PayloadAction<INotification>) => {
            state.messages.push(action.payload);
        },
        removeMessage: (state, action: PayloadAction<number>) => {
            state.messages = state.messages.filter((item) => item.id !== action.payload);
        },
        resetMessages: (state) => {
            state.messages = [] as INotification[];
        }
    }
});

export const {
    setMessages,
    addMessage,
    removeMessage,
    resetMessages,
} = notificationsSlide.actions;

export default notificationsSlide.reducer;
