#include "Server.h"
#include "VirtualMachine.h"
#include "Requests.h"
#include "unordered_set"
#include "unordered_map"
#include "vector"
#include "iostream"
#include <chrono>
#ifndef SDK_C_INPUT_H
#define SDK_C_INPUT_H
using namespace std;
typedef unsigned long long ULL;
struct modified_hash {

    static uint64_t splitmix64(uint64_t x)
    {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30))
            * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27))
            * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    int operator()(uint64_t x) const
    {
        static const uint64_t random
                = std::chrono::steady_clock::now()
                        .time_since_epoch()
                        .count();
        return splitmix64(x + random);
    }
};
struct MigInfo {
    int from_id;
    int to_id;
    string apply_info;
};
struct Input{
public:
    Input();
    int inputServer();
    int inputVM();
    int inputNum();
    int inputRequests();
    int CPU_MEM;
    vector<int> all_cpu;
    vector<int> all_mem;

    std::vector<std::string> split(std::string &empStr, char c);
    // map vmOnlyID to VM name
//    unordered_map<int,std::string> reqID2vmName;
    unordered_map<int,int ,modified_hash> reqID2VmTypeID;
    vector<VirtualMachine> vmLists;
//    unordered_map<std::string,VirtualMachine> vmName2VM;
    unordered_map<string,int> vmName2typeId;
    std::vector<ServerInfo> serverLists;
    std::vector<ServerInfo> cpuServerLists;
    std::vector<ServerInfo> MemServerLists;
    //vms running now
    int K;
    int T;
    vector<vector<int>> all_del_index;
    unordered_map<int,int> del_day;
    std::vector<std::vector<Request>> allRequests;
    std::vector<std::vector<Request>> addRequests;
    std::vector<std::string> res;
//    ULL total_mem = 0, total_cpu = 0;
    double ratio;
    int MEM= 0,CPU = 0;
    int del_MEM = 0,del_CPU = 0;
    std::vector<MigInfo> mig_info;
    struct VMServerMatchInfo{
        std::vector<ServerInfo> servers;
        // serverExist[serverName] == true means vm->server
        std::unordered_set<std::string> serverExist;
    };
};

#endif //SDK_C_INPUT_H
