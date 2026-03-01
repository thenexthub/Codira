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

import { optionsService, EOptionsEndpoints } from './options';
import { fetchGetEntity } from './common';
import { optionsModel, IOption } from '../models/options';

jest.mock('./common', () => ({
    fetchGetEntity: jest.fn(),
}));

describe('optionsService', () => {
    describe('fetchGetOptions', () => {
        it('should call fetchGetEntity with correct parameters and return options on success', async () => {
            const mockOptions: IOption[] = [
                { flag: 'option1', values: [1, 2, 3], default: 1 },
                { flag: 'option2', values: ['a', 'b', 'c'], default: 'a' },
            ];
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: mockOptions, error: '' });

            const response = await optionsService.fetchGetOptions();

            expect(fetchGetEntity).toHaveBeenCalledWith(
                EOptionsEndpoints.GET_OPTIONS,
                [],
                optionsModel.fromApi,
            );
            expect(response).toEqual({ data: mockOptions, error: '' });
        });

        it('should return fallback value (empty array) on failure', async () => {
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: [], error: 'Request failed' });

            const response = await optionsService.fetchGetOptions();

            expect(response).toEqual({ data: [], error: 'Request failed' });
        });
    });
});
