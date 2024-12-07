#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.hpp"

void halt()
{
	int i, j;

	/*1. write back the current dir */
	chdir("..");
	iput(cur_path_inode);

	/*2. free the u_ofile and sys_ofile and inode*/
	for (i = 0; i < USERNUM; i++)
	{
		if (user[i].user_id != 0)
		{
			for (j = 0; j < NOFILE; j++)
			{
				if (user[i].open_files[j] != SYSOPENFILE + 1)
				{
					close(i, j);
					user[i].open_files[j] = SYSOPENFILE + 1;
				} // if
			} // for
		} // if
	} // for

	/*3. write back the filesys to the disk*/
	memcpy(disk + BLOCK_SIZE, &fileSystem, sizeof(FileSystem));

	/*4. close the file system column*/

	/*5. say GOOD BYE to all the user*/
	printf("\nGood Bye. See You Next Time. Please turn off the switch\n");
	exit(0);
}
