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

export interface CSSRuleLike {
    mergeRule(rule: CSSRuleLike): void;
}

export class CSSRule implements CSSRuleLike {
    mergeRule(rule: CSSRuleLike): void {
    }
}

export class CSSLogicRule extends CSSRule {
    mergeRule(rule: CSSRule): void {
    }
}

export function createCSSLogicRule(): CSSLogicRule {
    return new CSSLogicRule();
}

export class CountOverloadRule {
    mergeRule(rule: number): boolean {
        return true;
    }

    mergeRule(rule1: number, rule2: number): boolean {
        return true;
    }
}

export class TypeOverloadRule {
    mergeRule(rule: number): boolean {
        return true;
    }

    mergeRule(rule1: string): boolean {
        return true;
    }
}