# DevEco Hello World Application

This application intermediate representation is based on built-in DevEco's `Empty Ability` template.

To reproduce the application scenario, the following steps can be applied:
- Create new project at DevEco, select `Application -> Empty Ability` template.
- build HAP (or just module named `entry` if defaults are used during project creation)
- go to `$PROJECT_ROOT/entry/build/default/cache/default/default@CompileArkTS/esmodule/debug`
- get `filesInfo.txt` file and `entry` directory contents with intermediate TS files
- create `modules` directory and copy `entry` directory inside
- copy `filesInfo.txt`, rename it to `filesInfo.rel.txt` and change absolute path prefix (like `/path/to/project/build/dir`) to relative `modules/`

## Application structure

```
|- filesInfo.rel.txt        # generated fileInfo.txt with absolute path changed to relatives
|- modules
   |- entry                 # generated intermediate TS files for `entry` module
```

## Building the application

```(bash)
# echo "generate fileInfo.txt for by extending its relative paths to absolute"
export APP_DIR=`realpath $LIBABCKIT/tests/apps/deveco-hello-world`; BUILD_DIR=build;
mkdir ${BUILD_DIR}
awk '{ print ENVIRON["APP_DIR"]"/"$$0 }' ${APP_DIR}/filesInfo.rel.txt > ${BUILD_DIR}/filesInfo.txt
es2abc --module --merge-abc --enable-annotations --output ${BUILD_DIR}/app.abc @${BUILD_DIR}/filesInfo.txt
```
