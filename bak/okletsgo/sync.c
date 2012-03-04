#define	_FILE_OFFSET_BITS	64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "type.h"
#include "layout.h"
#include "sync.h"
#include "log.h"

static int g_fd=0; /* sync_init, THE BIG-DISK-FILE descriptor */
static FILE* log_file=NULL;

STATE sync_init(char *pathname, char* sync_log)
{
	assert(pathname);
	if (sync_log) {
		log_file=fopen(sync_log, "w");/*XXX: append write?? */
		if (log_file==NULL) {
			printf ("[%s:%d] %s: LOG_FILE:%s open failed. File may be exist, or try using SUDO.\n", __FILE__, __LINE__, strerror(errno), sync_log);
			return SYNC_NG_INIT_LOG;
		}
	} else {
		printf ("[%s:%d]warning: not specify \"sync_log\" file.\n", __FILE__, __LINE__);
	}

	g_fd=open(pathname, O_RDWR | O_LARGEFILE | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (g_fd<=0) {
		log_err(__FILE__, __LINE__, log_file, "THE_BIG_FILE %s:%s open failed. May be open twice.",strerror(errno), pathname);
		return SYNC_NG_FD;
	}

	if (log_file) {
		fprintf (log_file, "[INIT   ]:sync init success.\n");
	} else {
		printf ("sync init success.\n");
	}

	return SYNC_SUCCESS;
}

void sync_exit()
{
	if (log_file) {
		fprintf (log_file, "EXIT.\n");
		fclose(log_file);
	} else {
		printf ("sync exit.\n");
	}
	log_file=NULL;
	close(g_fd);
}

STATE sync_write(PTR_DISK _disk, const char* _mem, int size)
{
	off64_t ret;
	ssize_t n, towrite;
	const char* ptr;

	if (g_fd<=0) {
		log_err(__FILE__, __LINE__, log_file, "Invalid file descriptor");
		return SYNC_NG_FD;
	}

	ret=lseek64(g_fd, (off64_t)_disk*CHUNK_SIZE, SEEK_SET);
	if (ret<0) {
		log_err(__FILE__, __LINE__, log_file, " %s: lseek64 %d CHUNK wrong", strerror(errno), _disk);
		return SYNC_NG_LSEEK;
	}

	towrite=size;
	ptr=_mem;
	while ( towrite>0 )
	{
		/*FIXME: write return value */
		n=write(g_fd, ptr, size); 
		towrite-=n;
		ptr+=n;
	}

	if (log_file) {
		fprintf (log_file,"[WRITE  ]:%20d\t\t@%20ld\t%d\n", n, (long int)_disk*CHUNK_SIZE,_disk);
		/* fflush(log_file);  */  /*XXX: need flush? */ 
	}

	return SYNC_SUCCESS;
}

STATE sync_read(PTR_DISK _disk, char* _mem, int size)
{
	off64_t ret;
	ssize_t n=0, toread;
	char* ptr;

	if (g_fd<=0) {
		log_err(__FILE__, __LINE__, log_file, "Invalid file descriptor");
		return SYNC_NG_FD;
	}

	ret=lseek64(g_fd, (off64_t)_disk*CHUNK_SIZE, SEEK_SET);
	if (ret<0) {
		log_err(__FILE__, __LINE__, log_file, " %s: lseek64 %d CHUNK wrong", strerror(errno), _disk);
		return SYNC_NG_LSEEK;
	}

	toread=size;
	ptr=_mem;
	while ( toread>0 )
	{
		/*FIXME: read return value */
		n=read(g_fd, ptr, size);
		toread-=n;
		ptr+=n;
	}

	if (log_file) {
		fprintf (log_file, "[READ   ]:%20d\t\t@%20ld\n",(int)n, (long int)_disk*CHUNK_SIZE);
	}
	return SYNC_SUCCESS;
}

