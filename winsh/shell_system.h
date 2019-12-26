#ifndef SHELL_SYSTEM_C
#define SHELL_SYSTEM_C

// Funcion para obtener el usuario
char *Get_User();

// Funcion para usar directorios
char *Get_Current_Directory();
int Change_Directory(const char* new_dir);

// Funciones para las variables
char *Get_Env_Variable(const char *);
int Set_Env_Variable(const char *,const char *);
int Unset_Env_Variable(const char *);

// Funciones para los ficheros
char *Get_File_Attributes(char *);


#endif
