#include"client.h"
#define CLI_IP "192.168.42.129"
#define CLI_PORT 9999
#define SER_IP "192.168.42.129"
#define SER_PORT 8888
int main()
{
    Client client(CLI_IP,CLI_PORT,SER_IP,SER_PORT);
}