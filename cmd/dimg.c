/**1 you must compile it in debug mod as assert() contaion valid code
 * 2 you can only run dimg under 'cmd','src','bin'..
 * 3, 根据argc来确定filepath，如果argc为1，则filepath取默认400m.img。若argc为2，则filepath *    取/dev/sdb。
 **/
#define ENV_NOT_KERNEL
#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<errno.h>
#include "../include/bootinfo.h"
static char *filepaths[] = { "../cmd/400m.img", "/dev/sdb" };
static int filesize(char *name){
	int fd = open(name);
	if(fd == -1) return -1;
	int count = lseek(fd, 0, SEEK_END);
	close(fd);
	return count;
}
int main(int argc, char *argv[]){
	char cmd[100];
	assert(argc <= 2);
	char *filepath = filepaths[argc - 1];
	assert(filesize("../bin/kernel.elf") < 60 * 1024 && 
			"kernel.elf firstly loaded at 0x80000, take care of 0xA0000, and \
			boot.asm only read 126 sectors.");
	sprintf(cmd,"dd if=../bin/kernel.elf of=%s bs=512 conv=notrunc seek=%u", filepath, kernel_image_start_sector-1);
	system(cmd);

	unsigned char boot_flag[]={0x55,0xaa};
	int fd_sys=open(filepath,1+1,0);
	if(fd_sys==-1){
		printf("open %s error:%d",filepath, errno);
		return 1;
	}
	char dpt[48];
	lseek(fd_sys,446,0);
	assert(read(fd_sys,dpt,48) == 48);
	/**dpt already buffered,we can write boot.bin to 400m.img now*/
	sprintf(cmd, "dd if=../bin/boot.bin of=%s bs=512 conv=notrunc seek=0", filepath);
	system(cmd);
	/**write back dpt*/
	lseek(fd_sys,446,0);
	assert(write(fd_sys,dpt,48)==48);

	/**set boot flags*/
	lseek(fd_sys,510,0);
	assert(write(fd_sys,boot_flag,2)==2);

	fsync(fd_sys);
	assert(close(fd_sys)!=-1);
	printf("dimg done..");
	return 0;
}
