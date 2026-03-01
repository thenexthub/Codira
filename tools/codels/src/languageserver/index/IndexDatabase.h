/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LSPSERVER_INDEX_INDEXDATABASE_H
#define LSPSERVER_INDEX_INDEXDATABASE_H

#include <forward_list>
#include <future>
#include <mutex>
#include "CallRelation.h"
#include "Ref.h"
#include "Relation.h"
#include "Symbol.h"
#include "../sql/wrapper/Connection.h"
#include "../sql/wrapper/Statement.h"
#include "Tokenizer.h"
#include "ThreadMemoize.h"

namespace ark {
namespace lsp {

using dberr_no = uint32_t;
constexpr uint32_t DIGEST_BYTES = 8;
using FileDigest = std::array<uint8_t, DIGEST_BYTES>;
constexpr bool RESULT_NEXT = true;
constexpr bool RESULT_DONE = false;
const unsigned int MUTI_INSERT_MAX_SIZE = 900;

class IndexDatabase {
    class DatabaseConnection;

public:
    struct Options {
        /**
         * The database will be opened in read-only mode.
         */
        bool openReadOnly = false;
        /**
         * The database will be opened as an in-memory database.
         */
        bool openInMemory = false;
    };

    IndexDatabase();

    ~IndexDatabase() {}

    dberr_no Initialize(std::function<sqldb::Connection()> openConnection);

    dberr_no FileExists(std::string fileURI, bool &exists);

    dberr_no GetFileDigest(uint32_t fileId, std::string &digest);

    dberr_no GetSymbols(std::function<bool(const Symbol &sym)> callback);

    dberr_no GetSymbolsByName(const std::string &name, std::function<bool(const Symbol &sym)> callback);

    dberr_no GetPkgSymbols(std::string pkgName, std::function<bool(const Symbol &sym)> callback);
    
    dberr_no GetSymbolByID(IDArray id, std::function<bool(const Symbol &sym)> callback);

    dberr_no GetCrossSymbolByID(IDArray id, std::function<void(const CrossSymbol &sym)> callback);

    dberr_no GetSymbol(std::string filePath, size_t line, size_t col,
                       std::function<bool(const Symbol &sym)> callback);

    dberr_no GetSymbolsAndCompletions(const std::string &prefix,
        std::function<void(const Symbol &sym, const CompletionItem &completion)> callback);

    dberr_no GetCompletion(std::string symPackage, const Symbol &sym,
        std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback);

    dberr_no GetExtendItem(IDArray id,
        std::function<void(const std::string &, const Symbol &, const ExtendItem &, const CompletionItem &)> callback);

    dberr_no GetComment(IDArray id, std::function<bool(const Comment &comment)> callback);

    dberr_no GetFileWithID(int fileId,
                           std::function<bool(std::string, std::string)> callback);

    dberr_no GetFileWithUri(std::string fileUri,
                            std::function<bool(std::string, std::string)> callback);

    dberr_no GetSourceFileRelations(std::string fileURI,
                                    std::function<bool(std::string)> callback);

    dberr_no GetHeaderFileRelations(std::string fileURI,
                                    std::function<bool(std::string)> callback);

    dberr_no GetMatchingSymbols(std::string query,
                                std::function<bool(const Symbol &sym)> callback,
                                std::optional<std::string> scope = std::nullopt,
                                std::optional<Symbol::SymbolFlag> flags = std::nullopt);

    dberr_no GetReferences(const SymbolID &id, RefKind kind,
                           std::function<bool(const Ref &ref)> callback);

    dberr_no GetFileReferences(const std::string &fileUri, RefKind kind,
         std::function<bool(const Ref &ref, const SymbolID symId)> callback);                       

    dberr_no GetReferred(const SymbolID &id,
                         std::function<void(const SymbolID &, const Ref &)> callback);

    dberr_no GetUsedSymbols(std::string fileURI,
                            std::set<SymbolID> &symbols);

    dberr_no GetReferenceAt(std::string fileURI, RefKind kind,
                            Position pos, SymbolID &id);

    dberr_no GetRelations(const SymbolID &subjectID, RelationKind kind,
                          std::function<bool(const Relation &rel)> callback);

    dberr_no GetCallRelations(const SymbolID &subjectID, CallRelationKind kind,
                              std::function<bool(const CallRelation &rel)> callback);

    dberr_no GetCrossSymbols(const std::string &pkgName, const std::string &symName,
        const std::function<void(const CrossSymbol &)> &callback);

    dberr_no GetRelationsRiddenUp(const SymbolID &objectID, RelationKind kind,
        std::function<bool(const Relation &rel)> callback);

    dberr_no GetRelationsRiddenDown(const SymbolID &subjectID, RelationKind kind,
                                  std::function<bool(const Relation &rel)> callback);

    template <typename T> dberr_no GetConfigValue(std::string name, std::optional<T> &value)
    {
        return UseSelectConfig().execute(sqldb::with(name), sqldb::into(value));
    }

    template <typename T> dberr_no SetConfigValue(std::string name, const T &value)
    {
        return UseInsertConfig().execute(sqldb::with(name, value));
    }

    class DBUpdate {
    public:
        explicit DBUpdate(DatabaseConnection &db) : db(db) {}

        /**
         * Insert file record into index database. If file with the same URI
         * already exists in the database then this function will update digest
         * of the existing file record.
         */
        dberr_no InsertFile(std::string fileURI, FileDigest digest);

        /**
         * Remove file record from index database.
         */
        dberr_no DeleteFile(const std::string &fileURI);

        /**
         * Reset Enumerated flag to false for all source files in index database.
         */
        dberr_no UpdateFilesNotEnumerated();

        /**
         * Set Enumerated flag to true for up-to-date file in index database.
         */
        dberr_no UpdateFileEnumerated(std::string fileURI);

        /**
         * Rename file
         */
        dberr_no RenameFile(std::pair<std::string, std::string> oldNewURI);

        /**
         * Insert files relation record.
         */

        /**
         * Insert new symbol into index database. If symbol with the same ID
         * already exists in the database then this function will use
         * mergeSymbol to merge both symbols and after that will replace
         * existing symbol record with the merged symbol.
         */
        dberr_no InsertSymbol(const Symbol &sym);

        void DealSymbols(const std::vector<Symbol> &syms);

        dberr_no InsertSymbols(const std::vector<Symbol> &syms);

        dberr_no InsertCompletion(const Symbol &sym, const CompletionItem &completionItem);

        void DealCompletions(const std::vector<std::pair<IDArray, CompletionItem>> &completions);

        dberr_no InsertCompletions(const std::vector<std::pair<IDArray, CompletionItem>> &completions);

        dberr_no InsertComment(const Symbol &sym, const AST::Comment &comment);

        void DealComments(const std::vector<std::pair<IDArray, Comment>> &comments);

        dberr_no InsertComments(const std::vector<std::pair<IDArray, Comment>> &comments);

        dberr_no InsertFileWithId(int fileID, std::vector<std::string> &fileInfo);

        /**
         * Insert new symbol reference into index database.
         */
        dberr_no InsertReference(const IDArray &id, const Ref &ref);

        void DealReferences(const std::vector<std::pair<IDArray, Ref>> &refs);

        dberr_no InsertReferences(const std::vector<std::pair<IDArray, Ref>> &refs);

        /**
         * Insert new relation into index database.
         */
        dberr_no InsertRelation(const IDArray &subject, RelationKind predicate, const IDArray &object);

        void DealRelations(const std::vector<Relation> &relations);

        dberr_no InsertRelations(const std::vector<Relation> &relations);

        /**
         * Insert new extends into index database.
         */
        dberr_no InsertExtend(const IDArray &extendId, const IDArray &id, Modifier modifier,
            const std::string &name, const std::string &curPkgName);

        dberr_no InsertExtends(const std::map<std::pair<std::string, SymbolID>, std::vector<ExtendItem>> &extends);

        /**
         * Insert new crosSymbols into index database.
         */
        dberr_no InsertCrossSymbol(const std::string &curPkgName, const CrossSymbol &crsSym);

        void DealCrossSymbols(const std::vector<std::pair<std::string, CrossSymbol>> &crsSyms);

        dberr_no InsertCrossSymbols(const std::vector<std::pair<std::string, CrossSymbol>> &crsSyms);

        template <typename... Ts>
        void BindValue(const sqldb::Bind<Ts...> &bind, sqlite3_stmt *stmt, int &Index)
        {
            sqldb::tuple_for_each(bind, [&](const auto &Value) mutable {
                    if (int RC = sqldb::traits::bind(stmt, ++Index, Value); RC != sqlite::OK)
                        Trace::Log(RC, "Failed to bind value to parameter at index " + std::to_string(Index));
            });
        }

        /**
         * Apply path mappings to paths inside index database.
         */

        /**
         * Convenience class method which converts dberr_noSuccess value
         * into dberr_no to resolve ambiguity of return type when using a
         * lambda expression as a callback for update method.
         */
        static dberr_no Done() { return 0; }

    private:
        DatabaseConnection &db;
    };

    /**
     * Start update operation(s) on index database.
     */
    dberr_no Update(std::function<dberr_no(DBUpdate)> callback);

    // Gather statistics about tables and indices and store the collected
    // information in internal tables of the database where the query optimizer
    // can access the information and use it to help make better query planning
    // choices.
    dberr_no AnalyzeDatabase();

    /**
     * Rebuild the database, repacking it into a minimal amount of disk space.
     */
    dberr_no CompactDatabase();

    /**
     * Attempt to free as much memory used by the underlying database library as
     * possible. Returns the number of bytes of heap memory actually freed.
     */
    static size_t ReleaseMemory();

    /**
     * Return number of bytes of heap memory currently allocated by the
     * underlying database library.
     */
    static size_t GetMemoryUsed();

    /**
     * Provide statement to remove source files for which Enumerated flag is
     * not set.
     * @return a statement that deletes batch of 10 file records with unset
     * enumerated flag
     */
    sqldb::Statement GetDeleteStaleFilesStatement();

    /**
     * Provide statement to remove source files for which Enumerated flag is
     * not set.
     * @return a statement that counts file records with unset enumerated flag
     */
    sqldb::Statement GetCountStaleFilesStatement();

    /**
     * Provide the number of rows modified
     * @return the number of modified rows
     */
    int GetChanges();

    ThreadMemoize<DatabaseConnection>& GetDatabaseCache()
    {
        return databaseCache;
    }

private:
    IndexDatabase(const IndexDatabase &) = delete;
    void operator=(const IndexDatabase &) = delete;

    dberr_no PopulateReferenceCount(Symbol &sym);

    class DatabaseConnection {
    public:
        DatabaseConnection() = default;
        explicit DatabaseConnection(sqldb::Connection sqlConnect)
            : connection(std::move(sqlConnect)) {}
        DatabaseConnection(DatabaseConnection &&) = default;
        DatabaseConnection &operator=(DatabaseConnection &&) = default;

        sqldb::Connection *operator->() { return &connection; }

        sqldb::Statement &Use(std::string_view SQL, bool useCacheStmt = true);

    private:
        sqldb::Connection connection;
        std::forward_list<sqldb::Statement> statementCache;
        using StatementLookupEntry = std::vector<sqldb::Statement *>;
        std::map<const char *, StatementLookupEntry> statementLookup;
    };
    std::mutex updateMutex;
    std::future<void> upgradeFuture;
    ThreadMemoize<DatabaseConnection> databaseCache;
    std::function<DatabaseConnection()> openDatabase;

    DatabaseConnection &Database() {return databaseCache.Get(openDatabase);}
    sqldb::Statement &Use(std::string_view sql) { return Database().Use(sql); }
    sqldb::Statement &UseSelectConfig();
    sqldb::Statement &UseInsertConfig();
};

dberr_no OpenIndexDatabase(IndexDatabase &db, const std::string &file,
                           IndexDatabase::Options opts = {});

} // namespace lsp
} // namespace ark

#endif // LSPSERVER_INDEX_INDEXDATABASE_H
