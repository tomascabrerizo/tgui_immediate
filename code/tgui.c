#include "tgui.h"

// TODO: stop using std lib to load files in the future
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Maybe the state should be provided by the application?
TGuiState tgui_global_state;

void tgui_set_hot(void *id)
{
    TGuiState *state = &tgui_global_state;
    state->hot.id = id; 
}

void tgui_set_active(void *id)
{
    TGuiState *state = &tgui_global_state;
    state->active.id = id; 
}

b32 tgui_is_over(TGuiRect rect)
{
    i32 min_x = rect.x;
    i32 min_y = rect.y;
    i32 max_x = rect.x + rect.width;
    i32 max_y = rect.y + rect.height;
    TGuiState *state = &tgui_global_state;
    b32 result = state->mouse_x >= min_x && state->mouse_x <= max_x &&
                 state->mouse_y >= min_y && state->mouse_y <= max_y;
    return result;
}


void tgui_compute_next_widget_pos(TGuiWindowDescriptor *window_descriptor, u32 widget_height)
{
    window_descriptor->next_y +=  window_descriptor->margin + widget_height;
}

b32 tgui_is_hot(void *id)
{
    TGuiState *state = &tgui_global_state;
    b32 result = state->hot.id == id; 
    return result; 
}

b32 tgui_is_active(void *id)
{
    TGuiState *state = &tgui_global_state;
    b32 result = state->active.id == id; 
    return result; 
}

// NOTE: core lib functions
void tgui_init(TGuiBackbuffer *backbuffer, TGuiFont *font)
{
    TGuiState *state = &tgui_global_state;
    state->backbuffer = backbuffer;
    
    state->font = font;
    state->font_height = 9;
    
    state->event_queue_count = 0;
    state->draw_command_buffer.top = 0;
}

void tgui_push_event(TGuiEvent event)
{
    TGuiState *state = &tgui_global_state;
    if(state->event_queue_count < TGUI_EVENT_QUEUE_MAX)
    {
        state->event_queue[state->event_queue_count++] = event;
    }
}

void tgui_push_draw_command(TGuiDrawCommand draw_command)
{
    TGuiState *state = &tgui_global_state;
    if(state->draw_command_buffer.top < TGUI_DRAW_COMMANDS_MAX)
    {
        state->draw_command_buffer.buffer[state->draw_command_buffer.top++] = draw_command;
    }
}

b32 tgui_pull_draw_command(TGuiDrawCommand *draw_cmd)
{
    TGuiState *state = &tgui_global_state;
    i32 current_top = (i32)state->draw_command_buffer.top - 1;
    if(current_top < 0)
    {
        return false;
    }
    *draw_cmd = state->draw_command_buffer.buffer[current_top];
    --state->draw_command_buffer.top;
    return true;
}

b32 tgui_button(void *id, char *text, i32 x, i32 y)
{
    TGuiState *state = &tgui_global_state;
    

    u32 text_height = state->font_height;
    u32 text_width = tgui_get_text_wdith(state->font, text, text_height);
    u32 h_padding = 20;
    u32 v_padding = 30;
    TGuiRect rect = {x, y, text_width + h_padding, text_height + v_padding};
    
    // TODO: this code seems to be the window layout code
    // TODO: create a function to set the differents layous
    if(state->parent_window)
    {
        rect.x = state->window_descriptor->next_x;
        rect.y = state->window_descriptor->next_y;
        tgui_compute_next_widget_pos(state->window_descriptor, rect.height);
        if(state->window_descriptor->current_width < (rect.width + state->window_descriptor->margin*2))
        {
            state->window_descriptor->current_width = (rect.width + state->window_descriptor->margin*2);
        }
        state->window_descriptor->current_height += rect.height + state->window_descriptor->margin;
    }

    b32 result = false;
    b32 is_over = tgui_is_over(rect);

    if(tgui_is_active(id))
    {
        if(state->mouse_up)
        {
            if(tgui_is_hot(id) && is_over)
            {
                result = true;
            }
            tgui_set_active(0);
        }
    }
    else if(tgui_is_hot(id))
    {
        if(state->mouse_down)
        {
            tgui_set_active(id);
        }
        if(!is_over)
        {
            tgui_set_hot(0);
        }
    }
    if(is_over)
    {
        tgui_set_hot(id);
    }
    
    u32 color = TGUI_GREY; 
    if(tgui_is_hot(id)) color = TGUI_GREY;
    if(tgui_is_active(id)) color = TGUI_GREEN;
    if(result) color = TGUI_RED;

    TGuiDrawCommand text_command = {0};
    text_command.type = TGUI_DRAWCMD_TEXT;
    text_command.descriptor.x = rect.x + h_padding*0.5f;
    text_command.descriptor.y = rect.y + v_padding*0.5f;
    text_command.descriptor.width = tgui_get_text_wdith(state->font, text, state->font_height);
    text_command.descriptor.height = state->font_height;
    text_command.text = text;
    tgui_push_draw_command(text_command);
    
    TGuiDrawCommand rect_command = {0};
    rect_command.type = TGUI_DRAWCMD_RECT;
    rect_command.descriptor = rect;
    rect_command.color = color;
    tgui_push_draw_command(rect_command);
    
    return result;
}

void tgui_label(void *id, char *text, i32 x, i32 y)
{
    // TODO: labels should care about hot active?
    UNUSED_VAR(id);
    TGuiState *state = &tgui_global_state;
    
    TGuiDrawCommand text_command = {0};
    text_command.type = TGUI_DRAWCMD_TEXT;
    text_command.descriptor.x = x;
    text_command.descriptor.y = y;
    text_command.descriptor.width = tgui_get_text_wdith(state->font, text, state->font_height);
    text_command.descriptor.height = state->font_height;
    text_command.text = text;
    tgui_push_draw_command(text_command);
}

void tgui_begin_window(void *id, TGuiWindowDescriptor *window_descriptor)
{
    TGuiState *state = &tgui_global_state;
    
    ASSERT(window_descriptor && "window must have a descriptor");

    state->parent_window = id;
    
    window_descriptor->next_x = window_descriptor->x + window_descriptor->margin;
    window_descriptor->next_y = window_descriptor->y;
    tgui_compute_next_widget_pos(window_descriptor, 0);
    state->window_descriptor = window_descriptor;
}

void tgui_end_window(void *id)
{
    UNUSED_VAR(id);
    TGuiState *state = &tgui_global_state;
     
    TGuiRect window_descriptor;
    window_descriptor.x = state->window_descriptor->x;
    window_descriptor.y = state->window_descriptor->y;
    window_descriptor.width = state->window_descriptor->current_width;
    window_descriptor.height = state->window_descriptor->current_height + state->window_descriptor->margin;

    TGuiDrawCommand rect_command = {0};
    rect_command.type = TGUI_DRAWCMD_RECT;
    rect_command.descriptor = window_descriptor;
    rect_command.color = TGUI_BLACK;
    tgui_push_draw_command(rect_command);

    state->parent_window = 0;
    state->window_descriptor = 0;
}

void tgui_update(void)
{
    TGuiState *state = &tgui_global_state;
    // NOTE: clear old state that are not needed any more
    state->mouse_up = false;
    state->mouse_down = false;
    // NOTE: pull tgui events from the queue
    for(u32 event_index = 0; event_index < state->event_queue_count; ++event_index)
    {
        TGuiEvent *event = state->event_queue + event_index;
        switch(event->type)
        {
            case TGUI_EVENT_KEYDOWN:
            {
                printf("key down event!\n");  
            } break;
            case TGUI_EVENT_KEYUP:
            {
                printf("key up event!\n");  
            } break;
            case TGUI_EVENT_MOUSEMOVE:
            {
                TGuiEventMouseMove *mouse = (TGuiEventMouseMove *)event;
                state->mouse_x = mouse->pos_x;
                state->mouse_y = mouse->pos_y;
            } break;
            case TGUI_EVENT_MOUSEDOWN:
            {
                state->mouse_down = true;
            } break;
            case TGUI_EVENT_MOUSEUP:
            {
                state->mouse_up = true;
            } break;
            default:
            {
                ASSERT(!"invalid code path");
            } break;
        }
    }
    state->event_queue_count = 0;
}

void tgui_draw_command_buffer(void)
{
    TGuiState *state = &tgui_global_state;
    // NOTE: pull tgui draw commands from the buffer 
    TGuiDrawCommand draw_cmd;
    while(tgui_pull_draw_command(&draw_cmd))
    {
        switch(draw_cmd.type)
        {
            case TGUI_DRAWCMD_CLEAR:
            {
                tgui_clear_backbuffer(state->backbuffer);
            } break;
            case TGUI_DRAWCMD_RECT:
            {
                i32 max_x = draw_cmd.descriptor.x + draw_cmd.descriptor.width;
                i32 max_y = draw_cmd.descriptor.y + draw_cmd.descriptor.height;
                tgui_draw_rect(state->backbuffer, draw_cmd.descriptor.x, draw_cmd.descriptor.y, max_x, max_y, draw_cmd.color);
            } break;
            case TGUI_DRAWCMD_BITMAP:
            {
                tgui_draw_bitmap(state->backbuffer, draw_cmd.bitmap, draw_cmd.descriptor.x, draw_cmd.descriptor.y, draw_cmd.descriptor.width, draw_cmd.descriptor.height);
            } break;
            case TGUI_DRAWCMD_TEXT:
            {
                tgui_draw_text(state->backbuffer, state->font, state->font_height, draw_cmd.descriptor.x, draw_cmd.descriptor.y, draw_cmd.text);
            } break;
            default:
            {
                ASSERT(!"invalid code path");
            } break;
        }
    }
}

// NOTE: DEBUG functions

// NOTE: only use in implementation
typedef struct TGuiBmpHeader
{
    u16 id;
    u32 file_size;
    u32 reserved;
    u32 pixel_array_offset;
    u32 dib_header_size;
    u32 width;
    u32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 bmp_size;
} TGuiBmpHeader;

// TODO: dont know if this macro is a good idea, probably not needed
#define READ_U16(data) *((u16 *)data); data = (u16 *)data + 1
#define READ_U32(data) *((u32 *)data); data = (u32 *)data + 1
#define READ_U64(data) *((u64 *)data); data = (u64 *)data + 1

TGuiBitmap tgui_debug_load_bmp(char *path)
{
    FILE *bmp_file = fopen(path, "rb");
    if(bmp_file)
    {
        fseek(bmp_file, 0, SEEK_END);
        u64 bmp_file_size = ftell(bmp_file);
        fseek(bmp_file, 0, SEEK_SET);
        void *bmp_data = (void *)malloc(bmp_file_size); 
        fread(bmp_data, bmp_file_size, 1, bmp_file);
        fclose(bmp_file);
        
        void *temp_bmp_data = bmp_data;
        TGuiBmpHeader bmp_header = {0};
        bmp_header.id = READ_U16(temp_bmp_data);
        bmp_header.file_size = READ_U32(temp_bmp_data);
        bmp_header.reserved = READ_U32(temp_bmp_data);
        bmp_header.pixel_array_offset = READ_U32(temp_bmp_data);
        bmp_header.dib_header_size = READ_U32(temp_bmp_data);
        bmp_header.width = READ_U32(temp_bmp_data);
        bmp_header.height = READ_U32(temp_bmp_data);
        bmp_header.planes = READ_U16(temp_bmp_data);
        bmp_header.bits_per_pixel = READ_U16(temp_bmp_data);
        bmp_header.compression = READ_U32(temp_bmp_data);
        bmp_header.bmp_size = READ_U32(temp_bmp_data);
    
        // NOTE: for now only allow 32bits bitmaps
        u32 bytes_per_pixel = bmp_header.bits_per_pixel / 8;
        if(bytes_per_pixel != sizeof(u32))
        {
            ASSERT(!"[ERROR]: only support for 32 bits bitmaps\n");
            return (TGuiBitmap){0};
        }

        u8 *bmp_src = (u8 *)bmp_data + bmp_header.pixel_array_offset;
        TGuiBitmap result = {0};
        result.width = bmp_header.width;
        result.height = bmp_header.height;
        u64 bitmap_size = result.width * result.height * bytes_per_pixel;
        result.pixels = (u32 *)malloc(bitmap_size); 
        
        // NOTE: cannot use memcpy, the bitmap must be flipped 
        // TODO: copy the bitmap pixels in a faster way
        u32 *src_row = (u32 *)bmp_src + (result.height-1) * bmp_header.width;
        u32 *dst_row = result.pixels;
        for(u32 y = 0; y < result.height; ++y)
        {
            u32 *src_pixels = src_row;
            u32 *dst_pixels = dst_row;
            for(u32 x = 0; x < result.width; ++x)
            {
                
                *dst_pixels++ = *src_pixels++;
            }
            src_row -= bmp_header.width;
            dst_row += result.width;
        }
        
        free(bmp_data);
        return result;
    }
    
    // TODO: create a log to print errors
    ASSERT(!"[ERROR]: cannot open file\n");
    return (TGuiBitmap){0};
}

void tgui_debug_free_bmp(TGuiBitmap *bitmap)
{
    free(bitmap->pixels);
    bitmap->pixels = 0;
    bitmap->width = 0;
    bitmap->height = 0;
}

void tgui_clear_backbuffer(TGuiBackbuffer *backbuffer)
{
    size_t backbuffer_size = (backbuffer->width*backbuffer->height*sizeof(u32));
    memset(backbuffer->data, 0, backbuffer_size);
}

// NOTE: simple rendering API

void tgui_draw_rect(TGuiBackbuffer *backbuffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, u32 color)
{
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(max_x > (i32)backbuffer->width)
    {
        max_x = backbuffer->width;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_y > (i32)backbuffer->height)
    {
        max_y = backbuffer->height;
    }

    i32 width = max_x - min_x;
    i32 height = max_y - min_y;

    u8 *row = (u8 *)backbuffer->data + min_y * backbuffer->pitch;
    for(i32 y = 0; y < height; ++y)
    {
        u32 *pixels = (u32 *)row + min_x;
        for(i32 x = 0; x < width; ++x)
        {
            *pixels++ = color;
        }
        row += backbuffer->pitch;
    }
}

void tgui_copy_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, i32 x, i32 y)
{
    i32 min_x = x;
    i32 min_y = y;
    i32 max_x = min_x + bitmap->width;
    i32 max_y = min_y + bitmap->height;
     
    u32 offset_x = 0;
    u32 offset_y = 0;
    if(min_x < 0)
    {
        offset_x = -min_x;
        min_x = 0;
    }
    if(max_x > (i32)backbuffer->width)
    {
        max_x = backbuffer->width;
    }
    if(min_y < 0)
    {
        offset_y = -min_y;
        min_y = 0;
    }
    if(max_y > (i32)backbuffer->height)
    {
        max_y = backbuffer->height;
    }

    i32 width = max_x - min_x;
    i32 height = max_y - min_y;

    u8 *row = (u8 *)backbuffer->data + min_y * backbuffer->pitch;
    u32 *bmp_row = bitmap->pixels + offset_y * bitmap->width;
    for(i32 y = 0; y < height; ++y)
    {
        u32 *pixels = (u32 *)row + min_x;
        u32 *bmp_pixels = bmp_row + offset_x;
        for(i32 x = 0; x < width; ++x)
        {
            *pixels++ = *bmp_pixels++;
        }
        row += backbuffer->pitch;
        bmp_row += bitmap->width;
    }
}

// TODO: create a round_f32u32() function
void tgui_draw_src_dest_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, TGuiRect src, TGuiRect dest)
{
    // TODO: Implment alpha bending
    // TODO: Implment filtering for the re-scale bitmap
    i32 min_x = dest.x;
    i32 min_y = dest.y;
    i32 max_x = min_x + dest.width;
    i32 max_y = min_y + dest.height;
     
    u32 offset_x = 0;
    u32 offset_y = 0;
    if(min_x < 0)
    {
        offset_x = -min_x;
        min_x = 0;
    }
    if(max_x > (i32)backbuffer->width)
    {
        max_x = backbuffer->width;
    }
    if(min_y < 0)
    {
        offset_y = -min_y;
        min_y = 0;
    }
    if(max_y > (i32)backbuffer->height)
    {
        max_y = backbuffer->height;
    }
    
    // TODO: this clipping is not necessary, the src rect should always be correct
    // NOTE: clip src rect to the bitmap
    i32 src_min_x = src.x;
    i32 src_min_y = src.y;
    i32 src_max_x = src_min_x + src.width;
    i32 src_max_y = src_min_y + src.height;
     
    if(src_min_x < 0)
    {
        src_min_x = 0;
    }
    if(src_max_x > (i32)bitmap->width)
    {
        src_max_x = bitmap->width;
    }
    if(src_min_y < 0)
    {
        src_min_y = 0;
    }
    if(src_max_y > (i32)bitmap->height)
    {
        src_max_y = bitmap->height;
    }
    
    i32 dest_width = max_x - min_x;
    i32 dest_height = max_y - min_y;
    i32 src_width = src_max_x - src_min_x;
    i32 src_height = src_max_y - src_min_y;

    u8 *row = (u8 *)backbuffer->data + min_y * backbuffer->pitch;
    for(i32 y = 0; y < dest_height; ++y)
    {
        f32 ratio_y = (f32)(y + offset_y) / (f32)dest.height;
        u32 bitmap_y = src_min_y + (u32)((f32)src_height * ratio_y + 0.5f);
        u32 *pixels = (u32 *)row + min_x;
        for(i32 x = 0; x < dest_width; ++x)
        {
            f32 ratio_x = (f32)(x + offset_x) / (f32)dest.width;
            u32 bitmap_x = src_min_x + (u32)((f32)src_width * ratio_x + 0.5f);
            u32 src_color = bitmap->pixels[bitmap_y*bitmap->width + bitmap_x];
            // TODO: implements real alpha bending
            u8 alpha = (u8)((src_color >> 24) & 0xFF);
            if(alpha > 128) 
            {
                *pixels = src_color;
            }
            pixels++;
        }
        row += backbuffer->pitch;
    }
}

void tgui_draw_bitmap(TGuiBackbuffer *backbuffer, TGuiBitmap *bitmap, i32 x, i32 y, i32 width, i32 height)
{
    TGuiRect src = (TGuiRect){0, 0, bitmap->width, bitmap->height};
    TGuiRect dest = (TGuiRect){x, y, width, height};
    tgui_draw_src_dest_bitmap(backbuffer, bitmap, src, dest);
}

// NOTE: font funtions

TGuiFont tgui_create_font(TGuiBitmap *bitmap, u32 char_width, u32 char_height, u32 num_rows, u32 num_cols)
{
    TGuiFont result = {0};
    result.src_rect.x = 0;
    result.src_rect.y = 0;
    result.src_rect.width = char_width;
    result.src_rect.height = char_height;
    result.num_rows = num_rows;
    result.num_cols = num_cols;
    result.bitmap = bitmap;
    return result;
}

void tgui_draw_char(TGuiBackbuffer *backbuffer, TGuiFont *font, u32 height, i32 x, i32 y, char character)
{
    ASSERT(font->bitmap && "font must have a bitmap");
    u32 index = (character - ' ');
    u32 x_index = index % font->num_rows;
    u32 y_index = index / font->num_rows;
    TGuiRect src = (TGuiRect){
        x_index*font->src_rect.width, 
        y_index*font->src_rect.height, 
        font->src_rect.width, 
        font->src_rect.height
    };
    f32 w_ration = (f32)font->src_rect.width / (f32)font->src_rect.height;
    TGuiRect dest = (TGuiRect){x, y, (w_ration * (f32)height + 0.5f), height};
    tgui_draw_src_dest_bitmap(backbuffer, font->bitmap, src, dest); 
}

void tgui_draw_text(TGuiBackbuffer *backbuffer, TGuiFont *font, u32 height, i32 x, i32 y, char *text)
{
    // TODO: the w_ratio is been calculated in two time
    f32 w_ration = (f32)font->src_rect.width / (f32)font->src_rect.height;
    u32 width = (u32)(w_ration * (f32)height + 0.5f);
     
    u32 length = strlen((const char *)text);
    for(u32 index = 0; index < length; ++index)
    {
        tgui_draw_char(backbuffer, font, height, x, y, text[index]);
        x += width;
    }
}

u32 tgui_get_text_wdith(TGuiFont *font, char *text, u32 height)
{
    f32 w_ration = (f32)font->src_rect.width / (f32)font->src_rect.height;
    u32 width = (u32)(w_ration * (f32)height + 0.5f);
     
    u32 length = strlen((const char *)text);
    u32 result = width * length;
    return result;
}
