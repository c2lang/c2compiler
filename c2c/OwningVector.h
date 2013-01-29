#ifndef OWNING_VECTOR_H
#define OWNING_VECTOR_H

#include <stdlib.h>

template <typename T>
class OwningVector {
public:
    OwningVector(int initial = 32)
        : m_capacity(initial)
        , m_size(0)
    {
        m_data = (T**)malloc(sizeof(T*)*m_capacity);
    }
    OwningVector(OwningVector& rhs)
        : m_capacity(rhs.m_capacity)
        , m_size(rhs.m_size)
        , m_data(rhs.m_data)
    {
        rhs.m_capacity = 0;
        rhs.m_size = 0;
        rhs.m_data = 0;
    }
    ~OwningVector () {
        if (m_data) {
            for (int i=0; i<m_size; i++) delete m_data[i];
            free(m_data);
        }
    }
    T* operator[] (int index) {
        assert(index < m_size);
        return m_data[index];
    }
    void push_back(T* s) {
        if (m_size == m_capacity) {
            if (m_capacity == 0) m_capacity = 4;
            else m_capacity *= 2;
            T** data2 = (T**)malloc(sizeof(T*)*m_capacity);
            memcpy(data2, m_data, m_size*sizeof(T*));
            // data = data2
        }
        m_data[m_size] = s;
        m_size++;
    }
    bool empty() const { return m_size == 0; }
    int size() const { return m_size; }
private:
    int m_capacity;
    int m_size;
    T** m_data;
};

#endif

