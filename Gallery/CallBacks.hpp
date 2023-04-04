#pragma once
#include <string>
#include <iostream>
#include <list>
#include "Album.h"
#include "User.h"

using std::string;

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

int callbackUserName(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (string(azColName[i]) == "NAME")
		{
			data = argv[i];
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