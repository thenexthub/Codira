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

package std.core;

export final class DataView implements ArrayBufferView {
    /** Underlying buffer */
    private readonly buffer_: ArrayBuffer
    /** Count of bytes in a view */
    private readonly byteLength_: int
    /** Offset from start of {@link buffer} */
    private readonly byteOffset_: int

    public get buffer(): ArrayBuffer {
        return this.buffer_
    }

    public get byteLength(): int {
        return this.byteLength_
    }

    public get byteOffset(): int {
        return this.byteOffset_
    }

    private readonly actualBuffer: ArrayBuffer

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @param byteLength lenth of bytes to take
     * @throws RangeError if provided indicies are invalid
     */
    public constructor(buffer: ArrayBuffer, byteOffset?: int, byteLength?: int) {
        this.buffer_ = buffer
        this.actualBuffer = buffer
        if (byteOffset === undefined) {
            byteOffset = 0
        }
        if (byteLength === undefined) {
            byteLength = this.buffer_.getByteLength() - byteOffset
        }
        const bufLen = this.buffer_.getByteLength()
        if (byteOffset < 0 || byteLength < 0 || byteOffset > bufLen || byteOffset + byteLength > bufLen) {
            throw new RangeError("invalid arguments")
        }
        this.byteOffset_ = byteOffset
        this.byteLength_ = byteLength
    }

    // === Int8 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt8(byteOffset: int): int {
        return this.getInt8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: int, value: int): void {
        this.setInt8Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: int, value: number): void {
        this.setInt8Big(byteOffset, value)
    }

    private getInt8Big(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 1 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 0 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        
        return res.toByte()
    }

    private setInt8Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 1.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 0 - i,  byteVal)
        }
    }

    private setInt8Big(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setInt8Big(byteOffset, val)
    }

    // === Uint8 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint8(byteOffset: int): int {
        return this.getUint8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: int, value: int): void {
        this.setUint8Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: int, value: number): void {
        this.setUint8Big(byteOffset, value)
    }

    private getUint8Big(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 1 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 0 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        return res
    }

    private setUint8Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 1.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 0 - i,  byteVal)
        }
    }

    private setUint8Big(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setUint8Big(byteOffset, val)
    }

    // === Int16 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt16(byteOffset: int): int {
        return this.getInt16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt16(byteOffset: int, value: int): void {
        this.setInt16Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setInt16Little(byteOffset, value)
        } else {
            this.setInt16Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setInt16Little(byteOffset, value)
        } else {
            this.setInt16Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt16(byteOffset: int, littleEndian?: boolean): int {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getInt16Little(byteOffset)
        } else {
            return this.getInt16Big(byteOffset)
        }
    }

    private getInt16Little(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        
        return res.toShort()
    }

    private setInt16Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setInt16Little(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setInt16Little(byteOffset, val)
    }

    private getInt16Big(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 1 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        
        return res.toShort()
    }

    private setInt16Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 1 - i,  byteVal)
        }
    }

    private setInt16Big(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setInt16Big(byteOffset, val)
    }

    // === Uint16 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint16(byteOffset: int): int {
        return this.getUint16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint16(byteOffset: int, value: int): void {
        this.setUint16Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setUint16Little(byteOffset, value)
        } else {
            this.setUint16Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setUint16Little(byteOffset, value)
        } else {
            this.setUint16Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint16(byteOffset: int, littleEndian?: boolean): int {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getUint16Little(byteOffset)
        } else {
            return this.getUint16Big(byteOffset)
        }
    }

    private getUint16Little(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        return res
    }

    private setUint16Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setUint16Little(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setUint16Little(byteOffset, val)
    }

    private getUint16Big(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 1 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        return res
    }

    private setUint16Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 1 - i,  byteVal)
        }
    }

    private setUint16Big(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setUint16Big(byteOffset, val)
    }

    // === Int32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt32(byteOffset: int): int {
        return this.getInt32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt32(byteOffset: int, value: int): void {
        this.setInt32Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt32(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setInt32Little(byteOffset, value)
        } else {
            this.setInt32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt32(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setInt32Little(byteOffset, value)
        } else {
            this.setInt32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt32(byteOffset: int, littleEndian?: boolean): int {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getInt32Little(byteOffset)
        } else {
            return this.getInt32Big(byteOffset)
        }
    }

    private getInt32Little(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        
        return res.toInt()
    }

    private setInt32Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setInt32Little(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setInt32Little(byteOffset, val)
    }

    private getInt32Big(byteOffset: int): int {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 3 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        
        return res.toInt()
    }

    private setInt32Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }

    private setInt32Big(byteOffset: int, value: number): void {
        let temp: long = value.toLong();
        if (temp == Long.MAX_VALUE || temp == Long.MIN_VALUE) {
            temp = 0;
        }
        const val: int = temp.toInt();
        this.setInt32Big(byteOffset, val)
    }

    // === Uint32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint32(byteOffset: int): number {
        return this.getUint32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint32(byteOffset: int, value: long): void {
        this.setUint32Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setUint32Little(byteOffset, value)
        } else {
            this.setUint32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setUint32Little(byteOffset, value)
        } else {
            this.setUint32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint32(byteOffset: int, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getUint32Little(byteOffset)
        } else {
            return this.getUint32Big(byteOffset)
        }
    }

    private getUint32Little(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return res
    }

    private setUint32Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        if (bits == Long.MAX_VALUE || bits == Long.MIN_VALUE) {
            bits = 0;
        }
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setUint32Little(byteOffset: int, value: number): void {
        
        const val: long = value.toLong();
        this.setUint32Little(byteOffset, val)
    }

    private getUint32Big(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 3 - i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return res
    }

    private setUint32Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        if (bits == Long.MAX_VALUE || bits == Long.MIN_VALUE) {
            bits = 0;
        }
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }

    private setUint32Big(byteOffset: int, value: number): void {
        
        const val: long = value.toLong();
        this.setUint32Big(byteOffset, val)
    }

    // === Float32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat32(byteOffset: int): number {
        return this.getFloat32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat32(byteOffset: int, value: float): void {
        this.setFloat32Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat32(byteOffset: int, value: float, littleEndian: boolean): void {
        if (littleEndian) {
            this.setFloat32Little(byteOffset, value)
        } else {
            this.setFloat32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat32(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setFloat32Little(byteOffset, value)
        } else {
            this.setFloat32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat32(byteOffset: int, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getFloat32Little(byteOffset)
        } else {
            return this.getFloat32Big(byteOffset)
        }
    }

    private getFloat32Little(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        return Float.bitCastFromInt(res)
    }

    private setFloat32Little(byteOffset: int, value: float): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setFloat32Little(byteOffset: int, value: number): void {
        
        const val: float = value.toFloat();
        this.setFloat32Little(byteOffset, val)
    }

    private getFloat32Big(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 3 - i)).toInt();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toInt();
        }
        return Float.bitCastFromInt(res)
    }

    private setFloat32Big(byteOffset: int, value: float): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }

    private setFloat32Big(byteOffset: int, value: number): void {
        
        const val: float = value.toFloat();
        this.setFloat32Big(byteOffset, val)
    }

    // === Int64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigInt64(byteOffset: int): bigint {
        return this.getBigInt64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setBigInt64(byteOffset: int, value: long): void {
        this.setBigInt64Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigInt64(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setBigInt64Little(byteOffset, value)
        } else {
            this.setBigInt64Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigInt64(byteOffset: int, value: bigint, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setBigInt64Little(byteOffset, value)
        } else {
            this.setBigInt64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigInt64(byteOffset: int, littleEndian?: boolean): bigint {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getBigInt64Little(byteOffset)
        } else {
            return this.getBigInt64Big(byteOffset)
        }
    }

    private getBigInt64Little(byteOffset: int): bigint {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return new BigInt(res)
    }

    private setBigInt64Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setBigInt64Little(byteOffset: int, value: bigint): void {
        const val: long = value.getLong();
        this.setBigInt64Little(byteOffset, val)
    }

    private getBigInt64Big(byteOffset: int): bigint {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 7 - i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return new BigInt(res)
    }

    private setBigInt64Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }

    private setBigInt64Big(byteOffset: int, value: bigint): void {
        const val: long = value.getLong();
        this.setBigInt64Big(byteOffset, val)
    }

    // === Uint64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigUint64(byteOffset: int): bigint {
        return this.getBigUint64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setBigUint64(byteOffset: int, value: long): void {
        this.setBigUint64Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigUint64(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setBigUint64Little(byteOffset, value)
        } else {
            this.setBigUint64Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigUint64(byteOffset: int, value: bigint, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setBigUint64Little(byteOffset, value)
        } else {
            this.setBigUint64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigUint64(byteOffset: int, littleEndian?: boolean): bigint {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getBigUint64Little(byteOffset)
        } else {
            return this.getBigUint64Big(byteOffset)
        }
    }

    private getBigUint64Little(byteOffset: int): bigint {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return DataView.bigintFromULong(res)
    }

    private setBigUint64Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private setBigUint64Little(byteOffset: int, value: bigint): void {
        const val: long = value.getLong();
        this.setBigUint64Little(byteOffset, val)
    }

    private getBigUint64Big(byteOffset: int): bigint {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 7 - i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return DataView.bigintFromULong(res)
    }

    private setBigUint64Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }

    private setBigUint64Big(byteOffset: int, value: bigint): void {
        const val: long = value.getLong();
        this.setBigUint64Big(byteOffset, val)
    }

    // === Float64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat64(byteOffset: int): number {
        return this.getFloat64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat64(byteOffset: int, value: number): void {
        this.setFloat64Big(byteOffset, value)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat64(byteOffset: int, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setFloat64Little(byteOffset, value)
        } else {
            this.setFloat64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat64(byteOffset: int, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getFloat64Little(byteOffset)
        } else {
            return this.getFloat64Big(byteOffset)
        }
    }

    private getFloat64Little(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return Double.bitCastFromLong(res)
    }

    private setFloat64Little(byteOffset: int, value: number): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }

    private getFloat64Big(byteOffset: int): number {
        if (byteOffset < 0 || byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            
            let byteVal = (this.actualBuffer.at(startByte + 7 - i)).toLong();
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)).toLong();
        }
        return Double.bitCastFromLong(res)
    }

    private setFloat64Big(byteOffset: int, value: number): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff).toByte()
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }

    private static bigintFromULong(x: long): bigint {
        const noSignMask: long = ((1 as long) << 63) - 1
        return new BigInt(x & noSignMask) + (new BigInt((x >> 63) & 0x1) << new BigInt(63))
    }
}
