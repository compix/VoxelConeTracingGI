#include "Pool.h"

BasePool::BasePool(size_t elemSize, size_t blockCapacity, size_t initialPoolCapacity)
    : m_elemSize(elemSize), m_blockCapacity(blockCapacity), m_size(0), m_capacity(0) { reserve(initialPoolCapacity); }

BasePool::~BasePool()
{
    // Free up all allocated blocks
    for (char* block : m_blocks)
        delete[] block;
}
