SRCROOT=/Volumes/Data/language
OBJROOT=/Volumes/Data/language-DA

HEADERS=${SRCROOT}/tools/language/include/language/Basic/ClusteredBitVector.h
SOURCES=${SRCROOT}/tools/language/lib/Basic/ClusteredBitVector.cpp

generator: generator.cpp ${HEADERS} ${SOURCES}
	xcrun clang++ -std=c++11 -stdlib=libc++ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I${OBJROOT}/include -I${SRCROOT}/include -I${SRCROOT}/tools/language/include -L${OBJROOT}/lib -lLLVMSupport -lcurses generator.cpp ${SOURCES} -o generator

test: test.cpp ${HEADERS} ${SOURCES}
	xcrun clang++ -g -std=c++11 -stdlib=libc++ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I${OBJROOT}/include -I${SRCROOT}/include -I${SRCROOT}/tools/language/include -L${OBJROOT}/lib -lLLVMSupport -lcurses test.cpp ${SOURCES} -o test
