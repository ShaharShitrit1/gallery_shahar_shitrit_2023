#include "DatabaseAccess.h"
#include "ItemNotFoundException.h"

//Database
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
		sqlStatement = "CREATE TABLE USERS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL);";


		char* errMessage = nullptr;
		int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			throw MyException(errMessage);;
			return false;
		}

		sqlStatement = "CREATE TABLE ALBUMS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL,\nCREATION_DATE DATE NOT NULL,\nUSER_ID INTEGER NOT NULL,\nFOREIGN KEY (USER_ID) REFERENCES USERS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			throw MyException(errMessage);
			return false;
		}

		sqlStatement = "CREATE TABLE PICTURES(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\nNAME TEXT NOT NULL,\nLOCATION TEXT NOT NULL,\nCREATION_DATE DATE NOT NULL,\nALBUM_ID INTEGER NOT NULL,\nFOREIGN KEY (ALBUM_ID) REFERENCES ALBUMS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			throw MyException(errMessage);
			return false;
		}

		sqlStatement = "CREATE TABLE TAGS(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT NOT NULL,\nPICTURE_ID INTEGER NOT NULL,\nUSER_ID INTEGER NOT NULL,\nFOREIGN KEY (PICTURE_ID) REFERENCES PICTURES(ID),\nFOREIGN KEY (USER_ID) REFERENCES USERS(ID));";

		errMessage = nullptr;
		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK) {
			throw MyException(errMessage);
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

//call backs
int callback(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		//this if is to avoid "," at the end of each line
		if (i != argc - 1)
		{
			std::cout << azColName[i] << " = " << argv[i] << " , ";
		}
		else
		{
			std::cout << azColName[i] << " = " << argv[i];
		}
	}
	std::cout << std::endl;
	return 0;
}

int callbackAlbums(void* data, int argc, char** argv, char** azColName)
{
	Album album;

	for (int i = 0; i < argc; i++)
	{
		if (azColName[i] == "NAME")
		{
			album.setName(std::string(argv[i]));
		}
		else if (azColName[i] == "CREATION_DATE")
		{
			album.setCreationDate(std::string(argv[i]));
		}
		else if (azColName[i] == "USER_ID")
		{
			album.setOwner(atoi(argv[i]));
		}
	}

	std::list<Album>* albums = (std::list<Album>*)data;
	albums->push_back(album);

	return 0;
}

int callbackAlbum(void* data, int argc, char** argv, char** azColName)
{
	Album* album = (Album*)data;

	for (int i = 0; i < argc; i++)
	{
		if (azColName[i] == "NAME")
		{
			album->setName(std::string(argv[i]));
		}
		else if (azColName[i] == "CREATION_DATE")
		{
			album->setCreationDate(std::string(argv[i]));
		}
		else if (azColName[i] == "USER_ID")
		{
			album->setOwner(atoi(argv[i]));
		}
	}

	return 0;
}

int callbackUserName(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (azColName[i] == "NAME")
		{
			data = argv[i];
		}
	}

	return 0;
}

int callbackBool(void* data, int argc, char** argv, char** azColName)
{
	bool fun = true;

	for (int i = 0; i < argc; i++)
	{
		if (azColName[i] == "NAME")
		{
			if (argv[i] != "")
			{
				data = &fun;
			}
		}
	}

	return 0;
}

//Albums
const std::list<Album> DatabaseAccess::getAlbums()
{
	const char* sqlStatement = "SELECT * FROM ALBUMS;";
	std::list<Album> albums;
	char* errMessage = "";
	int res = sqlite3_exec(db, sqlStatement, callbackAlbums, &albums, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
	return albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
	std::list<Album> albums;
	char* errMessage = "";
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackAlbums, &albums, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
	return albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	string sqlStatement = "INSERT INTO ALBUMS (NAME,CREATION_DATE,USER_ID) VALUES ('" + album.getName() + "', '" + album.getCreationDate() + "', " + std::to_string(album.getOwnerId()) + ");";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	string sqlStatement;
	sqlStatement = "DELETE FROM ALBUMS WHERE NAME LIKE '%" + albumName + "%' AND USER_ID = " + std::to_string(userId) + ";";

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	bool check = false;
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME LIKE '%" + albumName + "%' AND USER_ID = " + std::to_string(userId) + ";";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackBool, &check, &errMessage);
	return check;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	Album album;
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME LIKE '%" + albumName + "%';";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackAlbum, &album, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
	
	return album;
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	delete(&pAlbum);
}

void DatabaseAccess::printAlbums()
{
	string sqlStatement = "SELECT * FROM ALBUMS;";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callback, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}


//Pictures
void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	string sqlStatement = "INSERT INTO ALBUMS (NAME,CREATION_DATE,USER_ID) VALUES ('" + picture.getName() + "', '" + picture.getCreationDate() + "', " + std::to_string(picture.getId()) + ");";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	string sqlStatement = "DELETE FROM PICTURES WHERE NAME = '" + pictureName + "' AND ALBUM ID = " + std::to_string(0) + ");";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	string sqlStatement;
	sqlStatement = "INSERT INTO TAGS (PICTURE_ID,USER_ID) VALUES (" + /*PICTURE ID*/std::to_string(0) + ", " + std::to_string(userId) + ");";

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	string sqlStatement;
	sqlStatement = "DELETE FROM TAGS WHERE TAGS.USER_ID = " + std::to_string(userId) + " AND TAGS.PICTURE_ID = " + /*PICTURE ID*/std::to_string(0) + ";";

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}


//Users
void DatabaseAccess::printUsers()
{
	string sqlStatement = "SELECT * FROM USERS;";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callback, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::createUser(User& user)
{
	string sqlStatement;
	sqlStatement = "INSERT INTO USERS VALUES (" + std::to_string(user.getId()) + ", '" + user.getName() + "');";

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

void DatabaseAccess::deleteUser(const User& user)
{
	string sqlStatement;
	sqlStatement = "DELETE FROM USERS WHERE USERS.ID = " + std::to_string(user.getId()) + ";";

	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

bool DatabaseAccess::doesUserExists(int userId)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE ID = " + std::to_string(userId) + ";";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		return false;
	}
	return true;
}

User DatabaseAccess::getUser(int userId)
{
	string name;
	string sqlStatement = "SELECT * FROM ALBUMS WHERE ID = " + std::to_string(userId) + ";";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackUserName, &name, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	User user(userId, name);

	return user;
}


//Statistics
int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	return 0;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	return 0;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	return 0;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	return 0.0f;
}


//Queries
User DatabaseAccess::getTopTaggedUser()
{
	User stam(0, "stam");
	return stam;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	Picture stam(0, "stam");
	return stam;
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	return std::list<Picture>();
}