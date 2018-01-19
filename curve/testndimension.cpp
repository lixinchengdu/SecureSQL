#include "zcurvendimension.h"

using namespace std;

int main(int argc, char** argv) {
    BitString x {0};
    BitString y {0};
    BitString z {1};
    auto xy = {x, y, z};
    const std::string pattern {"012"};
    auto code = zCurveEncode (pattern, xy);
    zCurveCodePrint (code);
    auto ranges = mergeRanges ({{1,2},{4,7}, {11,15}}, {{3,3}});
    for (auto& range: ranges) {
        printRange(range);
    }
    Bigint a("110010");
    cout << a << endl;

    vector<bool> bigBool (3, 1);

    //BitStringToBigint (z);
    cout << BitStringToBigint (bigBool) << endl;
    Point start = {{0},{0},{0}};
    Point end = {{1},{0},{1}};
    QueryHyperCube query {start, end};

    auto ranges1 = getZCurveRanges(pattern, pattern.length(), query); 
    for (auto& range : ranges1) {
        printRange(range);
    }
}