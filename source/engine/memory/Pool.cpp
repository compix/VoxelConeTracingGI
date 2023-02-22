#include "Pool.h"

BasePool::BasePool(std::size_t elemSize, std::size_t blockCapacity, std::size_t initialPoolCapacity)
    : m_elemSize(elemSize), m_blockCapacity(blockCapacity), m_size(0), m_capacity(0) { reserve(initialPoolCapacity); }

BasePool::~BasePool()
{
    // Free up all allocated blocks
    for (char* block : m_blocks)
        delete[] block;
}
