#pragma once

#include <cstdlib>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

struct QueryRec {
    uint8_t xLow;
    uint8_t yLow;
    uint8_t xHigh;
    uint8_t yHigh;
};

struct Coordinate {
    uint8_t x;
    uint8_t y;
};

struct Range16 {
    uint16_t start;
    uint16_t end;
};

/**
 *  @param pattern determines how bits are interleaved. "0" means bit from x and "1" means bit from y.
 */
uint16_t zCurveEncode16(std::string pattern, uint8_t x, uint8_t y);

std::vector<uint8_t> zCurveDecode16(std::string pattern, uint16_t zCode);

std::vector<Range16> getZCurveRanges16(std::string pattern, uint8_t highstBit, QueryRec queryRec); 

void inline printRange16 (Range16 range) {
    std::cout << "[" << range.start << ", " << range.end << "]" << std::endl;
}
