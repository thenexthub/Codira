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

const path = require('path');
const rootDir = path.resolve(__dirname); // Получаем абсолютный путь к корневой директории проекта
console.log('rootDir:', rootDir);
console.log('Path to importJestDOM:', path.join(rootDir, '__tests__/config/importJestDOM.ts'));

module.exports = {
    preset: 'ts-jest',
    testEnvironment: 'jsdom',
    rootDir: './',
    transformIgnorePatterns: ['/node_modules/(?!monaco-editor).+\\\\.js$'],
    transform: {
        '^.+\\.(ts|tsx)$': ['ts-jest', { tsconfig: 'tsconfig.test.json', babelConfig: {} }],
        '^.+\\.(js|jsx)$': 'babel-jest',
    },
    coverageDirectory: 'coverage',
    collectCoverageFrom: [
        '<rootDir>/src/**/*.{ts,tsx}',
        '!<rootDir>/**/__mocks__/*.{ts,tsx}'
    ],
    coveragePathIgnorePatterns: [
        '/node_modules/',
        '/coverage/',
        '/public/',
        '/src/index.tsx',
        '/src/utils',
        '/src/App.tsx',
        '/src/reportWebVitals.ts',
        '/src/setupTests.ts',
        '/src/components/mosaic'
    ],
    moduleNameMapper: {
        '^.+\\.(css|less|scss)$': 'identity-obj-proxy',
        '^@blueprintjs/core$': '<rootDir>/__mocks__/@blueprintjs/core.js',
        '^@monaco-editor/react$': '<rootDir>/__mocks__/@monaco-editor/react.js',
        '^monaco-editor$': '<rootDir>/__mocks__/monaco-editor/monaco-editor.js',
        '\\.(jpg|ico|jpeg|png|gif|eot|otf|webp|svg|ttf|woff|woff2|mp4|webm|wav|mp3|m4a|aac|oga)$': '<rootDir>/__tests__/mocks/imageMock.js',
    },
    setupFilesAfterEnv: ['<rootDir>/__tests__/config/importJestDom.ts'],
    unmockedModulePathPatterns: [
        '<rootDir>/src/'
    ],
    testPathIgnorePatterns: [
        'node_modules',
        '<rootDir>/__tests__/config/',
        '<rootDir>/__tests__/mocks/',
        '<rootDir>/__tests__/constants/',
        '<rootDir>/__tests__/utils/',
    ],
    automock: false,
};
