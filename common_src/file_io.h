#ifndef __FILE_IO_H__
#define __FILE_IO_H__


#define FIO_SEEK_START 0
#define FIO_SEEK_CUR   1
#define FIO_SEEK_END   2

#define FIO_OPEN_RD 0
#define FIO_OPEN_WR 1
#define FIO_OPEN_RW 2

#ifdef USE_CUSTOM_FILE_IO /* In case the C stdlib file handling is buggy, like in DOS/OpenWatcom */

	#define FIO_CACHE 32
	
	struct file_t {
	  unsigned short fh;        /* file handle */
	  unsigned long flen;       /* file length */
	  unsigned long curpos;     /* current offset position (ftell) */
	  unsigned char buff[FIO_CACHE]; /* buffer storage   */
	  unsigned long bufoffs;    /* offset of buffer */
	  unsigned char flags;      /* flags */
	};

	/* open file fname and set fhandle with the associated file handle. returns 0 on success, non-zero otherwise */
	int fio_open(struct file_t *f, const char *fname, int mode);

	/* reads count bytes from file pointed at by fhandle, and writes the data into buff. returns the number of bytes actually read */
	int fio_read(struct file_t *f, void *buff, int count);

	/* seek to offset position of file pointed at by fhandle. returns current file position on success, a negative error otherwise */
	signed long fio_seek(struct file_t *f, unsigned short origin, signed long offset);

	/* close file handle. returns 0 on success, non-zero otherwise */
	int fio_close(struct file_t *f);

#else /* Just use the C standard library */
	#include <stdio.h>

	struct file_t {
	  FILE *fh;        /* file handle */
	  long flen;       /* file length */
	  long curpos;     /* current offset position (ftell) */
	};

	/* open file fname and set fhandle with the associated file handle. returns 0 on success, non-zero otherwise */
	static int fio_open(struct file_t *f, const char *fname, int mode) {
		switch(mode) {
			case FIO_OPEN_RD:
				f->fh = fopen(fname, "r");
				break;
			case FIO_OPEN_WR:
				f->fh = fopen(fname, "w");
				break;
			case FIO_OPEN_RW:
				f->fh = fopen(fname, "r+");
				break;
			default:
				return -1;
		}
		
		if(f->fh==NULL) {
			return -1;
		}
		
		if(fseek(f->fh, 0, SEEK_END) != 0) {
			return -1;
		}
		
		if( (f->flen = ftell(f->fh)) < 0 ) {
			return -1;
		}
		
		if(fseek(f->fh, 0, SEEK_SET) != 0) {
			return -1;
		}
		
		f->curpos = 0;
		return 0;
	}

	/* reads count bytes from file pointed at by fhandle, and writes the data into buff. returns the number of bytes actually read */
	static int fio_read(struct file_t *f, void *buff, int count) {
		int out = fread(buff, 1, count, f->fh);
		if( (f->curpos = ftell(f->fh)) < 0 ) {
			return -1;
		}
		return out;
	}

	/* seek to offset position of file pointed at by fhandle. returns current file position on success, a negative error otherwise */
	static signed long fio_seek(struct file_t *f, unsigned short origin, signed long offset) {
		int o = SEEK_SET;
		switch(origin) {
			case FIO_SEEK_START:
				o = SEEK_SET;
				break;
			case FIO_SEEK_CUR:
				o = SEEK_CUR;
				break;
			case FIO_SEEK_END:
				o = SEEK_END;
				break;
			default:
				return -1;
		}
		
		if(fseek(f->fh, offset, o) != 0) {
			return -1;
		}
		
		if( (f->curpos = ftell(f->fh)) < 0 ) {
			return -1;
		}
		
		return 0;
	}

	/* close file handle. returns 0 on success, non-zero otherwise */
	static int fio_close(struct file_t *f) {
		if(fclose(f->fh) == EOF) {
			return -1;
		}
		return 0;
	}
#endif /* USE_CUSTOM_FILE_IO */

#endif /* __FILE_IO_H__ */
