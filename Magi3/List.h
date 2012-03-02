/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: List.h
 Desc: The base class for a optimal list implementation. 
 
***************************************************************/

// Inclusion guard
#ifndef __LIST_H_
#define __LIST_H_

// Includes
#include "MUtil.h"

// The base class for a optimal list implementation
template <typename Type> class List
{
public:

    // Constructor (The minimum list size is 0)
    List(int size = 0)
    {
        // Create the new list
        listData = NULL;
        listSize = 0;

        // Initialize the data
        Resize(size);
    }
    
    // Copy constructor (used for deep copy)
    List(const List& OriginalObj)
    {
        // Allocate enough of the original object
        listSize = OriginalObj.listSize;
        listData = new Type[listSize];
        
        // Copy over data
        for(int i = 0; i < listSize; i++)
            listData[i] = OriginalObj.listData[i];
    }
    
    // Copy constructor (via assignment)
    List& operator=(const List& OriginalObj) 
    {
        // Allocate enough of the original object
        listSize = OriginalObj.listSize;
        listData = new Type[listSize];
        
        // Copy over data
        for(int i = 0; i < listSize; i++)
            listData[i] = OriginalObj.listData[i];
        
        // Return self
        return *this;
    }

    // Destructor
    ~List()
    {
        // Remove the data list
        if(listData != NULL)
            delete[] listData;
    }

    // Array access operator overloaded. If out of bounds, throws Err_OutOfBounds from ErrCode.h
    Type& operator[](int index)
    {
        // Exit on empty data error
        UtilAssert(listSize > 0, "Empty data set memory access.");

        // Exit on out of bounds access
        UtilAssert(index >= 0 && index < listSize, "Index %d is out of bounds.", index);

        // Return data
        return listData[index];
    }

    // Resize the data without loss (Unless we cut it short)
    void Resize(int newSize)
    {
        // If new size is invalid, empty the dataset
        if(newSize <= 0)
        {
            listSize = 0;
            if(listData != NULL)
                delete[] listData;
            listData = NULL;
        }
        else
        {
            // Create the new data array
            Type *newData = new Type[newSize];
            
            // Copy over the original data
            //memcpy((void*)newData, (void*)listData, sizeof(Type) * min(newSize, listSize));
            for(int i = 0; i < min(listSize, newSize); i++)
                newData[i] = listData[i];
            
            // Remove the old list, set the new data pointer and size
            if(listData != NULL)
                delete[] listData;
            
            // Save list data and list size
            listData = newData;
            listSize = newSize;
        }
    }

    // Returns the size of the list
    int GetSize()
    {
        return listSize;
    }

private:

    Type *listData;    // Data
    int listSize;    // List size
};

#endif
