#include <windows.h>
#include <string.h>
#include "wave.h"

#define MAXHEADSIZE 128 //文件读取缓存

/*错误ID设定*/
#define errID_UnKErr 0
#define errID_FileErr 1
#define errID_TypeErr 2
#define errID_DecodeErr 3
char *errStr[] =
    {
        "Unknown err",
        "File open failed",
        "File was not a WAVE file",
        "Can not decode format"};

int err(const int errID) //错误处理
{
    puts(errStr[errID]);
    return errID;
}

int chkID(const char id[4]) //检查CHUNKID
{
    int i = 0;
    int j = 0;
    int outID = -1;

    char okID[5][4] =
        {
            {'R', 'I', 'F', 'F'},
            {'W', 'A', 'V', 'E'},
            {'f', 'm', 't', ' '},
            {'f', 'a', 'c', 't'},
            {'d', 'a', 't', 'a'},
        };
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (okID[i][j] == id[j])
            {
                outID = i;
            }
            else
            {
                outID = -1;
                break;
            }
        }
        if (outID != -1)
            return outID;
    }
    return -1;
}

void *readWaveHead(const char *path) //文件头读取
{
    FILE *fp = NULL;
    BYTE *cache = NULL;
    void *waveHead = NULL;
    WAVE_RIFF_CHUNK *pHEAD = NULL;
    WAVE_FMT_CHUNK *pFMT = NULL;
    WAVE_FACT_CHUNK *pFACT = NULL;
    BYTE *pTemp;
    int HEAD_LEN = 0;

    fp = fopen(path, "rb");
    if (NULL == fp)
    {
        err(errID_FileErr);
        return NULL;
    }
    cache = calloc(MAXHEADSIZE, sizeof(char));
    fread(cache, sizeof(char), MAXHEADSIZE, fp);
    pHEAD = (WAVE_RIFF_CHUNK *)cache;
    pFMT = (WAVE_FMT_CHUNK *)(cache + sizeof(WAVE_RIFF_CHUNK));
    if ((chkID(pHEAD->strRiffID) != 0) || (chkID(pHEAD->strRiffType) != 1)) //文件头检查
    {
        err(errID_TypeErr);
        free(cache);
        fclose(fp);
        pHEAD = NULL;
        pFMT = NULL;
        return NULL;
    }

    HEAD_LEN = HEAD_LEN + sizeof(WAVE_RIFF_CHUNK) + pFMT->dwFmtSize + 8;

    pFACT = (WAVE_FACT_CHUNK *)(cache + HEAD_LEN);
    if (chkID(pFACT->strFactID) != 3) //FACT检查
    {
        if (4 == chkID(pFACT->strFactID)) //Not have FACT
        {
            pFACT = NULL;
        }
        else
        {
            err(errID_DecodeErr);
            // free(cache);
            // fclose(fp);
            // pHEAD = NULL;
            // pFMT = NULL;
            pFACT = NULL;
            // return NULL;
        }
        //pFACT = NULL;
    }
    else
    {
        HEAD_LEN = HEAD_LEN + pFACT->dwFactSize + 8 + sizeof(int *);
    }

    waveHead = calloc(HEAD_LEN + sizeof(int), sizeof(char));
    memcpy(waveHead, &HEAD_LEN, sizeof(int));

    if (HEAD_LEN > 38)
    {
        memcpy((int *)waveHead + 1, cache, 20 + pFMT->dwFmtSize + 8);
        pTemp = (char *)waveHead + sizeof(int);
        pTemp += (20 + pFMT->dwFmtSize);
        pTemp += (8 + sizeof(int *));
        memcpy(pTemp, (char *)cache + (20 + pFMT->dwFmtSize + 8), pFACT->dwFactSize);
        pTemp -= (8 + sizeof(int *));
        ((WAVE_FACT_CHUNK *)pTemp)->pFactData = (void *)(pTemp + (sizeof(int *) + 8));
    }
    else
    {
        memcpy((int *)waveHead + 1, cache, HEAD_LEN);
    }

    free(cache);
    cache = NULL;
    fclose(fp);
    pHEAD = NULL;
    pFMT = NULL;
    pFACT = NULL;
    pTemp = NULL;
    return waveHead;
}

int putnc(const int n, const char c) //字符重复输出
{
    int i;
    for (i = 0; i < n; i++)
    {
        putchar(c);
    }
    return n;
}

void readID(const char src[], char dts[], const int n) //读取chunkID
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        dts[i] = src[i];
    }
    dts[i] = '\0';
}

void getCodeFmtbyName(const DWORD tag, char *dst) //读取编码格式代码对应的编码名称
{
    int i = 0;
    for (i = 0; i < 104; i++)
    {
        if (tag == CODE_FORMAT_LIST[i].wFormatTag)
        {
            sprintf(dst, "%s", CODE_FORMAT_LIST[i].strName);
            return;
        }
    }
}

void labPrint(const char *labName, const char *labData, const unsigned short lablevel)
{
    short i = 0;
    unsigned short w = 0;
    unsigned short m = 0;
    unsigned short pn = 0;
    HANDLE hOut;
    WORD wOldatt;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

    struct
    {
        unsigned short tab;
        unsigned short bg;
        unsigned short nameStyle;
        unsigned short dataStyle;
    } styleList[] =
        {{1, 16, 31, 30},
         {2, 16, 128, 142},
         {2, 128, 112, 116},
         {2, 112, 96, 105}};

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hOut, &csbiInfo);
    wOldatt = csbiInfo.wAttributes;
    w = csbiInfo.dwSize.X - 24;
    putnc(8, ' ');
    for (i = 0; lablevel >= i; i++) //左边
    {
        SetConsoleTextAttribute(hOut, styleList[i].bg);
        putnc(styleList[i].tab, ' ');
        w -= (styleList[i].tab * 2);
    }
    SetConsoleTextAttribute(hOut, styleList[lablevel].nameStyle);
    if (labName != NULL)
        pn += printf("%s: ", labName);
    SetConsoleTextAttribute(hOut, styleList[lablevel].dataStyle);
    if (labData != NULL)
        pn += printf("%s", labData);
    if (pn < w)
    {
        putnc(w - pn, ' ');
    }
    for (i = lablevel; i >= 0; i--) //右边
    {
        SetConsoleTextAttribute(hOut, styleList[i].bg);
        putnc(styleList[i].tab, ' ');
    }
    SetConsoleTextAttribute(hOut, wOldatt);
    putchar('\n');
}
void labClose(const unsigned short level)
{
    short i;
    for (i = level - 1; i >= 0; i--)
    {
        labPrint(NULL, NULL, i);
    }
}

void hexPrint(const BYTE * data,const unsigned int len)
{
    int i;
    HANDLE hOut;
    WORD wOldatt;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hOut, &csbiInfo);
    wOldatt = csbiInfo.wAttributes;

    SetConsoleTextAttribute(hOut,100);
    puts("Offset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    for (i=0;i<len;i++)
	{
		if (i%16 == 0)
		{
            SetConsoleTextAttribute(hOut,100);
            if (0==i)
                printf("%08X",i);
            else
                printf("\n%08X",i);
            SetConsoleTextAttribute(hOut,14);
            printf("   ");
        }
		printf("%02X ",*(data+i));
	}
    printf("\n");
    SetConsoleTextAttribute(hOut,wOldatt);
}

int printWaveHead(const void *phead) //输出weav头信息
{
    WAVE_RIFF_CHUNK *pRiff = NULL;
    WAVE_FMT_CHUNK *pFmt = NULL;
    WAVE_FACT_CHUNK *pFact = NULL;
    int HEAD_LEN = 0;
    unsigned short i = 0;
    char strcahe[64];

    pRiff = (WAVE_RIFF_CHUNK *)((char *)phead + sizeof(int));
    pFmt = (WAVE_FMT_CHUNK *)((char *)pRiff + sizeof(WAVE_RIFF_CHUNK));
    HEAD_LEN = *(int *)phead;

    labPrint("Wav File Head", "", 0);
    readID(pRiff->strRiffID, strcahe, 4);
    labPrint(" I D ", strcahe, 1);
    sprintf(strcahe, "%lu bytes", pRiff->dwRiffSize);
    labPrint("Size", strcahe, 2);
    readID(pRiff->strRiffType, strcahe, 4);
    labPrint("Type", strcahe, 2);
    labClose(2);
    readID(pFmt->strFmtID, strcahe, 4);
    labPrint(" I D ", strcahe, 1);
    sprintf(strcahe, "%lu bytes", pFmt->dwFmtSize);
    labPrint("Size", strcahe, 2);
    getCodeFmtbyName(pFmt->wavFormat.wFormatTag, strcahe);
    labPrint("编码格式", strcahe, 2);
    sprintf(strcahe, "%d 通道", pFmt->wavFormat.wChannels);
    labPrint("通道数量", strcahe, 2);
    sprintf(strcahe, "%d 样本/秒", pFmt->wavFormat.dwSamplesPerSec);
    labPrint("  采样率", strcahe, 2);
    sprintf(strcahe, "%d 字节/秒", pFmt->wavFormat.dwAvgBytesPerSec);
    labPrint("平均码率", strcahe, 2);
    sprintf(
        strcahe, "%d 字节（采样精度：%d bit）",
        pFmt->wavFormat.wBlockAlign,
        pFmt->wavFormat.wBlockAlign * 8 / pFmt->wavFormat.wChannels);
    labPrint("样本大小", strcahe, 2);
    labClose(2);
    putchar('\n');

    if (HEAD_LEN > 38)
    {
        pFact = (WAVE_FACT_CHUNK *)((char *)pFmt + 8 + pFmt->dwFmtSize);
        for (i = 0; i < pFact->dwFactSize; i++)
        {
            printf("%c ", *((char *)(pFact->pFactData)) + i);
        }
        putchar('\n');
        hexPrint((BYTE *)(pFact->pFactData),pFact->dwFactSize);
        putchar('\n');
        // printf("%s\n", pFact->pFactData);
    }
    return HEAD_LEN;
}