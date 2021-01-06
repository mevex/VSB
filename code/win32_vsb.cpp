#include "defines.h"
#include "vsb.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>

#include "win32_vsb.h"

global_variable LARGE_INTEGER globalPerfCountFreq;
global_variable bool globalRunning;

DEBUG_FREE_FILE(DebugFreeFile)
{
    if(file->memory)
    {
        VirtualFree(file->memory, 0, MEM_RELEASE);
        file->memory = 0;
        file->size = 0;
    }
}

DEBUG_READ_FILE(DebugReadFile)
{
    debug_file result = {};
        
    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize64;
        if(GetFileSizeEx(fileHandle, &fileSize64))
        {
            uint32 fileSize = (int)fileSize64.QuadPart;
            result.memory = VirtualAlloc(0, fileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(result.memory)
            {
                DWORD bytesRead;
                if(ReadFile(fileHandle, result.memory, fileSize, &bytesRead, 0) &&
                   (fileSize == bytesRead))
                {
                    // NOTE: File read successfully
                    result.size = fileSize;
                }
                else
                {
                    DebugFreeFile(&result);
                    result.memory = 0;
                }
            }
        }
        CloseHandle(fileHandle);
    }

    return result;
}

DEBUG_WRITE_FILE(DebugWriteFile)
{
    bool result = false;

    HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if(WriteFile(fileHandle, memory, fileSize, &bytesWritten, 0) &&
           (fileSize == bytesWritten))
        {
            // NOTE: File written successfully
            result = (bytesWritten == fileSize);
        }
        CloseHandle(fileHandle);
    }

    return result;
}

internal FILETIME Win32GetLastWriteTime(char *filename)
{
    FILETIME result = {};

    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle)
    {
        GetFileTime(fileHandle, 0, 0, &result);
        CloseHandle(fileHandle);
    }

    return result;
}

internal win32_game_code Win32LoadGameCode(char *pathDll, char *pathTempDll)
{
    // TODO: The shipping version of this does not need to copy nor check
    // the last write time
    win32_game_code result = {};

    CopyFile(pathDll, pathTempDll, FALSE);

    result.lastWriteDll = Win32GetLastWriteTime(pathDll);
    result.gameCodeDll = LoadLibraryA(pathTempDll);
    if(result.gameCodeDll)
    {
        result.UpdateAndRender = (game_update_and_render *)GetProcAddress(result.gameCodeDll,
                                                                          "GameUpdateAndRender");
    }
    else
    {
        result.UpdateAndRender = GameUpdateAndRenderStub;
    }

    return result;
}

internal void Win32UnloadGameCode(win32_game_code *game)
{
    if(game->gameCodeDll)
    {
        FreeLibrary(game->gameCodeDll);
        game->gameCodeDll = 0;
    }
    game->UpdateAndRender = GameUpdateAndRenderStub;
}

internal void Win32LoadXInput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        if(!XInputLibrary)
        {
            XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        }
    }

    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {
        XInputGetState = XInputGetStateStub;
        XInputSetState = XInputSetStateStub;
    }
}

internal void Win32CrateRenderBuffer(win32_render_buffer *win32Buffer, int width, int height)
{
    if(win32Buffer->memory)
    {
        VirtualFree(win32Buffer->memory, 0, MEM_RELEASE);
    }

    int bytesPerPixel = 4;
    win32Buffer->width = width;
    win32Buffer->height = height;

    // NOTE: The negative biHeight field is the clue to Windows to
    // treat this bitmap as top-down.
    win32Buffer->info.bmiHeader.biSize = sizeof(win32Buffer->info.bmiHeader);
    win32Buffer->info.bmiHeader.biWidth = width;
    win32Buffer->info.bmiHeader.biHeight = -height;
    win32Buffer->info.bmiHeader.biPlanes = 1;
    win32Buffer->info.bmiHeader.biBitCount = 32;
    win32Buffer->info.bmiHeader.biCompression = BI_RGB;

    // NOTE: The pages allocated with virtual alloc are arleady
    // cleared to 0, therefore the buffer is cleared to black for
    // free. The allocation must include also the MEM_RESERVE flag to
    // ensure that the function succeed, otherwise there may occur an
    // unexpected behavior and fail.
    int bitmapMemorySize = width * height * bytesPerPixel;
    win32Buffer->memory = VirtualAlloc(0, bitmapMemorySize,
                                       MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32PaintWindow(HDC deviceContext, win32_render_buffer *win32Buffer)
{
    // NOTE: StretchDIBits is not the fastest way to show the back
    // buffer in the window, this whould be BitBlt(), but is less
    // "parameters demanding". Since performance is not a real concern
    // atm we will use this for now.
    StretchDIBits(deviceContext,
                  0, 0, win32Buffer->width, win32Buffer->height,
                  0, 0, win32Buffer->width, win32Buffer->height,
                  win32Buffer->memory, &win32Buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal f32 Win32NormalizeStickValue(int value, int deadZone)
{
    f32 result = 0;

    if(value < -deadZone)
    {
        result = (f32)(value + deadZone) / (f32)(32768 - deadZone);
    }
    else if(value > deadZone)
    {
        result = (f32)(value - deadZone) / (f32)(32767 - deadZone);
    }

    return result;
}

internal f32 Win32NormalizeTriggerValue(int value, int deadZone)
{
    f32 result = 0;

    if(value > deadZone)
    {
        result = (f32)(value - deadZone) / (f32)(255 - deadZone);
    }

    return result;
}

internal void Win32PoolGamepadState(controller *gamepad)
{
    // TODO: Add multiple controllers support
    int controllerIndex = 0;
    XINPUT_STATE controllerState;
    if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
    {
        // NOTE: Controller plugged in
        gamepad->connected = true;
        XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

        gamepad->up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
        gamepad->down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
        gamepad->left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
        gamepad->right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

        gamepad->a = (pad->wButtons & XINPUT_GAMEPAD_A);
        gamepad->b = (pad->wButtons & XINPUT_GAMEPAD_B);
        gamepad->x = (pad->wButtons & XINPUT_GAMEPAD_X);
        gamepad->y = (pad->wButtons & XINPUT_GAMEPAD_Y);

        gamepad->leftBumper = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
        gamepad->rightBumper = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

        gamepad->menu = (pad->wButtons & XINPUT_GAMEPAD_START);
        gamepad->view = (pad->wButtons & XINPUT_GAMEPAD_BACK);
        gamepad->leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
        gamepad->rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

        gamepad->leftTriggerValue = Win32NormalizeTriggerValue(pad->bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
        gamepad->leftTriggerValue = Win32NormalizeTriggerValue(pad->bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

        gamepad->leftStickX = Win32NormalizeStickValue(pad->sThumbLX, VSB_GAMEPAD_TRIGGER_THRESHOLD);
        gamepad->leftStickY = Win32NormalizeStickValue(pad->sThumbLY, VSB_GAMEPAD_TRIGGER_THRESHOLD);
        gamepad->rightStickX = Win32NormalizeStickValue(pad->sThumbRX, VSB_GAMEPAD_TRIGGER_THRESHOLD);
        gamepad->rightStickY = Win32NormalizeStickValue(pad->sThumbRY, VSB_GAMEPAD_TRIGGER_THRESHOLD);
    }
    else
    {
        // NOTE: Controller not aviable
        gamepad->connected = false;
    }
}

internal void Win32ProcessKeyboardInput(controller *keyboard, WPARAM VKCode, LPARAM lParam)
{
    bool prevStateDown = (lParam & (1 << 30)) != 0;
    bool keyIsDown = (lParam & (1 << 31)) == 0;
            
    // NOTE: This way we process just the transision between the states
    if(prevStateDown != keyIsDown)
    {
        switch(VKCode)
        {
            case 'W':
            {
                keyboard->up = keyIsDown;
                keyboard->leftStickY = keyIsDown ? 1.0f : 0;
            } break;

            case 'S':
            {
                keyboard->down = keyIsDown;
                keyboard->leftStickY = keyIsDown ? -1.0f : 0;
            } break;

            case 'A':
            {
                keyboard->left = keyIsDown;
                keyboard->leftStickX = keyIsDown ? -1.0f : 0;
            } break;

            case 'D':
            {
                keyboard->right = keyIsDown;
                keyboard->leftStickX = keyIsDown ? 1.0f : 0;
            } break;

            case VK_UP:
            {
                keyboard->y = keyIsDown;
                keyboard->rightStickY = keyIsDown ? 1.0f : 0;
            } break;

            case VK_DOWN:
            {
                keyboard->a = keyIsDown;
                keyboard->rightStickY = keyIsDown ? -1.0f : 0;
            } break;

            case VK_LEFT:
            {
                keyboard->x = keyIsDown;
                keyboard->rightStickX = keyIsDown ? -1.0f : 0;
            } break;

            case VK_RIGHT:
            {
                keyboard->b = keyIsDown;
                keyboard->rightStickX = keyIsDown ? 1.0f : 0;
            } break;

            case 'Q':
            {
                keyboard->leftBumper = keyIsDown;
            } break;

            case 'E':
            {
                keyboard->rightBumper = keyIsDown;
            } break;
        }
    }
}

internal uint64 Win32GetTime()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (uint64)time.QuadPart;
}

internal f32 Win32GetMSDifference(uint64 timeToCompare)
{
    uint64 timeNow = Win32GetTime();
    f32 resultMS = ((f32)(timeNow - timeToCompare) / (f32)globalPerfCountFreq.QuadPart)*1000.0f;
    return resultMS;
}

internal void Win32ProfileCode(uint64 *lastCounter, uint64 *lastCycleCount, f32 frameTimeMS)
{
    uint64 endCycleCount = __rdtsc();
    uint64 cyclesElapsed = endCycleCount - *lastCycleCount;

    f32 MCyclesPerFrame = (f32)cyclesElapsed / (1000.0f * 1000.0f);
    f32 profileTimeMS = Win32GetMSDifference(*lastCounter);
    f32 FPS = 1000.0f / profileTimeMS;
    f32 blitMS = profileTimeMS - frameTimeMS;

    char Buffer[256];
    sprintf_s(Buffer, "Time:%.02fms,  FPS:%.02f,  Mc/f:%.02f, Blit-time:%.02fms\n",
              profileTimeMS, FPS, MCyclesPerFrame, blitMS);
    OutputDebugStringA(Buffer);

    *lastCounter = Win32GetTime();
    *lastCycleCount = endCycleCount;
}

LRESULT CALLBACK Win32WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0; // Return 0 if the message was processed

    switch(message)
    {
        case WM_DESTROY:
        {
            // TODO: Handle this as an error and recreate the window?
            OutputDebugStringA("WM_DESTROY\n");
            result = DefWindowProc(window, message, wParam, lParam);
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // TODO insert assertion
            OutputDebugStringA("WARNING! Keyboard message passed through the callback\n");
        } break;

        case WM_CLOSE:
        {
            // TODO: Ask for confirmation
            PostQuitMessage(0);
        } break;

        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        }
    }

    return result;
}

int WINAPI wWinMain(HINSTANCE instanceHandle,
                    HINSTANCE hPrevInstance,
                    PWSTR comandLineArguments,
                    int showStyle)
{
    WNDCLASSEXA windowClass = {};

    // NOTE: HREDRAW and VREDRAW ensure that the entire window will be
    // repainted every time, not only the update region. OWNDC allow
    // us to get the DC at the beginning of the program and use forever
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    windowClass.lpfnWndProc = Win32WindowCallback;
    windowClass.hInstance = instanceHandle;
    windowClass.lpszClassName = "VBSClass";
    // windowClass.hIcon; Maybe set this in the future

    if(RegisterClassEx(&windowClass))
    {
        // NOTE: The XOR removes the THICKFRAME flag that allow the
        // user to resize the window. VISIBLE causes the window to be
        // visible at startup so there is no need to call ShowWindow()
        HWND windowHandle = CreateWindowExA(0,
                                            windowClass.lpszClassName,
                                            "Valentino's SandBox",
                                            (WS_OVERLAPPEDWINDOW|WS_VISIBLE),
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            1296, 759,
                                            0, 0, instanceHandle, 0);
        if(windowHandle)
        {
            // NOTE: Profiling initialization
            QueryPerformanceFrequency(&globalPerfCountFreq);
            uint64 lastCounter = Win32GetTime();
            uint64 lastCycleCount = __rdtsc();

            // TODO: Maybe this can be queried
            int monitorRefreshRate = 60;
            f32 targetMSPerFrame = 1000.0f / (f32)monitorRefreshRate;
            UINT desiredSchedulerMS = 1;
            bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
            uint64 beginningFrameTime = Win32GetTime();

            // NOTE: Input initialization
            Win32LoadXInput();
            game_input input = {};
            input.keyboard.connected = true;
            input.analog = false;

            // NOTE: Game initialization
            globalRunning = true;
            // TODO: The shipping version does not need hardcoded
            // paths and the temp file because will not be any reloading
            char *pathDll = "../build/vsb.dll";
            char *pathTempDll = "../build/temp_vsb.dll";
            win32_game_code game = Win32LoadGameCode(pathDll, pathTempDll);

            HDC deviceContext = GetDC(windowHandle);
            win32_render_buffer win32BackBuffer;
            Win32CrateRenderBuffer(&win32BackBuffer, 1280, 720);
            game_memory gameMemory = {};
            gameMemory.DebugFreeFile = DebugFreeFile;
            gameMemory.DebugReadFile = DebugReadFile;
            gameMemory.DebugWriteFile = DebugWriteFile;
            gameMemory.backBuffer.memory = win32BackBuffer.memory;
            gameMemory.backBuffer.width = win32BackBuffer.width;
            gameMemory.backBuffer.height = win32BackBuffer.height;
            gameMemory.memorySize = MB(64);
            gameMemory.memory = VirtualAlloc(0, gameMemory.memorySize,
                                             MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            if(gameMemory.memory)
            {
                while(globalRunning)
                {
                    beginningFrameTime = Win32GetTime();

                    // NOTE: This checks if the dll is rebuild and
                    // reloads the new code allowing to change mostly
                    // all of the game code and istantly see the
                    // changes without the need to close and reopen
                    // the application
                    FILETIME newWriteDll = Win32GetLastWriteTime(pathDll);
                    if(CompareFileTime(&newWriteDll, &game.lastWriteDll) != 0)
                    {
                        Win32UnloadGameCode(&game);
                        game = Win32LoadGameCode(pathDll, pathTempDll);
                    }

                    MSG message;
                    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                    {
                        if(message.message == WM_QUIT)
                        {
                            OutputDebugStringA("Quit message\n");
                            globalRunning = false;
                        }
                        else if(message.message == WM_KEYDOWN ||
                                message.message == WM_KEYUP)
                        {
                            Win32ProcessKeyboardInput(&input.keyboard, message.wParam, message.lParam);
                        }
                        else
                        {
                            TranslateMessage(&message);
                            DispatchMessage(&message);
                        }
                    }

                    Win32PoolGamepadState(&input.gamepad);
                    game.UpdateAndRender(&gameMemory, &input);
                    Win32PaintWindow(deviceContext, &win32BackBuffer);

                    // NOTE: After we blit, we sleep and spinlock for
                    // the remaining time of the frame. Since we
                    // cannot blit in v-sync we can at least ensure
                    // that we have roughly the same frame rate so
                    // that we can tune the game. Later gpu support
                    // will be added
                    f32 frameTimeMS = Win32GetMSDifference(beginningFrameTime);
                    if(frameTimeMS < targetMSPerFrame)
                    {

                        if(sleepIsGranular)
                        {
                            DWORD sleepMS = (DWORD)(targetMSPerFrame - frameTimeMS);
                            if(sleepMS > 0)
                            {
                                Sleep(sleepMS);
                            }
                        }

                        frameTimeMS = Win32GetMSDifference(beginningFrameTime);

                        while(frameTimeMS < targetMSPerFrame)
                        {
                            // NOTE: We spinlock the remaining time
                            frameTimeMS = Win32GetMSDifference(beginningFrameTime);
                        }
                    }

//                    Win32ProfileCode(&lastCounter, &lastCycleCount, frameTimeMS);
                }
            }
            else
            {
                OutputDebugStringA("Memory allocation failed");
            }
        }
    }

    // NOTE: Windows takes care of destroying the window and cleaning
    // up. No need to waste the user time!!!
    // (https://guide.handmadehero.org/code/day003/#550)
    return 0;
}
