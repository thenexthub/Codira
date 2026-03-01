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
import {ILog} from '../../models/logs';
import styles from './styles.module.scss';
import {Icon, Tooltip, Checkbox} from '@blueprintjs/core';
import cx from 'classnames';
import { AppDispatch } from '../../store';
import { useDispatch, useSelector } from 'react-redux';
import {
    setOutLogs,
    setErrLogs,
    setJumpTo
} from '../../store/slices/logs';
import {
    selectOutLogs,
    selectErrLogs
} from '../../store/selectors/logs';


interface IProps {
    logArr: ILog[]
    clearFilters: () => void
    logType: 'out' | 'err' | 'compilation' | 'runtime';
}

const LogsView = ({logArr, clearFilters, logType}: IProps): JSX.Element => {
    const [logs, setLogs] = useState<ILog[]>(logArr);
    const [showOut, setShowOut] = useState<boolean>(true);
    const [showErr, setShowErr] = useState<boolean>(true);
    const [filterText, setFilterText] = useState<string>('');
    const dispatch = useDispatch<AppDispatch>();
    const containerRef = useRef<HTMLDivElement | null>(null);

    const outLogs = useSelector(selectOutLogs);
    const errLogs = useSelector(selectErrLogs);

    useEffect(() => {
        let filtered = logArr;

        // Filter by type (out/err)
        filtered = filtered.filter(log => {
            const isErr = log.from?.includes('Err');
            if (!showOut && !isErr) {
                return false;
            }
            if (!showErr && isErr) {
                return false;
            }
            return true;
        });

        // Filter by text
        if (filterText) {
            filtered = filtered.filter(el =>
                el.message.some(m => m.message.includes(filterText))
            );
        }

        setLogs(filtered);
    }, [logArr, showOut, showErr, filterText]);

    useEffect(() => {
        if (!containerRef.current) {
            return undefined;
        }
        const observer = new IntersectionObserver(
            (entries) => {
                entries.forEach((entry) => {
                    if (entry.isIntersecting) {
                        const logIndex = Number(entry.target.getAttribute('data-index'));

                        if (logIndex !== undefined && !logs[logIndex].isRead) {
                            const currentLog = logs[logIndex];
                            const updatedLogs = [...logs];
                            updatedLogs[logIndex] = { ...updatedLogs[logIndex], isRead: true };
                            setLogs(updatedLogs);

                            const isOutputLog = currentLog.message.some(m => m.source === 'output');

                            if (isOutputLog) {
                                const updatedOutLogs = outLogs.map(log =>
                                    log === currentLog ? { ...log, isRead: true } : log
                                );
                                dispatch(setOutLogs(updatedOutLogs));
                            } else {
                                const updatedErrLogs = errLogs.map(log =>
                                    log === currentLog ? { ...log, isRead: true } : log
                                );
                                dispatch(setErrLogs(updatedErrLogs));
                            }
                        }
                    }
                });
            },
            { threshold: 0.1 }
        );

        containerRef.current.querySelectorAll('[data-index]').forEach((el): void => observer.observe(el));

        return () => {
            observer.disconnect();
        };
    }, [logs, dispatch, outLogs, errLogs]);

    return (
        <div className={styles.container}>
            <div className={styles.containerLogs} ref={containerRef}>
                {logs.map((log: ILog, index) => (
                    <div key={index} data-index={index} className={styles.logRow}>
                        <span
                            className={cx({
                                [styles.tag]: true,
                                [styles.red]: log.from?.includes('Err')})}
                        >
                            [{log.from?.includes('Err') ? 'err' : 'out'}]
                        </span>
                        {log.from?.includes('Aot') && (
                            <span className={cx(styles.tag, styles.modeTag)}>[AOT]</span>
                        )}
                        {(log.from === 'runOut' || log.from === 'runErr') && (
                            <span className={cx(styles.tag, styles.modeTag)}>[JIT]</span>
                        )}
                        <div
                            className={cx({
                                [styles.tag]: true,
                                [styles.orange]: log.exit_code !== 0})}
                        >
                            <Tooltip content='exit_code' placement='bottom'><span>[{log.exit_code}]:</span></Tooltip>
                        </div>
                        <div className={styles.messageContainer}>
                            {log.message.map((el, ind) => (
                                <pre
                                    key={`${el.message}-${ind}`}
                                    className={styles.rowContainer}
                                >
                                    <span
                                        className={styles.logText}
                                        onClick={(): void => {
                                            if (el?.line && el?.column) {
                                                dispatch(setJumpTo({line: el.line, column: el.column, nonce: Date.now()}));
                                            }
                                        }}
                                    >{el.message}</span>
                                </pre>
                            ))}
                        </div>
                    </div>
                ))}
            </div>
            <div className={styles.filterContainer}>
                <Icon
                    icon={'disable'}
                    className={styles.clearIc}
                    onClick={clearFilters}
                    data-testid="clear-icon"
                />
                <Checkbox
                    checked={showOut}
                    label="Out"
                    onChange={(e): void => setShowOut(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showErr}
                    label="Err"
                    onChange={(e): void => setShowErr(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <input
                    placeholder="Filter"
                    className={styles.input}
                    onChange={(e): void => setFilterText(e.target.value)}
                    value={filterText}
                />
            </div>
        </div>);
};

export default LogsView;
