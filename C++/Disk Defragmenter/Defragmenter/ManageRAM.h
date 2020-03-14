//
//  ManageRAM.hpp
//  Defragmenter
//
//  Created by Dhruv Nandakumar on 11/17/17.
//  Copyright © 2017 Dhruv Nandakumar. All rights reserved.
//

#ifndef ManageRAM_hpp
#define ManageRAM_hpp


#include <iostream>
#include "DefragRunner.h"
#include "defragmenter.h"
#include "mynew.h"
#include "BinaryHeap.h"

//NOTE: Return values are negative for placementHandler


class DiskRAM //Array of disk blocks in RAM
{
    friend class DiskDrive;
    
public:
    int currentSize;
    int firstEmpty;
    DiskBlock** diskArray = new DiskBlock*[7000];//Array
    BinaryHeap* free_index = new BinaryHeap(7000);
    
    DiskRAM();
    int insert(DiskBlock* to_insert); //Returns index of disk block in array
    DiskBlock* remove(int index); //Returns the Diskblock located in diskArray[index], adds index to free_index;
    bool isAvailable(){return currentSize<7000;}
    
}; 

class placementHandler
{
    friend class DiskDrive;
    friend class IndexHash;
    
public:
    
    int ActualSize;
    int lastEmpty;
    DiskDrive* HDD;

    
    placementHandler(DiskDrive *drive);
    int placeBlock(DiskBlock* to_place, int block_ID); //Returns Index of placement
    void freeBlock(int block_ID); //Updates FAT Table and adds index to heap
    int findLastEmpty(int block_ID);
    
    
};


class IndexHash
{
    friend class DiskDrive;
    
public:
    
    explicit IndexHash( int nf, int size);
    
    
    struct HashEntry
    {
        
        int storage_index;
        int block_ID;
        
        HashEntry(int loc = 0, int block = 0)
        :storage_index(loc), block_ID(block) {}
    };
    
    vector<HashEntry> array;
    int notfound;
    int currentSize;
    
    
    int find(int block_ID);
    void insert(int block_ID, int index);
    void remove(int block_ID);
    bool isActive( int current);
    int hash(int key, int tableSize) { return key%tableSize; }
    int findPos(int block_ID);
    
};




#endif /* ManageRAM_hpp */








