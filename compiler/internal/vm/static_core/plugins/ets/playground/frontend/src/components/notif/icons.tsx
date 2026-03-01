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

export interface ISvgProps {
    className?: string
    onClick?: (event: React.MouseEvent<SVGElement>) => void
    width?: number
    height?: number
}

export const SuccessIconSvg = ({
    className
}: ISvgProps): JSX.Element => {
    return (
        <svg className={className} width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2C6.5 2 2 6.5 2 12C2 17.5 6.5 22 12 22C17.5 22 22 17.5 22 12C22 6.5 17.5 2 12 2Z" fill="#5B8E69"/>
            <path d="M16.5999 10.5999L11.7999 15.3999C11.3999 15.7999 10.7999 15.7999 10.3999 15.3999L8.1999 13.1999C7.7999 12.7999 7.7999 12.1999 8.1999 11.7999C8.5999 11.3999 9.1999 11.3999 9.5999 11.7999L11.0999 13.2999L15.1999 9.1999C15.5999 8.7999 16.1999 8.7999 16.5999 9.1999C16.9999 9.5999 16.9999 10.1999 16.5999 10.5999Z" fill="#F0FBF3"/>
        </svg>
    );
};

export const InfoIconSvg = ({
    className
}: ISvgProps): JSX.Element => {
    return (
        <svg className={className} width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2C6.5 2 2 6.5 2 12C2 17.5 6.5 22 12 22C17.5 22 22 17.5 22 12C22 6.5 17.5 2 12 2Z" fill="#8D7D72"/>
            <path d="M13 12C13 11.4 12.6 11 12 11C11.4 11 11 11.4 11 12V16C11 16.6 11.4 17 12 17C12.6 17 13 16.6 13 16V12Z" fill="#FFF2F2"/>
            <path d="M12 7C11.4 7 11 7.4 11 8C11 8.6 11.4 9 12 9C12.6 9 13 8.6 13 8C13 7.4 12.6 7 12 7Z" fill="#FFF2F2"/>
        </svg>
    );
};

export const ErrorIconSvg = ({
    className
}: ISvgProps): JSX.Element => {
    return (
        <svg className={className} width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M21.7045 17.0054L14.377 4.34048C13.6533 3.074 11.9345 2.62168 10.668 3.34539C10.2157 3.61678 9.85385 3.97863 9.67292 4.34048L2.34539 17.0054C1.62168 18.2718 2.074 19.9906 3.34048 20.7144C3.7928 20.9857 4.24512 21.0762 4.69744 21.0762H19.262C20.7999 21.0762 21.9759 19.8097 21.9759 18.3623C22.0664 17.8195 21.8855 17.3672 21.7045 17.0054Z" fill="#FB5D5D"/>
            <path d="M13 13C13 13.6 12.6 14 12 14C11.4 14 11 13.6 11 13V9C11 8.4 11.4 8 12 8C12.6 8 13 8.4 13 9V13Z" fill="#FFF2F2"/>
            <path d="M12 18C11.4 18 11 17.6 11 17C11 16.4 11.4 16 12 16C12.6 16 13 16.4 13 17C13 17.6 12.6 18 12 18Z" fill="#FFF2F2"/>
        </svg>
    );
};