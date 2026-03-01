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
// Regular
import {
    regularImportFunc1FromModule1
} from './modules/module1';
import {
    regularImportFunc1FromModule2 as MF1M2
} from './modules/module2';
import regularDefaultImportFunc1FromModule3 from './modules/module3';
import {
    default as regularDefaultImportFunc2FromModule2
} from './modules/module2';

// Namespace
import * as NS from './modules/module1';

// Local export
export function LocalExportFunc() {}
export class LocalExportClass {}
export let LocalExportLet = 2;
export let LocalExportVar;
export default class LocalDefaultExportClass {}

// Indirect export
export {
    indirectExportFunc1FromModule1
}
from './modules/module1';
export {
    indirectExportFunc2FromModule1 as MF2M1
}
from './modules/module1';

// Star export
export * as StarExport from './modules/module1';
export * as StarExport2 from './modules/module2';
export * from './modules/module2';

// External import
import ExternalImport from 'external_module';

regularImportFunc1FromModule1();
MF1M2();
regularDefaultImportFunc1FromModule3();
regularDefaultImportFunc2FromModule2();
NS.namespaceImportFunc2FromModule1();
NS.namespaceImportFunc3FromModule1();
print(LocalExportLet);
