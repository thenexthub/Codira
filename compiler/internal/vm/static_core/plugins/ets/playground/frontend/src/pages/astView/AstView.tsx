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

import React, {useEffect} from 'react';
import Editor, {useMonaco} from '@monaco-editor/react';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useDispatch, useSelector} from 'react-redux';
import {selectCode} from '../../store/selectors/code';
import * as monaco from 'monaco-editor/esm/vs/editor/editor.api';
import { loader } from '@monaco-editor/react';
import { selectASTLoading, selectASTRes } from '../../store/selectors/ast';
import { AppDispatch } from '../../store';
import { fetchAst } from '../../store/actions/ast';
import { selectAstMode } from '../../store/selectors/features';

loader.config({ monaco });
// @ts-ignore
globalThis.MonacoEnvironment = {
    // @ts-ignore
    getWorker(_, label): Worker {
        if (label === 'json') {
            return new Worker(
                new URL('monaco-editor/esm/vs/language/json/json.worker?worker', import.meta.url),
                { type: 'module' }
            );
        }
        return new Worker(
            new URL('monaco-editor/esm/vs/editor/editor.worker?worker', import.meta.url),
            { type: 'module' }
        );
    }
};

const AstView: React.FC = () => {
    const dispatch = useDispatch<AppDispatch>();
    const monaco = useMonaco();
    const {theme} = useTheme();
    const [astCode, setASTCode] = React.useState('');
    const astRes = useSelector(selectASTRes);
    const isASTLoading = useSelector(selectASTLoading);
    const code = useSelector(selectCode);
    const astmode = useSelector(selectAstMode);

    useEffect(() => {
        if (astRes?.ast) {
            setASTCode(astRes.ast);
        } else if (astRes?.output) {
            setASTCode(astRes.output);
        }
    }, [astRes]);

    useEffect(() => {
        if (monaco) {
            const existingLang = monaco.languages.getLanguages()
                // @ts-ignore
                .find(lang => lang.id === 'json');
            if (!existingLang) {
                monaco.languages.register({ id: 'json' });
            }
        }
    }, [monaco]);

    useEffect(() => {
        if (astmode === 'auto') {
            dispatch(fetchAst())
        }
    }, [code, astmode])

    return (
        <Editor
            language={'json'}
            defaultValue={astCode}
            value={isASTLoading ? 'Loading...' : astCode}
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            className={styles.editor}
            height="100%"
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
                automaticLayout: true
            }}
        />
    );
};

export default AstView;
