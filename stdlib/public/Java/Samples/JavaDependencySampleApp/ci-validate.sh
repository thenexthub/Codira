#!/bin/sh

set -e
set -x

# invoke resolve as part of a build run
language run --disable-sandbox

# explicitly invoke resolve without explicit path or dependency
# the dependencies should be uses from the --language-module
language run language-java resolve \
  Sources/JavaCommonsCSV/language-java.config \
  --language-module JavaCommonsCSV \
  --output-directory .build/plugins/outputs/javadependencysampleapp/JavaCommonsCSV/destination/CodiraJavaPlugin/
