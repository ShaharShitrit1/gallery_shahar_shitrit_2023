﻿#include "AlbumManager.h"
#include <iostream>
#include "Constants.h"
#include "MyException.h"
#include "AlbumNotOpenException.h"
#include <windows.h>

#define MSPAINT_PATH "C:\\Users\\test0\\AppData\\Local\\Microsoft\\WindowsApps\\mspaint.exe"	
#define IRFANV_PATH "C:\\Program Files\\IrfanView\\i_view64.exe"
#define MSPAINT "1"
#define IRFANV "2"

PROCESS_INFORMATION pi = { 0 };

// Handler function to close the process when the user types Ctrl+C
BOOL CtrlHandler(DWORD fdwCtrlType) 
{
	switch (fdwCtrlType) 
	{
	case CTRL_C_EVENT:
		TerminateProcess(pi.hProcess, 0);
		return TRUE;
	default:
		return FALSE;
	}
}

AlbumManager::AlbumManager(IDataAccess& dataAccess) :
    m_dataAccess(dataAccess), m_nextPictureId(100), m_nextUserId(200)
{
	// Left empty
	m_dataAccess.open();
}

void AlbumManager::executeCommand(CommandType command) {
	try {
		AlbumManager::handler_func_t handler = m_commands.at(command);
		(this->*handler)();
	} catch (const std::out_of_range&) {
			throw MyException("Error: Invalid command[" + std::to_string(command) + "]\n");
	}
}

void AlbumManager::printHelp() const
{
	std::cout << "Supported Album commands:" << std::endl;
	std::cout << "*************************" << std::endl;
	
	for (const struct CommandGroup& group : m_prompts) {
		std::cout << group.title << std::endl;
		std::string space(".  ");
		for (const struct CommandPrompt& command : group.commands) {
			space = command.type < 10 ? ".   " : ".  ";

			std::cout << command.type << space << command.prompt << std::endl;
		}
		std::cout << std::endl;
	}
}


// ******************* Album ******************* 
void AlbumManager::createAlbum()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: Can't create album since there is no user with id [" + userIdStr+"]\n");
	}

	std::string name = getInputFromConsole("Enter album name - ");
	if ( m_dataAccess.doesAlbumExists(name,userId) ) {
		throw MyException("Error: Failed to create album, album with the same name already exists\n");
	}

	Album newAlbum(userId,name);
	m_dataAccess.createAlbum(newAlbum);

	std::cout << "Album [" << newAlbum.getName() << "] created successfully by user@" << newAlbum.getOwnerId() << std::endl;
}

void AlbumManager::openAlbum()
{
	if (isCurrentAlbumSet()) {
		closeAlbum();
	}

	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: Can't open album since there is no user with id @" + userIdStr + ".\n");
	}

	std::string name = getInputFromConsole("Enter album name - ");
	if ( !m_dataAccess.doesAlbumExists(name, userId) ) {
		throw MyException("Error: Failed to open album, since there is no album with name:"+name +".\n");
	}

	m_openAlbum = m_dataAccess.openAlbum(name);
    m_currentAlbumName = name;
	// success
	std::cout << "Album [" << name << "] opened successfully." << std::endl;
}

void AlbumManager::closeAlbum()
{
	refreshOpenAlbum();

	std::cout << "Album [" << m_openAlbum.getName() << "] closed successfully." << std::endl;
	m_dataAccess.closeAlbum(m_openAlbum);
	m_currentAlbumName = "";
}

void AlbumManager::deleteAlbum()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if (!m_dataAccess.doesUserExists(userId)) {
		throw MyException("Error: There is no user with id @" + userIdStr +"\n");
	}

	std::string albumName = getInputFromConsole("Enter album name - ");
	if ( !m_dataAccess.doesAlbumExists(albumName, userId) ) {
		throw MyException("Error: Failed to delete album, since there is no album with name:" + albumName + ".\n");
	}

	// album exist, close album if it is opened
	if ( (isCurrentAlbumSet() ) &&
		 (m_openAlbum.getOwnerId() == userId && m_openAlbum.getName() == albumName) ) {

		closeAlbum();
	}

	m_dataAccess.deleteAlbum(albumName, userId);
	std::cout << "Album [" << albumName << "] @"<< userId <<" deleted successfully." << std::endl;
}

void AlbumManager::listAlbums()
{
	m_dataAccess.printAlbums();
}

void AlbumManager::listAlbumsOfUser()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if (!m_dataAccess.doesUserExists(userId)) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}

	const User& user = m_dataAccess.getUser(userId);
	const std::list<Album>& albums = m_dataAccess.getAlbumsOfUser(user);

	std::cout << "Albums list of user@" << user.getId() << ":" << std::endl;
	std::cout << "-----------------------" << std::endl;

	for (const auto& album : albums) {
		std::cout <<"   + [" << album.getName() <<"] - created on "<< album.getCreationDate() << std::endl;
	}
}


// ******************* Picture ******************* 
void AlbumManager::addPictureToAlbum()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if (m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: Failed to add picture, picture with the same name already exists.\n");
	}
	
	Picture picture(++m_nextPictureId, picName);
	std::string picPath = getInputFromConsole("Enter picture path: ");
	picture.setPath(picPath);

	m_dataAccess.addPictureToAlbumByName(m_openAlbum.getName(), picture);

	std::cout << "Picture [" << picture.getId() << "] successfully added to Album [" << m_openAlbum.getName() << "]." << std::endl;
}

void AlbumManager::removePictureFromAlbum()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	
	auto picture = m_openAlbum.getPicture(picName);
	m_dataAccess.removePictureFromAlbumByName(m_openAlbum.getName(), picture.getName());
	std::cout << "Picture <" << picName << "> successfully removed from Album [" << m_openAlbum.getName() << "]." << std::endl;
}

void AlbumManager::listPicturesInAlbum()
{
	refreshOpenAlbum();

	std::cout << "List of pictures in Album [" << m_openAlbum.getName() 
			  << "] of user@" << m_openAlbum.getOwnerId() <<":" << std::endl;
	
	const std::list<Picture>& albumPictures = m_openAlbum.getPictures();
	for (auto iter = albumPictures.begin(); iter != albumPictures.end(); ++iter) {
		std::cout << "   + Picture [" << iter->getId() << "] - " << iter->getName() << 
			"\tLocation: [" << iter->getPath() << "]\tCreation Date: [" <<
				iter->getCreationDate() << "]\tTags: [" << iter->getTagsCount() << "]" << std::endl;
	}
	std::cout << std::endl;
}

void AlbumManager::showPictureInProgram(Picture pic, std::string programPath)
{
	std::string picPathStr = " " + pic.getPath();
	PSTR picPath = (PSTR)(picPathStr.c_str());

	// Start IrfanView with the picture file as an argument
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = true;
	pi = { 0 };
	if (!CreateProcess(programPath.c_str(), picPath, NULL, NULL, FALSE, 0, NULL, NULL, (LPSTARTUPINFOA)&si, &pi))
	{
		MessageBoxW(NULL, L"Failed to start IrfanView.", L"Error", MB_OK | MB_ICONERROR);
	}
	else
	{
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close the process and thread handles
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}

void AlbumManager::showPicture()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	
	auto pic = m_openAlbum.getPicture(picName);
	if ( !fileExistsOnDisk(pic.getPath()) ) {
		throw MyException("Error: Can't open <" + picName+ "> since it doesnt exist on disk.\n");
	}

	std::cout << "1 <-> paint\n2 <-> irfanview" << std::endl;
	std::string procNum = getInputFromConsole("Enter (1/2): ");
	if (procNum == MSPAINT)
	{
		std::cout << "\nThe picture is now showing in Paint\nENTER Ctrl+C if you want to close the photo(make sure you in the console)" << std::endl;
		showPictureInProgram(pic, MSPAINT_PATH);
	}
	else if (procNum == IRFANV)
	{
		std::cout << "\nThe picture is now showing in Irfanview\nENTER Ctrl+C if you want to close the photo(make sure you in the console)" << std::endl;
		showPictureInProgram(pic, IRFANV_PATH);
	}
	else
	{
		std::cout << "Error: Wrong Input" << std::endl;
	}
}

void setFilePermission(LPCSTR filepath, int mode)
{
	DWORD fileAttributes = GetFileAttributes(filepath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) 
	{
		std::cerr << "Failed to get file attributes." << std::endl;
		return;
	}

	if (mode == 1) 
	{
		// set the file to read-only
		fileAttributes |= FILE_ATTRIBUTE_READONLY;
	}
	else if (mode == 2) 
	{
		// set the file to read and write by removing read-only
		fileAttributes &= ~FILE_ATTRIBUTE_READONLY;
	}
	else 
	{
		std::cerr << "Invalid mode argument.{Wrong Input}" << std::endl;
		return;
	}

	if (!SetFileAttributes(filepath, fileAttributes)) 
	{
		std::cerr << "Failed to set file attributes." << std::endl;
		return;
	}

	std::cout << "File permission changed successfully." << std::endl;
}

void AlbumManager::changePicPermission()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if (!m_openAlbum.doesPictureExists(picName)) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}

	auto pic = m_openAlbum.getPicture(picName);
	if (!fileExistsOnDisk(pic.getPath())) {
		throw MyException("Error: Can't open <" + picName + "> since it doesnt exist on disk.\n");
	}
	
	std::cout << "1 <-> read-only\n2 <-> read and write" << std::endl;
	std::string mode = getInputFromConsole("Enter (1/2): ");
	setFilePermission(pic.getPath().c_str(), stoi(mode));
}

void AlbumManager::tagUserInPicture()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	
	Picture pic = m_openAlbum.getPicture(picName);
	
	std::string userIdStr = getInputFromConsole("Enter user id to tag: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}
	User user = m_dataAccess.getUser(userId); 

	m_dataAccess.tagUserInPicture(m_openAlbum.getName(), pic.getName(), user.getId());
	std::cout << "User @" << userIdStr << " successfully tagged in picture <" << pic.getName() << "> in album [" << m_openAlbum.getName() << "]" << std::endl;
}

void AlbumManager::untagUserInPicture()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if (!m_openAlbum.doesPictureExists(picName)) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}

	Picture pic = m_openAlbum.getPicture(picName);

	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = stoi(userIdStr);
	if (!m_dataAccess.doesUserExists(userId)) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}
	User user = m_dataAccess.getUser(userId);

	if (! pic.isUserTagged(user)) {
		throw MyException("Error: The user was not tagged! \n");
	}

	m_dataAccess.untagUserInPicture(m_openAlbum.getName(), pic.getName(), user.getId());
	std::cout << "User @" << userIdStr << " successfully untagged in picture <" << pic.getName() << "> in album [" << m_openAlbum.getName() << "]" << std::endl;

}

void AlbumManager::listUserTags()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	auto pic = m_openAlbum.getPicture(picName); 

	const std::set<int> users = pic.getUserTags();

	if ( 0 == users.size() )  {
		throw MyException("Error: There is no user tagged in <" + picName + ">.\n");
	}

	std::cout << "Tagged users in picture <" << picName << ">:" << std::endl;
	for (const int user_id: users) {
		const User user = m_dataAccess.getUser(user_id);
		std::cout << user << std::endl;
	}
	std::cout << std::endl;

}


// ******************* User ******************* 
void AlbumManager::addUser()
{
	std::string name = getInputFromConsole("Enter user name: ");

	User user(++m_nextUserId,name);
	
	m_dataAccess.createUser(user);
	std::cout << "User " << name << " with id @" << user.getId() << " created successfully." << std::endl;
}

void AlbumManager::removeUser()
{
	// get user name
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}
	const User& user = m_dataAccess.getUser(userId);
	if (isCurrentAlbumSet() && userId == m_openAlbum.getOwnerId()) {
		closeAlbum();
	}

	m_dataAccess.deleteUser(user);
	std::cout << "User @" << userId << " deleted successfully." << std::endl;
}

void AlbumManager::listUsers()
{
	m_dataAccess.printUsers();	
}

void AlbumManager::userStatistics()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}

	const User& user = m_dataAccess.getUser(userId);

	std::cout << "user @" << userId << " Statistics:" << std::endl << "--------------------" << std::endl <<
		"  + Count of Albums User Owns: " << m_dataAccess.countAlbumsOwnedOfUser(user) << std::endl <<
		"  + Count of Albums Tagged: " << m_dataAccess.countAlbumsTaggedOfUser(user) << std::endl <<
		"  + Count of Tags: " << m_dataAccess.countTagsOfUser(user) << std::endl <<
		"  + Avarage Tags per Album: " << m_dataAccess.averageTagsPerAlbumOfUser(user) << std::endl;
}


// ******************* Queries ******************* 
void AlbumManager::topTaggedUser()
{
	const User& user = m_dataAccess.getTopTaggedUser();

	std::cout << "The top tagged user is: " << user.getName() << std::endl;
}

void AlbumManager::topTaggedPicture()
{
	const Picture& picture = m_dataAccess.getTopTaggedPicture();

	std::cout << "The top tagged picture is: " << picture.getName() << std::endl;
}

void AlbumManager::picturesTaggedUser()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}

	auto user = m_dataAccess.getUser(userId);

	auto taggedPictures = m_dataAccess.getTaggedPicturesOfUser(user);

	std::cout << "List of pictures that User@" << user.getId() << " tagged :" << std::endl;
	for (const Picture& picture: taggedPictures) {
		std::cout <<"   + "<< picture << std::endl;
	}
	std::cout << std::endl;
}


// ******************* Help & exit ******************* 
void AlbumManager::exit()
{
	std::exit(EXIT_SUCCESS);
}

void AlbumManager::help()
{
	system("CLS");
	printHelp();
}

void AlbumManager::helpCreate()
{
	m_dataAccess.addTwoUsers();
}

std::string AlbumManager::getInputFromConsole(const std::string& message)
{
	std::string input;
	do {
		std::cout << message;
		std::getline(std::cin, input);
	} while (input.empty());
	
	return input;
}

bool AlbumManager::fileExistsOnDisk(const std::string& filename)
{
	struct stat buffer;   
	return (stat(filename.c_str(), &buffer) == 0); 
}

void AlbumManager::refreshOpenAlbum() {
	if (!isCurrentAlbumSet()) {
		throw AlbumNotOpenException();
	}
    m_openAlbum = m_dataAccess.openAlbum(m_currentAlbumName);
}

bool AlbumManager::isCurrentAlbumSet() const
{
    return !m_currentAlbumName.empty();
}

const std::vector<struct CommandGroup> AlbumManager::m_prompts  = {
	{
		"Supported Albums Operations:\n----------------------------",
		{
			{ CREATE_ALBUM        , "Create album" },
			{ OPEN_ALBUM          , "Open album" },
			{ CLOSE_ALBUM         , "Close album" },
			{ DELETE_ALBUM        , "Delete album" },
			{ LIST_ALBUMS         , "List albums" },
			{ LIST_ALBUMS_OF_USER , "List albums of user" }
		}
	},
	{
		"Supported Album commands (when specific album is open):",
		{
			{ ADD_PICTURE    , "Add picture." },
			{ REMOVE_PICTURE , "Remove picture." },
			{ SHOW_PICTURE   , "Show picture." },
			{ CHANGE_PICTURE_PERMISSION   , "Change picture premission." },
			{ LIST_PICTURES  , "List pictures." },
			{ TAG_USER		 , "Tag user." },
			{ UNTAG_USER	 , "Untag user." },
			{ LIST_TAGS		 , "List tags." }
		}
	},
	{
		"Supported Users commands: ",
		{
			{ ADD_USER         , "Add user." },
			{ REMOVE_USER      , "Remove user." },
			{ LIST_OF_USER     , "List of users." },
			{ USER_STATISTICS  , "User statistics." },
		}
	},
	{
		"Supported Queries:",
		{
			{ TOP_TAGGED_USER      , "Top tagged user." },
			{ TOP_TAGGED_PICTURE   , "Top tagged picture." },
			{ PICTURES_TAGGED_USER , "Pictures tagged user." },
		}
	},
	{
		"Supported Operations:",
		{
			{ HELP , "Help (clean screen)" },
			{ CREATE_HELP , "Help (Open Two new users)" },
			{ EXIT , "Exit." },
		}
	}
};

const std::map<CommandType, AlbumManager::handler_func_t> AlbumManager::m_commands = {
	{ CREATE_ALBUM, &AlbumManager::createAlbum },
	{ OPEN_ALBUM, &AlbumManager::openAlbum },
	{ CLOSE_ALBUM, &AlbumManager::closeAlbum },
	{ DELETE_ALBUM, &AlbumManager::deleteAlbum },
	{ LIST_ALBUMS, &AlbumManager::listAlbums },
	{ LIST_ALBUMS_OF_USER, &AlbumManager::listAlbumsOfUser },
	{ ADD_PICTURE, &AlbumManager::addPictureToAlbum },
	{ REMOVE_PICTURE, &AlbumManager::removePictureFromAlbum },
	{ LIST_PICTURES, &AlbumManager::listPicturesInAlbum },
	{ SHOW_PICTURE, &AlbumManager::showPicture },
	{ CHANGE_PICTURE_PERMISSION, &AlbumManager::changePicPermission },
	{ TAG_USER, &AlbumManager::tagUserInPicture, },
	{ UNTAG_USER, &AlbumManager::untagUserInPicture },
	{ LIST_TAGS, &AlbumManager::listUserTags },
	{ ADD_USER, &AlbumManager::addUser },
	{ REMOVE_USER, &AlbumManager::removeUser },
	{ LIST_OF_USER, &AlbumManager::listUsers },
	{ USER_STATISTICS, &AlbumManager::userStatistics },
	{ TOP_TAGGED_USER, &AlbumManager::topTaggedUser },
	{ TOP_TAGGED_PICTURE, &AlbumManager::topTaggedPicture },
	{ PICTURES_TAGGED_USER, &AlbumManager::picturesTaggedUser },
	{ HELP, &AlbumManager::help },
	{ CREATE_HELP, &AlbumManager::helpCreate },
	{ EXIT, &AlbumManager::exit }
};