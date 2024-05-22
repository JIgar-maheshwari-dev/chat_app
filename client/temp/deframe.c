//`&jigar&mahesh&`

#include <stdio.h>
#include <string.h>

#define FS "`"
#define MS "&"

#define START "`&"
#define END "&`"

char str[]="CONNECT: Jigar";

int main(){

	char *s = strtok(str,":");
	if(s != NULL)
		s = strtok(NULL," ");
	printf("[ %s ]\n",s);

}
