#include"server.h"
#define SER_IP "192.168.42.129"
#define SER_PORT 8888
int main()
{
    Server server(SER_IP,SER_PORT,5);
    return 0;
}