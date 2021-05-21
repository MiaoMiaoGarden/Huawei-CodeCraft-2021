#include "iostream"
#include <unordered_map>
#ifndef SDK_C_REQUESTS_H
#define SDK_C_REQUESTS_H
struct Request {
    int id;
    int operation;
    std::string virtualHostName;
    int vmOnlyID; // 创建请求的虚拟机ID唯一
};

#endif //SDK_C_REQUESTS_H
