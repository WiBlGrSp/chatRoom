# 聊天室协议设计
发送消息时进行序列化处理,接收消息后进行反序列化处理
单条消息的结构
struct Message
{
    uint32_t type;  //消息的类型
    char name[32];  //用户昵称,用于标识消息的来源
    char text[512]; //消息正文,字符串,固定512字节
};

消息的类型
enum MsgType
{
    LOGIN = 1,
    CHAT = 2,
    LOGOUT = 3,

    LOGIN_OK = 101,
    LOGIN_FAIL = 102,

    USER_JOIN = 103,
    USER_LEAVE = 104
};

