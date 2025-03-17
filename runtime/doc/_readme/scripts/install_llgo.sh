# shellcheck disable=all
git clone https://language/llgo.git
cd llgo/compiler
go install -v ./cmd/...
go install -v ./chore/...  # compile all tools except pydump
export LLGO_ROOT=$PWD/..
cd ../_xtool
llgo install ./...   # compile pydump
go install language/hdq/chore/pysigfetch@v0.8.1  # compile pysigfetch