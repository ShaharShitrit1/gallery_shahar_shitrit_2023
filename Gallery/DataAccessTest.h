#pragma once
#include "DatabaseAccess.h"

class DataAccessTest : public DatabaseAccess
{
public:

	//open new database
	bool open() override;

	//add three new users
	bool AddThreeUsers();

	//update a user that entered wrong name
	bool UpdateOneUser();

	//delete one of the users
	bool DeleteOneUser();


};