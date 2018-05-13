#include <stdio.h>
#include <stdlib.h>
#include "info_out.h"

int main(int argc,char* argv[])
{
    char *strPath;
    void *head;
    char exitf;
    while (1)
    {
        if (2 == argc)
        {
            strPath = argv[1];
        }
        else
        {
            puts("Please input the .wav file name\n");
            scanf("%s", strPath);
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

