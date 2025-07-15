LANGUAGE_SRCROOT=${CURDIR}/../..
SRCROOT=${LANGUAGE_SRCROOT}/..

HEADERS=${LANGUAGE_SRCROOT}/include/language/Basic/HeaderFooterLayout.h

CXXFLAGS=-Wall -std=c++17 -stdlib=libc++ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I${OBJROOT}/include -I${LANGUAGE_SRCROOT}/include

TestHeaderFooterLayout: TestHeaderFooterLayout.o
	$(CXX) $< -o $@

TestHeaderFooterLayout.o: ${HEADERS}
