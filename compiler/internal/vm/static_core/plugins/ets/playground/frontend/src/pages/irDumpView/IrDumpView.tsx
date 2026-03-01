/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import React, {useEffect} from 'react';
import Editor from '@monaco-editor/react';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useSelector} from 'react-redux';
import {selectCompileLoading, selectRunLoading, selectRunRes, selectCompileRes} from '../../store/selectors/code';

export const DisasmDumpView: React.FC = () => {
    const {theme} = useTheme();
    const [code, setCode] = React.useState('');
    const runRes = useSelector(selectRunRes);
    const compileRes = useSelector(selectCompileRes);
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);

    useEffect(() => {
        if (isCompileLoading || isRunLoading) {
            setCode('');
            return;
        }
        if (compileRes?.ir_dump?.disasm_dump) {
            setCode(compileRes.ir_dump.disasm_dump);
        } else if (runRes?.ir_dump?.disasm_dump) {
            setCode(runRes.ir_dump.disasm_dump);
        } else if (compileRes !== null || runRes !== null) {
            setCode('');
        }
    }, [runRes, compileRes, isCompileLoading, isRunLoading]);

    return (
        <Editor
            defaultLanguage="plaintext"
            defaultValue={code}
            value={isCompileLoading || isRunLoading ? 'Loading...' : code}
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            className={styles.editor}
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
                automaticLayout: true
            }}
        />
    );
};

export const CompilerDumpView: React.FC = () => {
    const {theme} = useTheme();
    const [code, setCode] = React.useState('');
    const runRes = useSelector(selectRunRes);
    const compileRes = useSelector(selectCompileRes);
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);

    useEffect(() => {
        if (isCompileLoading || isRunLoading) {
            setCode('');
            return;
        }
        if (compileRes?.ir_dump?.compiler_dump) {
            setCode(compileRes.ir_dump.compiler_dump);
        } else if (runRes?.ir_dump?.compiler_dump) {
            setCode(runRes.ir_dump.compiler_dump);
        } else if (compileRes !== null || runRes !== null) {
            setCode('');
        }
    }, [runRes, compileRes, isCompileLoading, isRunLoading]);

    return (
        <Editor
            defaultLanguage="plaintext"
            defaultValue={code}
            value={isCompileLoading || isRunLoading ? 'Loading...' : code}
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            className={styles.editor}
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
                automaticLayout: true
            }}
        />
    );
};

export default DisasmDumpView;
