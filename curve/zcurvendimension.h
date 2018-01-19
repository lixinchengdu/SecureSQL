#pragma once

#include <cstdlib>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include <assert.h>

#include "bigint.h"

using BitString = std::vector<bool>;
using Point = std::vector<BitString>;
using Bigint = Dodecahedron::Bigint;

struct QueryHyperCube {
    Point start;
    Point end;
};

struct Range{
    Bigint start;
    Bigint end;
};

BitString zCurveEncode(const std::string& pattern, const std::vector<BitString>& bitStrings);

std::vector<Range> getZCurveRanges(const std::string& pattern, uint8_t highstBit, const QueryHyperCube& queryHyperCube);

void zCurveCodePrint (const BitString& zCurveCode);

std::vector<Range> mergeRanges (const std::vector<Range>& left, const std::vector<Range>& right);

void inline printRange (Range range) {
    std::cout << "[" << range.start << ", " << range.end << "]" << std::endl;
}

void inline printBitString(const BitString& bitString) {
    for (const auto& b : bitString) {
        std::cout << b;
    }
    std::cout << std::endl;
}

Bigint BitStringToBigint (const BitString& bitString);