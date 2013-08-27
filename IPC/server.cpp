
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>
#include <stdlib.h>
#define BUF_SIZE 256
TCHAR szName[]=TEXT("Global\\MyFileMappingObject");
class Person
{
public:
    Person()
    {
        mAge = 0;
        mName[0] = 'j';
        mName[1] = 'o';
        mName[2] = 'h';
        mName[3] = 'n';
        mName[4] = '\0';

    }
    void setAge(int x)
    {
        mAge = x;
    }
    char* getName()
    {
        return mName;
    }
    int getAge()
    {
        return mAge;
    }
private:
    int mAge;
    char mName[50];

};

int _tmain()
{
    HANDLE hMapFile;
    //pointer to person object to be created in shared memory
    Person* aDudePtr;

    //create file mapping . not sure this "file" can store multiple object
    //of same/different class or not.
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        BUF_SIZE,                // maximum object size (low-order DWORD)
        szName);                 // name of mapping object

    if (hMapFile == NULL)
    {
        _tprintf(TEXT("Could not create file mapping object (%d).\n"),
            GetLastError());
        return 1;
    }
    //reserve some memory 
    aDudePtr = (Person*) MapViewOfFile(hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        sizeof(Person));

    if (aDudePtr == NULL)
    {
        _tprintf(TEXT("Could not map view of file (%d).\n"),
            GetLastError());

        CloseHandle(hMapFile);

        return 1;
    }
    //placement new: create new object at the starting shared address
    aDudePtr = new(aDudePtr) Person();
    HANDLE ghWriteEvent = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("Global\\WriteEvent")  // object name
        ); 

    if (ghWriteEvent == NULL) 
    { 
        printf("CreateEvent failed (%d)\n", GetLastError());
        return 1;
    }


    //get input from user
//    while(1)
 //   {
        std::cout<< "\n Enter  the age for john Smith: \n";
        int age;
        std::cin>>age;
        aDudePtr->setAge(age);
        //signal the event that input has been written to buffer
        SetEvent(ghWriteEvent);
   // }

   std::cin.ignore();
  Sleep(10000);

     UnmapViewOfFile(aDudePtr);

    CloseHandle(hMapFile);

    return 0;
}
