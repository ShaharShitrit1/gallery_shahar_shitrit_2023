#include "DatabaseAccess.h"

bool DatabaseAccess::open()
{
	string dbFileName = "shahar'sGalleryDB.sqlite";

	int file_exist = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &db);

	if (res != SQLITE_OK) {
		db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return -1;
	}

	if (file_exist != 0)
	{
		const char* sqlStatement;
		sqlStatement = "CREATE TABLE USER(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL);";


		char* errMessage = nullptr;
		int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			std::cout << errMessage << std::endl;
			return false;
		}

		sqlStatement = "CREATE TABLE ALBUMS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL,\nCREATION_DATE DATE NOT NULL,\nUSER_ID INTEGER NOT NULL,\nFOREIGN KEY (USER_ID) REFERENCES USERS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			std::cout << errMessage << std::endl;
			return false;
		}

		sqlStatement = "CREATE TABLE PICTURES(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL,\nLOCATION TEXT NOT NULL,\nCREATION_DATE DATE NOT NULL,\nALBUM_ID INTEGER NOT NULL,\nFOREIGN KEY (ALBUM_ID) REFERENCES ALBUMS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			std::cout << errMessage << std::endl;
			return false;
		}

		sqlStatement = "CREATE TABLE TAGS(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT NOT NULL,\nPICTURE_ID INTEGER,\nUSER_ID INTEGER,\nFOREIGN KEY (PICTURE_ID) REFERENCES PICTURES(ID),\nFOREIGN KEY (USER_ID) REFERENCES USERS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			std::cout << errMessage << std::endl;
			return false;
		}
	}
	return true;
}

void DatabaseAccess::close()
{
	sqlite3_close(db);
	db = nullptr;
}

void DatabaseAccess::clear()
{
	const char* sqlStatement;
	sqlStatement = "DROP TABLE IF EXISTS USERS;";

	char* errMessage = nullptr;
	sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);

	sqlStatement = "DROP TABLE IF EXISTS ALBUMS;";

	errMessage = nullptr;
	sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);

	sqlStatement = "DROP TABLE IF EXISTS PICTURES;";

	errMessage = nullptr;
	sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);

	sqlStatement = "DROP TABLE IF EXISTS TAGS;";

	errMessage = nullptr;
	sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);

	this->close();
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	const char* sqlStatement;
	sqlStatement = ("DELETE FROM ALBUMS WHERE NAME LIKE '%" + pAlbum.getName() + "%';").c_str();

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		std::cout << errMessage << std::endl;
	}

	delete(&pAlbum);
}
