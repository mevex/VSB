#pragma once

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
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE: XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define VSB_GAMEPAD_THUMB_DEADZONE 3000
#define VSB_GAMEPAD_TRIGGER_THRESHOLD 30

struct win32_render_buffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
};

struct win32_game_code
{
    HMODULE gameCodeDll;
    FILETIME lastWriteDll;
    game_update_and_render *UpdateAndRender;
};
