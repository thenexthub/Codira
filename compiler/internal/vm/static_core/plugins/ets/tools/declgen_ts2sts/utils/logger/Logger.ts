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

export interface ILogger {
  doTrace(message: string): void;
  doDebug(message: string): void;
  doInfo(message: string): void;
  doWarn(message: string): void;
  doError(message: string): void;
}

export enum LogLevel {
  TRACE = 'trace',
  DEBUG = 'debug',
  INFO = 'info',
  WARN = 'warn',
  ERROR = 'error'
}

export class Logger {
  // Forbids extension and instantiation
  private constructor() {}

  static init(instance: ILogger): void {
    this.instance = instance;
  }

  static [LogLevel.TRACE](message: string): void {
    this.getInstance().doTrace(message);
  }

  static [LogLevel.DEBUG](message: string): void {
    this.getInstance().doDebug(message);
  }

  static [LogLevel.INFO](message: string): void {
    this.getInstance().doInfo(message);
  }

  static [LogLevel.WARN](message: string): void {
    this.getInstance().doWarn(message);
  }

  static [LogLevel.ERROR](message: string): void {
    this.getInstance().doError(message);
  }

  private static getInstance(): ILogger {
    if (!this.instance) {
      throw new Error('Not initialized');
    }
    return this.instance;
  }

  private static instance?: ILogger = undefined;
}
