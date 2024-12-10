#ifndef LOG_H
#define LOG_H

#define LOGIN_SUCCESS 0
#define ERR_INCORRECT_PASSWORD -1
#define ERR_TOO_MANY_USERS -2

int find_user_index(uint_16 uid, const char *passwd);
int find_free_user_slot();

#endif // LOG_H