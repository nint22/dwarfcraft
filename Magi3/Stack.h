/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Stack.h
 Desc: The base class for a optimal list implementation.
 
***************************************************************/

// Inclusion guard
#ifndef __STACK_H_
#define __STACK_H_

// Includes
#include "MUtil.h"
#include "List.h"
#include "Queue.h"

// List class; A simple template implementation
template <typename Type> class Stack
{
public:

    // Constructor. PageSize is the chunks of memory allocated at run-time as needed
    Stack(int size = DEFAULT_PAGE_SIZE, int PageSize = DEFAULT_PAGE_SIZE)
        : data(size) // Create the list structure
    {
        // Set the page size
        pageSize = PageSize;
        if(pageSize <= 0)
            pageSize = DEFAULT_PAGE_SIZE;

        // Set the default variables
        top = 0;
    }

    // Destructor
    ~Stack()
    {
        // Nothing to do
    }

    // Add an element onto the top of the stack
    void Push(Type newData)
    {
        // Resize if needed
        if(top >= data.GetSize())
        {
            // Grow by a page
            data.Resize(data.GetSize() + pageSize);
        }

        // Insert data
        memcpy((void*)&data[top++], (void*)&newData, sizeof(Type));
    }

    // Remove an element from the top of the stack.
    Type Pop()
    {
        // Return the top of the stack as well as recudes the count index
        UtilAssert(top >= 1, "Attemted to pop off empty stack.");
        return data[--top];
    }

    // Returns the top-most element. If there are none, it throws an out of bounds error.
    Type Peek()
    {
        // Returns the top of the stack
        UtilAssert(top > 0, "Attemted to peek empty stack.");
        return data[top - 1];
    }

    // Returns the current size
    int GetSize()
    {
        return top;
    }

    // Returns true if empty (No elements)
    bool IsEmpty()
    {
        if(top <= 0)
            return true;
        else
            return false;
    }

    // Flips the entire stack
    void Flip()
    {
        // Flip data
        for(int i = 0; i < top / 2; i++)
        {
            // Swap i with top - i
            Type temp = data[i];
            data[i] = data[top - i - 1];
            data[top - i - 1] = temp;
        }
    }
    
    // Returns true if object is found
    bool Contains(Type Obj)
    {
        // Seek from top to bottom
        for(int i = top - 1; i >= 0; i--)
            if(data[i] == Obj)
                return true;
        
        // Else, never found
        return false;
    }

private:

    // Define the default page size
    static const int DEFAULT_PAGE_SIZE = 16;

    List< Type > data;    // Data
    int top;            // The top stack index
    int pageSize;        // The size of a page of memory to grow the list by
};

#endif
