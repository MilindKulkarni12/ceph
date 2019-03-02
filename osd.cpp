#include<iostream>
#include<fstream>
#include <cstring>
#include <string>
#include<openssl/sha.h>

#define BLOCK_SIZE 2147483648
#define NO_REF_COUNT -1
#define OSD_NUM 1
using namespace std;

class BlockHashTable;
class Node
{
    string fName; //256 bit name of the file blocks
    long ref_counter;//counter for duplicate file blocks
    Node *nextFileBlock;

public:
    Node();
    Node(string fName, long ref_counter);

    friend class BlockHashTable;
};//Node

Node :: Node()
{
    this->fName = "";
    this->ref_counter = 0;
    this->nextFileBlock = NULL;
}//default constructor


Node :: Node(string fName, long ref_counter =0)
{
    this->fName = fName;
    this->ref_counter = ref_counter;
    this->nextFileBlock = NULL;
}//parameterized constructor

class FileBlockTable
{
    long long block_Name;//33bit decimal version filename
    Node *block_Head;//head pointer of the chain
    long block_length;

public:
    FileBlockTable();
    FileBlockTable(long long block_Name, long block_length);
    bool checkCollision(FileBlockTable **fbt,long long block_Name);
    bool isDuplicate(FileBlockTable **fbt,long long block_Name, string file256);
    void addFileBlock(FileBlockTable **fbt, long long block_Name, string file256);
    void newFileBlock(FileBlockTable **fbt, long long block_Name, string file256);
    long getRefCounter(FileBlockTable **fbt, long long block_Name, string file256);
    long getBlockCount(FileBlockTable **fbt, long long block_Name, string file256);

    friend initializeFileBlockTable(FileBlockTable **fbt);
};//FileBlockTable

FileBlockTable :: FileBlockTable()
{
    this->block_Name = 0;
    this->block_Head = NULL;
    this->block_length = 0;
}//def constr

FileBlockTable :: FileBlockTable(long long block_Name, long block_length =-1)
{
    this->block_Name = block_Name;
    this->block_Head = NULL;
    this->block_length = block_length;
}//par constr

long FileBlockTable :: getRefCounter(FileBlockTable **fbt, long long block_Name, string file256)
{
    for(Node *block = fbt[block_Name%(OSD_NUM*BLOCK_SIZE)]->block_Head; block != NULL; block = block->nextFileBlock)
    {
        if(block->fName == file256)
            return block->ref_counter;
    }//for
    return NO_REF_COUNT;
}//getRefCounter

long FileBlockTable ::  getBlockCount(FileBlockTable **fbt, long long block_Name, string file256)
{
    long i;
    if(block_length == -1)
    {
        for(i =0, Node *block = fbt[block_Name %(OSD_NUM*BLOCK_SIZE)]->block_Head; block->nextFileBlock != NULL; i++, block = block->nextFileBlock);
        block_length = i;
        return block_length;
    }//for
    else
        return block_length;
}//getBlockCount

bool FileBlockTable :: checkCollision(FileBlockTable **fbt,long long block_Name)
{   
    if(fbt[block_Name%(OSD_NUM*BLOCK_SIZE)]->block_Head == NULL)
        return false;
    else
        return true;
}//checkCollision

bool FileBlockTable :: isDuplicate(FileBlockTable **fbt,long long block_Name, string file256)
{
    for(Node *block = fbt[block_Name%(OSD_NUM*BLOCK_SIZE)]->block_Head; block != NULL; block = block->nextFileBlock)
    {
        if(block->fName == file256)
        {
            block->ref_counter++;
            return true;
        }//if duplicate block found

        else//else duplicate block not found
            return false;
    }//for searching for duplicacy
}//isDuplicate

void FileBlockTable :: addFileBlock(FileBlockTable **fbt,long long block_Name, string file256)
{
    //first block for the index(block_Name)
    if(fbt[block_Name]->block_Head == NULL)
    {
        fbt[block_Name]->block_Head = new Node(file256, 0);
        fbt[block_Name]->block_length = -1;
    }//if

    else
    {//blocks already exist, chaining to end
        for(Node *block = fbt[block_Name%(OSD_NUM*BLOCK_SIZE)]->block_Head; block->nextFileBlock != NULL; block = block->nextFileBlock);
        block->nextFileBlock = new Node(file256, 0);
        fbt[block_Name]->block_length = -1;
    }//else
}//addFileBlock

void FileBlockTable :: newFileBlock(FileBlockTable **fbt, long long block_Name, string file256)
{
    if(checkCollision(fbt, block_Name))
    {
        if(!isDuplicate(fbt, block_Name, file256))
            addFileBlock(fbt, block_Name, file256);
            //if unique 256bit hash value
            //else ref_counter value incremented
    }//if 33bit hash value exists

    else//add file block to unused index
        addFileBlock(fbt, block_Name, file256);    
}//newFileBlock

void initializeFileBlockTable(FileBlockTable **fbt)
{
    for(long i =0; i < BLOCK_SIZE; i++)
        *fbt[i] = new FileBlockTable(OSD_NUM*BLOCK_SIZE + i);
}//initializeFileBlockTable

int main()
{
    FileBlockTable **fbt = new FileBlockTable[BLOCK_SIZE];
    initializeFileBlockTable(**fbt);

    return 0;
}//main