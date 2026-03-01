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

import { IServiceResponse } from './types';
import {fetchGetEntity, fetchPostEntity} from './common';
import {codeModel, ICodeFetch, ICodeReq, ICodeShare, IRunReq, IShareReq} from '../models/code';

export enum EActionsEndpoints {
    COMPILE = '/compile',
    RUN = '/run',
    SHARE = '/share',
    SHARE_UUID = '/share/:uuid'
}

export const codeService = {
    fetchCompile: async (payload: ICodeFetch): Promise<IServiceResponse<ICodeReq | IRunReq>> => {
        // @ts-ignore
        return fetchPostEntity<ICodeFetch, ICodeReq>(
            payload,
            EActionsEndpoints.COMPILE,
            null,
            codeModel.fromApiCompile,
        );
    },
    fetchRun: async (payload: ICodeFetch): Promise<IServiceResponse<ICodeReq | IRunReq>> => {
        // @ts-ignore
        return fetchPostEntity<ICodeFetch, ICodeReq | IRunReq>(
            payload,
            EActionsEndpoints.RUN,
            null,
            codeModel.fromApiRun,
        );
    },
    shareCode: async (payload: ICodeShare): Promise<IServiceResponse<IShareReq>> => {
        // @ts-ignore
        return fetchPostEntity<ICodeShare, IShareReq>(
            payload,
            EActionsEndpoints.SHARE,
            null
        );
    },
    fetchShareCode: async (payload: string): Promise<IServiceResponse<ICodeShare>> => {
        // @ts-ignore
        return fetchGetEntity(
            EActionsEndpoints.SHARE_UUID.replace(':uuid', payload),
            null,
            (data) => data
        );
    }
};
