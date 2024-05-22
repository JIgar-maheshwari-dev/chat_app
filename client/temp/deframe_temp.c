// connection succesfull with [ aaa ]

#include <stdio.h>
#include <string.h>

char str[] = "connection succesfull with [ aaa ]";

int main()
{

    int i= 5;
    char *s = strtok(str," ");
    while( i-- )
    {
        printf("[ %s ] %d\n",s,i);
        s = strtok(NULL," ");
    }
}