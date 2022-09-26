#define MAXLEN  1024
int main(void)
{ 
    int ret;
   // char key[17];
    char *p;

    p = (char*)malloc(MAXLEN*(sizeof(char*)));	//申請一塊記憶體，存放讀取資料
    memset(p,0,MAXLEN);

    printf("請輸入一串字元：\n");

    ret = read(0,p,MAXLEN);

    printf("read 函式就可以直接從command line 讀取資料嗎？試一試read到沒！\n");
    printf("下面為定義記憶體資料輸出。。。\n");

    for(int i =0; p[i]!='\n';i++)
    {
        printf("key[%d] = %c\n",i,p[i]);
    }

    printf("試一試write函式~~~\n");
	for(int i =0; p[i] != '\n';i++)
	{
		ret = write(1,&p[i],1);
		printf("\n");
		printf("寫入資料個數：ret = %d\n",ret);
	}
	free(p);
	
}
