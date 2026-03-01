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

import * as fs from 'node:fs';
import * as path from 'node:path';

import type { OptionValues } from 'commander';
import { Command } from 'commander';

import { CLI } from './CLI';

export interface TestRunnerCLIOptions {
  testSuites: string[];
  testDirPath: string;
  testOutPath: string;
}

export class TestRunnerCLI extends CLI<TestRunnerCLIOptions> {
  constructor() {
    super();
  }

  doRead(): string[] {
    void this;

    return process.argv;
  }

  doInit(): Command {
    void this;

    const cliParser = new Command();

    cliParser.name('declgen_test_runner').description('Test runner for declgen.');
    cliParser.requiredOption('--testDirPath <name>', 'Directory with the test suites.');
    cliParser.requiredOption('--testOutPath <name>', 'Directory where the test artifacts should be stored.');
    cliParser.option(
      '-t, --testSuite <name>',
      'Test suite to run. Test suite name is the same as the name of the folder in <testDirPath>/ directory.',
      (val, prev) => {
        return prev.concat(val);
      },
      [] as string[]
    );

    return cliParser;
  }

  doOptions(opts: OptionValues): TestRunnerCLIOptions {
    void this;

    return {
      testSuites: opts.testSuite,
      testDirPath: opts.testDirPath,
      testOutPath: opts.testOutPath
    };
  }

  doValidate(opts: TestRunnerCLIOptions): Error | undefined {
    void this;

    if (!fs.existsSync(opts.testDirPath)) {
      return new Error(`Invalid testDirPath agrument! No directory <${opts.testDirPath}> exists!`);
    }

    for (const testSuite of opts.testSuites) {
      if (!fs.existsSync(path.join(opts.testDirPath, testSuite))) {
        return new Error(`No test suite <${testSuite}> exisist in the test directory <${opts.testDirPath}>`);
      }
    }

    return undefined;
  }
}
