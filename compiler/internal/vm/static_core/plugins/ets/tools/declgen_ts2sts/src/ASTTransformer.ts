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

import * as ts from 'typescript';

import { isVisitResultNode, visitVisitResult } from './utils/ASTHelpers';

export class Transformer {
  private readonly context: ts.TransformationContext;
  private readonly sourceFileMap: Map<string, ts.SourceFile>;
  private readonly transformers: ts.Visitor[];

  constructor(
    context: ts.TransformationContext,
    customSourceFileMap: Map<string, ts.SourceFile>,
    transformerCallbacks: ts.Visitor[]
  ) {
    this.context = context;
    this.sourceFileMap = customSourceFileMap;
    this.transformers = transformerCallbacks;
  }

  createCustomTransformer(onVisitSourceFile: (sf: ts.SourceFile) => void): ts.CustomTransformer {
    const visitor = <T extends ts.Node>(sourceFile: T): T => {
      const visitResult = ts.visitNode(sourceFile, this.visitNode.bind(this));

      if (visitResult && ts.isSourceFile(visitResult)) {
        onVisitSourceFile(visitResult);
      }

      return visitResult;
    };

    return {
      transformSourceFile: visitor,
      transformBundle: visitor
    };
  }

  visitNode(node: ts.Node): ts.VisitResult<ts.Node> {

    /* Depth-first order */

    let visitResult: ts.VisitResult<ts.Node> = ts.visitEachChild(node, this.visitNode.bind(this), this.context);

    for (const transformer of this.transformers) {
      visitResult = visitVisitResult(visitResult, transformer);
    }

    /* Reverse order */
    /*
     * let fixedNode: ts.VisitResult<ts.Node> = node;
     * fixedNode = this.autofixer.fixNode(node);
     * return ts.visitEachChild(fixedNode, this.visitNode.bind(this), this.context);
     */

    /*
     * Here we set custom source file map in order to later create custom compilerHost
     * to traverse and typecheck modified files
     */
    if (isVisitResultNode(visitResult) && ts.isSourceFile(visitResult)) {
      this.sourceFileMap?.set(
        visitResult.fileName,
        ts.createSourceFile(
          visitResult.fileName,
          ts.createPrinter().printFile(visitResult),
          visitResult.languageVersion
        )
      );
    }

    return visitResult;
  }
}
