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

import axios, { AxiosError, AxiosResponse, AxiosRequestConfig, AxiosHeaders, AxiosInstance } from 'axios';
import { store } from '../store'
import { showMessage } from '../store/actions/notification';

interface CustomAxiosRequestConfig extends AxiosRequestConfig {
    _retry?: boolean;
    headers: AxiosHeaders;
}

let isRefreshing = false;
let failedQueue: Array<{ resolve: (token: string) => void; reject: (error: AxiosError) => void }> = [];

const processQueue = (error: AxiosError | null, token: string | null = null): void => {
    failedQueue.forEach(prom => {
        if (error) {
            prom.reject(error);
        } else {
            prom.resolve(token as string);
        }
    });
    failedQueue = [];
};

const handleResponseSuccess = (response: AxiosResponse): AxiosResponse => response;

const handleResponseError = async (err: AxiosError): Promise<CustomAxiosRequestConfig> => {
    const config = err.config as CustomAxiosRequestConfig;
    store.dispatch(showMessage({
        type: 'error',
        title: err.message,
        message: String(err),
    }));
    return Promise.reject(err);
};

export const getApiInstance = (baseURL = window?.runEnv?.apiUrl || window?.location?.origin): AxiosInstance => {
    const instance = axios.create();
    instance.defaults.baseURL = baseURL;

    /* Response interceptors - log errors */
    instance.interceptors.response.use(
        handleResponseSuccess,
        handleResponseError,
    );

    return instance;
};
