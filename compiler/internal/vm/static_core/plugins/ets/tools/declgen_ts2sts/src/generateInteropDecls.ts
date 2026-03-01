/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from 'typescript';
import { Logger } from '../utils/logger/Logger';
import { logMessages, SilentLogger } from '../utils/logger/SilentLogger';
import { DeclgenCLIOptions } from './cli/DeclgenCLI';
import { CodeInput, Declgen } from './Declgen';

export interface RunnerParms {
  inputFiles: string[];
  inputDirs: string[];
  outDir: string;
  rootDir?: string;
  customResolveModuleNames?: (moduleName: string[], containingFile: string) => ts.ResolvedModuleFull[];
  customCompilerOptions?: ts.CompilerOptions;
  includePaths?: string[];
  uiInteropTransformer?: Function;
}

export interface CodeInputParams {
  codeInputs: CodeInput[];
  outDir: string;
  rootDir?: string;
  customResolveModuleNames?: (moduleName: string[], containingFile: string) => ts.ResolvedModuleFull[];
  customCompilerOptions?: ts.CompilerOptions;
  includePaths?: string[];
}

export function generateInteropDecls(config: RunnerParms): string[] {
  Logger.init(new SilentLogger());

  const tsConfig: DeclgenCLIOptions = {
    outDir: config.outDir,
    inputFiles: config.inputFiles,
    inputDirs: config.inputDirs,
    rootDir: config.rootDir,
    tsconfig: undefined,
    includePaths: config.includePaths
  }
  const declgen = new Declgen(tsConfig, config.customResolveModuleNames, config.customCompilerOptions, [], config.uiInteropTransformer);
  declgen.run();
  return logMessages;
}

//Create ArkTS1.2 declaration file based on file contend.
export function generateInteropDeclsFromCode(config: CodeInputParams): string[] {
  Logger.init(new SilentLogger());

  const tsConfig: DeclgenCLIOptions = {
    outDir: config.outDir,
    inputFiles: [],
    inputDirs: [],
    rootDir: config.rootDir,
    tsconfig: undefined,
    includePaths: config.includePaths
  }

  const declgen = new Declgen(
    tsConfig, 
    config.customResolveModuleNames, 
    config.customCompilerOptions,
    config.codeInputs
  );

  declgen.run();
  return logMessages;
}