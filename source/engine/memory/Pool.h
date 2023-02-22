#pragma once
#include <vector>
#include <assert.h>
#include <cstddef>

/**
* Pool consisting of multiple contiguous blocks of a defined number of contiguous elements which are of a predefined size.
* Blocks are not necessarily contiguous. Only the elements in the blocks are guaranteed to be contiguous in memory.
*/
class BasePool
{
public:
    BasePool(std::size_t elemSize, std::size_t blockCapacity, std::size_t initialPoolCapacity);

    virtual ~BasePool();

    void resize(std::size_t size)
    {
        if (m_size < size)
        {
            m_size = size;

            // Make sure there is enough space
            if (m_capacity < size)
                reserve(size);
        }
    }

    /**
    * The pool will reserve at least the requested amount of memory.
    * It might have to reserve more than requested because pool capacity depends on
    * the block capacity.
    */
    void reserve(std::size_t capacity)
    {
        while (m_capacity < capacity)
        {
            // Allocate memory for the block, add it to the block container and increase the capacity
            char* block = new char[m_blockCapacity * m_elemSize];
            m_blocks.push_back(block);
            m_capacity += m_blockCapacity;
        }
    }

    void* get(std::size_t idx)
    {
        assert(idx < m_size);
        // Get pointer to the corresponding block first and then add the relative offset
        return m_blocks[idx / m_blockCapacity] + m_elemSize * (idx % m_blockCapacity);
    }

    const void* get(std::size_t idx) const
    {
        assert(idx < m_size);
        // Get pointer to the corresponding block first and then add the relative offset
        return m_blocks[idx / m_blockCapacity] + m_elemSize * (idx % m_blockCapacity);
    }

    void* operator[](std::size_t idx) { return get(idx); }

    const void* operator[](std::size_t idx) const { return get(idx); }

    std::size_t size() const { return m_size; }

    std::size_t capacity() const { return m_capacity; }

    bool empty() const { return m_size == 0; }

    virtual void destroy(std::size_t idx) = 0;

protected:
    std::vector<char*> m_blocks; // Vector of all contiguous memory blocks
    const std::size_t m_elemSize; // Size in bytes of one element
    const std::size_t m_blockCapacity; // Number of elements per block
    std::size_t m_size; // Number of elements in the pool
    std::size_t m_capacity; // How many elements can fit into the pool
};

template <class T, std::size_t BlockCapacity = 8192, std::size_t InitialCapacity = 0>
class Pool : public BasePool
{
public:
    Pool() : BasePool(sizeof(T), BlockCapacity, InitialCapacity) { }

    /**
    * Calls the destructor of the specified element.
    */
    void destroy(std::size_t idx) override
    {
        assert(idx < m_size);
        // The destructor needs to be called explicitly because placement new is used
        static_cast<T*>(get(idx))->~T();
    }

    /**
    * Constructs a new element at the specified index.
    * args are the constructor parameters for the class of the pool
    */
    template <class ... Args>
    T* create(std::size_t idx, Args&& ... args)
    {
        // Placement new into the pool
        return new(get(idx)) T(std::forward<Args>(args) ...);
    }

    T& getRef(std::size_t idx) { return *static_cast<T*>(get(idx)); }

    const T& getRef(std::size_t idx) const { return *static_cast<T*>(get(idx)); }

    T* getPtr(std::size_t idx) { return static_cast<T*>(get(idx)); }

    const T* getPtr(std::size_t idx) const { return static_cast<T*>(get(idx)); }
};
