/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package std.core;

export namespace reflect {

/**
 * Represents instance field of class or interface
 *
 */
export final class InstanceField extends Field {
    private constructor() { throw new Error("InstanceField constructor called") }

    public equals(other: InstanceField): boolean {
        return this.fieldPtr == other.fieldPtr
    }

    /**
     * Reads value from an instance field.
     *
     * @param thisObj object used as `this`.
     *
     * @returns field value.
     *
     * @throws OutofMemoryError if there is no memory to allocate return value.
     *
     * @throws TypeError if there is an incompatible `this`.
     */
    public getValue(thisObj: Object): Any {
        return super.getValueInternal(thisObj)
    }

    /**
     * Writes value into an instance field.
     *
     * @param thisObj object used as `this`.
     *
     * @param value value to write.
     *
     * @throws TypeError if there is an incompatible `this` or value type.
     */
    public setValue(thisObj: Object, value: Any): void {
        if (this.isReadonly()) {
            throw new TypeError("setValue to a readonly field")
        }
        super.setValueInternal(thisObj, value)
    }
}

} // namespace reflect
