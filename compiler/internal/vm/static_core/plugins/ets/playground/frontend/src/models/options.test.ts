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

import { optionsModel, IOption, TData } from './options';

describe('optionsModel', () => {
    describe('fromApi', () => {
        it('should transform API data correctly with all fields provided', () => {
            const apiData: TData = {
                compileOptions: [
                    { flag: 'optimize', values: [0, 1, 2], default: 1 },
                    { flag: 'debug', values: [true, false], default: false },
                ]
            };

            const expectedData: IOption[] = [
                { flag: 'optimize', values: [0, 1, 2], default: '1', isSelected: '1' },
                { flag: 'debug', values: [true, false], default: 'false', isSelected: 'false' },
            ];
            expect(optionsModel.fromApi(apiData)).toEqual(expectedData);
        });

        it('should handle missing fields by setting defaults', () => {
            const apiData: TData = {
                compileOptions: [
                    { flag: 'optimize', values: [0, 1, 2], default: 1 },
                    // missing `values` and `default` fields
                    { flag: 'missingFields', values: [], default: '' },
                ]
            };

            const expectedData: IOption[] = [
                { flag: 'optimize', values: [0, 1, 2], default: '1', isSelected: '1' },
                { flag: 'missingFields', values: [], default: '', isSelected: '' },
            ];

            expect(optionsModel.fromApi(apiData)).toEqual(expectedData);
        });

        it('should return an empty array if compileOptions is missing', () => {
            const apiData = {} as TData;

            expect(optionsModel.fromApi(apiData)).toEqual([]);
        });

        it('should return an empty array if compileOptions is empty', () => {
            const apiData: TData = { compileOptions: [] };

            expect(optionsModel.fromApi(apiData)).toEqual([]);
        });
    });
});
