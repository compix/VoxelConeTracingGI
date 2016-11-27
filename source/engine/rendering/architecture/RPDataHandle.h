#pragma once

class RPDataHandle
{
public:
    RPDataHandle() { }

    RPDataHandle(void* data)
        : m_data(data) { }

    void* get() const { return m_data; }

private:
    void* m_data{nullptr};
};
