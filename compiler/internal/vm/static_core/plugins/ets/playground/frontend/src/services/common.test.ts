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

import {AxiosError, AxiosResponse, AxiosResponseHeaders} from 'axios';
import {
    fetchGetEntity,
    fetchPostEntity,
    fetchPutEntity,
    fetchDeleteEntity
} from './common';
import { getApiInstance } from './apiInstance';

jest.mock('./apiInstance');

describe('API CRUD Operations', () => {
    const mockGetApiInstance = getApiInstance as jest.Mock;
    type MockApiInstance<P, U> = {
        get: jest.Mock<Promise<AxiosResponse<U>>, [string]>,
        post: jest.Mock<Promise<AxiosResponse<U>>, [string, P]>,
        put: jest.Mock<Promise<AxiosResponse<U>>, [string, P]>,
        delete: jest.Mock<Promise<AxiosResponse<U>>, [string]>
    };
    
    type RequestPayloadType = { name: string; age: number };
    type ResponseDataType = { id: string; name: string; age: number };
    
    let mockApiInstance: MockApiInstance<RequestPayloadType, ResponseDataType>;

    beforeEach(() => {
        mockApiInstance = {
            get: jest.fn(),
            post: jest.fn(),
            put: jest.fn(),
            delete: jest.fn(),
        };
        mockGetApiInstance.mockReturnValue(mockApiInstance);
    });

    afterEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchGetEntity', () => {
        it('should return fallback value if apiInstance is null', async () => {
            mockGetApiInstance.mockReturnValue(null);
            const fallbackValue = { key: 'value' };
            const result = await fetchGetEntity('endpoint', fallbackValue, (data) => data);
            expect(result).toEqual({ data: fallbackValue, error: '' });
        });

        it('should return parsed data if request is successful', async () => {
            const response: AxiosResponse = { data: { key: 'value' }, status: 200, statusText: 'OK', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.get.mockResolvedValue(response);
            const result = await fetchGetEntity('endpoint', {}, (data) => data);
            expect(result).toEqual({ data: { key: 'value' }, error: '' });
        });

        it('should return error message if request fails', async () => {
            const axiosError = new AxiosError('Request failed');
            axiosError.response = { data: 'Error message', status: 500, statusText: 'ERR', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.get.mockRejectedValue(axiosError);
            const result = await fetchGetEntity('endpoint', {}, (data) => data);
            expect(result).toEqual({ data: {}, error: 'Error message' });
        });
    });

    describe('fetchPostEntity', () => {
        it('should return fallback value if apiInstance is null', async () => {
            mockGetApiInstance.mockReturnValue(null);
            const fallbackValue = { key: 'value' };
            const result = await fetchPostEntity({}, 'endpoint', fallbackValue, (data) => data);
            expect(result).toEqual({ data: fallbackValue, error: '' });
        });

        it('should return parsed data if POST request is successful', async () => {
            const response: AxiosResponse = { data: { key: 'value' }, status: 200, statusText: 'OK', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.post.mockResolvedValue(response);
            const result = await fetchPostEntity({}, 'endpoint', {}, (data) => data);
            expect(result).toEqual({ data: { key: 'value' }, error: '' });
        });

        it('should return error message if POST request fails', async () => {
            const axiosError = new AxiosError('Request failed');
            axiosError.response = { data: 'Error message', status: 500, statusText: 'ERR', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.post.mockRejectedValue(axiosError);
            const result = await fetchPostEntity({}, 'endpoint', {}, (data) => data);
            expect(result).toEqual({ data: {}, error: 'Error message' });
        });
    });

    describe('fetchPutEntity', () => {
        it('should return fallback value if apiInstance is null', async () => {
            mockGetApiInstance.mockReturnValue(null);
            const fallbackValue = { key: 'value' };
            const result = await fetchPutEntity({}, 'endpoint', fallbackValue, (data) => data);
            expect(result).toEqual({ data: fallbackValue, error: '' });
        });

        it('should return parsed data if PUT request is successful', async () => {
            const response: AxiosResponse = { data: { key: 'value' }, status: 200, statusText: 'OK', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.put.mockResolvedValue(response);
            const result = await fetchPutEntity({}, 'endpoint', {}, (data) => data);
            expect(result).toEqual({ data: { key: 'value' }, error: '' });
        });

        it('should return error message if PUT request fails', async () => {
            const axiosError = new AxiosError('Request failed');
            axiosError.response = { data: 'Error message', status: 500, statusText: 'ERR', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.put.mockRejectedValue(axiosError);
            const result = await fetchPutEntity({}, 'endpoint', {}, (data) => data);
            expect(result).toEqual({ data: {}, error: 'Error message' });
        });
    });

    describe('fetchDeleteEntity', () => {
        it('should return fallback value if apiInstance is null', async () => {
            mockGetApiInstance.mockReturnValue(null);
            const result = await fetchDeleteEntity('endpoint', false);
            expect(result).toEqual({ data: false, error: '' });
        });

        it('should return true if DELETE request is successful', async () => {
            const response: AxiosResponse = { data: {}, status: 200, statusText: 'No Content', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.delete.mockResolvedValue(response);
            const result = await fetchDeleteEntity('endpoint', false);
            expect(result).toEqual({ data: true, error: '' });
        });

        it('should return error message if DELETE request fails', async () => {
            const axiosError = new AxiosError('Request failed');
            axiosError.response = { data: 'Error message', status: 500, statusText: 'ERR', headers: {}, config: { headers: {} as AxiosResponseHeaders}};
            mockApiInstance.delete.mockRejectedValue(axiosError);
            const result = await fetchDeleteEntity('endpoint', false);
            expect(result).toEqual({ data: false, error: 'Error message' });
        });
    });
});
