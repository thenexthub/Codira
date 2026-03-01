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

function id<T>(v: T): T {
    return v;
}

type NullishStr = string | undefined | null;

const idNullish      = id<NullishStr>;
const idNullishArray = id<Array<NullishStr>>;
const idSettledArray = id<PromiseSettledResult<NullishStr>[]>;

const onReject = (e: Error): void => {
    console.log('Test failed: ' + e);
};

Promise.resolve<NullishStr>(null)
    .then<NullishStr>(idNullish)
    .catch(onReject);

Promise.resolve<NullishStr>(undefined)
    .then<NullishStr>(idNullish)
    .catch(onReject);

Promise.all<NullishStr>([Promise.resolve<NullishStr>(undefined)])
    .then<Array<NullishStr>>(idNullishArray)
    .catch(onReject);

Promise.all<NullishStr>([Promise.resolve<NullishStr>(null)])
    .then<Array<NullishStr>>(idNullishArray)
    .catch(onReject);

Promise.allSettled<NullishStr>([Promise.resolve<NullishStr>(undefined)])
    .then<PromiseSettledResult<NullishStr>[]>(idSettledArray)
    .catch(onReject);

Promise.allSettled<NullishStr>([Promise.resolve<NullishStr>(null)])
    .then<PromiseSettledResult<NullishStr>[]>(idSettledArray)
    .catch(onReject);

Promise.race<NullishStr>([Promise.resolve<NullishStr>(undefined)])
    .then<NullishStr>(idNullish)
    .catch(onReject);

Promise.race<NullishStr>([Promise.resolve<NullishStr>(null)])
    .then<NullishStr>(idNullish)
    .catch(onReject);

Promise.any<NullishStr>([Promise.resolve<NullishStr>(undefined)])
    .then<NullishStr>(idNullish)
    .catch(onReject);

Promise.any<NullishStr>([Promise.resolve<NullishStr>(null)])
    .then<NullishStr>(idNullish)
    .catch(onReject);
