/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Saam Barati. <saambarati1@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "ControlFlowProfiler.h"

#include "VM.h"

namespace JSC {

ControlFlowProfiler::ControlFlowProfiler()
    : m_dummyBasicBlock(BasicBlockLocation(-1, -1))
{
}

ControlFlowProfiler::~ControlFlowProfiler()
{
    for (const BlockLocationCache& cache : m_sourceIDBuckets.values()) {
        for (BasicBlockLocation* block : cache.values())
            delete block;
    }
}

BasicBlockLocation* ControlFlowProfiler::getBasicBlockLocation(intptr_t sourceID, int startOffset, int endOffset)
{
    auto addResult = m_sourceIDBuckets.add(sourceID, BlockLocationCache());
    BlockLocationCache& blockLocationCache = addResult.iterator->value;
    BasicBlockKey key(startOffset, endOffset);
    auto addResultForBasicBlock = blockLocationCache.add(key, nullptr);
    if (addResultForBasicBlock.isNewEntry)
        addResultForBasicBlock.iterator->value = new BasicBlockLocation(startOffset, endOffset);
    return addResultForBasicBlock.iterator->value;
}

void ControlFlowProfiler::dumpData() const
{
    auto iter = m_sourceIDBuckets.begin();
    auto end = m_sourceIDBuckets.end();
    for (; iter != end; ++iter) {
        dataLog("SourceID: ", iter->key, "\n");
        for (const BasicBlockLocation* block : iter->value.values())
            block->dumpData();
    }
}

Vector<BasicBlockRange> ControlFlowProfiler::getBasicBlocksForSourceID(intptr_t sourceID, VM& vm) const 
{
    Vector<BasicBlockRange> result(0);
    auto bucketFindResult = m_sourceIDBuckets.find(sourceID);
    if (bucketFindResult == m_sourceIDBuckets.end())
        return result;

    const BlockLocationCache& cache = bucketFindResult->value;
    for (const BasicBlockLocation* block : cache.values()) {
        bool hasExecuted = block->hasExecuted();
        const Vector<BasicBlockLocation::Gap>& blockRanges = block->getExecutedRanges();
        for (BasicBlockLocation::Gap gap : blockRanges) {
            BasicBlockRange range;
            range.m_hasExecuted = hasExecuted;
            range.m_startOffset = gap.first;
            range.m_endOffset = gap.second;
            result.append(range);
        }
    }

    const Vector<std::tuple<bool, unsigned, unsigned>>& unexecutedFunctionRanges = vm.functionHasExecutedCache()->getFunctionRanges(sourceID);
    for (const auto& functionRange : unexecutedFunctionRanges) {
        BasicBlockRange range;
        range.m_hasExecuted = std::get<0>(functionRange);
        range.m_startOffset = static_cast<int>(std::get<1>(functionRange));
        range.m_endOffset = static_cast<int>(std::get<2>(functionRange) + 1);
        result.append(range);
    }

    return result;
}

} // namespace JSC
