#include "input.h"

#ifndef SDK_C_AUXILIARY_H
#define SDK_C_AUXILIARY_H
#define BUY_CPU_MEM_LOW 0.6
#define BUY_CPU_MEM_HIGH 1.8

#include "set"
#include <numeric>
#include <algorithm>
#include <time.h>
#include <thread>
//struct modified_hash {
//
//    static uint64_t splitmix64(uint64_t x)
//    {
//        x += 0x9e3779b97f4a7c15;
//        x = (x ^ (x >> 30))
//            * 0xbf58476d1ce4e5b9;
//        x = (x ^ (x >> 27))
//            * 0x94d049bb133111eb;
//        return x ^ (x >> 31);
//    }
//
//    int operator()(uint64_t x) const
//    {
//        static const uint64_t random
//                = std::chrono::steady_clock::now()
//                        .time_since_epoch()
//                        .count();
//        return splitmix64(x + random);
//    }
//};
struct VMServerMatchInfo {
    std::vector<ServerInfo> servers;
    // serverExist[serverName] == true means vm->server
    std::unordered_set<std::string> serverExist;
};

//struct VmAtServer {
//    string vimId;
//    int ServerId;
//};
class Helper {
public:
    Helper(Input *_input, int totalDays) : totalDays(totalDays), input(_input), serverCost(0), powerCost(0) {}

    void dailySolve(int day);

    void displayCost();

    void addOutput(std::string s);

    Input *input;
//    void getMemweight ( int &a ,int &val);
//    void getMoneyWeight( int &a, int &val);
//    void getCpuweight(int &a  ,int &val);
private:
    // 当前已经的购买服务器
    std::vector<Server> currentServers;
    // vmOnlyID --> {serverId , nodeType}
    vector<bool> deleted_machine;
    std::unordered_map<int, std::pair<int, int>, modified_hash> request2ServerInfo;
    int currentDay, totalDays;
    //为了purchase 输出
    std::unordered_map<std::string, int> serversToBuy;
    std::unordered_map<int,int, modified_hash>reqId2Del;
    ULL serverCost, powerCost;
        vector<int> vmsid;
//    void purchase(int mem_need,int cpu_need,int mem_least,int cpu_least);
    ServerInfo buyServer(Request &request);

    void distribute();

    int migrate();

    int try_fit(Server &dstServer, const VirtualMachine &virtualHost);

    // 寻找合适的虚拟机
    int fitServer(Server &server, Request &request, VirtualMachine &virtualHost,int res);

//    int fitServer(Server &server, const Request &request);
    int findServer(std::vector<Server> &currentServers, std::vector<int> &ServerIds, VirtualMachine &vm,
                 Server &min_server, int op,int server_now_id);

    double eps = 1e-8;
    double func(double p0,double q0,double p1,double q1);
    int count_cpu_mem(int p0,int p1) {return p0*p0+p1*p1;}
    // 为服务器分配资源
    void allocatedServer(Server &server, int coreCost, int memCost, int type);

    // 服务器名称->可以装下的虚拟机列表
    std::unordered_map<std::string, std::vector<VirtualMachine>> serverVMMatch;
    // 虚拟机名称->可以装下该虚拟机的服务器列表
    std::unordered_map<std::string, VMServerMatchInfo> vmServerMatch;

    void deleteVirtualHost(int serverID, int deployType, VirtualMachine &virtualHost);

};

#endif //SDK_C_AUXILIARY_CUH
