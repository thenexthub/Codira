#include "vm/core/DebugInfo/PDB/Native/NativeTypeTypedef.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/PDBExtras.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeTypeTypedef::NativeTypeTypedef(NativeSession &Session, SymIndexId Id,
                                     codeview::UDTSym Typedef)
    : NativeRawSymbol(Session, PDB_SymType::Typedef, Id),
      Record(std::move(Typedef)) {}

NativeTypeTypedef::~NativeTypeTypedef() = default;

void NativeTypeTypedef::dump(raw_ostream &OS, int Indent,
                             PdbSymbolIdField ShowIdFields,
                             PdbSymbolIdField RecurseIdFields) const {
  NativeRawSymbol::dump(OS, Indent, ShowIdFields, RecurseIdFields);
  dumpSymbolField(OS, "name", getName(), Indent);
  dumpSymbolIdField(OS, "typeId", getTypeId(), Indent, Session,
                    PdbSymbolIdField::Type, ShowIdFields, RecurseIdFields);
}

std::string NativeTypeTypedef::getName() const {
  return std::string(Record.Name);
}

SymIndexId NativeTypeTypedef::getTypeId() const {
  return Session.getSymbolCache().findSymbolByTypeIndex(Record.Type);
}
