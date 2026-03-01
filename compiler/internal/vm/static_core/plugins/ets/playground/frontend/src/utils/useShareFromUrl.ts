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

import { useLayoutEffect } from 'react';
import { useDispatch } from 'react-redux';
import { AppDispatch } from '../store';
import { decodeShareData } from './shareUtils';
import { setCodeAction } from '../store/actions/code';
import { setOptionsPicked } from '../store/slices/options';
import { showMessage } from '../store/actions/notification';

export const useShareFromUrl = (): void => {
    const dispatch = useDispatch<AppDispatch>();

    useLayoutEffect(() => {
        const url = new URL(window.location.href);

        const hashParams = new URLSearchParams(url.hash.startsWith('#') ? url.hash.slice(1) : url.hash);
        const searchParams = new URLSearchParams(url.search);

        const fromHash = hashParams.get('share');
        const fromSearch = searchParams.get('share');
        const shareParam = fromHash ?? fromSearch;

        if (!shareParam) {
            return;
        }
        const decodedData = decodeShareData(shareParam);

        if (decodedData) {
            dispatch(setCodeAction(decodedData.code));
            if (decodedData.options) {
                dispatch(setOptionsPicked(decodedData.options));
            }
            dispatch(
                showMessage({
                type: 'success',
                title: 'Shared code loaded',
                message: 'Code and options have been loaded from the shared link',
                duration: 3000,
                }),
            );

            if (fromHash) {
                hashParams.delete('share');
                url.hash = hashParams.toString();
            } else {
                searchParams.delete('share');
                url.search = searchParams.toString();
            }
            window.history.replaceState({}, '', url.toString());
        } else {
            dispatch(
                showMessage({
                type: 'error',
                title: 'Invalid share link',
                message: 'Failed to load code from the shared link. The link may be corrupted',
                duration: 5000,
                }),
            );
        }
    }, [dispatch]);
};
