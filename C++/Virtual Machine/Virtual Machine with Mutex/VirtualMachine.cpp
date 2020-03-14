//
//  VirtualMachine.cpp
//  Virtual Machine
//
//  Created by Dhruv Nandakumar and Harris Zia on 10/27/18.
//


#include "VirtualMachine.h"
#include "Machine.h"
#include <queue>
#include <iostream>
#include <iterator>
#include <cstring>

using namespace std;


/////////////////GLOBAL SETUP////////////////////////////////////////////////
extern "C"
{
    TVMMainEntry VMLoadModule(const char *module);
    void VMUnloadModule(void);
    TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);
    TVMStatus VMDateTime(SVMDateTimeRef curdatetime);
    uint32_t VMStringLength(const char *str);
    void VMStringCopy(char *dest, const char *src);
    void VMStringCopyN(char *dest, const char *src, int32_t n);
    void VMStringConcatenate(char *dest, const char *src);
    TVMStatus VMFileSystemValidPathName(const char *name);
    TVMStatus VMFileSystemIsRelativePath(const char *name);
    TVMStatus VMFileSystemIsAbsolutePath(const char *name);
    TVMStatus VMFileSystemGetAbsolutePath(char *abspath, const char *curpath, const char *destpath);
    TVMStatus VMFileSystemPathIsOnMount(const char *mntpt, const char *destpath);
    TVMStatus VMFileSystemDirectoryFromFullPath(char *dirname, const char *path);
    TVMStatus VMFileSystemFileFromFullPath(char *filename, const char *path);
    TVMStatus VMFileSystemConsolidatePath(char *fullpath, const char *dirname, const char *filename);
    TVMStatus VMFileSystemSimplifyPath(char *simpath, const char *abspath, const char *relpath);
    TVMStatus VMFileSystemRelativePath(char *relpath, const char *basepath, const char *destpath);
    
}


class Thread_Control_Block
{
public:
    TVMMemorySize Memory_Size;
    TVMTick Tick_Time;
    TVMThreadID Thread_ID;
    TVMThreadPriority Priority;
    TVMThreadState State;
    TVMThreadEntry Entry_Point = NULL;
    
    void* Params = (void*) 0;
    void* Stack_Addr = NULL;
    int result;
    
    vector<TVMMutexID> Mutexes;
    vector<TVMMutexID> Mutex_Wait_List;
    
    SMachineContext Operating_Context;
};

struct Compare_Threads
{
    bool operator ()(Thread_Control_Block* Thread1, Thread_Control_Block* Thread2)
    {
        return Thread1->Priority < Thread2->Priority;
    }
};

class Memory_Block
{
public:
    void* base;
    TVMMemorySize Block_Size;
    bool used; //1 = Active, 0 = Free
};

class Memory_Pool
{
public:
    TVMMemoryPoolID Pool_ID;
    TVMMemorySize Memory_Size;
    TVMMemorySize Available_Memory;
    void* base;
    bool active;
    
    vector<Memory_Block*> Free_Blocks;
    vector<Memory_Block*> Allocated_Blocks;
    
};

class BIOS_Parameter_Block
{
    uint32_t BS_jmpBoot;
    uint8_t BS_OEM_Name;
    
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectorCount;
    
    uint8_t NumFATs;
    uint16_t RootEntryCount;
    uint16_t TotalSector16;
    
    uint8_t Media;
    
    uint16_t FATSize16;
    uint16_t SectorsPerTrack;
    uint16_t NumHeads;
    uint32_t HiddenSectors;
    
};


vector<Thread_Control_Block*> All_Threads;
vector<Thread_Control_Block*> Waiting_Threads;
vector<Thread_Control_Block*> Dead_Threads;
priority_queue<Thread_Control_Block*, vector<Thread_Control_Block*>, Compare_Threads> Ready_List; //https://www.geeksforgeeks.org/stl-priority-queue-for-structure-or-class/


Thread_Control_Block* Running_Thread = new Thread_Control_Block;
Thread_Control_Block* Main_Thread = new Thread_Control_Block;
Thread_Control_Block* Idle_Thread = new Thread_Control_Block;

int Thread_Number = 2;
unsigned int Total_Ticks = 0;
unsigned int Original_Ticks = 0;


vector<Memory_Pool*> All_Memory_Pools;

int Memory_Pool_Number = 1; //0 reserved for machine-VM shared memory, 1 for heap
const TVMMemoryPoolID VM_MEMORY_POOL_ID_SYSTEM = 1;

void* Shared_Memory;
void* heapMemory;

///////////////////////////THREAD SCHEDULER///////////////////////////////////////////////////////////////////

void Change_Thread()
{
    
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   Change_Thread"<<endl;
    
    Thread_Control_Block* Old_Thread = Running_Thread;
    
    if(Ready_List.empty())
    {
        // cout<<"       INSERTION: "<<Idle_Thread->Thread_ID<<endl;
        Ready_List.push(Idle_Thread);
    }
    
    bool right = true;
    
    do{
        
        // cout<<"   Running Thread ID: "<<Old_Thread->Thread_ID<<endl;
        // cout<<"   Q Size: "<<Ready_List.size()<<endl;
        Running_Thread = Ready_List.top(); //https://www.geeksforgeeks.org/priority_queuetop-c-stl/
        Ready_List.pop();
    }while(Running_Thread->State == VM_THREAD_STATE_DEAD);
    
    if(Running_Thread->State == VM_THREAD_STATE_READY)
    {
        
        // cout<<"   Here: "<<Running_Thread->Thread_ID<<endl;
        
        
        if(Old_Thread->Thread_ID != Running_Thread->Thread_ID)
        {
            
            // cout<<"   Switching from "<<Old_Thread->Thread_ID<<" to "<<Running_Thread->Thread_ID<<endl;
            Running_Thread->State = VM_THREAD_STATE_RUNNING;
            MachineContextSwitch(&Old_Thread->Operating_Context, &Running_Thread->Operating_Context);
        }
        else
        {
            return;
        }
    }
    MachineResumeSignals(&Machine_Signal_State);
}

//////////HELPER FUNCTIONS///////////////////////////////////////////////

void whiteout(Memory_Block* Block)
{
    // cout << "   whiteout" << endl;
    for(int i = 0; i<Block->Block_Size; i++)
    {
        *((uint8_t*)Block->base + i) = 0;
        
    }
}

Memory_Pool* Get_Memory_Pool(TVMMemoryPoolID ID)
{
    for(int i = 0; i < All_Memory_Pools.size(); i++)
    {
        if(All_Memory_Pools[i]->Pool_ID == ID)
        {
            return All_Memory_Pools[i];
        }
    }
    
    // // // cout<<"Returning null"<<endl;
    return NULL;
}

int Get_Pool_Index(Memory_Pool* Pool)
{
    // cout << "   get pool" << endl;
    for(int i = 0; i<All_Memory_Pools.size(); i++)
    {
        if(Pool->Pool_ID == All_Memory_Pools[i]->Pool_ID)
            return i;
    }
    
    return -1;
}

int Get_Block_Index(Memory_Block* Block, Memory_Pool* Pool)
{
    // cout << "   get block index" <<endl;
    for(int i = 0; i<Pool->Allocated_Blocks.size(); i++)
    {
        if(Pool->Allocated_Blocks[i]->base == Block->base)
        {
            // // // cout<<"Returning Block Index from allocated: "<<i<<endl;
            return i;
        }
    }
    
    for(int i = 0; i<Pool->Free_Blocks.size(); i++)
    {
        if(Pool->Free_Blocks[i]->base == Block->base)
        {
            // // // cout<<"Returning Block Index from free: "<<i<<endl;
            return i;
        }
    }
    
    return -1;
    
}

bool ThreadExists(TVMThreadID ID, vector<Thread_Control_Block*> Threads)
{
    // cout << "   bool thread exist" << endl;
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    bool found = false;
    
    for(int i = 0; i<Threads.size(); i++)
    {
        if(Threads[i]->Thread_ID == ID)
        {
            found = true;
            break;
        }
    }
    
    MachineResumeSignals(&Machine_Signal_State);
    return found;
    
}

int FindIndex(TVMThreadID ID, vector<Thread_Control_Block*> Threads)
{
    // cout << "   FindIndex" << endl;
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    int id = -1;
    for(int i = 0; i < Threads.size();i++)
    {
        if(Threads[i]->Thread_ID == ID)
        {
            id = i;
            break;
            
        }
    }
    MachineResumeSignals(&Machine_Signal_State);
    return id;
}

void SortWaitingThreads()
{
    // cout << "    SortWaitingThreads" << endl;
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    Thread_Control_Block* Temporary = new Thread_Control_Block;
    
    for(int i = 1; i<Waiting_Threads.size(); i++)
    {
        for(int j = 0; j<Waiting_Threads.size() - 1; j++)
        {
            if(Waiting_Threads[j]->Priority < Waiting_Threads[j +1] -> Priority)
            {
                Temporary = Waiting_Threads[j];
                Waiting_Threads[j] = Waiting_Threads[j+1];
                Waiting_Threads[j+1] = Temporary;
            }
            
        }
        
    }
    
    MachineResumeSignals(&Machine_Signal_State);
}

void sortBlocks(Memory_Pool* Pool)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    Memory_Block* Temp = new Memory_Block;
    // cout << "   sortBlocks" <<endl;
    
    for(int i = 0; i<Pool->Free_Blocks.size(); i++)
    {
        
        for(int j = 0; j<Pool->Free_Blocks.size() -1; j++)
        {
            if(Pool->Free_Blocks[j]->base > Pool->Free_Blocks[j+1]->base)
            {
                
                Temp = Pool->Free_Blocks[j];
                Pool->Free_Blocks[j] = Pool->Free_Blocks[j+1];
                Pool->Free_Blocks[j+1] = Temp;
                
            }
            
        }
    }
    for(int i = 0; i<Pool->Allocated_Blocks.size(); i++)
    {
        
        
        
        for(int j = 0; j<Pool->Allocated_Blocks.size() -1; j++)
        {
            if(Pool->Allocated_Blocks[j]->base > Pool->Allocated_Blocks[j+1]->base)
            {
                Temp = Pool->Allocated_Blocks[j];
                Pool->Allocated_Blocks[j] = Pool->Allocated_Blocks[j+1];
                Pool->Allocated_Blocks[j+1] = Temp;
                
            }
            
        }
    }
    
    MachineResumeSignals(&Machine_Signal_State);
}

//////////////////////////////////MEMORY POOL FUNCTIONS////////////////////////////////////////////////////////////

TVMStatus VMMemoryPoolCreate(void *base, TVMMemorySize size, TVMMemoryPoolIDRef memory)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout << "   VMMemoryPoolCreate" << endl;
    
    if(size == 0 || base == NULL || memory == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    //Create memmory pool
    Memory_Pool* Pool = new Memory_Pool;
    Pool->Pool_ID = ++Memory_Pool_Number;
    *memory = Pool->Pool_ID;
    
    Pool->Memory_Size = size;
    Pool->Available_Memory = size;
    Pool->base = base;
    Pool->active = true;
    
    
    Memory_Block* Block = new Memory_Block;
    Block->base = base;
    Block->Block_Size = size;
    
    Pool->Free_Blocks.push_back(Block);
    All_Memory_Pools.push_back(Pool);
    
    sortBlocks(Pool);
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
}

TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   Pooldelet"<<endl;
    
    Memory_Pool* Pool = Get_Memory_Pool(memory);
    
    if(Pool == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(Pool->Memory_Size != Pool->Available_Memory)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_STATE;
    }
    
    
    int index = Get_Pool_Index(Pool);
    
    if(index == -1)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    
    //Delete
    Pool->active = false;
    
    All_Memory_Pools.erase(All_Memory_Pools.begin() + index);
    
    
    MachineResumeSignals(&Machine_Signal_State);
    
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory, TVMMemorySizeRef bytesleft)
{
    
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   Poolquery"<<endl;
    
    Memory_Pool* Pool = Get_Memory_Pool(memory);
    
    if(Pool == NULL || bytesleft == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    
    *bytesleft = Pool->Available_Memory;
    
    MachineResumeSignals(&Machine_Signal_State);
    
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
{
    
    
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout<<"   Poolalloc:"<<memory<<endl;
    
    
    Memory_Pool* Pool = Get_Memory_Pool(memory);
    
    
    
    if(Pool == NULL || size == 0 || pointer == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(Pool->Available_Memory < size)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
    }
    
    
    //Can Allocate
    sortBlocks(Pool);
    
    TVMMemorySize Adjusted_Size = (size + 0x3F)&(~0x3F); //https://math.stackexchange.com/questions/291468/how-to-find-the-nearest-multiple-of-16-to-my-given-number-n
    
    
    bool contigious_resources = false;
    
    
    for(int i = 0; i<Pool->Free_Blocks.size(); i++)
    {
        
        if(Pool->Free_Blocks[i]->Block_Size == Adjusted_Size)
        {
            contigious_resources = true;
            
            *pointer = Pool->Free_Blocks[i]->base;
            Pool->Available_Memory -= Adjusted_Size;
            Pool->Free_Blocks[i]->used = true;
            
            Pool->Allocated_Blocks.push_back(Pool->Free_Blocks[i]);
            
            Pool->Free_Blocks.erase(Pool->Free_Blocks.begin() + i);
            
            break;
            
        }
        if(Pool->Free_Blocks[i]->Block_Size > Adjusted_Size)
        { //Have to split memory block
            
            Memory_Block* Top_Half = new Memory_Block;
            
            Top_Half->base = Pool->Free_Blocks[i]->base;
            
            Top_Half->Block_Size = Adjusted_Size;
            
            
            Top_Half->used = true;
            *pointer = Top_Half->base;
            Pool->Allocated_Blocks.push_back(Top_Half);
            
            
            Pool->Free_Blocks[i]->base = (void*)((uint8_t*)(Top_Half->base) + Adjusted_Size); //https://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c
            Pool->Free_Blocks[i]->Block_Size -= Adjusted_Size;
            
            
            
            Pool->Available_Memory -= Adjusted_Size;
            contigious_resources = true;
            break;
            
        }
        
    }
    sortBlocks(Pool);
    
    
    MachineResumeSignals(&Machine_Signal_State);
    
    if(!contigious_resources)
        return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
    
    
    return VM_STATUS_SUCCESS;
}

TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory, void *pointer)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   PoolDealloc"<<memory<<endl;
    
    Memory_Pool* Pool = Get_Memory_Pool(memory);
    
    
    if(Pool == NULL || pointer == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    sortBlocks(Pool);
    
    Memory_Block* Target_Block = NULL; //Get Target Block referenced by **pointer
    
    for(int i = 0; i<Pool->Allocated_Blocks.size(); i++)
    {
        
        if(Pool->Allocated_Blocks[i]->base == pointer)
        {
            Target_Block = Pool->Allocated_Blocks[i];
            break;
        }
    }
    
    if(Target_Block == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    int Target_Index  = Get_Block_Index(Target_Block, Pool);
    
    
    for(int i = 0; i<Pool->Free_Blocks.size(); i++)
    {
        if((void*)((uint8_t*)Pool->Free_Blocks[i]->base + Pool->Free_Blocks[i]->Block_Size) == Target_Block->base) //Upper Free Block
        {
            
            Pool->Free_Blocks[i]->Block_Size += Target_Block->Block_Size;
            Pool->Available_Memory += Target_Block->Block_Size;
            
            
            
            for(int j = i+1; j<Pool->Free_Blocks.size()-1; j++)
            {
                if((void*)((uint8_t*)Pool->Free_Blocks[i]->base + Pool->Free_Blocks[i]->Block_Size) == Pool->Free_Blocks[j]->base) //Sandwich
                {
                    
                    
                    Pool->Free_Blocks[i]->Block_Size += Pool->Free_Blocks[j]->Block_Size;
                    
                    Pool->Free_Blocks.erase(Pool->Free_Blocks.begin() + j);
                    
                    
                }
                
            }
            
            whiteout(Pool->Free_Blocks[i]);
            
            Pool->Allocated_Blocks.erase(Pool->Allocated_Blocks.begin() + Target_Index);
            
            
            
            sortBlocks(Pool);
            
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_SUCCESS;
        }
        
        if((void*)((uint8_t*)Target_Block->base + Target_Block->Block_Size) == Pool->Free_Blocks[i]->base) //Only Lower Free Block
        {
            
            
            Pool->Free_Blocks[i]->base = Target_Block->base;
            Pool->Free_Blocks[i]->Block_Size += Target_Block->Block_Size;
            Pool->Available_Memory += Target_Block->Block_Size;
            
            whiteout(Pool->Free_Blocks[i]);
            
            
            Pool->Allocated_Blocks.erase(Pool->Allocated_Blocks.begin() + Target_Index);
            
            
            sortBlocks(Pool);
            
            MachineResumeSignals(&Machine_Signal_State);
            
            return VM_STATUS_SUCCESS;
            
        }
    }
    
    //No Sandwich
    
    whiteout(Target_Block);
    
    
    Pool->Free_Blocks.push_back(Target_Block);
    Pool->Available_Memory += Target_Block->Block_Size;
    
    Pool->Allocated_Blocks.erase(Pool->Allocated_Blocks.begin() + Target_Index);
    
    
    sortBlocks(Pool);
    
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////VM SKELETON FUNCTIONS////////////////////////////////////////////////////////////////////
void VMTickHandler(void* calldata)
{
    
    // cout<<"   TickHandler"<<endl;
    Total_Ticks++;
    
    SortWaitingThreads();
    
    for(int i = 0; i<Waiting_Threads.size(); i++)
    {
        if(Waiting_Threads[i]->Tick_Time != 0)
        {
            (Waiting_Threads[i]->Tick_Time)--;
            
        }
        else
        {
            Waiting_Threads[i] -> State = VM_THREAD_STATE_READY;
            
            
            // cout<<"       INSERTION: "<<Waiting_Threads[i]->Thread_ID<<endl;
            Ready_List.push(Waiting_Threads[i]);
            
            
            Waiting_Threads.erase(Waiting_Threads.begin() + i); //https://www.geeksforgeeks.org/vector-erase-and-clear-in-cpp/
            
            SortWaitingThreads();
            
        }
    }
    
    if(!Ready_List.empty() && (Ready_List.top()->Priority >= Running_Thread->Priority))
    {
        
        // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
        Running_Thread->State = VM_THREAD_STATE_READY;
        Ready_List.push(Running_Thread);
        
        Change_Thread();
    }
    
}

void IdleProcess(void*)
{
    // cout << "   IdleProcess"<<endl;
    MachineEnableSignals();
    
    while(1);
    
}

void Thread_Skeleton(void*)
{
    // cout << "   Thread_Skeleton" << endl;
    MachineEnableSignals();
    
    Running_Thread->Entry_Point(Running_Thread->Params);
    
    VMThreadTerminate(Running_Thread->Thread_ID);
    
}

///////////////////////////////////VM THREAD FUNCTIONS/////////////////////////////////////////////////////////////////

void MakeMain()
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout << "   makemain" << endl;
    
    Main_Thread->Priority = VM_THREAD_PRIORITY_NORMAL;
    Main_Thread->State = VM_THREAD_STATE_RUNNING;
    Main_Thread->Memory_Size = 0x1000; //https://softwareengineering.stackexchange.com/questions/310658/how-much-stack-usage-is-too-much
    Main_Thread->Thread_ID = 1;
    Main_Thread->Tick_Time = 0;
    
    All_Threads.push_back(Main_Thread);
    
    MachineResumeSignals(&Machine_Signal_State);
}

void MakeIdle()
{
    TMachineSignalState Machine_Signal_State;
    
    // cout << "   MakeIdle" << endl;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    Idle_Thread->Thread_ID = 0;
    Idle_Thread->Memory_Size = 0x300000; //https://softwareengineering.stackexchange.com/questions/310658/how-much-stack-usage-is-too-much
    Idle_Thread->Priority = VM_THREAD_PRIORITY_LOW;
    Idle_Thread->State = VM_THREAD_STATE_READY;
    Idle_Thread->Tick_Time = 0;
    Idle_Thread->Entry_Point = IdleProcess;
    
    void* Stack_Address;//Allocate Memory for Stack, used to be malloc
    VMMemoryPoolAllocate(1, Idle_Thread->Memory_Size, &Stack_Address);
    
    Idle_Thread->Stack_Addr = Stack_Address;
    
    //Creat Machine Context Variable
    MachineContextCreate(&Idle_Thread->Operating_Context, IdleProcess, NULL, Stack_Address, Idle_Thread->Memory_Size);
    
    // cout<<"       INSERTION: "<<Idle_Thread->Thread_ID<<endl;
    Ready_List.push(Idle_Thread);
    
    All_Threads.push_back(Idle_Thread);
    
    MachineResumeSignals(&Machine_Signal_State);
    
}

TVMStatus VMStart(int tickms, TVMMemorySize heapsize, TVMMemorySize sharedsize, int argc, char *argv[])
{
    // cout << "   VmStart" << endl;
    Shared_Memory = MachineInitialize(sharedsize);
    
    Memory_Block* shared = new Memory_Block;
    shared->base = Shared_Memory;
    shared->Block_Size = sharedsize;
    
    
    
    Memory_Pool* Shared_Memory_Pool = new Memory_Pool;
    Shared_Memory_Pool->Pool_ID = 0;
    Shared_Memory_Pool->active = true;
    Shared_Memory_Pool->Available_Memory = sharedsize;
    Shared_Memory_Pool->base = Shared_Memory;
    Shared_Memory_Pool->Memory_Size = sharedsize;
    Shared_Memory_Pool->Free_Blocks.push_back(shared);
    
    All_Memory_Pools.push_back(Shared_Memory_Pool);
    
    heapMemory = new uint8_t[heapsize];
    Memory_Block* Block = new Memory_Block;
    Block->base = heapMemory;
    Block->Block_Size = heapsize;
    
    Memory_Pool* heap = new Memory_Pool;
    heap->Pool_ID = 1;
    heap->active = true;
    heap->Available_Memory = heapsize;
    heap->base = heapMemory;
    heap->Memory_Size = heapsize;
    heap->Free_Blocks.push_back(Block);
    
    All_Memory_Pools.push_back(heap);
    
    MachineEnableSignals();
    
    Original_Ticks = tickms;
    
    MachineRequestAlarm(tickms*1000, VMTickHandler, Running_Thread);
    
    
    
    char* Module_Name = argv[0];
    
    TVMMainEntry Main_Start = VMLoadModule(Module_Name);
    
    if(Main_Start != NULL)
    {
        MakeMain();
        MakeIdle();
        
        Running_Thread = Main_Thread;
        
        Main_Start(argc, argv);
        
        MachineTerminate();
        return VM_STATUS_SUCCESS;
    }
    else
    {
        MachineTerminate();
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
}

TVMStatus VMTickCount(TVMTickRef tickref)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMTickCount" << endl;
    if(tickref != NULL)
    {
        
        *(tickref) = Total_Ticks;
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
        
    }
    else
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
}

TVMStatus VMTickMS(int *tickmsref)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMTickMS" << endl;
    if(tickmsref != NULL)
    {
        *tickmsref = Original_Ticks;
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
        
    }
    else
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
}

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   ThreadCreate"<<endl;
    if(entry == NULL || tid == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    Thread_Control_Block* New_Thread = new Thread_Control_Block;
    New_Thread->Thread_ID = Thread_Number++;
    *tid = Thread_Number -1;
    
    New_Thread->Priority = prio;
    New_Thread->Memory_Size = memsize;
    New_Thread->Params = param;
    New_Thread->Entry_Point = entry;
    New_Thread->State = VM_THREAD_STATE_DEAD;
    New_Thread->Tick_Time = 0;
    
    void* Stack_Address; //Used to be malloc
    VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SYSTEM, memsize, &Stack_Address);
    
    New_Thread->Stack_Addr = Stack_Address;
    
    
    MachineContextCreate(&(New_Thread->Operating_Context), Thread_Skeleton, New_Thread->Params, Stack_Address, New_Thread->Memory_Size);
    
    
    Dead_Threads.push_back(New_Thread);
    All_Threads.push_back(New_Thread);
    MachineResumeSignals(&Machine_Signal_State);
    
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMThreadDelete(TVMThreadID thread)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   ThreadDelete"<<thread<<endl;
    
    bool Exists = ThreadExists(thread, All_Threads);
    bool Is_Dead = ThreadExists(thread, Dead_Threads);
    
    
    if(!Exists)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    else
    {
        if(!Is_Dead)
        {
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_ERROR_INVALID_STATE;
        }
        else
        {
            int Dead_Index = FindIndex(thread, Dead_Threads);
            int All_Index = FindIndex(thread, All_Threads);
            
            
            VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SYSTEM, All_Threads[All_Index]->Stack_Addr);
            
            Dead_Threads.erase(Dead_Threads.begin() + (Dead_Index)); //https://www.geeksforgeeks.org/vector-erase-and-clear-in-cpp/
            All_Threads.erase(Dead_Threads.begin() + (All_Index));
            
            
            
        }
    }
    
    
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMThreadID(TVMThreadIDRef threadref)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMThreadID" << endl;
    if(threadref == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    else
    {
        *threadref = Running_Thread ->Thread_ID;
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
    }
}

TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMThreadState"<<thread << endl;
    if(!ThreadExists(thread, All_Threads))
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    else if(stateref == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    else
    {
        int Index = FindIndex(thread, All_Threads);
        *stateref = All_Threads[Index]->State;
        
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
    }
}

TVMStatus VMThreadSleep(TVMTick tick)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    
    // cout<<"   ThreadSleep"<<endl;
    
    if(tick == VM_TIMEOUT_INFINITE)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(tick == VM_TIMEOUT_IMMEDIATE)
    {
        Running_Thread->State = VM_THREAD_STATE_READY;
        
        
        // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
        Ready_List.push(Running_Thread);
        Change_Thread();
        
    }
    else
    {
        Running_Thread->Tick_Time = tick;
        Running_Thread->State = VM_THREAD_STATE_WAITING;
        
        Waiting_Threads.push_back(Running_Thread);
        SortWaitingThreads();
        
        Change_Thread();
    }
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadActivate(TVMThreadID thread)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   ThreadActivate: "<<thread<<endl;
    
    if(!ThreadExists(thread, All_Threads))
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    else
    {
        if(!ThreadExists(thread, Dead_Threads))
        {
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_ERROR_INVALID_STATE;
        }
        else
        {
            int Dead_Index = FindIndex(thread, Dead_Threads);
            
            SMachineContextRef Context = new SMachineContext;
            //void* Stack_Address; //(void*)malloc(Dead_Threads[Dead_Index]->Memory_Size);
            //VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SYSTEM, Dead_Threads[Dead_Index]->Memory_Size, &Dead_Threads[Dead_Index]->Stack_Addr);
            MachineContextCreate(Context, Thread_Skeleton, Dead_Threads[Dead_Index]->Params, Dead_Threads[Dead_Index]->Stack_Addr, Dead_Threads[Dead_Index]->Memory_Size);
            
            Dead_Threads[Dead_Index]->Operating_Context = *Context;
            Dead_Threads[Dead_Index]->State = VM_THREAD_STATE_READY;
            
            // cout<<"       INSERTION: "<<Dead_Threads[Dead_Index]->Thread_ID<<endl;
            Ready_List.push(Dead_Threads[Dead_Index]);
            
            Dead_Threads.erase(Dead_Threads.begin() + Dead_Index);
            
        }
    }
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
    
}

void File_Open_Callback(void* Thread, int descriptor)
{
    // cout << "   File_Open_Callback" << endl;
    
    
    if(((Thread_Control_Block*)Thread) -> State != VM_THREAD_STATE_DEAD)
    {
        
        ((Thread_Control_Block*)Thread) -> result = descriptor;
        
        ((Thread_Control_Block*)Thread) ->State = VM_THREAD_STATE_READY;
        
        
        // cout<<"       INSERTION: "<<((Thread_Control_Block*)Thread)->Thread_ID<<endl;
        Ready_List.push((Thread_Control_Block*)Thread);
        
    }
    Running_Thread->State = VM_THREAD_STATE_READY;
    
    
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
    
    
}

TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor)
{
    TMachineSignalState Machine_Signal_State;
    
    if(filename == NULL || filedescriptor == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout << "   VMFileOpen" << endl;
    
    MachineFileOpen(filename, flags, mode, File_Open_Callback, Running_Thread);
    
    Running_Thread->State = VM_THREAD_STATE_WAITING;
    Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
    
    Change_Thread();
    
    if(Running_Thread->result < 0)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_FAILURE;
    }
    
    
    *filedescriptor = Running_Thread->result;
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
    
}

void File_Close_Callback(void* thread, int result)
{
    // cout << "   File_Close_Callback" << endl;
    ((Thread_Control_Block*)thread)->result = result;
    
    ((Thread_Control_Block*)thread)->State = VM_THREAD_STATE_READY;
    
    // cout<<"       INSERTION: "<<((Thread_Control_Block*)thread)->Thread_ID<<endl;
    Ready_List.push(((Thread_Control_Block*)thread));
    
    Running_Thread->State = VM_THREAD_STATE_READY;
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
    
    
}

TVMStatus VMFileClose(int filedescriptor)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMFileClose" << endl;
    MachineFileClose(filedescriptor, File_Open_Callback, Running_Thread);
    
    Running_Thread->State = VM_THREAD_STATE_WAITING;
    Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
    
    Change_Thread();
    
    MachineResumeSignals(&Machine_Signal_State);
    
    if(Running_Thread->result < 0)
        return VM_STATUS_FAILURE;
    else
        return VM_STATUS_SUCCESS;
    
}

void File_Read_Callback(void* thread, int result)
{
    // cout << "   File_Read_Callback" << endl;
    
    ((Thread_Control_Block*)thread)->State = VM_THREAD_STATE_READY;
    ((Thread_Control_Block*)thread)->result = result;
    // cout<<"       INSERTION: "<<((Thread_Control_Block*)thread)->Thread_ID<<endl;
    Ready_List.push(((Thread_Control_Block*)thread));
    
    
    Running_Thread->State = VM_THREAD_STATE_READY;
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
    
    
}

TVMStatus VMFileRead(int filedescriptor, void *data, int *length)
{
    TMachineSignalState Machine_Signal_State;
    if(data == NULL || length == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout << "   VMFileRead" << endl;
    
    void* Shared_Buffer;//Use memcpy?
    
    if(*length <= 512)
    {
        
        VMMemoryPoolAllocate(0, *length, &Shared_Buffer);
        
        MachineFileRead(filedescriptor, Shared_Buffer, *length, File_Open_Callback, Running_Thread);
        
        Running_Thread->State = VM_THREAD_STATE_WAITING;
        Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
        Change_Thread();
        
        memcpy(data, Shared_Buffer, *length);
        
        
        *length = Running_Thread->result;
        VMMemoryPoolDeallocate(0, Shared_Buffer);
    }
    
    else
    {
        VMMemoryPoolAllocate(0, 512, &Shared_Buffer);
        
        int Num_Sections = *length/512;
        int original = *length;
        int i;
        for(i = 0; i<Num_Sections; i++)
        {
            MachineFileRead(filedescriptor, Shared_Buffer, 512, File_Open_Callback, Running_Thread);
            Running_Thread->State = VM_THREAD_STATE_WAITING;
            Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
            Change_Thread();
            // // cout << "changed thread" << endl;
            memcpy((void*)((uint8_t*)data + 512*i), Shared_Buffer, 512);
            *length += Running_Thread->result;
            
        }
        
        VMMemoryPoolDeallocate(0, Shared_Buffer);
        
        if(original % 512 != 0)
        {
            VMMemoryPoolAllocate(0, original%512, &Shared_Buffer);
            
            MachineFileRead(filedescriptor, Shared_Buffer, original%512, File_Open_Callback, Running_Thread);
            Running_Thread->State = VM_THREAD_STATE_WAITING;
            Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
            Change_Thread();
            // // cout << "changed thread" << endl;
            memcpy((void*)((uint8_t*)data + 512*i), Shared_Buffer, original%512);
            *length += Running_Thread->result;
            
        }
        
        VMMemoryPoolDeallocate(0, Shared_Buffer);
    }
    
    
    MachineResumeSignals(&Machine_Signal_State);
    
    if(*length <0)
        return VM_STATUS_FAILURE;
    else
        return VM_STATUS_SUCCESS;
    
}

void File_Write_Callback(void* thread, int result)
{
    
    // cout<<"   FileWriteCallback"<<endl;
    
    
    
    ((Thread_Control_Block*)thread)->State = VM_THREAD_STATE_READY;
    ((Thread_Control_Block*)thread)->result = result;
    
    // cout<<"       INSERTION: "<<((Thread_Control_Block*)thread)->Thread_ID<<endl;
    Ready_List.push(((Thread_Control_Block*)thread));
    
    
    // cout<<"     Callback: "<<Running_Thread->Thread_ID<<endl;
    Running_Thread->State = VM_THREAD_STATE_READY;
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
    
}

TVMStatus VMFileWrite(int filedescriptor, void *data, int *length)
{
    // // // cout<<"File Write start"<<endl;
    TMachineSignalState Machine_Signal_State;
    
    if(data == NULL || length == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   File write"<<endl;
    
    void* Shared_Buffer;
    
    if(*length < 512)
    {
        
        // // // cout<<"Short buffer write"<<endl;
        
        VMMemoryPoolAllocate(0, *length, &Shared_Buffer);
        
        memcpy(Shared_Buffer, data, *length);
        
        MachineFileWrite(filedescriptor, Shared_Buffer, *length, File_Open_Callback, Running_Thread);
        
        // // // cout<<"File write success"<<endl;
        
        Running_Thread->State = VM_THREAD_STATE_WAITING;
        Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
        // // coutd();
        Change_Thread();
        
        
        *length = Running_Thread->result;
        
        VMMemoryPoolDeallocate(0, Shared_Buffer);
        
        // // // cout<<"Returned from dealloc"<<endl;
        
    }
    else
    {
        
        
        VMMemoryPoolAllocate(0, 512, &Shared_Buffer);
        
        int original = *length;
        int Num_Sections = original / 512;
        int i;
        
        for(i = 0; i<Num_Sections; i++)
        {
            // // cout<<i<<" "<<endl;
            
            memcpy(Shared_Buffer, (void*)((uint8_t*)data + 512*i), 512);
            
            MachineFileWrite(filedescriptor, Shared_Buffer, 512, File_Open_Callback, Running_Thread);
            
            Running_Thread->State = VM_THREAD_STATE_WAITING;
            Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
            // // coutd();
            Change_Thread();
            
            
            *length += Running_Thread->result;
            
        }
        
        VMMemoryPoolDeallocate(0, Shared_Buffer);
        
        if(original%512 != 0)
        {
            VMMemoryPoolAllocate(0, original%512, &Shared_Buffer);
            
            
            memcpy( Shared_Buffer, (void*)((uint8_t*)data + 512*i), original%512);
            
            MachineFileWrite(filedescriptor, Shared_Buffer, original%512, File_Open_Callback, Running_Thread);
            Running_Thread->State = VM_THREAD_STATE_WAITING;
            Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
            // // coutd();
            Change_Thread();
            
            
            
            *length += Running_Thread->result;
            
            VMMemoryPoolDeallocate(0, Shared_Buffer);
        }
        
    }
    
    
    MachineResumeSignals(&Machine_Signal_State);
    
    if(*length <0)
        return VM_STATUS_FAILURE;
    else
        return VM_STATUS_SUCCESS;
    
    
    
}

void File_Seek_Callback(void* thread, int result)
{
    // cout << "   File_Seek_Callback" << endl;
    
    ((Thread_Control_Block*)thread)->State = VM_THREAD_STATE_READY;
    ((Thread_Control_Block*)thread)->result = result;
    
    // cout<<"       INSERTION: "<<((Thread_Control_Block*)thread)->Thread_ID<<endl;
    Ready_List.push(((Thread_Control_Block*)thread));
    
    Running_Thread->State = VM_THREAD_STATE_READY;
    
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
}

TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset)
{
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout << "   VMFileSeek" << endl;
    MachineFileSeek(filedescriptor, offset, whence, File_Open_Callback, Running_Thread);
    
    Running_Thread->State = VM_THREAD_STATE_WAITING;
    Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
    Change_Thread();
    
    *newoffset = Running_Thread->result;
    
    MachineResumeSignals(&Machine_Signal_State);
    
    if(*newoffset < 0)
        return VM_STATUS_FAILURE;
    else
        return VM_STATUS_SUCCESS;
    
}



///////////////////////MUTEX AND THREAD TERMINATE//////////////////

class Mutex
{
public:
    TVMMutexID ID;
    bool Locked;
    Thread_Control_Block* Owner;
    
    priority_queue<Thread_Control_Block*, vector<Thread_Control_Block*>, Compare_Threads> Wait_List;
    
    void GetNext()
    {
        
        // cout<<"   GetNext"<<endl;
        if(Wait_List.empty())
        {
            Locked = false;
            Owner = NULL;
        }
        
        else
        {
            
            Owner == NULL;
            
            do{
                
                if(Wait_List.empty())
                {
                    Owner = NULL;
                    break;
                }
                
                Owner = Wait_List.top();
                Wait_List.pop();
            }while(Owner->State == VM_THREAD_STATE_DEAD);
            
            if(Owner == NULL)
            {
                Locked = false;
                Owner = NULL;
                return;
            }
            
            Owner->Mutexes.push_back(ID);
            
            for(int i = 0; i<Owner->Mutex_Wait_List.size(); i++)
            {
                if(Owner->Mutex_Wait_List[i] == ID)
                {
                    if(Owner->Tick_Time == VM_TIMEOUT_INFINITE)
                    {
                        
                        // cout<<"     Mutex pushing ready list: "<<Owner->Thread_ID<<endl;
                        Owner->Tick_Time = 0;
                        
                        Owner->State = VM_THREAD_STATE_READY;
                        // cout<<"       INSERTION: "<<Owner->Thread_ID<<endl;
                        Ready_List.push(Owner);
                        Owner->Mutex_Wait_List.erase(Owner->Mutex_Wait_List.begin() + i);
                    }
                    else
                    {
                        if(ThreadExists(Owner->Thread_ID, Waiting_Threads) && Owner->State != VM_THREAD_STATE_DEAD)
                        {
                            int index = FindIndex(Owner->Thread_ID, Waiting_Threads);
                            
                            
                            // cout<<"     Mutex pushing ready list: "<<Owner->Thread_ID<<endl;
                            Owner->Tick_Time = 0;
                            
                            Owner->State = VM_THREAD_STATE_READY;
                            
                            // cout<<"       INSERTION: "<<Owner->Thread_ID<<endl;
                            Ready_List.push(Owner);
                            
                            Waiting_Threads.erase(Waiting_Threads.begin()+index);
                            SortWaitingThreads();
                            
                            Owner->Mutex_Wait_List.erase(Owner->Mutex_Wait_List.begin() + i);
                            
                        }
                    }
                }
            }
            
        }
        
    }
    
    bool Lock(Thread_Control_Block* Thread)
    {
        
        // cout<<"   Lock"<<endl;
        if(Locked)
            return false;
        
        Owner = Thread;
        Thread->Mutexes.push_back(ID);
        Locked = true;
        
        return true;
    }
    
    bool Unlock()
    {
        // cout<<"   Unlock"<<endl;
        if(!Locked)
            return false;
        
        Thread_Control_Block* Prev_Owner = Owner;
        
        
        if(Prev_Owner != NULL)
        {
            int i;
            for(i = 0; i<Prev_Owner->Mutexes.size(); i++)
            {
                if(Prev_Owner->Mutexes[i] == ID)
                    break;
            }
            
            Prev_Owner->Mutexes.erase(Prev_Owner->Mutexes.begin() + i);
        }
        
        GetNext();
        
        return true;
    }
    
    
};

vector<Mutex*> All_Mutexes;

int Mutex_Number = 0;

Mutex* GetMutex(TVMMutexID ID)
{
    
    // cout<<"   GetMutex"<<endl;
    for(int i = 0; i<All_Mutexes.size(); i++)
    {
        if(All_Mutexes[i]->ID == ID)
            return All_Mutexes[i];
    }
    
    return NULL;
}

TVMStatus VMMutexCreate(TVMMutexIDRef mutexref)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   Mutex Create"<<endl;
    
    if(mutexref == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    Mutex* mutex = new Mutex;
    
    mutex->ID = ++Mutex_Number;
    mutex->Locked = false;
    mutex->Owner = NULL;
    
    All_Mutexes.push_back(mutex);
    
    *mutexref = mutex->ID;
    
    MachineResumeSignals(&Machine_Signal_State);
    
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMMutexDelete(TVMMutexID mutex)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout<<"   Mutex Delete"<<endl;
    
    for(int i = 0; i<All_Mutexes.size(); i++)
    {
        if(All_Mutexes[i]->ID == mutex)
        {
            if(All_Mutexes[i]->Owner != NULL)
            {
                MachineResumeSignals(&Machine_Signal_State);
                return VM_STATUS_ERROR_INVALID_STATE;
            }
            
            else
            {
                
                All_Mutexes.erase(All_Mutexes.begin() + i);
                
                MachineResumeSignals(&Machine_Signal_State);
                return VM_STATUS_SUCCESS;
            }
        }
        
    }
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_ERROR_INVALID_ID;
    
    
}

TVMStatus VMMutexQuery(TVMMutexID mutex, TVMThreadIDRef ownerref)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout<<"   Mutex Query"<<endl;
    
    Mutex* Target = GetMutex(mutex);
    
    if(Target == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    
    if(ownerref == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    
    if(Target->Locked == false)
    {
        *ownerref = VM_THREAD_ID_INVALID;
    }
    else
        *ownerref = Target->Owner->Thread_ID;
    
    
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout<<"   Mutex Aqcuire"<<endl;
    
    Mutex* Target = GetMutex(mutex);
    
    if(Target == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    
    if(Target->Owner == Running_Thread)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
    }
    
    if(timeout == VM_TIMEOUT_IMMEDIATE)
    {
        if(Target->Locked)
        {
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_FAILURE;
        }
        
        else
        {
            Target->Lock(Running_Thread);
            
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_SUCCESS;
            
        }
    }
    if(timeout != VM_TIMEOUT_INFINITE)
    {
        if(Target->Locked)
        {
            Target->Wait_List.push(Running_Thread);
            Running_Thread->State = VM_THREAD_STATE_WAITING;
            Running_Thread->Tick_Time = timeout;
            Running_Thread->Mutex_Wait_List.push_back(Target->ID);
            Waiting_Threads.push_back(Running_Thread);
            SortWaitingThreads();
            
            Change_Thread();
            
            if(Target->Locked && Target->Owner->Thread_ID != Running_Thread->Thread_ID)
            {
                MachineResumeSignals(&Machine_Signal_State);
                return VM_STATUS_FAILURE;
            }
            
            bool Lock_Success = Target->Lock(Running_Thread);
            
            if(Lock_Success)
            {
                MachineResumeSignals(&Machine_Signal_State);
                return VM_STATUS_SUCCESS;
            }
            else
            {
                MachineResumeSignals(&Machine_Signal_State);
                return VM_STATUS_FAILURE;
            }
            
        }
        else
        {
            Target->Lock(Running_Thread);
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_SUCCESS;
        }
    }
    else //VM Timeout Infinite
    {
        if(!Target->Locked)
        {
            Target->Lock(Running_Thread);
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_SUCCESS;
        }
        
        Target->Wait_List.push(Running_Thread);
        Running_Thread->State = VM_THREAD_STATE_WAITING;
        Running_Thread->Tick_Time = VM_TIMEOUT_INFINITE;
        Running_Thread->Mutex_Wait_List.push_back(Target->ID);
        
        Change_Thread();
        
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_SUCCESS;
    }
    
    
}

TVMStatus VMMutexRelease(TVMMutexID mutex)
{
    
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    
    // cout<<"   Mutex Releas"<<endl;
    
    Mutex* Target = GetMutex(mutex);
    
    if(Target == NULL)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    
    if(Target->Owner != Running_Thread)
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_STATE;
    }
    
    Target->Unlock();
    
    
    Running_Thread->State = VM_THREAD_STATE_READY;
    
    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
    Ready_List.push(Running_Thread);
    
    Change_Thread();
    
    MachineResumeSignals(&Machine_Signal_State);
    
    return VM_STATUS_SUCCESS;
    
}

TVMStatus VMThreadTerminate(TVMThreadID thread)
{
    
    TMachineSignalState Machine_Signal_State;
    
    MachineSuspendSignals(&Machine_Signal_State);
    // cout<<"   TERMINATING: "<<thread<<endl;
    
    if(!ThreadExists(thread, All_Threads))
    {
        MachineResumeSignals(&Machine_Signal_State);
        return VM_STATUS_ERROR_INVALID_ID;
    }
    else
    {
        if(ThreadExists(thread, Dead_Threads))
        {
            MachineResumeSignals(&Machine_Signal_State);
            return VM_STATUS_ERROR_INVALID_STATE;
        }
        else
        {
            int Index = FindIndex(thread, All_Threads);
            if(All_Threads[Index]->State != VM_THREAD_STATE_RUNNING)
            {
                
                TVMThreadState Previous_State = All_Threads[Index]->State;
                
                
                All_Threads[Index]->State = VM_THREAD_STATE_DEAD;
                Dead_Threads.push_back(All_Threads[Index]);
                
                if(Previous_State == VM_THREAD_STATE_WAITING)
                {
                    
                    int Waiting_Index = FindIndex(thread, Waiting_Threads);
                    Waiting_Threads.erase(Waiting_Threads.begin() + Waiting_Index);
                    
                    SortWaitingThreads();
                }
                
                for(int i = 0; i<All_Threads[Index]->Mutexes.size(); i++)
                {
                    
                    Mutex* Target = GetMutex(All_Threads[Index]->Mutexes[i]);
                    
                    Target->GetNext();
                }
                
                All_Threads[Index]->Mutexes.clear();
                
                if(!Ready_List.empty() && (Ready_List.top()->Priority >= Running_Thread->Priority))
                {
                    Running_Thread->State = VM_THREAD_STATE_READY;
                    
                    // cout<<"       INSERTION: "<<Running_Thread->Thread_ID<<endl;
                    Ready_List.push(Running_Thread);
                    
                    // cout<<"   Changing from: "<<All_Threads[Index]->Thread_ID<<endl;
                    Change_Thread();
                }
            }
            else
            {
                Running_Thread->State = VM_THREAD_STATE_DEAD;
                Dead_Threads.push_back(Running_Thread);
                
                for(int i = 0; i<Running_Thread->Mutexes.size(); i++)
                {
                    Mutex* Target = GetMutex(Running_Thread->Mutexes[i]);
                    Target->GetNext();
                    
                }
                
                Running_Thread->Mutexes.clear();
                
                Change_Thread();
                
                
            }
        }
    }
    MachineResumeSignals(&Machine_Signal_State);
    return VM_STATUS_SUCCESS;
}
