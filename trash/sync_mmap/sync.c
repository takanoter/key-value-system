/*============================================================================
# Author: Wade Leng
# E-mail: wade.hit@gmail.com
# Last modified: 2011-12-11 20:11
# Filename: sync.c
# Description: 
============================================================================*/
#define	_FILE_OFFSET_BITS	64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "layout.h"
#include "sync.h"

static	int	fd = -1; 
static	FILE*	log_file = NULL;
static	char*	start_disk = NULL;
static	int	BIGFILE_LEN;

int sync_init(char* pathname, int file_len, char* sync_log)
{
	if (sync_log) 
	{
		log_file = fopen(sync_log, "w");/*XXX:fopen?mmap?*/
		if (log_file == NULL)
			return -1;
	}

	if ((fd = open(pathname, O_RDWR | O_LARGEFILE | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
		return -1;
	
	BIGFILE_LEN = file_len;
	start_disk = (char*)mmap(NULL, BIGFILE_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (start_disk == MAP_FAILED)
		return -1;

	return 0;
}

int sync_read(char* mem, int size, OFFSET_T disk_offset)
{
	if (memcpy(mem, start_disk + disk_offset, size))
		return -1;

	return 0;
}

int sync_write(const char* mem, int size, OFFSET_T disk_offset)
{
	if(memcpy(start_disk + disk_offset, mem, size))
		return -1;

	return 0;
}

int sync_flush()
{
	if (msync(start_disk, BIGFILE_LEN, MS_ASYNC | MS_INVALIDATE) == -1)
		return -1;

	return 0;
}

int sync_exit()
{
	if (log_file) {
		fprintf (log_file, "EXIT.\n");
		fclose(log_file);
	}
	log_file=NULL;
	close(fd);
	if (!sync_flush())
		return -1;

	return 0;
}
