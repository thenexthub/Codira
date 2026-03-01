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

import { configureStore } from '@reduxjs/toolkit';
import logsReducer from '../slices/logs';
import { clearCompilationLogs, clearRuntimeLogs } from './logs';
import { ELogType } from '../../models/logs';

describe('Clear logs actions', () => {
    it('clearCompilationLogs removes only compilation logs', async () => {
        const store = configureStore({
            reducer: {
                logs: logsReducer
            }
        });

        // Add logs of different types
        store.dispatch({
            type: 'logsState/setOutLogs',
            payload: [
                { message: [{ message: 'compile out' }], from: ELogType.COMPILE_OUT },
                { message: [{ message: 'run out' }], from: ELogType.RUN_OUT }
            ]
        });
        store.dispatch({
            type: 'logsState/setErrLogs',
            payload: [
                { message: [{ message: 'compile err' }], from: ELogType.COMPILE_ERR },
                { message: [{ message: 'run err' }], from: ELogType.RUN_ERR }
            ]
        });

        // Clear compilation logs
        await store.dispatch(clearCompilationLogs());

        const state = store.getState();

        // Should only have runtime logs left
        expect(state.logs.out).toHaveLength(1);
        expect(state.logs.out[0].from).toBe(ELogType.RUN_OUT);

        expect(state.logs.err).toHaveLength(1);
        expect(state.logs.err[0].from).toBe(ELogType.RUN_ERR);
    });

    it('clearRuntimeLogs removes only runtime logs', async () => {
        const store = configureStore({
            reducer: {
                logs: logsReducer
            }
        });

        // Add logs of different types
        store.dispatch({
            type: 'logsState/setOutLogs',
            payload: [
                { message: [{ message: 'compile out' }], from: ELogType.COMPILE_OUT },
                { message: [{ message: 'run out' }], from: ELogType.RUN_OUT }
            ]
        });
        store.dispatch({
            type: 'logsState/setErrLogs',
            payload: [
                { message: [{ message: 'compile err' }], from: ELogType.COMPILE_ERR },
                { message: [{ message: 'run err' }], from: ELogType.RUN_ERR }
            ]
        });

        // Clear runtime logs
        await store.dispatch(clearRuntimeLogs());

        const state = store.getState();

        // Should only have compilation logs left
        expect(state.logs.out).toHaveLength(1);
        expect(state.logs.out[0].from).toBe(ELogType.COMPILE_OUT);

        expect(state.logs.err).toHaveLength(1);
        expect(state.logs.err[0].from).toBe(ELogType.COMPILE_ERR);
    });

    it('clearCompilationLogs handles empty logs', async () => {
        const store = configureStore({
            reducer: {
                logs: logsReducer
            }
        });

        await store.dispatch(clearCompilationLogs());

        const state = store.getState();
        expect(state.logs.out).toEqual([]);
        expect(state.logs.err).toEqual([]);
    });

    it('clearRuntimeLogs handles empty logs', async () => {
        const store = configureStore({
            reducer: {
                logs: logsReducer
            }
        });

        await store.dispatch(clearRuntimeLogs());

        const state = store.getState();
        expect(state.logs.out).toEqual([]);
        expect(state.logs.err).toEqual([]);
    });
});
