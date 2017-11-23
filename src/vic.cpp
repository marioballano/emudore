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

#include "vic.h"
#include "util.h"

// ctor and emulate()  ///////////////////////////////////////////////////////

Vic::Vic()
{
  /* raster */
  raster_irq_ = raster_c_ = 0; 
  irq_enabled_ = irq_status_ = 0;
  next_raster_at_ = kLineCycles;
  /* sprites */
  for(int i = 0 ; i<8 ; i++)
  {
    mx_[i] = my_[i] = sprite_colors_[i] = 0;
  }
  msbx_ = sprite_double_height_ = sprite_double_width_ = 0;
  sprite_enabled_ = sprite_priority_ = sprite_multicolor_ = 0;       
  sprite_shared_colors_[0] = sprite_shared_colors_[1] = 0;
  /* colors */
  border_color_ = 0;
  bgcolor_[0] = bgcolor_[1] = bgcolor_[2] = bgcolor_[3] = 0;
  /* control regs */
  cr1_ = cr2_ = 0;
  /* frame counter */
  frame_c_ = 0;
  /* default memory pointers */
  screen_mem_ = Memory::kBaseAddrScreen;
  char_mem_   = Memory::kBaseAddrChars;
  bitmap_mem_ = Memory::kBaseAddrBitmap;
  /* bit 0 is unused */
  mem_pointers_ = (1 << 0);
  /* current graphic mode */
  graphic_mode_ = kCharMode;
}

bool Vic::emulate()
{
  /** 
   * if there are unacknowledged interrupts 
   * raise an interrupt again
   **/
  if((read_register(0x19) & 0x80) != 0)
  {
    cpu_->irq();
  }
  /* are we at the next raster line? */
  if (cpu_->cycles() >= next_raster_at_)
  {
    int rstr = raster_counter();
    /* check raster IRQs */
    if (raster_irq_enabled() &&
        rstr == raster_irq_)
    {
      /* set interrupt origin (raster) */
      irq_status_ |= (1<<0);
      /* raise interrupt */
      cpu_->irq();
    }
    if (rstr >= kFirstVisibleLine &&
        rstr < kLastVisibleLine)
    {
      /* draw border */
      int screen_y = rstr - kFirstVisibleLine;
      io_->screen_draw_border(screen_y,border_color_);
      /* draw raster on current graphic mode */
      switch(graphic_mode_)
      {
      case kCharMode:
      case kMCCharMode:
        draw_raster_char_mode();
        break;
      case kBitmapMode:
      case kMCBitmapMode:
        draw_raster_bitmap_mode();
        break;
      default:
        D("unsupported graphic mode: %d\n",graphic_mode_);
        return false;
      }
      /* draw sprites */
      draw_raster_sprites();
    }
    /* next raster */
    if(is_bad_line())
      next_raster_at_+= kBadLineCycles;
    else
      next_raster_at_+= kLineCycles;
    /* update raster */
    raster_counter(++rstr);
    if (rstr >= kScreenLines)
    {
      io_->screen_refresh();
      frame_c_++;
      raster_counter(0);
    }
  }
  return true;
}

// DMA register access  //////////////////////////////////////////////////////

uint8_t Vic::read_register(uint8_t r)
{
  uint8_t retval;
  switch(r)
  {
  /* get X coord of sprite n*/
  case 0x0:
  case 0x2:
  case 0x4:
  case 0x6:
  case 0x8:
  case 0xc:
  case 0xe:
    retval = mx_[r >> 1];
    break;
  /* get Y coord of sprite n */
  case 0x1:
  case 0x3:
  case 0x5:
  case 0x7:
  case 0x9:
  case 0xb:
  case 0xd:
  case 0xf:
    retval = my_[r >> 1];
    break;
  /* MSBs of sprites X coordinates */
  case 0x10:
    retval = msbx_;
    break;
  /* control register 1 */
  case 0x11:
    retval = cr1_;
    break;
  /* raster counter */
  case 0x12:
    retval = raster_c_;
    break;
  /* sprite enable register */
  case 0x15:
    retval = sprite_enabled_;
    break;
  /* control register 2 */
  case 0x16:
    retval = cr2_;
    break;
  /* sprite double height */
  case 0x17:
    retval = sprite_double_height_;
    break;
  /* memory pointers */
  case 0x18:
    retval = mem_pointers_;
    break;
  /** 
   * interrupt status register 
   * IRQ|  - |  - |  - | ILP|IMMC|IMBC|IRST|
   */
  case 0x19:
    retval = (0xf & irq_status_);
    if(retval!=0) retval |= 0x80; // IRQ bit 
    retval |= 0x70; // non-connected bits (always set)
    break;
  /** 
   * interrupt enable register 
   *   - |  - |  - |  - | ELP|EMMC|EMBC|ERST|
   */
  case 0x1a:
    retval = (0xf0 | irq_enabled_);
    break;
  /* sprite priority register */
  case 0x1b:
    retval = sprite_priority_;
    break;
  /* sprite multicolor mode */
  case 0x1c:
    retval = sprite_multicolor_;
    break;
  /* sprite double width */
  case 0x1d:
    retval = sprite_double_width_;
    break;
  /* border color */
  case 0x20:
    retval = border_color_;
    break;
  /* background colors */
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
    retval = bgcolor_[r-0x21];
    break;
  /* sprite colors */
  case 0x25:
  case 0x26:
    retval = sprite_shared_colors_[r-0x25];
    break;
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2c:
  case 0x2d:
  case 0x2e:
    retval = sprite_colors_[r-0x27];
    break;
  /* unused */
  case 0x2f:
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x39:
  case 0x3a:
  case 0x3b:
  case 0x3c:
  case 0x3d:
  case 0x3e:
  case 0x3f:
  default:
    retval = 0xff;
    break;
  }
  return retval;
}

void Vic::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
  /* store X coord of sprite n*/
  case 0x0:
  case 0x2:
  case 0x4:
  case 0x6:
  case 0x8:
  case 0xc:
  case 0xe:
    mx_[r >> 1] = v;
    break;
  /* store Y coord of sprite n */
  case 0x1:
  case 0x3:
  case 0x5:
  case 0x7:
  case 0x9:
  case 0xb:
  case 0xd:
  case 0xf:
    my_[r >> 1] = v;
    break;
  /* MSBs of X coordinates */
  case 0x10:
    msbx_ = v;
    break;
  /* control register 1 */
  case 0x11:
    cr1_ = (v&0x7f);
    raster_irq_ &= 0xff;
    raster_irq_ |= (v&0x80) << 1;
    set_graphic_mode();
    break;
  /* raster irq */
  case 0x12:
    raster_irq_ = v | (raster_irq_ & (1 << 8));
    break;
  /* sprite enable register */
  case 0x15:
    sprite_enabled_ = v;
    break;
  /* control register 2 */
  case 0x16:
    cr2_ = v;
    set_graphic_mode();
    break;
  /* sprite double height */
  case 0x17:
    sprite_double_height_ = v;
    break;
  /* memory pointers  */
  case 0x18:
    /* bits ----xxx- */
    char_mem_   = (v&0xe) << 10;
    /* bits xxxx---- */
    screen_mem_ = (v&0xf0) << 6;
    /* bit  ----x--- */
    bitmap_mem_ = (v&0x8) << 10;
    /* save reg value (last bit always set)*/
    mem_pointers_ = v | (1 << 0);
    break;
  /* interrupt request register */
  case 0x19:
    /* acknowledge interrupts by mask */
    irq_status_ &= ~(v&0xf);
    break;
  /* interrupt enable register */
  case 0x1a:
    irq_enabled_= v;
    break;
  /* sprite priority register */
  case 0x1b:
    sprite_priority_ = v;
    break;
  /* sprite multicolor mode */
  case 0x1c:
    sprite_multicolor_ = v;
    break;
  /* sprite double width */
  case 0x1d:
    sprite_double_width_ = v;
    break;
  /* border color */
  case 0x20:
    border_color_ = v;
    break;
  /* background colors */
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
    bgcolor_[r-0x21] = v;
    break;
  /* sprite colors */
  case 0x25:
  case 0x26:
    sprite_shared_colors_[r-0x25] = v;
    break;
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2c:
  case 0x2d:
  case 0x2e:
    sprite_colors_[r-0x27] = v;
    break;
  /* unused */
  case 0x2f:
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x39:
  case 0x3a:
  case 0x3b:
  case 0x3c:
  case 0x3d:
  case 0x3e:
  case 0x3f:
  default:
    break;
  }
}       

// graphics //////////////////////////////////////////////////////////////////

/**
 * @brief set current graphic mode 
 */
void Vic::set_graphic_mode()
{
  bool ecm = ((cr1_ & (1<<6))!= 0);
  bool bmm = ((cr1_ & (1<<5))!= 0);
  bool mcm = ((cr2_ & (1<<4))!= 0);

  if(!ecm && !bmm && !mcm) 
    graphic_mode_ = kCharMode;
  else if(!ecm && !bmm && mcm)
    graphic_mode_ = kMCCharMode;
  else if(!ecm && bmm && !mcm)
    graphic_mode_ = kBitmapMode;
  else if(!ecm && bmm && mcm)
    graphic_mode_ = kMCBitmapMode;
  else if(ecm && !bmm && !mcm)
    graphic_mode_ = kExtBgMode;
  else 
    graphic_mode_ = kIllegalMode;
}

/**
 * @brief retrieves a character from screen Memory
 */
uint8_t Vic::get_screen_char(int column, int row)
{
  uint16_t addr = screen_mem_ + (row * kGCols) + column;
  return mem_->vic_read_byte(addr);
}

/**
 * @brief retrives color RAM for given screen coords
 */
uint8_t Vic::get_char_color(int column, int row)
{
  uint16_t addr = Memory::kAddrColorRAM + (row * kGCols) + column;
  return (mem_->read_byte_no_io(addr) & 0x0f);
}

/**
 * @brief retrieves pixel data from character memory
 */
uint8_t Vic::get_char_data(int chr, int line)
{
  uint16_t addr = char_mem_ + (chr * 8) + line;
  return mem_->vic_read_byte(addr);
}

/**
 * @brief retrieves pixel data from bitmap memory
 */
uint8_t Vic::get_bitmap_data(int column, int row, int line)
{
  uint16_t addr = bitmap_mem_ + (row * kGCols + column) * 8 + line;
  return mem_->vic_read_byte(addr);
}

/**
 * @brief get sprite pointer 
 *
 * n is sprite # (0-7)
 */
uint16_t Vic::get_sprite_ptr(int n)
{
  uint16_t ptraddr = screen_mem_ + kSpritePtrsOffset + n;
  return kSpriteSize * mem_->vic_read_byte(ptraddr);
}
 
// raster drawing  ///////////////////////////////////////////////////////////

void Vic::draw_char(int x, int y, uint8_t data, uint8_t color)
{
  for(int i=0 ; i < 8 ; i++)
  {
    int xoffs = x + 8 - i + horizontal_scroll();
    /* don't draw outside (due to horizontal scroll) */
    if(xoffs > kGFirstCol + kGResX)
      continue;
    /* draw pixel */
    if(ISSET_BIT(data,i))
    {
      io_->screen_update_pixel(
        xoffs, 
        y,
        color);
    }
  }
}

void Vic::draw_mcchar(int x, int y, uint8_t data, uint8_t color)
{
  for(int i=0 ; i < 4 ; i++)
  {
    /* color */
    uint8_t c;
    /* color source */
    uint8_t cs = ((data >> i*2) & 0x3);
    switch(cs)
    {
    case 0:
      c = bgcolor_[0]; 
      break;
    case 1:
      c = bgcolor_[1];
      break;
    case 2:
      c = bgcolor_[2];
      break;
    case 3:
      c = color;
      break;
    }
    int xoffs = x + 8 - i * 2 + horizontal_scroll();
    io_->screen_update_pixel(
      xoffs,
      y,
      c);
    io_->screen_update_pixel(
      xoffs + 1,
      y,
      c);
  }
}

void Vic::draw_raster_char_mode()
{
  int rstr = raster_counter();
  int y = rstr - kFirstVisibleLine;
  if((rstr >= kGFirstLine) && 
     (rstr < kGLastLine) && 
     !is_screen_off())
  {
    /* draw background */
    io_->screen_draw_rect(kGFirstCol,y,kGResX,bgcolor_[0]);
    /* draw characters */
    for(int column=0; column < kGCols ; column++)
    {
      /* check 38 cols mode */
      if(!ISSET_BIT(cr2_,3))
      {
        if (column == 0) continue; 
        if (column == kGCols -1 ) continue; 
      }
      int x = kGFirstCol + column * 8;
      int line = rstr - kGFirstLine;
      int row = line/8;
      int char_row = line % 8;
      /* retrieve screen character */
      uint8_t c = get_screen_char(column,row);
      /* retrieve character bitmap data */
      uint8_t data = get_char_data(c,char_row);
      /* retrieve color data */
      uint8_t color  = get_char_color(column,row);
      /* draw character */
      if(graphic_mode_ == kMCCharMode && ISSET_BIT(color,3))
        draw_mcchar(x,y,data,(color&0x7));
      else
        draw_char(x,y,data,color);
    }
  }
}

void Vic::draw_bitmap(int x, int y, uint8_t data, uint8_t color)
{
  uint8_t forec   = (color >> 4) & 0xf;
  uint8_t bgc =  color & 0xf;
  for(int i=0 ; i < 8 ; i++)
  {
    int xoffs = x + 8 - i + horizontal_scroll();
    /* don't draw outside (due to horizontal scroll) */
    if(xoffs > kGFirstCol + kGResX)
      continue;
    /* draw pixel */
    if(ISSET_BIT(data,i))
    {
      io_->screen_update_pixel(
        xoffs, 
        y,
        forec);
    }
    else
    {
      io_->screen_update_pixel(
        xoffs, 
        y,
        bgc);
    }
  }
}

void Vic::draw_mcbitmap(int x, int y, uint8_t data, uint8_t scolor, uint8_t rcolor)
{
  for(int i=0 ; i < 4 ; i++)
  {
    /* color */
    uint8_t c;
    /* color source */
    uint8_t cs = ((data >> i*2) & 0x3);
    switch(cs)
    {
    case 0:
      c = bgcolor_[0]; 
      break;
    case 1:
      c = (scolor >> 4) & 0xf;
      break;
    case 2:
      c = scolor & 0xf;
      break;
    case 3:
      c = rcolor;
      break;
    }
    int xoffs = x + 8 - i * 2 + horizontal_scroll();
    io_->screen_update_pixel(
      xoffs,
      y,
      c);
    io_->screen_update_pixel(
      xoffs + 1,
      y,
      c);
  }  
}
 
void Vic::draw_raster_bitmap_mode()
{
  int rstr = raster_counter();
  int y = rstr - kFirstVisibleLine;
  if((rstr >= kGFirstLine) && 
     (rstr < kGLastLine) && 
     !is_screen_off())
  {
    /* draw background */
    io_->screen_draw_rect(kGFirstCol,y,kGResX,bgcolor_[0]);
    /* draw bitmaps */
    for(int column=0; column < kGCols ; column++)
    {
      int x = kGFirstCol + column * 8;
      int line = rstr - kGFirstLine;
      int row = line/8;
      int bitmap_row = line % 8;
      /* retrieve bitmap data */
      uint8_t data = get_bitmap_data(column,row,bitmap_row);
      /* retrieve color data */
      uint8_t scolor = get_screen_char(column,row);
      uint8_t rcolor = get_char_color(column,row); 
      /* draw bitmap */
      if(graphic_mode_ == kBitmapMode)
        draw_bitmap(x,y,data,scolor);
      else
        draw_mcbitmap(x,y,data,scolor,rcolor);
    }
  }
}

void Vic::draw_mcsprite(int x, int y, int sprite, int row)
{
  uint16_t addr = get_sprite_ptr(sprite);
  for (int i=0; i < 3 ; i++)
  {
    uint8_t  data = mem_->vic_read_byte(addr + row * 3 + i);
    for (int j=0; j < 4; j++)
    {
      /* color */
      uint8_t c = 0;
      /* color source */
      uint8_t cs = ((data >> j*2) & 0x3);
      switch(cs)
      {
      /* transparent */
      case 0:
        break;
      case 1:
        c = sprite_shared_colors_[0];
        break;
      case 2:
        c = sprite_colors_[sprite];
        break;
      case 3:
        c = sprite_shared_colors_[1]; 
        break;
      }
      /* draw if not transparent */
      if(cs != 0)
      {
        io_->screen_update_pixel(
          x + i*8 + 8 - j * 2,          
          y,
          c);
        io_->screen_update_pixel(
          x + i*8 + 8 - j * 2 + 1,          
          y,
          c);
      }
    }
  }      
}

void Vic::draw_sprite(int x, int y, int sprite, int row)
{
  int swid = is_double_width_sprite(sprite) ? 2 : 1;
  uint16_t addr = get_sprite_ptr(sprite);
  for(int w=0; w < swid ; w++ )
  {
    for (int i=0; i < 3 ; i++)
    {
      uint8_t  data = mem_->vic_read_byte(addr + row * 3 + i);
      for (int j=0; j < 8; j++)
      {
        if(ISSET_BIT(data,j))
        {
          int new_x = (x+w + (i*8*swid) + (8*swid) - (j*swid)) ;
          int color = sprite_colors_[sprite];
          int side_border_offset = 0;
          int top_border_offset  = 0;
          int btm_border_offset  = 0;
          /* check 38 cols mode */
          if(!ISSET_BIT(cr2_,3))
            side_border_offset = 8;
          /* check 24 line mode */
          if(!ISSET_BIT(cr1_,3))
          {
            top_border_offset = 2;
            btm_border_offset = 4;
          }
          /* check bounds */
          if(new_x <= kGFirstCol+side_border_offset ||
             y < kGFirstCol + top_border_offset ||
             new_x > kGResX+kGFirstCol-side_border_offset ||
             y >= kGResY+kGFirstCol - btm_border_offset)
            color = border_color_;
          /* update pixel */
          io_->screen_update_pixel(
            new_x,
            y,
            color);
        }
      }
    }
  }
}

void Vic::draw_raster_sprites()
{
  if(sprite_enabled_ != 0)
  {
    int rstr = raster_counter();
    int y = rstr - kFirstVisibleLine;
    int sp_y = rstr - kSpritesFirstLine;
    /* loop over sprites reverse order */
    for(int n=7; n >= 0 ; n--)
    {
      int height = is_double_height_sprite(n) ? kSpriteHeight * 2 : kSpriteHeight;
      /* check if the sprite is visible */
      if(is_sprite_enabled(n) && 
         sp_y >= my_[n] && 
         sp_y < my_[n] + height)
      {
        int row = sp_y - my_[n];
        int x = kSpritesFirstCol + sprite_x(n);
        if(is_double_height_sprite(n))
        {
          row = (sp_y - my_[n])/2;
        }
        if(is_multicolor_sprite(n))
        {
          draw_mcsprite(x,y,n,row);
        }
        else
        {
          draw_sprite(x,y,n,row);
        }
      }
    }
  }
}

// helpers ///////////////////////////////////////////////////////////////////

void Vic::raster_counter(int v)
{
  raster_c_ = (uint8_t)(v&0xff);
  cr1_ &= 0x7f;
  cr1_ |= ((v >> 1)&0x80);
}

int Vic::raster_counter()
{
  return (raster_c_ | ((cr1_&0x80) << 1));
}

/**
 * @brief screen on/off
 *
 * Bit #4 of cr1 :
 *
 * 0 = Screen off, complete screen is covered by border. 
 * 1 = Screen on, normal screen contents are visible.
 */
bool Vic::is_screen_off()
{
  return ((cr1_&(1<<4)) == 0);
}

/**
 * @brief checks for bad line condition
 *
 * According to Christian Bauer's paper:
 * 
 * A Bad Line Condition is given at any arbitrary clock cycle, 
 * if at the negative edge of 0 at the beginning of the cycle 
 * RASTER >= $30 and RASTER <= $f7 and the lower three bits 
 * of RASTER are equal to YSCROLL and if the DEN bit was set 
 * during an arbitrary cycle of raster line $30.
 *
 */
bool Vic::is_bad_line()
{
  int rstr = raster_counter();
  return (rstr >= 0x30 &&
          rstr <= 0xf7 &&
          (rstr & 0x7) == (vertical_scroll() & 0x7));
}

/**
 * @brief check if raster irq is enabled 
 */
bool Vic::raster_irq_enabled()
{
  return ISSET_BIT(irq_enabled_,0);
}

/** 
 * @brief get vertical raster scroll
 */
uint8_t Vic::vertical_scroll()
{
  return(cr1_&0x7);
} 
 
/** 
 * @brief get horizontal raster scroll
 */
uint8_t Vic::horizontal_scroll()
{
  return(cr2_&0x7);
}

/**
 * @brief check if sprite #n is enabled
 */
bool Vic::is_sprite_enabled(int n)
{
  return ISSET_BIT(sprite_enabled_,n);
}

/**
 * @brief check if sprite #n is in background
 */
bool Vic::is_background_sprite(int n)
{
  return ISSET_BIT(sprite_priority_,n);
}
 
/**
 * @brief check if sprite #n is double width
 */
bool Vic::is_double_width_sprite(int n)
{
  return ISSET_BIT(sprite_double_width_,n);
}

/**
 * @brief check if sprite #n is double height
 */
bool Vic::is_double_height_sprite(int n)
{
  return ISSET_BIT(sprite_double_height_,n);
}

/**
 * @brief check if sprite #n is multicolor
 */
bool Vic::is_multicolor_sprite(int n)
{
  return ISSET_BIT(sprite_multicolor_,n);
}

/**
 * @brief returns sprite x coordinate 
 */
int Vic::sprite_x(int n)
{
  int x = mx_[n];
  if(ISSET_BIT(msbx_,n))
    x |= 1 << 8;
  return x;
}
