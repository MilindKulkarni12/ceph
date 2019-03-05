#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<iomanip>
#include<sstream>
#include<openssl/sha.h>
#include<stdio.h>
//#include<pthread.h>

#define BUFFER_SIZE 128*1024
#define HASH33_SIZE 9
#define HASH33_START 55
#define BLOCK_SIZE 2147483648
#define OSD_1_2 "192.168.6.14"
#define OSD_3_4 "192.168.6.13"
#define OSD_5_6 "192.168.6.12"
#define OSD_7_8 "192.168.6.10"
#define NUM_THREADS 5

using namespace std;

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}//sha256

int getHexValue(char a)
{//converting hash hex-char form to actual hex form.
    switch(a)
    {
        case '0': return 0x0;
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0x4;
        case '5': return 0x5;
        case '6': return 0x6;
        case '7': return 0x7;
        case '8': return 0x8;
        case '9': return 0x9;
        case 'a': return 0xA;
        case 'b': return 0xB;
        case 'c': return 0xC;
        case 'd': return 0xD;
        case 'e': return 0xE;
        case 'f': return 0xF;
    }//switch
}//getHexValue

long long int convertHash(string hashValue)
{
    string hash33 = hashValue.substr (HASH33_START, HASH33_SIZE);
    char a = hash33[0];
    int x, hex = getHexValue(a); 
    long long int final_binary = hex & 0x1;

    for(int i =1; i < HASH33_SIZE; i++)
    {
        x = getHexValue(hash33[i]);
        final_binary = final_binary << 4;
        final_binary = final_binary | x;        
    }//for
    return final_binary;
}//convertHash

string findDestOSD(long long hashValue)
{//finding OSD no., by 33hash.
    if(hashValue >= 0 and hashValue <(2*BLOCK_SIZE))
        return OSD_1_2;
    if(hashValue >= (2*BLOCK_SIZE) and hashValue <(4*BLOCK_SIZE))
        return OSD_3_4;
    if(hashValue >= (4*BLOCK_SIZE) and hashValue <(6*BLOCK_SIZE))
        return OSD_5_6;
    if(hashValue >= (6*BLOCK_SIZE) and hashValue <=(8*BLOCK_SIZE))
        return OSD_7_8;

    return "";
}//findDestOSD

bool sendToOSD(string hashValue, long long newHashValue)
{//identifying receiving OSD's IP and sending the 
 //256hash named file to that OSD.
    string send_scp = "scp /home/cephuser/user/"+ hashValue + " ";

    if(findDestOSD(newHashValue) == OSD_1_2)//sendToOSD1_2
        send_scp = send_scp + OSD_1_2 + ":/home/cephuser/user";
    else if(findDestOSD(newHashValue) == OSD_3_4)//sendToOSD3_4
        send_scp = send_scp + OSD_3_4 + ":/home/cephuser/user";
    else if(findDestOSD(newHashValue) == OSD_5_6)//sendToOSD5_6
        send_scp = send_scp + OSD_5_6 + ":/home/cephuser/user";
    else if(findDestOSD(newHashValue) == OSD_7_8)//sendToOSD7_8
        send_scp = send_scp + OSD_7_8 + ":/home/cephuser/user";
    else//out of range -prec
        return false;
    //sent successfully
    system(send_scp.c_str());
    return true;
}//sendToOSD

int main()
{
    string fileName;
    fstream fin, fout, fout1, fout2, fout3, fout4;
    char buffer[BUFFER_SIZE];
    
    //block variables
    string hashValue;
    long long newHashValue;

    //sending variables
    int retry_attempt;
    bool sent;
    
    fout4.open(OSD_7_8, ios::out);
    fout3.open(OSD_5_6, ios::out);
    fout2.open(OSD_3_4, ios::out);
    fout1.open(OSD_1_2, ios::out);

    cout<<"\nEnter File Name : ";
    cin>>fileName;
    fin.open(fileName, ios::in | ios::binary);
    while(fin)
    {
        //reading 128K from file in a buffer and sending it to hash
        fin.read(buffer, BUFFER_SIZE);
        string *data = new string(buffer);
            
        //sha256 algorithm
        hashValue = sha256(*data);
        newHashValue = convertHash(hashValue);
        cout<<newHashValue<<"\n";
        
        //creating blocks with 256bit hash values as name
        fout.open(hashValue, ios::out | ios::binary);
        fout.write(buffer, fin.gcount());
        fout.close();

        /*
        //send to desired osd multithread
        retry_attempt = 1;
        sent = false;
        while(!sent)
        {//retrying
            sent = sendToOSD(hashValue, newHashValue);
            if(!sent)
                cout<<"\nSending Error, retrying...."<<retry_attempt++;
        }//while
        if(sent)//file sent to OSD successfully
            cout<<"\n"<<hashValue<<" Sent in attempt number "<<retry_attempt<<" !";
        */

        string s = to_string(newHashValue) + ":" + hashValue;

        if(findDestOSD(newHashValue) == OSD_1_2)//sendToOSD1_2
            fout1<<s<<endl;
        else if(findDestOSD(newHashValue) == OSD_3_4)//sendToOSD3_4
            fout2<<s<<endl;
        else if(findDestOSD(newHashValue) == OSD_5_6)//sendToOSD5_6
            fout3<<s<<endl;
        else if(findDestOSD(newHashValue) == OSD_7_8)//sendToOSD7_8
            fout4<<s<<endl;

    }//while
    fout1.close();
    fout2.close();
    fout3.close();
    fout4.close();

    string send_scp;
    send_scp = "scp /home/cephuser/cephStorage/192.168.6.14 osd1:/home/cephuser/user/";
    system(send_scp.c_str());
    send_scp = "scp /home/cephuser/cephStorage/192.168.6.13 osd2:/home/cephuser/user/";
    system(send_scp.c_str());
    send_scp = "scp /home/cephuser/cephStorage/192.168.6.12 osd3:/home/cephuser/user/";
    system(send_scp.c_str());
    send_scp = "scp /home/cephuser/cephStorage/192.168.6.10 osd4:/home/cephuser/user/";
    system(send_scp.c_str());

    return 0;
}//main