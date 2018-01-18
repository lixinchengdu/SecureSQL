#include "zcurve.h"
#include <iostream>
#include <bitset>
#include <assert.h>

using namespace std;

uint8_t getCutLowBits (uint8_t source, int position) {
    return ((1 << position) - 1) | (source & (~(1 << (position + 1) - 1)));
}

int main(int argc, char** argv) {
    string pattern = "0011";
    uint8_t x = 0b010;
    uint8_t y = 0b011;
    uint16_t zCode = (zCurveEncode16(pattern, x ,y));
    vector<uint8_t> xy = zCurveDecode16 (pattern, zCode);
    assert(xy[0] == x);
    assert(xy[1] == y);

    QueryRec queryRec {0 ,0, 3, 0};

    auto ranges = getZCurveRanges(pattern, pattern.length() , queryRec);
    for (auto range : ranges) {
        printRange (range);
    }
}