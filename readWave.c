#include <stdio.h>
#include <stdlib.h>
#include "info_out.h"
#define MAXPATH 128

int main(int argc,char* argv[])
{
    char strPath[MAXPATH];
    void *head = NULL;
    char exitf = ' ';
    while (1)
    {
        if (2 == argc && (strlen(argv[1])+1 <= MAXPATH))
        {
            sprintf(strPath,"%s",argv[1]);                
        }
        else
        {
            fflush(stdin);
        	puts("Please input the .wav file name\n");
        	scanf("%[^\n]", strPath);
            fflush(stdin);
            strPath[MAXPATH-1] = '\0';
        }
        head = readWaveHead(strPath);
        if (head != NULL)
        {
            printWaveHead(head);
            free(head);
        }
        putchar('\n');
        printf("EXIT?(Y/N)");
        scanf("%s", &exitf);
        if (exitf == 'y'||exitf == 'Y')
            break;
    }
    return 0;
}

