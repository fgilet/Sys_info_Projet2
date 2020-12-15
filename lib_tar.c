#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib_tar.h"


unsigned int baseEightToTen(char *a) {
	int size = 0;
	while(a[size] != '\0') size++;
	unsigned int retVal = a[size - 1] - 48;
	int val = 8;
	for(int i = size - 2; i >= 0; i--) {
		retVal = retVal + ((a[i] - 48) * val); 
		val = val * 8;
	}
	return retVal;
}

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {

	int number_of_headers = 0;
	char buf[512];
	
	read(tar_fd, (void *) buf, 512);
	
	while(buf[0] != '\0') {
		
		number_of_headers++;
		
		for(int i = 0; i < 6; i++) {
			if(buf[257+i] != TMAGIC[i]) return -1;
		}
		
		for(int i = 0; i < 2; i++) {
			if(buf[263+i] != TVERSION[i]) return -2;
		}
	
		//computing checksum
		unsigned int computed_checksum = 0;
		int space = (int) ' ';
		for(int i = 0; i < 512; i++) {
			
			if(i >= 148 && i < 156) {
				computed_checksum = computed_checksum + space;
			}
			else computed_checksum = computed_checksum + buf[i]; 
		}
		
		//reading checksum
		char readchecksum[9];
		for(int i = 0; i < 8; i++) {
			readchecksum[i] = buf[148+i];
		}
		readchecksum[8] = '\0';
			
		//comparing checksums
		unsigned int rchsm = baseEightToTen(readchecksum);
		if(computed_checksum != rchsm) return -3;
		
		//skipping to next header block
		char size[13];
		for(int i = 0; i < 12; i++) {
			size[i] = buf[124 + i];
		}
		size[12] = '\0';
		int s = baseEightToTen(size);
		if(s != 0) {
			int skip = (s - (s%512)) / 512;
			if(s%512 != 0) skip++;
			lseek(tar_fd, 512 * skip, SEEK_CUR); 
		}
		read(tar_fd, (void *) buf, 512);
	}
	return number_of_headers;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    char buf[512];
        read(tar_fd, (void *) buf, 512);
        
        while (buf[0] != '\0') {
            int i = 0;
            int name = 1;
            while(path[i] != '\0'){
                if(path[i] != buf[i]) name = 0;
                    i++;
                }
                if(buf[i] != '\0') name = 0;
                if(name == 1) return 1;

            char size[13];
            for(int i = 0; i < 12; i++) {
                size[i] = buf[124 + i];
            }
            size[12] = '\0';
            int s = baseEightToTen(size);
            if(s != 0) {
                int skip = (s - (s%512)) / 512;
                if(s%512 != 0) skip++;
                lseek(tar_fd, 512 * skip, SEEK_CUR);
            }
            read(tar_fd, (void *) buf, 512);
        }
        
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {

	char buf[512];
	read(tar_fd, (void *) buf, 512);
	
	while(buf[0] != '\0') {
		if(buf[156] == '5') {			
			int i = 0;
			int same_name = 1;
			while(path[i] != '\0') {
				if(path[i] != buf[i]) same_name = 0;
				i++;
			}
			if(buf[i] != '\0') same_name = 0;
			if(same_name == 1) return 1;
		}
		
		char size[13];
		for(int i = 0; i < 12; i++) {
			size[i] = buf[124 + i];
		}
		size[12] = '\0';
		int s = baseEightToTen(size);
		if(s != 0) {
			int skip = (s - (s%512)) / 512;
			if(s%512 != 0) skip++;
			lseek(tar_fd, 512 * skip, SEEK_CUR); 
		}
		read(tar_fd, (void *) buf, 512);
	}
	return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
	
	char buf[512];
	read(tar_fd, (void *) buf, 512);
	
	while(buf[0] != '\0') {
		if(buf[156] == '0' || buf[156] == '\0') {			
			int i = 0;
			int same_name = 1;
			while(path[i] != '\0') {
				if(path[i] != buf[i]) same_name = 0;
				i++;
			}
			if(buf[i] != '\0') same_name = 0;
			if(same_name == 1) return 1;
		}
		
		char size[13];
		for(int i = 0; i < 12; i++) {
			size[i] = buf[124 + i];
		}
		size[12] = '\0';
		int s = baseEightToTen(size);
		if(s != 0) {
			int skip = (s - (s%512)) / 512;
			if(s%512 != 0) skip++;
			lseek(tar_fd, 512 * skip, SEEK_CUR); 
		}
		read(tar_fd, (void *) buf, 512);
	}
	return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    
    char buf[512];
	read(tar_fd, (void *) buf, 512);
	
	while(buf[0] != '\0') {
		if(buf[156] == '2') {			
			int i = 0;
			int same_name = 1;
			while(path[i] != '\0') {
				if(path[i] != buf[i]) same_name = 0;
				i++;
			}
			if(buf[i] != '\0') same_name = 0;
			if(same_name == 1) return 1;
		}
		
		char size[13];
		for(int i = 0; i < 12; i++) {
			size[i] = buf[124 + i];
		}
		size[12] = '\0';
		int s = baseEightToTen(size);
		if(s != 0) {
			int skip = (s - (s%512)) / 512;
			if(s%512 != 0) skip++;
			lseek(tar_fd, 512 * skip, SEEK_CUR); 
		}
		read(tar_fd, (void *) buf, 512);
	}
	return 0;
}


//return the index on which the name of the file (excluding folder name) starts
int getDirIndex(char *path) {

	int count_slash = 0;
	int i = 0;

	while(path[i] != '\0') {
		if(path[i] == '/') count_slash++;
		i++;
	}
	
	i = 0;
	while(count_slash > 0) {
		if(path[i] == '/') count_slash--;
		i++;
	}
	return i;
}

int list2(int tar_fd, char *path, char **entries, size_t *no_entries) {

	char buf[512];

	if(is_symlink(tar_fd, path) == 1) {
	
		//TO DO
	
		/*lseek(tar_fd, -512, SEEK_CUR);
		read(tar_fd, (void *) buf, 512);
		lseek(tar_fd, 0, SEEK_SET);
		char new_path[100];
		for(int i = 157; buf[i] != '\0'; i++) {
			printf("buf[%d] : %c\n", i, buf[i]);
			new_path[i] = buf[i];
		}
		printf("new_path : %s\n", new_path);
		return list(tar_fd, new_path, entries, no_entries);*/
	}

	if(is_dir(tar_fd, path) == 0) {
		(*no_entries) = 0;
		return 0;
	}
	
	char name[100];
	read(tar_fd, (void *) buf, 512);
	int written = 0;
	
	while(buf[0] != '\0' && *no_entries > written) {
	
		for(int i = 0; i < 100; i++) {
			name[i] = buf[i];
		}
		
		//printf("name : %s\n", name);
		
		int index = getDirIndex(name);
		char folder[100];
		
		for(int i = 0; i < 100; i++) {
			if(i < index) folder[i] = name[i];
			else folder[i] = '\0';
		}
		
		//printf("folder : %s\n", folder);
		
		char lastName[100];
		
		for(int i = index; i < 100 + index; i++) {
			if(i < 100 - index) lastName[i - index] = name[i];
			else lastName[i - index] = '\0';
		}
		
		//printf("lastName : %s\n", lastName);
		
		//printf("%s == %s : %d",folder, path, strcmp(folder, path));
		
		if(strcmp(folder, path) == 0 && name[index] != '\0') {
			strcpy(entries[written], lastName);
			written++;
		}
	
		
	
		char size[13];
        	for(int i = 0; i < 12; i++) {
            	size[i] = buf[124 + i];
       	 }
       	 size[12] = '\0';
       	 int s = baseEightToTen(size);
       	 if(s != 0) {
          	 	int skip = (s - (s%512)) / 512;
         	 	if(s%512 != 0) skip++;
         	 	lseek(tar_fd, 512 * skip, SEEK_CUR);
        	}
        	read(tar_fd, (void *) buf, 512);
		
	}
	(*no_entries) = written;
	return 1;
}


int check_and_point(int tar_fd, char *path){
    if(is_dir(tar_fd, path) == 1) {
        lseek(tar_fd, -512, SEEK_CUR);
        return 1;
    }
    
    lseek(tar_fd, 0, SEEK_SET);
    if(is_symlink(tar_fd, path) == 1) {
        lseek(tar_fd, -512, SEEK_CUR);
        char buf[512];
        read(tar_fd, (void *) buf, 512);
        int count_slash = 0;
        int i = 0;
        while(buf[i] != '\0') {
            if(buf[i] == '/') count_slash++;
            i++;
            
        }
        char link_to[100];
        i = 0;
        while(count_slash > 0) {
            link_to[i] = buf[i];
            if(buf[i] == '/') count_slash--;
            i++;
        }
        int j = 0;
        while(buf[157 + j] != '\0' && j < 100) {
            link_to[i] = buf[157 + j];
            i++;
            j++;
        }
        char final_path[i + 1];
        for(int k = 0; k < i; k++) {
            final_path[k] = link_to[k];
        }
        final_path[i] = '\0';
        lseek(tar_fd, 0, SEEK_SET);
        return point_to_beginning(tar_fd, final_path);
    }
    return 0;
}

/**
 * Lists the entries at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entry in entries.
 *                   The callee set it to the number of entry listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    
    if(check_and_point(tar_fd, path) == 0) return 0;
    
    char buf[512];
    int fileCount = 0;
    read(tar_fd, (void *) buf, 512);
    
    while(buf[0] != '\0' ){
        fileCount++;
        int i = 0;
        char name[100];
        while(buf[i] != '\0' && i < 100) {
            name[i] = buf[i];
            i++;
        }
        entries[fileCount] = name;
        char size[13];
        for(int i = 0; i < 12; i++) {
            size[i] = buf[124 + i];
        }
        size[12] = '\0';
        int s = baseEightToTen(size);
        if(s != 0) {
            int skip = (s - (s%512)) / 512;
            if(s%512 != 0) skip++;
            lseek(tar_fd, 512 * skip, SEEK_CUR);
        }
        read(tar_fd, (void *) buf, 512);
    }
    (*no_entries) = fileCount;
    return 1;
}

int point_to_beginning(int tar_fd, char *path) {
	if(is_file(tar_fd, path) == 1) {
		lseek(tar_fd, -512, SEEK_CUR);
		return 0;
	}
	
	lseek(tar_fd, 0, SEEK_SET);
	if(is_symlink(tar_fd, path) == 1) {
		lseek(tar_fd, -512, SEEK_CUR);
		char buf[512];
		read(tar_fd, (void *) buf, 512);
		int count_slash = 0;
		int i = 0;
		while(buf[i] != '\0') {
			if(buf[i] == '/') count_slash++;
			i++;
			
		}
		char link_to[100];
		i = 0;
		while(count_slash > 0) {
			link_to[i] = buf[i];
			if(buf[i] == '/') count_slash--;
			i++;
		}
		int j = 0;
		while(buf[157 + j] != '\0' && j < 100) {
			link_to[i] = buf[157 + j];
			i++;
			j++;
		}
		char final_path[i + 1];
		for(int k = 0; k < i; k++) {
			final_path[k] = link_to[k];
		}
		final_path[i] = '\0';
		lseek(tar_fd, 0, SEEK_SET);
		return point_to_beginning(tar_fd, final_path);
	}
	return -1;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {

	if(point_to_beginning(tar_fd, path) == -1) return -1;
	char buf[512];
	read(tar_fd, (void *) buf, 512);	
	
	char size[13];
	for(int i = 0; i < 12; i++) {
		size[i] = buf[124 + i];
	}
	size[12] = '\0';
	int length = baseEightToTen(size) - offset; 
	if(length < 0) return -2;
	
	
	lseek(tar_fd, offset, SEEK_CUR);
	read(tar_fd, (void *) buf, 512);
	
	
	int dest_size = *len;
	*len = 0;
	int i = 0;
	int k = 0;
	
	
	while(buf[i] != '\0' && k < dest_size && k < length) {
		dest[k] = buf[i];
		(*len)++;
		k++;
		i++;
		if(i >= 512) {
			i = 0;
			read(tar_fd, (void *) buf, 512);
		}
	}
	
	return length - k;
}
