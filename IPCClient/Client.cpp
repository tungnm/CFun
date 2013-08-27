#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#pragma comment(lib, "user32.lib")

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
    Person* aDudePtr;

    //retrieve the event object, and see if need to wait for the server.
    //the name of this event object should match the server's
    HANDLE ghWriteEvent = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,             // initial state is nonsignaled
        TEXT("Global\\WriteEvent")  // object name
        ); 

    DWORD dwWaitResult;

    printf("\nClient is waiting for server inut");

    dwWaitResult = WaitForSingleObject( 
        ghWriteEvent, // event handle
        INFINITE);    // indefinite wait
    int i = 0;
    switch (dwWaitResult) 
    {
        // Event object was signaled
    case WAIT_OBJECT_0: 
        //
        // TODO: Read from the shared buffer
        //
        printf("\n Client signaled, I will make the dude older");
        hMapFile = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            szName);               // name of mapping object
        //get the mapping object
        if (hMapFile == NULL)
        {
            _tprintf(TEXT("Could not open file mapping object (%d).\n"),
                GetLastError());
            return 1;
        }
        //copy the message from the buffer
        aDudePtr = (Person*) MapViewOfFile(hMapFile, // handle to map object
            FILE_MAP_ALL_ACCESS,  // read/write permission
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
        printf("\n the original age is: %d", aDudePtr->getAge());
        //do some magic processing with this object:
        aDudePtr->setAge(70);
         printf("\n ...after processing, the age is: %d", aDudePtr->getAge());
        break; 

        // An error occurred
    default: 
        printf("Wait error (%d)\n", GetLastError()); 
        return 0; 
    }


    UnmapViewOfFile(aDudePtr);

    CloseHandle(hMapFile);

    return 0;
}
