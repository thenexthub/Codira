# coding=utf-8
#
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

"""Creates a unique name.

The mangling scheme preserves information about original underscore positions in identifiers
while creating a flat namespace. This is useful for languages that allow underscores in
package names but need to generate unique identifiers for linking or bytecode generation.

Algorithm:
1. Join all segments with underscore separator
2. Record positions of "special" underscores (those in original identifiers)
3. Encode special positions using ULEB8 compression
4. Append kind marker and encoded positions as suffix

Input:
    pkg_segments: package/name segments (e.g. ['a', 'b_c', 'd', 'e_f_g']).
    Last segment is assumed to be the identifier name

Returns: Mangled name that preserves underscore position information
Format: {joined_segments}_{kind}{encoded_positions}

Example:
    Input:
        pkg_segments = ['a', 'b_c', 'd', 'e_f_g']
        kind = 'f' (function)

    Mangling steps:
    1. Join segments:      'a_b_c_d_e_f_g'
       Underscore positions: 0 1 2 3 4 5

    2. Record special underscore positions:
        - 'b_c'   -> position 1
        - 'e_f_g' -> positions 4,5
        special_positions = [1, 4, 5]

    3. Convert to position deltas:
        [1, 4-1=3, 5-4=1] = [1, 3, 1]

    4. Encode as ULEB8:
        [0b0001, 0b0011, 0b0001] = 0x131

    5. Add kind marker and suffix:
        Final result = 'a_b_c_d_e_f_g_f131'

Note:
    - Position encoding uses ULEB128 for compact representation
    - Kind marker is a single character indicating symbol type:
    - Decoder can reconstruct original identifier structure using
        encoded position information
"""

from enum import Enum


class DeclKind(Enum):
    FUNC = "f"
    METHOD = "m"
    TYPE = "t"
    IID = "i"
    UNION = "union"
    FTABLE = "ftable"
    VTABLE = "vtable"
    DYNAMIC_CAST = "dynamic"
    STATIC_CAST = "static"


def _encode_uleb8(value: int) -> list[int]:
    """Encodes a single integer in ULEB8 format.

    Args:
        value: Non-negative integer to encode

    Returns:
        List of bytes representing the ULEB8 encoding

    Raises:
        ValueError: If value is negative
    """
    if value < 0:
        raise ValueError("ULEB8 encoding requires non-negative values")

    result = []
    while True:
        byte = value & 0x0F
        value >>= 4
        if value:
            byte |= 0x10  # Set continuation bit
        result.append(byte)
        if not value:
            break
    return result


def _decode_uleb8(encoded: str, start_pos: int) -> tuple[int, int]:
    """Decodes a single ULEB8 value from a hex string.

    Args:
        encoded: Hex string containing ULEB8 encoded values
        start_pos: Starting position in the hex string

    Returns:
        Tuple of (decoded value, number of bytes consumed)

    Raises:
        ValueError: If invalid hex characters are encountered
    """
    value = 0
    shift = 0
    pos = start_pos

    while pos < len(encoded):
        try:
            byte = int(encoded[pos], 16)
        except ValueError:
            raise ValueError(f"Invalid hex character at position {pos}") from None

        value |= (byte & 0x0F) << shift
        pos += 1
        if not (byte & 0x10):
            break
        shift += 4

    return value, pos - start_pos


def encode(segments: list[str], kind: DeclKind) -> str:
    """Encodes segments into a mangled name.

    Args:
        segments: List of name segments
        kind: Single character kind marker

    Returns:
        Mangled name string

    Raises:
        ValueError: If segments is empty or kind is invalid
    """
    if not segments:
        raise ValueError("segments list cannot be empty")

    # Find positions of "special" underscores (those within segments)
    special_positions = []
    total_underscores = 0

    for segment in segments:
        for char in segment:
            if char == "_":
                special_positions.append(total_underscores)
                total_underscores += 1
        total_underscores += 1  # For segment separator

    # Convert to position deltas
    deltas = []
    if special_positions:
        deltas.append(special_positions[0])  # First position is absolute
        for i in range(1, len(special_positions)):
            deltas.append(special_positions[i] - special_positions[i - 1])

    # Encode positions
    encoded_bytes = []
    for delta in deltas:
        encoded_bytes.extend(_encode_uleb8(delta))

    # Convert to hex string
    encoded_str = "".join(f"{byte:x}" for byte in encoded_bytes)

    base = "_".join(segments)
    return f"{base}_{kind.value}{encoded_str}"


def decode(mangled_name: str) -> tuple[list[str], DeclKind]:
    """Decodes a mangled name back into original segments.

    Args:
        mangled_name: The mangled name to decode

    Returns:
        List of original name segments

    Raises:
        ValueError: If mangled_name format is invalid
    """
    try:
        last_underscore = mangled_name.rindex("_")
    except ValueError:
        raise ValueError("Invalid mangled name: missing underscore separator") from None

    if last_underscore + 2 >= len(mangled_name):
        raise ValueError("Invalid mangled name: missing kind marker or encoding")

    base_name = mangled_name[:last_underscore]
    kind = DeclKind(mangled_name[last_underscore + 1])
    encoding = mangled_name[last_underscore + 2 :]

    # Decode special underscore positions
    special_positions = []
    pos = 0

    try:
        while pos < len(encoding):
            delta, consumed = _decode_uleb8(encoding, pos)
            if pos == 0:
                special_positions.append(delta)
            else:
                special_positions.append(special_positions[-1] + delta)
            pos += consumed
    except ValueError as e:
        raise ValueError(f"Failed to decode positions: {e!s}") from None

    # Split and reconstruct segments
    parts = base_name.split("_")
    if not parts:
        raise ValueError("Invalid mangled name: empty base name") from None

    segments = []
    current_parts = [parts[0]]
    underscore_pos = 0

    for underscore_pos in range(1, len(parts)):
        if underscore_pos - 1 in special_positions:
            current_parts.append(parts[underscore_pos])
        else:
            segments.append("_".join(current_parts))
            current_parts = [parts[underscore_pos]]

    if current_parts:
        segments.append("_".join(current_parts))

    return segments, kind