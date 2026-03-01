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

import {AxiosResponse, AxiosInstance, AxiosResponseHeaders} from 'axios';
import { getApiInstance } from './apiInstance';

describe('API Utilities', () => {
    let instance: AxiosInstance;

    beforeEach(() => {
        instance = getApiInstance();
    });

    describe('getApiInstance', () => {
        it('should create an Axios instance with the correct baseURL', () => {
            const defaultBaseURL = window?.runEnv?.apiUrl || window?.location?.origin;
            expect(instance.defaults.baseURL).toBe(defaultBaseURL);
        });

        it('should create an Axios instance with response interceptors', () => {
            const responseInterceptors = instance.interceptors.response;
            // @ts-ignore
            expect(responseInterceptors.handlers.length).toBeGreaterThan(0);
        });
    });

    describe('handleResponseSuccess', () => {
        it('should return the response as is', () => {
            const response: AxiosResponse = {
                data: {},
                status: 200,
                statusText: 'OK',
                headers: {},
                config: { headers: {} as AxiosResponseHeaders}
            };
            // @ts-ignore
            expect(instance.interceptors.response.handlers[0].fulfilled(response)).toEqual(response);
        });
    });
});
