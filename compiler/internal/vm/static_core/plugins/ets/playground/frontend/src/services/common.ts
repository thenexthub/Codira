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
import { AxiosError, AxiosRequestConfig, AxiosResponse } from 'axios';
import { getApiInstance } from './apiInstance';
import { IServiceResponse } from './types';

export const fetchGetEntity = async function<U, T>(
    endpoint: string,
    fallbackValue: T,
    parse: (data: U) => T,
    successCode = 200,
    params?: AxiosRequestConfig['params'],
): Promise<IServiceResponse<T>> {
    try {
        const apiInstance = getApiInstance();
        if (!apiInstance) {
            return {
                data: fallbackValue,
                error: '',
            };
        }
        const response: AxiosResponse<U> = await apiInstance.get(endpoint, { params });
        if (response.status !== successCode) {
            return {
                data: fallbackValue,
                error: JSON.stringify(response.data || 'Request failed'),
            };
        }
        return {
            data: parse(response.data),
            error: '',
        };
    } catch (err) {
        const error = err as AxiosError;
        return {
            data: fallbackValue,
            // @ts-ignore
            error: error.response?.data || 'Request failed',
        };
    }
};

export const fetchPostEntity = async function<P, U, T>(
    payload: P,
    endpoint: string,
    fallbackValue: T,
    parse: (data: U) => T,
    successCode = 200,
): Promise<IServiceResponse<T>> {
    try {
        const apiInstance = getApiInstance();
        if (!apiInstance) {
            return {
                data: fallbackValue,
                error: '',
            };
        }
        const response: AxiosResponse<U> = await apiInstance.post(endpoint, payload);
        if (response.status !== successCode) {
            return {
                data: fallbackValue,
                error: JSON.stringify(response.data || 'Request failed'),
            };
        }
        return {
            data: parse(response.data),
            error: '',
        };
    } catch (err) {
        const error = err as AxiosError;
        return {
            data: fallbackValue,
            // @ts-ignore
            error: error.response?.data || 'Request failed',
        };
    }
};

export const fetchPutEntity = async function<P, U, T>(
    payload: P,
    endpoint: string,
    fallbackValue: T,
    parse: (data: U) => T,
    successCode = 200,
): Promise<IServiceResponse<T>> {
    try {
        const apiInstance = getApiInstance();
        if (!apiInstance) {
            return {
                data: fallbackValue,
                error: '',
            };
        }
        const response: AxiosResponse<U> = await apiInstance.put(endpoint, payload);
        if (response.status !== successCode) {
            return {
                data: fallbackValue,
                error: JSON.stringify(response.data || 'Request failed'),
            };
        }
        return {
            data: parse(response.data),
            error: '',
        };
    } catch (err) {
        const error = err as AxiosError;
        return {
            data: fallbackValue,
            // @ts-ignore
            error: error.response?.data || 'Request failed',
        };
    }
};

export const fetchDeleteEntity = async function<U>(
    endpoint: string,
    fallbackValue = false,
    successCode = 200,
): Promise<IServiceResponse<boolean>> {
    try {
        const apiInstance = getApiInstance();
        if (!apiInstance) {
            return {
                data: fallbackValue,
                error: '',
            };
        }
        const response: AxiosResponse<U> = await apiInstance.delete(endpoint);
        if (response.status !== successCode) {
            return {
                data: fallbackValue,
                error: JSON.stringify(response.data || 'Request failed'),
            };
        }
        return {
            data: true,
            error: '',
        };
    } catch (err) {
        const error = err as AxiosError;
        return {
            data: fallbackValue,
            // @ts-ignore
            error: error.response?.data || 'Request failed',
        };
    }
};
