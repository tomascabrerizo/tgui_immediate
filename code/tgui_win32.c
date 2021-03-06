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
        }break;
        case WM_MOUSEMOVE:
        {
            TGuiEventMouseMove mouse_event = {0};
            mouse_event.type = TGUI_EVENT_MOUSEMOVE;
            mouse_event.pos_x = (i16)LOWORD(l_param);
            mouse_event.pos_y = (i16)HIWORD(l_param);
            tgui_push_event((TGuiEvent)mouse_event);
        }break;
        case WM_LBUTTONDOWN:
        {
            TGuiEvent mouse_event = {0};
            mouse_event.type = TGUI_EVENT_MOUSEDOWN;
            tgui_push_event(mouse_event);
        }break;
        case WM_LBUTTONUP:
        {
            TGuiEvent mouse_event = {0};
            mouse_event.type = TGUI_EVENT_MOUSEUP;
            tgui_push_event(mouse_event);
        }break;
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
    
    // NOTE: load bitmap for testing
    TGuiBitmap test_bitmap = tgui_debug_load_bmp("data/font.bmp");
    TGuiBitmap dog_bitmap = tgui_debug_load_bmp("data/test.bmp");
    UNUSED_VAR(dog_bitmap);
    // NOTE: create a font for testing 
    TGuiFont test_font = tgui_create_font(&test_bitmap, 7, 9, 18, 6);
    
    // NOTE: init TGUI lib
    tgui_init(&tgui_backbuffer, &test_font);
    

    TGuiWindowDescriptor window_descriptor_1 = {0};
    window_descriptor_1.x = 20;
    window_descriptor_1.y = 100;
    window_descriptor_1.margin = 20;
    
    TGuiWindowDescriptor window_descriptor_2 = {0};
    window_descriptor_2.x = 200;
    window_descriptor_2.y = 100;
    window_descriptor_2.margin = 20;

    while(global_running)
    {
        LARGE_INTEGER large_current_time;
        QueryPerformanceCounter(&large_current_time);
        u64 current_time = large_current_time.QuadPart;
        f32 curret_sec_per_frame = (f32)(current_time - last_time) / (f32)large_frequency.QuadPart;
        u64 current_ms_per_frame = (u64)(curret_sec_per_frame * 1000.0f); 
        if(current_ms_per_frame < ms_per_frame)
        {
            i32 ms_to_sleep = ms_per_frame - current_ms_per_frame;
            Sleep(ms_to_sleep);
        }
        QueryPerformanceCounter(&large_current_time);
        current_time = large_current_time.QuadPart;
        f32 debug_current_ms = (f32)(current_time - last_time) / (f32)large_frequency.QuadPart;
        last_time = current_time;

        MSG message;
        while(PeekMessageA(&message, window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // NOTE: Update TGUI lib
        tgui_update();
        // NOTE: clear the screen
        tgui_clear_backbuffer(&tgui_backbuffer);
         
        char debug_str[256];
        u32 font_height = 9;
        sprintf(debug_str, "mouse pos (x:%d, y:%d)", tgui_global_state.mouse_x, tgui_global_state.mouse_y);
        tgui_draw_text(&tgui_backbuffer, &test_font, font_height, 0, tgui_backbuffer.height - font_height, debug_str);
        
        sprintf(debug_str, "ms:%.3f", debug_current_ms);
        tgui_draw_text(&tgui_backbuffer, &test_font, font_height, 0, 0, debug_str);
        sprintf(debug_str, "fps:%d", (u32)(1.0f/debug_current_ms+0.5f));
        tgui_draw_text(&tgui_backbuffer, &test_font, font_height, 0, font_height, debug_str);

        tgui_draw_bitmap(&tgui_backbuffer, &test_bitmap, tgui_backbuffer.width - test_bitmap.width, 0, test_bitmap.width, test_bitmap.height);
        
        static int button = 0;
        {
            i32 window = 0; 
            tgui_begin_window(&window, &window_descriptor_1);
            if(tgui_button((void *)1, "test_button_1", 0, 0))
            {
                button = 1;
            }
            if(tgui_button((void *)2, "test_button_2", 0, 0))
            {
                button = 2;
            }
            if(tgui_button((void *)3, "test_button_3", 0, 0))
            {
                button = 3;
            }
            if(tgui_button((void *)4, "test_button_4", 0, 0))
            {
                button = 4;
            }
            if(tgui_button((void *)5, "test_button_5", 0, 0))
            {
                button = 5;
            }
            tgui_end_window(&window);
        }

        {
            i32 window = 10;
            tgui_begin_window(&window, &window_descriptor_2);
            if(tgui_button((void *)6, "test_button_6", 0, 0))
            {
                button = 6;
            }
            if(tgui_button((void *)7, "test_button_7", 0, 0))
            {
                button = 7;
            }
            tgui_end_window(&window);
        }
        
        char button_text[64];
        sprintf(button_text, "button %d pressed!", button);
        tgui_draw_text(&tgui_backbuffer, &test_font, font_height, 0, tgui_backbuffer.height - font_height*2, button_text); 
        
        tgui_draw_command_buffer();
        
        //----------------------------------------------------------------
        // NOTE: tree of widgets build test
        tgui_begin_widget();
            
            tgui_begin_widget();
                tgui_begin_widget();
                    tgui_begin_widget();
                    tgui_end_widget();
                    tgui_begin_widget();
                    tgui_end_widget();
                tgui_end_widget();
            tgui_end_widget();
            
            tgui_begin_widget();
                tgui_begin_widget();
                tgui_end_widget();
            tgui_end_widget();
            
        tgui_end_widget();
    
        TGuiWidgetNode *root = tgui_global_state.root_widget;
        u32 break_here = 0;
        UNUSED_VAR(break_here);
        tgui_recursive_descent_tree(root, 400, 50);
        
        // NOTE: tree of widgets draw for debuging
        
        //----------------------------------------------------------------

        // NOTE: Blt the backbuffer on to the destination window
        BitBlt(global_device_context, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, global_backbuffer_dc, 0, 0, SRCCOPY);
    }
    
    tgui_debug_free_bmp(&test_bitmap);
    tgui_debug_free_bmp(&dog_bitmap);

    return 0;
}
