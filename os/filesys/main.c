#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCKSIZE 1024  // 磁盘块大小
#define SIZE 1024000  // 虚拟磁盘空间大小
#define END 65535  // FAT中的文件结束标志
#define FREE 0  // FAT中盘块空闲标志
#define ROOTBLOCKNUM 2  // 根目录区所占盘块数
#define MAXOPENFILE 10  // 最多同时打开文件个数t
#define MAXTEXT 10000

/* 文件控制块 */
typedef struct FCB{

    char filename[8];  // 文件名
    char exname[3];  // 文件扩展名
    unsigned char attribute;  // 文件属性字段，值为0时表示目录文件，值为1时表示数据文件
    unsigned short time;  // 文件创建时间
    unsigned short date;  // 文件创建日期
    unsigned short first;  // 文件起始盘块号
    unsigned long length;  // 文件长度
    char free;  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配
}fcb;

/* 文件分配表 */
typedef struct FAT
{
    unsigned short id;  // 磁盘块的状态（空闲的，最后的，下一个）
}fat;

/* 用户打开文件表 */
typedef struct USEROPEN
{
    char filename[8];  // 文件名
    char exname[3];  // 文件扩展名
    unsigned char attribute;//文件属性字段，值为0时表示目录文件，值为1时表示数据文件
    unsigned short time;  // 文件创建时间
    unsigned short date;  // 文件创建日期
    unsigned short first;  // 文件起始盘块号
    unsigned long length;//文件长度（对数据文件是字节数，对目录文件可以是目录项个数）
    char free;  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配

    unsigned short dirno;  // 相应打开文件的目录项在父目录文件中的盘块号
    int diroff;  // 相应打开文件的目录项在父目录文件的dirno盘块中的目录项序号
    char dir[80];  // 相应打开文件所在的路径名，这样方便快速检查出指定文件是否已经打开
    int count;  // 读写指针在文件中的位置,文件的总字符数
    char fcbstate;  // 是否修改了文件的FCB的内容，如果修改了置为1，否则为0
    char topenfile;  // 表示该用户打开表项是否为空，若值为0，表示为空，否则表示已被某打开文件占据
}useropen;

/* 引导块 */
typedef struct BLOCK0
{
    char magic[10];  // 文件系统魔数
    char information[200];//存储一些描述信息，如磁盘块大小、磁盘块数量、最多打开文件数等
    unsigned short root;  // 根目录文件的起始盘块号
    unsigned char *startblock;  // 虚拟磁盘上数据区开始位置
}block0;
unsigned char *myvhard;  // 指向虚拟磁盘的起始地址
useropen openfilelist[MAXOPENFILE];  // 用户打开文件表数组
int curdir;  // 用户打开文件表中的当前目录所在打开文件表项的位置
char currentdir[80];  // 记录当前目录的目录名（包括目录的路径）
unsigned char* startp;  // 记录虚拟磁盘上数据区开始位置
char myfilename[] = "myfilesys";//文件系统的文件名

void startsys();  // 进入文件系统
void my_format();  // 磁盘格式化
int FileSubstr(char *str);
void my_cd(char *dirname);  // 更改当前目录
void my_mkdir(char *dirname);  // 创建子目录
void my_rmdir(char *dirname);  // 删除子目录
void my_ls(char *filename);  // 显示目录
void my_create (char *filename);  // 创建文件
void my_rm(char *filename);  // 删除文件
int open(char *filename);
int my_open(char *filename);  // 打开文件
int open_path(char *dirname) ;
void my_close(int fd);  // 关闭文件
int my_write(int fd);  // 写文件
int do_write(int fd, char *text, int len, char wstyle);  // 实际写文件
int my_read (int fd, int len);  // 读文件
int do_read (int fd, int len,char *text);  // 实际读文件
void my_exitsys();  // 退出文件系统
unsigned short findblock();  // 寻找空闲盘块
int findopenfile();  // 寻找空闲文件表项
void startsys()
{
    FILE *fp;
    unsigned char buf[SIZE];
    fcb *root;
    int i;
    myvhard = (unsigned char *)malloc(SIZE);//申请虚拟磁盘空间
    memset(myvhard, 0, SIZE);//将myvhard中前SIZE个字节用 0 替换并返回 myvhard
    if((fp = fopen(myfilename, "r")) != NULL)
    {
        fread(buf, SIZE, 1, fp);//将二进制文件读取到缓冲区
        fclose(fp);
        if(strcmp(((block0 *)buf)->magic, "10101010"))
        {
            printf("myfilesys is not exist,begin to creat the file...\n");
            my_format();
        }
        else
        {
            for(i = 0; i < SIZE; i++)
                myvhard[i] = buf[i];
        }
    }
    else
    {
        printf("myfilesys is not exist,begin to creat the file...\n");
        my_format();
    }
    root = (fcb *)(myvhard + 5 * BLOCKSIZE);
    strcpy(openfilelist[0].filename, root->filename);
    strcpy(openfilelist[0].exname, root->exname);
    openfilelist[0].attribute = root->attribute;
    openfilelist[0].time = root->time;
    openfilelist[0].date = root->date;
    openfilelist[0].first = root->first;
    openfilelist[0].length = root->length;
    openfilelist[0].free = root->free;
    openfilelist[0].dirno = 5;
    openfilelist[0].diroff = 0;
    strcpy(openfilelist[0].dir, "/root/");
    openfilelist[0].count = 0;
    openfilelist[0].fcbstate = 0;
    openfilelist[0].topenfile = 1;
    for(i = 1; i < MAXOPENFILE; i++)
        openfilelist[i].topenfile = 0;
    curdir = 0;
    strcpy(currentdir, "/root/");
    startp = ((block0 *)myvhard)->startblock;
}


void my_format()
{
    FILE *fp;
    fat *fat1, *fat2;
    block0 *blk0;
    time_t now;
    struct tm *nowtime;
    fcb *root;
    int i;
    printf("my_format started\n");
   memset(myvhard,0,SIZE);
    blk0 = (block0 *)myvhard;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);
    root = (fcb *)(myvhard + 5 * BLOCKSIZE);
    strcpy(blk0->magic, "10101010");
    strcpy(blk0->information, "My FileSystem Ver 1.0 \n Blocksize=1KB Whole size=1000KB Blocknum=1000 RootBlocknum=2\n");
    blk0->root = 5;
    blk0->startblock = (unsigned char *)root;
    for(i = 0; i < 5; i++)
    {
        fat1->id = END;
        fat2->id = END;
        fat1++;
        fat2++;
    }
    fat1->id = 6;
    fat2->id = 6;
    fat1++;
    fat2++;
    fat1->id = END;
    fat2->id = END;
    fat1++;
    fat2++;
    for(i = 7; i < SIZE / BLOCKSIZE; i++)
    {
        fat1->id = FREE;
        fat2->id = FREE;
        fat1++;
        fat2++;
    }
    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(root->filename, ".");
    strcpy(root->exname, "");
    root->attribute = 1;
    root->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    root->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    root->first = 5;
    root->length = 2 * sizeof(fcb);
    root->free = 1;
    root++;
    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(root->filename, "..");
    strcpy(root->exname, "");
    root->attribute = 1;
    root->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    root->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    root->first = 5;
    root->length = 2 * sizeof(fcb);
    root->free = 1;
    fp = fopen(myfilename, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
    printf("my_format exited\n");
}

void my_cd(char *dirname)
{
    printf("my_cd %s\n",dirname);
    int fd=open_path(dirname);
    printf("my_cd open_path %d\n",fd);
    if(fd==-1)
    {
        printf("the %s error\n",dirname);
        return;
    }
    my_close(curdir);
    curdir=fd;
    strcpy(currentdir,openfilelist[curdir].dir);
    printf("my_cd curdir %d, currendir %s\n",curdir,currentdir);
    printf("my_cd exited\n");
}

void my_mkdir(char *dirname)
{
    fcb *fcbptr;
    fat *fat1, *fat2;
    time_t now;
    struct tm *nowtime;
    char text[MAXTEXT];
    unsigned short blkno;
    int rbn,fd=-1, i;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);
    printf("my_mkdir %s\n",dirname);
    int k=FileSubstr(dirname),tempfd=curdir;
    int flag=0;
    char dirpath[80];
    char newdir[20];
    if(k!=-1) {
        dirname[k] = 0;
        strcpy(newdir, dirname + k + 1);
        strcpy(dirpath, dirname);
        fd = open_path(dirpath);
        printf("mkdir open_path %d:\n",fd);
        if(fd!=-1) {
            curdir=fd;
            flag=1;
            strcpy(dirname,newdir);
        }
        else {
            printf("error\n");
            return ;
        }
    }
    printf("mkdir curdir:%d\n",curdir);
    openfilelist[curdir].count = 0;
    rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)//在当前目录下找，是否有重名目录
    {
        if(strcmp(fcbptr->filename, dirname) == 0 && strcmp(fcbptr->exname, "") == 0)
        {
            printf("Error,the dirname is already exist!\n");
            return;
        }
        fcbptr++;
    }
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)
    {
        if(fcbptr->free == 0)
            break;
        fcbptr++;
    }
    blkno = findblock();//寻找空闲盘块
    if(blkno == -1)
        return;
    (fat1 + blkno)->id = END;
    (fat2 + blkno)->id = END;
    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(fcbptr->filename, dirname);
    strcpy(fcbptr->exname, "");
    fcbptr->attribute = 1;
    fcbptr->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    fcbptr->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    fcbptr->first = blkno;
    fcbptr->length = 2 * sizeof(fcb);
    fcbptr->free = 1;
    openfilelist[curdir].count = i * sizeof(fcb);//把当前目录的文件读写指针定位到文件末尾
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);//从指针fcbptr开始写一个fcb大小的内容到当前目录文件末尾

    fd = my_open(dirname);//返回新建立的目录文件在用户打开文件数组的下标
    printf("mkdir my_open %d\n",fd);
    if(fd == -1)
        return;
    fcbptr = (fcb *)malloc(sizeof(fcb));//建立新目录的'.','..'目录
    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(fcbptr->filename, ".");
    strcpy(fcbptr->exname, "");
    fcbptr->attribute = 1;
    fcbptr->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    fcbptr->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    fcbptr->first = blkno;
    fcbptr->length = 2 * sizeof(fcb);
    fcbptr->free = 1;
    do_write(fd, (char *)fcbptr, sizeof(fcb), 2);
    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(fcbptr->filename, "..");
    strcpy(fcbptr->exname, "");
    fcbptr->attribute = 1;
    fcbptr->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    fcbptr->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    fcbptr->first = blkno;
    fcbptr->length = 2 * sizeof(fcb);
    fcbptr->free = 1;
    do_write(fd, (char *)fcbptr, sizeof(fcb), 2);
    free(fcbptr);
    my_close(fd);

    fcbptr = (fcb *)text;
    fcbptr->length = openfilelist[curdir].length;
    openfilelist[curdir].count = 0;
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);//更新当前目录文件的内容
    openfilelist[curdir].fcbstate = 1;
    printf("mdkir write length %d\n",openfilelist[curdir].length);
    if(flag){
        my_close(curdir);
        printf("my_mkdir my_close %d\n",curdir);
    }
    curdir=tempfd;
    printf("mkdir restore curdir %d\n",curdir);
    printf("my_mkdir exited\n");
}

void my_rmdir(char *dirname)
{
    fcb *fcbptr,*fcbptr2;
    fat *fat1, *fat2, *fatptr1, *fatptr2;
    char text[MAXTEXT], text2[MAXTEXT];
    unsigned short blkno;
    int rbn, rbn2, fd, i, j;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);

    int k=FileSubstr(dirname),tempfd=curdir;
    char dirpath[80];
    char newdir[20];
    int flag=0;
    if(k!=-1) {
        dirname[k] = 0;
        strcpy(newdir, dirname + k + 1);
        strcpy(dirpath, dirname);
        fd = open_path(dirpath);
        if(fd!=-1) {
            curdir=fd;
            strcpy(dirname,newdir);
            printf("my_rmdir open_path %d\n",fd);
            flag=1;
        }
        else {
            printf("error\n");
            return ;
        }
    }

    if(strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0)
    {
        printf("Error,can't remove this directory.\n");
        return;
    }
    openfilelist[curdir].count = 0;
    rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)//查找要删除的目录
    {
        if(strcmp(fcbptr->filename, dirname) == 0 && strcmp(fcbptr->exname, "") == 0)
            break;
        fcbptr++;
    }
    if(i == rbn / sizeof(fcb))
    {
        printf("Error,the directory is not exist.\n");
        return;
    }
    fd = open(dirname);//目录在当前打开文件数组中的下标
    rbn2 = do_read(fd, openfilelist[fd].length, text2);//读取要删除的目录的内容
    fcbptr2 = (fcb *)text2;
    for(j = 0; j < rbn2 / sizeof(fcb); j++)//判断要删除目录是否为空
    {
        if(strcmp(fcbptr2->filename, ".") && strcmp(fcbptr2->filename, "..") && strcmp(fcbptr2->filename, ""))
        {
            my_close(fd);
            printf("Error,the directory is not empty.\n");
            return;
        }
        fcbptr2++;
    }
    blkno = openfilelist[fd].first;
    while(blkno != END)//修改要删除目录在fat中所占用的目录项的属性
    {
        fatptr1 = fat1 + blkno;
        fatptr2 = fat2 + blkno;
        blkno = fatptr1->id;
        fatptr1->id = FREE;
        fatptr2->id = FREE;
    }
    my_close(fd);
    strcpy(fcbptr->filename, "");//修改已删除目录在当前目录的fcb的属性
    fcbptr->free = 0;
    openfilelist[curdir].count = i * sizeof(fcb);//更新当前目录文件的内容
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);
    openfilelist[curdir].fcbstate = 1;
    if(flag){
        printf("my_rmdir my_close %d\n",curdir);
        my_close(curdir);
    }
    curdir=tempfd;
    printf("my_rmdir exited\n");
}

void my_ls(char *filename)
{
    printf("my_ls %d \n",curdir);
    fcb *fcbptr;
    char text[MAXTEXT];
    int rbn, i,tempfd=curdir;
    int flag=0;
    if(strcmp(filename,"")!=0){
        char *str;
        str=strtok(filename," ");
        strcpy(filename,str);
        int fd=open_path(filename);
        curdir=fd;
        flag=1;
        if(fd==-1){
            printf("path %s doesn't exist \n",filename);
            curdir=tempfd;
            return ;
        }
    }
    openfilelist[curdir].count = 0;
    rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)
    {
        if(fcbptr->free)
        {
            if(fcbptr->attribute)
                printf("%s/\t\t<DIR>\t\t%d/%d/%d\t%02d:%02d:%02d\n", fcbptr->filename, (fcbptr->date >> 9) + 1980, (fcbptr->date >> 5) & 0x000f, fcbptr->date & 0x001f, fcbptr->time >> 11, (fcbptr->time >> 5) & 0x003f, fcbptr->time & 0x001f * 2);
            else
                printf("%s.%s\t\t%dB\t\t%d/%d/%d\t%02d:%02d:%02d\t\n", fcbptr->filename, fcbptr->exname, (int)(fcbptr->length), (fcbptr->date >> 9) + 1980, (fcbptr->date >> 5) & 0x000f, fcbptr->date & 0x1f, fcbptr->time >> 11, (fcbptr->time >> 5) & 0x3f, fcbptr->time & 0x1f * 2);
        }
        fcbptr++;
    }
    if(flag) my_close(curdir);
    curdir=tempfd;
    printf("my_ls exit\n");
}

int FileSubstr(char *str){
    int len=strlen(str);
    int cnt=0,flag=0;
    for(int i=1;i<len-1;i++)
    {
        if(str[i]=='/')
        {
            cnt++;
            flag=i;
        }
    }
    if(cnt==0) return -1;
    else return flag;
}

void my_create(char *filename)
{
    printf("my_create %s\n",filename);
    fcb *fcbptr;
    fat *fat1, *fat2;
    char *fname, *exname, text[MAXTEXT];
    unsigned short blkno;
    int rbn, i;
    time_t now;
    struct tm *nowtime;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3*BLOCKSIZE);

    int k=FileSubstr(filename),fd=-1,tempfd=curdir;
    char dirpath[80];
    char newdir[20];
    int flag=0;
    if(k!=-1) {
        filename[k] = 0;
        strcpy(newdir, filename + k + 1);
        strcpy(dirpath, filename);
        fd = open_path(dirpath);
        if(fd!=-1) {
            curdir=fd;
            strcpy(filename,newdir);
            flag=1;
            printf("my_create open_path %d\n",fd);
        }
        else {
            printf("error\n");
            return ;
        }
    }

    fname = strtok(filename, ".");
    exname = strtok(NULL, ".");
    if(strcmp(fname, "") == 0)
    {
        printf("Error,creating file must have a right name.\n");
        return;
    }
    if(!exname)
    {
        printf("Error,creating file must have a extern name.\n");
        return;
    }
    openfilelist[curdir].count = 0;
    rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)
    {
        if(strcmp(fcbptr->filename, fname) == 0 && strcmp(fcbptr->exname, exname) == 0)
        {
            printf("Error,the filename is already exist!\n");
            return;
        }
        fcbptr++;
    }
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)
    {
        if(fcbptr->free == 0) //找到未使用的文件表项，内容填写进去
            break;
        fcbptr++;
    }
    blkno = findblock();
    if(blkno == -1)
        return;
    (fat1 + blkno)->id = END;
    (fat2 + blkno)->id = END;

    now = time(NULL);
    nowtime = localtime(&now);
    strcpy(fcbptr->filename, fname);
    strcpy(fcbptr->exname, exname);
    fcbptr->attribute = 0;
    fcbptr->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;
    fcbptr->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;
    fcbptr->first = blkno;
    fcbptr->length = 0;
    fcbptr->free = 1;
    openfilelist[curdir].count = i * sizeof(fcb);
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);
    fcbptr = (fcb *)text;
    fcbptr->length = openfilelist[curdir].length;
    openfilelist[curdir].count = 0;
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);
    openfilelist[curdir].fcbstate = 1;
    if(flag) {
        my_close(curdir);
        printf("my_create my_close %d\n",curdir);
    }
    curdir=tempfd;
    printf("create exited\n");
}

void my_rm(char *filename)
{
    printf("my_rm %s\n",filename);
    fcb *fcbptr;
    fat *fat1, *fat2, *fatptr1, *fatptr2;
    char *fname, *exname, text[MAXTEXT];
    unsigned short blkno;
    int rbn, i;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);
    int k=FileSubstr(filename),fd=-1,tempfd=curdir;
    char dirpath[80];
    char newdir[20];
    int flag=0;
    if(k!=-1) {
        filename[k] = 0;
        strcpy(newdir, filename + k + 1);
        strcpy(dirpath, filename);
        fd = open_path(dirpath);
        if(fd!=-1) {
            curdir=fd;
            strcpy(filename,newdir);
            printf("my_create open_path %d\n",fd);
        }
        else {
            printf("error\n");
            return ;
        }
    }

    fname = strtok(filename, ".");
    exname = strtok(NULL, ".");
    if(strcmp(fname, "") == 0)
    {
        printf("Error,removing file must have a right name.\n");
        return;
    }
    if(!exname)
    {
        printf("Error,removing file must have a extern name.\n");
        return;
    }
    openfilelist[curdir].count = 0;
    rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for(i = 0; i < rbn / sizeof(fcb); i++)
    {
        if(strcmp(fcbptr->filename, fname) == 0 && strcmp(fcbptr->exname, exname) == 0)
            break;
        fcbptr++;
    }
    if(i == rbn / sizeof(fcb))
    {
        printf("Error,the file is not exist.\n");
        return;
    }
    blkno = fcbptr->first; //回收文件使用的磁盘空间
    while(blkno != END)
    {
        fatptr1 = fat1 + blkno;
        fatptr2 = fat2 + blkno;
        blkno = fatptr1->id;
        fatptr1->id = FREE;
        fatptr2->id = FREE;
    }
    strcpy(fcbptr->filename, "");
    fcbptr->free = 0;
    openfilelist[curdir].count = i * sizeof(fcb);
    do_write(curdir, (char *)fcbptr, sizeof(fcb), 2);
    openfilelist[curdir].fcbstate = 1;
    if(flag){
        printf("my_rm my_close %d\n",curdir);
        my_close(curdir);
    }
    curdir=tempfd;
    printf("my_rm exited\n");
}

int open(char *filename){
    int fd,tempfd=curdir;
    int k = FileSubstr(filename);
    char dirpath[80];
    char newdir[20];
    if (k != -1) {
        filename[k] = 0;
        strcpy(newdir, filename + k + 1);
        strcpy(dirpath, filename);
        fd = open_path(dirpath);
        if (fd != -1) {
            curdir = fd;
            strcpy(filename, newdir);
            if (!filename)
                strcmp(filename,".");
        }else {
            printf("error\n");
            curdir=tempfd;
            return -1;
        }
    }
    fd=my_open(filename);
    curdir=tempfd;
    return fd;
}

int my_open(char *filename)
{
    fcb *fcbptr;
    int i;
    char *fname, exname[3], *str, text[MAXTEXT];
    printf("my_open %s\n",filename);
    if(strcmp(filename,".")==0){
        printf("my_open exit \n");
        return curdir;
    }
    else if(strcmp(filename,"..")==0){
        if(curdir==0) return 0;
        strcpy(filename,openfilelist[curdir].dir);
        int k=FileSubstr(filename),fd=-1,tempfd=curdir;
        char dirpath[80];
        if(k!=-1) {
            filename[k] = 0;
            strcpy(dirpath, filename);
            fd = open_path(dirpath);
            if(fd!=-1) {
                return fd;
            }
            else {
                printf("my_open open_path %d\n",fd);
                curdir=tempfd;
                printf("error\n");
                return -1 ;
            }
        }
    }
   fname=strtok(filename,".");
    str = strtok(NULL, ".");
    if(str)
        strcpy(exname, str);
    else
        strcpy(exname, "");
    for( i = 0; i < MAXOPENFILE; i++)//在用户打开文件数组查找看当前文件是否已经打开
    {
        if(strcmp(openfilelist[i].filename, fname) == 0 && strcmp(openfilelist[i].exname, exname) == 0 && \
                openfilelist[i].dirno==openfilelist[curdir].first)
        {
            printf("my_open curdir %s has opened\n",fname);
           return i;
        }
    }
    openfilelist[curdir].count = 0;
    int rbn = do_read(curdir, openfilelist[curdir].length, text);
    fcbptr = (fcb *)text;
    for( i = 0; i < rbn / sizeof(fcb); i++)//在当前目录下找要打开的文件是否存在
    {
        if((strcmp(fcbptr->filename, fname) == 0 ) && ( strcmp(fcbptr->exname, exname) == 0 ))
            break;
        fcbptr++;
    }
    if(i == rbn / sizeof(fcb))
    {
        printf("Error,the file is not exist.\n");
        return -1;
    }
    int fd = findopenfile();//寻找空闲文件表项
    if(fd == -1)
        return -1;
    strcpy(openfilelist[fd].filename, fcbptr->filename); //将文件的属性信息复制到文件表项中
    strcpy(openfilelist[fd].exname, fcbptr->exname);
    openfilelist[fd].attribute = fcbptr->attribute;
    openfilelist[fd].time = fcbptr->time;
    openfilelist[fd].date = fcbptr->date;
    openfilelist[fd].first = fcbptr->first;
    openfilelist[fd].length = fcbptr->length;
    openfilelist[fd].dirno = openfilelist[curdir].first;
    openfilelist[fd].diroff = i;
    openfilelist[fd].free = fcbptr->free;
    strcpy(openfilelist[fd].dir, openfilelist[curdir].dir);
    if(fcbptr->attribute ) {
        strcat(openfilelist[fd].dir, filename);
        strcat(openfilelist[fd].dir, "/");
    }
    openfilelist[fd].count = 0;
    openfilelist[fd].fcbstate = 0;
    openfilelist[fd].topenfile = 1;
    printf("my_open exit \n");
    return fd;
}

void my_close(int fd)
{
    fcb *fcbptr;
    int father;
    if(fd < 0 || fd >= MAXOPENFILE)
    {
        printf("Error,the file is not exist.\n");
        return ;
    }
    if(openfilelist[fd].fcbstate) //如果文件的内容已经被修改，需要写入磁盘；or 只是关闭文件即可
    {
        printf("my_close %d changed \n",fd);
        fcbptr = (fcb *)malloc(sizeof(fcb));
        strcpy(fcbptr->filename, openfilelist[fd].filename);
        strcpy(fcbptr->exname, openfilelist[fd].exname);
        fcbptr->attribute = openfilelist[fd].attribute;
        fcbptr->time = openfilelist[fd].time;
        fcbptr->date = openfilelist[fd].date;
        fcbptr->first = openfilelist[fd].first;
        fcbptr->length = openfilelist[fd].length;
        fcbptr->free = openfilelist[fd].free;
        char temp[20];
        int flag=0;
        for(int i=0;i<MAXOPENFILE;i++){
            if(openfilelist[i].topenfile&&(openfilelist[i].first==openfilelist[fd].dirno))
            {
                flag=1;
                father=i;
                break;
            }
        }
        if(!flag){
            printf("the father parent didn't opened \n");
            if (openfilelist[fd].attribute) {
                char temp[80];
                strcpy(temp,"..");
                father=my_open(temp);
            }
            else {
                father=open_path(openfilelist[fd].dir);
            }
        }
        printf("the father %d parent open \n",father);
        openfilelist[father].count = openfilelist[fd].diroff * sizeof(fcb);
        do_write(father, (char *)fcbptr, sizeof(fcb), 2);
        if(father!=0 && !flag){
            strcpy(openfilelist[father].filename, "");
            strcpy(openfilelist[father].exname, "");
            openfilelist[father].topenfile = 0;
        }
        free(fcbptr);
        openfilelist[fd].fcbstate = 0;
    }else{
        printf("my_close %d didn't change \n",fd);
    }
    if(fd!=0)
    {
        strcpy(openfilelist[fd].filename, "");
        strcpy(openfilelist[fd].exname, "");
        openfilelist[fd].topenfile = 0;
    }
    printf("my_close exited\n");
}
char *outer=NULL;

int open_path(char *dirname){
    //如果dirname是绝对路径
    int fd=0, ok = 1, fdtemp = -1;//ok代表是否找到dirname所指的文件,fdtemp存临时的,用来关闭
    int temp=curdir;//暂时保管一下ptrcurdir原始值,可能要回
    printf("open_path curdir %d,currendir %s\n",curdir,dirname);
    if (dirname[0] == '/') {
        curdir = 0;//根目录
        //使工作目录暂时指向根目录
        char *p = strtok_r(dirname + 1, "/",&outer);//用“/”分割 dirname[1]开始的字符串
        if (p != NULL) {p=".";}//跳过/root
        while (p) {
            fd = my_open(p);
            printf("open_path my_open %d\n",fd);
            if (fd == -1) {
                ok = 0;
                if (fdtemp != -1 && fdtemp!=0 )  //离开前记得关文件
                {
                    printf("open_path my_close %d\n",fdtemp);
                    my_close(fdtemp);//把上个打开的文件关掉
                }
                break;
            }
            curdir =  fd;
            if (fdtemp != -1 && fdtemp!=0) {
                printf("open_path my_close %d\n",fdtemp);
                my_close(fdtemp);//把上个打开的文件关掉
            }
            fdtemp = fd;
            p = strtok_r(NULL, "/",&outer);
        }
    }
        //如果是相对路径
    else{
        char *p= strtok_r(dirname, "/",&outer);
        while (p) {
            fd = my_open(p);
            printf("open_path my_open %d\n",fd);
            if (fd == -1) {
                ok = 0;
                if (fdtemp != -1 && fdtemp!=0)  //离开前记得关文件
                {
                    my_close(fdtemp);//把上个打开的文件关掉
                    printf("open_path my_close %d\n",fdtemp);
                }
                break;
            }
            curdir = fd;
            if (fdtemp != -1 && fdtemp!=0 ) {
                my_close(fdtemp);//把上个打开的文件关掉
                printf("open_path my_close %d\n",fdtemp);
            }
            fdtemp = fd;
            p = strtok_r(NULL, "/",&outer);
        }
    }
    curdir = temp;
    printf("open_path restore curdir %d\n",curdir);
    //输出数据
    if (ok == 1)
    {
        return fd;
    }
    else
        return -1;
}


int my_write(int fd)
{
    fat *fat1, *fat2, *fatptr1, *fatptr2;
    int wstyle, len, ll, tmp;
    char text[MAXTEXT];
    unsigned short blkno;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);
    if(fd < 0 || fd >= MAXOPENFILE)
    {
        printf("The file is not exist!\n");
        return -1;
    }
    while(1)
    {
        printf("Please enter the number of write style:\n1.cut write\t2.cover write\t3.add write\n");
        scanf("%d", &wstyle);
        if(wstyle > 0 && wstyle < 4)
            break;
        printf("Input Error!");
    }
    getchar();  //吸收回车带来的多余的字符串
    switch(wstyle)
    {
        case 1://截断写把原文件所占的虚拟磁盘空间重置为1
            blkno = openfilelist[fd].first;
            fatptr1 = fat1 + blkno;
            fatptr2 = fat2 + blkno;
            blkno = fatptr1->id;
            fatptr1->id = END;
            fatptr2->id = END;
            while(blkno != END)
            {
                fatptr1 = fat1 + blkno;
                fatptr2 = fat2 + blkno;
                blkno = fatptr1->id;
                fatptr1->id = FREE;
                fatptr2->id = FREE;
            }
            openfilelist[fd].count = 0;
            openfilelist[fd].length = 0;
            break;
        case 2:
            openfilelist[fd].count = 0;
            break;
        case 3:
            openfilelist[fd].count = openfilelist[fd].length; //追加写的情况下，指针移动到文章的末尾
            break;
        default:
            break;
    }
    ll = 0;
    printf("please input write data(end with Ctrl+Z):\n");
    while(gets(text))
    {
        len = strlen(text);
        text[len++] = '\n';
        text[len] = '\0';
        tmp = do_write(fd, text, len, wstyle);
        if(tmp != -1)
            ll += tmp;
        if(tmp < len)
        {
            printf("Wirte Error!");
            break;
        }
    }
    return ll;//实际写的字节数
}

int do_write(int fd, char *text, int len, char wstyle)
{
    fat *fat1, *fat2, *fatptr1, *fatptr2;
    unsigned char *buf, *blkptr;
    unsigned short blkno, blkoff;
    int i, ll;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);
    buf = (unsigned char *)malloc(BLOCKSIZE); //申请1024单元的缓存的区域
    if(buf == NULL)
    {
        printf("malloc failed!\n");
        return -1;
    }
    blkno = openfilelist[fd].first;
    blkoff = openfilelist[fd].count;
    fatptr1 = fat1 + blkno;
    fatptr2 = fat2 + blkno;
    while(blkoff >= BLOCKSIZE)
    {
        blkno = fatptr1->id;
        if(blkno == END)
        {
            blkno = findblock();
            if(blkno == -1)
            {
                free(buf);
                return -1;
            }
            fatptr1->id = blkno;
            fatptr2->id = blkno;
            fatptr1 = fat1 + blkno;
            fatptr2 = fat2 + blkno;
            fatptr1->id = END;
            fatptr2->id = END;
        }
        else
        {
            fatptr1 = fat1 + blkno;
            fatptr2 = fat2 + blkno;
        }
        blkoff = blkoff - BLOCKSIZE;//让blkoff定位到文件最后一个磁盘块的读写位置
    }

    ll = 0;//实际写的字节数
    while(ll < len)//len是用户输入的字节数
    {
        blkptr = (unsigned char *)(myvhard + blkno * BLOCKSIZE);
        for(i = 0; i < BLOCKSIZE; i++)
            buf[i] = blkptr[i];
        for(;blkoff < BLOCKSIZE; blkoff++)
        {
            buf[blkoff] = text[ll++];
            openfilelist[fd].count++;
            if(ll == len)
                break;
        }
        for(i = 0; i < BLOCKSIZE; i++)
            blkptr[i] = buf[i];
        if(ll < len)//如果一个磁盘块写不下，则再分配一个磁盘块
        {
            blkno = fatptr1->id;
            if(blkno == END)
            {
                blkno = findblock();
                if(blkno == -1)
                    break;
                fatptr1->id = blkno;
                fatptr2->id = blkno;
                fatptr1 = fat1 + blkno;
                fatptr2 = fat2 + blkno;
                fatptr1->id = END;
                fatptr2->id = END;
            }
            else
            {
                fatptr1 = fat1 + blkno;
                fatptr2 = fat2 + blkno;
            }
            blkoff = 0;
        }
    }
    if(openfilelist[fd].count > openfilelist[fd].length)
        openfilelist[fd].length = openfilelist[fd].count;
    openfilelist[fd].fcbstate = 1;
    free(buf);
    return ll;
}
int my_read(int fd, int len)
{
    char text[MAXTEXT];
    int ll;
    if(fd < 0 || fd >= MAXOPENFILE)
    {
        printf("The File is not exist!\n");
        return -1;
    }
    if(openfilelist[fd].topenfile==0){
        printf("the %d is closed\n",fd);
        return -1;
    }
    openfilelist[fd].count = 0; //读指针清零，读取文章所有的内容
    ll = do_read(fd, len, text);//ll是实际读出的字节数
    if(ll != -1)
        printf("%s", text);
    else
        printf("Read Error!\n");
    return ll;
}

int do_read(int fd, int len, char *text)
{
    fat *fat1, *fatptr;
    unsigned char *buf, *blkptr;
    unsigned short blkno, blkoff;
    int i, ll;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    buf = (unsigned char *)malloc(BLOCKSIZE); //申请1024 bytes的缓存区
    if(buf == NULL)
    {
        printf("malloc failed!\n");
        return -1;
    }
    blkno = openfilelist[fd].first;
    blkoff = openfilelist[fd].count;  //设置从blkoff的位置开始读取数据
    if(blkoff >= openfilelist[fd].length)
    {
        puts("Read out of range!");
        free(buf);
        return -1;
    }
    fatptr = fat1 + blkno;
    while(blkoff >= BLOCKSIZE)//blkoff为最后一块盘块剩余的容量
    {
        blkno = fatptr->id;
        blkoff = blkoff - BLOCKSIZE;
        fatptr = fat1 + blkno;
    }
    ll = 0;
    while(ll < len)
    {
        blkptr = (unsigned char *)(myvhard + blkno * BLOCKSIZE);
        for(i = 0; i < BLOCKSIZE; i++)//将最后一块盘块的内容读取到buf中
            buf[i] = blkptr[i];
        for(; blkoff < BLOCKSIZE; blkoff++)
        {
            text[ll++] = buf[blkoff];
            openfilelist[fd].count++;
            if(ll == len || openfilelist[fd].count == openfilelist[fd].length)
                break;
        }
        if(ll < len && openfilelist[fd].count != openfilelist[fd].length)
        {
            blkno = fatptr->id;
            if(blkno == END)
                break;
            blkoff = 0;
            fatptr = fat1 + blkno;
        }
    }
    text[ll] = '\0';
    free(buf);
    return ll;
}
void my_exitsys()
{
    FILE *fp;
    int i;
    for(i = 0; i < MAXOPENFILE; i++)//在用户打开文件数组查找看当前文件是否已经打开
    {
        my_close(i);
    }
    fp = fopen(myfilename, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
    free(myvhard);
}
unsigned short findblock()
{
    unsigned short i;
    fat *fat1, *fatptr;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    for(i = 7; i < SIZE / BLOCKSIZE; i++)
    {
        fatptr = fat1 + i;
        if(fatptr->id == FREE)
            return i;
    }
    printf("Error,Can't find free block!\n");
    return -1;
}

int findopenfile() //遍历文件表项的标志位
{
    int i;
    for(i = 0; i < MAXTEXT; i++)
    {
        if(openfilelist[i].topenfile == 0)
            return i;
    }
    printf("Error,open too many files!\n");
    return -1;
}

void help()
{
    printf("\n");
    printf("-----------------------help------------\n");
    printf("format         :-------Format The Disk.\n");
    printf("exit           :-------Exit OS File System\n");
    printf("cd     dirname :-------Change Directory\n");
    printf("mkdir  dirname :-------Make Directory.\n");
    printf("rmdir  dirname :-------Delete Directory.\n");
    printf("ls     dirname :-------List Directory .\n");
    printf("creat  filename:-------Creat File\n");
    printf("write  fd      :-------Wirte File\n");
    printf("read   fd len  :-------Read File\n");
    printf("rm     filename:-------Remove File\n");
    printf("open   filename:-------Open File\n");
    printf("close  fd      :-------Close File\n");
    printf("--------------------------------------\n\n");
}

void operate(){
    char order[80];
    strcpy(order,"ls");
    int fd,len;
    while(strcmp(order, "exit")){
        printf("CS>>~%s$ ",currentdir);
        scanf("%s", order);
        if(!strcmp(order, "ls")){
            char temp[80];
            gets(temp);
            my_ls(temp);
        }else if(!strcmp(order, "format")){
            my_format();
            return;
        }else if(!strcmp(order, "mkdir")){
            char name[80];
            //printf("directory name(the length of the name < 11):");
            scanf("%s", name);
            my_mkdir(name);
        }else if(!strcmp(order, "cd")){
            char dirname[80];
            scanf("%s",dirname);
            my_cd(dirname);
        }else if(!strcmp(order, "rmdir")){
            char dirname[80];
            scanf("%s",dirname);
            my_rmdir(dirname);
        }else if(!strcmp(order, "create")){
            char filename[110];
            scanf("%s",filename);
            my_create(filename);
        }else if(!strcmp(order, "open")){
            char filename[80];
            scanf("%s",filename);
            fd=open(filename);
            printf("fd %d opened \n",fd);
        }else if(!strcmp(order, "close")){
            scanf("%d",&fd);
            my_close(fd);
        }else if(!strcmp(order, "rm")){
            char filename[80];
            scanf("%s",filename);
            my_rm(filename);
        }else if(!strcmp(order, "write")){
            scanf("%d",&fd);
            my_write(fd);
        }else if(!strcmp(order, "read")){
            scanf("%d%d",&fd,&len);
            my_read(fd,len);
        }
        else if(!strcmp(order, "h")){
            help();
        } else if(!strcmp(order, "exit")){
            my_exitsys();
            break;
        }
    }
}

int main()
{
    startsys();
    help();
    operate();
    return 0;
}
