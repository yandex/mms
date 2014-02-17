
/*
 * example.cpp -- a short example of mms usage
 *
 * Copyright (c) 2011-2014 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru>
 *
 * This file is part of mms, the memory-mapped storage library.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <mms/vector.h>
#include <mms/string.h>
#include <mms/map.h>
#include <mms/writer.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

template<class P>
struct My {
    int i;
    mms::string<P> str;
    mms::map<P, mms::string<P>, int> map;

    // Expose struct's fields to mms
    template<class A> void traverseFields(A a) const { a(i)(str)(map); }
};

int main()
{
    // Populate the struct
    My<mms::Standalone> my;
    my.i = 22;
    my.str = "a string";
    my.map.insert(std::make_pair("ten", 10));
    my.map.insert(std::make_pair("eleven", 11));
    my.map.insert(std::make_pair("twelve", 12));
    
    // Serialize it
    std::ofstream out("filename");
    size_t pos = mms::write(out, my);
    out.close();
    
    // mmap() serialized data
    int fd = ::open("filename", O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    char* data = (char*) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    const My<mms::Mmapped>* pmy = reinterpret_cast<const My<mms::Mmapped>*>(data + pos);
    
    // Use the data
    std::cout << pmy->i << std::endl;
    std::cout << pmy->str << std::endl;
    std::cout << pmy->map.size() << std::endl;
    std::cout << pmy->map["twelve"] << std::endl;
}
