#include "zcurve.h"
#include <iostream>
#include <bitset>
#include <vector>

static uint8_t getCutLowBits (uint8_t source, int position) {
    return ((1 << position) - 1) | (source & (~(1 << (position + 1) - 1)));
}

static uint8_t getCutHighBits (uint8_t source, int position) {
    #ifdef DEBUG
    std::cout << "source:" << std::bitset<8>(source) << std::endl;
    std::cout << "position:" << position << std::endl;
    #endif
    return (1 << position) | (source & (~((1 << (position + 1)) - 1)));
}

static std::pair<QueryRec, QueryRec> cutBox(std::string pattern, char hyperPlaneDimension, std::vector<int> hyperplaneCutIndex, QueryRec queryRec) {
    uint8_t x1Low = queryRec.xLow;
    uint8_t y1Low = queryRec.yLow;
    uint8_t x1High = queryRec.xHigh;
    uint8_t y1High = queryRec.yHigh;

    uint8_t x2Low = queryRec.xLow;
    uint8_t y2Low = queryRec.yLow;
    uint8_t x2High = queryRec.xHigh;
    uint8_t y2High = queryRec.yHigh;

    uint8_t xBits = 0;
    uint8_t yBits = 0;

    for (const char c : pattern) {
        if (c == '0') {
            ++xBits;
        } else {
            ++yBits;
        }
    }

    #ifdef DEBUG
    std::cout << "before x1High:" << std::bitset<8>(x1High) << std::endl; 
    std::cout << "before x2Low:" << std::bitset<8>(x2Low) << std::endl;
    std::cout << "before y1High:" << std::bitset<8>(y1High) << std::endl; 
    std::cout << "before y2Low:" << std::bitset<8>(y2Low) << std::endl;
    std::cout << "dimension:" << hyperPlaneDimension << std::endl;
    #endif

    if (hyperPlaneDimension == '0') {
        x1High = getCutLowBits(x1High, xBits -1 - hyperplaneCutIndex[0]);
        x2Low = getCutHighBits(x2High, xBits -1 - hyperplaneCutIndex[0]);
    } else {
        y1High = getCutLowBits(y1High, yBits - 1 - hyperplaneCutIndex[1]);
        y2Low = getCutHighBits(y2High, yBits - 1 - hyperplaneCutIndex[1]);
    }
    #ifdef DEBUG
    std::cout << "after:" << std::bitset<8>(x1High) << std::endl;
    std::cout << "after:" << std::bitset<8>(x2Low) << std::endl;  
    std::cout << "after:" << std::bitset<8>(y1High) << std::endl;
    std::cout << "after:" << std::bitset<8>(y2Low) << std::endl;  
    #endif

    return {{x1Low, y1Low, x1High, y1High}, {x2Low, y2Low, x2High, y2High}};
}

uint16_t zCurveEncode16(std::string pattern, uint8_t x, uint8_t y){
    uint16_t result = 0;
    uint8_t xBits = 0;
    uint8_t yBits = 0;

    for (const char c : pattern) {
        if (c == '0') {
            ++xBits;
        } else {
            ++yBits;
        }
    }
    for (const char c : pattern) {
        result = result << 1;
        if (c == '0') {
            result |= (x >> --xBits) & 1;
        } else {
            result |= (y >> --yBits) & 1;
        }
    }
    return result;
}

std::vector<uint8_t> zCurveDecode16(std::string pattern, uint16_t zCode) {
    size_t totalBits = pattern.length();
    uint8_t x = 0;
    uint8_t y = 0;
    for (int i = 0 ; i < totalBits; ++i) {
        
        if (pattern[i] == '0') {
            x = x << 1;
            x |= (zCode >> (totalBits - i - 1)) & 1;
        } else {
            y = y << 1;
            y |= (zCode >> (totalBits - i - 1)) & 1;
        }
        #ifdef DEBUG
        std::cout << "x:" << std::bitset<8> (x) << std::endl;
        std::cout << "y:" << std::bitset<8> (y) << std::endl;
        #endif 
    }
    return {x,y};
}

std::vector<Range> getZCurveRanges(std::string pattern, uint8_t highstBit, QueryRec queryRec) {
    uint8_t xLow = queryRec.xLow;
    uint8_t yLow = queryRec.yLow;
    uint8_t xHigh = queryRec.xHigh;
    uint8_t yHigh = queryRec.yHigh;

    uint16_t zCodeLow = zCurveEncode16(pattern, xLow, yLow);
    uint16_t zCodeHigh = zCurveEncode16(pattern, xHigh, yHigh);

    #ifdef DEBUG
    std::cout << "zCodeLow:" << std::bitset<16> (zCodeLow) << std::endl;
    std::cout << "zCodeHigh:" << std::bitset<16> (zCodeHigh) << std::endl;
    #endif

    int index = 0;
    char hyperPlaneDimension = '0';
    std::vector<int> hyperplaneCutIndex {-1, -1};
    for (int i = 0; i < highstBit; ++i) {
        hyperPlaneDimension = pattern[i];
        #ifdef DEBUG
        std::cout << "loop dimension:" << hyperPlaneDimension << std::endl;
        #endif
        if (hyperPlaneDimension == '0') {
            ++hyperplaneCutIndex[0];
        } else {
            ++hyperplaneCutIndex[1];
        }
        index = i;
        if (((zCodeLow^zCodeHigh) >> (highstBit-1-i)) & 1) {
            break;
        }
    }

    // check if this is a single point
    if (index >= highstBit - 1) {
        return {{zCodeLow, zCodeHigh}};
    }

    // check if the range only contains one continous curve
    bool allOnes = true;
    bool allNulls = true;

    for (int i = 0; i < highstBit; ++i) {
        if (!((zCodeHigh >> i) & 1)) {
            allOnes = false;
            break;
        }
    }

    for (int i = 0; i < highstBit; ++i) {
        if ((zCodeLow >> i) & 1) {
            allNulls = false;
            break;
        }
    }
    
    if (allOnes && allNulls) {
        return {{zCodeLow, zCodeHigh}};
    }

    auto queryBoxes = cutBox(pattern, hyperPlaneDimension, hyperplaneCutIndex, queryRec);
    std::cout << "left" << std::endl;
    auto rangeLeft = getZCurveRanges(pattern, highstBit, queryBoxes.first);
    std::cout << "right" << std::endl;
    auto rangeRight = getZCurveRanges(pattern, highstBit, queryBoxes.second);
    
    auto lastLeft = rangeLeft.back().end;
    auto firstRight = rangeRight.front().start;

    if (firstRight - lastLeft == 1) {
        rangeLeft.back() = {rangeLeft.back().start, rangeRight.front().end};
    } else {
        rangeLeft.push_back (rangeRight.front());
    }
    rangeLeft.insert(rangeLeft.end(), rangeRight.begin() + 1, rangeRight.end());

    return rangeLeft;
}


