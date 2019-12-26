#ifndef STRING_EXT_H
#define STRING_EXT_H
typedef int(*cmp_string_ext)(const char *,const char *);

char *strbeq(char*,char*);
int   strsub(char*,char*);
char *strnsc(char*,char );
char *strnss(char*,char*);
int   strton(char*,int *);
char *strbri(int  ,int,char);
char *strbrs(char*,int,char);
char *strers(char*,int,char);
char *strtup(char*);
char *strtlo(char*);
void strQuickSort(char**,int,cmp_string_ext);
void strShellSort(char**,int,cmp_string_ext);
#endif
