#pragma once

#include <cstdint>
#include <map>

class FreeList {
public:
    FreeList();
    explicit FreeList(uint32_t size);

    void Resize(uint32_t size);
    uint32_t Allocate();
    void Free(uint32_t offset);

    uint32_t Size() const { return m_size; }
    uint32_t FreeCount() const { return m_freeCount; }

private:
    // 未使用の領域を格納する
    // key : offset
    // val : size
    std::map<uint32_t, uint32_t> m_freeBlocks;
    
    uint32_t m_size;
    uint32_t m_freeCount;
};