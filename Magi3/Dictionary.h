/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Dictionary.h
 Desc: A simple template list hash-table (dictionary) implementation. Keys
 are string based. Linked-list implemented for collisions. The hash
 algorithm is based off of the jenkins-once-at-a-time hash. Read
 about it here: http://en.wikipedia.org/wiki/Hash_table
 
***************************************************************/

// Inclusion guard
#ifndef __DICTIONARY_H_
#define __DICTIONARY_H_

// Includes
#include "MUtil.h"
#include "List.h"
#include "Stack.h"

// A dicionary linked-list node
template <typename Type> struct DictionaryNode
{
    DictionaryNode(const char *givenKey, Type givenData)
    {
        // Copy the given data and set the next pointer to null
        key = new char[strlen(givenKey) + 1];
        strcpy(key, givenKey);
        data = givenData;
        next = NULL;
    }

    ~DictionaryNode()
    {
        delete[] key;
    }

    Type data;                    // The data object
    char *key;                    // The original non-hashed key (used for comparison in key-conflict)
    DictionaryNode<Type> *next;   // The pointer to the next node
};

// Dictionary class; A simple hash-table implementation
template <typename Type> class Dictionary
{
public:

    // Constructor. Size is the initial size of the table.
    Dictionary(int size = DEFAULT_PAGE_SIZE, int RehashBound = DEFAULT_REHASH_BOUND)
        : data(size)
    {
        // Set the data list pointers to null
        for(int i = 0; i < data.GetSize(); i++)
            data[i] = NULL;
        
        // Get the rehash count
        rehashConflictCount = RehashBound;
        elemCount = 0;
    }
    
    // Destructor
    ~Dictionary()
    {
        // For each linked-list
        for(int i = 0; i < data.GetSize(); i++)
        {
            // For each element in the linked-list
            DictionaryNode<Type> *current = data[i];

            while(current != NULL)
            {
                DictionaryNode<Type> *next = current->next;
                delete current;
                current = next;
            }
        }
    }
    
    // Insert an element
    void Insert(const char* key, Type obj)
    {
        // Let us check that we do not already have this data, to prevent repeats
        if(Find(key, &obj))
            return;
        
        // Get the key
        unsigned int keyIndex = CreateKey(key);
        
        // Create a DictionaryNode
        DictionaryNode<Type> *newObj = new DictionaryNode<Type>(key, obj);
        
        // Find the next valid spot in this chain (May 16: Optimized with pointer usage)
        int depth = 0;
        if(data[keyIndex % data.GetSize()] == NULL)
            data[keyIndex % data.GetSize()] = newObj;
        // Key-collision
        else
        {
            // Find the end of list to append to, but count our depth
            DictionaryNode<Type> *current = data[keyIndex % data.GetSize()];
            
            while(current->next != NULL)
            {
                current = current->next;
                depth++;
            }
            
            // We found a valid pointer to place our new object
            current->next = newObj;
        }
        
        // Grow the element counter
        elemCount++;
        
        // Check if we need to rehash
        if(depth >= rehashConflictCount)
            Rehash();
    }
    
    // Delete an element
    void Delete(const char* key)
    {
        // Get the key
        unsigned int keyIndex = CreateKey(key);
        
        // Find the next valid spot in this chain
        DictionaryNode<Type> *current = data[keyIndex % data.GetSize()];
        DictionaryNode<Type> *prev = NULL;
        
        // Bugfix on the 25th, October, 2011; assignment, not comparison
        while(current != NULL)
        {
            // Check for match
            if(strcmp(current->key, key) == 0)
            {
                // Matched: Remove this current node
                
                // This is the parent of the list, update parent location
                if(prev == NULL)
                {
                    DictionaryNode<Type> *next = current->next;
                    delete current;
                    data[keyIndex % data.GetSize()] = next;
                }
                // Remove current node and fix previous and next nodes
                else
                {
                    prev->next = current->next;
                    delete current;
                }

                // Remove the number from the list and return
                elemCount--;
                return;
            }

            // Set the previous nodes
            prev = current;
            current = current->next;
        }
        
        // Nothing found, thus nothing removed
    }
    
    // Find an element. Return the true if found, and fills the given obj with the data, else returns false and leaves obj alone
    bool Find(const char* key, Type* obj = NULL)
    {
        // Get the key
        unsigned int keyIndex = CreateKey(key);
        
        // Find the next valid spot in this chain
        DictionaryNode<Type> *current = data[keyIndex % data.GetSize()];
        
        while(current != NULL)
        {
            // Check for a match
            if(strcmp(current->key, key) == 0)
            {
                // Copy over the data
                if(obj != NULL)
                    *obj = current->data;
                return true;
            }

            // Look at the next current
            current = current->next;
        }

        // No match, return false
        return false;
    }

    // Returns the number of elements
    int GetCount()
    {
        return elemCount;
    }
    
    // Returns a copy of the element at the given index
    // True if found, false otherwise
    // Warning, this function should never be used during insertion and deletions
    bool GetItem(int index, Type* obj)
    {
        // Check for out of bounds
        if(index < 0 || index >= GetCount())
            return false;
        
        // Main key
        int keyindex = 0;
        
        // For each dictionary element...
        for(int i = 0; i < data.GetSize(); i++)
        {
            // Keep going through
            DictionaryNode<Type>* current = data[i];
            while(current != NULL)
            {
                // Index match?
                if(keyindex == index)
                {
                    // Copy over the data
                    *obj = current->data;
                    return true;
                }
                
                // Look at the next current
                current = current->next;
                keyindex++;
            }
        }
        
        // No match, return false
        return false;
    }

private:

    // Generate an index by a given key (String length). Based off of the Jenkins-one-at-a-time-hash
    unsigned int CreateKey(const char* key)
    {
        unsigned int hash = 0;
        for(unsigned int i = 0; i < strlen(key); i++)
        {
            hash += key[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        
        // Return the hash index for the given data size
        return hash;
    }

    // Re-hash the entire table
    void Rehash()
    {
        // Create a stack for all data
        Stack<DictionaryNode<Type>*> stack;
        
        // Collect all data and push into the stack
        for(int i = 0; i < data.GetSize(); i++)
        {
            // Find the next valid spot in this chain
            DictionaryNode<Type> *current = data[i];

            while(current != NULL)
            {
                // Push current on stack and set current to next
                stack.Push(current);
                current = current->next;
            }
        }
        
        // Grow the data list and clear it out (by a page)
        for(int i = 0; i < data.GetSize(); i++)
            data[i] = NULL;
        data.Resize(data.GetSize() + DEFAULT_PAGE_SIZE);
        
        // Pop back in the data
        // Bugfix: October 25th 2011, looks like another missing'!'
        elemCount = 0;
        while(!stack.IsEmpty())
        {
            DictionaryNode<Type>* node = stack.Pop();
            Insert(node->key, node->data);
            //delete node;// Yeah, Magi3 desperatly needs a re-write
        }
    }
    
    // Define the default page size
    static const int DEFAULT_PAGE_SIZE = 16;
    
    // Define the number of same-key conflicts before a full re-hash
    static const int DEFAULT_REHASH_BOUND = 4;
    
    int elemCount;                      // The number of elements in this dictionary
    int rehashConflictCount;            // The number of conflicts before a rehash is called
    List<DictionaryNode<Type>*> data;   // The data array of linked list pointers of the same data
};

#endif
