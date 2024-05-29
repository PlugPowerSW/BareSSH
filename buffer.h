#ifndef __BUFFER_H
#define __BUFFER_H

#include "includes.h"

struct buf {
  unsigned char* data; //pointer to malloc holding data
  unsigned int len; //size of used memory
  unsigned int pos;
  unsigned int size; //memory size
};

typedef struct buf buffer;

buffer* buf_new( unsigned int size );
buffer* buf_resize( buffer *buf, unsigned int newsize );
void buf_free( buffer *buf );
void buf_burn_free( buffer *buf );
buffer* buf_newcopy( const buffer *buf );
void buf_setlen( buffer *buf, unsigned int len );
void buf_incrlen( buffer *buf, unsigned int incr );
void buf_setpos( buffer *buf, unsigned int pos );
void buf_incrpos( buffer *buf, unsigned int incr );
void buf_decrpos( buffer *buf, unsigned int decr );
void buf_incrwritepos( buffer *buf, unsigned int incr );
unsigned char buf_getbyte( buffer *buf );
unsigned char buf_getbool( buffer *buf );
void buf_putbyte( buffer *buf, unsigned char val );
unsigned char* buf_getptr( const buffer *buf, unsigned int len );
unsigned char* buf_getwriteptr( const buffer *buf, unsigned int len );
char* buf_getstring( buffer *buf, unsigned int retlen );
buffer* buf_getstringbuf( buffer *buf );
buffer* buf_getbuf( buffer *buf );
void buf_eatstring( buffer *buf );
void buf_putint( buffer *buf, unsigned int val );
void buf_putstring( buffer *buf, const char *str, unsigned int len );
void buf_putbufstring( buffer *buf, const buffer *buf_str );
void buf_putbytes( buffer *buf, const unsigned char *bytes, unsigned int len );
void buf_putmpint( buffer *buf, const mp_int *mp );
int buf_getmpint( buffer *buf, mp_int *mp );
unsigned int buf_getint( buffer *buf );

#endif //__BUFFER_H
