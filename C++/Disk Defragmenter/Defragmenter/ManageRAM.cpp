//
//  ManageRAM.cpp
//  Defragmenter
//
//  Created by Dhruv Nandakumar on 11/17/17.
//  Copyright © 2017 Dhruv Nandakumar. All rights reserved.
//

#include "ManageRAM.h"

using namespace std;

/////////Class DiskRAM////////////


DiskRAM::DiskRAM()
{
    
    for(int i = 0; i<7000; i++)
    {
        free_index->insert(i);
    }
    
    
    currentSize = 0000; //Set 5000 to test placementHandler
    
}

int DiskRAM::insert(DiskBlock* to_insert)
{
    
    int index = free_index->findMin();
    free_index->deleteMin();
    
    diskArray[index] = to_insert;
    
    currentSize++;
    
    return index;
}

DiskBlock*  DiskRAM::remove(int index)
{
    
    currentSize--;
    
    free_index->insert(index);
    
    return diskArray[index];
}

///////////Class PlacementHandler//////////////////

placementHandler::placementHandler(DiskDrive *drive)
{
    HDD = drive; //Set HDD to point to diskDrive
    
    int HDDSize = drive->getCapacity() -1;
    
    int count = 0;
    
    for(; ;HDDSize--)
    {
        if(!(HDD->FAT[HDDSize]))
            break;
    }
    lastEmpty = HDDSize;
    

    
    ActualSize = count;
    
}

int placementHandler::placeBlock(DiskBlock* to_place, int block_ID)
{
    int index = 0;
    do
    {
        index = findLastEmpty(block_ID);
        
    }while(HDD->FAT[index]); //Find a free index (precuation for disk writes done outside of placement handler)
    
    
    HDD->writeDiskBlock(to_place, index);
    HDD->FAT[index] = true;
    
    ActualSize--;
    
    return -index;
}

void placementHandler::freeBlock(int block_ID)
{
    HDD->FAT[block_ID] = false;
    
    ActualSize++;
    
}

int placementHandler::findLastEmpty(int block_ID)
{
    
    if(block_ID>lastEmpty)
        lastEmpty = block_ID;
    

        int oldLast = lastEmpty;
        
        for(; ;lastEmpty--)
        {
            if(!HDD->FAT[lastEmpty])
                break;
        }
        
        return oldLast;
}



/////////Class IndexHash///////////////////

IndexHash::IndexHash(int nf, int size)
: array(size), notfound(nf)
{
    currentSize = 0;
    for(int i =0; i<array.size(); i++)
    {
        array[i].storage_index = -1;
    }
}


void IndexHash::insert(int block_ID, int index)
{
    
    int current;
    
    current = findPos(block_ID);
    
    
    if(!isActive(current))
    {
        array[current] = HashEntry(index, block_ID);
        
    }
    
    
}



int IndexHash::findPos(int block_ID)
{
    int current = hash(block_ID, array.size());
    
    while( array[current].storage_index != -1 && array[current].block_ID != block_ID)
    {
        current++;
        if(current >= array.size())
            current = 0;
        
    } //Finds Available spot in hash table
    
    return current;
    
    
}


int IndexHash::find(int block_ID)
{
    
    
    int current = findPos(block_ID);
    
    
    if(isActive(current))
    {
        
        return array[current].storage_index;
    }
    else
        return notfound;
    
}

void IndexHash::remove(int block_ID)
{
    int current  = findPos(block_ID);
    
    if(isActive(current))
        array[current].storage_index = -1;
    
    
}

bool IndexHash::isActive( int current)
{
    
    if(array[current].storage_index != -1)
        return true;
    
    else
        return false;
    
}

















































