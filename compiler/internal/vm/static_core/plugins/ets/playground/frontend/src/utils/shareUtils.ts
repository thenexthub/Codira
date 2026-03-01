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


import { IObj } from '../store/slices/options';

export interface IShareData {
    code: string;
    options: IObj | null;
}

/**
 * Encodes share data (code + options) into a base64 URL-safe string
 * @param code - Source code to share
 * @param options - Compiler options to share
 * @returns Base64-encoded string safe for URLs
 * @throws Error if encoding fails (e.g., circular structure in options)
 */
export const encodeShareData = (code: string, options: IObj | null): string => {
    const data: IShareData = { code, options };
    const jsonString = JSON.stringify(data);
    const encodedJson = encodeURIComponent(jsonString);
    return btoa(encodedJson);
};

/**
 * Decodes a base64-encoded share data string back to original data
 * @param encoded - Base64-encoded string from encodeShareData
 * @returns Decoded share data or null if decoding/validation fails
 */
export const decodeShareData = (encoded: string): IShareData | null => {
    try {
        const encodedJson = atob(encoded);
        const jsonString = decodeURIComponent(encodedJson);
        const data: IShareData = JSON.parse(jsonString);

        if (!data || typeof data !== 'object') {
            throw new Error('Invalid share data: must be an object');
        }
        if (typeof data.code !== 'string') {
            throw new Error('Invalid share data: code must be a string');
        }
        if (data.options !== null && (typeof data.options !== 'object' || Array.isArray(data.options))) {
            throw new Error('Invalid share data: options must be an object or null');
        }

        return data;
    } catch (error) {
        console.error('Failed to decode share data:', error instanceof Error ? error.message : String(error));
        return null;
    }
};

/**
 * Copies text to clipboard using the Clipboard API
 * @param text - Text to copy to clipboard
 * @returns true if successful, false if clipboard API unavailable or permission denied
 */
export const copyToClipboard = async (text: string): Promise<boolean> => {
    try {
        await navigator.clipboard.writeText(text);
        return true;
    } catch (error) {
        console.error('Failed to copy to clipboard:', error instanceof Error ? error.message : String(error));
        return false;
    }
};

/**
 * Builds a shareable URL with encoded data in the hash
 * @param encoded - Base64-encoded share data from encodeShareData (must be safe base64 string)
 * @param loc - Location object (defaults to window.location)
 * @returns Complete shareable URL
 * @warning Only pass encoded strings from encodeShareData to prevent XSS
 */
export const buildShareUrl = (encoded: string, loc: Location = window.location): string => {
    const url = new URL(loc.href);
    url.search = '';
    const hashParams = new URLSearchParams(url.hash.startsWith('#') ? url.hash.slice(1) : url.hash);
    hashParams.set('share', encoded);
    url.hash = hashParams.toString();
    return url.toString();
};

/**
 * Extracts share data from URL hash or query parameters
 * @param loc - Location object (defaults to window.location)
 * @returns Encoded share string or null if not found
 */
export const getShareFromLocation = (loc: Location = window.location): string | null => {
    const hashParams = new URLSearchParams(loc.hash.startsWith('#') ? loc.hash.slice(1) : loc.hash);
    const fromHash = hashParams.get('share');
    if (fromHash) {
        return fromHash;
    }
    return new URLSearchParams(loc.search).get('share');
};
