/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

package js_proxy.test;

class jsvars {
    static global = JSRuntime.getGlobal();
};

class FooBase {
    public fn(): String { return "fn_base"; }
    public static sfn(): String { return "sfn_base"; }

    public getAsObject(): Object { return this; }
    public static createAsObject(): Object { return new FooDerived(); }

    public override toString(): String {
        return "FooBase toString";
    }

    public fnBase(): String { return "fn_base"; }
    public static sfnBase(): String { return "sfn_base"; }

    public selfFn(): String { return this.fn(); }

    public final somethingFinal(): void { }
};

class FooDerived extends FooBase {
    public override fn(): String { return "fn_derived"; }
    public static sfn(): String { return "sfn_derived"; }

    public getAsFooBase(): FooBase { return this; }
    public static createAsFooBase(): FooBase { return new FooDerived(); }

    public override toString(): String {
        return "FooDerived toString";
    }
};
