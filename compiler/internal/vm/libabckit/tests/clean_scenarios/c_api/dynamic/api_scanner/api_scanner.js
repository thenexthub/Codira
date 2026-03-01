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
import {
    ApiNamespace
} from './modules/apiNamespace';
import {
    bar
} from './modules/toplevelApi';
import {
    geolocation
} from '@ohos.geolocation';

class Person {
    personUseFoo() {
        ApiNamespace.foo();
    }
    personUseBar() {
        bar();
    }
    personUseClass() {
        return new ApiNamespace.MyClass();
    }
    personUseAll() {
        ApiNamespace.foo();
        bar();
        return new ApiNamespace.MyClass();
    }
    personUseLocation() {
        geolocation.getCurrentLocation();
    }
    personUseOther() {
        geolocation.getLastLocation();
    }
    personNoUse() {}
}

function useFoo() {
    ApiNamespace.foo();
}

function useBar() {
    bar();
}

function useClass() {
    return new ApiNamespace.MyClass();
}

function useLocation() {
    geolocation.getCurrentLocation();
}

function useAll() {
    ApiNamespace.foo();
    bar();
    geolocation.getCurrentLocation();
    return new ApiNamespace.MyClass();
}

function noUse() {}
