/*
 * File I/O, based on the DOSMID source code
 * http://dosmid.sourceforge.net
 *
 * Copyright (C) 2018 Mateusz Viste
 * with modifications
 * Copyright (C) 2021 Andreas Zdziarstek
 */

#include <dos.h>    /* REGS */
#include <string.h> /* _fmemcpy() */

#include "file_io.h"

#define FIO_FLAG_SEEKSYNC 1

static void fio_seek_sync(struct file_t *f) {
/* DOS 2+ - LSEEK - SET CURRENT FILE POSITION
   AH = 42h
   AL = origin of move
     00h start of file
     01h current file position
     02h end of file
   BX = file handle
   CX:DX = (signed) offset from origin of new file position
   Return on success:
     CF clear
     DX:AX = new file position in bytes from start of file
   Return on error:
     CF set
     AX = error code */
  union REGS regs;
  unsigned short *off;
  long offset;
  if ((f->flags & FIO_FLAG_SEEKSYNC) == 0) return;
  offset = f->curpos;
  off = (unsigned short *)(&offset);
  regs.x.ax = 0x4200u;
  regs.x.bx = f->fh;
  regs.x.cx = off[1];
  regs.x.dx = off[0];
  int86(0x21, &regs, &regs);
  f->flags &= ~FIO_FLAG_SEEKSYNC;
  /* if (regs.x.cflag != 0) return(0 - regs.x.ax);
  return(((long)regs.x.dx << 16) | regs.x.ax); */
}

/* seek to offset position of file pointed at by fhandle. returns current file position on success, a negative error otherwise */
signed long fio_seek(struct file_t *f, unsigned short origin, signed long offset) {
  switch (origin) {
    case FIO_SEEK_START:
      if (offset < 1) {
        f->curpos = 0;
      } else {
        f->curpos = offset;
      }
      break;
    case FIO_SEEK_END:
      f->curpos = f->flen;
    case FIO_SEEK_CUR:
      f->curpos += offset;
      break;
  }
  f->flags |= FIO_FLAG_SEEKSYNC;
  return(f->curpos);
}

static void loadcache(struct file_t *f) {
  union REGS regs;
  struct SREGS sregs;
  fio_seek_sync(f);
  f->flags |= FIO_FLAG_SEEKSYNC;
  f->bufoffs = f->curpos;
  regs.h.ah = 0x3f;
  regs.x.bx = f->fh;
  regs.x.cx = FIO_CACHE;
  sregs.ds = FP_SEG(f->buff);
  regs.x.dx = FP_OFF(f->buff);
  int86x(0x21, &regs, &regs, &sregs);
}

/* open file fname and set fhandle with the associated file handle. returns 0 on success, non-zero otherwise */
int fio_open(struct file_t *f, const char *fname, int mode) {
  /* DOS 2+ - OPEN - OPEN EXISTING FILE
     AH = 3Dh
     AL = access and sharing modes
     DS:DX -> ASCIZ filename
     CL = attribute mask of files to look for (server call only)
     Return:
     CF clear if successful
     AX = file handle or error code
     CF set on error */
  union REGS regs;
  struct SREGS sregs;
  /* */
  regs.h.ah = 0x3d;
  regs.h.al = mode;
  sregs.ds = FP_SEG(fname);
  regs.x.dx = FP_OFF(fname);
  int86x(0x21, &regs, &regs, &sregs);
  f->fh = regs.x.ax;
  if (regs.x.cflag != 0) return(-1);
  /* */
  f->curpos = 0;
  loadcache(f);
  /* fseek to end so I know the file length */
  regs.x.ax = 0x4202u;
  regs.x.bx = f->fh;
  regs.x.cx = 0;
  regs.x.dx = 0;
  int86(0x21, &regs, &regs);
  f->flen = (((long)regs.x.dx << 16) | regs.x.ax);
  /* fseek to start */
  regs.x.ax = 0x4200u;
  regs.x.bx = f->fh;
  regs.x.cx = 0;
  regs.x.dx = 0;
  int86(0x21, &regs, &regs);
  /* */
  f->flags = FIO_FLAG_SEEKSYNC;
  return(0);
}

/* reads count bytes from file pointed at by fhandle, and writes the data into buff. returns the number of bytes actually read, or a negative number on error */
int fio_read(struct file_t *f, void *buff, int count) {
/* DOS 2+ - READ - READ FROM FILE OR DEVICE
 * AH = 3Fh
 * BX = file handle
 * CX = number of bytes to read
 * DS:DX -> buffer for data
 * Return:
 * CF clear if successful
 * AX = number of bytes actually read (0 if at EOF before call)
 * CF set on error
 * AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h) */
  union REGS regs;
  struct SREGS sregs;
  if (f->curpos + count > f->flen) count = f->flen - f->curpos;
  if (count == 0) return(0);
  if (count <= FIO_CACHE) {
    if ((f->curpos < f->bufoffs) || (f->curpos + count > f->bufoffs + FIO_CACHE)) {
      loadcache(f);
    }
    _fmemcpy(buff, f->buff + (f->curpos - f->bufoffs), count);
    f->curpos += count;
    return(count);
  }
  fio_seek_sync(f);
  regs.h.ah = 0x3f;
  regs.x.bx = f->fh;
  regs.x.cx = count;
  sregs.ds = FP_SEG(buff);
  regs.x.dx = FP_OFF(buff);
  int86x(0x21, &regs, &regs, &sregs);
  if (regs.x.cflag != 0) return(0 - regs.x.ax);
  f->curpos += regs.x.ax;
  return(regs.x.ax);
}

/* close file handle. returns 0 on success, non-zero otherwise */
int fio_close(struct file_t *f) {
  /* DOS 2+ - CLOSE - CLOSE FILE
     AH = 3Eh
     BX = file handle
     Return on success:
       CF clear
       AX destroyed
     On error:
       CF set
       AX = error code */
  union REGS regs;
  regs.h.ah = 0x3e;
  regs.x.bx = f->fh;
  int86(0x21, &regs, &regs);
  if (regs.x.cflag != 0) return(0 - regs.x.ax);
  return(0);
}
