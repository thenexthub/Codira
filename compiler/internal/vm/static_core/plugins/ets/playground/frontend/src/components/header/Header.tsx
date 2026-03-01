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

import React, { useEffect, useRef, useState } from 'react';
import {Icon, Menu, MenuItem, Popover, Switch} from '@blueprintjs/core';
import {useTheme} from '../theme/ThemeContext';
import cx from 'classnames';
import styles from './styles.module.scss';
import { useClickOutside } from '../../utils/useClickOutside';
import { useDispatch, useSelector } from 'react-redux';
import { AppDispatch } from '../../store';
import { fetchVersions, setClearLogsAction } from '../../store/actions/appState';
import { clearLogsEachRun, versions } from '../../store/selectors/appState';
import { Versions } from '../../store/slices/appState';

export const primaryColors = ['#e32b49', '#da70d6', '#0F6894', '#1C6E42', '#935610', '#634DBF', '#00A396', '#D1980B'];

const exampleMenu = (
    theme: string,
    toggleTheme: () => void,
    togglePrimaryColor: (val: string) => void,
    versionsData: Versions,
    autoClearLogs: boolean,
    handleClearLogs: (next: boolean) => void,
): JSX.Element => (
    <Menu className={styles.menu}>
        <MenuItem
            icon="moon"
            text="Dark mode"
            className={styles.menuItem}
            labelElement={
                <Switch
                    onChange={(e: React.ChangeEvent<HTMLInputElement>): void => {
                        e.stopPropagation();
                    }}
                    onClick={(e: React.MouseEvent<HTMLInputElement, MouseEvent>): void => {
                        e.stopPropagation();
                        e.preventDefault();
                    }}
                    checked={theme === 'dark'}
                    className={styles.switch}
                />
            }
            onClick={toggleTheme}
        />
        <div className={styles.colorContainer}>
            {primaryColors.map(color => (
                <div
                    data-testid={color}
                    className={styles.color}
                    style={{backgroundColor: color}}
                    key={color}
                    onClick={(): void => togglePrimaryColor(color)}
                />))}
        </div>
        <div className={styles.hr} />
        <MenuItem
            icon="disable"
            text="Auto clear logs"
            className={styles.menuItem}
            labelElement={
                <Switch
                    onChange={(e: React.ChangeEvent<HTMLInputElement>): void => {
                        e.stopPropagation();
                    }}
                    onClick={(e: React.MouseEvent<HTMLInputElement, MouseEvent>): void => {
                        e.stopPropagation();
                        e.preventDefault();
                    }}
                    checked={!!autoClearLogs}
                    className={styles.switch}
                />
            }
            onClick={(): void => handleClearLogs(!autoClearLogs)}
        />
        <div className={styles.hr} />
        <div className={styles.versionsContainer}>
            <pre className={styles.preTag}>
                <span className={styles.versionTitle}>frontend:</span>
                {versionsData?.frontend}
            </pre>
            <pre className={styles.preTag}>
                <span className={styles.versionTitle}>backend:</span>
                {versionsData?.backendVersion}
            </pre>
            <pre className={styles.preTag}>
                <span className={styles.versionTitle}>ArkTS:</span>
                {versionsData?.arktsVersion}
            </pre>
            <pre className={styles.preTag}>
                <span className={styles.versionTitle}>es2panda:</span>
                {versionsData?.es2pandaVersion}
            </pre>
        </div>
    </Menu>
);

const Header = (): JSX.Element => {
    const popoverRef = useRef<HTMLDivElement | null>(null);
    const { theme, toggleTheme, togglePrimaryColor } = useTheme();
    const dispatch = useDispatch<AppDispatch>();
    const versionsData = useSelector(versions);
    const autoClearLogs = useSelector(clearLogsEachRun);

    useEffect(() => {
        dispatch(fetchVersions());
    }, []);

    const [isOpen, setIsOpen] = useState<boolean>(false);

    const handleClosePopover = (): void => {
        setIsOpen(false);
    };
    const handleOpenPopover = (): void => {
        setIsOpen(true);
    };
    const handleClearLogs = (next: boolean): void => {
        dispatch(setClearLogsAction(next));
    };

    useClickOutside(popoverRef, handleClosePopover);

    return (
        <div
            className={cx({
                [styles.dark]: theme === 'dark',
                [styles.container]: true,
            })}
            data-testid="header-component"
        >
            <span className={cx({[styles.title]: true})}>ArkTS playground</span>
            <div className={styles.linksContainer}>
                <a
                    href="https://gitcode.com/igelhaus/arkcompiler_runtime_core/releases"
                    target="_blank"
                    rel="noopener noreferrer"
                    className={styles.link}
                    data-testid="docs-link"
                >
                    Docs
                </a>
            </div>
            <Popover
                content={exampleMenu(
                    theme,
                    toggleTheme,
                    togglePrimaryColor,
                    versionsData,
                    autoClearLogs,
                    handleClearLogs,
                )}
                placement="bottom"
                className={styles.settings}
                popoverClassName={styles.pop}
                isOpen={isOpen}
                popoverRef={popoverRef}
            >
                <div
                    data-testid="settings-icon"
                    onClick={handleOpenPopover}
                >
                    <Icon icon={'cog'} />
                </div>
            </Popover>
        </div>);
};

export default Header;
