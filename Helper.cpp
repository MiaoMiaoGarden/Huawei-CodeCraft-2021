#include <algorithm>
#include "Helper.h"
#include<thread>
int flag =true;
int currentVMNum = 0;

void Helper::addOutput(std::string s) {
    input->res.emplace_back(s);
}

inline double Helper::func(double p0, double q0, double p1, double q1) {
    double miu0 = (p0 + q0) / 2 + eps, miu1 = (p1 + q1) / 2 + eps;
    return 0.5 * (p0 * log((p0 + eps) / miu0) + q0 * log((q0 + eps) / miu0))
           + 0.5 * (p1 * log((p1 + eps) / miu1) + q1 * log((q1 + eps) / miu1));
}

//原来的purchase
ServerInfo Helper::buyServer(Request &request) {
    // TODO:买得分最高的server
    int n = input->serverLists.size();
    int choose_id = 0;
    VirtualMachine &vm = input->vmLists[input->reqID2VmTypeID[request.vmOnlyID]];//input->vmName2VM[request.virtualHostName];
    int del_ = input->del_day.find(request.vmOnlyID) == input->del_day.end() ? input->T
                                                                             : input->del_day[request.vmOnlyID];
    int life = del_ - currentDay;
    int left_Day = input->T - currentDay;
    double p0 = 1.0 * input->all_cpu[currentDay] / (input->all_cpu[currentDay] + input->all_mem[currentDay]);
    double p1 = 1.0 * input->all_mem[currentDay] / (input->all_cpu[currentDay] + input->all_mem[currentDay]);
    auto getWeight = [&](ServerInfo &server) -> double {
        double score = 0.0;
        int last_cpu = (server.cupNum >> (1 - vm.is_double)) - vm.cupNum;
        int last_mem = (server.memoryNum >> (1 - vm.is_double)) - vm.memoryNum;
        double q0 = 1.0 * last_cpu / (last_mem + last_cpu), q1 = 1.0 * last_mem / (last_cpu + last_mem);
        double js_1 = func(p0, q0, p1, q1);
        int money = server.hardwareCost + left_Day * server.dailyCost;
        q0 = 1.0 * server.cupNum / (server.cupNum + server.memoryNum);
        q1 = 1.0 * server.memoryNum / (server.cupNum + server.memoryNum);
        double js_new = func(p0, q0, p1, q1);
        //input->addRequests.size() > -1
        //input->addRequests.size() > 300
        if (input->addRequests[currentDay].size() > 300) {// For special purchase
            if (!vm.is_double)
                score = 15231 * js_1 * left_Day + money +
                        750.0 * money / (last_cpu + last_mem + server.cupNum / 2 + server.memoryNum / 2);
            else
                score = 15231 * (js_1 * life + js_new * (left_Day - life)) + money +
                        750.0 * money / (last_cpu + last_mem);
        } else {
            if (!vm.is_double)
                score = 15231 * (js_1 * life + js_new * (left_Day - life)) + money +
                        750.0 * money / (last_mem + last_cpu + server.memoryNum / 2 + server.cupNum / 2);
            else
                score = 15231 * (js_1 * life + js_new * (left_Day - life)) + money +
                        750.0 * money / (last_cpu + last_mem);
        }
        return score;
    };
    double choose_weight = 1e+9;
    for (int l = 0; l < n; l++) {
        auto &server = input->serverLists[l];
        if ((server.cupNum >> (1 - vm.is_double)) < (vm.cupNum) ||
            (server.memoryNum >> (1 - vm.is_double)) < (vm.memoryNum))
            continue;
        double weight = getWeight(server);
        if (weight < choose_weight) {
            choose_id = l;
            choose_weight = weight;
        }
    }
    return input->serverLists[choose_id];
}

int Helper::fitServer(Server &server, Request &request, VirtualMachine &virtualHost, int res) {
    int coreCost = virtualHost.cupNum >> virtualHost.is_double;
    int memCost = virtualHost.memoryNum >> virtualHost.is_double;
    if (res == UNABLE) { if ((res = try_fit(server, virtualHost)) == UNABLE) return false; }
    {
        allocatedServer(server, coreCost, memCost, res);
        server.requestIDs.insert(request.vmOnlyID);
        switch (res) {
            case DEPLOY_A:
                server.A_instance.push_back(request.id);
                break;
            case DEPLOY_B:
                server.B_instance.push_back(request.id);
                break;
            case DEPLOY_AB:
                server.AB_instance.push_back(request.id);
                break;
        }
        request2ServerInfo[request.vmOnlyID] = {server.id, res};
    }
    return res;
}

int Helper::findServer(std::vector<Server> &currentServers, std::vector<int> &ServerIds, VirtualMachine &vm,
                       Server &min_server, int op, int server_now_id) {
    int ret = UNABLE;
    int cpuCost = vm.cupNum >> vm.is_double;
    int memCost = vm.memoryNum >> vm.is_double;
    double p0 = 1.0 * input->all_cpu[currentDay] / (input->all_cpu[currentDay] + input->all_mem[currentDay]);
    double p1 = 1.0 * input->all_mem[currentDay] / (input->all_cpu[currentDay] + input->all_mem[currentDay]);
    if (op == FIT) {
        double min_score = INTMAX_MAX;
        switch (vm.is_double) {
            case 0:
                for (int i = 0; i < ServerIds.size(); i++) {
                    Server &server = currentServers[ServerIds[i]];
                    if (server.fit(server.A, cpuCost, memCost)) {
                        int last_cpu = server.A.first - cpuCost;
                        int last_mem = server.A.second - memCost;
                        double last_cpu_ratio = (double) last_cpu / (double) (last_cpu + last_mem);
                        double last_mem_ratio = (double) last_mem / (double) (last_cpu + last_mem);
//                        if (4 / func(p0, last_cpu_ratio, p1, last_mem_ratio) >= abs(last_cpu - last_mem)) {
                        int cur_score = count_cpu_mem(last_cpu, last_mem);
                        if (server.requestIDs.empty()) cur_score *= server.serverInfo.dailyCost;//todo:
                        if (cur_score < min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_A;
                        }
//                        }
                    }
                    if (server.fit(server.B, cpuCost, memCost)) {
                        int last_cpu = server.B.first - cpuCost;
                        int last_mem = server.B.second - memCost;
                        double last_cpu_ratio = (double) last_cpu / (double) (last_cpu + last_mem);
                        double last_mem_ratio = (double) last_mem / (double) (last_cpu + last_mem);
//                        if (4 / func(p0, last_cpu_ratio, p1, last_mem_ratio) >= abs(last_cpu - last_mem)) {
                        int cur_score = count_cpu_mem(last_cpu, last_mem);
                        if (server.requestIDs.empty()) cur_score *= server.serverInfo.dailyCost;//todo:
                        if (cur_score < min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_B;
                        }
//                        }
                    }
                }
                break;
            case 1://double
                for (int i = 0; i < ServerIds.size(); i++) {
                    Server &server = currentServers[ServerIds[i]];
                    int lasta_cpu = server.A.first - cpuCost;
                    int lasta_mem = server.A.second - memCost;
                    double lasta_cpu_ratio = (double) lasta_cpu / (double) (lasta_cpu + lasta_mem);
                    double lasta_mem_ratio = (double) lasta_mem / (double) (lasta_cpu + lasta_mem);
                    int lastb_cpu = server.B.first - cpuCost;
                    int lastb_mem = server.B.second - memCost;
                    double lastb_cpu_ratio = (double) lastb_cpu / (double) (lastb_cpu + lastb_mem);
                    double lastb_mem_ratio = (double) lastb_mem / (double) (lastb_cpu + lastb_mem);
                    if (!server.fit(server.A, cpuCost, memCost) ||
                        4 / func(p0, lasta_cpu_ratio, p1, lasta_mem_ratio) < std::abs(lasta_cpu - lasta_mem))
                        continue;
                    if (!server.fit(server.B, cpuCost, memCost) ||
                        4 / func(p0, lastb_cpu_ratio, p1, lastb_mem_ratio) < std::abs(lastb_cpu - lastb_mem))
                        continue;

                    {
                        int cur_score = std::min(lasta_cpu * lasta_cpu + lasta_mem * lasta_mem,
                                                 lastb_cpu * lastb_cpu + lastb_mem * lastb_mem);;
                        if (server.requestIDs.empty()) cur_score *= server.serverInfo.dailyCost;//todo:
                        if (cur_score < min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_AB;
                        }
                    }
                }
        }
    } else {
        double min_score = 0;
        auto getScore = [&](Server &server, int choice) -> double {
            return server.getCpuUsage() * 0.6 + server.getMemoryUsage() * 0.4;
//            int m = 0, c = 0;
//            switch (choice) {
//                case DEPLOY_A:
//                    m = server.A.second - vm.memoryNum;
//                    c = server.A.first - vm.cupNum;
//                    break;
//                case DEPLOY_B:
//                    m = server.B.second - vm.memoryNum;
//                    c = server.B.first - vm.cupNum;
//                    break;
//                case DEPLOY_AB:
//                    m = server.B.second + server.A.second - vm.memoryNum;
//                    c = server.B.first + server.A.first - vm.cupNum;
//                    break;
//
//            }
//            return (m * 1.0 / (server.serverInfo.memoryNum)) * (m * 1.0 / (server.serverInfo.memoryNum)) +
//                   (c * 1.0 / server.serverInfo.cupNum) * (c * 1.0 / server.serverInfo.cupNum);
        };
        int size = ServerIds.size();
        switch (vm.is_double) {
            case 0:
                for (int i = 0; i < size; i++) {
                    Server &server = currentServers[ServerIds[i]];
                    if (server.fit(server.A, cpuCost, memCost)) {
                        double cur_score = getScore(server, DEPLOY_A);
                        if (cur_score > min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_A;
                        }
                    }
                    if (server.fit(server.B, cpuCost, memCost)) {
                        double cur_score = getScore(server, DEPLOY_B);
                        if (cur_score > min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_B;
                        }
                    }
                }
                break;
            case 1:
                for (int i = 0; i < size; i++) {
                    Server &server = currentServers[ServerIds[i]];
                    if (server.fit(server.A, cpuCost, memCost) && server.fit(server.B, cpuCost, memCost)) {
                        double cur_score = getScore(server, DEPLOY_AB);
                        if (cur_score > min_score) {
                            min_score = cur_score;
                            min_server = server;
                            ret = DEPLOY_AB;
                        }
                    }
                }
        }
    }
    return ret;
}

void Helper::allocatedServer(Server &server, int coreCost, int memCost, int type) {
    switch (type) {
        case DEPLOY_A:
            server.A.first -= coreCost;
            server.A.second -= memCost;
            break;
        case DEPLOY_B:
            server.B.first -= coreCost;
            server.B.second -= memCost;
            break;
        case DEPLOY_AB:
            server.A.first -= coreCost;
            server.B.first -= coreCost;
            server.A.second -= memCost;
            server.B.second -= memCost;
            break;
    }
}

void Helper::deleteVirtualHost(int serverID, int Type, VirtualMachine &virtualHost) {
    // delete VM from ret: [serverID,deploy type]
    Server &server = currentServers[serverID];
    int coreCost = virtualHost.cupNum >> virtualHost.is_double;
    int memCost = virtualHost.memoryNum >> virtualHost.is_double;
    allocatedServer(server, -1 * coreCost, -1 * memCost, Type);
}

void Helper::distribute() {
    std::vector<Request> requests = input->allRequests[currentDay];
    int previousServerNum = currentServers.size();
//    if (currentDay % 2)
    int migrationTime = migrate();
//input->addRequests[currentDay].size() > -1
//input->addRequests.size() > 300
    if (input->addRequests[currentDay].size() > 300) {
        std::sort(requests.begin(), requests.end(), [&](Request &left, Request &right) {
            if (left.operation != right.operation) {
                return left.operation < right.operation;
            }
            return input->vmLists[input->reqID2VmTypeID[left.vmOnlyID]].qrt_sum() >
                   input->vmLists[input->reqID2VmTypeID[right.vmOnlyID]].qrt_sum();
//            return input->vmName2VM[left.virtualHostName].qrt_sum()
//                   > input->vmName2VM[right.virtualHostName].qrt_sum();
        });
    }
    int n = (int) currentServers.size();
    std::vector<int> serverIDs(n);
    iota(serverIDs.begin(), serverIDs.end(), 0);
    for (auto &request:requests) {
        if (request.operation == ADD) {
//            auto vm = input->vmName2VM[request.virtualHostName];
            auto vm = input->vmLists[input->reqID2VmTypeID[request.vmOnlyID]];
            vmsid.emplace_back(request.vmOnlyID);
            ++currentVMNum;
            int ret;
            Server server;
            ret = findServer(currentServers, serverIDs, vm, server, FIT, 0);
            if (ret != UNABLE) {
                fitServer(currentServers[server.id], request, vm, ret);
            } else {
                ServerInfo serverType = buyServer(request);
                serversToBuy[serverType.name]++;
                Server server = Server(serverType, currentServers.size());
                currentServers.emplace_back(server);
                deleted_machine.push_back(false);
                serverIDs.push_back(serverIDs.size());
                vector<int> temp = {serverIDs.back()};
//                ret = findServer(currentServers, temp, vm, server1, FIT);
                int res;
                if (vm.is_double == 0) res = DEPLOY_A;
                else res = DEPLOY_AB;
                fitServer(currentServers[currentServers.size() - 1], request, vm, res);
            }
        } else {
            reqId2Del[request.vmOnlyID]++;
//            auto vm = input->vmName2VM[request.virtualHostName];
            auto vm = input->vmLists[input->reqID2VmTypeID[request.vmOnlyID]];
            --currentVMNum;
            //currentVmInfo[vm.name]--;
            auto &ret = request2ServerInfo[request.vmOnlyID];
            int serverID = ret.first;
            int deployType = ret.second;
            deleted_machine[serverID] = true;
            deleteVirtualHost(serverID, deployType, vm);
            Server &server = currentServers[serverID];
            server.requestIDs.erase(request.vmOnlyID);
        }
    }
    vmsid.erase(std::remove_if(vmsid.begin(), vmsid.end(), [&](int id) {
        return reqId2Del[id] > 0;
    }), vmsid.end());
//重新映射id
    {
        addOutput("(purchase, " + std::to_string(serversToBuy.size()) + ")");
        int currentServerNum = currentServers.size();
        int currentID = previousServerNum;
        for (int i = previousServerNum; i < currentServerNum; i++) {
            Server &server = currentServers[i];
            if (server.remappedID == -1) {
                int num = serversToBuy[server.serverInfo.name];
                serverCost += server.serverInfo.hardwareCost * (ULL) num;
                addOutput("(" + server.serverInfo.name + ", " + std::to_string(num) + ")");
                for (int j = i; j < currentServerNum; j++) {
                    if (server.serverInfo.name == currentServers[j].serverInfo.name) {
                        currentServers[j].remappedID = currentID++;
                    }
                }
            }
        }
    }
    //输出迁移信息
    {
        serversToBuy.clear();
        addOutput("(migration, " + std::to_string(input->mig_info.size()) + ")");
        for (auto &s:input->mig_info) {
            string &&s_str = "(" + std::to_string(s.from_id) + ", " + std::to_string(s.to_id) + s.apply_info;
            addOutput(std::move(s_str));
        }
        input->mig_info.clear();
    }
    // 输出add操作对应的分配序列
    {
        for (Request &request : input->allRequests[currentDay]) {
            if (request.operation == ADD) {
                auto &ret = request2ServerInfo[request.vmOnlyID];
                Server &server = currentServers[ret.first];
                std::string tmpStr = "(" + std::to_string(server.remappedID);
                if (ret.second == DEPLOY_AB) {
                    addOutput(tmpStr + ")");
                } else if (ret.second == DEPLOY_A) {
                    addOutput(tmpStr + ", A)");
                } else {
                    addOutput(tmpStr + ", B)");
                }
            }
        }
    }
    // compute daily cost
    for (Server &server:currentServers) {
        if (!server.requestIDs.empty()) powerCost += server.serverInfo.dailyCost;
    }
}

int Helper::try_fit(Server &server, const VirtualMachine &virtualHost) {
    bool fit = false;
    int coreCost = virtualHost.cupNum >> virtualHost.is_double,
            memoryCost = virtualHost.memoryNum >> virtualHost.is_double;
    if (virtualHost.is_double) {
        fit = server.fit(server.A, coreCost, memoryCost)
              && server.fit(server.B, coreCost, memoryCost);
        if (fit) {
            return DEPLOY_AB;
        }
    } else {
        if (server.fit(server.A, coreCost, memoryCost) && server.fit(server.B, coreCost, memoryCost)) {
            if (((double) (server.A.first - coreCost) + (double) (server.A.second - memoryCost)) /
                ((double) (server.serverInfo.cupNum / 2) +
                 (double) (server.serverInfo.memoryNum / 2)) <
                ((double) (server.B.first - coreCost) + (double) (server.B.second - memoryCost)) /
                ((double) (server.serverInfo.cupNum / 2) +
                 (double) (server.serverInfo.memoryNum / 2))) {
                return DEPLOY_B;

            } else {
                return DEPLOY_A;
            }
        } else {
            fit |= server.fit(server.A, coreCost, memoryCost);
            if (fit) {
                return DEPLOY_A;
            } else {
                fit |= server.fit(server.B, coreCost, memoryCost);
                if (fit) {
                    return DEPLOY_B;
                }
            }
        }

    }
    return UNABLE;
}

int Helper::migrate() {

    int maxMigrateOp = 3 * currentVMNum / 100;
//    // if (currentDay == totalDays / 3) maxMigrateOp = currentVMNum;
//    if(flag&&currentDay>1 && input->allRequests[currentDay].size()>5*input->allRequests[currentDay-1].size())  {
//        maxMigrateOp = currentVMNum;
//        flag = false;
//    }
    if (maxMigrateOp == 0) return 0;
    int n = (int) currentServers.size();
    std::vector<int> serverIDs(n);
    iota(serverIDs.begin(), serverIDs.end(), 0);
//    for (int l = 0; l < n; l++) {
//        serverIDs.emplace_back(
//                make_pair(l, currentServers[l].getCpuUsage()  + currentServers[l].getMemoryUsage()));
//    }
    sort(serverIDs.begin(), serverIDs.end(), [&](int &l, int &r) {
//        return currentServers[l].getCpuUsage() * 2.1 + currentServers[l].getMemoryUsage() <
//               currentServers[r].getCpuUsage() * 2.1 + currentServers[r].getMemoryUsage();
        return currentServers[l].getLeftCpu() * 2.1 + currentServers[l].getLeftMem() >
               currentServers[r].getLeftCpu() * 2.1  + currentServers[r].getLeftMem();
//        return std::log(currentServers[l].getLeftCpu() * 2.3)  +  std::log(currentServers[l].getLeftMem()) >
//                std::log(currentServers[r].getLeftCpu() * 2.3)  +  std::log(currentServers[r].getLeftMem());
    });
    std::unordered_map<int, bool> isMigrate;
    std::vector<std::vector<int>> vmServerFitList(input->vmLists.size());
    for (int i = 0; maxMigrateOp && i < n; i++) {
        int from_ServerID = serverIDs[i];
        Server &fromServer = currentServers[from_ServerID];
        if (isMigrate[from_ServerID]) continue;
        std::vector<int> migratedRequestIDs;
        double cpuRatio = fromServer.getCpuUsage();
        double memRatio = fromServer.getMemoryUsage();
//         if (  cpuRatio > 0.996) continue;
        for (int vmOnlyID:fromServer.requestIDs) {
            auto originalResult = request2ServerInfo[vmOnlyID];
            //try fit
            VirtualMachine &virtualHost = input->vmLists[input->reqID2VmTypeID[vmOnlyID]];
            int coreCost = virtualHost.cupNum >> virtualHost.is_double;
            int memCost = virtualHost.memoryNum >> virtualHost.is_double;
            auto solveForServer = [&]() -> bool {
                Server toServer;
                int ret;
                ret = findServer(currentServers, serverIDs, virtualHost, toServer, MIGRATE, from_ServerID);
                if (toServer.id == fromServer.id)
                    return false;

                if (ret != UNABLE) {
                    // if ( toServer.getLeftCpu()  > fromServer.getLeftCpu() ||  toServer.getLeftMem() > fromServer.getLeftMem()) return false;
                    allocatedServer(currentServers[toServer.id], coreCost, memCost, ret);
                    isMigrate[toServer.id] = true;
                    migratedRequestIDs.emplace_back(vmOnlyID);
                    currentServers[toServer.id].requestIDs.insert(vmOnlyID);
                    request2ServerInfo[vmOnlyID] = {toServer.id, ret};
                    maxMigrateOp--;
                    std::string s;
                    if (ret == DEPLOY_A) {
                        s += ", A";
                    } else if (ret == DEPLOY_B) s += ", B";
                    s += ")";
                    input->mig_info.emplace_back(
                            std::move(MigInfo({vmOnlyID, currentServers[toServer.id].remappedID, s})));
                    deleteVirtualHost(from_ServerID, originalResult.second, virtualHost);
                    return true;
                }
            };
            solveForServer();
            if (maxMigrateOp == 0) break;
        }
        for (int x:migratedRequestIDs) fromServer.requestIDs.erase(x);
    }
    return 0;

}

void Helper::dailySolve(int day) {
    currentDay = day;
    distribute();
}

void Helper::displayCost() {
    std::cout << "ServerCost: " << serverCost << "\nPowerCost: " << powerCost << "\nTotal: "
              << serverCost + powerCost
              << std::endl;
}


