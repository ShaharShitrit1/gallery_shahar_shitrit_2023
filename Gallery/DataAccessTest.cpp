#include "DataAccessTest.h"

bool DataAccessTest::open()
{
    return DatabaseAccess::open();
}

bool DataAccessTest::AddThreeUsers()
{
    User user1(1, "shahar");
    User user2(2, "my femily");
    User user3(3, "hello");

    DatabaseAccess::createUser(user1);
    DatabaseAccess::createUser(user2);
    DatabaseAccess::createUser(user3);
}
