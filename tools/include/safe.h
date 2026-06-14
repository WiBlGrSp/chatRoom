#ifndef _SAFE_H_
#define _SAFE_H_
#include <cstddef>
namespace Safe
{
int copy(char*dst,std::size_t dst_size,const char*src);
int input(char*dst,std::size_t dst_size);
}
#endif