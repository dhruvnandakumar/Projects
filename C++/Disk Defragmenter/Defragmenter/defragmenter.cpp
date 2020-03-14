//
//  defragmenter.cpp
//  Defragmenter
//
//  Created by Dhruv Nandakumar on 11/17/17.
//  Copyright © 2017 Dhruv Nandakumar. All rights reserved.
//  Defragmenter:  V 2.0
//
#include "defragmenter.h"
#include "DefragRunner.h"
#include "mynew.h"
#include "ManageRAM.h"
Defragmenter::Defragmenter(DiskDrive *diskDrive)
{
    
    DiskRAM *RAMStorage = new DiskRAM;
    placementHandler *diskStroage = new placementHandler(diskDrive);
    IndexHash *indexTracker = new IndexHash(-1, 500000);
    
    int fileNum = diskDrive->getNumFiles();
    int start_index = 2;
    DiskBlock* file;
    DiskBlock* displaced;
    int current = 0;
    int block_ID = 0;
    int storage_index = 0;
    
    //Setup complete
    

    
    
    for(int i = 0; i<fileNum; i++) //Defrag Every File
    {
        
        
        block_ID = diskDrive->directory[i].getFirstBlockID();
        diskDrive->directory[i].setFirstBlockID(start_index);
        
        
        do
        {
            //Read First
            
            
            if(block_ID<start_index) //Handled in RAM or placement handler
            {
                current = indexTracker->find(block_ID);
                
                if(current >= 0) //In RAM Storage
                {
                    file = RAMStorage->remove(current);
                    indexTracker->remove(block_ID);
                    
                }
                else //In Disk stored elsewhere
                {
                    //NOTE: All negative because find() of disk storage returns negative index (SEG FAULT due to bad access)
                    
                    
                    block_ID *= -1;
                    
                    
                    while(indexTracker->find(-current) != -1)
                    {
                        
                        indexTracker->remove(-block_ID);
                        block_ID = current;
                        current = indexTracker->find(-block_ID);
                        if(!(current<0))
                            break;
                        
                    }
                    
                    if(current<0) //Everything in HDD
                    {
                        file = diskDrive->readDiskBlock(-current);
                        
                        if(current != start_index)
                            diskStroage->freeBlock(-current); //Add the index it came from to the list of open spots
                        
                    }
                    else //Moved more than once, but lastly to RAM
                    {
                        file = RAMStorage->remove(current);
                        
                    }
                    
                    
                    indexTracker->remove(-block_ID);
                    
                    block_ID *= -1;
                    
                }
                
                
            }
            else //Block is in its initialised position
            {
                file = diskDrive->readDiskBlock(block_ID);
                
                if(block_ID != start_index)
                    diskStroage->freeBlock(block_ID);
                
            }
            
            ///Read complete, now write
            
            if(start_index != block_ID)
            {//We need to write this somewhere
                
                
                if(diskDrive->FAT[start_index])
                { //There will be a displaced block
                    
                    displaced = diskDrive->readDiskBlock(start_index);
                    
                    if(RAMStorage->isAvailable()) //Can put the disk in RAM
                    {
                        
                        storage_index = RAMStorage->insert(displaced);//Put in RAM and get index
                        
                    }
                    else //Place displaced back in disk
                    {
                        storage_index = diskStroage->placeBlock(displaced, block_ID);
                        delete displaced;
                    }
                    
                    
                    indexTracker->insert(start_index, storage_index); //Put inserted index in hash table
                    
                }
                
                block_ID = file->getNext();
                
                if(block_ID != 1)
                    file->setNext(start_index+1);
                
                diskDrive->writeDiskBlock(file, start_index);
                diskDrive->FAT[start_index] = true;
                diskStroage->ActualSize--; //Decrement the number of available indices (DEBUGGING ONLY)
                
                delete file;
                
            }
            else
            {
                block_ID = file->getNext();
                
                if(block_ID != 1)
                    file->setNext(start_index+1);
            }
        
            start_index++;
            
            
            
        }while(block_ID != 1);
        
    }
    
    delete RAMStorage;
    delete diskStroage;
    delete indexTracker;
    
    
} // Defragmenter()






















