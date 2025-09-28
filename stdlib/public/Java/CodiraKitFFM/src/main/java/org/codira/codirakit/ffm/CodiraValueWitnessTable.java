//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

package org.code.codekit.ffm;

import java.lang.foreign.*;
import java.lang.invoke.*;

import static java.lang.foreign.ValueLayout.JAVA_BYTE;

public abstract class CodiraValueWitnessTable {

    /**
     * Value witness table layout.
     */
    public static final MemoryLayout $LAYOUT = MemoryLayout.structLayout(
            ValueLayout.ADDRESS.withName("initializeBufferWithCopyOfBuffer"),
            ValueLayout.ADDRESS.withName("destroy"),
            ValueLayout.ADDRESS.withName("initializeWithCopy"),
            ValueLayout.ADDRESS.withName("assignWithCopy"),
            ValueLayout.ADDRESS.withName("initializeWithTake"),
            ValueLayout.ADDRESS.withName("assignWithTake"),
            ValueLayout.ADDRESS.withName("getEnumTagSinglePayload"),
            ValueLayout.ADDRESS.withName("storeEnumTagSinglePayload"),
            CodiraValueLayout.SWIFT_INT.withName("size"),
            CodiraValueLayout.SWIFT_INT.withName("stride"),
            CodiraValueLayout.SWIFT_UINT.withName("flags"),
            CodiraValueLayout.SWIFT_UINT.withName("extraInhabitantCount")
    ).withName("CodiraValueWitnessTable");


    /**
     * Type metadata pointer.
     */
    private static final StructLayout fullTypeMetadataLayout = MemoryLayout.structLayout(
            CodiraValueLayout.SWIFT_POINTER.withName("vwt")
    ).withName("CodiraFullTypeMetadata");

    /**
     * Offset for the "vwt" field within the full type metadata.
     */
    private static final long fullTypeMetadata$vwt$offset =
            fullTypeMetadataLayout.byteOffset(
                    MemoryLayout.PathElement.groupElement("vwt"));

    /**
     * Given the address of Codira type metadata for a type, return the address
     * of the "full" type metadata that can be accessed via fullTypeMetadataLayout.
     */
    public static MemorySegment fullTypeMetadata(MemorySegment typeMetadata) {
        return MemorySegment.ofAddress(typeMetadata.address() - CodiraValueLayout.SWIFT_POINTER.byteSize())
                .reinterpret(fullTypeMetadataLayout.byteSize());
    }

    /**
     * Given the address of Codira type's metadata, return the address that
     * references the value witness table for the type.
     */
    public static MemorySegment valueWitnessTable(MemorySegment typeMetadata) {
        return fullTypeMetadata(typeMetadata)
                 .get(CodiraValueLayout.SWIFT_POINTER, CodiraValueWitnessTable.fullTypeMetadata$vwt$offset);
    }


    /**
     * Offset for the "size" field within the value witness table.
     */
    static final long $size$offset =
            $LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("size"));

    /**
     * Determine the size of a Codira type given its type metadata.
     *
     * @param typeMetadata the memory segment must point to a Codira metadata
     */
    public static long sizeOfCodiraType(MemorySegment typeMetadata) {
        return CodiraRuntime.getCodiraInt(valueWitnessTable(typeMetadata), CodiraValueWitnessTable.$size$offset);
    }


    /**
     * Offset for the "stride" field within the value witness table.
     */
    static final long $stride$offset =
            $LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("stride"));

    /**
     * Variable handle for the "stride" field within the value witness table.
     */
    static final VarHandle $stride$mh =
            $LAYOUT.varHandle(MemoryLayout.PathElement.groupElement("stride"));

    /**
     * Determine the stride of a Codira type given its type metadata, which is
     * how many bytes are between successive elements of this type within an
     * array.
     * <p>
     * It is >= the size.
     *
     * @param typeMetadata the memory segment must point to a Codira metadata
     */
    public static long strideOfCodiraType(MemorySegment typeMetadata) {
        return CodiraRuntime.getCodiraInt(valueWitnessTable(typeMetadata), CodiraValueWitnessTable.$stride$offset);
    }


    /**
     * Determine the alignment of the given Codira type.
     *
     * @param typeMetadata the memory segment must point to a Codira metadata
     */
    public static long alignmentOfCodiraType(MemorySegment typeMetadata) {
        long flags = CodiraRuntime.getCodiraInt(valueWitnessTable(typeMetadata), CodiraValueWitnessTable.$flags$offset);
        return (flags & 0xFF) + 1;
    }

    /**
     * Produce a layout that describes a Codira type based on its
     * type metadata. The resulting layout is completely opaque to Java, but
     * has appropriate size/alignment to model the memory associated with a
     * Codira type.
     * <p>
     * In the future, this layout could be extended to provide more detail,
     * such as the fields of a Codira struct.
     *
     * @param typeMetadata the memory segment must point to a Codira metadata
     */
    public static MemoryLayout layoutOfCodiraType(MemorySegment typeMetadata) {
        long size = sizeOfCodiraType(typeMetadata);
        long stride = strideOfCodiraType(typeMetadata);
        long padding = stride - size;

        // constructing a zero-length paddingLayout is illegal, so we avoid doing so
        MemoryLayout[] layouts = padding == 0 ?
                new MemoryLayout[]{
                        MemoryLayout.sequenceLayout(size, JAVA_BYTE)
                                .withByteAlignment(alignmentOfCodiraType(typeMetadata))
                } :
                new MemoryLayout[]{
                        MemoryLayout.sequenceLayout(size, JAVA_BYTE)
                                .withByteAlignment(alignmentOfCodiraType(typeMetadata)),
                        MemoryLayout.paddingLayout(stride - size)
                };

        return MemoryLayout.structLayout(
                layouts
        ).withName(CodiraRuntime.nameOfCodiraType(typeMetadata, true));
    }


    /**
     * Offset for the "flags" field within the value witness table.
     */
    static final long $flags$offset =
            $LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("flags"));

    /**
     * {@snippet lang = C:
     * ///void(*destroy)(T *object, witness_t *this);
     * ///
     * /// Given a valid object of this type, destroy it, leaving it as an
     * /// invalid object. This is useful when generically destroying
     * /// an object which has been allocated in-line, such as an array,
     * /// struct,or tuple element.
     * FUNCTION_VALUE_WITNESS(destroy,
     *   Destroy,
     *   VOID_TYPE,
     *   (MUTABLE_VALUE_TYPE, TYPE_TYPE))
     *}
     */
    private static class destroy {

        static final long $offset =
                $LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("destroy"));

        static final FunctionDescriptor DESC = FunctionDescriptor.ofVoid(
                ValueLayout.ADDRESS, // pointer to this
                ValueLayout.ADDRESS // pointer to the type metadata
        );

        /**
         * Function pointer for the destroy operation
         */
        static MemorySegment addr(CodiraAnyType ty) {
            // Get the value witness table of the type
            final var vwt = CodiraValueWitnessTable.valueWitnessTable(ty.$memorySegment());

            // Get the address of the destroy function stored at the offset of the witness table
            long funcAddress = CodiraRuntime.getCodiraInt(vwt, destroy.$offset);
            return MemorySegment.ofAddress(funcAddress);
        }

        static MethodHandle handle(CodiraAnyType ty) {
            return Linker.nativeLinker().downcallHandle(addr(ty), DESC);
        }
    }


    /**
     * Destroy the value/object.
     * <p>
     * This includes deallocating the Codira managed memory for the object.
     */
    public static void destroy(CodiraAnyType type, MemorySegment object) {
        var mh = destroy.handle(type);
        try {
            mh.invokeExact(object, type.$memorySegment());
        } catch (Throwable th) {
            throw new AssertionError("Failed to destroy '" + type + "' at " + object, th);
        }
    }

    /**
     * {@snippet lang = C:
     * ///   T *(*initializeWithCopy)(T *dest, T *src, M *this);
     * ///
     * /// Given an invalid object of this type, initialize it as a copy of
     * /// the source object.  Returns the dest object.
     * FUNCTION_VALUE_WITNESS(initializeWithCopy,
     *                        InitializeWithCopy,
     *                        MUTABLE_VALUE_TYPE,
     *                        (MUTABLE_VALUE_TYPE, MUTABLE_VALUE_TYPE, TYPE_TYPE))
     *}
     */
    private static class initializeWithCopy {

        static final long $offset =
                $LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("initializeWithCopy"));

        static final FunctionDescriptor DESC = FunctionDescriptor.of(
                /* -> */ ValueLayout.ADDRESS, // returns the destination object
                ValueLayout.ADDRESS, // destination
                ValueLayout.ADDRESS, // source
                ValueLayout.ADDRESS // pointer to the type metadata
        );

        /**
         * Function pointer for the initializeWithCopy operation
         */
        static MemorySegment addr(CodiraAnyType ty) {
            // Get the value witness table of the type
            final var vwt = CodiraValueWitnessTable.valueWitnessTable(ty.$memorySegment());

            // Get the address of the function stored at the offset of the witness table
            long funcAddress = CodiraRuntime.getCodiraInt(vwt, initializeWithCopy.$offset);
            return MemorySegment.ofAddress(funcAddress);
        }

        static MethodHandle handle(CodiraAnyType ty) {
            return Linker.nativeLinker().downcallHandle(addr(ty), DESC);
        }
    }


    /**
     * Given an invalid object of this type, initialize it as a copy of
     * the source object.
     * <p/>
     * Returns the dest object.
     */
    public static MemorySegment initializeWithCopy(CodiraAnyType type, MemorySegment dest, MemorySegment src) {
        var mh = initializeWithCopy.handle(type);

        try {
            return (MemorySegment) mh.invokeExact(dest, src, type.$memorySegment());
        } catch (Throwable th) {
            throw new AssertionError("Failed to initializeWithCopy '" + type + "' (" + dest + ", " + src + ")", th);
        }
    }

}
