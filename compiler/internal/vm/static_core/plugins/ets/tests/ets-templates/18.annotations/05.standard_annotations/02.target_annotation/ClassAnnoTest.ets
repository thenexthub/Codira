/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

/*---
desc: Target is a standard meta-annotation that is used to annotate a declaration of another annotation.
tags: [compile-only]
---*/

import * as ads from "./Annotations"

@ads.NamespaceAnnotation
export namespace app {

    @ads.NamespaceAnnotation
    export namespace api {

        @ads.InterfaceAnnotation
        @ads.InterfaceOrClassOrStruct // Annotation allowed both for interface, class and struct
        interface MyInterface {
            @ads.InterfaceMethod
            @ads.MethodOrFunction
            someDefault() {}
            @ads.InterfaceMethod
            @ads.MethodOrFunction
            someToImplement()
        }


        @ads.ClassAnnotation
        @ads.InterfaceOrClassOrStruct // Annotation allowed both for interface, class and struct
        export final class MyClass implements MyInterface {
            @ads.ClassMethod
            @ads.MethodOrFunction
            static classStatic() {}

            @ads.ClassMethod
            @ads.MethodOrFunction
            classMethod() {}

            @ads.ClassMethod
            @ads.MethodOrFunction
            final classFinal(@ads.CallParameter echo: string) {}

            @ads.ClassMethod
            @ads.MethodOrFunction
            override someToImplement() {}
        }

        @ads.InterfaceAnnotation
        interface Person {
            @ads.PropertyOrField @ads.Property name: string;
            @ads.PropertyOrField @ads.Property age: int
        }

        @ads.ClassAnnotation
        class User {
            @ads.ClassField @ads.PropertyOrField login?: string
            @ads.ClassField @ads.PropertyOrField password?: string
        }

        @ads.ClassAnnotation
        @ads.InterfaceOrClassOrStruct
        abstract class Stub {
            @ads.ClassMethod
            @ads.MethodOrFunction
            abstract method()
        }
        @ads.ClassAnnotation
        class GetTarget {
            private _target: String;
            get target(): String { return this._target; }
        }
    }
}

@ads.MethodOrFunction
@ads.ReceiverFunction
function echo(@ads.CallParameter this: app.api.MyClass) {
}

@ads.Alias
type MyClass = app.api.MyClass

@ads.AFunction
@ads.MethodOrFunction
function main() {
    let test: MyClass = new MyClass
    test.echo()
}
