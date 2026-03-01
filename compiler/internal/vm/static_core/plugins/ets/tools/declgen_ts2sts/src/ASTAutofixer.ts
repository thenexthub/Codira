/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

import { FaultID } from '../utils/lib/FaultId';
import { visitVisitResult } from './utils/ASTHelpers';
import {
  ETSKeyword,
  FINAL_CLASS,
  ArrayType,
  JSValue,
  ESObject,
  KitPrefix,
  LIMIT_DECORATOR,
  UtilityTypes,
  SpecificTypes,
  InvalidFuncParaNames,
  BuiltInType
} from '../utils/lib/TypeUtils';

export class Autofixer {
  private readonly typeChecker: ts.TypeChecker;
  private readonly context: ts.TransformationContext;

  constructor(typeChecker: ts.TypeChecker, context: ts.TransformationContext) {
    this.typeChecker = typeChecker;
    this.context = context;
  }

  private readonly autofixes = new Map<ts.SyntaxKind, ts.Visitor[]>([
    [
      ts.SyntaxKind.VariableDeclarationList,
      [this[FaultID.VarDeclaration].bind(this), this[FaultID.VarDeclarationAssignment].bind(this)]
    ],
    [
      ts.SyntaxKind.PropertyDeclaration,
      [
        this[FaultID.PrivateIdentifier].bind(this),
        this[FaultID.NoInitializer].bind(this),
        this[FaultID.NoETSKeyword].bind(this),
        this[FaultID.PropertyAccessExpression].bind(this)
      ]
    ],
    [
      ts.SyntaxKind.SourceFile,
      [
        this[FaultID.ImportAfterStatement].bind(this),
        this[FaultID.LimitImport].bind(this),
        this[FaultID.DuplicatedDeclaration].bind(this),
        this[FaultID.DuplicatedEnum].bind(this),
        this[FaultID.EnumWithMixedType].bind(this),
        this[FaultID.LimitExtends].bind(this),
        this[FaultID.AddDeclareToTopLevelInterfaces].bind(this),
        this[FaultID.AddDeclareToTopLevelVariable].bind(this),
        this[FaultID.AddDeclareToTopLevelFunction].bind(this),
        this[FaultID.AddDeclareToTopLevelNamespace].bind(this),
        this[FaultID.AddDeclareToTopLevelClass].bind(this)
      ]
    ],
    [
      ts.SyntaxKind.LiteralType,
      [
        this[FaultID.NumbericLiteral].bind(this),
        this[FaultID.BooleanLiteral].bind(this)
      ]
    ],
    [
      ts.SyntaxKind.TypeAliasDeclaration,
      [
        this[FaultID.StringTypeAlias].bind(this),
        this[FaultID.IndexAccessType].bind(this),
        this[FaultID.ConditionalTypes].bind(this),
        this[FaultID.TypeQuery].bind(this)
      ]
    ],
    [ts.SyntaxKind.ConstructorType, [this[FaultID.ConstructorType].bind(this)]],
    [ts.SyntaxKind.TemplateLiteralType, [this[FaultID.TemplateLiteralType].bind(this)]],
    [ts.SyntaxKind.ModuleDeclaration, [this[FaultID.Module].bind(this)]],
    [
      ts.SyntaxKind.ModuleBlock,
      [
        this[FaultID.ExportNamespace].bind(this),
        this[FaultID.DuplicatedDeclaration].bind(this),
        this[FaultID.DuplicatedEnum].bind(this)
      ]
    ],
    [ts.SyntaxKind.TypeOperator, [this[FaultID.KeyofType].bind(this)]],
    [ts.SyntaxKind.TypeLiteral, [this[FaultID.TypeLiteral].bind(this)]],
    [ts.SyntaxKind.AnyKeyword, [this[FaultID.AnyToJSValue].bind(this)]],
    [ts.SyntaxKind.UnknownKeyword, [this[FaultID.UnknownToJSValue].bind(this)]],
    [
      ts.SyntaxKind.InterfaceDeclaration,
      [
        this[FaultID.MergeOverloadedMethods].bind(this),
        this[FaultID.NoETSKeyword].bind(this),
        this[FaultID.CallorOptionFuncs].bind(this),
        this[FaultID.NoConstructorInInterface].bind(this)
      ]
    ],
    [ts.SyntaxKind.Identifier, [this[FaultID.WrapperToPrimitive].bind(this)]],
    [ts.SyntaxKind.SymbolKeyword, [this[FaultID.SymbolToJSValue].bind(this)]],
    [ts.SyntaxKind.IntersectionType, [this[FaultID.IntersectionTypeJSValue].bind(this)]],
    [
      ts.SyntaxKind.TypeReference,
      [
        this[FaultID.ObjectParametersToJSValue].bind(this),
        this[FaultID.InstanceType].bind(this),
        this[FaultID.NoBuiltInType].bind(this),
        this[FaultID.ESObjectType].bind(this),
        this[FaultID.UtilityType].bind(this)
      ]
    ],
    [
      ts.SyntaxKind.FunctionDeclaration,
      [
        this[FaultID.GeneratorFunction].bind(this),
        this[FaultID.ObjectBindingParams].bind(this),
        this[FaultID.DefaultExport].bind(this),
        this[FaultID.RemoveLimitDecorator].bind(this)
      ]
    ],
    [
      ts.SyntaxKind.Parameter,
      [
        this[FaultID.ReservedFuncParameter].bind(this),
        this[FaultID.RestParameterArray].bind(this)
      ]
    ],
    [ts.SyntaxKind.TypeQuery, [this[FaultID.TypeQuery].bind(this)]],
    [ts.SyntaxKind.TypeParameter, [this[FaultID.LiteralType].bind(this)]],
    [
      ts.SyntaxKind.ClassDeclaration,
      [
        this[FaultID.MergeOverloadedConstructors].bind(this),
        this[FaultID.MergeOverloadedMethods].bind(this),
        this[FaultID.NoPrivateMember].bind(this),
        this[FaultID.AddDeclareToClass].bind(this),
        this[FaultID.NoETSKeyword].bind(this),
        this[FaultID.RemoveLimitDecorator].bind(this),
        this[FaultID.NoOptionalMemberFunction].bind(this)
      ]
    ],
    [ts.SyntaxKind.MethodDeclaration, [this[FaultID.NoETSKeyword].bind(this)]],
    [
      ts.SyntaxKind.ImportDeclaration,
      [
        this[FaultID.NoEmptyImport].bind(this),
        this[FaultID.NoLazyImport].bind(this),
      ]
    ],
    [ts.SyntaxKind.ImportSpecifier, [this[FaultID.NoETSKeyword].bind(this)]],
    [ts.SyntaxKind.ExportDeclaration, [this[FaultID.NoEmptyExport].bind(this)]],
    [ts.SyntaxKind.ExportSpecifier, [this[FaultID.NoETSKeyword].bind(this)]],
    [ts.SyntaxKind.MappedType, [this[FaultID.MappedType].bind(this)]],
    [ts.SyntaxKind.TupleType, [this[FaultID.TupleTypeToArray].bind(this)]],
    [ts.SyntaxKind.StructDeclaration, [this[FaultID.StructDeclaration].bind(this)]],
    [ts.SyntaxKind.UnionType, [this[FaultID.NoVoidUnionType].bind(this)]],
    [ts.SyntaxKind.VariableDeclaration, [this[FaultID.ConstLiteralToType].bind(this)]],
    [ts.SyntaxKind.IndexedAccessType, [this[FaultID.IndexAccessType].bind(this)]],
    [ts.SyntaxKind.FunctionType, [this[FaultID.FunctionType].bind(this)]],
    [ts.SyntaxKind.ImportType, [this[FaultID.NoImportType].bind(this)]]
  ]);

  fixNode(node: ts.Node): ts.VisitResult<ts.Node> {
    const autofixes = this.autofixes.get(node.kind);

    if (autofixes === undefined) {
      return node;
    }

    let result: ts.VisitResult<ts.Node> = node;

    for (const autofix of autofixes) {
      result = visitVisitResult(result, autofix);
    }

    return result;
  }

  /**
   * Rule: `arkts-no-var`
   */
  private [FaultID.VarDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Ensure that all variable declarations are using `let` or `const`.
     */

    if (ts.isVariableDeclarationList(node)) {
      const isLetDeclaration = node.flags & ts.NodeFlags.Let;
      const isConstDeclaration = node.flags & ts.NodeFlags.Const;

      if (!isLetDeclaration && !isConstDeclaration) {
        const newFlags = node.flags | ts.NodeFlags.Let;
        return this.context.factory.createVariableDeclarationList(node.declarations, newFlags);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-bigint-binaryExpression`
   */
  private [FaultID.VarDeclarationAssignment](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * bigint literal map to number,and binary experssion map to boolean in arkts1.2
     */

    if (ts.isVariableDeclarationList(node)) {
      const isLetDeclaration = node.flags & ts.NodeFlags.Let;
      const isConstDeclaration = node.flags & ts.NodeFlags.Const;

      // update boolean type declaration function
      if (isConstDeclaration || isLetDeclaration) {
        return transformLogicalOperators(
          node,
          this.context,
          isConstDeclaration ? ts.NodeFlags.Const : ts.NodeFlags.Let
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-ESObject-is-Any`
   */
  private [FaultID.ESObjectType](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * Replace `ESObject` type with `Any` in declarations.
     */

    if (ts.isTypeReferenceNode(node) &&
      ts.isIdentifier(node.typeName) &&
      node.typeName.escapedText === ESObject) {
      return replaceEsObjectTypeName(node, this.context.factory);
    }

    return node;
  }
  
  /**
   * Rule: `arkts-no-private-identifiers`
   */
  private [FaultID.PrivateIdentifier](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /*
     * Since we can access only public members of the imported dynamic class,
     * there is no need to fix private fields, we just do not emit them
     */

    if (ts.isPropertyDeclaration(node)) {
      if (
        ts.isPrivateIdentifier(node.name) ||
        node.modifiers?.find((v) => {
          return v.kind === ts.SyntaxKind.PrivateKeyword;
        })
      ) {
        return undefined;
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-misplaced-imports`
   */
  private [FaultID.ImportAfterStatement](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * This algorithm is very very bad for both memory and performance.
     * Redo later, when implementing other autofixes
     */

    if (ts.isSourceFile(node)) {
      const importDeclarations: ts.Statement[] = [];
      const statements: ts.Statement[] = [];

      for (const stmt of node.statements) {
        if (ts.isImportDeclaration(stmt)) {
          importDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }

      return this.context.factory.updateSourceFile(node, [...importDeclarations, ...statements]);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-limit-imports`
   */
  private [FaultID.LimitImport](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Ensure the order of import statements and remove restricted import statements.
     */

    if (ts.isSourceFile(node)) {
      const { importDeclarations, statements } = processSourceFileStatements(node.statements, this.context);
      return this.context.factory.updateSourceFile(node, [...importDeclarations, ...statements]);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-number-literal`
   */
  private [FaultID.NumbericLiteral](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * NumbericLiteral mapped to number in arkts1.2
     */

    if (ts.isLiteralTypeNode(node)) {
      const typeNode = node.literal;
      if (
        (ts.isPrefixUnaryExpression(typeNode) && typeNode.operand.kind === ts.SyntaxKind.NumericLiteral) ||
        ts.isNumericLiteral(typeNode)
      ) {
        return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-boolean-literal`
   */
  private [FaultID.BooleanLiteral](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isLiteralTypeNode(node)) {
      const literal = node.literal;
      if (literal.kind === ts.SyntaxKind.TrueKeyword || literal.kind === ts.SyntaxKind.FalseKeyword) {
        return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-conditional-types`
   */
  private [FaultID.ConditionalTypes](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * ConditionalTypes mapped to JSValue in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node)) {
      if (ts.isConditionalTypeNode(node.type)) {
        const newType = ts.factory.createTypeReferenceNode(JSValue, undefined);
        return ts.factory.createTypeAliasDeclaration(node.modifiers, node.name, node.typeParameters, newType);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-indexed-access-type`
   */
  private [FaultID.IndexAccessType](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * IndexedAccessType mapped to JSValue in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node) && ts.isIndexedAccessTypeNode(node.type)) {
      const type = this.typeChecker.getTypeAtLocation(node.type);
      const newType = this.typeChecker.typeToTypeNode(type, undefined, ts.NodeBuilderFlags.NoTruncation);
      if (newType) {
        // Create a new type alias declaration node
        return ts.factory.createTypeAliasDeclaration(node.modifiers, node.name, node.typeParameters, newType);
      }
    } else if (ts.isIndexedAccessTypeNode(node) && !ts.isLiteralTypeNode(node.indexType)) {
      return this.context.factory.createTypeReferenceNode(this.context.factory.createIdentifier(JSValue), undefined);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-module`
   */
  private [FaultID.Module](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Module mapped to namespace in arkts1.2
     */

    if (ts.isModuleDeclaration(node)) {
      const newFlags = node.flags | ts.NodeFlags.Namespace;
      return this.context.factory.createModuleDeclaration(node.modifiers, node.name, node.body, newFlags);
    }

    return node;
  }

  /**
   * `arkts-no-type-literal`
   */
  private [FaultID.TypeLiteral](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * TypeLiteral mapped to JSValue in arkts1.2
     */

    if (ts.isTypeLiteralNode(node)) {
      const createNode = this.context.factory.createIdentifier(JSValue);
      return this.context.factory.createTypeReferenceNode(createNode, undefined);
    }
    return node;
  }

  /**
   * return keyof result in TS function to JSValue
   */
  private [FaultID.KeyofType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * keyof result in TS function mapped to JSValue in arkts1.2
     */

    if (ts.isTypeOperatorNode(node)) {
      const createNode = this.context.factory.createIdentifier(JSValue);
      return this.context.factory.createTypeReferenceNode(createNode, undefined);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-export-namespace`
   */
  private [FaultID.ExportNamespace](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * In ArkTS 1.2, it is required to explicitly export namespaces in declaration files.
     */

    const statements: ts.Statement[] = [];
    let shouldChange: boolean = true;

    if (ts.isModuleBlock(node)) {
      for (const stmt of node.statements) {
        stmt.forEachChild((child) => {
          if (child.kind === ts.SyntaxKind.ExportKeyword) {
            shouldChange = false;
            return;
          }
        });
        if (!shouldChange) {
          // reset initial value
          shouldChange = true;
          statements.push(stmt);
        } else {
          let newDeclaration = transModuleBlockformation(stmt, this.context, stmt.kind);
          statements.push(newDeclaration);
        }
      }
      return this.context.factory.updateModuleBlock(node, [...statements]);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-intersection-type`
   */
  private [FaultID.IntersectionTypeJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * Intersection type mapped to JSValue in arkts1.2
     */

    if (ts.isIntersectionTypeNode(node)) {
      return this.context.factory.createTypeReferenceNode(JSValue);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-any`
   */
  private [FaultID.AnyToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'any' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.AnyKeyword) {
      return this.context.factory.createTypeReferenceNode(JSValue);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-unknown`
   */
  private [FaultID.UnknownToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'unknown' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.UnknownKeyword) {
      return this.context.factory.createTypeReferenceNode(JSValue);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-symbol`
   */
  private [FaultID.SymbolToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'symbol' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.SymbolKeyword) {
      return this.context.factory.createTypeReferenceNode(JSValue);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-duplicated-declaration`
   */
  private [FaultID.DuplicatedDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Duplicate interface and class declarations will be merged into one.
     */

    if (!ts.isSourceFile(node) && !ts.isModuleBlock(node)) {
      return node;
    }
    const statements: ts.Statement[] = [];
    const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
    const classDeclarations: ts.ClassDeclaration[] = [];
    for (const stmt of node.statements) {
      if (ts.isInterfaceDeclaration(stmt)) {
        interfaceDeclarations.push(stmt);
      } else if (ts.isClassDeclaration(stmt)) {
        classDeclarations.push(stmt);
      } else {
        statements.push(stmt);
      }
    }
    if (ts.isModuleBlock(node)) {
      const moduleBlockNode = this.context.factory.updateModuleBlock(node, [
        ...statements,
        ...findSameInterfaceAndClassList(interfaceDeclarations, this.context, ts.SyntaxKind.InterfaceDeclaration),
        ...findSameInterfaceAndClassList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration)
      ]);
      return moduleBlockNode;
    } else {
      return this.context.factory.updateSourceFile(node, [
        ...statements,
        ...findSameInterfaceAndClassList(interfaceDeclarations, this.context, ts.SyntaxKind.InterfaceDeclaration),
        ...findSameInterfaceAndClassList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration)
      ]);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-funtion-objec-JSValue`
   */
  private [FaultID.ObjectParametersToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Object parameters mapped to JSValue in arkts1.2
     */

    if (ts.isTypeReferenceNode(node)) {
      const typeName = node.typeName;
      if (ts.isIdentifier(typeName)) {
        switch (typeName.text) {
          case 'Object':
          case 'Function':
          case 'Resource':
            return ts.factory.createTypeReferenceNode(JSValue, undefined);
          default:
            return node;
        }
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-wrapperTypes`
   */
  private [FaultID.WrapperToPrimitive](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Wrapper types mapped to primitive type in arkts1.2
     */

    if (ts.isIdentifier(node)) {
      if (node.text !== undefined && node.text === 'Wrapper') {
        return ts.factory.createTypeReferenceNode(JSValue, undefined);
      }
      switch (node.text) {
        case 'Number':
          return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
        case 'String':
          return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
        case 'Boolean':
          return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
        case 'BigInt':
          return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
        default:
          return node;
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-string-type-alias`
   */
  private [FaultID.StringTypeAlias](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * StringTypeAlias mapped to string in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node)) {
      if (ts.isLiteralTypeNode(node.type)) {
        let isStringLiteral = false;
        node.type.forEachChild((child: ts.Node) => {
          if (child.kind === ts.SyntaxKind.StringLiteral) {
            isStringLiteral = true;
            return;
          }
        });
        if (isStringLiteral) {
          return ts.factory.createTypeAliasDeclaration(
            node.modifiers,
            node.name,
            node.typeParameters,
            ts.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword)
          );
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-generator-function`
   */
  private [FaultID.GeneratorFunction](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * GeneratorFunction mapped to JSValue in arkts1.2
     */

    if (ts.isFunctionDeclaration(node) && node.type) {
      const returnType = node.type;
      if (
        ts.isTypeReferenceNode(returnType) &&
        ts.isIdentifier(returnType.typeName) &&
        returnType.typeName.text === 'Generator'
      ) {
        const newType = ts.factory.createTypeReferenceNode(JSValue, undefined);
        return ts.factory.updateFunctionDeclaration(
          node,
          node.modifiers,
          undefined,
          node.name,
          node.typeParameters,
          node.parameters,
          newType,
          undefined
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-object-bind-params`
   */
  private [FaultID.ObjectBindingParams](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * ObjectBindingParams mapped to JSValue in arkts1.2
     */

    if (ts.isFunctionDeclaration(node)) {
      const parameters = node.parameters;
      const newParamters: ts.ParameterDeclaration[] = [];
      if (parameters !== undefined && parameters.length > 0) {
        parameters.forEach((parameter, index) => {
          if (ts.isObjectBindingPattern(parameter.name)) {
            const typeNode = this.context.factory.createTypeReferenceNode(
              this.context.factory.createIdentifier(JSValue),
              undefined
            );
            const currentParamter = this.context.factory.createParameterDeclaration(
              undefined,
              undefined,
              this.context.factory.createIdentifier(`args${index}`),
              undefined,
              typeNode
            );
            newParamters.push(currentParamter);
          } else {
            newParamters.push(parameter);
          }
        });

        return this.context.factory.updateFunctionDeclaration(
          node,
          node.modifiers,
          node.asteriskToken,
          node.name,
          node.typeParameters,
          newParamters,
          node.type,
          node.body
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-type-query`
   */
  private [FaultID.TypeQuery](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * TypeQuery mapped to Reflect in arkts1.2
     */
    const identifiers = new Set(['Reflect', 'Promise']);
    if (ts.isTypeQueryNode(node) && ts.isIdentifier(node.exprName)) {
      if (identifiers.has(node.exprName.text)) {
        return this.context.factory.createTypeReferenceNode(node.exprName.text, undefined);
      } else {
        return this.context.factory.createTypeReferenceNode(JSValue, undefined);
      }
    }

    return node;
  }

  /**
 * Rule: `arkts-no-instance-type`
 */
  private [FaultID.InstanceType](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * For InstanceType, convert them to JSValue
     */
    if (ts.isTypeReferenceNode(node) && node.typeName !== undefined) {
      const referenceNode = node.typeName as ts.Identifier;
      if (referenceNode.text === 'InstanceType') {
        return ts.factory.createTypeReferenceNode(JSValue);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-mappedtype`
   */
  private [FaultID.MappedType](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * For mapped types, convert them to JSValue
     */
    if (ts.isMappedTypeNode(node)) {
      return ts.factory.createTypeReferenceNode(JSValue);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-literal-type`
   */
  private [FaultID.LiteralType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Union literal type mapped to JSValue in arkts1.2
     */
    if (ts.isTypeParameterDeclaration(node)) {
      const { unionTypeNode, typeNode } = extractTypeNodes(node, this.context);

      if (unionTypeNode === undefined && typeNode !== undefined) {
        return createTypeParameterDeclaration(node, undefined, typeNode, this.context);
      } else if (unionTypeNode !== undefined && typeNode === undefined) {
        return createTypeParameterDeclaration(node, unionTypeNode, undefined, this.context);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-initializer`
   */
  private [FaultID.NoInitializer](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For member variables with initial assignment, convert literals to their corresponding data types.
     */

    if (!ts.isPropertyDeclaration(node)) {
      return node;
    }

    const nodeType = inferNodeTypeFromInitializer(node, this.context);

    if (nodeType !== undefined) {
      return this.context.factory.updatePropertyDeclaration(
        node,
        node.modifiers,
        node.name,
        undefined,
        nodeType,
        undefined
      );
    }

    return node;
  }

  /*
   * Rule: `For enums with heterogeneous types, convert them to JSValue`
   */
  private [FaultID.EnumWithMixedType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For enums with heterogeneous types, convert them to JSValue
     */

    if (ts.isSourceFile(node)) {
      const { mixedEnumNames, firstPassStatements } = findMixedEnums(node as ts.SourceFile);
      const firstPassSourceFile = ts.factory.updateSourceFile(node, firstPassStatements);
      const visit = (typeNode: ts.Node): ts.VisitResult<ts.Node> => {
        if (ts.isTypeReferenceNode(typeNode)) {
          const typeName = typeNode.typeName;
          if (ts.isIdentifier(typeName) && mixedEnumNames.includes(typeName.text)) {
            return ts.factory.createTypeReferenceNode(ts.factory.createIdentifier(JSValue), undefined);
          }
        }
        return ts.visitEachChild(typeNode, visit, this.context);
      };
      let statements = firstPassSourceFile.statements.slice();
      for (let i = 0; i < statements.length; i++) {
        const stmt = statements[i];
        const newStmt = visit(stmt);
        if (newStmt) {
          statements[i] = newStmt as ts.Statement;
        }
      }
      return ts.factory.updateSourceFile(firstPassSourceFile, statements);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-duplicated-enum`
   */
  private [FaultID.DuplicatedEnum](node: ts.Node): ts.VisitResult<ts.Node> {

    /**
     * For duplicated enums, convert them to one enum
     */

    const statements: ts.Statement[] = [];
    const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
    const classDeclarations: ts.ClassDeclaration[] = [];
    const enumDeclarations: ts.EnumDeclaration[] = [];
    if (ts.isSourceFile(node) || ts.isModuleBlock(node)) {
      for (const stmt of node.statements) {
        if (ts.isInterfaceDeclaration(stmt)) {
          interfaceDeclarations.push(stmt);
        } else if (ts.isClassDeclaration(stmt)) {
          classDeclarations.push(stmt);
        } else if (ts.isEnumDeclaration(stmt)) {
          enumDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }
      if (ts.isModuleBlock(node)) {
        const moduleBlockNode = this.context.factory.updateModuleBlock(node, [
          ...statements,
          ...findSameInterfaceOrClassOrEnumList(
            interfaceDeclarations,
            this.context,
            ts.SyntaxKind.InterfaceDeclaration
          ),
          ...findSameInterfaceOrClassOrEnumList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration),
          ...findSameInterfaceOrClassOrEnumList(enumDeclarations, this.context, ts.SyntaxKind.EnumDeclaration)
        ]);
        return moduleBlockNode;
      } else {
        return this.context.factory.updateSourceFile(node, [
          ...statements,
          ...findSameInterfaceOrClassOrEnumList(
            interfaceDeclarations,
            this.context,
            ts.SyntaxKind.InterfaceDeclaration
          ),
          ...findSameInterfaceOrClassOrEnumList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration),
          ...findSameInterfaceOrClassOrEnumList(enumDeclarations, this.context, ts.SyntaxKind.EnumDeclaration)
        ]);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-private-members`
   */
  private [FaultID.NoPrivateMember](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * For members modified by private, they do not appear in the declaration file.
     */
    if (ts.isClassDeclaration(node)) {
      const newMembers = node.members.filter((member) => {
        if (!ts.canHaveModifiers(member)) {
          return true;
        }
        const modifiers = ts.getModifiers(member);
        return !(modifiers === null || modifiers === void 0 ?
          void 0 :
          modifiers.some((modifier) => {
            return modifier.kind === ts.SyntaxKind.PrivateKeyword;
          }));
      });
      
      const decorators = ts.getAllDecorators(node) ?? [];
      const modifiers = ts.canHaveModifiers(node) ? (ts.getModifiers(node) ?? []) : [];

      const modifierLike: ts.NodeArray<ts.ModifierLike> | undefined = (decorators.length || modifiers.length) ?
        ts.factory.createNodeArray<ts.ModifierLike>([...decorators, ...modifiers]) : undefined;

      return this.context.factory.updateClassDeclaration(
        node,
        modifierLike,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers
      );
    }

    return node;
  }

  /**
   * Rule: `arkts-no-etskeyword`
   */
  private [FaultID.NoETSKeyword](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Remove some ets keyword from the declaration.
     */

    if (
      ts.isClassDeclaration(node) ||
      ts.isInterfaceDeclaration(node) ||
      ts.isMethodDeclaration(node) ||
      ts.isPropertyDeclaration(node) ||
      ts.isImportSpecifier(node) ||
      ts.isExportSpecifier(node)
    ) {
      return restrictIdentifierName(node);
    }
    return node;
  }

  /**
   * Rule: `arkts-ESObject-to-object-type`
   */
  private [FaultID.DefaultExport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isFunctionDeclaration(node) || ts.isClassDeclaration(node)) {
      return exportDefaultAssignment(node, this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-add-declare-to-classes`
   */
  private [FaultID.AddDeclareToClass](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isClassDeclaration(node)) {
      return node;
    }

    const modifiers = node.modifiers;
    if (!modifiers) {
      return node;
    }

    const isExported = modifiers.some(m => m.kind === ts.SyntaxKind.ExportKeyword);
    if (!isExported) {
      return node;
    }
    const hasDeclare = modifiers.some(m => m.kind === ts.SyntaxKind.DeclareKeyword);
    if (hasDeclare) {
      return node;
    }

    const isDefaultExport = modifiers.some(m => m.kind === ts.SyntaxKind.DefaultKeyword);
    
    if (isDefaultExport) {
      // For default export class, add declare after 'export default'
      return exportDefaultAssignment(node, this.context);
    } else {
      return node;
    }
  }

  /**
   * Rule: `arkts-add-declare-to-exported-variables`
   */
  private [FaultID.AddDeclareToTopLevelVariable](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map(stmt => {
      if (ts.isVariableStatement(stmt) && stmt.modifiers?.some(m => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some(m => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateVariableStatement(
          stmt,
          newModifiers,
          stmt.declarationList
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-add-declare-to-exported-functions`
   */
  private [FaultID.AddDeclareToTopLevelFunction](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map(stmt => {
      if (ts.isFunctionDeclaration(stmt) && stmt.modifiers?.some(m => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some(m => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        if (stmt.modifiers.some(m => m.kind === ts.SyntaxKind.DefaultKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateFunctionDeclaration(
          stmt,
          newModifiers,
          stmt.asteriskToken,
          stmt.name,
          stmt.typeParameters,
          stmt.parameters,
          stmt.type,
          stmt.body
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-no-call-signature-or-optional-methods`
   */
  private [FaultID.CallorOptionFuncs](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Convert interfaces with call signatures or optional methods to JSValue
     */

    if (ts.isInterfaceDeclaration(node)) {
      const updatedMembers: Array<ts.TypeElement> = []
      for (const member of node.members) {
        if (ts.isIndexSignatureDeclaration(member) ||
          ts.isCallSignatureDeclaration(member)) {
          /**
           * If the header comment of an interface declaration contains `@noninterop` field, 
           * the interface node will not be converted.
           */
          if (isNonInterop(node)) {
            return undefined;
          }
          const typeAliasDeclaration = ts.factory.createTypeAliasDeclaration(
          node.modifiers,
          node.name,
          node.typeParameters,
          ts.factory.createTypeReferenceNode(JSValue)
          );

          return typeAliasDeclaration;
        }
        if (!(ts.isMethodSignature(member) && member.questionToken)) {
          updatedMembers.push(member);
        }
      }
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers
      )
    }

    return node;
  }

  /**
    * Rule: `arkts-no-ts-utility-type`
    */
  private [FaultID.UtilityType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Map TS utility type to JSValue in arkts1.2
     */

    if (ts.isTypeReferenceNode(node) && isUtilityTypes(node)) {
      return createNodeFromUtilType(
        UtilityTypes.get((node.typeName as ts.Identifier).text)!,
        this.context.factory
      );
    }
    return node;
  }

  /**
   * Rule: `arkts-no-constructor-type`
   */
  private [FaultID.ConstructorType](node: ts.Node): ts.VisitResult<ts.Node> {
    return this.context.factory.createTypeReferenceNode(
      this.context.factory.createIdentifier(JSValue),
      undefined
    );
  }

  /**
   * Rule: `arkts-no-constructor-in-interface`
   */
  private [FaultID.NoConstructorInInterface](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Remove constructor method signatures from interface declarations
     */

    if (ts.isInterfaceDeclaration(node)) {
      const updatedMembers: Array<ts.TypeElement> = [];
      for (const member of node.members) {
        if (ts.isMethodSignature(member) &&
            ts.isIdentifier(member.name) &&
            member.name.text === 'constructor') {
          continue;
        }
        updatedMembers.push(member);
      }
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers
      );
    }

    return node;
  }

  /**
   * Rule: `arkts-no-template-literal-type`
   */
  private [FaultID.TemplateLiteralType](node: ts.Node): ts.VisitResult<ts.Node> {
    return this.context.factory.createTypeReferenceNode(
      this.context.factory.createIdentifier(JSValue),
      undefined
    );
  }

  /**
   * Rule: `arkts-no-empty-export`
   */
  private [FaultID.NoEmptyExport](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Remove empty export statements
     */

    if (ts.isExportDeclaration(node)) {
      const exportClause = node.exportClause;
      if (exportClause && ts.isNamedExports(exportClause)) {
        if (exportClause.elements.length === 0) {
          // If the named exports are empty, return undefined to remove this export statement
          return undefined;
        }
      }
    }

    return node;
  }

  /*
   * Rule: `arkts-no-limit-extends`
   */
  private [FaultID.LimitExtends](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * If some classes inherit from special classes or interfaces,
     * these classes will be directly removed from the declaration file.
     */

    if (ts.isSourceFile(node)) {
      const importDeclarations: ts.ImportDeclaration[] = [];
      const classDeclarations: ts.ClassDeclaration[] = [];
      const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
      const statements: ts.Statement[] = [];
      for (const stmt of node.statements) {
        if (ts.isImportDeclaration(stmt)) {
          importDeclarations.push(stmt);
        } else if (ts.isInterfaceDeclaration(stmt)) {
          interfaceDeclarations.push(stmt);
        } else if (ts.isClassDeclaration(stmt)) {
          classDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }

      const importSpecifierNames = getImportSpecifierNames(importDeclarations);

      return this.context.factory.updateSourceFile(node, [
        ...importDeclarations,
        ...statements,
        ...tranClassDeclarationList(classDeclarations, this.context, importSpecifierNames),
        ...tranInterfaceDeclarationList(interfaceDeclarations, this.context, importSpecifierNames)])
    }

    return node;
  }

  /**
   * Rule: `arkts-no-empty-import`
   */
  private [FaultID.NoEmptyImport](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Remove empty import statements
     */

    if (ts.isImportDeclaration(node)) {
      const importClause = node.importClause;
      if (importClause?.namedBindings && ts.isNamedImports(importClause.namedBindings)) {
        if (importClause.namedBindings.elements.length === 0) {
          // If the named imports are empty, return undefined to remove this import statement
          return undefined;
        }
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-lazy-import`
   */
  private [FaultID.NoLazyImport](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Remove lazy property from import statements
     */
    if (ts.isImportDeclaration(node)) {
      const importClause = node.importClause;
      if (importClause && importClause.isLazy) {
        const newImportClause = this.context.factory.createImportClause(
          importClause.isTypeOnly,
          importClause.name,
          importClause.namedBindings
        );
        return this.context.factory.updateImportDeclaration(
          node,
          node.modifiers,
          newImportClause,
          node.moduleSpecifier,
          node.assertClause
        );
      }
    }
    return node;
  }

  /*
   * Rule: `arkts-no-property-access-expression`
   */
  private [FaultID.PropertyAccessExpression](node: ts.Node): ts.VisitResult<ts.Node> {

    /**
     * Property access expression convert to specific type
     */

    if (ts.isPropertyDeclaration(node)) {
      const initializer = node.initializer;
      if (!initializer || initializer.kind !== ts.SyntaxKind.PropertyAccessExpression) {
        return node;
      }

      const updatedInitializer = updatePropertyAccessExpression(initializer as ts.PropertyAccessExpression, this.context);
      if (updatedInitializer) {
        return this.context.factory.updatePropertyDeclaration(
          node,
          node.modifiers,
          node.name,
          node.questionToken,
          updatedInitializer,
          undefined
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-built-in-type`
   */
  private [FaultID.NoBuiltInType](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * Built in types will convert to ESObject
     */

    if (ts.isTypeReferenceNode(node)) {
      const typeName = node.typeName;
      if (ts.isIdentifier(typeName) && BuiltInType.includes(typeName.text)) {
        const newTypeNode = ts.factory.createTypeReferenceNode(JSValue, undefined);
        return ts.factory.updateTypeReferenceNode(
          node,
          newTypeNode.typeName,
          newTypeNode.typeArguments
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts:add-declare-to-top-level-interfaces`
   */
  private [FaultID.AddDeclareToTopLevelInterfaces](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map(stmt => {
      if (ts.isInterfaceDeclaration(stmt) && stmt.modifiers?.some(m => m.kind === ts.SyntaxKind.ExportKeyword)) {
        // If 'declare' already exists, do not add it again
        if (stmt.modifiers.some(m => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        // Transform interface nodes by adding the declare keyword
        const newModifiers = [
          ...stmt.modifiers.filter(m => ts.isModifier(m)),
          this.context.factory.createModifier(ts.SyntaxKind.DeclareKeyword)
        ] as ts.Modifier[];

        return this.context.factory.updateInterfaceDeclaration(
          stmt,
          newModifiers,
          stmt.name,
          stmt.typeParameters,
          stmt.heritageClauses,
          stmt.members
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `remove-limit-decorator`
   */
  private [FaultID.RemoveLimitDecorator](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isFunctionDeclaration(node) && !ts.isClassDeclaration(node)) {
      return node;
    }

    let decorators: ts.Decorator[] = [];

    const illegalDecorators = ts.getAllDecorators(node)
    decorators = illegalDecorators?.filter(ts.isDecorator) as ts.Decorator[];
    // Filter out restricted decorators
    const filteredDecorators = decorators?.filter((decorator) => {
      const expression = decorator.expression;
      return !(ts.isIdentifier(expression) && LIMIT_DECORATOR.includes(expression.text));
    });
    (node as any).illegalDecorators = filteredDecorators;

    return node;
  }

  /**
   * Rule: `arkts-no-optional-member-function`
   */
  private [FaultID.NoOptionalMemberFunction](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Member functions with the optional question token will no longer be supported in ArkTS 1.2.
     * Remove such methods from class declarations.
     */
    if (ts.isClassDeclaration(node)) {
      const updatedMembers = node.members.filter(isNotOptionalMemberFunction);
      return this.context.factory.updateClassDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers
      );
    }
    return node;
  }

  /**
   * Rule: `arkts-reserved-function-parameter-name`
   */
  private [FaultID.ReservedFuncParameter](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Some of the function parameters are reserved keywords in ArkTS 1.2.
     * decorate them with underscore prefix to avoid conflicts.
     */
    if (
      ts.isParameter(node) &&
      ts.isIdentifier(node.name) &&
      InvalidFuncParaNames.has(node.name.text)
    ) {
      const updatedNode = this.context.factory.updateParameterDeclaration(
        node,
        node.modifiers,
        node.dotDotDotToken,
        this.context.factory.createIdentifier(`_${node.name.text}`),
        node.questionToken,
        node.type,
        node.initializer
      );
      return updatedNode;
    }
    return node;
  }

  /**
   * Rule: `arkts-rest-parameter-array`
   */
  private [FaultID.RestParameterArray](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Rest parameters named `ESObject` will be converted to `Any[]` type in ArkTS 1.2.
     */
    if (
      ts.isParameter(node) &&
      node.dotDotDotToken &&
      node.type &&
      ts.isTypeReferenceNode(node.type)
    ) {
      const typeName = node.type.typeName;
      if (
        ts.isIdentifier(typeName) &&
        (typeName.text === ESObject || typeName.text === JSValue)
      ) {
        const typeNode = this.context.factory.createArrayTypeNode(
          this.context.factory.createTypeReferenceNode(JSValue, undefined)
        );
        return this.context.factory.updateParameterDeclaration(
          node,
          node.modifiers,
          node.dotDotDotToken,
          node.name,
          node.questionToken,
          typeNode,
          node.initializer
        );
      }
    }
    return node;
  }

  /**
   * Rule: `tuple map to Array in type annotation`
   */
  private [FaultID.TupleTypeToArray](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTupleTypeNode(node)) {
      return this.context.factory.createTypeReferenceNode(
        this.context.factory.createIdentifier('Array'),
        [this.context.factory.createTypeReferenceNode(JSValue)]
      );
    }

    return node;
  }
  
  /**
   * Rule: `keeping the struct in its original form`
   */
  private [FaultID.StructDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isStructDeclaration(node)) {

    const filteredMembers = node.members.filter(member => {
      return !ts.isConstructorDeclaration(member);
    });

    // Create a new struct node, removing only the constructor
    const newStruct = ts.factory.updateStructDeclaration(
      node,
      node.modifiers,
      node.name,
      node.typeParameters,
      node.heritageClauses,
      ts.factory.createNodeArray(filteredMembers)
    );

      return newStruct;
    }

    return node;
  }

  /**
   * Rule: `union type with void mapped to Any`
   */
  private [FaultID.NoVoidUnionType](node: ts.Node): ts.VisitResult<ts.Node> {
    
    /**
     * If a union type contains the void type,
     * convert the union type to Any.
     */
    if (ts.isUnionTypeNode(node)) {
      const hasVoid = node.types.some((type) => type.kind === ts.SyntaxKind.VoidKeyword);
      if (hasVoid) {
        return this.context.factory.createTypeReferenceNode(JSValue);
      }
    }
    
    return node;
  }  

  /*      
  * Rule: `arkts-const-literal-to-type`
  */
  private [FaultID.ConstLiteralToType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Convert const variable declarations with literal values to type declarations.
     * e.g. export declare const c = 123; -> export declare const c: number;
     */
    
    if (ts.isVariableDeclaration(node) && node.initializer) {
      let typeNode: ts.TypeNode | undefined;

      if (ts.isNumericLiteral(node.initializer)) {
        typeNode = this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      }

      if (typeNode) {
        const result = this.context.factory.createVariableDeclaration(
          node.name,
          node.exclamationToken,
          typeNode,
          undefined
        );

        return result;
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-generic-function-type`
   */
  private [FaultID.FunctionType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For function types with generic parameters, convert them to Any
     */
    if (ts.isFunctionTypeNode(node)) {
      if (node.typeParameters && node.typeParameters.length > 0) {
        return this.context.factory.createTypeReferenceNode(
          this.context.factory.createIdentifier(JSValue),
          undefined
        );
      }

      let hasImportType = false;
      const visitNode = (childNode: ts.Node): void => {
        if (ts.isImportTypeNode(childNode)) {
          hasImportType = true;
          return;
        }
        ts.forEachChild(childNode, visitNode);
      };

      node.parameters.forEach(param => {
        if (param.type) {
          visitNode(param.type);
        }
      });
    
      if (node.type) {
        visitNode(node.type);
      }
    
      if (hasImportType) {
        return this.context.factory.createTypeReferenceNode(
          this.context.factory.createIdentifier(JSValue),
          undefined
        );
      }
    }
  
    return node;
  }

  /**
   * Rule: `Merge overloaded methods in interfaces and classes`
   */
  private [FaultID.MergeOverloadedMethods](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isInterfaceDeclaration(node)) {
      const hasOverloads = node.members.some((member) => {
        if ((ts.isMethodSignature(member) || ts.isMethodDeclaration(member)) && ts.isIdentifier(member.name)) {
          const memberName = member.name.escapedText;
          return (
            node.members.filter(
              (m: ts.TypeElement): m is typeof member =>
                (ts.isMethodSignature(m) || ts.isMethodDeclaration(m)) &&
                ts.isIdentifier(m.name) &&
                m.name.escapedText === memberName
            ).length > 1
          );
        }
        return false;
      });

      if (!hasOverloads) {
        return node;
      }

      const mergedMembers = mergeOverloadedMembers(node.members, this.context) as ts.NodeArray<ts.TypeElement>;
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        mergedMembers
      );
    } else if (ts.isClassDeclaration(node)) {
      const hasOverloads = node.members.some((member) => {
        if ((ts.isMethodSignature(member) || ts.isMethodDeclaration(member)) && ts.isIdentifier(member.name)) {
          const memberName = member.name.escapedText;
          return (
            node.members.filter(
              (m: ts.ClassElement): m is typeof member =>
                (ts.isMethodSignature(m) || ts.isMethodDeclaration(m)) &&
                ts.isIdentifier(m.name) &&
                m.name.escapedText === memberName
            ).length > 1
          );
        }
        return false;
      });

      if (!hasOverloads) {
        return node;
      }

      const mergedMembers = mergeOverloadedMembers(node.members, this.context) as ts.NodeArray<ts.ClassElement>;
      return this.context.factory.updateClassDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        mergedMembers
      );
    }
    return node;
  }

  /**
   * Rule: `Merge overloaded constructors classes`
   */
  private [FaultID.MergeOverloadedConstructors](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isClassDeclaration(node)) {
      const constructors = node.members.filter(ts.isConstructorDeclaration);
      const nonConstructors = node.members.filter((member) => !ts.isConstructorDeclaration(member));
      if (constructors.length <= 1) {
        return node;
      }

      const mergedConstructor = mergeSignatures(constructors, this.context) as ts.ConstructorDeclaration

      return this.context.factory.updateClassDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        this.context.factory.createNodeArray<ts.ClassElement>([
          mergedConstructor,
          ...nonConstructors
        ])
      );

    }
    return node
  }

  /**
   * Rule: `arkts-add-declare-to-exported-top-lavel-namespaces`
   */
  private [FaultID.AddDeclareToTopLevelNamespace](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isModuleDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateModuleDeclaration(stmt, undefined, newModifiers, stmt.name, stmt.body);
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-add-declare-to-exported-top-lavel-classes`
   */
  private [FaultID.AddDeclareToTopLevelClass](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isClassDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier as ts.Modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateClassDeclaration(
          stmt,
          newModifiers,
          stmt.name,
          stmt.typeParameters,
          stmt.heritageClauses,
          stmt.members
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /*
   * Rule: `Convert import type to Any`
   */
  private [FaultID.NoImportType](node: ts.Node): ts.VisitResult<ts.Node> {
    return this.context.factory.createTypeReferenceNode(this.context.factory.createIdentifier(JSValue));
  }
}

/**
 * Modify the ModuleBlock node to add Export so that it can be referenced by external methods.
 */
function transModuleBlockformation(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind,
): ts.Statement {
  const exportToken = context.factory.createToken(ts.SyntaxKind.ExportKeyword);
  return updateNodetoAddExport(stmt, context, kindType, exportToken);
}

/**
 * Extract interface members together.
 */
function extractInterfaceMembers(item: ts.InterfaceDeclaration, methods: ts.TypeElement[]): void {
  item.members.forEach((member) => {
    if ((ts.isMethodSignature(member) || ts.isPropertySignature(member)) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (
        !methods.some(
          (m) =>
            isSameFunction(m, member) ||
            (ts.isPropertySignature(m) && ts.isPropertySignature(member) && m.name.getText() === memberName)
        )
      ) {
        methods.push(member);
      }
    }
  });
}

/**
 * Extract class members together.
 */
function extractClassMembers(item: ts.ClassDeclaration, classMethods: ts.ClassElement[]): void {
  item.members.forEach((member) => {
    if ((ts.isMethodDeclaration(member) || ts.isPropertyDeclaration(member)) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (
        !classMethods.some(
          (m) =>
            (ts.isMethodDeclaration(m) && ts.isMethodDeclaration(member) && m.name.getText() === memberName) ||
            (ts.isPropertyDeclaration(m) && ts.isPropertyDeclaration(member) && m.name.getText() === memberName)
        )
      ) {
        classMethods.push(member);
      }
    }
  });
}

function isSameFunction(m: ts.TypeElement, member: ts.TypeElement): boolean {
  const memberName = (member.name as ts.Identifier).text;
  const mName = (m.name as ts.Identifier).text;

  if (mName !== memberName) {
    return false;
  }
  if (!ts.isMethodSignature(m) || !ts.isMethodSignature(member)) {
    return false;
  }
  if (!compareParameters(m.parameters, member.parameters)) {
    return false;
  }
  if (m.type && member.type) {
    if (!typesAreEqual(m.type, member.type)) {
      return false;
    }
  } else if (!!m.type !== !!member.type) {
    return false;
  }
  if (!compareTypeParameters(m.typeParameters, member.typeParameters)) {
    return false;
  }
  return true;
}

function compareParameters(
  params1: ts.NodeArray<ts.ParameterDeclaration>,
  params2: ts.NodeArray<ts.ParameterDeclaration>
): boolean {
  if (params1.length !== params2.length) {
    return false;
  }
  for (let i = 0; i < params1.length; i++) {
    const p1 = params1[i];
    const p2 = params2[i];
    if (p1.name.getText() !== p2.name.getText()) {
      return false;
    }
    if (!typesAreEqual(p1.type, p2.type)) {
      return false;
    }

    if (p1.questionToken !== p2.questionToken || p1.initializer !== p2.initializer) {
      return false;
    }
  }

  return true;
}

function typesAreEqual(type1: ts.TypeNode | undefined, type2: ts.TypeNode | undefined): boolean {
  if (!type1 || !type2) {
    return type1 === type2;
  }

  return type1.getText() === type2.getText();
}

function compareTypeParameters(
  params1: ts.NodeArray<ts.TypeParameterDeclaration> | undefined,
  params2: ts.NodeArray<ts.TypeParameterDeclaration> | undefined
): boolean {
  if (!params1 && !params2) {
    return true;
  }
  if (!params1 || !params2) {
    return false;
  }
  if (params1.length !== params2.length) {
    return false;
  }

  for (let i = 0; i < params1.length; i++) {
    const p1 = params1[i];
    const p2 = params2[i];

    if (p1.name.getText() !== p2.name.getText()) {
      return false;
    }
  }

  return true;
}

/**
 * Find a list of interfaces or classes with the same name
 */
function findSameInterfaceAndClassList(
  statements: ts.Statement[],
  context: ts.TransformationContext,
  kindType: number
): ts.Statement[] {
  const sameInterfaceMap = new Map<string, { count: number; nodes: ts.Statement[] }>();
  statements.forEach((statement) => {
    let name: string | undefined;
    const isInterfaceDeclaration = ts.isInterfaceDeclaration(statement) && statement.name;
    const isClassDeclaration = ts.isClassDeclaration(statement) && statement.name;
    if (isInterfaceDeclaration || isClassDeclaration) {
      name = statement.name.text;
    }
    if (name) {
      if (!sameInterfaceMap.has(name)) {
        sameInterfaceMap.set(name, { count: 0, nodes: [] });
      }
      const entry = sameInterfaceMap.get(name)!;
      entry.count++;
      entry.nodes.push(statement);
    }
  });
  const uniqueNodes: ts.Statement[] = [];
  const newNodes: ts.Statement[] = [];
  sameInterfaceMap.forEach((entry) => {
    if (entry.count === 1) {
      uniqueNodes.push(...entry.nodes);
    } else {
      // Reorganize interface information
      const newInterface = createCombinedInterfaceAndClass(entry.nodes, context, kindType);
      newNodes.push(newInterface);
    }
  });

  return [...uniqueNodes, ...newNodes];
}

/**
 * Modify the node node and add the export keyword.
 */
function updateNodetoAddExport(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind,
  exportToken: ts.Modifier
): ts.Statement {
  switch (kindType) {
    case ts.SyntaxKind.VariableDeclaration:
    case ts.SyntaxKind.VariableStatement:
      return updateVariableOrStatement(stmt, context, exportToken);
    case ts.SyntaxKind.ModuleDeclaration:
      return updateModuleDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.FunctionDeclaration:
      return updateFunctionDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.InterfaceDeclaration:
      return updateInterfaceDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.ClassDeclaration:
      return updateClassDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.TypeAliasDeclaration:
      return updateTypeAliasDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.EnumDeclaration:
      return updateEnumDeclaration(stmt, context, exportToken);
    default:
      return stmt;
  }
}

function updateVariableOrStatement(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isVariableStatement(stmt)) {
    return context.factory.updateVariableStatement(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.declarationList
    );
  }
  throw new Error('Unsupported statement type');
}

function updateModuleDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isModuleDeclaration(stmt)) {
    return context.factory.updateModuleDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.body
    );
  }
  return stmt;
}

function updateFunctionDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isFunctionDeclaration(stmt)) {
    return context.factory.updateFunctionDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.asteriskToken,
      stmt.name,
      stmt.typeParameters,
      stmt.parameters,
      stmt.type,
      stmt.body
    );
  }
  return stmt;
}

function updateInterfaceDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isInterfaceDeclaration(stmt)) {
    return context.factory.updateInterfaceDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.heritageClauses,
      stmt.members
    );
  }
  return stmt;
}

function updateClassDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isClassDeclaration(stmt)) {
    return context.factory.updateClassDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.heritageClauses,
      stmt.members
    );
  }
  return stmt;
}

function updateTypeAliasDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isTypeAliasDeclaration(stmt)) {
    return context.factory.updateTypeAliasDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.type
    );
  }
  return stmt;
}

function updateEnumDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isEnumDeclaration(stmt)) {
    return context.factory.updateEnumDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.members
    );
  }
  return stmt;
}

/**
 * Determine whether the type reference needs to be replaced.
 */
function shouldReplaceTypeReference(typeName: ts.EntityName, typeAliasMap: Map<string, string>): boolean {
  if (ts.isIdentifier(typeName)) {
    return typeAliasMap.has(typeName.text);
  } else if (ts.isQualifiedName(typeName)) {
    const firstIdentifier = getFirstIdentifierInQualifiedName(typeName);
    if (firstIdentifier !== undefined) {
      return typeAliasMap.has(firstIdentifier.text);
    }
  }
  return false;
}

/**
 * Replace the type reference
 */
function replaceTypeReference(node: ts.TypeReferenceNode, context: ts.TransformationContext): ts.Node {
  return context.factory.createTypeReferenceNode(context.factory.createIdentifier(JSValue), undefined);
}

/**
 * Traverse the nodes
 */
function traverseNode(node: ts.Node, context: ts.TransformationContext, typeAliasMap: Map<string, string>): ts.Node {
  if (ts.isTypeReferenceNode(node)) {
    const typeName = node.typeName;
    if (shouldReplaceTypeReference(typeName, typeAliasMap)) {
      return replaceTypeReference(node, context);
    }
  }
  return ts.visitEachChild(node, (child) => traverseNode(child, context, typeAliasMap), context);
}

/**
 * Replace the type reference with JSValue
 */
function replaceTypeReferenceWithObject(
  node: ts.Node,
  context: ts.TransformationContext,
  typeAliasMap: Map<string, string>
): ts.Node {
  return traverseNode(node, context, typeAliasMap);
}

/**
 * Get the first identifier in a qualified name
 */
function getFirstIdentifierInQualifiedName(qualifiedName: ts.QualifiedName): ts.Identifier | undefined {
  let current: ts.Node = qualifiedName;
  while (ts.isQualifiedName(current)) {
    current = current.left;
  }
  return ts.isIdentifier(current) ? current : undefined;
}

/**
 * Check if the module import is limited
 */
function isLimitImport(moduleSpecifier: ts.Expression, limitImport: string[]): boolean {
  for (const item of limitImport) {
    if (ts.isStringLiteral(moduleSpecifier) && moduleSpecifier.text.startsWith(item)) {
      return true;
    }
  }

  return false;
}

/**
 * Check if the node is of bigint type.
 */
function isBigintType(v: ts.Node): boolean {
  if (ts.isLiteralTypeNode(v)) {
    const literalTypeNode = v as ts.LiteralTypeNode;
    if (literalTypeNode.literal && literalTypeNode.literal.kind !== undefined) {
      if (literalTypeNode.literal.kind === ts.SyntaxKind.PrefixUnaryExpression) {
        const prefix = literalTypeNode.literal as ts.PrefixUnaryExpression;
        return prefix.pos !== -1 && prefix.operand.kind === ts.SyntaxKind.BigIntLiteral;
      }
      return literalTypeNode.literal.kind === ts.SyntaxKind.BigIntLiteral;
    }
  }
  return v.kind === ts.SyntaxKind.BigIntKeyword || v.kind === ts.SyntaxKind.BigIntLiteral;
}

/**
 * Check if the node is of boolean type.
 */
function isBooleanType(v: ts.Node): boolean {
  if (ts.isLiteralTypeNode(v)) {
    const literalTypeNode = v as ts.LiteralTypeNode;
    if (literalTypeNode.literal && literalTypeNode.literal.kind !== undefined) {
      return (
        literalTypeNode.literal.kind === ts.SyntaxKind.FalseKeyword ||
        literalTypeNode.literal.kind === ts.SyntaxKind.TrueKeyword
      );
    }
  }
  return (
    v.kind === ts.SyntaxKind.FalseKeyword ||
    v.kind === ts.SyntaxKind.TrueKeyword ||
    v.kind === ts.SyntaxKind.BooleanKeyword
  );
}

/**
 * Get the updated type.
 */
function getUpdatedType(
  v: ts.Node,
  nodeFlag: ts.NodeFlags,
  context: ts.TransformationContext
): ts.TypeNode | undefined {
  if (ts.isIdentifier(v)) {
    return undefined;
  }
  const isBigint = isBigintType(v);
  const isBoolean = isBooleanType(v);

  const allowedFlags = [ts.NodeFlags.Const, ts.NodeFlags.Let];
  if (isBigint && allowedFlags.includes(nodeFlag)) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
  } else if (isBoolean) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
  }
  return undefined;
}

/**
 * Process variable declarations
 */
function processVariableDeclaration(
  child: ts.VariableDeclaration,
  nodeFlag: ts.NodeFlags,
  context: ts.TransformationContext
): ts.VariableDeclaration {
  let updatedType: ts.TypeNode | undefined;
  child.forEachChild((v) => {
    updatedType = getUpdatedType(v, nodeFlag, context);
    return !!updatedType;
  });
  if (updatedType) {
    return context.factory.createVariableDeclaration(child.name, undefined, updatedType, undefined);
  }

  return child;
}

/**
 * Change the declarations of logical operators to the boolean type,
 * and change the declarations of bigint constants to the bigint type.
 */
function transformLogicalOperators(
  node: ts.Node,
  context: ts.TransformationContext,
  nodeFlag: ts.NodeFlags
): ts.VisitResult<ts.Node> {
  const variableDeclarations: ts.VariableDeclaration[] = [];
  node.forEachChild((child) => {
    if (ts.isVariableDeclaration(child)) {
      const processedDeclaration = processVariableDeclaration(child, nodeFlag, context);
      variableDeclarations.push(processedDeclaration);
    }
  });
  return context.factory.createVariableDeclarationList(variableDeclarations, nodeFlag);
}

/**
 * Change the literal union type to a type union.
 */
function createUnionTypeNode(
  unionTypeNode: ts.UnionTypeNode,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  let typeNode: ts.TypeNode[] = [];
  unionTypeNode.forEachChild((child) => {
    if (ts.isLiteralTypeNode(child)) {
      if (child.literal.kind === ts.SyntaxKind.FalseKeyword || child.literal.kind === ts.SyntaxKind.TrueKeyword) {
        typeNode.push(context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword));
      } else if (child.literal.kind === ts.SyntaxKind.NumericLiteral) {
        typeNode.push(context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword));
      } else {
        typeNode.push(child);
      }
    } else {
      typeNode.push(child as ts.TypeNode);
    }
  });
  return context.factory.createUnionTypeNode(typeNode);
}

function processSourceFileStatements(
  statements: readonly ts.Statement[],
  context: ts.TransformationContext
): {
  importDeclarations: ts.Statement[];
  statements: ts.Statement[];
  typeAliasMap: Map<string, string>;
} {
  const importDeclarations: ts.Statement[] = [];
  const statementsResult: ts.Statement[] = [];
  const typeAliasMap = new Map<string, string>();

  for (const stmt of statements) {
    if (ts.isImportDeclaration(stmt)) {
      processImportDeclaration(stmt, importDeclarations, typeAliasMap);
    } else {
      const newStmt = replaceTypeReferenceWithObject(stmt, context, typeAliasMap);
      statementsResult.push(newStmt as ts.Statement);
    }
  }

  return { importDeclarations, statements: statementsResult, typeAliasMap };
}

function processImportDeclaration(
  stmt: ts.ImportDeclaration,
  importDeclarations: ts.Statement[],
  typeAliasMap: Map<string, string>
): void {
  const moduleSpecifier = stmt.moduleSpecifier;
  if (isLimitImport(moduleSpecifier, KitPrefix)) {
    const importClause = stmt.importClause;
    if (importClause && importClause.namedBindings && ts.isNamedImports(importClause.namedBindings)) {
      processNamedImports(importClause.namedBindings, typeAliasMap);
    } else if (importClause?.name) {
      const originalName = importClause?.name?.text;
      const alias = originalName;
      typeAliasMap.set(alias, originalName);
    }
    return;
  }
  importDeclarations.push(stmt);
}

function processNamedImports(namedImports: ts.NamedImports, typeAliasMap: Map<string, string>): void {
  namedImports.elements.forEach((element) => {
    const originalName = element.propertyName ? element.propertyName.text : element.name.text;
    const alias = element.propertyName ? element.name.text : originalName;
    typeAliasMap.set(alias, originalName);
  });
}

function extractTypeNodes(
  node: ts.TypeParameterDeclaration,
  context: ts.TransformationContext
): {
  unionTypeNode: ts.TypeNode | undefined;
  typeNode: ts.TypeNode | undefined;
} {
  let unionTypeNode: ts.TypeNode | undefined;
  let typeNode: ts.TypeNode | undefined;

  node.forEachChild((parm) => {
    if (!ts.isIdentifier(parm)) {
      if (ts.isUnionTypeNode(parm)) {
        unionTypeNode = createUnionTypeNode(parm, context) as ts.TypeNode;
      } else if (ts.isLiteralTypeNode(parm)) {
        typeNode = getLiteralType(parm, context);
      } else {
        typeNode = undefined;
      }
    }
  });

  return { unionTypeNode, typeNode };
}

function getLiteralType(parm: ts.LiteralTypeNode, context: ts.TransformationContext): ts.TypeNode | undefined {
  if (parm.literal.kind === ts.SyntaxKind.NumericLiteral) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
  } else if (parm.literal.kind === ts.SyntaxKind.TrueKeyword || parm.literal.kind === ts.SyntaxKind.FalseKeyword) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
  }
  return undefined;
}

function createTypeParameterDeclaration(
  node: ts.TypeParameterDeclaration,
  constraint: ts.TypeNode | undefined,
  defaultType: ts.TypeNode | undefined,
  context: ts.TransformationContext
): ts.TypeParameterDeclaration {
  return context.factory.createTypeParameterDeclaration(undefined, node.name, constraint, defaultType);
}

/***
 * Handle initializes are not allowed in the environment content
 */
function inferNodeTypeFromInitializer(
  node: ts.PropertyDeclaration,
  context: ts.TransformationContext
): ts.TypeNode | undefined {
  if (node.initializer !== undefined) {
    switch (node.initializer.kind) {
      case ts.SyntaxKind.FalseKeyword:
      case ts.SyntaxKind.TrueKeyword:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
      case ts.SyntaxKind.NumericLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      case ts.SyntaxKind.BigIntLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
      case ts.SyntaxKind.StringLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
      default:
        return undefined;
    }
  } else if (node.type !== undefined && ts.isLiteralTypeNode(node.type)) {
    if (isBooleanType(node.type)) {
      return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
    } else if (isBigintType(node.type)) {
      return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
    }
  }

  return undefined;
}

function findMixedEnums(sourceFile: ts.SourceFile): { mixedEnumNames: string[]; firstPassStatements: ts.Statement[] } {
  let mixedEnumNames: string[] = [];
  const firstPassStatements: ts.Statement[] = [];

  for (const stmt of sourceFile.statements) {
    if (!ts.isEnumDeclaration(stmt)) {
      firstPassStatements.push(stmt);
      continue;
    }

    if (!hasMixedTypes(stmt)) {
      firstPassStatements.push(stmt);
      continue;
    }

    const enumName = stmt.name.text;
    mixedEnumNames.push(enumName);

    const jsValueType = ts.factory.createTypeReferenceNode(JSValue, undefined);
    const variableDeclaration = ts.factory.createVariableDeclaration(enumName, undefined, jsValueType, undefined);
    const variableDeclarationList = ts.factory.createVariableDeclarationList(
      [variableDeclaration],
      ts.NodeFlags.Const
    );
    const variableStatement = ts.factory.createVariableStatement(
      [ts.factory.createToken(ts.SyntaxKind.ExportKeyword), ts.factory.createToken(ts.SyntaxKind.DeclareKeyword)],
      variableDeclarationList
    );

    firstPassStatements.push(variableStatement);
  }

  return { mixedEnumNames, firstPassStatements };
}

/**
 * For enum member initializers that are prefix unary expressions such as `-1`,
 * return the kind of the operand; otherwise, return the kind of the initializer itself.
 */
function decayEnumMemberInitializerKind(initializer: ts.Expression): ts.SyntaxKind {
  if (ts.isPrefixUnaryExpression(initializer)) {
    return initializer.operand.kind;
  }
  return initializer.kind;
}

function hasMixedTypes(enumDeclaration: ts.EnumDeclaration): boolean {
  let firstKind: ts.SyntaxKind | null = null;
  for (const member of enumDeclaration.members) {
    if (member.initializer) {
      const currentKind = decayEnumMemberInitializerKind(member.initializer);
      if (firstKind === null) {
        firstKind = currentKind;
      } else if (currentKind !== firstKind) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Extract enum members together.
 */
function extractEnumMembers(item: ts.EnumDeclaration, enumbers: ts.EnumMember[]): void {
  item.members.forEach((member) => {
    if (ts.isEnumMember(member) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (!enumbers.some((m) => ts.isEnumMember(m) && ts.isEnumMember(member) && m.name.getText() === memberName)) {
        enumbers.push(member);
      }
    }
  });
}

/**
 * Merge classes or interfaces or enum with the same name
 */
function createCombinedInterfaceAndClass(
  list: ts.Statement[],
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind
): ts.Statement {
  const methods: ts.TypeElement[] = [];
  const classMethods: ts.ClassElement[] = [];
  const enumMembers: ts.EnumMember[] = [];

  list.forEach((item) => {
    if (ts.isInterfaceDeclaration(item)) {
      extractInterfaceMembers(item, methods);
    } else if (ts.isClassDeclaration(item)) {
      extractClassMembers(item, classMethods);
    } else if (ts.isEnumDeclaration(item)) {
      extractEnumMembers(item, enumMembers);
    }
  });

  if (kindType === ts.SyntaxKind.ClassDeclaration) {
    const classNode = list[0] as ts.ClassDeclaration;
    return context.factory.createClassDeclaration(
      classNode.modifiers,
      classNode.name,
      undefined,
      undefined,
      classMethods
    );
  } else if (kindType === ts.SyntaxKind.EnumDeclaration) {
    const enumNode = list[0] as ts.EnumDeclaration;
    return context.factory.createEnumDeclaration(enumNode.modifiers, enumNode.name, enumMembers);
  } else {
    const node = list[0] as ts.InterfaceDeclaration;
    return context.factory.createInterfaceDeclaration(node.modifiers, node.name, undefined, undefined, methods);
  }
}

/**
 * Find a list of interfaces or classes with the same name
 */
function findSameInterfaceOrClassOrEnumList(
  statements: ts.Statement[],
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind
): ts.Statement[] {
  const sameInterfaceMap = new Map<string, { count: number; nodes: ts.Statement[] }>();
  statements.forEach((statement) => {
    let name: string | undefined;
    const isInterfaceDeclaration = ts.isInterfaceDeclaration(statement) && statement.name;
    const isClassDeclaration = ts.isClassDeclaration(statement) && statement.name;
    const isEnumDeclaration = ts.isEnumDeclaration(statement) && statement.name;
    if (isInterfaceDeclaration || isClassDeclaration || isEnumDeclaration) {
      name = statement.name.text;
    }
    if (name) {
      if (!sameInterfaceMap.has(name)) {
        sameInterfaceMap.set(name, { count: 0, nodes: [] });
      }
      const entry = sameInterfaceMap.get(name)!;
      entry.count++;
      entry.nodes.push(statement);
    }
  });
  const uniqueNodes: ts.Statement[] = [];
  const newNodes: ts.Statement[] = [];
  sameInterfaceMap.forEach((entry) => {
    if (entry.count === 1) {
      uniqueNodes.push(...entry.nodes);
    } else {
      // Reorganize interface information
      const newInterface = createCombinedInterfaceAndClass(entry.nodes, context, kindType);
      newNodes.push(newInterface);
    }
  });

  return [...uniqueNodes, ...newNodes];
}

function restrictIdentifierName(
  node: ts.PropertyDeclaration | ts.MethodDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration | ts.ImportSpecifier | ts.ExportSpecifier
): ts.VisitResult<ts.Node> {
  const restrictedNames: ReadonlySet<string> = new Set(ETSKeyword);

  if (
    ts.isPropertyDeclaration(node) ||
    ts.isMethodDeclaration(node) ||
    ts.isClassDeclaration(node) ||
    ts.isInterfaceDeclaration(node) ||
    ts.isImportSpecifier(node) ||
    ts.isExportSpecifier(node)
  ) {
    return restrictDeclarationName(node, restrictedNames);
  }

  return node;
}

function restrictDeclarationName(
  node: ts.PropertyDeclaration | ts.MethodDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration | ts.ImportSpecifier | ts.ExportSpecifier,
  restrictedNames: ReadonlySet<string>
): ts.VisitResult<ts.Node> {
  const name = node.name;

  if (name && ts.isIdentifier(name) && restrictedNames.has(name.text)) {
    return undefined;
  }

  return node;
}

function exportDefaultAssignment(
  node: ts.FunctionDeclaration | ts.ClassDeclaration,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  const modifiers = node.modifiers;

  if (modifiers === undefined) {
    return node;
  }

  if (modifiers.some(modifier => modifier.kind === ts.SyntaxKind.DefaultKeyword)) {
    const newModifiers = [...modifiers];
    
    if (!modifiers.some(modifier => modifier.kind === ts.SyntaxKind.DeclareKeyword)) {
      const declareModifier = context.factory.createModifier(ts.SyntaxKind.DeclareKeyword);
      
      const defaultIndex = modifiers.findIndex(mod => mod.kind === ts.SyntaxKind.DefaultKeyword);
      if (defaultIndex !== -1) {
        newModifiers.splice(defaultIndex + 1, 0, declareModifier);
      } else {
        newModifiers.push(declareModifier);
      }
    }

    const safeModifiers = newModifiers as ts.Modifier[];

    return updateNodeWithModifiers(node, safeModifiers, context);
  }

  return node;
}

function updateNodeWithModifiers(
  node: ts.FunctionDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  if (ts.isFunctionDeclaration(node)) {
    return updateFunctionDeclarationWithModifiers(node, modifiers, context);
  } else if (ts.isClassDeclaration(node)) {
    return updateClassDeclarationWithModifiers(node, modifiers, context);
  }

  return node;
}

function updateFunctionDeclarationWithModifiers(
  node: ts.FunctionDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.FunctionDeclaration {
  return context.factory.updateFunctionDeclaration(
    node,
    modifiers,
    node.asteriskToken,
    node.name,
    node.typeParameters,
    node.parameters,
    node.type,
    node.body
  );
}

function updateClassDeclarationWithModifiers(
  node: ts.ClassDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.ClassDeclaration {
  return context.factory.updateClassDeclaration(
    node,
    modifiers,
    node.name,
    node.typeParameters,
    node.heritageClauses,
    node.members
  );
}

function updateInterfaceDeclarationWithModifiers(
  node: ts.InterfaceDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.InterfaceDeclaration {
  return context.factory.updateInterfaceDeclaration(
    node,
    modifiers,
    node.name,
    node.typeParameters,
    node.heritageClauses,
    node.members
  );
}

function tranDeclarationList<T extends ts.ClassDeclaration | ts.InterfaceDeclaration>(
  declarations: T[],
  context: ts.TransformationContext,
  promises: string[]
): T[] {
  return declarations.map((decl) => {
    const heritageClause = decl.heritageClauses?.find((clause) => clause.token === ts.SyntaxKind.ExtendsKeyword);
    const expressions = expressionList(heritageClause?.types);

    if (shouldCreateTypeAlias(decl, expressions, promises)) {
      return createTypeAliasForClass(decl, context) as T;
    } else {
      return decl;
    }
  });
}

function shouldCreateTypeAlias<T extends ts.ClassDeclaration | ts.InterfaceDeclaration>(
  decl: T,
  expressions: string[],
  promises: string[]
): boolean {
  if (ts.isInterfaceDeclaration(decl) && decl.typeParameters && decl.typeParameters.length > 0) {
    return true;
  }

  return expressions.some(className => FINAL_CLASS.includes(className)) &&
         !promises.some(promise => expressions.includes(promise));
}

function tranClassDeclarationList(
  classDeclarations: ts.ClassDeclaration[],
  context: ts.TransformationContext,
  promises: string[]
): ts.ClassDeclaration[] {
  return tranDeclarationList(classDeclarations, context, promises);
}

function createTypeAliasForClass(
  decl: ts.ClassDeclaration | ts.InterfaceDeclaration,
  context: ts.TransformationContext
): ts.TypeAliasDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration {
  if (decl.modifiers !== undefined && decl.name !== undefined) {
    return context.factory.createTypeAliasDeclaration(
      filterModifiers(decl.modifiers as ts.NodeArray<ts.Modifier>),
      decl.name,
      decl.typeParameters,
      context.factory.createTypeReferenceNode(JSValue)
    );
  } else {
    return decl;
  }
}

function filterModifiers(modifiers: ts.NodeArray<ts.Modifier>): ts.NodeArray<ts.Modifier> | undefined {
  const filteredModifiers = modifiers.filter(mod => mod.kind !== undefined && mod.kind !== ts.SyntaxKind.DeclareKeyword);
  return filteredModifiers.length > 0 ?
    ts.factory.createNodeArray(filteredModifiers, modifiers.hasTrailingComma) :
    undefined;
}

function tranInterfaceDeclarationList(
  interfaceDeclarations: ts.InterfaceDeclaration[],
  context: ts.TransformationContext,
  promises: string[]
): ts.InterfaceDeclaration[] {
  return tranDeclarationList(interfaceDeclarations, context, promises);
}

function expressionList(typeArguments?: readonly ts.ExpressionWithTypeArguments[]): string[] {
  let expressions: string[] = [];
  if (typeArguments !== undefined && typeArguments.length > 0) {
    for (const arg of typeArguments) {
      if (arg.expression !== undefined && ts.isIdentifier(arg.expression)) {
        expressions.push(arg.expression.text);
      }
    }
  }
  return expressions;
}

function isFinalClassImport(element: ts.ImportSpecifier): boolean {
  return FINAL_CLASS.includes(element.name.text);
}

function collectFinalClassImports(namedImports: ts.NamedImports, promises: string[]): void {
  namedImports.elements.forEach((element) => {
    if (ts.isImportSpecifier(element) && isFinalClassImport(element)) {
      promises.push(element.name.text);
    }
  });
}

function getImportSpecifierNames(importSpecifierNodes: ts.ImportDeclaration[]): string[] {
  const promises: string[] = [];

  importSpecifierNodes.forEach((item) => {
    if (ts.isImportDeclaration(item)) {
      const namedImports = item.importClause?.namedBindings as ts.NamedImports | undefined;
      if (namedImports !== undefined && ts.isNamedImports(namedImports)) {
        collectFinalClassImports(namedImports, promises);
      }
    }
  });

  return promises;
}

function updatePropertyAccessExpression(node: ts.PropertyAccessExpression, context: ts.TransformationContext): ts.TypeNode | undefined {
  let identifiers: ts.Identifier[] = [];
  if (ts.isPropertyAccessExpression(node.expression)) {
    identifiers = [
      node.expression.expression,
      node.expression.name
    ] as ts.Identifier[];
  } else {
    identifiers = [node.expression] as ts.Identifier[];
  }
  if (identifiers.length > 1) {
    return context.factory.createTypeReferenceNode(
      context.factory.createQualifiedName(
        identifiers[0],
        identifiers[1]
      )
    );
  } else if (identifiers.length === 1) {
    return context.factory.createTypeReferenceNode(
      identifiers[0]
    );
  }

  return undefined;
}

// Check whether the header comment contains the @noninterop field.
function isNonInterop(node: ts.Node): boolean {
  const fullText = ts.getOriginalNode(node).getFullText();
  const codeText = ts.getOriginalNode(node).getText();
  const commentText = fullText.replace(codeText, '');

  return /\/\*\*.*?@noninterop\b.*?\*\//s.test(commentText);
}

function replaceEsObjectTypeName(node: ts.TypeReferenceNode, factory: ts.NodeFactory): ts.TypeReferenceNode {
  return factory.updateTypeReferenceNode(
    node,
    factory.createIdentifier(JSValue),
    node.typeArguments
  );
}
  
/**
 * helper function to filter out optional(with question token) methods in class declaration.
 */ 
function isNotOptionalMemberFunction(member: ts.ClassElement): boolean {
  if (ts.isMethodDeclaration(member) && member.questionToken) {
    return false;
  }
  return true;
}

function isUtilityTypes(node: ts.TypeReferenceNode): boolean {
  return ts.isIdentifier(node.typeName) &&
    UtilityTypes.has(node.typeName.text);
}

function isSpecificTypes(node: ts.Node): boolean {
  return SpecificTypes.includes(node.kind);
}

/**
 * Creates an AST node that represents a type compatible with ArkTS 1.2,
 * based on the given utility type in ArkTS 1.1
 */
function createNodeFromUtilType(targetType: string, factory: ts.NodeFactory):
  ts.TypeReferenceNode | ts.ArrayTypeNode {
  if (targetType === ArrayType) {
    return factory.createArrayTypeNode(factory.createTypeReferenceNode(JSValue));
  }
  return factory.createTypeReferenceNode(JSValue);
}

function mergeOverloadedMembers(
  members: readonly ts.TypeElement[] | readonly ts.ClassElement[],
  context: ts.TransformationContext
): ts.NodeArray<ts.TypeElement> | ts.NodeArray<ts.ClassElement> {
  const memberMap = new Map<string, { signatures: ts.SignatureDeclarationBase[]; isMethod: boolean }>();

  // Step 1: Group members by name
  for (const member of members) {
    if (
      (ts.isMethodSignature(member) || ts.isMethodDeclaration(member)) &&
      ts.isIdentifier(member.name)
    ) {
      const name = member.name.text;
      if (!memberMap.has(name)) {
        memberMap.set(name, { signatures: [], isMethod: true });
      }
      memberMap.get(name)!.signatures.push(member as ts.SignatureDeclarationBase);
    }
  }

  // Step 2: Merge overloaded methods
  const mergedMembers: (ts.TypeElement | ts.ClassElement)[] = [];
  memberMap.forEach(({ signatures }, name) => {
    if (signatures.length > 1) {
      const mergedSignature = mergeSignatures(signatures, context);
      mergedMembers.push(mergedSignature);
    } else {
      signatures.forEach(signature => {
        if (ts.isTypeElement(signature)) {
          mergedMembers.push(signature);
        } else if (ts.isClassElement(signature)) {
          mergedMembers.push(signature);
        }
      });
    }
  });

  // Step 3: Return the correct type based on input
  if (members.some(member => ts.isClassElement(member))) {
    return ts.factory.createNodeArray(mergedMembers as ts.ClassElement[]);
  } else {
    return ts.factory.createNodeArray(mergedMembers as ts.TypeElement[]);
  }
}

function mergeSignatures(
  signatures: ts.SignatureDeclarationBase[] | ts.ConstructorDeclaration[],
  context: ts.TransformationContext
): ts.MethodSignature | ts.MethodDeclaration | ts.ConstructorDeclaration {
  const firstSignature = signatures[0];
  const parameters = mergeParameters(signatures, context);
  const returnType = mergeReturnTypes(signatures, context);
  
  let typeParameters: ts.NodeArray<ts.TypeParameterDeclaration> | undefined;
  if (firstSignature.typeParameters) {
    typeParameters = firstSignature.typeParameters;
  }

  if (ts.isMethodSignature(firstSignature)) {
    return context.factory.createMethodSignature(
      firstSignature.modifiers,
      firstSignature.name,
      firstSignature.questionToken,
      typeParameters,
      parameters,
      returnType
    );
  } else if (ts.isMethodDeclaration(firstSignature)) {
    return context.factory.createMethodDeclaration(
      firstSignature.modifiers,
      firstSignature.asteriskToken,
      firstSignature.name,
      firstSignature.questionToken,
      typeParameters,
      parameters,
      returnType,
      undefined
    );
  }
  else if (ts.isConstructorDeclaration(firstSignature)) {
    return context.factory.createMethodDeclaration(
      firstSignature.modifiers,
      undefined,
      'constructor',
      undefined,
      undefined,
      parameters,
      undefined,
      undefined
    )
  }

  throw new Error('Unsupported signature type');
}

function mergeParameters(
  signatures: ts.SignatureDeclarationBase[] | ts.ConstructorDeclaration[],
  context: ts.TransformationContext
): ts.ParameterDeclaration[] {
  const mergedParameters: ts.ParameterDeclaration[] = [];
  const maxParams = Math.max(...signatures.map(sig => sig.parameters.length));

  for (let i = 0; i < maxParams; i++) {
    const paramTypes: ts.TypeNode[] = [];
    let isOptional = false;

    for (const sig of signatures) {
      const param = sig.parameters[i];
      if (param) {
        if (param.questionToken || param.initializer) {
          isOptional = true;
        }
        if (param.type) {
          paramTypes.push(param.type);
        }
      } else {
        // If a signature does not have the parameter, mark it as optional.
        isOptional = true;
      }
    }

    const uniqueTypes = removeDuplicateTypes(paramTypes);
    const newParamType = uniqueTypes.length > 1
      ? context.factory.createUnionTypeNode(uniqueTypes)
      : uniqueTypes[0];

    const newParam = context.factory.createParameterDeclaration(
      undefined,
      undefined,
      `param${i}`,
      isOptional ? context.factory.createToken(ts.SyntaxKind.QuestionToken) : undefined,
      newParamType,
      undefined
    );

    mergedParameters.push(newParam);
  }

  return mergedParameters;
}

function mergeReturnTypes(
  signatures: ts.SignatureDeclarationBase[],
  context: ts.TransformationContext
): ts.TypeNode {
  let returnTypes = signatures.map(sig => sig.type).filter(Boolean) as ts.TypeNode[];
  returnTypes = removeDuplicateTypes(returnTypes);
  return returnTypes.length > 1
    ? context.factory.createUnionTypeNode(returnTypes)
    : returnTypes[0];
}

function getTypeNameKey(type: ts.TypeNode): string {
  if (ts.isTypeReferenceNode(type)) {
    const getName = (name: ts.EntityName): string =>
      ts.isIdentifier(name)
        ? name.escapedText.toString()
        : `${getName(name.left)}.${name.right.escapedText.toString()}`;
    return getName(type.typeName);
  }
  if (ts.isArrayTypeNode(type)) {
    return getTypeNameKey(type.elementType) + '[]';
  }
  return ts.SyntaxKind[type.kind];
}

function removeDuplicateTypes(types: ts.TypeNode[]): ts.TypeNode[] {
  const seenTypes = new Set<string>();
  const uniqueTypes: ts.TypeNode[] = [];

  for (const type of types) {
    const typeText = getTypeNameKey(type);
    if (!seenTypes.has(typeText)) {
      seenTypes.add(typeText);
      uniqueTypes.push(type);
    }
  }

  return uniqueTypes;
}
