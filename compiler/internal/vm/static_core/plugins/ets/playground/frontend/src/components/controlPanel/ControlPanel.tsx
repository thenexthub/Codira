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

import React, {useEffect, useRef, useState} from 'react';
import {Button, ButtonGroup, Checkbox, Icon, Popover, Tooltip, Radio, RadioGroup} from '@blueprintjs/core';
import styles from './styles.module.scss';
import CompileOptions from '../../pages/compileOptions/CompileOptions';
import {useDispatch, useSelector} from 'react-redux';
import {withAstView, withDisasm, selectVerificationMode, withVerifier, withIrDump, withAotMode} from '../../store/selectors/appState';
import {AppDispatch} from '../../store';
import {setDisasmAction, setVerificationModeAction, setVerifAction, setIrDumpCompilerDumpAction, setIrDumpDisasmDumpAction, setAotModeAction} from '../../store/actions/appState';
import {fetchCompileCode, fetchRunCode, shareCodeLocally, flushPendingCodeUpdate} from '../../store/actions/code';
import {selectCompileLoading, selectRunLoading, selectShareLoading} from '../../store/selectors/code';
import {useClickOutside} from '../../utils/useClickOutside';
import cx from 'classnames';
import { setAstView, setActiveLogTab } from '../../store/slices/appState';
import { VerificationMode } from '../../models/code';

const ControlPanel = (): JSX.Element => {
    const popoverRef = useRef<HTMLDivElement | null>(null);
    const [isOpen, setIsOpen] = useState<boolean>(false);
    const disasm = useSelector(withDisasm);
    const astView = useSelector(withAstView);
    const verificationMode = useSelector(selectVerificationMode);
    const verifier = useSelector(withVerifier);
    const irDump = useSelector(withIrDump);
    const aotMode = useSelector(withAotMode);
    const dispatch = useDispatch<AppDispatch>();
    const isCompileLoading = useSelector(selectCompileLoading);
    const isShareLoading = useSelector(selectShareLoading);
    const isRunLoading = useSelector(selectRunLoading);
    const [localVerificationMode, setLocalVerificationMode] = useState<VerificationMode>('disabled');
    const [runAotMode, setRunAotMode] = useState(false);
    const [irDumpCompilerDump, setIrDumpCompilerDumpLocal] = useState(false);
    const [irDumpDisasmDump, setIrDumpDisasmDumpLocal] = useState(false);

    useEffect(() => {
        setLocalVerificationMode(verificationMode);
    }, [verificationMode]);

    useEffect(() => {
        setRunAotMode(aotMode);
    }, [aotMode]);

    useEffect(() => {
        setIrDumpCompilerDumpLocal(irDump.compilerDump);
        setIrDumpDisasmDumpLocal(irDump.disasmDump);
    }, [irDump]);


    const handleDisasmChange = (): void => {
        dispatch(setDisasmAction(!disasm));
    };
    const handleVerifChange = (): void => {
        dispatch(setVerifAction(!verifier));
    };
    const handleAstViewChange = (): void => {
        dispatch(setAstView(!astView));
    };
    const handleCompile = (): void => {
        flushPendingCodeUpdate();
        dispatch(setActiveLogTab('compilation'));
        dispatch(fetchCompileCode());
    };
    const handleRun = (): void => {
        flushPendingCodeUpdate();
        dispatch(setActiveLogTab('runtime'));
        dispatch(fetchRunCode());
    };
    const handleShare = (): void => {
        flushPendingCodeUpdate();
        dispatch(shareCodeLocally());
    };
    const handleClosePopover = (): void => {
        setIsOpen(false);
        setLocalVerificationMode(verificationMode);
        setRunAotMode(aotMode || false);
        setIrDumpCompilerDumpLocal(irDump.compilerDump);
        setIrDumpDisasmDumpLocal(irDump.disasmDump);
    };
    const handleOpenPopover = (): void => {
        setIsOpen(true);
    };
    const handleVerificationModeReset = (): void => {
        dispatch(setVerificationModeAction('disabled'));
        dispatch(setAotModeAction(false));
        dispatch(setIrDumpCompilerDumpAction(false));
        dispatch(setIrDumpDisasmDumpAction(false));
        setIsOpen(false);
    };
    const handleVerificationModeChange = (): void => {
        dispatch(setVerificationModeAction(localVerificationMode));
        dispatch(setAotModeAction(runAotMode));
        dispatch(setIrDumpCompilerDumpAction(irDumpCompilerDump));
        dispatch(setIrDumpDisasmDumpAction(irDumpDisasmDump));
        handleClosePopover();
    };
    useClickOutside(popoverRef, handleClosePopover);

    return (
        <div className={styles.container}>
            <ButtonGroup>
                <Tooltip content="Run" placement="bottom">
                    <Button
                        icon={isRunLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11}/></div>
                            : <div className={styles.icon}><Icon icon="play" size={20}/></div>}
                        className={styles.btn}
                        onClick={handleRun}
                        disabled={isRunLoading || isCompileLoading}
                        data-testid="run-btn"
                    />
                </Tooltip>
                <Tooltip content="Compile" placement="bottom">
                    <Button
                        icon={isCompileLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11} /></div>
                            : <div className={styles.icon}><Icon icon="code-block" size={14}/></div>}
                        className={styles.btn}
                        onClick={handleCompile}
                        disabled={isCompileLoading || isRunLoading}
                        data-testid="compile-btn"
                    />
                </Tooltip>
                <Tooltip content="Options" placement="bottom">
                    <Popover
                        content={<div className={styles.options}>
                            <CompileOptions onClose={handleClosePopover} />
                            <span className={styles.header}>Run options</span>
                            <RadioGroup
                                label="Verification mode:"
                                inline={true}
                                onChange={(e): void => setLocalVerificationMode(e.currentTarget.value as VerificationMode)}
                                selectedValue={localVerificationMode}
                                className={styles.radioGroup}
                            >
                                <Radio label="Disabled" value="disabled" />
                                <Radio label="Ahead of Time" value="ahead-of-time" />
                                <Radio label="On the Fly" value="on-the-fly" />
                            </RadioGroup>
                            <Checkbox
                                checked={runAotMode}
                                label="AOT mode"
                                onChange={(e): void => setRunAotMode(e.target.checked)}
                                className={styles.disasm}
                                data-testid="aotMode-checkbox"
                            />
                            <span className={styles.sectionHeader}>IR Dump options</span>
                            <Checkbox
                                checked={irDumpCompilerDump}
                                label="Compiler Dump"
                                onChange={(e): void => setIrDumpCompilerDumpLocal(e.target.checked)}
                                className={styles.disasm}
                                data-testid="irDumpCompiler-checkbox"
                            />
                            <Checkbox
                                checked={irDumpDisasmDump}
                                label="Disasm Dump"
                                onChange={(e): void => setIrDumpDisasmDumpLocal(e.target.checked)}
                                className={styles.disasm}
                                data-testid="irDumpDisasm-checkbox"
                            />
                            <div className={styles.btnContainer}>
                                <Button className={cx(styles.btn, styles.btnBorder)} onClick={handleVerificationModeReset}>
                                    Reset
                                </Button>
                                <Button className={cx(styles.btn, styles.btnBorder)} onClick={handleVerificationModeChange}>
                                    Save
                                </Button>
                            </div>
                        </div>}
                        interactionKind="click"
                        popoverClassName={styles.pop}
                        isOpen={isOpen}
                        popoverRef={popoverRef}
                        position='bottom'
                    >
                        <Button
                            icon={<div className={styles.icon}><Icon icon="more" size={14} /></div>}
                            className={styles.btn}
                            onClick={handleOpenPopover}
                            data-testid='options'
                        />
                    </Popover>
                </Tooltip>
                <Tooltip content="Share" placement="bottom">
                    <Button
                        icon={isShareLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11} /></div>
                            : <div className={styles.icon}><Icon icon="share" size={14}/></div>}
                        className={styles.btn}
                        onClick={handleShare}
                        disabled={isShareLoading}
                        data-testid="share-btn"
                    />
                </Tooltip>
            </ButtonGroup>
            <Checkbox
                checked={astView}
                label="AST View"
                onChange={handleAstViewChange}
                className={styles.disasm}
                data-testid="ast-checkbox"
            />
            <Checkbox
                checked={disasm}
                label="Disasm"
                onChange={handleDisasmChange}
                className={styles.disasm}
                data-testid="disasm-checkbox"
            />
            <Checkbox
                checked={verifier}
                label="Verifier"
                onChange={handleVerifChange}
                className={styles.disasm}
                data-testid="verifier-checkbox"
            />
        </div>
    );
};

export default ControlPanel;
