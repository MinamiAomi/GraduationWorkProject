#include "FreeList.h"

#include <cassert>

FreeList::FreeList() :
    m_size(0),
    m_freeCount(0) {
}

FreeList::FreeList(uint32_t size) {
    Resize(size);
}

void FreeList::Resize(uint32_t size) {
    m_freeBlocks.clear();
    m_freeBlocks[0] = size;
    m_size = size;
    m_freeCount = size;
}

uint32_t FreeList::Allocate() {
    assert(m_freeCount <= m_size);

    auto firstBlock = m_freeBlocks.begin();
    if (firstBlock == m_freeBlocks.end() ||
        m_freeCount == 0) {
        return m_size;  // 空きがない
    }
    uint32_t offset = firstBlock->first;

    uint32_t blockBegin = firstBlock->first + 1;
    uint32_t blockEnd = firstBlock->first + firstBlock->second;
    m_freeBlocks.erase(firstBlock);
    if (blockBegin != blockEnd) {
        m_freeBlocks[blockBegin] = blockEnd - blockBegin;
    }

    --m_freeCount;
    return offset;
}

void FreeList::Free(uint32_t offset) {
    assert(offset < m_size);

#ifdef _DEBUG
    assert(!IsFree(offset) && "Double Free detected!");
#endif

    uint32_t blockBegin = offset;
    uint32_t blockEnd = blockBegin + 1;

    auto nextBlock = m_freeBlocks.upper_bound(offset);
    auto prevBlock = nextBlock;

    if (prevBlock != m_freeBlocks.begin()) {
        --prevBlock;
    }
    else {
        prevBlock = m_freeBlocks.end();
    }

    ++m_freeCount;

    if (prevBlock != m_freeBlocks.end() &&
        blockBegin == prevBlock->first + prevBlock->second) {
        blockBegin = prevBlock->first;

        m_freeBlocks.erase(prevBlock);
    }
    if (nextBlock != m_freeBlocks.end() &&
        blockEnd == nextBlock->first) {
        blockEnd = nextBlock->first + nextBlock->second;

        m_freeBlocks.erase(nextBlock);
    }

    m_freeBlocks[blockBegin] = blockEnd - blockBegin;
}

bool FreeList::IsFree(uint32_t offset) const {
    auto it = m_freeBlocks.upper_bound(offset);
    if (it != m_freeBlocks.begin()) {
        --it;
        uint32_t begin = it->first;
        uint32_t end = begin + it->second;
        return offset >= begin && offset < end;
    }
    return false;
}

