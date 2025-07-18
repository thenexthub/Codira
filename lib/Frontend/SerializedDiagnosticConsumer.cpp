//===--- SerializedDiagnosticConsumer.cpp - Serialize Diagnostics ---------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements the SerializedDiagnosticConsumer class.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/SerializedDiagnostics.h"

#include "language/Frontend/SerializedDiagnosticConsumer.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceManager.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Parse/Lexer.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Bitstream/BitstreamWriter.h"

using namespace language;
using namespace clang::serialized_diags;

namespace {
class AbbreviationMap {
  toolchain::DenseMap<unsigned, unsigned> Abbrevs;
public:
  AbbreviationMap() {}

  void set(unsigned recordID, unsigned abbrevID) {
    assert(Abbrevs.find(recordID) == Abbrevs.end()
           && "Abbreviation already set.");
    Abbrevs[recordID] = abbrevID;
  }

  unsigned get(unsigned recordID) {
    assert(Abbrevs.find(recordID) != Abbrevs.end() &&
           "Abbreviation not set.");
    return Abbrevs[recordID];
  }
};

using RecordData = SmallVector<uint64_t, 64>;
using RecordDataImpl = SmallVectorImpl<uint64_t>;

struct SharedState : toolchain::RefCountedBase<SharedState> {
  SharedState(StringRef serializedDiagnosticsPath)
      : Stream(Buffer),
        SerializedDiagnosticsPath(serializedDiagnosticsPath),
        EmittedAnyDiagBlocks(false) {}

  /// The byte buffer for the serialized content.
  toolchain::SmallString<1024> Buffer;

  /// The BitStreamWriter for the serialized diagnostics.
  toolchain::BitstreamWriter Stream;

  /// The path of the diagnostics file.
  std::string SerializedDiagnosticsPath;

  /// The set of constructed record abbreviations.
  AbbreviationMap Abbrevs;

  /// A utility buffer for constructing record content.
  RecordData Record;

  /// A text buffer for rendering diagnostic text.
  toolchain::SmallString<256> diagBuf;

  /// The collection of files used.
  toolchain::DenseMap<const char *, unsigned> Files;

  /// The collection of categories used.
  toolchain::DenseMap<const char *, unsigned> Categories;

  /// The collection of flags used.
  toolchain::StringMap<unsigned> Flags;

  /// Whether we have already started emission of any DIAG blocks. Once
  /// this becomes \c true, we never close a DIAG block until we know that we're
  /// starting another one or we're done.
  bool EmittedAnyDiagBlocks;
};

/// Diagnostic consumer that serializes diagnostics to a stream.
class SerializedDiagnosticConsumer : public DiagnosticConsumer {
  /// State shared among the various clones of this diagnostic consumer.
  toolchain::IntrusiveRefCntPtr<SharedState> State;
  bool EmitMacroExpansionFiles = false;
  bool CalledFinishProcessing = false;
  bool CompilationWasComplete = true;

public:
  SerializedDiagnosticConsumer(StringRef serializedDiagnosticsPath,
                               bool emitMacroExpansionFiles)
      : State(new SharedState(serializedDiagnosticsPath)),
        EmitMacroExpansionFiles(emitMacroExpansionFiles) {
    emitPreamble();
  }

  ~SerializedDiagnosticConsumer() {
    assert(CalledFinishProcessing && "did not call finishProcessing()");
  }

  bool finishProcessing() override {
    assert(!CalledFinishProcessing &&
           "called finishProcessing() multiple times");
    CalledFinishProcessing = true;

    // NOTE: clang also does check for shared instances.  We don't
    // have these yet in Codira, but if we do we need to add an extra
    // check here.

    // Finish off any diagnostic we were in the process of emitting.
    if (State->EmittedAnyDiagBlocks)
      exitDiagBlock();

    // Write the generated bitstream to the file.
    std::error_code EC;
    std::unique_ptr<toolchain::raw_fd_ostream> OS;
    OS.reset(new toolchain::raw_fd_ostream(State->SerializedDiagnosticsPath, EC,
                                      toolchain::sys::fs::OF_None));
    if (EC) {
      // Create a temporary diagnostics engine to print the error to stderr.
      SourceManager dummyMgr;
      DiagnosticEngine DE(dummyMgr);
      PrintingDiagnosticConsumer PDC;
      DE.addConsumer(PDC);
      DE.diagnose(SourceLoc(), diag::cannot_open_serialized_file,
                  State->SerializedDiagnosticsPath, EC.message());
      return true;
    }

    if (CompilationWasComplete) {
      OS->write((char *)&State->Buffer.front(), State->Buffer.size());
      OS->flush();
    }
    return false;
  }

  /// In batch mode, if any error occurs, no primaries can be compiled.
  /// Some primaries will have errors in their diagnostics files and so
  /// a client (such as Xcode) can see that those primaries failed.
  /// Other primaries will have no errors in their diagnostics files.
  /// In order for the driver to distinguish the two cases without parsing
  /// the diagnostics, the frontend emits a truncated diagnostics file
  /// for the latter case.
  /// The unfortunate aspect is that the truncation discards warnings, etc.
  
  void informDriverOfIncompleteBatchModeCompilation() override {
    CompilationWasComplete = false;
  }

  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info) override;

private:
  /// Emit bitcode for the preamble.
  void emitPreamble();

  /// Emit bitcode for the BlockInfoBlock (part of the preamble).
  void emitBlockInfoBlock();

  /// Emit bitcode for metadata block (part of preamble).
  void emitMetaBlock();

  /// Emit bitcode to enter a block for a diagnostic.
  void enterDiagBlock() {
    State->Stream.EnterSubblock(BLOCK_DIAG, 4);
  }

  /// Emit bitcode to exit a block for a diagnostic.
  void exitDiagBlock() {
    State->Stream.ExitBlock();
  }

  // Record identifier for the file.
  unsigned getEmitFile(
      SourceManager &SM, StringRef Filename, unsigned bufferID
  );

  // Record identifier for the category.
  unsigned getEmitCategory(StringRef Category, StringRef CategoryURL);

  /// Emit a flag record that contains the documentation URL associated with
  /// a diagnostic or `0` if there is none.
  ///
  /// \returns a flag record identifier that could be embedded in
  /// other records.
  unsigned emitDocumentationURL(const DiagnosticInfo &info);

  /// Add a source location to a record.
  void addLocToRecord(SourceLoc Loc,
                      SourceManager &SM,
                      StringRef Filename,
                      RecordDataImpl &Record);

  void addRangeToRecord(CharSourceRange Range, SourceManager &SM,
                        StringRef Filename, RecordDataImpl &Record);

  /// Emit the message payload of a diagnostic to bitcode.
  void emitDiagnosticMessage(SourceManager &SM, SourceLoc Loc,
                             DiagnosticKind Kind,
                             StringRef Text, const DiagnosticInfo &Info);
};
} // end anonymous namespace

namespace language {
namespace serialized_diagnostics {
  std::unique_ptr<DiagnosticConsumer> createConsumer(
      StringRef outputPath, bool emitMacroExpansionFiles
  ) {
    return std::make_unique<SerializedDiagnosticConsumer>(
        outputPath, emitMacroExpansionFiles);
  }
} // namespace serialized_diagnostics
} // namespace language

/// Sanitize a filename for the purposes of the serialized diagnostics reader.
static StringRef sanitizeFilename(
    StringRef filename, SmallString<32> &scratch) {
  if (!filename.ends_with("/") && !filename.ends_with("\\"))
    return filename;

  scratch = filename;
  scratch += "_operator";
  return scratch;
}

unsigned SerializedDiagnosticConsumer::getEmitFile(
    SourceManager &SM, StringRef Filename, unsigned bufferID
) {
  // FIXME: Using Filename.data() here is wrong, since the provided
  // SourceManager may not live as long as this consumer (which is
  // the case if it's a diagnostic produced from building a module
  // interface). We ought to switch over to using a StringMap once
  // buffer names are unique (currently not the case for
  // pretty-printed decl buffers).
  unsigned &existingEntry = State->Files[Filename.data()];
  if (existingEntry)
    return existingEntry;

  // Lazily generate the record for the file.  Note that in
  // practice we only expect there to be one file, but this is
  // general and is what the diagnostic file expects.
  unsigned entry = State->Files.size();
  existingEntry = entry;
  RecordData Record;
  Record.push_back(RECORD_FILENAME);
  Record.push_back(entry);
  Record.push_back(0); // For legacy.
  Record.push_back(0); // For legacy.
  
  // Sanitize the filename enough that the serialized diagnostics reader won't
  // reject it.
  SmallString<32> filenameScratch;
  auto sanitizedFilename = sanitizeFilename(Filename, filenameScratch);
  Record.push_back(sanitizedFilename.size());
  State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_FILENAME),
                                   Record, sanitizedFilename.data());

  // If the buffer contains code that was synthesized by the compiler,
  // emit the contents of the buffer.
  auto generatedInfo = SM.getGeneratedSourceInfo(bufferID);
  if (!generatedInfo)
    return entry;

  Record.clear();
  Record.push_back(RECORD_SOURCE_FILE_CONTENTS);
  Record.push_back(entry);

  // The source range that this buffer was generated from, expressed as
  // offsets into the original buffer.
  if (generatedInfo->originalSourceRange.isValid()) {
    auto originalFilename = SM.getDisplayNameForLoc(generatedInfo->originalSourceRange.getStart(),
                                EmitMacroExpansionFiles);
    addRangeToRecord(
        generatedInfo->originalSourceRange,
        SM, originalFilename, Record
    );
  } else {
    addLocToRecord(SourceLoc(), SM, "", Record); // Start
    addLocToRecord(SourceLoc(), SM, "", Record); // End
  }

  // Contents of the buffer.
  auto sourceText = SM.getEntireTextForBuffer(bufferID);
  Record.push_back(sourceText.size());
  State->Stream.EmitRecordWithBlob(
      State->Abbrevs.get(RECORD_SOURCE_FILE_CONTENTS),
      Record, sourceText);

  return entry;
}

unsigned SerializedDiagnosticConsumer::getEmitCategory(
    StringRef Category, StringRef CategoryURL
) {
  unsigned &entry = State->Categories[Category.data()];
  if (entry)
    return entry;

  // Lazily generate the record for the category.
  entry = State->Categories.size();
  RecordData Record;
  Record.push_back(RECORD_CATEGORY);
  Record.push_back(entry);
  Record.push_back(Category.size());

  if (CategoryURL.empty()) {
    State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_CATEGORY),
                                     Record, Category);
  } else {
    std::string encodedCategory = (Category + "@" + CategoryURL).str();
    State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_CATEGORY),
                                     Record, encodedCategory);
  }

  return entry;
}

unsigned
SerializedDiagnosticConsumer::emitDocumentationURL(const DiagnosticInfo &Info) {
  if (Info.CategoryDocumentationURL.empty())
    return 0;

  unsigned &recordID = State->Flags[Info.CategoryDocumentationURL];
  if (recordID)
    return recordID;

  recordID = State->Flags.size();

  RecordData Record;
  Record.push_back(RECORD_DIAG_FLAG);
  Record.push_back(recordID);
  Record.push_back(Info.CategoryDocumentationURL.size());
  State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_DIAG_FLAG), Record,
                                   Info.CategoryDocumentationURL);
  return recordID;
}

void SerializedDiagnosticConsumer::addLocToRecord(SourceLoc Loc,
                                                  SourceManager &SM,
                                                  StringRef Filename,
                                                  RecordDataImpl &Record) {
  if (!Loc.isValid()) {
    // Emit a "sentinel" location.
    Record.push_back((unsigned)0); // File.
    Record.push_back((unsigned)0); // Line.
    Record.push_back((unsigned)0); // Column.
    Record.push_back((unsigned)0); // Offset.
    return;
  }

  auto bufferId = SM.findBufferContainingLoc(Loc);
  unsigned line, col;
  std::tie(line, col) = SM.getPresumedLineAndColumnForLoc(Loc);

  Record.push_back(getEmitFile(SM, Filename, bufferId));
  Record.push_back(line);
  Record.push_back(col);
  Record.push_back(SM.getLocOffsetInBuffer(Loc, bufferId));
}

void SerializedDiagnosticConsumer::addRangeToRecord(CharSourceRange Range,
                                                    SourceManager &SM,
                                                    StringRef Filename,
                                                    RecordDataImpl &Record) {
  assert(Range.isValid());
  addLocToRecord(Range.getStart(), SM, Filename, Record);
  addLocToRecord(Range.getEnd(), SM, Filename, Record);
}

/// Map a Codira DiagnosticKind to the diagnostic level expected
/// for serialized diagnostics.
static clang::serialized_diags::Level getDiagnosticLevel(DiagnosticKind Kind) {
  switch (Kind) {
  case DiagnosticKind::Error:
    return clang::serialized_diags::Error;
  case DiagnosticKind::Note:
    return clang::serialized_diags::Note;
  case DiagnosticKind::Warning:
    return clang::serialized_diags::Warning;
  case DiagnosticKind::Remark:
    return clang::serialized_diags::Remark;
  }

  toolchain_unreachable("Unhandled DiagnosticKind in switch.");
}

void SerializedDiagnosticConsumer::emitPreamble() {
  State->Stream.Emit((unsigned)'D', 8);
  State->Stream.Emit((unsigned)'I', 8);
  State->Stream.Emit((unsigned)'A', 8);
  State->Stream.Emit((unsigned)'G', 8);
  emitBlockInfoBlock();
  emitMetaBlock();
}


void SerializedDiagnosticConsumer::emitMetaBlock() {
  toolchain::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  Stream.EnterSubblock(BLOCK_META, 3);
  Record.clear();
  Record.push_back(RECORD_VERSION);
  Record.push_back(clang::serialized_diags::VersionNumber);
  Stream.EmitRecordWithAbbrev(Abbrevs.get(RECORD_VERSION), Record);
  Stream.ExitBlock();
}


/// Emits a block ID in the BLOCKINFO block.
static void emitBlockID(unsigned ID, const char *Name,
                        toolchain::BitstreamWriter &Stream,
                        RecordDataImpl &Record) {
  Record.clear();
  Record.push_back(ID);
  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_SETBID, Record);

  // Emit the block name if present.
  if (Name == nullptr || Name[0] == 0)
    return;

  Record.clear();

  while (*Name)
    Record.push_back(*Name++);

  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_BLOCKNAME, Record);
}

/// Emits a record ID in the BLOCKINFO block.
static void emitRecordID(unsigned ID, const char *Name,
                         toolchain::BitstreamWriter &Stream,
                         RecordDataImpl &Record) {
  Record.clear();
  Record.push_back(ID);

  while (*Name)
    Record.push_back(*Name++);

  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_SETRECORDNAME, Record);
}

/// Emit bitcode for abbreviation for source locations.
static void
addSourceLocationAbbrev(std::shared_ptr<toolchain::BitCodeAbbrev> Abbrev) {
  using namespace toolchain;
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 5));    // File ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Line.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Column.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Offset;
}

/// Emit bitcode for abbreviation for source ranges.
static void
addRangeLocationAbbrev(std::shared_ptr<toolchain::BitCodeAbbrev> Abbrev) {
  addSourceLocationAbbrev(Abbrev);
  addSourceLocationAbbrev(Abbrev);
}

void SerializedDiagnosticConsumer::emitBlockInfoBlock() {
  State->Stream.EnterBlockInfoBlock();

  using namespace toolchain;
  toolchain::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  // ==---------------------------------------------------------------------==//
  // The subsequent records and Abbrevs are for the "Meta" block.
  // ==---------------------------------------------------------------------==//

  emitBlockID(BLOCK_META, "Meta", Stream, Record);
  emitRecordID(RECORD_VERSION, "Version", Stream, Record);
  auto Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_VERSION));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrevs.set(RECORD_VERSION, Stream.EmitBlockInfoAbbrev(BLOCK_META, Abbrev));

  // ==---------------------------------------------------------------------==//
  // The subsequent records and Abbrevs are for the "Diagnostic" block.
  // ==---------------------------------------------------------------------==//

  emitBlockID(BLOCK_DIAG, "Diag", Stream, Record);
  emitRecordID(RECORD_DIAG, "DiagInfo", Stream, Record);
  emitRecordID(RECORD_SOURCE_RANGE, "SrcRange", Stream, Record);
  emitRecordID(RECORD_CATEGORY, "CatName", Stream, Record);
  emitRecordID(RECORD_DIAG_FLAG, "DiagFlag", Stream, Record);
  emitRecordID(RECORD_FILENAME, "FileName", Stream, Record);
  emitRecordID(RECORD_FIXIT, "FixIt", Stream, Record);
  emitRecordID(
      RECORD_SOURCE_FILE_CONTENTS, "SourceFileContents", Stream, Record);

  // Emit abbreviation for RECORD_DIAG.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_DIAG));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));  // Diag level.
  addSourceLocationAbbrev(Abbrev);
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Category.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Mapped Diag ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Diagnostic text.
  Abbrevs.set(RECORD_DIAG, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));

  // Emit abbreviation for RECORD_CATEGORY.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_CATEGORY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Category ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 8));  // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));      // Category text.
  Abbrevs.set(RECORD_CATEGORY, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));

  // Emit abbreviation for RECORD_SOURCE_RANGE.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_SOURCE_RANGE));
  addRangeLocationAbbrev(Abbrev);
  Abbrevs.set(RECORD_SOURCE_RANGE,
              Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));

  // Emit the abbreviation for RECORD_DIAG_FLAG.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_DIAG_FLAG));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Mapped Diag ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Flag name text.
  Abbrevs.set(RECORD_DIAG_FLAG, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                           Abbrev));

  // Emit the abbreviation for RECORD_FILENAME.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_FILENAME));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 5)); // Mapped file ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Modification time.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File name text.
  Abbrevs.set(RECORD_FILENAME, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                          Abbrev));

  // Emit the abbreviation for RECORD_FIXIT.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_FIXIT));
  addRangeLocationAbbrev(Abbrev);
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));      // FixIt text.
  Abbrevs.set(RECORD_FIXIT, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                       Abbrev));

  // Emit the abbreviation for RECORD_SOURCE_FILE_CONTENTS.
  Abbrev = std::make_shared<BitCodeAbbrev>();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_SOURCE_FILE_CONTENTS));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 5)); // File ID.
  addRangeLocationAbbrev(Abbrev);
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 16)); // File size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File contents.
  Abbrevs.set(RECORD_SOURCE_FILE_CONTENTS,
              Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));

  Stream.ExitBlock();
}

void SerializedDiagnosticConsumer::
emitDiagnosticMessage(SourceManager &SM,
                      SourceLoc Loc,
                      DiagnosticKind Kind,
                      StringRef Text,
                      const DiagnosticInfo &Info) {

  // Emit the diagnostic to bitcode.
  toolchain::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  StringRef filename = "";
  if (Loc.isValid())
    filename = SM.getDisplayNameForLoc(Loc, EmitMacroExpansionFiles);

  // Emit the RECORD_DIAG record.
  Record.clear();
  Record.push_back(RECORD_DIAG);
  Record.push_back(getDiagnosticLevel(Kind));
  addLocToRecord(Loc, SM, filename, Record);

  // Emit the category.
  if (!Info.Category.empty()) {
    Record.push_back(
        getEmitCategory(Info.Category, Info.CategoryDocumentationURL));
  } else {
    Record.push_back(0);
  }

  // Use "flags" slot to emit the category documentation URL. If there is not
  // such URL, the `0` placeholder would be emitted instead.
  // FIXME: This is a kludge. The category documentation URL is part of the
  // category description now, and we will switch back to using the flag field
  // as intended once clients have had a chance to adopt the new place.
  Record.push_back(emitDocumentationURL(Info));

  // Emit the message.
  Record.push_back(Text.size());
  Stream.EmitRecordWithBlob(Abbrevs.get(RECORD_DIAG), Record, Text);

  // If the location is invalid, do not emit source ranges or fixits.
  if (Loc.isInvalid())
    return;

  // Emit source ranges.
  auto RangeAbbrev = State->Abbrevs.get(RECORD_SOURCE_RANGE);
  for (const auto &R : Info.Ranges) {
    if (R.isInvalid())
      continue;
    State->Record.clear();
    State->Record.push_back(RECORD_SOURCE_RANGE);
    addRangeToRecord(R, SM, filename, State->Record);
    State->Stream.EmitRecordWithAbbrev(RangeAbbrev, State->Record);
  }

  // Emit FixIts.
  auto FixItAbbrev = State->Abbrevs.get(RECORD_FIXIT);
  for (const auto &F : Info.FixIts) {
    if (F.getRange().isValid()) {
      State->Record.clear();
      State->Record.push_back(RECORD_FIXIT);
      addRangeToRecord(F.getRange(), SM, filename, State->Record);
      State->Record.push_back(F.getText().size());
      Stream.EmitRecordWithBlob(FixItAbbrev, Record, F.getText());
    }
  }
}

void SerializedDiagnosticConsumer::handleDiagnostic(
    SourceManager &SM, const DiagnosticInfo &Info) {

  // Enter the block for a non-note diagnostic immediately, rather
  // than waiting for beginDiagnostic, in case associated notes
  // are emitted before we get there.
  if (Info.Kind != DiagnosticKind::Note) {
    if (State->EmittedAnyDiagBlocks)
      exitDiagBlock();

    enterDiagBlock();
    State->EmittedAnyDiagBlocks = true;
  }

  // Special-case diagnostics with no location.
  // Make sure we bracket all notes as "sub-diagnostics".
  bool bracketDiagnostic = (Info.Kind == DiagnosticKind::Note);

  if (bracketDiagnostic)
    enterDiagBlock();

  // Actually substitute the diagnostic arguments into the diagnostic text.
  toolchain::SmallString<256> Text;
  {
    toolchain::raw_svector_ostream Out(Text);
    DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                           Info.FormatArgs);
  }

  emitDiagnosticMessage(SM, Info.Loc, Info.Kind, Text, Info);

  if (bracketDiagnostic)
    exitDiagBlock();
}
