#include "defines.h"

#include <windows.h>
#include <xinput.h>

// TODO: Move this in a header file
struct win32_render_buffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
};

global_variable bool globalRunning;


// NOTE: This procedure is not required at all for the scope of this
//project but it's a great example of how to mantain backwards
//compatibility in a smart and clever way and also allow the player to
//play the game without a controller.
//CREDITS:(https://guide.handmadehero.org/code/day006/)

// NOTE: XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE: XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        if(XInputLibrary)
        {
            XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
            XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        }
        else
        {
            XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
            
            XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
            XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");

            if(!XInputLibrary)
            {
                XInputGetState = XInputGetStateStub;
                XInputSetState = XInputSetStateStub;
            }
        }
    }
}

internal void Win32CrateRenderBuffer(win32_render_buffer *buffer, int width, int height)
{
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    int bytesPerPixel = 4;
    buffer->width = width;
    buffer->height = height;
    buffer->pitch = width * bytesPerPixel;

    // NOTE: The negative biHeight field is the clue to Windows to
    // treat this bitmap as top-down.
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    // NOTE: The pages allocated with virtual alloc are arleady
    // cleared to 0, therefore the buffer is cleared to black for free
    int bitmapMemorySize = width * height * bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32PaintWindow(HDC deviceContext, win32_render_buffer *buffer)
{
    // NOTE: StretchDIBits is not the fastest way to show the back
    // buffer in the window, this whould be BitBlt(), but is less
    // "parameters demanding". Since performance is not a real concern
    // atm we will use this for now.
    StretchDIBits(deviceContext,
                  0, 0, buffer->width, buffer->height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void Win32RenderWierdGradient(win32_render_buffer *buffer, int redOffset, int greenOffset)
{
    uint32 *pixel = (uint32 *)buffer->memory;
    for(int y = 0; y < buffer->height; y++)
    {
        for(int x = 0; x < buffer->width; x++)
        {
            *pixel++ = VSB_RGB(redOffset + x, greenOffset + y, 0);
        }
    }
}

internal void Win32PoolControllerState(f32 *red, f32 *green)
{
    // TODO: Add multiple controllers support
    int controllerIndex = 0;
    XINPUT_STATE controllerState;
    if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
    {
        // NOTE: Controller plugged in
        XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

        bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
        bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
        bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
        bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
        bool menu = (pad->wButtons & XINPUT_GAMEPAD_START);
        bool view = (pad->wButtons & XINPUT_GAMEPAD_BACK);
        bool lsButton = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
        bool rsButton = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
        bool lb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
        bool rb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
        bool a = (pad->wButtons & XINPUT_GAMEPAD_A);
        bool b = (pad->wButtons & XINPUT_GAMEPAD_B);
        bool x = (pad->wButtons & XINPUT_GAMEPAD_X);
        bool y = (pad->wButtons & XINPUT_GAMEPAD_Y);

        // TODO: Handle dead-zones and normalize the values
        uint8 lTrigger = pad->bLeftTrigger;
        uint8 rTrigger = pad->bRightTrigger;
        // TODO: Maybe use vectors once we have them?
        int16 leftStickX = pad->sThumbLX;
        int16 leftStickY = pad->sThumbLY;
        int16 rightStickX = pad->sThumbRX;
        int16 rightStickY = pad->sThumbRY;

        if(a) *red += 0.5f;
        if(b) *green += 0.5f;
    }
    else
    {
        // NOTE: Controller not aviable
    }
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
            bool prevStateDown = (lParam & (1 << 30)) != 0;
            bool keyIsDown = (lParam & (1 << 31)) == 0;
            
            // NOTE: This way we process just the transision between the states
            if(prevStateDown != keyIsDown)
            {
                switch(wParam)
                {
                    case 'W':
                    {
                        OutputDebugStringA("W\n");
                    } break;
                    case 'A':
                    {
                        OutputDebugStringA("A\n");
                    } break;
                    case 'S':
                    {
                        OutputDebugStringA("S\n");
                    } break;
                    case 'D':
                    {
                        OutputDebugStringA("D\n");
                    } break;
                    case 'Q':
                    {
                        OutputDebugStringA("Q\n");
                    } break;
                    case 'E':
                    {
                        OutputDebugStringA("E\n");
                    } break;
                }
            }
        } break;

        case WM_CLOSE:
        {
            globalRunning = false;
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
                                            (WS_OVERLAPPEDWINDOW|WS_VISIBLE)^WS_THICKFRAME,
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            VBS_WINDOW_WIDTH, VBS_WINDOW_HEIGHT,
                                            0, 0, instanceHandle, 0);
        if(windowHandle)
        {
            // Game initialization
            globalRunning = true;

            Win32LoadXInput();

            HDC deviceContext = GetDC(windowHandle);
            win32_render_buffer backBuffer;
            Win32CrateRenderBuffer(&backBuffer, VBS_WINDOW_WIDTH, VBS_WINDOW_HEIGHT);

            f32 redOffset = 0;
            f32 greenOffset = 0;
            while(globalRunning)
            {
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if(message.message == WM_QUIT)
                    {
                        globalRunning = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                Win32PoolControllerState(&redOffset, &greenOffset);

                Win32RenderWierdGradient(&backBuffer, (int)redOffset, (int)greenOffset);
                Win32PaintWindow(deviceContext, &backBuffer);
                redOffset += 0.25f;
                greenOffset += 0.25f;
            }
        }
    }

    // NOTE: Windows takes care of destroying the window and cleaning
    // up. No need to waste the user time!!!
    // (https://guide.handmadehero.org/code/day003/#550)
    return 0;
}
