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

class Entity {
    public id: number = 1
}

function isEntity(obj: Entity) {
    return obj instanceof Entity;
}

class DataModel {
    public numericId: number = 1;
    public stringCode: string = 'default'
}

function isDataModel(obj: DataModel) {
    return obj instanceof DataModel;
}

function hasValidRecordProperty(record: Record<string, number>) {
    return record['prop'] === 1;
}

// must be exported because of explict dependence in the test
export interface IDataInterface {
    value: number;
}

function isValidDataInterface(data: IDataInterface): boolean {
    return typeof data === 'object' &&
           typeof data.value === 'number';
}

// must be exported because of explict dependence in the test
export interface IUserProfile {
    name: string;
    age: number;
}

function isValidUserProfile(profile: IUserProfile): boolean {
    return typeof profile === 'object' && 
           typeof profile.name === 'string' && 
           typeof profile.age === 'number';
}