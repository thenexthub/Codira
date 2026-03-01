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

declare namespace info {
  export const a = 1;
  export function add(x: number, y: number): number;
  export class Calculator {
    add(x: number, y: number): number;
  }
  export interface Point {
    x: number;
    y: number;
  }
  export type NumberRange = number;
  export enum Direction {
    Up,
    Down,
    Left,
    Right
  }
  namespace OpenMode {
    export const a = 1;
  }
}

export declare class Calculator {
    add(x: number, y: number): number;
}

declare namespace info1{
  export {OpenMode}
  namespace OpenMode{
    const a=1;
  }
}