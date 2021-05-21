#include <iostream>
#include <unordered_set>
#include <vector>
#include <math.h>

#ifndef SDK_C_SERVER_H
#define SDK_C_SERVER_H
struct ServerInfo {
    std::string name;
    int cupNum, memoryNum, hardwareCost, dailyCost;
    int type_index;
};
enum Operation {
    ADD, DEL,
    PURCHASE, MIGRATE,FIT,
    DEPLOY_A, DEPLOY_B, DEPLOY_AB, UNABLE
};

struct Server {
    int id; // curServer 中的下标
    int remappedID; //serverLists 中的下标
    ServerInfo serverInfo;
    //current resource
    std::pair<int, int> A, B;
    //map server to request ID
    std::unordered_set<int> requestIDs;
    std::vector<int> A_instance;
    std::vector<int> B_instance;
    std::vector<int> AB_instance;
    double cpuUsage, memUsage;
    double totalUsage;
    double cmRatio;
    int type_index;

    // int minCpu,minMem;
    Server() = default;
    inline int getLeftCpu() {
        return A.first + B.first;
    }
    inline int getLeftMem() {
        return A.second + B.second;
    }
    Server(const ServerInfo &s, int id_) : id(id_),
                                           remappedID(-1), serverInfo(s), A({s.cupNum / 2, s.memoryNum / 2}),
                                           B({s.cupNum / 2, s.memoryNum / 2}), cmRatio((double) s.cupNum / s.memoryNum),
                                           A_instance(0), B_instance(0), AB_instance(0) {}

    inline bool fit(std::pair<int, int> &C, int cupNum, int memoryNum) {
        return C.first >= cupNum && C.second >= memoryNum;
    }
    inline double unusage_a(){
        double c = 2.0*A.first/serverInfo.cupNum,m = 2.0*A.second/serverInfo.memoryNum;
        return c*c+m*m;
    }
    inline double unusage_b(){
        double c = 2.0*B.first/serverInfo.cupNum,m = 2.0*B.second/serverInfo.memoryNum;
        return c*c+m*m;
    }
    inline double unusage_ab(){
        double c = 1.0*(A.first+B.first)/serverInfo.cupNum,m = 1.0*(A.second+B.second)/serverInfo.memoryNum;
        return c*c+m*m;
    }
    inline double getCpuUsage() {
        return 1.0 - (A.first + B.first) * 1.0 / serverInfo.cupNum;
    }

    inline double getMemoryUsage() {
        return 1.0 - (A.second + B.second) * 1.0 / serverInfo.memoryNum;
    }

    inline bool isGood(std::pair<int, int> &C, int cupNum, int memoryNum) {
        return abs(log(double(C.first + cupNum) /
                double(C.second + memoryNum))) <=  abs(
                log(double(C.first) / double(C.second)));
    }
    inline bool isGood(std::pair<int, int> &C,std::pair<int, int> &D, int cupNum, int memoryNum) {
        return abs(log(double(C.first + cupNum) + double(D.first + cupNum)/
                       double(C.second + memoryNum) + double(D.second + memoryNum))) <= abs(
                log(double(C.first + D.first) / double(C.second + D.second)));
    }
    inline int qrt_sum_a(){return A.first*A.first+A.second*A.second;}
    inline int qrt_sum_b(){return B.first*B.first+B.second*B.second;}
};

#endif //SDK_C_SERVER_H
