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
@interface AI1 {
  a1F1: string
}

@interface AI3 {
  a3F1: string
}

const f1: string = 'Hello';

class C1 {
  @AI1({a1F1: 'C1F1'})
  public c1F1: string = 'public';
  protected c1F2: string = 'protected';
  private c1F3: string = 'private';
  public static c1F4: string = 'static';
  public readonly c1F6: int = 2;
  public final c1F7: int = 1;
}

interface I1 {
  @AI3({a3F1: 'interface annotation'})
  i1F1: number;
  readonly i1F2: string;
}

enum E1 {
  ONE = 0
}

namespace NS1 {
  const nS1F1: string[] = ['namespace'];
}
