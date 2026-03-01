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

declare enum Colors {
    Red = 'RED',
    Green = 'GREEN',
    Blue = 'BLUE'
  }
 
 
export function as1(callback:AsyncCallback<void>):void;
export function as2(callback:AsyncCallback<string>):void;
export function as3(callback:AsyncCallback<Colors>):void;
export function as4(callback:AsyncCallback<Array<number>>):void;
export function as5(callback:AsyncCallback<bigint>):void;
export function as6(callback:AsyncCallback<Uint8Array>):void;
 
declare class ASC {
    constructor(
        param1:string,
        param2:{a:number,b:string},
        param3:number,
        funcParam:()=>string,
        callback?:AsyncCallback<string>
    );
    static AS1(callback:AsyncCallback<void>):void;
}
 
declare class ForParamToJs {
    constructor(
        param1:common.Context,
        param2:Colors,
        param3:BigInt,
        param4:Uint8Array,
        param5:number[],
        param6:Array<number>,
        callback:AsyncCallback<string>,
        paramEnum:Array<Colors>,
        param7?:Array<number>,
        param8?:Colors,
        param9?:Array<UInt8>
    )
}