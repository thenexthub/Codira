#!/bin/sh

set -e
set -x

./gradlew jar

SWIFT_VERSION="$(language -version | awk '/Codira version/ { print $3 }')"

# This is how env variables are set by setup-java
if [ "$(uname -m)" = 'arm64' ]; then
  ARCH=ARM64
  JAVAC="${JAVA_HOME_24_ARM64}/bin/javac"
  JAVA="${JAVA_HOME_24_ARM64}/bin/java"
else
  ARCH=X64
  JAVAC="${JAVA_HOME_24_X64}/bin/javac"
  JAVA="${JAVA_HOME_24_X64}/bin/java"
fi

if [ -n "$JAVA_HOME_24_$ARCH" ]; then
   export JAVA_HOME="$JAVA_HOME_24_$ARCH"
elif [ "$(uname -s)" = 'Linux' ]
then
  export PATH="${PATH}:/usr/lib/jvm/jdk-24/bin" # we need to make sure to use the latest JDK to actually compile/run the executable
fi

# check if we can compile a plain Example file that uses the generated Java bindings that should be in the generated jar
# The classpath MUST end with a * if it contains jar files, and must not if it directly contains class files.
SWIFTKIT_CORE_CLASSPATH="$(pwd)/../../CodiraKitCore/build/libs/*"
SWIFTKIT_FFM_CLASSPATH="$(pwd)/../../CodiraKitFFM/build/libs/*"
MYLIB_CLASSPATH="$(pwd)/build/libs/*"
CLASSPATH="$(pwd)/:${SWIFTKIT_FFM_CLASSPATH}:${SWIFTKIT_CORE_CLASSPATH}:${MYLIB_CLASSPATH}"
echo "CLASSPATH       = ${CLASSPATH}"

$JAVAC -cp "${CLASSPATH}" Example.java

# FIXME: move all this into Gradle or CodiraPM and make it easier to get the right classpath for running
if [ "$(uname -s)" = 'Linux' ]
then
  SWIFT_LIB_PATHS=/usr/lib/language/linux
  SWIFT_LIB_PATHS="${SWIFT_LIB_PATHS}:$(find . | grep libMyCodiraLibrary.so$ | sort | head -n1 | xargs dirname)"

  # if we are on linux, find the Codiraly or System-wide installed libraries dir
  SWIFT_CORE_LIB=$(find "$HOME"/.local -name "liblanguageCore.so" 2>/dev/null | grep "$SWIFT_VERSION" | head -n1)
  if [ -n "$SWIFT_CORE_LIB" ]; then
    SWIFT_LIB_PATHS="${SWIFT_LIB_PATHS}:$(dirname "$SWIFT_CORE_LIB")"
    ls "$SWIFT_LIB_PATHS"
  else
    # maybe there is one installed system-wide in /usr/lib?
    SWIFT_CORE_LIB2=$(find /usr/lib -name "liblanguageCore.so" 2>/dev/null | grep "$SWIFT_VERSION" | head -n1)
    if [ -n "$SWIFT_CORE_LIB2" ]; then
      SWIFT_LIB_PATHS="${SWIFT_LIB_PATHS}:$(dirname "$SWIFT_CORE_LIB2")"
    fi
  fi
elif [ "$(uname -s)" = 'Darwin' ]
then
  SWIFT_LIB_PATHS=$(find "$(languagely use --print-location)" | grep dylib$ | grep liblanguageCore | grep macos | head -n1 | xargs dirname)
  SWIFT_LIB_PATHS="${SWIFT_LIB_PATHS}:$(pwd)/$(find . | grep libMyCodiraLibrary.dylib$ | sort | head -n1 | xargs dirname)"

fi
echo "SWIFT_LIB_PATHS = ${SWIFT_LIB_PATHS}"

# Can we run the example?
${JAVA} --enable-native-access=ALL-UNNAMED \
     -Djava.library.path="${SWIFT_LIB_PATHS}" \
     -cp "${CLASSPATH}" \
     Example
