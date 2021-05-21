#include "Helper.h"
#include "input.h"

using namespace std;
// #define TEST
const std::string testFilePath = "../data/training-1.txt";

int main() {
#ifdef TEST
    std::freopen(testFilePath.c_str(), "r", stdin);

#endif
    Input *input = new Input();
    int N = input->inputServer();
    int M = input->inputVM();
    input->T = input->inputNum();
    input->K = input->inputNum();
    Helper helper(input, input->T);
    for (int day = 0; day < input->K - 1 ; day++) {
        input->inputRequests();
    }
    int day = 0;
    for (; day < input->T - input->K + 1 ; day++) {
#ifdef TEST
        cout << "day:" << day << endl;
#endif
        input->inputRequests();
        helper.dailySolve(day);
        for (auto &s:input->res)
            std::cout << s << "\n";
        input->res.clear();
    }
    for (; day < input->T; day++) {
        helper.dailySolve(day);
        for (auto &s:input->res)
            std::cout << s << "\n";
        input->res.clear();
    }
    #ifdef TEST
    helper.displayCost();
#endif
    fflush(stdout);
    return 0;
}
