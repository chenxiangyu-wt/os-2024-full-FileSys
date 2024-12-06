#include <iostream>
#include "filesys.h"
#include "CommandLine.hpp"

InodeHashTableEntry hinode[NHINO];
Directory dir;
File sys_ofile[SYSOPENFILE];
FileSystem fileSystem;
UserPassword pwd[PWDNUM];
UserContext user[USERNUM];
MemoryINode *cur_path_inode;
int user_id;
char disk[(DINODEBLK + FILEBLK + 2) * BLOCKSIZ];
char str[100];

int main()
{
	int username;
	char password[16];
	user_id = -1;

	format();
	install();

	printf("Welcome to mini filesystem!\n");

	while (user_id == -1)
	{
		printf("Login: ");
		scanf("%d", &username);
		printf("Password: ");
		scanf("%s", password);
		user_id = login(username, password);
	}

	CommandLine commandLine;

	std::string input;
	do
	{
		std::cout << "> ";
		std::getline(std::cin, input);

	} while (commandLine.execute(input));

	logout(user_id);
	halt();

	return 0;
}