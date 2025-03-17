module language/llgo/compiler

go 1.22.0

require (
	language/gogen v1.16.6
	language/llgo v0.9.9
	language/llgo/runtime v0.0.0-00010101000000-000000000000
	language/llvm v0.8.1
	language/mod v0.13.16
	github.com/qiniu/x v1.13.11
	golang.org/x/tools v0.29.0
)

require (
	golang.org/x/mod v0.22.0 // indirect
	golang.org/x/sync v0.10.0 // indirect
)

replace language/llgo => ../

replace language/llgo/runtime => ../runtime
