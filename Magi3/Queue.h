/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Queue.h
 Desc: A template queue implementation with List. Grows by
 x2 the previous value, rather than constant growth. This is
 proven to be more helpful in memory allocations.
 
***************************************************************/

// Inclusion guard
#ifndef __QUEUE_H_
#define __QUEUE_H_

// Includes
#include "MUtil.h"
#include "List.h"

// Template queue implementation with Lists base
template <typename Type> class Queue
{
public:

    // Constructor. Size is just the inital queuesize if wanted
    Queue(int size = DEFAULT_PAGE_SIZE)
        : data(size) // Create the list structure
    {
        // Set the default variables
        front = 0;
        back = data.GetSize() - 1;
        count = 0;
    }

    // Destructor
    ~Queue()
    {
        // Nothing to do
    }
    
    // Add an element into the back of the queue.
    // Bug-fix on the front after back pointer on April 27th 2011
    Type Enqueue(Type newData)
    {
        // If we need to grow, use square-growth
        if(count >= data.GetSize())
        {
            // Grow by *2 since constant growth is too slow
            int oldSize = data.GetSize();
            data.Resize(oldSize * 2);
            
            // Move data only if the front pointer is greater than the back pointer (wrapping)
            if(front > back)
            {
                // Move all elements from the front index pointer to the end of the current page block one full new page to the right
                for(int i = 0; i < oldSize; i++)
                    data[data.GetSize() - i - 1] = data[data.GetSize() - oldSize - i - 1];
                front += oldSize;
            }
        }
        
        // Find the next index
        back = (back + 1) % data.GetSize();
        
        // Grow the last index and insert data
        data[back] = newData;
        count++;
        
        // Return the element we have added
        return newData;
    }
    
    // Remove an element from the front and return it. Returns the first element (even if garbage) if out of bounds or empty.
    Type Dequeue()
    {
        // Make sure there is data
        UtilAssert(!IsEmpty(), "Attempting to access empty queue.");

        // Return the data
        Type temp = data[front];
        front = (front + 1) % data.GetSize();
        count--; 

        // Return temp
        return temp;
    }
    
    // Peek at the front of the list (the element that would be removed if Dequeue() was called...)
    Type Peek()
    {
        // Make sure there is data
        UtilAssert(!IsEmpty(), "Attempting to access empty queue.");
        
        // Return the data
        return data[front];
    }

    // Returns the current size
    int GetSize()
    {
        return count;
    }

    // Returns true if empty (No elements)
    bool IsEmpty()
    {
        if(count <= 0)
            return true;
        return false;
    }
    
    // Returns the number of times a given object is within this queue
    int Contains(Type Obj)
    {
        int total = 0;
        for(int i = 0; i < count; i++)
            if(data[(front + i) % data.GetSize()] == Obj)
                total++;
        return total;
    }

private:

    // Define the default page size
    static const int DEFAULT_PAGE_SIZE = 16;

    List<Type> data;    // Data

    int front;            // Front queue index
    int back;            // Back queue index
    int count;            // The number of elements of the queue

    int pageSize;        // The size of a page of memory in the list (in elements)

};

#endif
