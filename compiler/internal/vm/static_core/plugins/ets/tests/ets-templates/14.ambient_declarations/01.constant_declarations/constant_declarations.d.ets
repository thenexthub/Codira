/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/


/*---
desc: Ambient Constant declarations
files: ['./constant_declarations.d.ets']
tags: [compile-only, not-a-test]
---*/

export declare const x: int;
  
export declare const obj: Object;
  
export declare const a1: int, b1: float, c1: string, d1: Byte[], e1: string[];
  
export declare const a2: short, b2: double, c2: Object[];
  
export declare enum Color1 { Red, Green, Blue }
  
export declare enum Color2 { Red = 1 , Green = 2, Blue =  3 }
  
export declare enum Color3 { Red = "1" , Green = "2", Blue =  "3" }
    
export declare const enum Color4 { Red, Green, Blue }
  
export declare const QuestionOfAll1 = 42

export declare const QuestionOfAll2 = 0xBAD3
  
export declare const BigLongLiteral = 9223372036854775807
  
export declare const FloatConst1 = 3.14f
  
export declare const FloatConst2 = 3.141_592
  
export declare const FloatConst3 = .5    
  
export declare const Hello = "Hello"
  
export declare const MultilineSentence = `This is an example of multi-line string,
      which should be enclosed in backticks (\`)`
