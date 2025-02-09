package llvm_test

import (
	"testing"

	"github.com/goplus/llvm"
)

// Dummy test function.
// All it does is test whether we can use LLVM at all.
func TestLLVM(t *testing.T) {
	t.Log("LLVM version:", llvm.Version)
}
