#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:

	//open new database
	bool opendb();

	//add three new users
	void AddThreeUsers();

	//update a user that entered wrong name
	void UpdateOneUser();

	//delete one of the users
	void DeleteOneUser();

private:
	DatabaseAccess* dbAccess;
};