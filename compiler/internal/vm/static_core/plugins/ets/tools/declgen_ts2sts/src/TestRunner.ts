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

import * as fs from 'node:fs';
import * as path from 'node:path';
import * as ts from 'typescript';
import { exit } from 'node:process';

import { Logger } from '../utils/logger/Logger';
import { ConsoleLogger } from '../utils/logger/ConsoleLogger';

import { CLI } from './cli/CLI';
import type { TestRunnerCLIOptions } from './cli/TestRunnerCLI';
import { TestRunnerCLI } from './cli/TestRunnerCLI';

import type { CheckerResult } from './Declgen';
import { Declgen } from './Declgen';
import type { FaultID } from '../utils/lib/FaultId';
import { Extension } from './utils/Extension';

interface Test {
  suite: string;
  name: string;
  outDir: string;
  rootDir?: string;
  testSource: Array<string>;
  expectedReport: string;
  expectedOutput: string;
}

interface TestResult {
  test: Test;
  outputsMatch: boolean;
  reportsMatch: boolean;
}

interface TestNodeInfo {
  line: number;
  column: number;
  faultId: FaultID;
}

interface TestReport {
  nodes: TestNodeInfo[];
}

function openFileWithoutComments(fileName: string): string {
  const code = fs.readFileSync(fileName).toString();
  const sf = ts.createSourceFile(
    fileName,
    code,
    ts.ScriptTarget.Latest,
    true,
    ts.ScriptKind.TS
  );
  const printer = ts.createPrinter({ removeComments: true, newLine: ts.NewLineKind.LineFeed });
  const result = printer.printFile(sf);

  return result;
}

function main(): void {
  Logger.init(new ConsoleLogger());

  const parsedCliOptions = CLI.parseCLI(new TestRunnerCLI());

  const testSuites = parsedCliOptions.testSuites.length ?
    parsedCliOptions.testSuites :
    readTestSuites(parsedCliOptions.testDirPath);

  const failedTests: TestResult[] = [];

  for (const testSuite of testSuites) {
    failedTests.push(...runTestSuite(testSuite, parsedCliOptions));

    Logger.info(`Finished running <${testSuite}> test suite`);
  }

  if (failedTests.length) {
    Logger.info('Failed tests:');

    for (const test of failedTests) {
      serializeTestResult(test);
    }

    exit(-1);
  }
}

function readDirRecursive(dir: string): string[] {
  const result: string[] = [];

  function walk(current: string): void {
    const entries = fs.readdirSync(current, { withFileTypes: true });

    for (const entry of entries) {
      const fullPath = path.join(current, entry.name);

      if (entry.isDirectory()) {
        walk(fullPath);
      } else {
        result.push(fullPath);
      }
    }
  }
  walk(dir);
  return result;
}



function serializeTestResult(failedTest: TestResult): void {
  Logger.info('');
  Logger.info(`Suite: ${failedTest.test.suite}`);
  Logger.info(`Test: ${failedTest.test.name}`);
  Logger.info(`Generated file: ${failedTest.outputsMatch ? '[SUCCESS]' : '[FAIL]'}`);
  Logger.info(`Generated report: ${failedTest.reportsMatch ? '[SUCCESS]' : '[FAIL]'}`);
}

function runTestSuite(testSuite: string, opts: TestRunnerCLIOptions): TestResult[] {
  Logger.info(`Running <${testSuite}> test suite`);

  const failedTests: TestResult[] = [];

  for (const test of collectTests(testSuite, opts)) {
    const res = runTest(test);

    if (!res.outputsMatch || !res.reportsMatch) {
      failedTests.push(res);
    }
  }

  return failedTests;
}

function runTest(test: Test): TestResult {
  Logger.info(`Running test ${test.name}`);

  const declgen = new Declgen({ outDir: test.outDir, rootDir: test.rootDir, inputFiles: test.testSource, inputDirs: [] });

  const { emitResult, checkResult } = declgen.run();

  const actualReport = toTestNodeInfo(checkResult);

  saveTestReport(actualReport, test);

  return {
    test: test,
    outputsMatch: compareExpectedAndActualOutputs(test, emitResult),
    reportsMatch: compareExpectedAndActualReports(test, actualReport)
  };
}

function toTestNodeInfo(checkResult: CheckerResult): TestReport {
  void checkResult;

  return { nodes: [] } as TestReport;
}

function saveTestReport(actualReport: TestReport, test: Test): void {
  if (!fs.existsSync(test.outDir)) {
    fs.mkdirSync(test.outDir);
  }

  const report = JSON.stringify(actualReport, undefined, 4);

  fs.writeFileSync(path.join(test.outDir, test.name + ts.Extension.Json), report);
}

function compareSingleFile(testName: string, expectedPath: string, actualPath: string): boolean {
  const expected = openFileWithoutComments(expectedPath);
  const actual = openFileWithoutComments(actualPath);
  
  if (expected !== actual) {
    Logger.error(`Outputs for ${testName} differ!
Expected (${expectedPath}):
'''
${expected}
'''
Actual (${actualPath}):
'''
${actual}
'''`);
    return false;
  }

  return true;
}

function compareMultipleFiles(test: Test): boolean {
  const expectedFiles = new Set(readDirRecursive(test.expectedOutput).map((f) => {
    const relativePath = path.relative(test.expectedOutput, f);
    return relativePath;
  }))
  const actualFiles = new Set(readDirRecursive(test.outDir).map((f) => {
    const relativePath = path.relative(test.outDir, f);
    return relativePath;
  }).filter((f) => f.endsWith('.d.ets')));

  if (expectedFiles.size !== actualFiles.size) {
    for (const expectedFile of expectedFiles) {
      if (!actualFiles.has(expectedFile)) {
        Logger.error(`Missing generated file: ${expectedFile}`);
      }
    }
    for (const actualFile of actualFiles) {
      if (!expectedFiles.has(actualFile)) {
        Logger.error(`Unexpected generated file: ${actualFile}`);
      }
    }
    return false;
  }

  let allMatch = true;

  for (const expectedFile of expectedFiles) {
    const expectedFilePath = path.join(test.expectedOutput, expectedFile);
    const actualFilePath = path.join(test.outDir, expectedFile);
    const match = compareSingleFile(test.name, expectedFilePath, actualFilePath);
    if (!match) {
      allMatch = false;
    }
  }
  return allMatch;
}

function compareExpectedAndActualOutputs(test: Test, emitResult: ts.EmitResult): boolean {
  void emitResult;

  if (test.suite !== 'package_source') {
    const actualPath = path.join(test.outDir, `${test.name}${Extension.DETS}`);
    return compareSingleFile(test.name, test.expectedOutput, actualPath);
  }
  else {
    return compareMultipleFiles(test);
  }
}

function compareExpectedAndActualReports(test: Test, actualReport: TestReport): boolean {
  const expectedReport: TestReport = JSON.parse(fs.readFileSync(test.expectedReport).toString());

  if (!expectedReport?.nodes || !Array.isArray(expectedReport.nodes)) {
    throw new Error(`Invalid format of the <${test.expectedReport}> file! JSON does not satisfy 'TestReport' format!`);
  }

  if (expectedReport.nodes.length !== actualReport.nodes.length) {
    Logger.error(
      `Results differ! Expected count: ${expectedReport.nodes.length} vs actual count: ${actualReport.nodes.length}`
    );

    return false;
  }

  let result = true;
  for (let i = 0; i < actualReport.nodes.length; ++i) {
    const actual = actualReport.nodes[i];
    const expected = expectedReport.nodes[i];

    if (!validateTestNodeInfo(expected)) {
      throw new Error(
        `Invalid format of the <${test.expectedReport}> file!` +
          `Node ${JSON.stringify(expected)} does not satisfy 'TestNodeInfo' format!`
      );
    }

    if (!areTestNodesEqual(expected, actual)) {
      Logger.error(`Expected node:\n${expected}\nActual node:\n${actual}`);
      result = false;
    }
  }

  return result;
}

function areTestNodesEqual(lhs: TestNodeInfo, rhs: TestNodeInfo): boolean {
  const columnsEq = lhs.column === rhs.column;
  const linesEq = lhs.line === rhs.line;
  const faultIdsEq = lhs.faultId === rhs.faultId;

  return columnsEq && linesEq && faultIdsEq;
}

function validateTestNodeInfo(node: TestNodeInfo): boolean {
  if (node === undefined) {
    return false;
  }

  if (node.column === undefined) {
    return false;
  }

  if (node.line === undefined) {
    return false;
  }

  if (node.faultId === undefined) {
    return false;
  }

  return true;
}

function collectTests(testSuite: string, opts: TestRunnerCLIOptions): Test[] {
  const testSuitePath = path.join(opts.testDirPath, testSuite);
  const testSuiteOutPath = path.join(opts.testOutPath, testSuite);
  const dirContents = fs.readdirSync(testSuitePath).filter((e) => {
    const dir = path.join(testSuitePath, e)
    return fs.statSync(dir).isFile() || (path.basename(dir).startsWith('arkts') && fs.statSync(dir).isDirectory());
  });
  const jsonDir = path.join(opts.testDirPath, 'json_storage');
  const detsDir = path.join(opts.testDirPath, 'dets_output');
  const basenames = new Set(
    dirContents.map((v) => {
      return v.split('.')[0];
    })
  );

  const tests: Test[] = [];

  for (const name of basenames) {
    let testSourceExtension = ''
    let inputFiles: Array<string> = []
    if (testSuite === 'ts_source') {
      testSourceExtension = ts.Extension.Ts;
    }
    else if (testSuite === 'dts_declarations') {
      testSourceExtension = ts.Extension.Dts;
    }
    let rootDir: string | undefined = testSuite === 'package_source' ? path.join(testSuitePath, name) : undefined;
    const testSource = `${name}${testSourceExtension}`;
    const expectedReport = path.join(jsonDir, name + ts.Extension.Json);
    const expectedOutput = testSuite === 'package_source' ? path.join(detsDir, name) : path.join(detsDir, name + Extension.DETS);

    if (!dirContents.includes(testSource)) {
      throw new Error(`Test ${name} is missing it's source file <${testSource}>!`);
    }
    if (!fs.existsSync(expectedReport)) {
      throw new Error(`Test ${name} is missing expected report file <${expectedReport}>!`);
    }
    if (!fs.existsSync(expectedOutput)) {
      throw new Error(`Test ${name} is missing expected output file <${expectedOutput}>!`);
    }

    if (testSuite !== 'package_source') {
      inputFiles.push(path.join(testSuitePath, testSource));
    }
    else {
      const inputJsonPath = path.join(testSuitePath, testSource, 'input.json');
      const inputJson = JSON.parse(fs.readFileSync(inputJsonPath).toString()) as Array<string>;
      inputFiles = inputJson.map((f) => path.join(testSuitePath, testSource, f));
    }

    tests.push({
      suite: testSuite,
      name: name,
      outDir: path.join(testSuiteOutPath, name),
      rootDir: rootDir,
      testSource: inputFiles,
      expectedOutput,
      expectedReport
    });
  }

  return tests;
}

function readTestSuites(testDirPath: string): string[] {
  const testSuites: string[] = [];

  for (const entity of fs.readdirSync(testDirPath)) {
    const entityPath = path.join(testDirPath, entity);
    if (fs.statSync(entityPath).isDirectory()) {
      testSuites.push(entity);
    }
  }

  return testSuites;
}

main();
