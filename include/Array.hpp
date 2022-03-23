/*
The MIT License (MIT)

Copyright (c) 2013 Sumanth V

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef ARRAY_HPP
#define ARRAY_HPP
#include <iostream>

class ARRAY_OUT_OF_RANGE {
};
template <typename T>
class LatticeArray {
public:
    //*****************************
    //Constructors and Destructors
    LatticeArray() : len(0),maxLen(1) {
        array=new T[maxLen];
        allocated=true;
    }
    LatticeArray(unsigned length, T&content=0) : len(length),maxLen(length*2) {
        array=new T[maxLen];
        allocated=true;
        for(unsigned int i=0; i<len; i++) {
            array[i]=content;
        }
    }
    ~LatticeArray() {
        delete[] array;
    }
    //**************************************
    //capacity related functions
    unsigned int size() {
        return len;
    }
    unsigned int capacity() {
        return maxLen;
    }

    void resize(unsigned int newSize, const T& content=0) {
        if(newSize==len) {
            return;
        }
        T* temp= new T[newSize];
        if(newSize>len) { //if newSize is more than len, then copy the previous array, and append content to the array
            int oldLen=len;
            len=newSize;
            checkMax();
            for(unsigned int i=0; i<oldLen; i++) {
                temp[i]=array[i];
            }
            for(unsigned int i=oldLen; i<newSize; i++) {
                temp[i]=content;
            }
        }
        if(newSize<len) { // if newSize is less than len, then just copy the previous array till the length
            for(unsigned int i=0; i<newSize; i++) {
                temp[i]=array[i];
            }
        }
        delete[] array;
        array=temp;
        len=newSize;
    }
    bool empty() const {
        return len==0;
    }
    void reserve(unsigned int newCapacity) {
        if(newCapacity>maxLen) {
            maxLen=newCapacity;
            T * temp = new T[maxLen];
            for(unsigned int i=0; i< maxLen; i++) {
                temp[i]=array[i];
            }
            delete [] array;
            array=temp;
        }
    }
    void shrink_to_fit() {
        maxLen=len;
        T*temp = new T[len];
        for(unsigned int i=0; i<len; i++) {
            temp[i]=array[i];
        }
        delete[] array;
        array=temp;

    }
    //********************************************
    //Element Access
    T& operator[](unsigned int i) {
        return array[i];
    }
    T& at(unsigned int index) throw(ARRAY_OUT_OF_RANGE) {
        if(index>=len) {
            std::cerr<<"Bad index " << index << std::endl;
            ARRAY_OUT_OF_RANGE a;
            throw a ;
        }
        return array[index];
    }
    const T& front() {
        return array[0];
    }
    const T& back() {
        return array[len-1];
    }
    T* data() {
        return array;
    }
    //**********************************************
    //Modifiers
    void push_back(const T& content) {
        checkMax();
        array[len]=content;
        len++;
    }

    void pop_back() {
        len--;
        array[len]=0;
    }

    void erase(unsigned int index, unsigned int length=1) {
        T*temp=new T[len-length];
        for(unsigned int i=0; i<index; i++) {
            temp[i]=array[i];
        }
        for(unsigned int i=index+length; i<len; i++) {
            temp[i-length]=array[i];
        }
        delete[] array;
        array=temp;
        len=len-length;

        if(maxLen>len*2) {
            maxLen=len*2;
            temp=new T[len*2];
            for(unsigned int i=0; i<len; i++) {
                temp[i]=array[i];
            }
            delete[] array;
            array=temp;
        }
    }

    void clear() {
        delete array;
        maxLen=1;
        array=new T[maxLen];
        len=0;
    }

    //*******************************************
private:
    T* array;
    unsigned int len;
    unsigned int maxLen;
    bool allocated;

    void checkMax() {
        if(len==maxLen) {
            //allocate more memory for the array
            maxLen*=2;
            T*temp=new T[maxLen];
            for(unsigned int i=0; i<len; i++) {
                temp[i]=array[i];
            }
            delete [] array;
            array=temp;
        }
    }
};

#endif // ARRAY_HPP
