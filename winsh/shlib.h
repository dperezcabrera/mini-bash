#ifndef SHLIB_H
#define SHLIB_H

#define LAST_IS_NULL -1
#define ERROR_MESSAGE "ERROR"
#define WARNING "\a"

void Show_Error_Message(char*,char*, char*);
void Show_Warning();
void Delete_Array(int, char**);
char **Word_Options(int, char**);
char *Char_Options(int, char**);
char **Number_Options(int, char**);
char *First_Word_Comand(int,char**);

#endif
