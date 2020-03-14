#ifndef _BINARY_HEAP_H_
#define _BINARY_HEAP_H_

#include "dsexceptions.h"
#include "vector.h"
#include "mynew.h"

// BinaryHeap class
//
// CONSTRUCTION: with a negative infinity sentinel and
//               optional capacity (that defaults to 100)
//
// ******************PUBLIC OPERATIONS*********************
// void insert( x )       --> Insert x
// deleteMin( minItem )   --> Remove (and optionally return) smallest item
// Comparable findMin( )  --> Return smallest item
// bool isEmpty( )        --> Return true if empty; else false
// bool isFull( )         --> Return true if full; else false
// void makeEmpty( )      --> Remove all items
// ******************ERRORS********************************
// Throws Underflow and Overflow as warranted

class BinaryHeap
{
public:
    explicit BinaryHeap( int capacity = 100 );
    
    bool isEmpty( ) const;
    bool isFull( ) const;
    const int & findMin( ) const;
    
    void insert( const int & x );
    void deleteMin( );
    void deleteMin( int & minItem );
    void makeEmpty( );
    
private:
    int                currentSize;  // Number of elements in heap
    vector<int> array;        // The heap array
    
    void buildHeap( );
    void percolateDown( int hole );
};

#endif
