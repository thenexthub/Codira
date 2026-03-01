# Tutorial for build panda SDK using Docker container 

## Build image
create some dir and clone project
```
mkdir ~/my_pr
cd ~/my_pr
BRANCH=OpenHarmony_feature_20241108
git clone https://gitee.com/openharmony/arkcompiler_runtime_core.git -b ${BRANCH} runtime_core
git clone https://gitee.com/openharmony/arkcompiler_ets_frontend.git -b ${BRANCH} ets_frontend
```
build image with name test-panda-im
```
docker build -t test-panda-im -f runtime_core/static_core/scripts/sdk/docker/Dockerfile .
```
```
$ docker images 
REPOSITORY                                                           TAG       IMAGE ID       CREATED          SIZE
test-panda-im                                                        latest    d5bb1289b83d   13 minutes ago   10.5GB
```
## run container with volume 
command with env transfer 
```
docker run -it --rm --name panda-sdk-con --env-file=runtime_core/static_core/scripts/sdk/docker/panda.env -v $(pwd):/arkcompiler -v ~/.gitconfig:/etc/gitconfig test-panda-im:latest
```
example
```
$ docker run -it --rm --name panda-sdk-con --env-file=runtime_core/static_core/scripts/sdk/docker/panda.env -v $(pwd):/arkcompiler -v ~/.gitconfig:/etc/gitconfig est-panda-im:latest 
root@d9d6080aba0f:/arkcompiler# ll
total 16
drwxrwxr-x  4 1000 1000 4096 Mar  5 16:45 ./
drwxr-xr-x  1 root root 4096 Mar  5 16:59 ../
drwxrwxr-x 14 1000 1000 4096 Mar  5 16:46 ets_frontend/
drwxrwxr-x 33 1000 1000 4096 Mar  5 16:49 runtime_core/
```
or setup env in container
```
export PANDA_SRC=$(pwd)/runtime_core/static_core
export PANDA_SDK=$(pwd)/build_ark_release
export PANDA_BUILD=${PANDA_SDK}/linux_host_tools
export OHOS_SDK=/opt/ohos-sdk
export OHOS_SDK_NATIVE=$OHOS_SDK/native
export BUILD_DIR=$(pwd)
export PATH=$PATH:$OHOS_SDK/toolchains
```
start build in container 
```
ln -s $(realpath ./ets_frontend/ets2panda) $(realpath ./runtime_core/static_core/tools/es2panda)
./runtime_core/static_core/scripts/sdk/build_sdk.sh build_ark_release \
    --build_type=Release \
    --ohos_arm64 \
    --ets_std_lib \
    --linux_tools \
    --linux_arm64_tools
```


