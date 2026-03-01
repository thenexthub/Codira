/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import { IObj } from '../store/slices/options';

export type VerificationMode = 'disabled' | 'ahead-of-time' | 'on-the-fly';

export interface IIrDumpOptions {
    compiler_dump: boolean;
    disasm_dump: boolean;
}

export interface ICodeFetch {
    code: string
    disassemble: boolean
    verifier: boolean
    verification_mode?: VerificationMode
    aot_mode?: boolean
    ir_dump?: IIrDumpOptions | null
    options: IObj | null
}
export interface ICodeShare {
    code: string
    options: IObj | null
}
export interface IShareReq {
    uuid: string
}
export interface IIrDumpResponse {
    output: string,
    compiler_dump: string | null,
    disasm_dump: string | null,
    error: string,
    exit_code?: number
}

export interface ICodeReq {
    compile: {
        output: string,
        error: string,
        exit_code?: number
    },
    disassembly: {
        output: string,
        code: string,
        error: string,
        exit_code?: number
    },
    verifier: {
        output: string,
        error: string,
        exit_code?: number
    },
    ir_dump?: IIrDumpResponse
}
export interface IRunReq {
    compile: {
        output: string,
        error: string,
        exit_code?: number
    },
    disassembly: {
        output: string,
        code: string,
        error: string,
        exit_code?: number
    },
    run: {
        output: string,
        error: string,
        exit_code?: number
    },
    run_aot?: {
        output: string,
        error: string,
        exit_code?: number
    },
    verifier: {
        output: string,
        error: string,
        exit_code?: number
    },
    ir_dump?: IIrDumpResponse
}

export const codeModel = {
    fillDefaults: <T extends Record<keyof T, string | number>>(
        data: Partial<T>,
        defaults: T
    ): T => {
        return Object.keys({...defaults, exit_code: null}).reduce((acc, key) => {
            const typedKey = key as keyof T;
            acc[typedKey] = data[typedKey] ?? defaults[typedKey];
            return acc;
        }, { ...defaults });
    },
    fillIrDumpDefaults: (data: IIrDumpResponse): IIrDumpResponse => ({
        output: data.output ?? '',
        compiler_dump: data.compiler_dump ?? null,
        disasm_dump: data.disasm_dump ?? null,
        error: data.error ?? '',
        exit_code: data.exit_code,
    }),
    fromApiCompile: (data: ICodeReq): ICodeReq => ({
        compile: codeModel.fillDefaults(data.compile || {}, { output: '', error: '' }),
        disassembly: codeModel.fillDefaults(data.disassembly || {}, { output: '', code: '', error: '' }),
        verifier: codeModel.fillDefaults(data.verifier || {}, { output: '', error: '' }),
        ir_dump: data.ir_dump ? codeModel.fillIrDumpDefaults(data.ir_dump) : undefined,
    }),
    fromApiRun: (data: IRunReq): IRunReq => ({
        compile: codeModel.fillDefaults(data.compile || {}, { output: '', error: '' }),
        disassembly: codeModel.fillDefaults(data.disassembly || {}, { output: '', code: '', error: '' }),
        run: codeModel.fillDefaults(data.run || {}, { output: '', error: '' }),
        run_aot: data.run_aot ? codeModel.fillDefaults(data.run_aot, { output: '', error: '' }) : undefined,
        verifier: codeModel.fillDefaults(data.verifier || {}, { output: '', error: '' }),
        ir_dump: data.ir_dump ? codeModel.fillIrDumpDefaults(data.ir_dump) : undefined,
    }),
};
