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
import styles from './styles.module.scss';
import {Button, Radio, RadioGroup} from '@blueprintjs/core';
import {useDispatch, useSelector} from 'react-redux';
import {selectOptions, selectPickedOptions} from '../../store/selectors/options';
import {IOption, TObj} from '../../models/options';
import {AppDispatch} from '../../store';
import {pickOptions} from '../../store/actions/options';

type TProps = {
    onClose: () => void
};

const CompileOptions = ({onClose}: TProps): JSX.Element => {
    const availableOptions = useSelector(selectOptions) || [];
    const pickedOptions = useSelector(selectPickedOptions);
    const [selected, setSelected] = React.useState<IOption[]>([]);
    const dispatch = useDispatch<AppDispatch>();

    useEffect(() => {
        const tmp: IOption[] = availableOptions.map((el: IOption) => {
            if (pickedOptions && pickedOptions[el.flag]) {
                return {
                    ...el,
                    isSelected: pickedOptions[el.flag],
                };
            }
            return el;
        });
        setSelected(tmp);
    }, [availableOptions, pickedOptions]);

    const handleChange = (item: IOption, val: string | number): void => {
        const tmpData: IOption[] = selected.map((el) => {
            if (el.flag === item.flag) {
                return {
                    ...el,
                    isSelected: val,
                };
            }
            return el;
        });
        setSelected(tmpData);
    };

    const handleSave = (): void => {
        const picked: TObj = {};
        selected.forEach((el) => {
            if (el.isSelected?.toString()) {
                picked[el.flag] = el.isSelected;
            }
        });
        dispatch(pickOptions(picked));
        onClose();
    };

    const handleReset = (): void => {
        setSelected(availableOptions);
        dispatch(pickOptions({}));
        onClose();
    };

    return (
        <div className={styles.container}>
            <span className={styles.header}>Compile options</span>
            <div className={styles.optionsContainer}>
                {availableOptions?.map((item: IOption, index) => (
                    <RadioGroup
                        key={index}
                        inline={true}
                        label={<span className={styles.label}>{item.flag}</span>}
                        name="group"
                        onChange={(e): void => handleChange(item, e.currentTarget?.value)}
                        selectedValue={selected.find((el) => el.flag === item.flag &&
                            el.isSelected)?.isSelected?.toString()}
                    >
                        {item.values?.map((el, ind) => (
                            <Radio
                                label={el.toString()}
                                value={el.toString()}
                                key={ind}
                                className={styles.checkbox}
                                data-testid={`radio-${el}`}
                            />
                        ))}
                    </RadioGroup>
                ))}
            </div>
            <div className={styles.btnContainer}>
                <Button className={styles.btn} onClick={handleReset}>
                    Reset
                </Button>
                <Button className={styles.btn} onClick={handleSave}>
                    Save
                </Button>
            </div>
        </div>
    );
};

export default CompileOptions;
