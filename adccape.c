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

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)


int main(int argc, char **argv) {
    int fd, i;
    void *map_base;
    unsigned long read_result, writeval;
    unsigned int numberOutputSamples = 16;
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
    
   for(i=0; i<numberOutputSamples; i++){
       read_result = *(((unsigned int *) map_base) + i);
       printf("Value at address 0x%X is: 0x%X\n", target + i * sizeof(unsigned int), read_result);
   }
   fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(fd);
    return 0;
}
