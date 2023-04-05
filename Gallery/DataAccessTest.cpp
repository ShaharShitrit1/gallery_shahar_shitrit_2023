#include "DataAccessTest.h"
#include "ItemNotFoundException.h"

bool DataAccessTest::opendb()
{
    return DatabaseAccess::open();
}

void DataAccessTest::AddThreeUsers()
{
    User user1(1, "shahar");
    User user2(2, "my femily");
    User user3(3, "hello");

    if (!DatabaseAccess::doesUserExists(user1.getId())) { DatabaseAccess::createUser(user1); }
    if (!DatabaseAccess::doesUserExists(user2.getId())) { DatabaseAccess::createUser(user2); }
    if (!DatabaseAccess::doesUserExists(user3.getId())) { DatabaseAccess::createUser(user3); }
}

void DataAccessTest::UpdateOneUser()
{
    string sqlStatement = "UPDATE USERS SET NAME = 'My Family' WHERE NAME = 'my femily';";

    char* errMessage = nullptr;
    int res = sqlite3_exec(DatabaseAccess::getDB(), sqlStatement.c_str(), nullptr, nullptr, &errMessage);
    if (res != SQLITE_OK) {
        std::cout << errMessage << std::endl;
        throw MyException(errMessage);
    }
}

void DataAccessTest::DeleteOneUser()
{
    User user3(3, "hello");
    DatabaseAccess::deleteUser(user3);
}
