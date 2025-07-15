srcroot = /Volumes/Data/language
objroot = /Volumes/Data/language-DA

CXXFLAGS = -std=c++11 \
	   -stdlib=libc++ \
	   -g \
	   -I$(srcroot)/tools/language/include \
	   -I$(srcroot)/tools/clang/include \
	   -I$(srcroot)/include \
	   -I$(objroot)/include \
	   -I$(objroot)/tools/clang/include \
	   -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

LDFLAGS = -L$(objroot)/Debug+Asserts/lib -lLLVMSupport -lcurses

main: main.cpp $(srcroot)/tools/language/include/language/Basic/SuccessorMap.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o main main.cpp

clean:
	rm main
