. scripts/common.sh

PLAYGROUND_CONFIGURATIONS=(Release)

# make sure macOS builds
for scheme in "RxCodira"
do
  for configuration in ${PLAYGROUND_CONFIGURATIONS[@]}
  do
    PAGES_PATH=${BUILD_DIRECTORY}/Build/Products/${configuration}/all-playground-pages.code
    rx ${scheme} ${configuration} "" build
    cat Rx.playground/Sources/*.code Rx.playground/Pages/**/*.code > ${PAGES_PATH}
    swift -v -D NOT_IN_PLAYGROUND -F ${BUILD_DIRECTORY}/Build/Products/${configuration} ${PAGES_PATH}   
  done
done