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

#include "loader.h"

Loader::Loader(C64 *c64)
{
  c64_ = c64;
  io_  = c64_->io();
  cpu_ = c64_->cpu();
  mem_ = c64_->memory();
  booted_up_ = false;
  format_ = kNone;
}
// common ///////////////////////////////////////////////////////////////////

uint16_t Loader::read_short_le()
{
  char b;
  uint16_t v = 0;
  is_.get(b);
  v |= (b);
  is_.get(b);
  v |= b << 8;
  return v;
}

// BASIC listings ///////////////////////////////////////////////////////////

void Loader::bas(const std::string &f)
{
  format_ = kBasic; 
  is_.open(f,std::ios::in);
}

void Loader::load_basic()
{
  char c;
  if(is_.is_open())
  {
    while(is_.good())
    {
      is_.get(c);
      io_->IO::type_character(c);
    }
  }
}

// PRG //////////////////////////////////////////////////////////////////////

void Loader::prg(const std::string &f)
{
  format_ = kPRG;
  is_.open(f,std::ios::in|std::ios::binary);
}

void Loader::load_prg()
{
  char b;
  uint16_t pbuf, addr;
  pbuf = addr = read_short_le();
  if(is_.is_open())
  {
    while(is_.good())
    {
      is_.get(b);
      mem_->write_byte_no_io(pbuf++,b);
    }
    /* basic-tokenized prg */
    if(addr == kBasicPrgStart)
    {
      /* make BASIC happy */
      mem_->write_word_no_io(kBasicTxtTab,kBasicPrgStart);
      mem_->write_word_no_io(kBasicVarTab,pbuf);
      mem_->write_word_no_io(kBasicAryTab,pbuf);
      mem_->write_word_no_io(kBasicStrEnd,pbuf);
      /* exec RUN */ 
      for(char &c: std::string("RUN\n"))
        io_->IO::type_character(c);
    }
    /* ML */
    else cpu_->pc(addr);
  }
}
 
// emulate //////////////////////////////////////////////////////////////////

bool Loader::emulate()
{
  if(booted_up_)
  {
    switch(format_)
    {
    case kBasic: 
      load_basic();
      break;
    case kPRG:
      load_prg();
      break;
    default: 
      break;
    }
    return false;
  }
  else
  {
    /* at this point BASIC is ready */
    if(cpu_->pc() == 0xa65c)
      booted_up_ = true;
  }
  return true;
}
