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

const args = process.argv.slice(2);
const tsModule = args[2].trim();
const ts = require(tsModule);
const fs = require('fs');
const path = require('path');

//读取文件内容
const filePath = args[0];
const outPath = args[1];
const prefix = args[3];

const getFilename = (absolutePath) => path.basename(absolutePath);

const fixme = '0/* FIXME: Initialization is required */';
const copyrightC = `/*
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
 */`;

let filename = '';
// Avoid duplicate filename in different directories, append prefix to filename
if (prefix !== undefined) {
    filename = prefix;
}
if (filePath.endsWith('.d.ts')) {
    filename += getFilename(filePath).split('.d.ts')[0];
} else if (filePath.endsWith('.d.ets')) {
    filename += getFilename(filePath).split('.d.ets')[0];
} else if (filePath.endsWith('.ets')) {
    filename += getFilename(filePath).split('.ets')[0];
} else if (filePath.endsWith('.ts')) {
    filename += getFilename(filePath).split('.ts')[0];
} else if (filePath.endsWith('.js')) {
    filename += getFilename(filePath).split('.js')[0];
}

const fileContent = fs.readFileSync(filePath, 'utf-8');

const program = ts.createProgram([filePath], { target: ts.ScriptTarget.ES2015 });

function newjson() {
    const newjson = { 'type': [], 'info': {}, 'method': [], 'class': [], 'interface': [], 'struct': [], 'enum': [], 'variable': [], 'module': [], 'import': [] };
    return newjson;
}

function newMethodJson() {
    return { 'name': '', 'param': [], 'return': '', 'modifier': [], 'isConstructor': false, 'parameters': [] };
}

function newObjectJson() {
    return { 'name': '', 'modifier': [], 'father': [] };
}

function newEnumJson() {
    return { 'name': '', 'item-value': [], 'modifier': [] };
}

function newTypeJosn() {
    return { 'name': '', 'type': '' };
}

function newVariableJson() {
    return { 'name': '', 'value': '', 'type': '', 'modifier': [], 'isOptional': false };
}

function newModuleJson() {
    return { 'name': '' };
}

function newImportJson() {
    return { 'name': '' };
}

function processNameForFiledOrMethod(node, sourceFile) {
    if (node.name.kind === ts.SyntaxKind.StringLiteral) {
        return node.name.text;
    } else if (node.name.kind === ts.SyntaxKind.Identifier) {
        return node.name.escapedText;
    } else {
        return node.name.getText(sourceFile);
    }
}

class ASTVisitor {
    constructor(sourceFile, checker, elejson) {
        this.sourceFile = sourceFile;
        this.checker = checker;
        this.ejson = elejson;
        this.ejson.symbols = {};
        this.ejson.exports = {};
    }

    visit(node, elejson) {
        ts.forEachChild(node, child => {
            if (child.modifiers?.some(
                modifier => modifier.kind === ts.SyntaxKind.PrivateKeyword) ?? false) {
                return;
            }

            // 处理类
            if (ts.isClassDeclaration(child)) {
                this.visitClass(child, elejson.class);
            } else if (ts.isFunctionDeclaration(child) || ts.isMethodDeclaration(child) ||
                ts.isMethodSignature(child) || ts.isConstructorDeclaration(child)) {
                this.visitFunc(child, elejson.method);
            } else if (ts.isInterfaceDeclaration(child)) {
                this.visitInterface(child, elejson.interface);
            } else if (ts.isConstructorDeclaration(node)) {
                this.visitConstructor(child, elejson.method);
            } else if (ts.isVariableDeclaration(child)) {
                this.visitVariable(child, elejson.variable);
            } if (ts.isPropertySignature(child) || ts.isPropertyDeclaration(child)) {
                this.visitProperty(child, elejson.variable);
            } else if (ts.isEnumDeclaration(child)) {
                this.visitEnum(child, elejson.enum);
            } else if (child.kind === ts.SyntaxKind.ModuleDeclaration) {
                this.visitModule(child, elejson.module);
            } else if (ts.isVariableStatement(child)) {
                this.visitVariable1(child, elejson.variable);
            } else if (ts.isTypeAliasDeclaration(child)) {
                this.visitTypeAlias(child, elejson.type);
            } else if (ts.isCallSignatureDeclaration(child) || ts.isIndexSignatureDeclaration(child)) {
                this.visitFunc1(child, elejson.method);
            } else if (ts.isImportDeclaration(child)) {
                this.visitImport(child, elejson.import);
            } else if (ts.isExportDeclaration(child)) {
                this.visitExport(child);
            }
        });
    }

    visitClass(node, rjson) {
        const content = { ...newjson(), info: newObjectJson() };
        rjson.push(content);
        const info = content.info;
        info.name = node.name.getText(this.sourceFile);

        node.modifiers?.forEach(modifier => {
            info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        if (node.heritageClauses) {
            node.heritageClauses.forEach(clause => {
                clause.types.forEach(heritage => {
                    info.father.push(`${heritage.expression.escapedText}`);
                });
            });

        }

        this.populateJsDoc(node, info);
        this.handleTypeParameters(node, info);

        let comment = '';

        
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );

        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        info.comment = comment;

        this.visit(node, content);

    }

    visitEnum(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newEnumJson();
        rjson[rjson.length - 1].info.name = node.name.getText(this.sourceFile);

        node.members.forEach(member => {
            let enumValue;
            let hasInitValue = false;
            if (member.initializer !== undefined) {
                if (member.initializer.text !== undefined) {
                    enumValue = member.initializer.text;
                    hasInitValue = true;
                } else if (member.initializer.operand !== undefined && member.initializer.operand.text !== undefined) {
                    enumValue = -member.initializer.operand.text;
                    hasInitValue = true;
                }
            }
            if (hasInitValue === false) {
                let parentNode = node;
                while (parentNode.parent !== undefined) {
                    parentNode = parentNode.parent;
                }
                let originText = parentNode.text;
                let sub1 = originText.substring(originText.lastIndexOf(member.name.text));
                let lastIdx = sub1.indexOf('\n');
                if (sub1[lastIdx - 1] === '\r') {
                    lastIdx--;
                }
                if (sub1[lastIdx - 1] === ',') {
                    lastIdx--;
                }
                // 3 is \n=\n
                enumValue = sub1.substring(member.name.text.length + 3, lastIdx);
            }
            rjson[rjson.length - 1].info['item-value'].push(`${member.name.text}${member.initializer ? ` = ${enumValue}` : ''}`);
        });

        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;

    }

    visitFunc(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newMethodJson();
        const isConstructor = ts.isConstructorDeclaration(node);
        rjson[rjson.length - 1].info.isConstructor = isConstructor;
        rjson[rjson.length - 1].info.name = isConstructor ? 'constructor' : processNameForFiledOrMethod(node, this.sourceFile);

        rjson[rjson.length - 1].info.parameters = [];
        node.parameters.forEach(param => {
            if (!param.questionToken) {
                rjson[rjson.length - 1].info.param.push(`${param.getText(this.sourceFile)}`);
                const typeNode = this.serializeType(param.type);
                rjson[rjson.length - 1].info.parameters.push({
                    name: ts.getNameOfDeclaration(param).getText(),
                    type: typeNode,
                    optional: !!param.questionToken,
                });
            }
        });
        node.parameters.forEach(param => {
            if (param.questionToken) {
                rjson[rjson.length - 1].info.param.push(`${param.getText(this.sourceFile)}`);
                const typeNode = this.serializeType(param.type);
                rjson[rjson.length - 1].info.parameters.push({
                    name: ts.getNameOfDeclaration(param).getText(),
                    type: typeNode,
                    optional: !!param.questionToken,
                });
            }
        });

        let returnType = 'void';
        if (node.type && node.type.kind === ts.SyntaxKind.ThisType) {
            const sym = this.checker.getSymbolAtLocation(node.type);
            if (sym) {
                returnType = sym.getName();
            }
        } else {
            returnType = node.type ? node.type.getText(this.sourceFile).replace(/Readonly<(.*)>/, '$1').replace(/\bstring\b/g, 'String') : 'void';
        }

        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        rjson[rjson.length - 1].info.return = `${returnType}`;
        
        // 将返回值转换为S表达式，以便序列化返回值为联合体时的返回值
        const returnTypeNode = node.type ? this.serializeType(node.type) : '(void)';
        rjson[rjson.length - 1].info.returnTypeNode = returnTypeNode;

        this.populateJsDoc(node, rjson[rjson.length - 1].info);
        this.handleTypeParameters(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitFunc1(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newMethodJson();

        node.parameters.forEach(param => {
            rjson[rjson.length - 1].info.param.push(`${param.getText(this.sourceFile)}`);
        });
        const returnType = node.type ? node.type.getText(this.sourceFile) : 'void';

        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        rjson[rjson.length - 1].info.return = `${returnType}`;

        this.handleTypeParameters(node, rjson[rjson.length - 1]['info']);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitConstructor(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newMethodJson();
        rjson[rjson.length - 1].info.name = node.name.getText(this.sourceFile);


        const parameters = constructor.parameters.map(param => {
            rjson[rjson.length - 1].info.param.push(`${param.getText(this.sourceFile)}`);
        });
        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitInterface(node, rjson) {
        rjson.push(newjson());
        rjson[rjson.length - 1].info = newObjectJson();
        rjson[rjson.length - 1].info.name = node.name.getText(this.sourceFile);
        if (node.heritageClauses) {
            node.heritageClauses.forEach(clause => {
                clause.types.forEach(heritage => {
                    rjson[rjson.length - 1].info.father.push(`${heritage.expression.escapedText}`);
                });
            });

        }
        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);
        this.handleTypeParameters(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;

        this.visit(node, rjson[rjson.length - 1]);

    }

    visitProperty(node, rjson) {
        rjson.push(newjson());
        rjson[rjson.length - 1].info = newVariableJson();
        rjson[rjson.length - 1].info.name = processNameForFiledOrMethod(node, this.sourceFile);
        rjson[rjson.length - 1].info.value = !node.initializer || !node.initializer.getText() ?
            '' : node.initializer.getText();

        rjson[rjson.length - 1].info.typeNode = node.type ?
            this.serializeType(node.type) : this.guessType(rjson[rjson.length - 1].info.value);
        rjson[rjson.length - 1].info.type = node.type ?
            node.type.getText(this.sourceFile).replace(/\bstring\b/g, 'String') :
            rjson[rjson.length - 1].info.typeNode.slice(1, -1);
        rjson[rjson.length - 1].info.isOptional = node.questionToken ? true : false;

        const inferredType = this.checker.getTypeFromTypeNode(node.type);
        rjson[rjson.length - 1].info.inferredType = this.serializeType(this.checker.typeToTypeNode(inferredType));

        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitVariable(node, rjson) {
        rjson.push(newjson());
        rjson[rjson.length - 1].info = newVariableJson();
        rjson[rjson.length - 1].info.name = node.name.getText(this.sourceFile);
        rjson[rjson.length - 1].info.value = node.value ? node.value.getText(this.sourceFile) : '';

        // todo: get inferred type
        // rjson[rjson.length - 1]["info"].typeNode = this.serializeType(param.type)

        node.modifiers?.forEach(modifier => {
            rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;

    }

    visitVariable1(node, rjson) {
        node.declarationList.declarations.forEach(declaration => {
            rjson.push(newjson());
            rjson[rjson.length - 1].info = newVariableJson();

            node.modifiers?.forEach(modifier => {
                rjson[rjson.length - 1].info.modifier.push(`${ts.SyntaxKind[modifier.kind]}`);
            });

            this.populateJsDoc(node, rjson[rjson.length - 1].info);

            let comment = '';
            const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
                ({ kind, pos, end }) => fileContent.substring(pos, end)
            );
            if (comments) {
                comment = comments.join('\n');
            }
            comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
            rjson[rjson.length - 1].info.comment = comment;

            rjson[rjson.length - 1].info.name = declaration.name.escapedText;
            if (!declaration.initializer || !declaration.initializer.getText()) {
                rjson[rjson.length - 1].info.comment = '/*\n' + comment;
                rjson[rjson.length - 1].info.value = fixme + '\n*/';
            } else {
                rjson[rjson.length - 1].info.comment = comment;
                rjson[rjson.length - 1].info.value = declaration.initializer.getText();
            }

            rjson[rjson.length - 1].info.typeNode = declaration.type ?
                this.serializeType(declaration.type) : this.guessType(rjson[rjson.length - 1].info.value);
            rjson[rjson.length - 1].info.type = declaration.type ?
                declaration.type.getText() : rjson[rjson.length - 1].info.typeNode.slice(1, -1);
        });
    }

    visitTypeAlias(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newTypeJosn();

        rjson[rjson.length - 1].info.name = node.name.escapedText;
        rjson[rjson.length - 1].info.typeNode = this.serializeType(node.type);

        if (node.type.kind === 150) {
            rjson[rjson.length - 1].info.type = 'number';
        } else if (node.type.kind === 136) {
            rjson[rjson.length - 1].info.type = 'boolean';
        } else if (node.type.kind === 154) {
            rjson[rjson.length - 1].info.type = 'string';
        } else {
            // ts.SyntaxKind[node.type.kind]
            rjson[rjson.length - 1].info.type = '';
        }

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitModule(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newModuleJson();
        rjson[rjson.length - 1].info.name = node.name.text;

        ts.forEachChild(node, child => {
            if (child.kind === ts.SyntaxKind.ModuleBlock) {
                this.visit(child, rjson[rjson.length - 1]);
            } else if (child.kind === ts.SyntaxKind.ModuleDeclaration) {
                this.visitModule(child, rjson[rjson.length - 1].module);
            }
        });

        this.populateJsDoc(node, rjson[rjson.length - 1].info);

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitImport(node, rjson) {

        rjson.push(newjson());
        rjson[rjson.length - 1].info = newImportJson();

        rjson[rjson.length - 1].info.name = node.getText(this.sourceFile);

        // console.log(importFile)

        let comment = '';
        const comments = ts.getLeadingCommentRanges(fileContent, node.getFullStart())?.map(
            ({ kind, pos, end }) => fileContent.substring(pos, end)
        );
        if (comments) {
            comment = comments.join('\n');
        }
        comment = comment.replace(new RegExp(`^${copyrightC.replace(/([.*+?^=!:${}()|\[\]\/\\,-])/g, "\\\$1")}`, 'gm'), '').trim();
        rjson[rjson.length - 1].info.comment = comment;
    }

    visitExport(node) {
        const exportClause = node.exportClause;
        if (exportClause === undefined) {
            return;
        }
        if (!ts.isNamedExports(exportClause)) {
            return;
        }
        exportClause.elements.filter(it => it.propertyName).forEach(it => {
            this.ejson.exports[it.propertyName.escapedText] = it.name.escapedText;
        });
    }

    processFieldExports(field) {
        this.ejson[field].forEach(it => {
            const originalName = it.info?.name;
            if (!originalName) {
                return;
            }
            const alias = this.ejson.exports[originalName];
            if (alias) {
                it.info.name = alias;
            }
        });
    }

    processExports() {
        ['method', 'class', 'interface', 'struct', 'enum', 'variable'].forEach(field => {
            this.processFieldExports(field);
        });
    }

    populateJsDoc(node, info) { }

    handleTypeParameters(node, info) {
        if (node.typeParameters instanceof Array) {
            info.typeParameters = node.typeParameters.map(it => ({
                name: it.name.getText()
            }));
        } else {
            info.typeParameters = [];
        }
    }

    generateGenericParameter(tp) {
        const constraint = tp.constraint ? ` ${this.serializeType(tp.constraint)}` : '';
        return `(${tp.name.escapedText}${constraint})`;
    }

    serializeLiteralType(typeNode) {
        const kind = typeNode.literal.kind;
        switch (kind) {
            case ts.SyntaxKind.StringLiteral:
                return `(StringLiteral "${typeNode.literal.text}")`;
            case ts.SyntaxKind.NumericLiteral:
                return `(NumericLiteral ${typeNode.literal.text})`;
            case ts.SyntaxKind.NullKeyword: // null is treated as literal type
                return '(null)';
            default:
                return `(unsupported ${ts.SyntaxKind[kind]})`;
        }
    }

    serializeType(typeNode) {
        if (!typeNode) {
            return '(any)';
        }
        switch (typeNode.kind) {
            case ts.SyntaxKind.NumberKeyword:
                return '(number)';
            case ts.SyntaxKind.StringKeyword:
                return '(string)';
            case ts.SyntaxKind.BooleanKeyword:
                return '(boolean)';
            case ts.SyntaxKind.BigIntKeyword:
                return '(bigint)';
            case ts.SyntaxKind.ObjectKeyword:
                return '(object)';
            case ts.SyntaxKind.SymbolKeyword:
                return '(symbol)';
            case ts.SyntaxKind.VoidKeyword:
                return '(void)';
            case ts.SyntaxKind.UndefinedKeyword:
                return '(undefined)';
            case ts.SyntaxKind.AnyKeyword:
                return '(any)';
            case ts.SyntaxKind.UnknownKeyword:
                return '(unknown)';
            case ts.SyntaxKind.NeverKeyword:
                return '(never)';
            case ts.SyntaxKind.TypeReference:
                this.trackSymbol(typeNode);
                // (ref name type-argument-list)
                return `(ref ${this.entityNameToString(typeNode.typeName)}${typeNode.typeArguments?.map(ta => ' ' + this.serializeType(ta)).join('') ?? ''})`;
            case ts.SyntaxKind.LiteralType:
                // (LiteralKind text)
                return this.serializeLiteralType(typeNode);
            case ts.SyntaxKind.FunctionType:
                // (function (type-parameter-list) ret-type (parameter p-name p-type))           
                let generics = typeNode.typeParameters?.map(tp =>
                    this.generateGenericParameter(tp)
                ).join(' ') ?? '';
                // If this is a typealias, typeparameters are saved in parent decl
                if (typeNode.parent !== undefined && typeNode.parent.kind === ts.SyntaxKind.TypeAliasDeclaration) {
                    generics = typeNode.parent.typeParameters?.map(tp =>
                        this.generateGenericParameter(tp)
                    ).join(' ') ?? '';
                }

                return `(function (${generics}) ${this.serializeType(typeNode.type)}${typeNode.parameters.map(
                    p => ` (parameter ${p.name.escapedText} ${this.serializeType(p.type)}${p.questionToken ? ' optional' : ''})`
                ).join('')})`;
            case ts.SyntaxKind.ArrayType:
                // (array inner-type)
                return `(array ${this.serializeType(typeNode.elementType)})`;
            case ts.SyntaxKind.OptionalType:
                return `(optional ${this.serializeType(typeNode.type)})`;
            case ts.SyntaxKind.UnionType:
                return `(union ${typeNode.types.map(t => this.serializeType(t)).join(' ')})`;
            case ts.SyntaxKind.IntersectionType:
                return `(intersection ${typeNode.types.map(t => this.serializeType(t)).join(' ')})`;
            case ts.SyntaxKind.ParenthesizedType:
                return this.serializeType(typeNode.type);
            case ts.SyntaxKind.TupleType:
                // (tuple type-list)
                return `(tuple ${typeNode.elements.map(t => this.serializeType(t)).join(' ')})`;
            case ts.SyntaxKind.TypeLiteral:
                // (literal member-list)
                // member is one of:
                //      | (prop name type-expr)
                //      | (index key-type value-type)
                //      | (call (type-parameters) ret-type param-types)
                return `(literal${typeNode.members.map(it => {
                    if (ts.isPropertySignature(it)) {
                        if (ts.isIdentifier(it.name)) {
                            const optional = it.questionToken ? ' optional' : '';
                            return ` (prop ${it.name.escapedText} ${this.serializeType(it.type)}${optional})`;
                        }
                        // Other cases are hard to support. For exmaple:
                        // { [123]?: string }
                        // { [Symbol.toStringTag]: number }
                        console.warn('Unsupported identifier name:', ts.SyntaxKind[it.name.kind]);
                        return '';
                    } else if (ts.isIndexSignatureDeclaration(it)) {
                        // index signature is guaranteed to have exactly one parameter
                        return ` (index ${this.serializeType(it.parameters[0].type)} ${this.serializeType(it.type)})`;
                    } else if (ts.isCallSignatureDeclaration(it)) {
                        // call signatures are like functions
                        const generics = it.typeParameters?.map(tp =>
                            this.generateGenericParameter(tp)
                        ).join(' ') ?? '';
                        return ` (call (${generics}) ${this.serializeType(it.type)}${it.parameters.map(
                            p => ` (parameter ${p.name.escapedText} ${this.serializeType(p.type)})`
                        ).join('')})`;
                    } else if (ts.isMethodSignature(it)) {
                        /* TODO: unsupported, and the test case is:
                            export type AnimationItem = {
                                play(name?: string): void;
                            };
                        */
                        console.warn('Unsupported member type');
                        return '';
                    }
                    else {
                        // unexpected: what's in this case?
                        throw `Unexpected member type: ${ts.SyntaxKind[it.kind]}`;
                    }
                }).join('')})`;

            // todo: unsupported types
            case ts.SyntaxKind.MappedType:
            case ts.SyntaxKind.IndexedAccessType:
            case ts.SyntaxKind.TypeOperator: // keyof | unique | readonly
            case ts.SyntaxKind.ImportType: // type T = import('xxx').SomeType
            default:
                return `(unsupported ${ts.SyntaxKind[typeNode.kind]})`;
        }
    }

    entityNameToString(name) {
        if (ts.isQualifiedName(name)) {
            // dots are kept as is when serializing qualified type names such as "A.B.C"
            return `${this.entityNameToString(name.left)}.${this.entityNameToString(name.right)}`;
        }
        return name.escapedText;
    }

    trackSymbol(typeNode) {
        // don't track complex type
        if (typeNode.typeArguments) {
            return;
        }
        const ty = this.checker.getTypeAtLocation(typeNode.typeName);
        // just track enums for now
        if (!ty || !(ty.getFlags() & ts.TypeFlags.EnumLike)) {
            return;
        }
        const sym = this.checker.getSymbolAtLocation(typeNode.typeName);
        this.ejson.symbols[sym.getName()] = 'enum';
        const testType = (ty, flag) => {
            return ty.types?.every(it => it.getFlags() & flag) || (ty.getFlags() & flag);
        };
        if (testType(ty, ts.TypeFlags.NumberLike)) {
            this.ejson.symbols[sym.getName()] = 'enum-numeric';
        } else if (testType(ty, ts.TypeFlags.StringLike)) {
            this.ejson.symbols[sym.getName()] = 'enum-string';
        }
    }

    guessType(value) {
        if (value.startsWith('"') && value.endsWith('"') ||
            value.startsWith('\'') && value.endsWith('\'')) {
            return '(string)';
        }
        if (!isNaN(parseFloat(value)) && !isNaN(value)) {
            return '(number)';
        }
        if (value.startsWith('false') || value.startsWith('true')) {
            return '(boolean)';
        }
        if (value !== fixme && value !== '') {
            console.log('WARNING: cannot infer type of ', value);
        }
        return `(unsupported ${ts.SyntaxKind[ts.SyntaxKind.Unknown]})`;
    }
}

for (const srcFile of program.getRootFileNames()) {
    const elejson = newjson();

    const sourceFile = program.getSourceFile(srcFile);
    const checker = program.getTypeChecker();

    let visitor = new ASTVisitor(sourceFile, checker, elejson);

    visitor.visit(sourceFile, visitor.ejson);

    visitor.processExports();

    const data = JSON.stringify(visitor.ejson, null, '\t');

    try {
        fs.writeFileSync(`${outPath}${filename}.json`, data);
        console.log(`${filePath.split('/')[filePath.split('/').length - 1]} is analyzed completely, json file saved!`);
    } catch (error) {
        console.error(error);
    }
}
