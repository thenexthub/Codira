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

import path from 'node:path';
import { fileURLToPath } from 'node:url';
import js from '@eslint/js';
import { FlatCompat } from '@eslint/eslintrc';
import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const compat = new FlatCompat({
  baseDirectory: __dirname,
  recommendedConfig: js.configs.recommended,
  allConfig: js.configs.all
});

export default tseslint.config(
  {
    ignores: [
      'bin/**/*',
      'build/**/*',
      'bundle/**/*',
      'dist/**/*',
      'docs/**/*',
      'node_modules/**/*',
      'scripts/**/*',
      'test/**/*',
      '**/**.json',
      '**/**.js'
    ]
  },
  {
    files: ['**/*.ts'],
    extends: [eslint.configs.recommended, tseslint.configs.recommended],
    plugins: {
      '@typescript-eslint': tseslint.plugin
    },

    languageOptions: {
      parser: tseslint.parser,
      ecmaVersion: 'latest',
      sourceType: 'module',
      parserOptions: {
        project: './tsconfig.json',
        tsconfigRootDir: import.meta.dirname
      }
    },

    rules: {
      'arrow-body-style': ['error', 'always'],
      camelcase: 'off',

      'class-methods-use-this': [
        'error',
        {
          exceptMethods: [],
          enforceForClassFields: true
        }
      ],

      complexity: [
        'error',
        {
          max: 15
        }
      ],

      'consistent-return': [
        'error',
        {
          treatUndefinedAsUnspecified: false
        }
      ],

      curly: ['error', 'all'],
      'dot-notation': 'error',
      eqeqeq: ['error', 'always'],

      'max-depth': [
        'error',
        {
          max: 4
        }
      ],

      'multiline-comment-style': ['error', 'starred-block'],

      'no-else-return': [
        'error',
        {
          allowElseIf: true
        }
      ],

      'no-extra-bind': 'error',
      'no-lonely-if': 'error',
      'no-unneeded-ternary': 'error',
      'no-useless-return': 'error',
      'no-var': 'error',
      'prefer-const': 'error',
      'spaced-comment': ['error', 'always'],
      'one-var': ['error', 'never'],

      'max-lines-per-function': [
        'error',
        {
          max: 50
        }
      ],

      '@typescript-eslint/explicit-function-return-type': 'error',
      '@typescript-eslint/adjacent-overload-signatures': 'error',

      '@typescript-eslint/explicit-member-accessibility': [
        'error',
        {
          accessibility: 'no-public'
        }
      ],

      '@typescript-eslint/method-signature-style': ['error', 'method'],
      '@typescript-eslint/no-confusing-non-null-assertion': 'error',
      '@typescript-eslint/no-confusing-void-expression': 'error',
      '@typescript-eslint/no-explicit-any': 'warn',
      '@typescript-eslint/no-extra-non-null-assertion': 'error',
      '@typescript-eslint/no-meaningless-void-operator': 'error',
      '@typescript-eslint/no-unnecessary-boolean-literal-compare': 'error',
      '@typescript-eslint/no-unnecessary-condition': 'off',
      '@typescript-eslint/no-unnecessary-type-assertion': 'error',
      '@typescript-eslint/prefer-as-const': 'error',
      '@typescript-eslint/prefer-optional-chain': 'error',
      '@typescript-eslint/prefer-readonly': 'error',
      '@typescript-eslint/consistent-type-imports': 'error',

      '@typescript-eslint/naming-convention': [
        'error',
        {
          selector: 'typeLike',
          format: ['PascalCase']
        }
      ]
    }
  }
);
