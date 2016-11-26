/*
    MIT License

    Copyright (c) 2017, namreeb (legal@namreeb.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#pragma once

#include <cstdlib>

class CDataStore
{
    private:
        const void * _vmt;                      // 0x00-0x04

    public:
        void * m_data;                          // 0x04-0x08
        const unsigned int m_base;              // 0x08-0x0C
        const unsigned int m_capacity;          // 0x0C-0x10
        unsigned int m_bytesWritten;            // 0x10-0x14
        unsigned int m_bytesRead;               // 0x14-0x18

        CDataStore(size_t size) : _vmt(nullptr), m_data(malloc(size)), m_base(0), m_capacity(size), m_bytesWritten(0), m_bytesRead(0) {}

        ~CDataStore()
        {
            free(m_data);
        }

        template <typename T> void Write(T);
        void Write(const void *, unsigned int);
};

template <typename T>
void CDataStore::Write(T val)
{
    Write(&val, sizeof(T));
}