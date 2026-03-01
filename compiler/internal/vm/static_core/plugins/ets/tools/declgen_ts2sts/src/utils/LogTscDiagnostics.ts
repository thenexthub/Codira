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

import * as ts from 'typescript';

import { Logger, type LogLevel } from '../../utils/logger/Logger';

export function logTscDiagnostics(diagnostics: readonly ts.Diagnostic[], logLevel: LogLevel): void {
  for (const diagnostic of diagnostics) {
    let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n');

    if (diagnostic.file && diagnostic.start) {
      const { line, character } = ts.getLineAndCharacterOfPosition(diagnostic.file, diagnostic.start);
      message = `${diagnostic.file.fileName} (${line + 1}, ${character + 1}): ${message}`;
    }

    Logger[logLevel](message);
  }
}
