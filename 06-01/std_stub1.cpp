#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <numeric>

int stub1()
{
    std::vector<int> v(1000);
    std::iota(v.begin(), v.end(), 1);
    std::map<std::string, int> m;
    for (int x : v) m[std::to_string(x)] = x;
    return m.size();
}
