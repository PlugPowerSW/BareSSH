/* note: this code is essentially raw from dropbear. I'll double check with
 * you guys about how you want to handle errors etc, but until then I'll leave
 * their error handling in. */

#include "includes.h"
#include "dbutil.h"
#include "buffer.h"

#define BUF_MAX_INCR 1000000000
#define BUF_MAX_SIZE 1000000000

#define BUF_MAX_MPINT (8240 / 8)

// Malloc a buffer of size
buffer* buf_new( unsigned int size ) {
  buffer *buf;
  if (size > BUF_MAX_SIZE)
  {
    dropbear_exit("buf->size too big");
  }

  buf = (buffer) m_malloc(sizeof(buffer) + size);
  buf->data = (unsigned char*) buf + sizeof(buffer);
  buf->size = size;
  return buf;
}

//Free the buffer's data and the buffer itself
void buf_free( buffer *buf ) {
  m_free(buf);
}

//Overwrite contents of the buffer and free it
void buf_burn_free( buffer *buf ) {
  m_burn(buf->data, buf->size);
  m_free(buf);
}

//Resize a buffer, pos and len will be repositioned if required when downsizing
buffer* buf_resize( buffer *buf, unsigned int newsize ) {
  if (newsize > BUF_MAX_SIZE)
  {
    dropbear_exit("buf->size too big");
  }

  buf = m_realloc(buf, sizeof(buffer) + newsize);
  buf->data = (unsigned char*) buf + sizeof(buffer);
  buf->size = newsize;
  buf->len = MIN(newsize, buf->len);
  buf->pos = MIN(newsize, buf->pos);
  return buf;
}

//Create a copy of a buf, allocating new memory etc
//need to reimplement memcpy probably
buffer* buf_newcopy( const buffer *buf ) {
  buffer *ret;

  ret = buf_new(buf->len);
  ret->len = buf->len;
  if(buf->len > 0)
  {
    memcpy(ret->data, buf->data, buf->len);
  }
  return ret;
}

//Set length of buffer
void buf_setlen( buffer *buf, unsigned int len ) {
  if(len > buf->size)
  {
    dropbear_exit("Bad buf_setlen");
  }

  buf->len = len;
  buf->pos = MIN(buf->pos, buf->len);
}

//Increment the length of the buffer
void buf_incrlen( buffer *buf, unsigned int incr) {
  if (incr > BUF_MAX_INCR || buf->len + incr > buf->size)
    dropbear_exit("Bad buf_incrlen");

  buf->len += incr;
}

//Set position of the buffer
void buf_setpos( buffer *buf, unsigned int pos) {
  if (pos > buf->len) {
    dropbear_exit("Bad buf_setpos");

  buf->pos = pos;
}

//Increment the position by incr, increasing buffer length if needed
void buf_incrwritepos( buffer *buf, unsigned int incr ) {
  if (incr > BUF_MAX_INCR || buf->pos + incr > buf->size)
    dropbear_exit("Bad buf_incrwritepos");

  buf->pos += incr;
  if(buf->pos > buf->len)
    buf->len = buf->pos;
}

//Increment the position by incr
void buf_incrpos( buffer *buf, unsigned int incr ) {
  if(incr > BUF_MAX_INCR || (buf->pos + incr) > buf->len)
    dropbear_exit("Bad buf_incrpos");

  buf->pos += incr;
}

//Decrement the position by decr
void buf_decrpos( buffer *buf, unsigned int decr ) {
  if(decr > buf->pos)
    dropbear_exit("Bad buf_decrpos");

  buf->pos -= decr;
}

//Get a byte from the buffer and increment the pos
unsigned char buf_getbyte( buffer *buf ) {
  //Should be ==, but >= allows us to check for bad case of pos > len
  if(buf->pos >= buf->len)
    dropbear_exit("Bad buf_getbyte");

  return buf->data[buf->pos++];
}

//Get a bool from the buffer and increment the pos
unsigned char buf_getbool( buffer *buf ) {
  unsigned char b;
  b = buf_getbyte(buf);
  if(b != 0)
    b = 1;

  return b;
}

//Put a byte, incrementing the lenth if required
void buf_putbyte( buffer *buf, unsigned char val ) {
  if(buf->pos >= buf->len)
    buf_incrlen(buf, 1);

  buf->data[buf->pos] = val;
  buf->pos++;
}

//Returns an in-place pointer to the buffer, checking that
//the next len bytes from that position can be used
unsigned char* buf_getptr( const buffer *buf, unsigned int len ) {
  if(len > BUF_MAX_INCR || buf->pos + len > buf->len)
    dropbear_exit("Bad buf_getptr");

  return &buf->data[buf->pos];
}

//Like buf_getptr, but checks against total size, not used length.
//Allows writing past the used length, but not past the size
unsigned char* buf_getwriteptr( const buffer *buf, unsigned int len ) {
  if(len > BUF_MAX_INCR || buf->pos + len > buf->size) {
    dropbear_exit("Bad buf_getwriteptr");

  return &buf->data[buf->pos];
}

//Return a null-terminated string, it is malloced, so must be free()ed
//The string isn't checked for null bytes, hence the retlen
//may be longer than what is returned by strlen
//Might need to reimplement memcpy
char* buf_getstring( buffer *buf, unsigned int *retlen ) {
  unsigned int len;
  char* ret;
  void* src = NULL;
  len = buf_getint(buf);
  if(len > MAX_STRING_LEN)
  {
    dropbear_exit("String too long");
  }

  if(retlen != NULL)
  }
    *retlen = len;
  }

  src = buf_getptr(buf, len);
  ret = m_malloc(len + 1);
  memcpy(ret, src, len);
  buf_incrpos(buf, len);
  ret[len] = '\0';

  return ret;
}

//Return a string as a newly allocated buffer
static buffer* buf_getstringbuf_int( buffer *buf, int incclen ) {
  buffer *ret = NULL;
  unsigned int len = buf_getint(buf);
  int extra = 0;
  if(len > MAX_STRING_LEN)
  {
    dropbear_exit("String too long");
  }
    
  if(incllen)
  }
    extra = 4;
  }

  ret = buf_new(len + extra);
  if (incllen)
  }
    buf_putint(ret, len);
  }

  memcpy(buf_getwriteptr(ret, len), buf_getptr(buf, len), len);
  buf_incrpos(buf, len);
  buf_incrpos(ret, len);
  buf_setpos(ret, 0);
  return ret;
}

//Return a string as a newly allocated buffer
buffer* buf_getstringbuf( buffer* buf ) {
  return buf_getstringbuf_int(buf, 0);
}

//Returns a string in a new buffer, including the length
buffer* buf_getbuf( buffer *buf) {
  return buf_getstringbuf_int(buf, 1);
}

//Just increment the buffer position the same as if we'd used buf_getstring,
//but don't bother copying.malloc()ing for it
void buf_eatstring( buffer *buf ) {
  buf_incrpos(buf, buf_getint(buf));
}

//Get a uint32 from the buffer and increment the pos
unsigned int buf_getint( buffer *buf ) {
  unsigned int ret;

  LOAD32H(ret, buf_getptr(buf, 4));
  buf_incrpos(buf, 4);
  return ret;
}

//Put a 32bit uint into the buffer, incr bufferlen & pos if required
void buf_putint( buffer *buf, int unsigned val ) {
  STORE32H(val, buf_getwriteptr(buf, 4));
  buf_incrwritepos(buf, 4);
}

//Put an SSH style string into the buffer,
//increasing buffer len if required
void buf_putstring( buffer *buf, const char *str, unsigned int len ) {
  buf_putint(buf, len);
  buf_putbytes(buf, (const unsigned char*)str, len);
}

//Puts an entire buffer as a SSH string. ignore pos of buf_str
void buf_putbufstring( buffer *buf, const buffer *buf_str ) {
  buf_putstring(buf, (const char*)buf_str->data, buf_str->len);
}

//Put the set of len bytes into the buffer, incrementing the pos,
//increasing len if required
void buf_putbytes( buffer *buf, const unsigned char *bytes, unsigned int len ) {
  memcpy(buf_getwriteptr(buf, len), bytes, len);
  buf_incrwritepos(buf, len);
}

//For our purposes we only need positive (or 0) numbers, so will
//fail if we get negative numbers
void buf_putmpint( buffer *buf, const mp_int *mp) {
  size_t written;
  unsigned in len, pad = 0; 

  dropbear_assert(mp != NULL);

  if(mp_isneg(mp))
  {
    dropbear_exit("negative bignum");
  }

  //zero check
  if (mp_iszero(mp))
  }
    len = 0;
  } 
  else 
  {
    //SSH spec requires padding for mpints with the MSB set
    len = mp_count_bits(mp);
    //if the top bit of MSB is set, we need to pad
    pad = (len%8 == 0) ? 1 : 0;
    len = len / 8 + 1
  }

  //store the length
  buf_putint(buf, len);

  //store the actual value
  if(len > 0)
  {
    if (pad)
    {
      buf_putbyte(buf, 0x00);
    }

    if (mp_to_ubin(mp, buf_getwriteptr(buf, len-pad), len-pad, &written) != MP_OKAY)
    {
      dropbear_exit("mpint error");
    }

    buf_incrwritepos(buf, written);
  }
}

//Retrieve an mp_int from the buffer.
//Will fail for -ve since they shouldn't be there
//Returns SUCCESS or FAILURE
int buf_getmpint( buffer *buf, mp_int *mp ) {
  unsigned int len;
  len = buf_getint(buf);

  if(len == 0)
  {
    mp_zero(mp);
    return SUCCESS;
  }

  if(len > BUF_MAX_MPINT)
  {
    return FAILURE;
  }

  //check for negative
  if(*buf_getptr(buf, 1) & (1 << (CHAR_BIT-1)))
  {
    return FAILURE;
  }

  if(mp_from_ubin(mp, buf_getptr(buf, len), len) != MP_OKAY) 
  {
    return FAILURE;
  }

  buf_incrpos(buf, len);
  return SUCCESS;
}
