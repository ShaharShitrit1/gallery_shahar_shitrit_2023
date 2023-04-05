#pragma once
#include "DatabaseAccess.h"

class DataAccessTest : public DatabaseAccess
{
public:

	//c'tor and d'tor
	DataAccessTest() = default;
	~DataAccessTest() = default;

	//open new database
	bool opendb();

	//add three new users
	void AddThreeUsers();

	//update a user that entered wrong name
	void UpdateOneUser();

	//delete one of the users
	void DeleteOneUser();
};