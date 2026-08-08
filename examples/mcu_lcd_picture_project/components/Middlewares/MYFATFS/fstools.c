#include "fstools.h"

#define FILE_MAX_TYPE_NUM 7 /* 最多FILE_MAX_TYPE_NUM个大类 */
#define FILE_MAX_SUBT_NUM 7 /* 最多FILE_MAX_SUBT_NUM个小类 */

/* 支持的文件类型定义 */
static const char *FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM] = {
    {"BIN", " ", " ", " ", " ", " ", " "},              /* BIN文件 */
    {"LRC", " ", " ", " ", " ", " ", " "},              /* LRC文件 */
    {"NES", "SMS", " ", " ", " ", " ", " "},            /* NES/SMS文件 */
    {"TXT", "C", "H", " ", " ", " ", " "},              /* 文本文件 */
    {"WAV", "MP3", "OGG", "FLAC", "AAC", "WMA", "MID"}, /* 音乐文件 */
    {"DB", "BMP", "JPG", "JPEG", "GIF", "PNG", " "},    /* 图片文件 */
    {"AVI", " ", " ", " ", " ", " ", " "},              /* 视频文件 */
};

FATFS *fs[FF_VOLUMES];

uint8_t fstools_init(void)
{
    /* 逻辑磁盘工作区(在调用任何FATFS相关函数之前,必须先给fs申请内存) */
    uint8_t i;
    for (i = 0; i < FF_VOLUMES; i++)
    {
        fs[i] = (FATFS *)malloc(sizeof(FATFS));
        if (fs[i] == NULL)
        {
            /* 释放已经申请成功的工作区 */
            while (i > 0)
            {
                i--;
                free(fs[i]);
                fs[i] = NULL;
            }
            return 1; /* 失败 */
        }
    }
    return 0; /* 成功 */
}

/**
 * @brief       将小写字母转为大写字母,如果是数字,则保持不变.
 * @param       c : 要转换的字母
 * @retval      转换后的字母,大写
 */
uint8_t fstools_char_upper(uint8_t c)
{
    if (c < 'A')
        return c; /* 数字,保持不变. */

    if (c >= 'a')
    {
        return c - 0x20; /* 变为大写. */
    }
    else
    {
        return c; /* 大写,保持不变 */
    }
}

/**
 * @brief       获取文件的类型
 * @param       fname : 文件名
 * @retval      文件类型
 *   @arg       0XFF , 表示无法识别的文件类型编号.
 *   @arg       其他 , 高四位表示所属大类, 低四位表示所属小类.
 */
uint8_t fstools_file_type(char *fname)
{
    uint8_t tbuf[5];
    char *attr = 0; /* 后缀名 */
    uint8_t i = 0, j;

    while (i < 250)
    {
        i++;

        if (*fname == '\0')
            break; /* 偏移到了最后了. */

        fname++;
    }

    if (i == 250)
        return 0XFF; /* 错误的字符串. */

    for (i = 0; i < 5; i++) /* 得到后缀名 */
    {
        fname--;

        if (*fname == '.')
        {
            fname++;
            attr = fname;
            break;
        }
    }

    if (attr == 0)
        return 0XFF;

    strcpy((char *)tbuf, (const char *)attr); /* copy */

    for (i = 0; i < 4; i++)
        tbuf[i] = fstools_char_upper(tbuf[i]); /* 全部变为大写 */

    for (i = 0; i < FILE_MAX_TYPE_NUM; i++) /* 大类对比 */
    {
        for (j = 0; j < FILE_MAX_SUBT_NUM; j++) /* 子类对比 */
        {
            if (*FILE_TYPE_TBL[i][j] == 0)
                break; /* 此组已经没有可对比的成员了. */

            if (strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0) /* 找到了 */
            {
                return (i << 4) | j;
            }
        }
    }

    return 0XFF; /* 没找到 */
}

/**
 * @brief       得到path路径下,目标文件的总个数
 * @param       path : 路径
 * @retval      总有效文件数
 */
uint16_t fstools_get_file_count(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    FF_DIR tdir;
    FILINFO *tfileinfo;
    tfileinfo = (FILINFO *)malloc(sizeof(FILINFO));
    res = f_opendir(&tdir, (const TCHAR *)path);
    if (res == FR_OK && tfileinfo)
    {
        while (1)
        {
            res = f_readdir(&tdir, tfileinfo);
            if (res != FR_OK || tfileinfo->fname[0] == 0)
                break;

            res = fstools_file_type(tfileinfo->fname);
            if ((res & 0x0F) != 0x00)
            {
                rval++;
            }
        }
    }
    free(tfileinfo);
    return rval;
}


/**
 * @brief       转换
 * @param       fs:文件系统对象
 * @param       clst:转换
 * @retval      =0:扇区号，0:失败
 */
static LBA_t fstools_clst2sect(FATFS *fs, DWORD clst)
{
    clst -= 2;  /* Cluster number is origin from 2 */

    if (clst >= fs->n_fatent - 2)
    {
        return 0;   /* Is it invalid cluster number? */
    }

    return fs->database + (LBA_t)fs->csize * clst;  /* Start sector number of the cluster */
}

/**
 * @brief       把已打开目录的读指针直接跳到指定字节偏移 ofs，下次 f_readdir 就读到那条目录项
 * @param       dp:指向打开目录对象
 * @param       Offset:目录表的偏移量
 * @retval      FR_OK(0):成功，!=0:错误
 */
FRESULT fstools_dir_sdi(FF_DIR *dp, DWORD ofs)
{
    DWORD clst;
    FATFS *fs = dp->obj.fs;

    if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? 0x10000000 : 0x200000) || ofs % 32)
    {
        /* Check range of offset and alignment */
        return FR_INT_ERR;
    }

    dp->dptr = ofs;        /* Set current offset */
    clst = dp->obj.sclust; /* Table start cluster (0:root) */

    if (clst == 0 && fs->fs_type >= FS_FAT32)
    { /* Replace cluster# 0 with root cluster# */
        clst = (DWORD)fs->dirbase;

        if (FF_FS_EXFAT)
        {
            dp->obj.stat = 0;
        }
        /* exFAT: Root dir has an FAT chain */
    }

    if (clst == 0)
    { /* Static table (root-directory on the FAT volume) */
        if (ofs / 32 >= fs->n_rootdir)
        {
            return FR_INT_ERR; /* Is index out of range? */
        }

        dp->sect = fs->dirbase;
    }
    else
    {    /* Dynamic table (sub-directory or root-directory on the FAT32/exFAT volume) */
        dp->sect = fstools_clst2sect(fs, clst);
    }

    dp->clust = clst; /* Current cluster# */

    if (dp->sect == 0)
    {
        return FR_INT_ERR;
    }

    dp->sect += ofs / fs->ssize;           /* Sector# of the directory entry */
    dp->dir = fs->win + (ofs % fs->ssize); /* Pointer to the entry in the win[] */

    return FR_OK;
}