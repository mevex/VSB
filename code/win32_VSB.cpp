#include "defines.h"

#include <windows.h>

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

internal void Win32PaintWindow(HDC deviceContext, win32_render_buffer buffer)
{
    // NOTE: StretchDIBits is not the fastest way to show the back
    // buffer in the window, this whould be BitBlt(), but is the less
    // "parameters demanding" one. Since performance is not a real
    // concern atm we will use this for now.
    StretchDIBits(deviceContext,
                  0, 0, buffer.width, buffer.height,
                  0, 0, buffer.width, buffer.height,
                  buffer.memory, &buffer.info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void Win32RenderWierdGradient(win32_render_buffer buffer, int redOffset, int greenOffset)
{
    for(int y = 0; y < buffer.height; y++)
    {
        uint32 *pixel = ((uint32 *)buffer.memory) + (y*buffer.width);
        for(int x = 0; x < buffer.width; x++)
        {
            *pixel++ = VSB_RGB(redOffset + x, greenOffset + y, 0);
        }
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
            globalRunning = true;
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

                Win32RenderWierdGradient(backBuffer, (int)redOffset, (int)greenOffset);
                Win32PaintWindow(deviceContext, backBuffer);
                redOffset += 0.5;
                greenOffset += 0.5;
            }
        }
    }

    // NOTE: Windows takes care of destroying the window and cleaning
    // up. No need to waste the user time!!!
    // (https://guide.handmadehero.org/code/day003/#550)
    return 0;
}
