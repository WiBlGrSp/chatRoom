#ifndef _SAFE_H_
#define _SAFE_H_
#include <cstddef>
namespace Safe
{
int Copy(char*dst,std::size_t dst_size,const char*src);
int Input(char*dst,std::size_t dst_size);
}
#endif