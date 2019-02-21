#include<iostream>
#include<fstream>
#include <cstring>
#include <string>
#include<openssl/sha.h>

#define BLOCK_SIZE (long long)8589934592/3

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


Node :: Node(string fName, long ref_counter)
{
    this->fName = fName;
    this->ref_counter = ref_counter;
    this->nextFileBlock = NULL;
}//parameterized constructor

class FileBlockTable
{
    long long block_Name;//33bit decimal version filename
    Node *block_Head;//head pointer of the chain

public:
    FileBlockTable();
    FileBlockTable(long long block_Name, Node* block_Head);
    checkCollision(FileBlockTable *fbt,long long block_Name);
    isDuplicate(FileBlockTable *fbt,long long block_Name, string file256);
    addFileBlock(FileBlockTable *fbt, long long block_Name, string file256);
    newFileBlock(FileBlockTable *fbt, long long block_Name, string file256);
};//FileBlockTable

FileBlockTable :: FileBlockTable()
{
    this->block_Name = 0;
    this->block_Head = NULL;
}//def constr

FileBlockTable :: FileBlockTable(long long block_Name, Node* block_Head)
{
    this->block_Name = block_Name;
    this->block_Head = block_Head;
}//par constr


bool FileBlockTable :: checkCollision(FileBlockTable *fbt,long long block_Name)
{   
    if(fbt[block_Name]->block_Head == NULL)
        return false;
    else
        return true;
}//checkCollision

bool FileBlockTable :: isDuplicate(FileBlockTable *fbt,long long block_Name, string file256)
{
    for(Node *block = fbt[block_Name]->block_Head; block != NULL; block = block->nextFileBlock)
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

void FileBlockTable :: addFileBlock(FileBlockTable *fbt,long long block_Name, string file256)
{
    for(Node *block = fbt[block_Name]->block_Head; block->nextFileBlock != NULL; block = block->nextFileBlock);
    block->nextFileBlock = new Node(file256, 0);
}//addFileBlock

void FileBlockTable :: newFileBlock(FileBlockTable *fbt, long long block_Name, string file256)
{
    if(checkCollision(fbt, block_Name))
    {
        if(!isDuplicate(fbt, block_Name, file256))
            addFileBlock(fbt, block_Name, file256);
            //if unique 256bit hash value
    }//if 33bit hash value exists

    else//add file block to unused index
        addFileBlock(fbt, block_Name, file256);    
}//newFileBlock

int main()
{
    FileBlockTable *fbt = new FileBlockTable[BLOCK_SIZE];

    return 0;
}//main