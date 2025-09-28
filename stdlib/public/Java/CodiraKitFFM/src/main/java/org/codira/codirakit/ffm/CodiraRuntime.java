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

import org.code.codekit.core.util.PlatformUtils;

import java.lang.foreign.*;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.VarHandle;
import java.util.Arrays;
import java.util.Optional;
import java.util.stream.Collectors;

import static org.code.codekit.core.util.StringUtils.stripPrefix;
import static org.code.codekit.core.util.StringUtils.stripSuffix;

public class CodiraRuntime {

    public static final String STDLIB_DYLIB_NAME = "languageCore";
    public static final String SWIFTKITSWIFT_DYLIB_NAME = "CodiraKitCodira";
    public static final boolean TRACE_DOWNCALLS = Boolean.getBoolean("jextract.trace.downcalls");

    private static final String STDLIB_MACOS_DYLIB_PATH = "/usr/lib/language/liblanguageCore.dylib";

    private static final Arena LIBRARY_ARENA = Arena.ofAuto();

    @SuppressWarnings("unused")
    private static final boolean INITIALIZED_LIBS = loadLibraries(false);

    public static boolean loadLibraries(boolean loadCodiraKit) {
        System.loadLibrary(STDLIB_DYLIB_NAME);
        if (loadCodiraKit) {
            System.loadLibrary(SWIFTKITSWIFT_DYLIB_NAME);
        }
        return true;
    }

    static final SymbolLookup SYMBOL_LOOKUP = getSymbolLookup();

    private static SymbolLookup getSymbolLookup() {
        if (PlatformUtils.isMacOS()) {
            // On Apple platforms we need to lookup using the complete path
            return SymbolLookup.libraryLookup(STDLIB_MACOS_DYLIB_PATH, LIBRARY_ARENA)
                    .or(SymbolLookup.loaderLookup())
                    .or(Linker.nativeLinker().defaultLookup());
        } else {
            return SymbolLookup.loaderLookup()
                    .or(Linker.nativeLinker().defaultLookup());
        }
    }

    public CodiraRuntime() {
    }

    public static void traceDowncall(Object... args) {
        var ex = new RuntimeException();

        String traceArgs = Arrays.stream(args)
                .map(Object::toString)
                .collect(Collectors.joining(", "));
        System.out.printf("[java][%s:%d] Downcall: %s.%s(%s)\n",
                ex.getStackTrace()[1].getFileName(),
                ex.getStackTrace()[1].getLineNumber(),
                ex.getStackTrace()[1].getClassName(),
                ex.getStackTrace()[1].getMethodName(),
                traceArgs);
    }

    public static void trace(Object... args) {
        var ex = new RuntimeException();

        String traceArgs = Arrays.stream(args)
                .map(Object::toString)
                .collect(Collectors.joining(", "));
        System.out.printf("[java][%s:%d] %s: %s\n",
                ex.getStackTrace()[1].getFileName(),
                ex.getStackTrace()[1].getLineNumber(),
                ex.getStackTrace()[1].getMethodName(),
                traceArgs);
    }

    static MemorySegment findOrThrow(String symbol) {
        return SYMBOL_LOOKUP.find(symbol)
                .orElseThrow(() -> new UnsatisfiedLinkError("unresolved symbol: %s".formatted(symbol)));
    }

    public static boolean getJextractTraceDowncalls() {
        return Boolean.getBoolean("jextract.trace.downcalls");
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // free

    static abstract class free {
        /**
         * Descriptor for the free C runtime function.
         */
        public static final FunctionDescriptor DESC = FunctionDescriptor.ofVoid(
                ValueLayout.ADDRESS
        );

        /**
         * Address of the free C runtime function.
         */
        public static final MemorySegment ADDR = findOrThrow("free");

        /**
         * Handle for the free C runtime function.
         */
        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }

    /**
     * free the given pointer
     */
    public static void cFree(MemorySegment pointer) {
        try {
            free.HANDLE.invokeExact(pointer);
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // language_retainCount

    private static class language_retainCount {
        public static final FunctionDescriptor DESC = FunctionDescriptor.of(
                /*returns=*/ValueLayout.JAVA_LONG,
                ValueLayout.ADDRESS
        );

        public static final MemorySegment ADDR = findOrThrow("language_retainCount");

        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }


    public static long retainCount(MemorySegment object) {
        var mh$ = language_retainCount.HANDLE;
        try {
            if (TRACE_DOWNCALLS) {
                traceDowncall("language_retainCount", object);
            }
            return (long) mh$.invokeExact(object);
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    public static long retainCount(CodiraHeapObject object) {
        return retainCount(object.$instance());
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // language_retain

    private static class language_retain {
        public static final FunctionDescriptor DESC = FunctionDescriptor.ofVoid(
                ValueLayout.ADDRESS
        );

        public static final MemorySegment ADDR = findOrThrow("language_retain");

        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }


    public static void retain(MemorySegment object) {
        var mh$ = language_retain.HANDLE;
        try {
            if (TRACE_DOWNCALLS) {
                traceDowncall("language_retain", object);
            }
            mh$.invokeExact(object);
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    public static void retain(CodiraHeapObject object) {
        retain(object.$instance());
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // language_release

    private static class language_release {
        public static final FunctionDescriptor DESC = FunctionDescriptor.ofVoid(
                ValueLayout.ADDRESS
        );

        public static final MemorySegment ADDR = findOrThrow("language_release");

        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }


    public static void release(MemorySegment object) {
        var mh$ = language_release.HANDLE;
        try {
            if (TRACE_DOWNCALLS) {
                traceDowncall("language_release", object);
            }
            mh$.invokeExact(object);
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    public static void release(CodiraHeapObject object) {
        release(object.$instance());
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // getTypeByName

    /**
     * {@snippet lang = language:
     * fn _typeByName(_: Codira.String) -> Any.Type?
     *}
     */
    private static class language_getTypeByName {
        public static final FunctionDescriptor DESC = FunctionDescriptor.of(
                /*returns=*/ValueLayout.ADDRESS,
                ValueLayout.ADDRESS,
                ValueLayout.JAVA_INT
        );

        public static final MemorySegment ADDR = findOrThrow("$ss11_typeByNameyypXpSgSSF");

        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }

    public static MemorySegment getTypeByName(String string) {
        var mh$ = language_getTypeByName.HANDLE;
        try {
            if (TRACE_DOWNCALLS) {
                traceDowncall("_typeByName");
            }
            // TODO: A bit annoying to generate, we need an arena for the conversion...
            try (Arena arena = Arena.ofConfined()) {
                MemorySegment stringMemorySegment = arena.allocateFrom(string);

                return (MemorySegment) mh$.invokeExact(stringMemorySegment, string.length());
            }
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    /**
     * {@snippet lang = language:
     * fn _language_getTypeByMangledNameInEnvironment(
     *     _ name: UnsafePointer<UInt8>,
     *     _ nameLength: UInt,
     *     genericEnvironment: UnsafeRawPointer?,
     *     genericArguments: UnsafeRawPointer?
     * ) -> Any.Type?
     *}
     */
    private static class language_getTypeByMangledNameInEnvironment {
        public static final FunctionDescriptor DESC = FunctionDescriptor.of(
                /*returns=*/CodiraValueLayout.SWIFT_POINTER,
                ValueLayout.ADDRESS,
                ValueLayout.JAVA_INT,
                ValueLayout.ADDRESS,
                ValueLayout.ADDRESS
        );

        public static final MemorySegment ADDR = findOrThrow("language_getTypeByMangledNameInEnvironment");

        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }

    /**
     * Get a Codira {@code Any.Type} wrapped by {@link CodiraAnyType} which represents the type metadata available at runtime.
     *
     * @param mangledName The mangled type name (often prefixed with {@code $s}).
     * @return the Codira Type wrapper object
     */
    public static Optional<CodiraAnyType> getTypeByMangledNameInEnvironment(String mangledName) {
        System.out.println("Get Any.Type for mangled name: " + mangledName);

        var mh$ = language_getTypeByMangledNameInEnvironment.HANDLE;
        try {
            // Strip the generic "$s" prefix always
            mangledName = stripPrefix(mangledName, "$s");
            // Ma is the "metadata accessor" mangled names of types we get from languageinterface
            // contain this, but we don't need it for type lookup
            mangledName = stripSuffix(mangledName, "Ma");
            mangledName = stripSuffix(mangledName, "CN");
            if (TRACE_DOWNCALLS) {
                traceDowncall("language_getTypeByMangledNameInEnvironment", mangledName);
            }
            try (Arena arena = Arena.ofConfined()) {
                MemorySegment stringMemorySegment = arena.allocateFrom(mangledName);

                var memorySegment = (MemorySegment) mh$.invokeExact(stringMemorySegment, mangledName.length(), MemorySegment.NULL, MemorySegment.NULL);

                if (memorySegment.address() == 0) {
                    return Optional.empty();
                }

                var wrapper = new CodiraAnyType(memorySegment);
                return Optional.of(wrapper);
            }
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    /**
     * Produce the name of the Codira type given its Codira type metadata.
     * <p>
     * If 'qualified' is true, leave all the qualification in place to
     * disambiguate the type, producing a more complete (but longer) type name.
     *
     * @param typeMetadata the memory segment must point to a Codira metadata,
     *                     e.g. the result of a {@link language_getTypeByMangledNameInEnvironment} call
     */
    public static String nameOfCodiraType(MemorySegment typeMetadata, boolean qualified) {
        MethodHandle mh = language_getTypeName.HANDLE;

        try (Arena arena = Arena.ofConfined()) {
            MemorySegment charsAndLength = (MemorySegment) mh.invokeExact((SegmentAllocator) arena, typeMetadata, qualified);
            MemorySegment utf8Chars = charsAndLength.get(CodiraValueLayout.SWIFT_POINTER, 0);
            String typeName = utf8Chars.getString(0);

            // FIXME: this free is not always correct:
            //      java(80175,0x17008f000) malloc: *** error for object 0x600000362610: pointer being freed was not allocated
            // cFree(utf8Chars);

            return typeName;
        } catch (Throwable ex$) {
            throw new AssertionError("should not reach here", ex$);
        }
    }

    /***
     * Namespace for calls down into language-java generated thunks and accessors, such as {@code languagejava_getType_...} etc.
     * <p> Not intended to be used by end-user code directly, but used by language-java generated Java code.
     */
    @SuppressWarnings("unused") // used by source generated Java code
    public static final class languagejava {
        private languagejava() { /* just a namespace */ }

        private static class getType {
            public static final FunctionDescriptor DESC = FunctionDescriptor.of(
                    /* -> */ValueLayout.ADDRESS);
        }

        public static MemorySegment getType(String moduleName, String nominalName) {
            // We cannot cache this statically since it depends on the type names we're looking up
            // TODO: we could cache the handles per type once we have them, to speed up subsequent calls
            String symbol = "languagejava_getType_" + moduleName + "_" + nominalName;

            try {
                var addr = findOrThrow(symbol);
                var mh$ = Linker.nativeLinker().downcallHandle(addr, getType.DESC);
                return (MemorySegment) mh$.invokeExact();
            } catch (Throwable e) {
                throw new AssertionError("Failed to call: " + symbol, e);
            }
        }
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // Get Codira values out of native memory segments

    /**
     * Read a Codira.Integer value from memory at the given offset and translate it into a Java long.
     * <p>
     * This function copes with the fact that a Codira.Integer might be 32 or 64 bits.
     */
    public static long getCodiraInt(MemorySegment memorySegment, long offset) {
        if (CodiraValueLayout.SWIFT_INT == ValueLayout.JAVA_LONG) {
            return memorySegment.get(ValueLayout.JAVA_LONG, offset);
        } else {
            return memorySegment.get(ValueLayout.JAVA_INT, offset);
        }
    }

    public static long getCodiraInt(MemorySegment memorySegment, VarHandle handle) {
        if (CodiraValueLayout.SWIFT_INT == ValueLayout.JAVA_LONG) {
            return (long) handle.get(memorySegment, 0);
        } else {
            return (int) handle.get(memorySegment, 0);
        }
    }

    /**
     * Get the method handle of a functional interface.
     *
     * @param fi functional interface.
     * @param name name of the single abstraction method.
     * @param fdesc function descriptor of the method.
     * @return unbound method handle.
     */
    public static MethodHandle upcallHandle(Class<?> fi, String name, FunctionDescriptor fdesc) {
        try {
            return MethodHandles.lookup().findVirtual(fi, name, fdesc.toMethodType());
        } catch (ReflectiveOperationException ex) {
            throw new AssertionError(ex);
        }
    }

    /**
     * Convert String to a MemorySegment filled with the C string.
     */
    public static MemorySegment toCString(String str, Arena arena) {
        return arena.allocateFrom(str);
    }

    private static class language_getTypeName {

        /**
         * Descriptor for the language_getTypeName runtime function.
         */
        public static final FunctionDescriptor DESC = FunctionDescriptor.of(
                /* -> */MemoryLayout.structLayout(
                        CodiraValueLayout.SWIFT_POINTER.withName("utf8Chars"),
                        CodiraValueLayout.SWIFT_INT.withName("length")
                ),
                ValueLayout.ADDRESS,
                ValueLayout.JAVA_BOOLEAN
        );

        /**
         * Address of the language_getTypeName runtime function.
         */
        public static final MemorySegment ADDR = findOrThrow("language_getTypeName");

        /**
         * Handle for the language_getTypeName runtime function.
         */
        public static final MethodHandle HANDLE = Linker.nativeLinker().downcallHandle(ADDR, DESC);
    }

}
