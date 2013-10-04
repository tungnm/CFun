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

int _tmain(int argc, _TCHAR* argv[])
{
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

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr, "Failed to get default endpoint\n");

    //initailize a capture stream in loopback mode on the rendering endpoint device
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr, "Failed to activate audio client\n");

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr, "Failed to get audio format\n");
    PrintWaveFormat(pwfx);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, pwfx, NULL);
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
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr, "Failed to get packet size\n");

        if (packetLength != 0) {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            EXIT_ON_ERROR(hr, "Failed to get buffer\n");
            
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;
            }

            PrintProgress(numFramesAvailable, flags);

            // Pass the capture data to OPUS encoder.
            // Note that pData is set to NULL for silence.
            //hr = CopyData(pData, numFramesAvailable);
            //EXIT_ON_ERROR(hr);

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
