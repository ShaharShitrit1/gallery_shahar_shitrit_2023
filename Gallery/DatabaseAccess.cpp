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

int callbackPictures(void* data, int argc, char** argv, char** azColName)
{
	string name;
	string location;
	string creation_date;
	int id = 0;

	for (int i = 0; i < argc; i++)
	{
		if (string(azColName[i]) == "NAME")
		{
			name = std::string(argv[i]);
		}
		else if (string(azColName[i]) == "ID")
		{
			id = atoi(argv[i]);
		}
		else if (string(azColName[i]) == "LOCATION")
		{
			location = std::string(argv[i]);
		}
		else if (string(azColName[i]) == "CREATION_DATE")
		{
			creation_date = std::string(argv[i]);
		}
	}

	Picture picture(id, name, location, creation_date);

	std::list<Picture>* pictures = (std::list<Picture>*)data;
	pictures->push_back(picture);

	return 0;
}

int callbackAlbums(void* data, int argc, char** argv, char** azColName)
{
	Album album;

	for (int i = 0; i < argc; i++)
	{
		if (string(azColName[i]) == "NAME")
		{
			album.setName(std::string(argv[i]));
		}
		else if (string(azColName[i]) == "CREATION_DATE")
		{
			album.setCreationDate(std::string(argv[i]));
		}
		else if (string(azColName[i]) == "USER_ID")
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
		if (string(azColName[i]) == "NAME")
		{
			album->setName(std::string(argv[i]));
		}
		else if (string(azColName[i]) == "CREATION_DATE")
		{
			album->setCreationDate(std::string(argv[i]));
		}
		else if (string(azColName[i]) == "USER_ID")
		{
			album->setOwner(atoi(argv[i]));
		}
	}

	return 0;
}

int callbackDoesExist(void* data, int argc, char** argv, char** azColName)
{
	string* str = (string*)data;

	str->assign(string(argv[0]));

	return 0;
}

int callbackPictureId(void* data, int argc, char** argv, char** azColName)
{
	int id;

	for (int i = 0; i < argc; i++)
	{
		if (string(azColName[i]) == "PICTURE_ID")
		{
			id = (int)argv[i];
		}
	}

	std::list<int>* ids = (std::list<int>*)data;
	ids->push_back(id);

	return 0;
}


//HELP
Picture DatabaseAccess::getPictureFromID(const int picture_id)
{
	string picture_name = "";
	string sqlStatement = "SELECT NAME FROM USERS WHERE PICTURE.ID = " + std::to_string(picture_id) + ";";
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackDoesExist, &picture_name, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	string location = "";
	sqlStatement = "SELECT LOCATION FROM USERS WHERE PICTURE.ID = " + std::to_string(picture_id) + ";";
	errMessage = nullptr;
	res = sqlite3_exec(db, sqlStatement.c_str(), callbackDoesExist, &location, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	string creation_date = "";
	sqlStatement = "SELECT CREATION_DATE FROM USERS WHERE PICTURE.ID = " + std::to_string(picture_id) + ";";
	errMessage = nullptr;
	res = sqlite3_exec(db, sqlStatement.c_str(), callbackDoesExist, &creation_date, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	Picture picture(picture_id, picture_name, location, creation_date);
	return picture;
}

sqlite3* DatabaseAccess::getDB()
{
	return this->db;
}

string DatabaseAccess::returnFirst(const string sqlStatement) const
{
	string callback;
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackDoesExist, &callback, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	return callback;
}

void DatabaseAccess::executeSQL(const string sqlStatement)
{
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}

Album DatabaseAccess::returnCallbackAlbum(const string sqlStatement)
{
	Album album;
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackAlbum, &album, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	return album;
}

std::list<Album> DatabaseAccess::returnCallbackAlbums(const string sqlStatement)
{
	std::list<Album> albums;
	char* errMessage = "";
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackAlbums, &albums, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
	return albums;
}

std::list<Picture> DatabaseAccess::returnCallbackPictures(const string sqlStatement)
{
	std::list<Picture> pictures;
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callbackPictures, &pictures, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}

	return pictures;
}

void DatabaseAccess::printSQL(const string sqlStatement)
{
	char* errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callback, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw MyException(errMessage);
	}
}


//Albums
const std::list<Album> DatabaseAccess::getAlbums()
{
	string sqlStatement = "SELECT * FROM ALBUMS;";
	return returnCallbackAlbums(sqlStatement);
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
	return returnCallbackAlbums(sqlStatement);
}

void DatabaseAccess::createAlbum(const Album& album)
{
	string sqlStatement = "SELECT * FROM USERS WHERE ID = " + std::to_string(album.getOwnerId()) + ";";
	string check = returnFirst(sqlStatement);
	
	if (check == "")
	{
		throw MyException("User id is incorrect!!");
	}
	else
	{
		sqlStatement = "INSERT INTO ALBUMS (NAME,CREATION_DATE,USER_ID) VALUES ('" + album.getName() + "', '" + album.getCreationDate() + "', " + std::to_string(album.getOwnerId()) + ");";
		executeSQL(sqlStatement);
	}	
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	//get album id
	string sqlStatement = "SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "' AND USER_ID = " + std::to_string(userId) + ";";
	string album_id = returnFirst(sqlStatement);

	if (album_id == "")
	{
		//delete the pictures in the album
		sqlStatement = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + std::to_string(stoi(album_id)) + ";";
		std::list<Picture> pictures = returnCallbackPictures(sqlStatement);

		//delete every picture in the album
		for (auto picture : pictures)
		{
			
			sqlStatement = "DELETE FROM PICTURES WHERE ID = " + std::to_string(picture.getId()) + "AND NAME = '" + picture.getName() + "';";
			executeSQL(sqlStatement);

			delete(&picture);
		}

		//delete the album
		sqlStatement = "DELETE FROM ALBUMS WHERE NAME = '" + albumName + "' AND USER_ID = " + std::to_string(userId) + ";";
		executeSQL(sqlStatement);
	}
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "' AND USER_ID = " + std::to_string(userId) + ";";
	string check = returnFirst(sqlStatement);
	
	if (check.size() < 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
	Album album = returnCallbackAlbum(sqlStatement);
	
	
	
	sqlStatement = "SELECT ID FROM ALBUMS WHERE USER_ID = " + std::to_string(album.getOwnerId()) + " AND NAME = '" + album.getName() + "';";
	string album_id = returnFirst(sqlStatement);

	if (album_id.size() >= 1)
	{	
		sqlStatement = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + std::to_string(stoi(album_id)) + ";";
		std::list<Picture> pictures = returnCallbackPictures(sqlStatement);

		for (auto picture : pictures)
		{
			album.addPicture(picture);
		}

		return album;
	}
	else
	{
		return Album();
	}
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	// basically here we would like to delete the allocated memory we got from openAlbum
}

void DatabaseAccess::printAlbums()
{
	string sqlStatement = "SELECT * FROM ALBUMS;";
	printSQL(sqlStatement);
}


//Pictures
void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
	string album_id = returnFirst(sqlStatement);

	if (album_id == "") { throw MyException("Album does not exist"); }

	sqlStatement = "INSERT INTO PICTURES (NAME,LOCATION,CREATION_DATE,ALBUM_ID) VALUES ('" + picture.getName() + "', '" + picture.getPath() + "', '" + picture.getCreationDate() + "', " + album_id + ");";
	executeSQL(sqlStatement);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
	string album_id = returnFirst(sqlStatement);

	if(album_id == ""){ throw MyException("Album does not exist"); }

	sqlStatement = "DELETE FROM PICTURES WHERE NAME = '" + pictureName + "' AND ALBUM_ID = " + album_id + ";";
	executeSQL(sqlStatement);
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	//check album id
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
	string album_id = returnFirst(sqlStatement);

	if (album_id == "") { throw MyException("Album does not exist"); }

	//check picture id
	sqlStatement = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + album_id + " AND NAME = '"+ pictureName + "';";
	string picture_id = returnFirst(sqlStatement);

	if (picture_id.size() < 1) { throw MyException("Picture does not exist"); }

	sqlStatement = "INSERT INTO TAGS (PICTURE_ID,USER_ID) VALUES (" + picture_id + ", " + std::to_string(userId) + ");";
	executeSQL(sqlStatement);

	openAlbum(albumName).tagUserInPicture(userId, pictureName);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	//check album id
	string sqlStatement = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
	string album_id = returnFirst(sqlStatement);

	if (album_id == "") { throw MyException("Album does not exist"); }

	//check picture id
	sqlStatement = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + album_id + " AND NAME = '" + pictureName + "';";
	string picture_id = returnFirst(sqlStatement);

	if (picture_id.size() < 1) { throw MyException("Picture does not exist"); }

	sqlStatement = "DELETE FROM TAGS WHERE TAGS.USER_ID = " + std::to_string(userId) + " AND TAGS.PICTURE_ID = " + picture_id + ";";
	executeSQL(sqlStatement);

	openAlbum(albumName).untagUserInPicture(userId, pictureName);
}


//Users
void DatabaseAccess::printUsers()
{
	string sqlStatement = "SELECT * FROM USERS;";
	printSQL(sqlStatement);
}

void DatabaseAccess::createUser(User& user)
{
	if (doesUserExists(user.getId()))
	{
		throw MyException("User Already Exist");
	}
	else
	{
		string sqlStatement = "INSERT INTO USERS VALUES (" + std::to_string(user.getId()) + ", '" + user.getName() + "');";
		executeSQL(sqlStatement);
	}
}

void DatabaseAccess::deleteUser(const User& user)
{
	if (doesUserExists(user.getId()))
	{
		//delete user from users
		string sqlStatement = "DELETE FROM USERS WHERE USERS.ID = " + std::to_string(user.getId()) + ";";
		executeSQL(sqlStatement);

		//delete all albums of the user
		std::list<Album> albums = getAlbumsOfUser(user);
		for (auto album : albums)
		{
			deleteAlbum(album.getName(), user.getId());

			delete(&album);
		}

		//delete user tags
		sqlStatement = "DELETE FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
		executeSQL(sqlStatement);
	}
	else
	{
		throw MyException("User Does Not Exist");
	}
}

bool DatabaseAccess::doesUserExists(int userId)
{
	string sqlStatement = "SELECT * FROM USERS WHERE ID = " + std::to_string(userId) + ";";
	string check = returnFirst(sqlStatement);

	if (check == "")
	{
		return false;
	}
	else
	{
		return true;
	}
}

User DatabaseAccess::getUser(int userId)
{
	string sqlStatement = "SELECT NAME FROM USERS WHERE ID = " + std::to_string(userId) + ";";
	string name = returnFirst(sqlStatement);

	if (name == "")
	{
		throw MyException("User Does Not Exist");
		return User(0,"0");
	}
	else
	{
		User user(userId, name);
		return user;
	}
}


//Statistics
int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	if (doesUserExists(user.getId()))
	{
		string sqlStatement = "SELECT ALBUMS.ID, COUNT(*) AS NUM FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
		string count = returnFirst(sqlStatement);

		return stoi(count);
	}
	else
	{
		throw MyException("User Does Not Exist");
	}

}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{	
	if (doesUserExists(user.getId()))
	{
		string sqlStatement = "SELECT TAGS.ID, COUNT(*) AS NUM FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
		string count = returnFirst(sqlStatement);

		return stoi(count);
	}
	else
	{
		throw MyException("User Does Not Exist");
	}
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	if (doesUserExists(user.getId()))
	{
		string sqlStatement = "SELECT TAGS.ID, COUNT(*) AS NUM FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
		string count = returnFirst(sqlStatement);

		return stoi(count);
	}
	else
	{
		throw MyException("User Does Not Exist");
	}
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albumsTaggedCount = countAlbumsTaggedOfUser(user);

	if (0 == albumsTaggedCount) {
		return 0;
	}

	return static_cast<float>(countTagsOfUser(user)) / albumsTaggedCount;
}


//Queries
User DatabaseAccess::getTopTaggedUser()
{
	string sqlStatement = "SELECT USER_ID FROM TAGS GROUP BY USER_ID ORDER BY COUNT(*) DESC LIMIT 1;";
	string user_id = returnFirst(sqlStatement);

	if (user_id != "")
	{
		sqlStatement = "SELECT NAME FROM USERS WHERE USERS.ID = " + std::to_string(stoi(user_id)) + ";";
		string user_name = returnFirst(sqlStatement);

		User user(stoi(user_id), user_name);
		return user;
	}
	else
	{
		User user(-999, "no one tagged");
		return user;
	}
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	string sqlStatement = "SELECT PICTURE_ID FROM TAGS GROUP BY PICTURE_ID ORDER BY COUNT(*) DESC LIMIT 1;";
	string picture_id = returnFirst(sqlStatement);

	if (picture_id != "")
	{
		return getPictureFromID(stoi(picture_id));
	}
	else
	{
		Picture picture(-999, "no one tagged");
		return picture;
	}
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	if (doesUserExists(user.getId()))
	{
		string sqlStatement = "SELECT PICTURE_ID FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";
		std::list<int> picturesId;
		char* errMessage = "";
		int res = sqlite3_exec(db, sqlStatement.c_str(), callbackPictureId, &picturesId, &errMessage);
		if (res != SQLITE_OK) {
			throw MyException(errMessage);
		}

		std::list<Picture> pictures;

		for (auto pictureId : picturesId)
		{
			pictures.push_back(getPictureFromID(pictureId));
		}
		return pictures;
	}
	return std::list<Picture>();
}