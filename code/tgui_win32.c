#include "tgui.h"
#include "tgui.c"

#include <Windows.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
static HDC global_device_context = 0;
static b32 global_running = false;

// NOTE: backbuffer variables
static BITMAPINFO global_bitmap_info;
static HBITMAP global_bitmap;
static HDC global_backbuffer_dc;
static void *global_backbuffer_data;

static void win32_create_backbuffer(HDC device)
{
    global_bitmap_info.bmiHeader.biSize = sizeof(global_bitmap_info.bmiHeader);
    global_bitmap_info.bmiHeader.biWidth = WINDOW_WIDTH;
    global_bitmap_info.bmiHeader.biHeight = -WINDOW_HEIGHT;
    global_bitmap_info.bmiHeader.biPlanes = 1;
    global_bitmap_info.bmiHeader.biBitCount = 32;
    global_bitmap_info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE: Alloc memory for the backbuffer 
    size_t backbuffer_size = (WINDOW_WIDTH*WINDOW_HEIGHT*sizeof(u32));
    global_backbuffer_data = (u32 *)VirtualAlloc(0, backbuffer_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    global_bitmap = CreateDIBSection(device, &global_bitmap_info, DIB_RGB_COLORS, &global_backbuffer_data, 0, 0);
    
    // NOTE: Create a compatible device context to be able to blit it on window
    global_backbuffer_dc = CreateCompatibleDC(device);
    SelectObject(global_backbuffer_dc, global_bitmap);
}

static LRESULT win32_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    switch(message)
    {
        case WM_CREATE:
        {
        }break;
        case WM_CLOSE:
        {
            global_running = false;
        }break;
        case WM_KEYDOWN:
        {
        }break;
        case WM_KEYUP:
        {
        }
        default:
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        }break;
    }
    return result;
}

int main(int argc, char** argv)
{
    UNUSED_VAR(argc);
    UNUSED_VAR(argv);

    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = win32_window_proc;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = "tgui_window_class";

    if(!RegisterClassA(&window_class))
    {
        printf("[ERROR]: cannot register win32 class\n");
        return -1;
    }

    RECT window_rect = {0};
    window_rect.bottom = WINDOW_HEIGHT;
    window_rect.right = WINDOW_WIDTH;
    AdjustWindowRect(&window_rect, WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0);

    u32 window_width = window_rect.right - window_rect.left;
    u32 window_height = window_rect.bottom - window_rect.top;
    HWND window = CreateWindowExA(0, window_class.lpszClassName, "tgui",
                                  WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
                                  100, 100, window_width, window_height,
                                  0, 0, window_class.hInstance, 0);
    if(!window)
    {
        printf("[ERROR]: cannot create win32 window\n");
        return -1;
    }
    
    // TODO: Call this functions on WM_CREATE
    global_device_context = GetDC(window);
    global_running = true;
    win32_create_backbuffer(global_device_context);
   
    u32 frames_per_second = 30;
    u32 ms_per_frame = 1000 / frames_per_second;
    f32 delta_time = 1.0f / (f32)frames_per_second;
    LARGE_INTEGER large_frequency;
    LARGE_INTEGER large_last_time;
    QueryPerformanceFrequency(&large_frequency);
    QueryPerformanceCounter(&large_last_time);
    u64 last_time = large_last_time.QuadPart;
    
    UNUSED_VAR(delta_time);

    // NOTE: backbuffer for tgui to draw all the elements
    TGuiBackbuffer tgui_backbuffer = {0};
    tgui_backbuffer.width = WINDOW_WIDTH;
    tgui_backbuffer.height = WINDOW_HEIGHT;
    tgui_backbuffer.pitch = WINDOW_WIDTH * sizeof(u32);
    tgui_backbuffer.data = global_backbuffer_data;
    
    // NOTE: load bitmap test
    TGuiBitmap test_bitmap = tgui_debug_load_bmp("data/font.bmp");
    // TODO: easier way to create a font (create a funtion)
    // NOTE: create font test
    TGuiFont test_font = {0};
    test_font.src_rect.x = 0;
    test_font.src_rect.y = 0;
    test_font.src_rect.width = 7;
    test_font.src_rect.height = 9;
    test_font.num_rows = 18;
    test_font.num_cols = 6;
    test_font.bitmap = &test_bitmap;
    
    while(global_running)
    {
        LARGE_INTEGER large_current_time;
        QueryPerformanceCounter(&large_current_time);
        u64 current_time = large_current_time.QuadPart;
        float curret_sec_per_frame = (f32)(current_time - last_time) / (f32)large_frequency.QuadPart;
        u64 current_ms_per_frame = (u64)(curret_sec_per_frame * 1000.0f); 
        if(current_ms_per_frame < ms_per_frame)
        {
            i32 ms_to_sleep = ms_per_frame - current_ms_per_frame;
            Sleep(ms_to_sleep);
        }
        QueryPerformanceCounter(&large_last_time);
        last_time = large_last_time.QuadPart;

        MSG message;
        while(PeekMessageA(&message, window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // NOTE: clear the screen
        tgui_clear_backbuffer(&tgui_backbuffer);
        
        TGuiRect src_rect = {0, 0, test_bitmap.width, test_bitmap.height};
        TGuiRect dest_rect = {0, 0, 128, 64};
        tgui_draw_src_dest_bitmap(&tgui_backbuffer, &test_bitmap, src_rect, dest_rect);

        tgui_draw_text(&tgui_backbuffer, &test_font, 27, 40, 300, "@tomascabrerizo");


        // NOTE: Blt the backbuffer on to the destination window
        BitBlt(global_device_context, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, global_backbuffer_dc, 0, 0, SRCCOPY);
    }

    return 0;
}
