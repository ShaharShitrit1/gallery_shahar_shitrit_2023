#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:

	//open new database
	bool open() override;

	//add three new users
	void AddThreeUsers();

	//update a user that entered wrong name
	void UpdateOneUser();

	//delete one of the users
	bool DeleteOneUser();

private:
	DatabaseAccess* dbAccess;
};