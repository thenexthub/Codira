/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

@interface Anno2{}
export namespace NS1{
    export namespace NS2{
    @interface Anno1{}
        interface Iface{
            field2:string
        }
        @Anno2
        export class C1{
            @Anno1
            f1(){}
            
            @Anno1
            field1:string;
            
        }
        const field3:string;
        function f2(){}
        export enum E1{}
        namespace NS3{
            namespace NS5{}
        }
    }
    namespace NS4{
        namespace NS5{}
    }
}