#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <stdio.h>

using namespace std;

void taks1(string mes1, string mes2)
{
	cout<<"task1 is saying:"<<mes1;
	cout<<"\ntask1 continue to say:"<<mes2<<endl;
	int x=0;
	for(int i=0;i<100000;i++)
		x++;
}


void ChangeMe(int* a)
{
	*a+=6;
}

void DontChangeMe(const int* x)
{
	
}
void LCM()
{
//test 
std::map<std::string, int> collectionPeriods;
collectionPeriods["metric1"] = 1;
collectionPeriods["metric2"] = 6;
collectionPeriods["metric3"] = 1;
collectionPeriods["metricC"] = 2;
collectionPeriods["metricA"] = 3;
collectionPeriods["metricA"] = 7;
collectionPeriods["metricA"] = 9;

    
   
    //convert metricName->collectionPeriod map to collectionPeriod->vectorOfmetricName(s)
    std::map<int, vector<string>> tMap;
    typedef std::map<std::string, int>::iterator CPIterator;
    int scheduleEndTime = 1;
    for (CPIterator currentElement = collectionPeriods.begin(); currentElement != collectionPeriods.end(); currentElement++)
    tMap[currentElement->second].push_back(currentElement->first);
    //find the lowest common multiple of all collection periods. This will be the time of the final cycle of a full schedule
    typedef std::map<int, vector<string>>::iterator tMapType;
    for (tMapType currentElement = tMap.begin(); currentElement != tMap.end(); currentElement++)
    {
        bool isNotFactorOfLaterElement = true;
        tMapType folowingElement = currentElement;
        for (++folowingElement; folowingElement != tMap.end(); folowingElement++)
        {
            if (folowingElement->first % currentElement->first == 0)
            {
                isNotFactorOfLaterElement = false;
                break;
            }
        }
        if (isNotFactorOfLaterElement)
        scheduleEndTime *= currentElement->first;
    }
    // end of finding lowest common multiple

}

struct EncodedFrame
{
    unsigned int bufferSize;
    unsigned char * buffer;
};

int main()
{
/*	int a[4];

	int* p1;
	int* p2;

	p1=&a[1];
	p2=&a[3];

	cout<<p1-p2;

	int b=5;
	int* pb=&b;
	DontChangeMe(pb);
	*/
	//LCM();

  /*  map<int , string> map1;
    map1[7] = "metric A";
    map1[17] = "metric B";
    map1[13] = "Metric C";
    map1[4] = "metric D";
    map<int , string>::iterator itType;
    itType = map1.begin();
    map1.erase(4);
     itType = map1.begin();
    map1.erase(itType);*/

    //Test byte ordering , should use the programmer calculator
    int x = 45682;
    char * c = (char *)&x;
    x++;

    //\\//\\//\\//\\//\\/\\//\\binary file write:
  /* EncodedFrame frame1;
   frame1.bufferSize = 15345;
   unsigned char buf1[15];
   for (int i = 0; i < 15; i ++)
   {
       buf1[i] = (i + 1) * 2;
   }

    FILE * ptrFileToWrite;
    ptrFileToWrite = fopen("c:/tmp/binFileTest2.bin", "wb");
    if(!ptrFileToWrite)
    {
        printf("\n file open error");
        return 1;
    }
   frame1.buffer = buf1;

    fwrite(&frame1.bufferSize, sizeof(unsigned int), 1, ptrFileToWrite);
   
    fwrite(frame1.buffer, sizeof(char), 15, ptrFileToWrite);

    frame1.bufferSize = 232456;
     fwrite(&frame1.bufferSize, sizeof(unsigned int), 1, ptrFileToWrite);
    unsigned char buf2[7];
   for (int i = 0; i < 7; i ++)
   {
       buf2[i] = (i + 1) * 2;
   }
   frame1.buffer = buf2;

    fwrite(frame1.buffer, sizeof(char), 7, ptrFileToWrite);

    fclose(ptrFileToWrite);

	cin.get();
*/
    //\\//\\//\\//\\//\//\/\/\/\/ Read binary integer in reversed order:
    FILE *ptr_myfile;
    ptr_myfile=fopen("c:/tmp/encodedBee2Secs720p.bin","rb");
	if (!ptr_myfile)
	{
		printf("Unable to open file!");
	}

  
        //read the size
        uint32_t siz;
         char* bytePointer = (char *)&siz;
        if (0) //this is read in normal order, all 4 bytes at once
        {
            fread(&siz,sizeof(uint32_t),1,ptr_myfile);
        }
        else
        {
            //if on big endian machine, then read the bytes of this int in 
            //reverse order
           
            fread(bytePointer + 3,sizeof(char),1,ptr_myfile);
            fread(bytePointer + 2,sizeof(char),1,ptr_myfile);
            fread(bytePointer + 1,sizeof(char),1,ptr_myfile);
            fread(bytePointer,sizeof(char),1,ptr_myfile);
        }

 
	fclose(ptr_myfile);



	return 0;
}
