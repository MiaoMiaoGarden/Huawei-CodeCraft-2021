#include <string>
#include <vector>
#include <unordered_map>

#ifndef SDK_C_VIRTUALMACHINE_H
#define SDK_C_VIRTUALMACHINE_H

struct VirtualMachine {
    int id;
    std::string name;
    int cupNum, memoryNum, is_double;

    inline int qrt_sum() {
        int c = cupNum, m = memoryNum;
        if (is_double == 1) {
            c = c / 2;
            m = m / 2;
        }
        return c * c + m * m;
    }
//    int lastServerID;
};

#endif //SDK_C_VIRTUALMACHINE_H
