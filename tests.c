#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open("archive.tar" , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
    
    
    size_t no_entries = 5;
    char **entries;
    entries = (char **) malloc(sizeof(char *) * no_entries);
    for(int i = 0; i < no_entries; i++) {
    	entries[i] = (char *) malloc(sizeof(char) * 100);
    }
    
        
    
    int ret = list(fd, argv[1], entries, &no_entries);
    printf("returned from list\n");
    for(int i = 0; i < no_entries; i++) {
    	if(ret != 0) printf("%s\n", entries[i]);
    }
    printf("no_entries : %ld\n", no_entries);
    printf("list returned %d\n", ret);

    return 0;
}
