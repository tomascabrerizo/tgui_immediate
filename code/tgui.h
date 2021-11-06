#ifndef TGUI_H
#define TGUI_H

#include <stdint.h>
#include <assert.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint32_t b32;

typedef float f32;
typedef double f64;

#define true 1
#define false 0
#define ASSERT(value) assert(value);
#define UNUSED_VAR(x) ((void)x)
#define OFFSET_OFF(s, p) (u64)(&(((s *)0)->p))
#define TGUI_API __declspec(dllexport)

// TODO: TGuiBackbuffer and TGuiBitmap are practicaly the same thing
typedef struct TGuiBackbuffer
{
    void *data;
    u32 width;
    u32 height;
    u32 pitch;
} TGuiBackbuffer;

typedef struct TGuiBitmap
{
    u32 *pixels;
    u32 width;
    u32 height;
} TGuiBitmap;

typedef struct TGuiRect
{
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} TGuiRect;

// NOTE: for now, only support for bitmaps fonts
typedef struct TGuiFont
{
    TGuiBitmap *bitmap;
    TGuiRect src_rect;
    u32 num_rows;
    u32 num_cols;
    // TODO: for now num_cols, src_rect.x and src_rect.y arent used
    // maybe remove them
} TGuiFont;

// NOTE: DEBUG function
TGuiBitmap tgui_debug_load_bmp(char *path);
void tgui_debug_free_bmp(TGuiBitmap *bitmap);

// NOTE: simple render API function
TGUI_API void tgui_clear_backbuffer(TGuiBackbuffer *backbuffer);
TGUI_API void tgui_draw_rect(TGuiBackbuffer *backbuffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, u32 color);
TGUI_API void tgui_copy_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, i32 x, i32 y);
TGUI_API void tgui_draw_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, i32 width, i32 height, i32 x, i32 y);
TGUI_API void tgui_draw_src_dest_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, TGuiRect src, TGuiRect dest);

// NOTE: font funtions
// TODO: stop using char * (null terminated string) create custom string_view like struct
// NOTE: height is in pixels
TGUI_API void tgui_draw_char(TGuiBackbuffer *backbuffer, TGuiFont *font, u32 height, i32 x, i32 y, char character);
TGUI_API void tgui_draw_text(TGuiBackbuffer *backbuffer, TGuiFont *font, u32 height, i32 x, i32 y, char *text);

#endif // TGUI_WIN32_H
