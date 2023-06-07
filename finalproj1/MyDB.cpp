#include <assert.h>
#include <fstream>

#include "MyDB.h"
#include "Utils.h"


using namespace std;

int loadOrSaveDb(sqlite3 *pInMemory, const std::string& zFilename, bool saveToHDD) {
	int rc;                   /* Function return code */
	sqlite3 *pFile;           /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;  /* Backup object used to copy data */
	sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
	sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

	/* Open the database file identified by zFilename. Exit early if this fails
	** for any reason. */
	rc = sqlite3_open(zFilename.c_str(), &pFile);
	if (rc == SQLITE_OK) {

		/* If this is a 'load' operation (isSave==0), then data is copied
		** from the database file just opened to database pInMemory.
		** Otherwise, if this is a 'save' operation (isSave==1), then data
		** is copied from pInMemory to pFile.  Set the variables pFrom and
		** pTo accordingly. */
		pFrom = (saveToHDD ? pInMemory : pFile);
		pTo = (saveToHDD ? pFile : pInMemory);

		/* Set up the backup procedure to copy from the "main" database of
		** connection pFile to the main database of connection pInMemory.
		** If something goes wrong, pBackup will be set to NULL and an error
		** code and message left in connection pTo.
		**
		** If the backup object is successfully created, call backup_step()
		** to copy data from pFile to pInMemory. Then call backup_finish()
		** to release resources associated with the pBackup object.  If an
		** error occurred, then an error code and message will be left in
		** connection pTo. If no error occurred, then the error code belonging
		** to pTo is set to SQLITE_OK.
		*/
		pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
		if (pBackup) {
			sqlite3_backup_step(pBackup, -1);
			sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pTo);
	}

	/* Close the database connection opened on database file zFilename
	** and return the result of this function. */
	sqlite3_close(pFile);
	return rc;
}

static int MyDB_callback(void *pStuff, int argc, char **argv, char **azColName) {
	assert(pStuff);
	MyDB *p = reinterpret_cast<MyDB*>(pStuff);
	return p->Callback(argc, argv, azColName);
}


void MyDB::Init(const std::string & _dbFileName, bool& doesExist) {
	assert(pDB == nullptr);
	dbFileName = _dbFileName;

	if (sqlite3_open(":memory:", &pDB)) {
		DebugPrint("Cannot open DB:", dbFileName);
		assert(false);
	}
	doesExist = false;
	ifstream f(dbFileName.c_str());
	if (f.good()) {
		f.close();
		int rc = loadOrSaveDb(pDB, dbFileName.c_str(), false);
		if (rc != SQLITE_OK) {
			DebugPrint("SQL error: cannot load DB into memory - ", dbFileName);
			assert(false);
		}
		doesExist = true;
	}

}

bool MyDB::ExecQuery(const string& query)
{
	results.clear();
	char *zErrMsg = 0;
	int rc = sqlite3_exec(pDB, query.c_str(), MyDB_callback, this, &zErrMsg);
	if (rc != SQLITE_OK) {
		DebugPrint("SQL error: ", zErrMsg);
		sqlite3_free(zErrMsg);
		assert(false);
		return false;
	}
	return true;
}

void MyDB::SaveToDisk() 
{
	assert(pDB && !dbFileName.empty());
	int rc = loadOrSaveDb(pDB, dbFileName.c_str(), true);
	if (rc != SQLITE_OK) {
		DebugPrint("SQL error: cannot save DB to disk - ", dbFileName);
		assert(false);
	}
}

void MyDB::Close() 
{
	sqlite3_close(pDB);
	pDB = nullptr;
}

int MyDB::Callback(int argc, char **argv, char **azColName) {
	Row row;
	for (int i = 0; i < argc; i++) {
		row.push_back(Field{ azColName[i], (argv[i] ? argv[i] : "NULL") });
	}
	results.push_back(row);
	return 0;
}

const string& MyDB::GetStr(int rowNum, const string& fieldName) {
	Row& r = results.at(rowNum);
	size_t id = 0;
	while (id < r.size() && r[id].name != fieldName) {
		++id;
	}
	assert(id < r.size());
	return r[id].value;
}

float MyDB::GetFloat(int rowNum, const string& fieldName) {
	string res = GetStr(rowNum, fieldName);
	return stof(res);
}
int MyDB::GetInt(int rowNum, const string& fieldName) {
	string res = GetStr(rowNum, fieldName);
	return stoi(res);
}
vector<string> MyDB::GetFieldNames(const string& table) {
	string sql = "SELECT * FROM " + table;
	vector<string> fields;
	sqlite3_stmt *res;
	int rc = sqlite3_prepare_v2(pDB, sql.c_str(), -1, &res, 0);
	if (rc == SQLITE_OK)
	{
		for (int i = 0; i < sqlite3_column_count(res); ++i)
			fields.push_back(sqlite3_column_name(res, i));
	}
	sqlite3_finalize(res);
	return fields;
}
