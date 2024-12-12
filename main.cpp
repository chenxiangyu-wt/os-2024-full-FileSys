#include <iostream>
#include <iomanip>
#include <cstdint>
#include "command_line.hpp"
#include "file_sys.hpp"
#include "globals.hpp"
#include "utils.hpp"

InodeHashTableEntry hinode[NHINO];
Directory dir;
File system_opened_file[SYSOPENFILE];
FileSystem fileSystem;
UserPassword pwd[PWDNUM];
UserContext user[USERNUM];
MemoryINode *cur_path_inode;

int user_id;
uint8_t disk[DISK_SIZE];
int main()
{
	int username;
	char password[16];
	user_id = -1;
	format();
	install();
	// printDisk();

	printf("Welcome to mini filesystem!\n");

	// while (user_id == -1)
	// {
	// 	printf("Login: ");
	// 	scanf("%d", &username);
	// 	printf("Password: ");
	// 	scanf("%s", password);
	// 	user_id = login(username, password);
	// }

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