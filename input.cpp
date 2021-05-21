#include "input.h"

#define TEST

std::vector<std::string> Input::split(std::string &str, char c) {
    auto res = std::vector<std::string>();
    size_t pos;
    while ((pos = str.find(c)) != std::string::npos) {
        res.emplace_back(str.substr(0, pos));
        str = str.substr(pos + 1);
    }
    res.emplace_back(str);
    return res;
}

Input::Input() {}

int fast_atoi(const char *str) {
    ++str;
    int val = 0;
    while (*str) {
        val = val * 10 + (*str++ - '0');
    }
    return val;
}

int Input::inputServer() {
    int serverTypeNum;
    std::cin >> serverTypeNum;
    std::cin.ignore(1024, '\n');
    for (int i = 0; i < serverTypeNum; ++i) {
        std::cin.ignore();
        std::string str;
        getline(std::cin, str);
        str.pop_back();
        auto datas = split(str, ',');
        std::string name = datas[0];
        int cpuNum = fast_atoi(datas[1].c_str());
        int memoryNum = fast_atoi(datas[2].c_str());
        int hardwareCost = fast_atoi(datas[3].c_str());
        int dailyCost = fast_atoi(datas[4].c_str());
        serverLists.push_back({name, cpuNum, memoryNum, hardwareCost, dailyCost, i});
    }
    return serverTypeNum;
}

int Input::inputVM() {
    int vncnt;
    std::cin >> vncnt;
    std::cin.ignore(1024, '\n');
    //read virtual hosts types
    for (int i = 0; i < vncnt; ++i) {
        std::cin.ignore();
        std::string str;
        getline(std::cin, str);
        str.pop_back();
        auto datas = split(str, ',');
        std::string name = datas[0];
        int cupNum = fast_atoi(datas[1].c_str());
        int memoryNum = fast_atoi(datas[2].c_str());
        int is_double = fast_atoi(datas[3].c_str());
        vmLists.push_back({i, name, cupNum, memoryNum, is_double});
        vmName2typeId[name] = i;
    }
    return vncnt;
}

int Input::inputNum() {
    int ret;
    std::cin >> ret;
    return ret;
}

int Input::inputRequests() {
    std::vector<Request> requests;
    std::vector<Request> add;
    vector<int> del_index_today;
    int requestNum;
    int cpu_today = all_cpu.empty() ? 0 : all_cpu.back();
    int mem_today = all_mem.empty() ? 0 : all_mem.back();
    std::cin >> requestNum;
    std::cin.ignore(1024, '\n');
    for (int i = 0; i < requestNum; ++i) {
        std::cin.ignore();
        std::string str;
        getline(std::cin, str);
        str.pop_back();
        auto datas = split(str, ',');
        int operation = (datas[0] == "add") ? ADD : DEL;
        std::string vmName;
        int vmOnlyID;
        // int vm_type_index = 0;
        if (operation == ADD) {
            vmName = datas[1].substr(1);
            vmOnlyID = fast_atoi(datas[2].c_str());
            int vm_type_index = vmName2typeId[vmName];
            reqID2VmTypeID[vmOnlyID] = vm_type_index;
//            reqID2vmName[vmOnlyID] = vmName;
            mem_today += vmLists[vm_type_index].is_double ? vmLists[vm_type_index].memoryNum / 2
                                                          : vmLists[vm_type_index].memoryNum ;
            cpu_today += vmLists[vm_type_index].is_double ? vmLists[vm_type_index].cupNum / 2
                                                          : vmLists[vm_type_index].cupNum ;
            add.push_back({i, operation, vmName, vmOnlyID});
        } else {
            //delete
            del_index_today.push_back(i);
            vmOnlyID = fast_atoi(datas[1].c_str());
            del_day[vmOnlyID] = allRequests.size();
//            vmName = reqID2vmName[vmOnlyID];
            int vm_type_index = reqID2VmTypeID[vmOnlyID];
            mem_today -= vmLists[vm_type_index].is_double ? vmLists[vm_type_index].memoryNum / 2
                                                          : vmLists[vm_type_index].memoryNum ;
            cpu_today -= vmLists[vm_type_index].is_double ? vmLists[vm_type_index].cupNum / 2
                                                          : vmLists[vm_type_index].cupNum ;
        }

        requests.push_back({i, operation, vmName, vmOnlyID});
    }
    del_index_today.push_back(requests.size());
    all_del_index.emplace_back(del_index_today);
    addRequests.emplace_back(add);
    all_cpu.emplace_back(cpu_today);
    all_mem.emplace_back(mem_today);
    allRequests.emplace_back(requests);
    return requestNum;
}
