#include "stdafx.h"

#define EXIT_ON_ERROR(_hres, _string)  \
              if (FAILED(_hres)) \
                { _tprintf(_T(_string)); goto Exit; }

#define SAFE_RELEASE(_punk)  \
              if ((_punk) != NULL)  \
                { (_punk)->Release(); (_punk) = NULL; }

#define DEBUG_LEVEL 1

#ifndef DEBUG_LEVEL

#define PrintWaveFormat 0
#define PrintBufferFrameCount 0

#else

void PrintWaveFormat(WAVEFORMATEX *pwfx);
void PrintBufferFrameCount(UINT32 bufferFrameCount);
void PrintProgress(UINT32 numFrames, DWORD flags);

#endif

void SetCtrlHandler(bool install);

BOOLEAN quit = false;

float absFloat(float f)
{
    return (f > 0) ? f : f*(-1.0f);
}


//note about size in byte(unsigned char) of audio frame/ packet...
//read: https://msdn.microsoft.com/en-us/library/windows/desktop/dd370858(v=vs.85).aspx
/*
The length of a data packet is expressed as the number of audio frames in the packet. 
The size of an audio frame is specified by the nBlockAlign member of the WAVEFORMATEX
(or WAVEFORMATEXTENSIBLE) structure that the client obtains by calling the 
IAudioClient::GetMixFormat method. The size in bytes of an audio frame equals 
the number of channels in the stream multiplied by the sample size per channel. 
For example, the frame size is four bytes for a stereo (2-channel) stream with 
16-bit samples. A packet always contains an integral number of audio frames.

also read: https://msdn.microsoft.com/en-us/library/windows/desktop/dd390970(v=vs.85).aspx

*/
bool convertFloat32ToInt16(BYTE * pData, int totalBytes)
{

    float sum = 0;
    //the total number of bytes of the 32-bit-float audio buffer( This is
    //Windows standard)
    int floatSize = sizeof(float);

    byte* current = pData;
   
    //convert 32 bit float to 16 bit short
    for (int i=0; i < totalBytes / floatSize ; i++)
    {
        float x = (*(float*)(current));
        sum += absFloat(x);
        x*= 32767; //2^15 - 1
       
        current+=floatSize;
    }
    //detect silent buffer
    //there is also an event_based mode for loopback capture but quick research
    //suggests that it is buggy, need more investigation.
    if(absFloat(sum) < 0.1f)
        return false;
    else
        return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: AudioCapture.exe [mic|loopback]");
        return -1;
    }

    //this flag passed from the command line is for switching between capturing 
    //from the real microphone or with loopback capture on a speaker.
    bool isMicrophoneCapture;

    if (strcmp(argv[1], "mic") == 0)
    {
        isMicrophoneCapture = true;
        printf("Use microphone capture \n");
    }
    else if (strcmp(argv[1], "loopback") == 0)
    {
        isMicrophoneCapture = false;
        printf("Use loopback capture \n");
    }
    
    UINT32 bufferFrameCount = 0;
    DWORD flags = 0;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    HRESULT hr = E_FAIL;
    UINT32 numFramesAvailable = 0;
    UINT32 packetLength = 0;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    BYTE *pData = NULL;
    //IMM device interface represent and hardware endpoint audio device.
    IMMDevice *pDevice = NULL;
    IMMDeviceEnumerator *pEnumerator = NULL;
    WAVEFORMATEX *pwfx = NULL;
    int retvalue = -1;

    SetCtrlHandler(true);

    hr = CoInitialize(NULL);
    EXIT_ON_ERROR(hr, "CoInitialize failed\n");
    
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    EXIT_ON_ERROR(hr, "CoCreateInstance failed\n");
    if (isMicrophoneCapture)
    {
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    }
    else
    {
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    }
    
    EXIT_ON_ERROR(hr, "Failed to get default endpoint\n");

    //initailize a capture stream in loopback mode on the rendering endpoint device
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr, "Failed to activate audio client\n");

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr, "Failed to get audio format\n");
    PrintWaveFormat(pwfx);

    if (isMicrophoneCapture)
    {
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
    }
    else
    {
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, pwfx, NULL);
    }
    
    EXIT_ON_ERROR(hr, "Failed to initialize audio client\n");

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr, "Failed to get buffer size\n");
    PrintBufferFrameCount(bufferFrameCount);

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr, "Failed to get capture service\n");

    hr = pAudioClient->Start();
    EXIT_ON_ERROR(hr, "Failed to start audio client\n");

    while (quit == false) {
        
        if (isMicrophoneCapture)
        {
            // Sleep for half the buffer duration.
            //Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);
        }


        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr, "Failed to get packet size\n");

        if (packetLength != 0) {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            EXIT_ON_ERROR(hr, "Failed to get buffer\n");
            
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;
            }

            int totalByte = pwfx->nBlockAlign * numFramesAvailable;
            
            //detect if there is no sound playing
            if (!(packetLength != 0 && ((flags & AUDCLNT_BUFFERFLAGS_SILENT) == 0) 
                && convertFloat32ToInt16(pData, totalByte) == true))
            {
              
            }
            else
            {

                PrintProgress(numFramesAvailable, flags);
            }
           
            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr, "Failed to release buffer\n");
        }
    }

    retvalue = 0;

Exit:
    pAudioClient->Stop();
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pCaptureClient);
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pEnumerator);
    CoUninitialize();
    SetCtrlHandler(false);
	return retvalue;
}

BOOL CtrlHandler(DWORD fdwCtrlType) 
{ 
    switch( fdwCtrlType ) 
    { 
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        quit = TRUE;
        _tprintf(_T("Received ctrl event..terminating\n"));
        return TRUE;

    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        quit = TRUE;
        _tprintf(_T("Received ctrl event..terminating\n"));
        return FALSE;
        break;

    default:
        return FALSE;
    }
}

void SetCtrlHandler(bool install)
{
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, install);
}

#ifdef DEBUG_LEVEL
void PrintWaveFormat(WAVEFORMATEX *pwfx)
{
    assert(pwfx != NULL);

    _tprintf(_T("Waveformat for audio endpoint\n"));
    if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
        _tprintf(_T("WAVE_FORMAT_PCM\n"));
    }
    _tprintf(_T("Number of channels: %d\n"), pwfx->nChannels);
    _tprintf(_T("Samples per sec: %d\n"), pwfx->nSamplesPerSec);
    _tprintf(_T("Average bytes per sec: %d\n"), pwfx->nAvgBytesPerSec);
    _tprintf(_T("Block align: %d\n"), pwfx->nBlockAlign);
    _tprintf(_T("Bits per sample: %d\n"), pwfx->wBitsPerSample);
    _tprintf(_T("Size of extra information: %d\n\n"), pwfx->cbSize);
}

void PrintBufferFrameCount(UINT32 bufferFrameCount)
{
    _tprintf(_T("Buffer frame count: %d\n"), bufferFrameCount);
}

void PrintProgress(UINT32 numFrames, DWORD flags)
{
    if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) == 0)
    {
        //_tprintf(_T("+%d "), numFrames);
        _tprintf(_T("+"), numFrames);
       
    }
}
#endif
