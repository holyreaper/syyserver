#include"kfifol.h"
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<stdlib.h>
kfifol::kfifol(int nsize)
:m_pbuffer(NULL)
,m_nsize(nsize)
,m_nin(0)
,m_nout(0)
{
     if (!is_power_of_2(nsize))
     {
         m_nsize = roundup_power_of_two(nsize);
     }
}
kfifol::kfifol()
:m_pbuffer(NULL)
,m_nsize(K_DEFAULT_LEN)
,m_nin(0)
,m_nout(0)
{

}

kfifol::~kfifol()
{
    if(NULL != m_pbuffer)
    {
        delete[] m_pbuffer;
        m_pbuffer = NULL;
    }
}
bool kfifol::init()
{
    m_pbuffer = new unsigned char[m_nsize];
    if (!m_pbuffer)
    {
         return false;
    }
     m_nin = m_nout = 0;
     return true;
 }
  
 unsigned long kfifol::roundup_power_of_two(unsigned long val)
 {
     if((val & (val-1)) == 0)
         return val;
  
     unsigned long maxulong = (unsigned long)((unsigned long)~0);
     unsigned long andv = ~(maxulong&(maxulong>>1));
     while((andv & val) == 0)
         andv = andv>>1;
  
    return andv<<1;
 }
 unsigned int kfifol::put(const unsigned char *buffer, unsigned int len)
{
     unsigned int l;
  
    len = std::min(len, m_nsize - m_nin + m_nout);
  
     /*
 *       * Ensure that we sample the m_nOut index -before- we
 *             * start putting bytes into the kfifol.
 *                   */
#ifndef PLATFORM_WIN32
	__sync_synchronize();
#else 
#endif
  
     /* first put the data starting from fifo->in to buffer end */
     l = std::min(len, m_nsize - (m_nin  & (m_nsize - 1)));
     memcpy(m_pbuffer + (m_nin & (m_nsize - 1)), buffer, l);
  
     /* then put the rest (if any) at the beginning of the buffer */
     memcpy(m_pbuffer, buffer + l, len - l);
  
     /*
 *       * Ensure that we add the bytes to the kfifo -before-
 *             * we update the fifo->in index.
 *                   */
#ifndef PLATFORM_WIN32
	 __sync_synchronize();
#else 
#endif  
     m_nin += len;
     return len;
 }
  
 unsigned int kfifol::get(unsigned char *buffer, unsigned int len)
 {
     unsigned int l;
  
     len = std::min(len, m_nin - m_nout);
  
     /*
 *       * Ensure that we sample the fifo->in index -before- we
 *             * start removing bytes from the kfifo.
 *                   */
#ifndef PLATFORM_WIN32
	 __sync_synchronize();
#else 
#endif  
     /* first get the data from fifo->out until the end of the buffer */
     l = std::min(len, m_nsize - (m_nout & (m_nsize - 1)));
     memcpy(buffer, m_pbuffer + (m_nout & (m_nsize - 1)), l);
  
     /* then get the rest (if any) from the beginning of the buffer */
     memcpy(buffer + l, m_pbuffer, len - l);
  
    /*
 *       * Ensure that we remove the bytes from the kfifo -before-
 *            * we update the fifo->out index.
 *                 */
#ifndef PLATFORM_WIN32
	 __sync_synchronize();
#else 
#endif 
    m_nout += len;
 
    return len;
 }
