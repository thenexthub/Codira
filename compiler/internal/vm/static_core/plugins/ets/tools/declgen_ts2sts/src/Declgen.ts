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

import * as fs from 'fs';
import * as ts from 'typescript';
import * as path from 'path';

import { defaultCompilerOptions, getSourceFilesFromDir, compile, parseConfigFile } from './compiler/Compiler';
import type { DeclgenCLIOptions } from './cli/DeclgenCLI';
import { Logger } from '../utils/logger/Logger';

import { Transformer } from './ASTTransformer';
import { Autofixer } from './ASTAutofixer';
import { Checker } from './ASTChecker';
import { Extension } from './utils/Extension';

export type CheckerResult = ts.Diagnostic[];

export interface CodeInput {
  fileName: string;
  content: string;
}

export interface DeclgenResult {
  emitResult: ts.EmitResult;
  checkResult: CheckerResult;
}

export class Declgen {
  private readonly sourceFileMap: Map<string, ts.SourceFile>;
  private readonly hookedHost: ts.CompilerHost;
  private readonly rootFiles: readonly string[];
  private readonly compilerOptions: ts.CompilerOptions;
  private readonly declgenOptions: DeclgenCLIOptions;
  private readonly codeInputs?: CodeInput[];
  private visitedFiles: Map<string, ts.SourceFile>;
  private writedFiles: Set<string>;
  private uiInteropTransformer?: Function;

  constructor(
    declgenOptions: DeclgenCLIOptions,
    customResolveModuleNames?: (moduleName: string[], containingFile: string) => ts.ResolvedModuleFull[],
    compilerOptions?: ts.CompilerOptions,
    codeInputs?: CodeInput[],
    uiInteropTransformer?: Function
  ) {
    const { rootNames, options } = Declgen.parseDeclgenOptions(declgenOptions);

    this.rootFiles = rootNames;
    this.declgenOptions = declgenOptions;
    this.codeInputs = codeInputs;
    this.sourceFileMap = new Map<string, ts.SourceFile>();
    this.visitedFiles = new Map<string, ts.SourceFile>();
    this.writedFiles = new Set<string>();
    this.uiInteropTransformer = uiInteropTransformer;

    // Create SourceFile based on code content.
    if (codeInputs) {
      for (const input of codeInputs) {
        const sourceFile = ts.createSourceFile(
          input.fileName,
          input.content,
          ts.ScriptTarget.Latest,
          true
        );
        this.sourceFileMap.set(input.fileName, sourceFile);
      }
    }

    this.compilerOptions = Object.assign({}, options, {
      declaration: true,
      emitDeclarationOnly: true,
      outDir: declgenOptions.outDir,
      ...(declgenOptions.rootDir ? { rootDir: declgenOptions.rootDir } : {}),
      ...(compilerOptions ?? {})
    });
    // Prevent the noemit of the passed compilerOptions from being true
    this.compilerOptions.noEmit = false;
    this.compilerOptions.experimentalDecorators = true;
    this.hookedHost = Declgen.createHookedCompilerHost(this.sourceFileMap, this.compilerOptions, declgenOptions, this.writedFiles);
    if (customResolveModuleNames) {
      this.hookedHost.resolveModuleNames = customResolveModuleNames;
    }
  }

  run(): DeclgenResult {
    /**
     * First compilation with the hooked CompilerHost:
     * collect the SourceFiles after transformation to the hooked Map
     */
    let program: ts.Program

    if (this.codeInputs && this.codeInputs.length > 0) {
      program = this.recompileWithCodeInputs();
    } else {
      program = this.recompile();
    }

    /**
     * If the rootFiles contains declaration files,
     * process the declaration files here.
     */
    this.processDeclarationFiles(program);
    const afterDeclarations: ts.CustomTransformerFactory[] = [
      ...(this.uiInteropTransformer ? [this.uiInteropTransformer(program)] : []),
      (ctx: ts.TransformationContext): ts.CustomTransformer => {
        const typeChecker = program.getTypeChecker();
        const autofixer = new Autofixer(typeChecker, ctx);
        const transformer = new Transformer(ctx, this.sourceFileMap, [autofixer.fixNode.bind(autofixer)]);

        return transformer.createCustomTransformer((sourceFile: ts.SourceFile) => {
          this.visitedFiles.set(sourceFile.fileName, sourceFile);
        });
      }
    ];

    const emitResult = program.emit(undefined, undefined, undefined, true, {
      before: [],
      after: [],
      afterDeclarations
    });

    this.visitedFiles.forEach((sourceFile, fileName) => {
        const outDir = this.compilerOptions.outDir || path.dirname(fileName);
        const rootDir = this.compilerOptions.rootDir || path.dirname(fileName);
        const relativePath = path.relative(rootDir, fileName);
        const parsedPath = path.parse(path.join(outDir, relativePath));
        const dEtsFilePath = path.join(parsedPath.dir, `${parsedPath.name}${Extension.DETS}`);
        if (!this.writedFiles.has(dEtsFilePath) && Declgen.isFileInAllowedPath(this.declgenOptions, [sourceFile])) {
          const outPath = path.dirname(dEtsFilePath);
          if (!fs.existsSync(outPath)) {
            fs.mkdirSync(outPath, { recursive: true });
          }
          const printer = ts.createPrinter({
            newLine: ts.NewLineKind.LineFeed
          });
          const transformedCode = printer.printFile(sourceFile);
          const finalCode = `'use static'\n${transformedCode}`;
          fs.writeFileSync(dEtsFilePath, finalCode, { encoding: 'utf-8' });
        }
    });

    return {
      emitResult: emitResult
    } as DeclgenResult;
  }

  private recompile(): ts.Program {
    return compile(this.rootFiles, this.compilerOptions, this.hookedHost);
  }

  private processDeclarationFiles(program: ts.Program): void {
    const typeChecker = program.getTypeChecker();

    this.rootFiles.forEach((fileName) => {
      const sourceFile = program.getSourceFile(fileName);
      if (sourceFile && sourceFile.isDeclarationFile) {
        this.processDeclarationFile(program, sourceFile, typeChecker);
      }
    });

    // Handle declaration files when converting based on file content.
    if (this.codeInputs && this.codeInputs.length > 0) {
      for (const codeInput of this.codeInputs) {
        const sourceFile = program.getSourceFile(codeInput.fileName);
        if (sourceFile && sourceFile.isDeclarationFile) {
          this.processDeclarationFile(program, sourceFile, typeChecker);
        }
      }
    }
  }

  private processDeclarationFile(program: ts.Program, sourceFile: ts.SourceFile, typeChecker: ts.TypeChecker): void {
    const compilerOptions = program.getCompilerOptions();
    const outDir = compilerOptions.outDir || path.dirname(sourceFile.fileName);
    const rootDir = compilerOptions.rootDir || path.dirname(sourceFile.fileName);
    const relativePath = path.relative(rootDir, sourceFile.fileName);
    const fileNameWithoutExt = relativePath.replace(/\.d\.(ts|ets)$/, '');
    const dEtsFilePath = path.join(outDir, `${fileNameWithoutExt}${Extension.DETS}`);

    const result = this.transformDeclarationFiles(program, sourceFile, typeChecker);
    const printer = ts.createPrinter();
    const transformedCode = printer.printFile(result.transformed[0] as ts.SourceFile);

    if (Declgen.isFileInAllowedPath(this.declgenOptions, [sourceFile])) {
      const outPath = path.dirname(dEtsFilePath);
      const finalCode = `'use static'\n${transformedCode}`;
      this.writedFiles.add(dEtsFilePath);
      if (!fs.existsSync(outPath)) {
        fs.mkdirSync(outPath, { recursive: true });
      }
      fs.writeFileSync(dEtsFilePath, finalCode, { encoding: 'utf8' });
    }
  }

  private transformDeclarationFiles(program: ts.Program, sourceFile: ts.SourceFile, typeChecker: ts.TypeChecker): ts.TransformationResult<ts.Node> { 
    const result = ts.transform(
      sourceFile,
      [
        (context: ts.TransformationContext): ts.Transformer<ts.SourceFile> => {
          const autofixer = new Autofixer(typeChecker, context);
          const visit = (node: ts.Node): ts.Node | undefined => {
            const fixedNode = autofixer.fixNode(node);

            if (fixedNode === undefined) {
              return node;
            } else if (Array.isArray(fixedNode)) {
              return context.factory.createBlock(fixedNode as readonly ts.Statement[]);
            } else {
              return ts.visitEachChild(fixedNode, visit, context);
            }
          };
          return (sourceFile: ts.SourceFile): ts.SourceFile => {
            return visit(sourceFile) as ts.SourceFile;
          };
        }
      ],
      program.getCompilerOptions()
    );

    return result;
  }

  private static createHookedCompilerHost(
    sourceFileMap: Map<string, ts.SourceFile>,
    compilerOptions: ts.CompilerOptions,
    declgenOptions: DeclgenCLIOptions,
    writedFiles: Set<string>
  ): ts.CompilerHost {
    const host = ts.createCompilerHost(compilerOptions);
    const fallbackGetSourceFile = host.getSourceFile;
    const fallbackWriteFile = host.writeFile;

    return Object.assign(host, {
      getSourceFile(
        fileName: string,
        languageVersionOrOptions: ts.ScriptTarget | ts.CreateSourceFileOptions,
        onError?: (message: string) => void,
        shouldCreateNewSourceFile?: boolean
      ): ts.SourceFile | undefined {
        return (
          sourceFileMap.get(fileName) ??
          fallbackGetSourceFile(fileName, languageVersionOrOptions, onError, shouldCreateNewSourceFile)
        );
      },
      writeFile(
        fileName: string,
        text: string,
        writeByteOrderMark: boolean,
        onError?: (message: string) => void,
        sourceFiles?: readonly ts.SourceFile[],
        data?: ts.WriteFileCallbackData
      ) {
        if (!Declgen.isFileInAllowedPath(declgenOptions, sourceFiles)) {
          return;
        }
        const newText = `'use static'\n${text}`;
        const parsedPath = path.parse(fileName);
        const outPath = path.join(parsedPath.dir, `${parsedPath.name}${Extension.ETS}`);
        if (writedFiles.has(outPath)) {
          return;
        }
        writedFiles.add(outPath);
        fallbackWriteFile(
          /*
           * Since `.d` part of `.d.ts` extension is a part of the parsedPath.name,
           * use `Extension.Ets` for output file name generation.
           */
          outPath,
          newText,
          writeByteOrderMark,
          onError,
          sourceFiles,
          data
        );
      }
    });
  }

  private static isFileInAllowedPath(
    declgenOptions: DeclgenCLIOptions,
    sourceFiles?: readonly ts.SourceFile[]
  ): boolean {
    // equal to includePaths = [*]
    if (!declgenOptions.includePaths?.length) {
      return true;
    }
    if (!sourceFiles?.length) {
      return false;
    }

    const filePath = path.resolve(sourceFiles[0].fileName);
    return declgenOptions.includePaths.some((allowedPath) => {
      const base = path.normalize(path.resolve(allowedPath) + path.sep);
      const target = path.normalize(filePath);
      return target.startsWith(base);
    });
  }

  static parseDeclgenOptions(opts: DeclgenCLIOptions): ts.CreateProgramOptions {
    const parsedConfigFile = opts.tsconfig ? parseConfigFile(opts.tsconfig) : undefined;

    if (parsedConfigFile) {
      return {
        rootNames: parsedConfigFile.fileNames,
        options: parsedConfigFile.options,
        projectReferences: parsedConfigFile.projectReferences,
        configFileParsingDiagnostics: ts.getConfigFileParsingDiagnostics(parsedConfigFile)
      };
    }
    return {
      rootNames: Declgen.collectInputFiles(opts),
      options: defaultCompilerOptions()
    };
  }

  private static collectInputFiles(opts: DeclgenCLIOptions): string[] {
    const inputFiles = [] as string[];

    if (opts.inputFiles) {
      inputFiles.push(...opts.inputFiles);
    }

    if (opts.inputDirs) {
      for (const dir of opts.inputDirs) {
        try {
          inputFiles.push(...getSourceFilesFromDir(dir));
        } catch (error) {
          Logger.error('Failed to read folder: ' + error);
          process.exit(-1);
        }
      }
    }

    return inputFiles;
  }

  private recompileWithCodeInputs(): ts.Program {
    const rootFileNames = this.codeInputs!.map(input => input.fileName);
    return compile(rootFileNames, this.compilerOptions, this.hookedHost);
  }
}
