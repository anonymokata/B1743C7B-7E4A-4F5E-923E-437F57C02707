#ifndef DRMRD_STRING_H
#define DRMRD_STRING_H
char *back_of_strcat(char *dest, char *src);
char *replace_substring(char *original, char *old_sub, char *new_sub);
char *replace_substring_and_realloc(char *original, char *old_sub, char *new_sub);
#endif /* DRMRD_STRING_H */
