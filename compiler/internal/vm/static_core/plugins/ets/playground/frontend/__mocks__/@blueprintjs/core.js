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

import React from 'react';

export const Icon = (props) => <span data-testid="mocked-icon" {...props}>Mocked Icon</span>;

export const Button = ({ disabled, ...props }) => (
    <button disabled={disabled} {...props} />
);
export const Checkbox = (props) => <input type="checkbox" {...props} />;
export const Popover = ({ content, children, popoverClassName, interactionKind, isOpen, popoverRef, ...props }) => (
    <div className={popoverClassName} ref={popoverRef} {...props}>
        {children}
        {isOpen && <div data-testid="compile-options-content">{content}</div>}
    </div>
);
export const Ratio = ({ children, ...props }) => <div data-testid="mocked-ratio" {...props}>{children}</div>;
export const Switch = ({ checked, onChange, ...props }) => (
    <label data-testid="mocked-switch">
        <input
            type="checkbox"
            checked={checked}
            onChange={onChange}
            {...props}
        />
        {checked ? 'On' : 'Off'}
    </label>
);
export const Menu = ({ children, ...props }) => (
    <div data-testid="mocked-menu" {...props}>{children}</div>
);

export const MenuItem = ({ icon, text, onClick, labelElement, ...props }) => (
    <div data-testid="mocked-menu-item" onClick={onClick} {...props}>
        {icon && <Icon icon={icon} />}
        {text}
        {labelElement}
    </div>
);

export const RadioGroup = ({ label, name, selectedValue, onChange, children, inline, ...props }) => (
    <div data-testid="mocked-radio-group" role="radiogroup" {...props}>
        {React.Children.map(children, (child) => {
            return React.cloneElement(child, {
                name,
                checked: child.props.value === selectedValue,
                onChange,
            });
        })}
        {selectedValue}
        {label}
    </div>
);

export const Radio = ({ label, value, checked, onChange, ...props }) => (
    <label data-testid="mocked-radio">
        <input
            type="radio"
            value={value}
            checked={checked}
            onChange={onChange}
            {...props}
        />
        {label}
    </label>
);

export const Tooltip = ({ content, children, tooltipClassName, isOpen, tooltipRef, ...props }) => (
    <div className={tooltipClassName} ref={tooltipRef} {...props}>
        {children}
        {isOpen && <div data-testid="mocked-tooltip-content">{content}</div>}
    </div>
);

export const ButtonGroup = ({ children, groupClassName, ...props }) => (
    <div className={groupClassName} data-testid="mocked-button-group" {...props}>
        {children}
    </div>
);