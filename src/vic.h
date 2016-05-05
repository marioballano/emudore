/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EMUDORE_VIC_H
#define EMUDORE_VIC_H

#include "io.h"
#include "cpu.h"
#include "memory.h"

/**
 * @brief MOS Video Interface Chip II (VIC-II)
 *
 * MOS 6567 NTSC
 * MOS 6569 PAL
 *
 * This class implements the PAL version of the chip
 */
class Vic
{
  private:
    Memory *mem_;
    Cpu    *cpu_;
    IO     *io_;
    /* sprites  */
    uint8_t mx_[8];
    uint8_t my_[8];
    uint8_t msbx_;
    uint8_t sprite_enabled_;
    uint8_t sprite_priority_;
    uint8_t sprite_multicolor_;
    uint8_t sprite_double_width_;
    uint8_t sprite_double_height_;
    uint8_t sprite_shared_colors_[2];
    uint8_t sprite_colors_[8];
    /* background and border colors */
    uint8_t border_color_;
    uint8_t bgcolor_[4];
    /* cpu sync */
    unsigned int next_raster_at_;
    /* frame counter */
    unsigned int frame_c_;
    /* control registers */
    uint8_t cr1_;
    uint8_t cr2_;
    /* raster */
    uint8_t raster_c_;
    int raster_irq_;
    /* interrupt control */
    uint8_t irq_status_;
    uint8_t irq_enabled_;
    /* screen, character memory and bitmap addresses */
    uint16_t screen_mem_;
    uint16_t char_mem_;
    uint16_t bitmap_mem_;
    uint8_t  mem_pointers_;
    /* helpers */
    inline void raster_counter(int v);
    inline int raster_counter();
    inline bool is_screen_off();
    inline bool is_bad_line();
    inline bool raster_irq_enabled();
    inline uint8_t vertical_scroll();
    inline uint8_t horizontal_scroll();
    inline bool is_sprite_enabled(int n);
    inline bool is_background_sprite(int n);
    inline bool is_double_width_sprite(int n);
    inline bool is_double_height_sprite(int n);
    inline bool is_multicolor_sprite(int n);
    inline int sprite_x(int n);
    /* graphics */ 
    inline void draw_raster_char_mode();
    inline void draw_raster_bitmap_mode();
    inline void draw_raster_sprites();
    inline void draw_sprite(int x, int y, int sprite, int row);
    inline void draw_mcsprite(int x, int y, int sprite, int row);
    inline void draw_char(int x, int y, uint8_t data, uint8_t color);
    inline void draw_mcchar(int x, int y, uint8_t data, uint8_t color);
    inline void draw_bitmap(int x, int y, uint8_t data, uint8_t color);
    inline void draw_mcbitmap(int x, int y, uint8_t data, uint8_t scolor, uint8_t rcolor);
    inline uint8_t get_screen_char(int column, int row);
    inline uint8_t get_char_color(int column, int row);
    inline uint8_t get_char_data(int chr, int line);
    inline uint8_t get_bitmap_data(int column, int row, int line);
    inline uint16_t get_sprite_ptr(int n);
    inline void set_graphic_mode();
  public:
    Vic();
    bool emulate();
    void memory(Memory *v){mem_ = v;};
    void cpu(Cpu *v){cpu_ = v;};
    void io(IO *v){io_ = v;};
    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);
    unsigned int frames(){return frame_c_;};
    /* constants */
    static const int kScreenLines = 312;
    static const int kScreenCols  = 504;
    static const int kVisibleScreenWidth  = 403;
    static const int kVisibleScreenHeight = 284;
    static const int kFirstVisibleLine = 14;
    static const int kLastVisibleLine = 298;
    static const int kLineCycles = 63;
    static const int kBadLineCycles = 23;
    static constexpr double kRefreshRate = 1 / 50.125; // ~50Hz (PAL)
    static const int kSpritePtrsOffset = 0x3f8;
    /* graphic modes */
    enum kGraphicMode
    {
      kCharMode,
      kMCCharMode,
      kBitmapMode,
      kMCBitmapMode,
      kExtBgMode,
      kIllegalMode,
    };
    kGraphicMode graphic_mode_;
    /* graphics constants */
    static const int kGResX = 320;
    static const int kGResY = 200;
    static const int kGCols = 40;
    static const int kGRows = 25;
    static const int kGFirstLine = 56;
    static const int kGLastLine  = 256;
    static const int kGFirstCol  = 42;
    /* sprites */
    static const int kSpriteWidth = 24;
    static const int kSpriteHeight = 21;
    static const int kSpriteSize = 64;
    static const int kSpritesFirstLine = 6;
    static const int kSpritesFirstCol = 18;
};

#endif
