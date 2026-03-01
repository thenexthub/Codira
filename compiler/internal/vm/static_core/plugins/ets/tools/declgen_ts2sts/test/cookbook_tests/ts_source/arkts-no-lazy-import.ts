/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { foo } from "./include/lib";
import lazy { bar, qux } from "./include/lib";
import lazy { fox } from "./include/lib";

import { NormalClass } from "./include/lib";
import lazy { HumanClass, AnimalClass, DogClass } from "./include/lib";


export const vfoo = foo;
export const vbar = bar;
export const vqux = qux;
export const vfox = fox;

export const vNormalClass = new NormalClass();
export const vHumanClass = new HumanClass();
export const vAnimalClass = new AnimalClass();
export const vDogClass = new DogClass();
