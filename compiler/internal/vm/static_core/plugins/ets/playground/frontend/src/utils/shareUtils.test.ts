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

import { encodeShareData, decodeShareData, copyToClipboard, buildShareUrl, getShareFromLocation } from './shareUtils';
import type { IObj } from '../store/slices/options';

interface GlobalWithBase64 {
  btoa?: (str: string) => string;
  atob?: (b64: string) => string;
}

describe('share utils', () => {
  const origClipboard = (globalThis.navigator as unknown as { clipboard?: Clipboard }).clipboard;

  beforeAll(() => {
    const global = globalThis as unknown as GlobalWithBase64;
    if (!global.btoa) {
      global.btoa = (str: string): string => Buffer.from(str, 'binary').toString('base64');
    }
    if (!global.atob) {
      global.atob = (b64: string): string => Buffer.from(b64, 'base64').toString('binary');
    }
  });

  afterEach(() => {
    Object.defineProperty(navigator, 'clipboard', {
      configurable: true,
      value: origClipboard,
      writable: true,
    });

    jest.restoreAllMocks();
    jest.clearAllMocks();
  });

  describe('encodeShareData / decodeShareData roundtrip', () => {
    it('encodes and decodes back the same payload (including unicode)', () => {
      const code = 'print("Привет 👋")';
      const options: IObj = { flag: true, count: 1, name: 'demo' };

      const encoded = encodeShareData(code, options);
      expect(typeof encoded).toBe('string');
      expect(encoded.length).toBeGreaterThan(0);

      const decoded = decodeShareData(encoded);
      expect(decoded).toEqual({ code, options });
    });

    it('throws an error when JSON.stringify fails (circular)', () => {
      const circular: Record<string, unknown> & { self?: unknown } = {};
      circular.self = circular;
      expect(() => encodeShareData('code', circular as unknown as IObj)).toThrow();
    });
  });

  describe('decodeShareData', () => {
    it('returns null for invalid base64 input', () => {
      expect(decodeShareData('@@@not_base64@@@')).toBeNull();
    });

    it('returns null when decoded object has non-string code', () => {
      const bad = { code: 123 as unknown as string, options: null };
      const global = globalThis as unknown as GlobalWithBase64;
      const encoded = global.btoa!(encodeURIComponent(JSON.stringify(bad)));
      expect(decodeShareData(encoded)).toBeNull();
    });
  });

  describe('copyToClipboard', () => {
    it('uses navigator.clipboard.writeText when available', async () => {
      const writeText = jest.fn().mockResolvedValue(undefined);
      Object.defineProperty(navigator, 'clipboard', {
        configurable: true,
        value: { writeText },
        writable: true,
      });

      const ok = await copyToClipboard('hello');
      expect(ok).toBe(true);
      expect(writeText).toHaveBeenCalledTimes(1);
      expect(writeText).toHaveBeenCalledWith('hello');
    });

    it('returns false when clipboard API is unavailable', async () => {
      Object.defineProperty(navigator, 'clipboard', {
        configurable: true,
        value: undefined,
        writable: true,
      });

      const ok = await copyToClipboard('text');
      expect(ok).toBe(false);
    });

    it('returns false when clipboard API throws permission error', async () => {
      const writeText = jest.fn().mockRejectedValue(new Error('Permission denied'));
      Object.defineProperty(navigator, 'clipboard', {
        configurable: true,
        value: { writeText },
        writable: true,
      });

      const ok = await copyToClipboard('text');
      expect(ok).toBe(false);
      expect(writeText).toHaveBeenCalledWith('text');
    });
  });

  describe('share url helpers', () => {
    it('buildShareUrl puts share into hash and clears query', () => {
      const loc = { href: 'https://app.test/editor?foo=1#tab=code' } as unknown as Location;
      const url = buildShareUrl('ENCODED', loc);
      const u = new URL(url);

      expect(u.search).toBe('');
      expect(u.hash).toContain('share=ENCODED');
      expect(u.hash).toContain('tab=code');
    });

    it('getShareFromLocation reads from hash', () => {
      const loc = { hash: '#tab=code&share=AAA', search: '' } as unknown as Location;
      expect(getShareFromLocation(loc)).toBe('AAA');
    });

    it('getShareFromLocation reads from query (back-compat)', () => {
      const loc = { hash: '', search: '?share=BBB' } as unknown as Location;
      expect(getShareFromLocation(loc)).toBe('BBB');
    });
  });
});
