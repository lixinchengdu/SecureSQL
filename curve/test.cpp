#include "zcurve.h"
#include <iostream>
#include <bitset>
#include <assert.h>

using namespace std;

int main(int argc, char** argv) {
    string pattern = "0101";
    uint8_t x = 0b010;
    uint8_t y = 0b011;
    uint16_t zCode = (zCurveEncode16(pattern, x ,y));
    vector<uint8_t> xy = zCurveDecode16 (pattern, zCode);
    assert(xy[0] == x);
    assert(xy[1] == y);

    QueryRec queryRec {1 ,1, 2, 2};

    auto ranges = getZCurveRanges16(pattern, pattern.length() , queryRec);
    for (auto range : ranges) {
        printRange16 (range);
    }
}