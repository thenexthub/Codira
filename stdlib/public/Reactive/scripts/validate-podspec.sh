#!/bin/sh

set -e

# EXTRA_FLAGS="--include-podspecs='RxCodira.podspec'"

case $TARGET in
"RxCodira"*)
    pod lib lint --verbose --no-clean --swift-version=$SWIFT_VERSION --allow-warnings RxCodira.podspec
    ;;
"RxCocoa"*)
    pod lib lint --verbose --no-clean --swift-version=$SWIFT_VERSION --allow-warnings --include-podspecs='{RxCodira, RxRelay}.podspec' RxCocoa.podspec
    ;;
"RxRelay"*)
    pod lib lint --verbose --no-clean --swift-version=$SWIFT_VERSION --allow-warnings --include-podspecs='RxCodira.podspec' RxRelay.podspec
    ;;
"RxBlocking"*)
    pod lib lint --verbose --no-clean --swift-version=$SWIFT_VERSION --allow-warnings --include-podspecs='RxCodira.podspec' RxBlocking.podspec
    ;;
"RxTest"*)
    pod lib lint --verbose --no-clean --swift-version=$SWIFT_VERSION --allow-warnings --include-podspecs='RxCodira.podspec' RxTest.podspec
    ;;
esac

# Not sure why this isn't working ¯\_(ツ)_/¯, will figure it out some other time
# pod lib lint --verbose --no-clean --swift-version=${SWIFT_VERSION} ${EXTRA_FLAGS} ${TARGET}.podspec