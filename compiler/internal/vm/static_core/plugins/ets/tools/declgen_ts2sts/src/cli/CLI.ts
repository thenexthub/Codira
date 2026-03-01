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

import type { OptionValues } from 'commander';
import type { Command } from 'commander';

export abstract class CLI<OptionsT> {
  static parseCLI<T>(cli: CLI<T>): T {
    const command = cli.doInit();
    const args = cli.doRead();
    const parsedOpts = command.parse(args).opts();
    const opts = cli.doOptions(parsedOpts);
    const err = cli.doValidate(opts);

    if (err) {
      throw err;
    }

    return opts;
  }

  abstract doRead(): string[];

  abstract doInit(): Command;

  abstract doOptions(opts: OptionValues): OptionsT;

  abstract doValidate(opts: OptionsT): Error | undefined;
}
