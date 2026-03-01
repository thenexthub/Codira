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

if (!('finalizeConstruction' in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, 'finalizeConstruction', () => { });
}
interface Index_Params {
    message?: string;
}
class Index extends ViewPU {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda = undefined, extraInfo) {
        super(parent, __localStorage, elmtId, extraInfo);
        if (typeof paramsLambda === 'function') {
            this.paramsGenerator_ = paramsLambda;
        }
        this.__message = new ObservedPropertySimplePU('Hello World', this, 'message');
        this.setInitiallyProvidedValue(params);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(params: Index_Params): void {
        if (params.message !== undefined) {
            this.message = params.message;
        }
    }
    updateStateVars(params: Index_Params): void {
    }
    purgeVariableDependenciesOnElmtId(rmElmtId): void {
        this.__message.purgeDependencyOnElmtId(rmElmtId);
    }
    aboutToBeDeleted(): void {
        this.__message.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __message: ObservedPropertySimplePU<string>;
    get message(): string {
        return this.__message.get();
    }
    set message(newValue: string) {
        this.__message.set(newValue);
    }
    initialRender(): void {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            RelativeContainer.create();
            RelativeContainer.height('100%');
            RelativeContainer.width('100%');
        }, RelativeContainer);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.message);
            Text.id('HelloWorld');
            Text.fontSize(50);
            Text.fontWeight(FontWeight.Bold);
            Text.alignRules({
                center: { anchor: '__container__', align: VerticalAlign.Center },
                middle: { anchor: '__container__', align: HorizontalAlign.Center }
            });
        }, Text);
        Text.pop();
        RelativeContainer.pop();
    }
    rerender(): void {
        this.updateDirtyElements();
    }
    static getEntryName(): string {
        return 'Index';
    }
}
registerNamedRoute(() => new Index(undefined, {}), '', { bundleName: 'com.example.hello', moduleName: 'entry', pagePath: 'pages/Index', pageFullPath: 'entry/src/main/ets/pages/Index', integratedHsp: 'false', moduleType: 'followWithHap' });
