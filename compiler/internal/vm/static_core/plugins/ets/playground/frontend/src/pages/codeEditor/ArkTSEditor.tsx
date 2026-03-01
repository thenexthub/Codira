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

import React, {useEffect, useRef, useState} from 'react';
import Editor, {useMonaco} from '@monaco-editor/react';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useDispatch, useSelector} from 'react-redux';
import {selectSyntax} from '../../store/selectors/syntax';
import {setCodeAction} from '../../store/actions/code';
import {AppDispatch} from '../../store';
import {selectCode} from '../../store/selectors/code';
import debounce from 'lodash.debounce';
import * as monaco from 'monaco-editor';
import { editor as monacoEditor } from 'monaco-editor';
import { loader } from '@monaco-editor/react';
import { selectHighlightErrors, selectJumpTo } from '../../store/selectors/logs';

loader.config({ monaco });

const ArkTSEditor: React.FC = () => {
    let backendSyntax = useSelector(selectSyntax);
    const monaco = useMonaco();
    const dispatch = useDispatch<AppDispatch>();
    const {theme} = useTheme();
    const [syn, setSyn] = useState(backendSyntax);
    const highlightErrors = useSelector(selectHighlightErrors);
    const jumpTo = useSelector(selectJumpTo);
    const codeFromStore = useSelector(selectCode);
    const [editorInstance, setEditorInstance] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
    const [cursorPos, setCursorPos] = useState<{ lineNumber: number; column: number }>({ lineNumber: 1, column: 1 });

    useEffect(() => {
        let cleanup: (() => void) | undefined;
        if (editorInstance) {
            setCursorPos(editorInstance.getPosition() ?? { lineNumber: 1, column: 1 });
            const sub = editorInstance.onDidChangeCursorPosition(e => setCursorPos(e.position));
            cleanup = (): void => sub.dispose();
        }
        return cleanup;
    }, [editorInstance]);

    useEffect(() => {
        if (!editorInstance || !jumpTo) {
            return;
        }
        const pos = { lineNumber: jumpTo.line, column: jumpTo.column || 1 };

        editorInstance.setSelection({
          startLineNumber: pos.lineNumber,
          startColumn: pos.column,
          endLineNumber: pos.lineNumber,
          endColumn: pos.column,
        });
        editorInstance.revealPositionInCenter(pos, monacoEditor.ScrollType.Smooth);
        editorInstance.focus();
      }, [jumpTo, editorInstance]);

    useEffect(() => {
        setSyn(backendSyntax);
    }, [backendSyntax]);

    useEffect(() => {
        if (monaco && backendSyntax) {
            const cloneSyntax = Object.assign({}, backendSyntax);
            const existingLang = monaco.languages.getLanguages()
                // @ts-ignore
                .find(lang => lang.id === 'arkts');
            if (!existingLang) {
                monaco.languages.register({ id: 'arkts' });
            }
            // @ts-ignore
            monaco.languages.setMonarchTokensProvider('arkts', cloneSyntax);
            monaco.languages.setLanguageConfiguration('arkts', {
                comments: {
                    lineComment: '//',
                    blockComment: ['/*', '*/'],
                },
                brackets: [
                    ['{', '}'],
                    ['[', ']'],
                    ['(', ')'],
                ],
                autoClosingPairs: [
                    { open: '{', close: '}' },
                    { open: '[', close: ']' },
                    { open: '(', close: ')' },
                    { open: '"', close: '"' },
                    { open: "'", close: "'" },
                ],
                surroundingPairs: [
                    { open: '{', close: '}' },
                    { open: '[', close: ']' },
                    { open: '(', close: ')' },
                    { open: '"', close: '"' },
                    { open: "'", close: "'" },
                ],
            });
        }
    }, [monaco, syn]);

    const hasCancel = (fn: unknown): fn is { cancel: () => void } =>
        !!fn && typeof (fn as { cancel?: unknown }).cancel === 'function';

    const hasFlush = (fn: unknown): fn is { flush: () => void } =>
        !!fn && typeof (fn as { flush?: unknown }).flush === 'function';

    const debouncedDispatchRef = useRef<ReturnType<typeof debounce>>(
        debounce((value: string) => {
          dispatch(setCodeAction(value));
        }, 800)
    );

    useEffect(() => {
        const handleFlush = (): void => {
            const d = debouncedDispatchRef.current;
            if (hasFlush(d)) {
                d.flush();
            }
        };

        window.addEventListener('flushPendingCodeUpdate', handleFlush);

        return () => {
            window.removeEventListener('flushPendingCodeUpdate', handleFlush);
            const d = debouncedDispatchRef.current;
            if (hasCancel(d)) {
                d.cancel();
            };
        }
    }, []);

    const handleEditorChange = (value?: string): void => {
        debouncedDispatchRef.current(value ?? '');
    };

    useEffect(() => {
        if (!editorInstance || !codeFromStore) {
            return;
        }

        const currentEditorValue = editorInstance.getValue();

        if (currentEditorValue !== codeFromStore) {
            const position = editorInstance.getPosition();
            editorInstance.setValue(codeFromStore);

            if (position) {
                editorInstance.setPosition(position);
            }
        }
    }, [codeFromStore, editorInstance]);

    useEffect(() => {
        if (!monaco || !editorInstance) {
            return;
        }
        const model = editorInstance.getModel();
        if (!model) {
            return;
        }
        const markers: monaco.editor.IMarkerData[] = highlightErrors.filter(el => !!el.line).map(err => ({
            startLineNumber: err.line,
            startColumn: err.column,
            endLineNumber: err.line,
            endColumn: err.column + 1,
            message: err.message,
            severity: err.type === 'Warning'
             ? monaco.MarkerSeverity.Warning
             : monaco.MarkerSeverity.Error
        })
    );

        monacoEditor.setModelMarkers(model, 'arkts', markers);
    }, [highlightErrors, monaco, editorInstance]);

    return (
        <div className={styles.editorContent}>
            <Editor
                onMount={(editor): void => {
                    setEditorInstance(editor);
                }}
                defaultLanguage="arkts"
                defaultValue={'console.log("Hello, ArkTS!");'}
                theme={theme === 'dark' ? 'vs-dark' : 'light'}
                onChange={handleEditorChange}
                className={styles.editor}
                options={{
                    fixedOverflowWidgets: true,
                    automaticLayout: true
                 }}
                height="100%"
            />
            <div className={styles.statusBar}>
                Ln {cursorPos.lineNumber}, Col {cursorPos.column}
            </div>
        </div>
    );
};

export default ArkTSEditor;
