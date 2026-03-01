/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import type * as ts from 'typescript';

/**
 * NOTE: hack for typecheck of readonly arrays.
 * See more here https://github.com/microsoft/TypeScript/issues/17002
 */
declare global {
  interface ArrayConstructor {
    isArray(arg: unknown): arg is unknown[] | readonly unknown[];
  }
}

export function isVisitResultNode(vr: ts.VisitResult<ts.Node>): vr is ts.Node {
  return vr !== undefined && !Array.isArray(vr);
}

export function isVisitResultNodeArray(vr: ts.VisitResult<ts.Node>): vr is readonly ts.Node[] {
  return Array.isArray(vr);
}

export function visitVisitResult(result: ts.VisitResult<ts.Node>, visitor: ts.Visitor): ts.VisitResult<ts.Node> {
  if (result === undefined) {
    return undefined;
  }

  if (isVisitResultNode(result)) {
    return visitor(result);
  }

  if (!isVisitResultNodeArray(result)) {
    throw new Error('Unreachable');
  }

  const newResult: ts.Node[] = [];
  for (const node of result) {
    const fixedNode = visitor(node);

    if (fixedNode === undefined) {
      continue;
    }

    if (Array.isArray(fixedNode)) {
      newResult.push(...fixedNode);
      continue;
    }

    newResult.push(fixedNode);
  }

  if (newResult.length === 0) {
    return undefined;
  }

  if (newResult.length === 1) {
    return newResult[0];
  }

  return newResult;
}
