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
import * as fs from 'node:fs';
import * as path from 'node:path';

import { logTscDiagnostics } from '../utils/LogTscDiagnostics';
import { LogLevel, Logger } from '../../utils/logger/Logger';
import { Extension } from '../utils/Extension'; 

export function defaultCompilerOptions(): ts.CompilerOptions {
  return {
    target: ts.ScriptTarget.Latest,
    module: ts.ModuleKind.CommonJS,
    allowJs: true,
    checkJs: true
  };
}

export function compile(
  rootFileNames: readonly string[],
  compilerOptions?: ts.CompilerOptions,
  compilerHost?: ts.CompilerHost
): ts.Program {
  const program = ts.createProgram(rootFileNames, compilerOptions ?? defaultCompilerOptions(), compilerHost);
  const diagnostics = ts.getPreEmitDiagnostics(program);

  logTscDiagnostics(diagnostics, LogLevel.ERROR);

  return program;
}

export function parseConfigFile(tsconfig: string): ts.ParsedCommandLine | undefined {
  const host: ts.ParseConfigFileHost = ts.sys as ts.System & ts.ParseConfigFileHost;

  const diagnostics: ts.Diagnostic[] = [];

  let parsedConfigFile: ts.ParsedCommandLine | undefined;

  try {
    const oldUnrecoverableDiagnostic = host.onUnRecoverableConfigFileDiagnostic;
    host.onUnRecoverableConfigFileDiagnostic = (diagnostic: ts.Diagnostic): void => {
      diagnostics.push(diagnostic);
    };
    parsedConfigFile = ts.getParsedCommandLineOfConfigFile(tsconfig, {}, host);
    host.onUnRecoverableConfigFileDiagnostic = oldUnrecoverableDiagnostic;

    if (parsedConfigFile) {
      diagnostics.push(...ts.getConfigFileParsingDiagnostics(parsedConfigFile));
    }

    if (diagnostics.length > 0) {
      // Log all diagnostic messages and exit program.
      Logger.error('Failed to read config file.');
      logTscDiagnostics(diagnostics, LogLevel.ERROR);
      process.exit(-1);
    }
  } catch (error) {
    Logger.error('Failed to read config file: ' + error);
    process.exit(-1);
  }

  return parsedConfigFile;
}

export function getSourceFilesFromDir(dir: string): string[] {
  const resultFiles: string[] = [];

  for (const entity of fs.readdirSync(dir)) {
    const entityName = path.join(dir, entity);

    if (fs.statSync(entityName).isFile()) {
      const extension = path.extname(entityName);
      if (extension === ts.Extension.Ts || extension === ts.Extension.Tsx || extension === Extension.ETS) {
        resultFiles.push(entityName);
      }
    }

    if (fs.statSync(entityName).isDirectory()) {
      resultFiles.push(...getSourceFilesFromDir(entityName));
      continue;
    }
  }

  return resultFiles;
}
