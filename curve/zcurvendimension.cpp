#include "zcurvendimension.h"

#include <algorithm>

BitString zCurveEncode(const std::string& pattern, const std::vector<BitString>& bitStrings) {
    BitString result;
    
    std::vector<size_t> bitCount (bitStrings.size(), 0);
    for (int i = 0; i < pattern.length(); ++i) {
        int index = pattern[i] - '0'; 
        assert(index >= 0 && index < bitStrings.size());
        result.push_back(bitStrings[index][bitCount[index]]);
        ++bitCount[pattern[i]-'0'];
    }

    return result;
}

static void appendRange(std::vector<Range>& result, const Range& range) {
    if (result.empty()) {
            result.push_back(range);
    } else {
        if (result.back().end + 1 == range.start) {
            result.back().end = range.end;
        } else {
            result.push_back(range);
        }
    }
}

std::vector<Range> mergeRanges (const std::vector<Range>& left, const std::vector<Range>& right) {
    if (right.size() > left.size()) {
        return mergeRanges (right, left);
    }

    std::vector<Range> result;

    int i = 0;
    int j = 0;
    const Range* processRange= NULL;
    while (i < left.size() && j < right.size()) {
        if (left[i].start < right[j].start) {
            processRange = &left[i++];
        } else {
            processRange = &right[j++];
        }
        appendRange(result, *processRange);
    }
    if (i == left.size()) {
        appendRange(result, right[j++]);
        if (j < right.size()) {
            result.insert(result.end(), right.begin() + j, right.end());
        }
    } else {
        appendRange(result, left[i++]);
        if (i < left.size()) {
            result.insert(result.end(), left.begin() + i, left.end());
        }
    }

    return result;

}

void zCurveCodePrint (const BitString& zCurveCode) {
    for (const auto& b : zCurveCode) {
        std::cout << b;
    }
    std::cout << std::endl;
}

Bigint BitStringToBigint (const BitString& bitString) {
    Bigint init = 0;
    for (auto b : bitString) {
        init = init * 2 + b;
    }
    return init;
}

static void cutLowBits (BitString& bitString, int pos) {
    int bitLen = bitString.size();
    bitString[bitLen - 1 - pos] = 0;
    for (int i = bitLen - pos; i < bitLen; ++i) {
        bitString[i] = 1;
    }
}

static void cutHighBits (BitString& bitString, int pos) {
    int bitLen = bitString.size();
    bitString[bitLen - 1 - pos] = 1;
    for (int i = bitLen - pos; i < bitLen; ++i) {
        bitString[i] = 0;
    }
}

static std::pair<QueryHyperCube, QueryHyperCube> cutBox(std::string pattern, char hyperPlaneDimension, const std::vector<int>& hyperPlaneCutIndex, QueryHyperCube queryHyperCube) {
    QueryHyperCube queryLow = queryHyperCube;
    QueryHyperCube queryHigh = queryHyperCube;

    std::vector<int> bitCount (hyperPlaneCutIndex.size(), 0);
    for (const char& c : pattern) {
        ++bitCount[c - '0'];
    }

    #ifdef DEBUG
    std::cout << "hyperPlaneDimension:" << hyperPlaneDimension << std::endl;
    std::cout << bitCount[hyperPlaneDimension - '0'] - 1 - hyperPlaneCutIndex[hyperPlaneDimension - '0'] << std::endl;
    #endif

    cutLowBits(queryLow.end[hyperPlaneDimension - '0'], bitCount[hyperPlaneDimension - '0'] - 1 - hyperPlaneCutIndex[hyperPlaneDimension - '0']);
    cutHighBits(queryHigh.start[hyperPlaneDimension - '0'], bitCount[hyperPlaneDimension - '0'] - 1 - hyperPlaneCutIndex[hyperPlaneDimension - '0']);

    #ifdef DEBUG
    std::cout << "queryLow:";
    printBitString(queryLow.end[0]);
    printBitString(queryLow.end[1]);
    printBitString(queryLow.end[2]);
    #endif

    return {queryLow, queryHigh};
}


std::vector<Range> getZCurveRanges(const std::string& pattern, uint8_t highstBit, const QueryHyperCube& queryHyperCube) {
    BitString zCurveCodeLow = zCurveEncode(pattern, queryHyperCube.start);
    BitString zCurveCodeHigh = zCurveEncode(pattern, queryHyperCube.end);

    if (zCurveCodeLow > zCurveCodeHigh) {
        std::swap(zCurveCodeLow, zCurveCodeHigh);
    }

    #ifdef DEBUG
    std::cout << "zCurveCodeLow:";
    printBitString (zCurveCodeLow);

    std::cout << "zCurveCodeHigh:";
    printBitString (zCurveCodeHigh);
    #endif

    int dimension = queryHyperCube.start.size();

    int index = 0;
    char hyperPlaneDimension = '0';
    std::vector<int> hyperPlaneCutIndex (dimension, -1);
    for (int i = 0; i < highstBit; ++i) {
        hyperPlaneDimension = pattern[i];
        ++hyperPlaneCutIndex[hyperPlaneDimension - '0'];

        index = i;
        if (zCurveCodeLow[i] != zCurveCodeHigh[i]) {
            break;
        }
    }

    // check if this is a single point
    if (index >= highstBit - 1 && zCurveCodeLow.back() == zCurveCodeHigh.back()) {
        //return {{zCurveCodeLow, zCurveCodeHigh}};
        return {{BitStringToBigint(zCurveCodeLow), BitStringToBigint(zCurveCodeHigh)}};
    }

    // check if the range only contains one continous curve
    bool allOnes = true;
    bool allNulls = true;

    for (int i = 0; i < highstBit - index; ++i) {
        if (!zCurveCodeHigh[i+index]) {
            allOnes = false;
            break;
        }
    }

    for (int i = 0; i < highstBit - index; ++i) {
        if (zCurveCodeLow[i+index]) {
            allNulls = false;
            break;
        }
    }

    if (allOnes && allNulls) {
        Bigint low = BitStringToBigint(zCurveCodeLow);
        Bigint high = BitStringToBigint(zCurveCodeHigh);

        #ifdef DEBUG
        std::cout << "low:" << low << std::endl;
        std::cout << "high:" << high << std::endl;
        #endif

       return {{low, high}};
    }

    auto queryBoxes = cutBox (pattern, hyperPlaneDimension, hyperPlaneCutIndex, queryHyperCube);
    
    #ifdef DEBUG
        
    #endif
    
    auto rangeLeft = getZCurveRanges (pattern, highstBit, queryBoxes.first);
    auto rangeRight = getZCurveRanges (pattern, highstBit, queryBoxes.second);
    

    return mergeRanges(rangeLeft, rangeRight);
    
}