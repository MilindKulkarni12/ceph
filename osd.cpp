#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<openssl/sha.h>
#include<experimental/filesystem>
#include<thread>
#include<regex>

#define BLOCK_SIZE 2147483648
#define NO_REF_COUNT -1
#define OSD_NUM 1

#define OSD_1_2 "192.168.6.14"
#define OSD_3_4 "192.168.6.13"
#define OSD_5_6 "192.168.6.12"
#define OSD_7_8 "192.168.6.10"

using namespace std;
namespace fs = std::experimental::filesystem;

class FileBlockTable;
class Node
{
    string fName; //256 bit name of the file blocks
    long ref_counter;//counter for duplicate file blocks
    Node *nextFileBlock;

public:
    Node();
    Node(string fName, long ref_counter);

    friend class FileBlockTable;
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
    void updateHashTable(FileBlockTable **fbt, string block_Name);

    friend void initializeFileBlockTable(FileBlockTable **fbt);
    //friend class Node;
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
        i =0;
        for(Node *block = fbt[block_Name %(OSD_NUM*BLOCK_SIZE)]->block_Head; block->nextFileBlock != NULL;block = block->nextFileBlock);
            i++;
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
        Node *block;
        for(block = fbt[block_Name%(OSD_NUM*BLOCK_SIZE)]->block_Head; block->nextFileBlock != NULL; block = block->nextFileBlock);
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

string getFileModificationTime()
{
    auto ftime = fs::last_write_time("/home/cephuser/cephStorage/");
    time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
    
    tm *tm = localtime(&cftime);
    string syear,smonth,sday,stimeh,sth,stm,sts; 
    syear = to_string(tm->tm_year +1900);
    smonth = to_string(tm->tm_mon+1);
    sday = to_string(tm->tm_mday);  
    sth = to_string(tm->tm_hour);
    stm = to_string(tm->tm_min);
    sts = to_string(tm->tm_sec);
    string s = syear + "." + smonth + "." + sday + "-" + sth + "." + stm + "." + sts; 
    
    return s;
}//getFileModificationTime

void FileBlockTable :: updateHashTable(FileBlockTable **fbt, string block_Name)
{
    fstream block, nonDup;
    block.open(block_Name, ios::in);
    nonDup.open("osd1", ios::out);

    string block_Values;

    while(getline(block, block_Values))
    {
        regex regex("\\:");
        vector<string> hashes(
            sregex_token_iterator(block_Values.begin(), block_Values.end(), regex, -1),
            sregex_token_iterator()
        );
        
        for (auto &block_Values: hashes)
            cout << block_Values << '\n';

        newFileBlock(fbt, stoi(hashes[0]), hashes[1]);//2^33 bit dec hash
        if(getRefCounter(fbt, stoi(hashes[0]), hashes[1]) == 0)
            nonDup<<hashes[1]<<endl;
    }//while reading ip-file fininshed.

    nonDup.close();

    system("scp osd1 mon:/home/cephuser/user");
}//updateHashTable

void initializeFileBlockTable(FileBlockTable **fbt)
{
    for(long i =0; i < BLOCK_SIZE; i++)
        fbt[i] = new FileBlockTable(OSD_NUM*BLOCK_SIZE + i);
}//initializeFileBlockTable

int main()
{
    FileBlockTable **fbt = new FileBlockTable*[BLOCK_SIZE];
    FileBlockTable f;
    initializeFileBlockTable(fbt);
    string btime = getFileModificationTime();
        
    while(true)
    {
        string ftime = getFileModificationTime();
        if(btime != ftime)
        {//update hash table
            f.updateHashTable(fbt, OSD_1_2);
        }//if
        this_thread::sleep_for(1s);
    }//while
    return 0;
}//main