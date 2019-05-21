#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAP_SIZE 8192UL
#define MAP_MASK (MAP_SIZE - 1)


int main(int argc, char **argv) {
    int fd;
    void *map_base;
    unsigned int numberOutputSamples = 2048;
    unsigned const int row_size = 16;
    unsigned long read_result;
    off_t target = 0x4a300000;

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
    	printf("Failed to open memory! Are you root?\n");
    	return -1;
    }
    fflush(stdout);

    map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target);
    if(map_base == (void *) -1) {
       printf("Failed to map base address\n");
       return -1;
    }
    fflush(stdout);
    
   for(unsigned int i=0; i < numberOutputSamples / row_size; i++){
       printf("0x%lX:\t", target + i * row_size * sizeof(unsigned int));
       for(unsigned int j=0; j < row_size; j++) {
           read_result = *(((unsigned int *) map_base) + i * row_size + j);
           printf("0x%lX\t", read_result);
       }
       printf("\n");
   }
   fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(fd);
    return 0;
}
