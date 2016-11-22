#ifndef _UNLOCK_QUEUE_H
#define _UNLOCK_QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include "../platform/_platform_def.h"
/*
* brief :a queue like linux kfifo 
* fbi warning: only support one productor and one consumer stations;
*/
#define K_DEFAULT_LEN 1024*1024
class kfifol
{
public:
     kfifol(int nsize);
	 kfifol();
     virtual ~kfifol();
     bool init();
     unsigned int put(const unsigned char *pbuffer, unsigned int nlen);
     unsigned int get(unsigned char *pbuffer, unsigned int nben);
     inline void clean() { m_nin = m_nout = 0; }
     inline unsigned int getlen() const { return  m_nin - m_nout; }
 private:
     inline bool is_power_of_2(unsigned long n) { return (n != 0 && ((n & (n - 1)) == 0)); };
     inline unsigned long roundup_power_of_two(unsigned long val);
 private:
     unsigned char *m_pbuffer;    /* the buffer holding the data */
     unsigned int   m_nsize;        /* the size of the allocated buffer */
     unsigned int   m_nin;        /* data is added at offset (in % size) */
     unsigned int   m_nout;        /* data is extracted from off. (out % size) */
 };
#endif
