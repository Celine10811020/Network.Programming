/*Linux環境下檔案的讀寫操作，使用到的函式有open、read、write、lseek。
open:用於開啟或者建立檔案。
read:從檔案中讀指定位元組的資料到記憶體中。
write：講記憶體中資料寫入到檔案中。
lseek：可以改變當前檔案偏移量。*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
  char read_file[] = "/home/imx/shares/open/read.bmp";
  char write_file[] = "/home/imx/shares/open/write.bmp";
  int file_len = 0;
  int file_total = 0;
  int read_len = 0;
  char buf[1024] = {0};
  int read_fd = open(read_file,O_RDWR);
  int write_fd = open(write_file,O_RDWR|O_TRUNC |O_CREAT,00777);
  
  if(read_fd < 0 || write_fd < 0 )
  {
    printf("file open error\n");
    return 1;
  }
  
  file_len = (int)lseek(read_fd,0,SEEK_END);//get file total size
  file_total = file_len;
  printf("file_total:%d\n",file_total);
  lseek(read_fd,0,SEEK_SET);
  printf("read start\n");
  
  while(read_len = read(read_fd,buf,1024))//file read
  {
    write(write_fd,buf,read_len);//file write
    file_len = file_len - read_len;
    memset(buf,0,1024);
  }
  
  printf("read and write end\n");
  
  if(file_len != 0)//check file read write
  {
    close(read_fd);
    close(write_fd);
    printf("error:read!=write\n");
    return 1;
  }
  
  close(read_fd);
  close(write_fd);
  return 0;
}
