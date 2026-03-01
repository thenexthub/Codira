#include "vm/core/Plugins/PassPlugin.h"
#define HANDLE_EXTENSION(Ext)                                                  \
		toolchain::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "vm/core/Support/Extension.def"


namespace vm::core {
	namespace details {
		void extensions_anchor() {
#define HANDLE_EXTENSION(Ext)                                                  \
			get##Ext##PluginInfo();
#include "vm/core/Support/Extension.def"
		}
	}
}
