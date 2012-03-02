/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: PriorityQueue.h
 Desc: Priority queue template class. Increasing order.
 Trivial linear implementation of O(n). Defaults invalid
 references by INT_MIN. Future implementations should either
 be based on a binary tree / heap / dictionary.
 
***************************************************************/

// Inclusion guard
#ifndef __PRIORITYQUEUE_H_
#define __PRIORITYQUEUE_H_

// Includes
#include "MUtil.h"
#include "List.h"
#include "Queue.h"
#include <limits.h>

// Priority Queue class; A simple template implementation; Ordered increasing
template <typename Type> class PriorityQueue
{
public:
    
    // Constructor. PageSize is the chunks of memory allocated at run-time as needed. Size is just the inital queuesize if wanted
    PriorityQueue(int size = DEFAULT_PAGE_SIZE, int PageSize = DEFAULT_PAGE_SIZE)
        : data(size), priority(size)
    {
        // Set the page size
        pageSize = PageSize;
        if(pageSize <= 0)
            pageSize = DEFAULT_PAGE_SIZE;
        
        // Default data
        for(int i = 0; i < size; i++)
            priority[i] = INT_MIN;
        
        // Set the default variables
        count = 0;
    }
    
    // Destructor
    ~PriorityQueue()
    {
        // Nothing to do
    }
    
    // Insert a given element based on the priority
    void Insert(int newPriority, Type newData)
    {
        // If count is greater or equal to data size, resize by a page
        if((count + 1) >= data.GetSize())
        {
            // Grow by a page
            int OldSize = data.GetSize();
            data.Resize(data.GetSize() + pageSize);
            priority.Resize(priority.GetSize() + pageSize);
            
            // Initialize these pages to invalid -MAX_INT
            for(int i = OldSize; i < data.GetSize(); i++)
                priority[i] = INT_MIN;
        }
        
        // Seek from far right to left, moving items back
        // if we don't have space. Note that the right
        // will always have spaces to place new items
        int pos = 0;
        for(pos = count - 1; pos >= 0; pos--)
        {
            // Only insert after a valid priority
            if(priority[pos] != INT_MIN && newPriority >= priority[pos])
                break;
            
            // Shift to the right
            data[pos + 1] = data[pos];
            priority[pos + 1] = priority[pos];
        }
        
        // Insert the element
        data[pos + 1] = newData;
        priority[pos + 1] = newPriority;
        
        // Grow our counter due to our insertion
        count++;
    }
    
    // Remove all elements that have this data.
    void Remove(Type newData)
    {
        // Build a queue of all index of targets to remove
        Queue<int> targets;
        for(int i = 0; i < count; i++)
        {
            if(data[i] == newData)
                targets.Enqueue(i);
        }
        
        Remove(&targets);
        
    }
    
    // Remove all elements of this priority level
    void Remove(int targetpriority)
    {
        // Build a queue of all index of targets to remove
        Queue<int> targets;
        int readpast = 0; // Optimal, stop searching early, hack
        for(int i = 0; i < count && readpast != 2; i++)
        {
            if(priority[i] == targetpriority)
            {
                targets.Enqueue(i);
                readpast = 1;
            }
            else if(readpast == 1)
                readpast = 2;
        }

        Remove(&targets);
    }

    // Remove all elements by the given queue of indexes
    void Remove(Queue<int> *targets)
    {
        // For each element index in the queue
        int moved = 0; // We must offset the target index due to the previous removes
        while(!targets->IsEmpty())
        {
            // Move everything from the right on-ward to the left over the target location
            for(int i = targets->Dequeue() - moved; i < count - 1; i++)
            {
                data[i] = data[i+1];
                priority[i] = priority[i+1];
            }
            count--;
            moved++;
        }
    }
    
    // Change the priority of the given object
    void ChangePriority(int newPriority, Type oldData)
    {
        // Remove the oldData
        Remove(oldData);

        // Add the oldData again at the new priority level
        Insert(newPriority, oldData);
    }
    
    // Returns the current size
    int GetSize()
    {
        return count;
    }
    
    // Returns true if the queue is empty
    bool IsEmpty()
    {
        if(count <= 0)
            return true;
        else
            return false;
    }
    
    // Clear out the entire queue
    void Clear()
    {
        // Set count to 0
        count = 0;
    }
    
    // Array access operator overloaded. If out of bounds, throws Err_OutOfBounds from ErrCode.h
    Type& operator[](int index)
    {
        // Exit on empty data error
        UtilAssert(count > 0, "No allocation in PriorityQueue.");

        // Exit on out of bounds access
        UtilAssert(index >= 0 && index < count, "Index %d out of bounds memory access.", index);

        // Return data
        return data[index];
    }

private:

    // Define the default page size
    static const int DEFAULT_PAGE_SIZE = 16;

    List< Type > data;        // Data list
    List< int > priority;    // Priority list (Mirrors data)
    int count;                // The number of elements of the queue

    int pageSize;            // The size of a page of memory in the list (in elements)
};

#endif
