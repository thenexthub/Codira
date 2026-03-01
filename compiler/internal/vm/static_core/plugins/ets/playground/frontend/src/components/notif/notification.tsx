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

import { useDispatch, useSelector } from 'react-redux';
import { selectMessages } from '../../store/selectors/notifications';
import cx from 'classnames';
import styles from './styles.module.scss';
import { TNotificationType } from '../../models/notifications';
import { AppDispatch } from '../../store';
import { removeMessage } from '../../store/slices/notifications';
import { ErrorIconSvg, InfoIconSvg, SuccessIconSvg } from './icons';

const getIcon = (type: TNotificationType): JSX.Element => {
    switch(type) {
        case 'success':
            return (
                <SuccessIconSvg className={styles.notifItem__icon} />
            );
        case 'info':
            return (
                <InfoIconSvg className={styles.notifItem__icon} />
            );
        case 'error':
            return (
                <ErrorIconSvg className={styles.notifItem__icon} />
            );
        default:
            console.error('ERROR: unknown notification type - ', type);
            return <></>;
    }
};

export const Notifications = (): JSX.Element => {
    const messages = useSelector(selectMessages);
    const dispatch = useDispatch<AppDispatch>();

    const handleClose = (event: React.MouseEvent<HTMLButtonElement>): void => {
        const target = event.currentTarget as HTMLButtonElement;
        const { id } = target.dataset;
        if (!id) {
            console.error('ERROR: no data-id found on notification while trying to close it manually');
            return;
        }
        const idParsed = Number(id);
        if (isNaN(idParsed)) {
            console.error('ERROR: failed to parse data-id on notification while trying to close it manually, expected number');
            return;
        }
        dispatch(removeMessage(idParsed));
    };

    return (
        <div className={styles.notifContainer}>
            {messages.map((item) => {
                return (
                    <div
                        key={item.id}
                        className={cx({
                            [styles.notifItem]: true,
                            [styles.notifItem_success]: item.type === 'success',
                            [styles.notifItem_info]: item.type === 'info',
                            [styles.notifItem_error]: item.type === 'error',
                        })}>
                        {getIcon(item.type)}
                        <p className={styles.notifItem__title}>{item.title}</p>
                        <p className={styles.notifItem__message}>{item.message}</p>
                        <button
                            className={styles.notifItem__close}
                            data-id={item.id}
                            onClick={handleClose}>
                            <svg width="16" height="16" viewBox="0 0 16 16" fill="none" xmlns="http://www.w3.org/2000/svg">
                                <path d="M4 4L12 12" stroke="#63554D" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
                                <path d="M4 12L12 4.00002" stroke="#63554D" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
                            </svg>
                        </button>
                    </div>
                );
            })}
        </div>
    );
};
