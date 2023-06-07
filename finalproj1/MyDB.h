#pragma once

#include <string>
#include <vector>

#include "..\..\sqlite\sqlite3.h"

/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
int loadOrSaveDb(sqlite3 *pInMemory, const std::string& zFilename, bool saveToHDD);

/*
Wrap the sqlite3 interface, give a more OOP flavour and
simplify its use.
*/
struct MyDB {
	sqlite3 *pDB = nullptr;	//main handle to database
	struct Field {
		std::string name;	//field name
		std::string value;	//field value - we don't know what type it is
	};
	typedef std::vector<Field> Row;	//one row of results
	std::vector<Row> results;		//all the rows returned from the last query
	std::string dbFileName;		//location of the database on HDD		

	//open the database, if it doesn't exist then make it
	void Init(const std::string& _dbFileName, bool& doesExist);
	//save the database to HDD
	void SaveToDisk();
	//called when we finish using the database
	void Close();
	//send an SQL query to the database
	bool ExecQuery(const std::string& query);

	//convert a particular row+field string to the target type
	const std::string& GetStr(int rowNum, const std::string& fieldName);
	float GetFloat(int rowNum, const std::string& fieldName);
	int GetInt(int rowNum, const std::string& fieldName);
	
	//get all the field names in a specific table
	std::vector<std::string> GetFieldNames(const std::string& table);
	//the callback is used to get results back from the database
	int Callback(int argc, char **argv, char **azColName);
};




