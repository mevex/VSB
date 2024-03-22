#include "defines.h"
#include "vsb.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>

#include "win32_vsb.h"

global_variable LARGE_INTEGER globalPerfCountFreq;
global_variable bool globalRunning;

global_variable WINDOWPLACEMENT globalWindowsPosition = {sizeof(globalWindowsPosition)};
global_variable bool toggleFullscreen;
global_variable bool fullscreenWindow;
global_variable i32 displayWidth;
global_variable i32 displayHeight;

#if VSB_DEBUG
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
            Assert(fileSize64.QuadPart <= 0xFFFFFFFF);
            ui32 fileSize = (ui32)fileSize64.QuadPart;
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
    Assert(fileSize <= 0xFFFFFFFF);
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
#endif

internal FILETIME Win32GetLastWriteTime(char *filename)
{
    FILETIME result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
    if(GetFileAttributesEx(filename, GetFileExInfoStandard, &fileAttributes))
    {
        result = fileAttributes.ftLastWriteTime;
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
        result.UpdateAndRender = 0;
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
    game->UpdateAndRender = 0;
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
    // TODO(mevex):  But why don't we have the negative height set?
    win32Buffer->info = {};
    win32Buffer->info.bmiHeader.biSize = sizeof(win32Buffer->info.bmiHeader);
    win32Buffer->info.bmiHeader.biWidth = win32Buffer->width;
    win32Buffer->info.bmiHeader.biHeight = win32Buffer->height;
    win32Buffer->info.bmiHeader.biPlanes = 1;
    win32Buffer->info.bmiHeader.biBitCount = 32;
    win32Buffer->info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE: The pages allocated with virtual alloc are arleady
    // cleared to 0, therefore the buffer is cleared to black for
    // free. The allocation must include also the MEM_RESERVE flag to
    // ensure that the function succeed, otherwise there may occur an
    // unexpected behavior and fail.
    int bitmapMemorySize = width * height * bytesPerPixel;
    win32Buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32PaintWindow(HDC deviceContextHandle, win32_render_buffer *win32Buffer)
{
    // NOTE: StretchDIBits is not the fastest way to show the back
    // buffer in the window, this whould be BitBlit(), but is less
    // "parameters demanding". Since performance is not a real concern
    // atm we will use this for now.
    // Also checkout ChangeDisplaySettings on msdn if we want to set the
    // monitor resolution in a convenient way if, for example, StretchDIBits
    // becomes too slow.
    // https://guide.handmadehero.org/code/day040/#3069
    if(fullscreenWindow)
    {
        StretchDIBits(deviceContextHandle,
                      0, 0, displayWidth, displayHeight,
                      0, 0, win32Buffer->width, win32Buffer->height,
                      win32Buffer->memory, &win32Buffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        StretchDIBits(deviceContextHandle,
                      0, 0, win32Buffer->width, win32Buffer->height,
                      0, 0, win32Buffer->width, win32Buffer->height,
                      win32Buffer->memory, &win32Buffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
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
    // TODO(mevex): Add multiple controllers support, maybe...
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
        
        gamepad->leftStick.x = Clamp(Win32NormalizeStickValue(pad->sThumbLX, VSB_GAMEPAD_THUMB_DEADZONE), -1, 1);
        gamepad->leftStick.y = Clamp(Win32NormalizeStickValue(pad->sThumbLY, VSB_GAMEPAD_THUMB_DEADZONE), -1, 1);
        gamepad->rightStick.x = Clamp(Win32NormalizeStickValue(pad->sThumbRX, VSB_GAMEPAD_THUMB_DEADZONE), -1, 1);
        gamepad->rightStick.y = Clamp(Win32NormalizeStickValue(pad->sThumbRY, VSB_GAMEPAD_THUMB_DEADZONE), -1, 1);
    }
    else
    {
        // NOTE: Controller not aviable
        gamepad->connected = false;
    }
}

internal void Win32ToggleFullscreen(HWND windowHandle)
{
    // NOTE(mevex): This follows Raymond Chen's prescriptions for fullscreen toggling, see:
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(windowHandle, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetWindowPlacement(windowHandle, &globalWindowsPosition) && GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(windowHandle, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(windowHandle, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            displayWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
            displayHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
            fullscreenWindow = true;
        }
    }
    else
    {
        SetWindowLong(windowHandle, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(windowHandle, &globalWindowsPosition);
        SetWindowPos(windowHandle, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        fullscreenWindow = false;
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
            } break;
            
            case 'S':
            {
                keyboard->down = keyIsDown;
            } break;
            
            case 'A':
            {
                keyboard->left = keyIsDown;
            } break;
            
            case 'D':
            {
                keyboard->right = keyIsDown;
            } break;
            
            case VK_UP:
            {
                keyboard->y = keyIsDown;
            } break;
            
            case VK_DOWN:
            {
                keyboard->a = keyIsDown;
            } break;
            
            case VK_LEFT:
            {
                keyboard->x = keyIsDown;
            } break;
            
            case VK_RIGHT:
            {
                keyboard->b = keyIsDown;
            } break;
            
            case 'Q':
            {
                keyboard->leftBumper = keyIsDown;
            } break;
            
            case 'E':
            {
                keyboard->rightBumper = keyIsDown;
            } break;
            
            case 'F':
            {
                if(!prevStateDown)
                {
                    toggleFullscreen = true;
                }
            } break;
        }
    }
}

internal ui64 Win32GetTime()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (ui64)time.QuadPart;
}

internal f32 Win32GetMSDifference(ui64 timeToCompare)
{
    ui64 timeNow = Win32GetTime();
    f32 resultMS = ((f32)(timeNow - timeToCompare) / (f32)globalPerfCountFreq.QuadPart)*1000.0f;
    return resultMS;
}

internal void Win32ProfileCode(ui64 *lastCounter, ui64 *lastCycleCount, f32 frameTimeMS)
{
    ui64 endCycleCount = __rdtsc();
    ui64 cyclesElapsed = endCycleCount - *lastCycleCount;
    
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

LRESULT CALLBACK Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0; // Return 0 if the message was processed
    
    switch(message)
    {
        case WM_DESTROY:
        {
            // TODO: Handle this as an error and recreate the window?
            OutputDebugStringA("WM_DESTROY\n");
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // TODO insert assertion
            Assert(false);
            OutputDebugStringA("WARNING! Keyboard message passed through the callback\n");
        } break;
        
        case WM_CLOSE:
        {
            // TODO: Ask for confirmation
            PostQuitMessage(0);
        } break;
        
        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
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
        // NOTE(mevex): I can XOR with the THICKFRAME flag to remove it, it allows the
        // user to resize the window.
        // NOTE: VISIBLE causes the window to be
        // visible at startup so there is no need to call ShowWindow()
        HWND windowHandle = CreateWindowEx(0,
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
            ui64 lastCounter = Win32GetTime();
            ui64 lastCycleCount = __rdtsc();
            
            // NOTE: Game initialization
            globalRunning = true;
            // TODO: The shipping version does not need hardcoded
            // paths and the temp file because reloading will nto be needed
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
            // TODO: Maybe we want to use MEM_LARGE_PAGES for this allocation?
            gameMemory.memory = VirtualAlloc(0, gameMemory.memorySize,
                                             MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            // NOTE: Fixed framerate initialization
            int monitorRefreshRate = 60;
            int vRefresh = GetDeviceCaps(deviceContext, VREFRESH);
            if(vRefresh > 1)
            {
                monitorRefreshRate = vRefresh;
            }
            f32 targetMSPerFrame = 1000.0f / (f32)monitorRefreshRate;
            UINT desiredSchedulerMS = 1;
            bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
            ui64 beginningFrameTime = Win32GetTime();
            
            // NOTE: Input initialization
            Win32LoadXInput();
            game_input input = {};
            input.keyboard.connected = true;
            input.analog = false;
            input.dTime = 1 / (f32)monitorRefreshRate;
            
            if(gameMemory.memory)
            {
                while(globalRunning)
                {
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
                    
                    if(toggleFullscreen)
                    {
                        Win32ToggleFullscreen(windowHandle);
                        toggleFullscreen = false;
                    }
                    
                    Win32PoolGamepadState(&input.gamepad);
                    if(game.UpdateAndRender)
                    {
                        game.UpdateAndRender(&gameMemory, &input);
                    }
                    Win32PaintWindow(deviceContext, &win32BackBuffer);
                    
                    // NOTE: After we blit, we sleep and spinlock for
                    // the remaining time of the frame. Since we
                    // cannot blit in v-sync we can at least ensure
                    // that we have roughly the same frame rate. Later
                    // gpu support will be added
                    f32 frameTimeMS = Win32GetMSDifference(beginningFrameTime);
                    DWORD sleepMS = (DWORD)(targetMSPerFrame - frameTimeMS);
                    if(frameTimeMS < targetMSPerFrame)
                    {
                        if(sleepIsGranular)
                        {
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
                    
                    // NOTE: beginningFrameTime gets updated here
                    // so we dont miss the profile time
                    beginningFrameTime = Win32GetTime();
#if 1
                    Win32ProfileCode(&lastCounter, &lastCycleCount, frameTimeMS);
#endif
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
