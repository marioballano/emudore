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

#include <fstream>
#include "memory.h"
#include "util.h"
#include "vic.h"
#include "cia1.h"
#include "cia2.h"

Memory::Memory()
{
  /**
   * 64 kB memory buffers, zeroed.
   *
   * We use two buffers to handle special circumstances, for instance,
   * any write to a ROM-mapped location will in turn store data on the 
   * hidden RAM, this trickery is used in certain graphic modes.
   */
  mem_ram_ = new uint8_t[kMemSize]();
  mem_rom_ = new uint8_t[kMemSize]();
  /* configure memory layout */
  setup_memory_banks(kLORAM|kHIRAM|kCHAREN);
  /* configure data directional bits */
  write_byte_no_io(kAddrDataDirection,0x2f);
}

Memory::~Memory()
{
  delete [] mem_ram_;
  delete [] mem_rom_;
}

/**
 * @brief configure memory banks
 *
 * There are five latch bits that control the configuration allowing
 * for a total of 32 different memory layouts, for now we only take
 * in count three bits : HIRAM/LORAM/CHAREN
 */
void Memory::setup_memory_banks(uint8_t v)
{
  /* get config bits */
  bool hiram  = ((v&kHIRAM) != 0);
  bool loram  = ((v&kLORAM) != 0);
  bool charen = ((v&kCHAREN)!= 0);
  /* init everything to ram */
  for(size_t i=0 ; i < sizeof(banks_) ; i++)
    banks_[i] = kRAM;
  /* load ROMs */
  load_rom("basic.901226-01.bin",kBaseAddrBasic);
  load_rom("characters.901225-01.bin",kBaseAddrChars);
  load_rom("kernal.901227-03.bin",kBaseAddrKernal);
  /* kernal */
  if (hiram) 
    banks_[kBankKernal] = kROM;
  /* basic */
  if (loram && hiram) 
    banks_[kBankBasic] = kROM;
  /* charen */
  if (charen && (loram || hiram))
    banks_[kBankCharen] = kIO;
  else if (charen && !loram && !hiram)
    banks_[kBankCharen] = kRAM;
  else 
    banks_[kBankCharen] = kROM;
  /* write the config to the zero page */
  write_byte_no_io(kAddrMemoryLayout, v);
}

/**
 * @brief writes a byte to RAM without performing I/O
 */
void Memory::write_byte_no_io(uint16_t addr, uint8_t v)
{
  mem_ram_[addr] = v;
}

/**
 * @brief writes a byte to RAM handling I/O
 */
void Memory::write_byte(uint16_t addr, uint8_t v)
{
  uint16_t page = addr&0xff00;
  /* ZP */
  if (page == kAddrZeroPage)
  {
    /* bank switching */
    if (addr == kAddrMemoryLayout)
      setup_memory_banks(v);
    else
      mem_ram_[addr] = v;
  }
  /* VIC-II DMA or Character ROM */
  else if (page >= kAddrVicFirstPage && page <= kAddrVicLastPage)
  {
    if(banks_[kBankCharen] == kIO)
      vic_->write_register(addr&0x7f,v);
    else
      mem_ram_[addr] = v;
  }
  /* CIA1 */
  else if (page == kAddrCIA1Page)
  {
    if(banks_[kBankCharen] == kIO)
      cia1_->write_register(addr&0x0f,v);
    else
      mem_ram_[addr] = v;
  }
  else if (page == kAddrCIA2Page)
  {
    if(banks_[kBankCharen] == kIO)
      cia2_->write_register(addr&0x0f,v);
    else
      mem_ram_[addr] = v;
  }
  /* default */
  else
  {
    mem_ram_[addr] = v;
  }
}

/**
 * @brief reads a byte from RAM or ROM (depending on bank config)
 */
uint8_t Memory::read_byte(uint16_t addr)
{
  uint8_t  retval = 0;
  uint16_t page   = addr&0xff00;
  /* VIC-II DMA or Character ROM */
  if (page >= kAddrVicFirstPage && page <= kAddrVicLastPage)
  {
    if(banks_[kBankCharen] == kIO)
      retval = vic_->read_register(addr&0x7f);
    else if(banks_[kBankCharen] == kROM)
      retval = mem_rom_[addr];
    else
      retval = mem_ram_[addr];
  }
  /* CIA1 */
  else if (page == kAddrCIA1Page)
  {
    if(banks_[kBankCharen] == kIO)
      retval = cia1_->read_register(addr&0x0f);
    else
      retval = mem_ram_[addr];
  }
  /* CIA2 */
  else if (page == kAddrCIA2Page)
  {
    if(banks_[kBankCharen] == kIO)
      retval = cia2_->read_register(addr&0x0f);
    else
      retval = mem_ram_[addr];
  }       
  /* BASIC or RAM */
  else if (page >= kAddrBasicFirstPage && page <= kAddrBasicLastPage)
  {
    if (banks_[kBankBasic] == kROM)
      retval = mem_rom_[addr];
    else
      retval = mem_ram_[addr];
  }
  /* KERNAL */
  else if (page >= kAddrKernalFirstPage && page <= kAddrKernalLastPage)
  {
    if (banks_[kBankKernal] == kROM)
      retval = mem_rom_[addr];
    else 
      retval = mem_ram_[addr];
  }
  /* default */
  else
  {
    retval = mem_ram_[addr];
  }
  return retval;
}

/**
 * @brief writes a byte without performing I/O (always to RAM)
 */

uint8_t Memory::read_byte_no_io(uint16_t addr)
{
  return mem_ram_[addr];
}

/**
 * @brief reads a word performing I/O
 */
uint16_t Memory::read_word(uint16_t addr)
{
  return read_byte(addr) | (read_byte(addr+1) << 8);
}

/**
 * @brief reads a word withouth performing I/O
 */
uint16_t Memory::read_word_no_io(uint16_t addr)
{
  return read_byte_no_io(addr) | (read_byte_no_io(addr+1) << 8);
}

/** 
 * @brief writes a word performing I/O
 */
void Memory::write_word(uint16_t addr, uint16_t v)
{
  write_byte(addr, (uint8_t)(v));
  write_byte(addr+1, (uint8_t)(v>>8));
}

/** 
 * @brief writes a word without performing I/O
 */
void Memory::write_word_no_io(uint16_t addr, uint16_t v)
{
  write_byte_no_io(addr, (uint8_t)(v));
  write_byte_no_io(addr+1, (uint8_t)(v>>8));
}

/**
 * @brief read byte (from VIC's perspective)
 *
 * The VIC has only 14 address lines so it can only access 
 * 16kB of memory at once, the two missing address bits are 
 * provided by CIA2.
 *
 * The VIC always reads from RAM ignoring the memory configuration,
 * there's one exception: the character generator ROM. Unless the 
 * Ultimax mode is selected, VIC sees the character generator ROM 
 * in the memory areas:
 *
 *  1000-1FFF
 *  9000-9FFF
 */
uint8_t Memory::vic_read_byte(uint16_t addr)
{
  uint8_t v;
  uint16_t vic_addr = cia2_->vic_base_address() + (addr & 0x3fff);
  if((vic_addr >= 0x1000 && vic_addr <  0x2000) ||
     (vic_addr >= 0x9000 && vic_addr <  0xa000))
    v = mem_rom_[kBaseAddrChars + (vic_addr & 0xfff)];
  else
    v = read_byte_no_io(vic_addr);
  return v;
}

/**
 * @brief loads a external binary into ROM
 */
void Memory::load_rom(const std::string &f, uint16_t baseaddr)
{
  std::string path = "./assets/roms/" + f;
  std::ifstream is(path, std::ios::in | std::ios::binary);
  if(is)
  {
    is.seekg (0, is.end);
    std::streamoff length = is.tellg();
    is.seekg (0, is.beg);
    is.read ((char *) &mem_rom_[baseaddr],length);
  }
}

/**
 * @brief loads a external binary into RAM
 */
void Memory::load_ram(const std::string &f, uint16_t baseaddr)
{
  std::string path = "./assets/" + f;
  std::ifstream is(path, std::ios::in | std::ios::binary);
  if(is)
  {
    is.seekg (0, is.end);
    std::streamoff length = is.tellg();
    is.seekg (0, is.beg);
    is.read ((char *) &mem_ram_[baseaddr],length);
  }
}

// debug ////////////////////////////////////////////////////////////////////

/**
 * @brief dumps memory as seen by the CPU to stdout
 */
void Memory::dump()
{
  for(unsigned int p=0 ; p < kMemSize ; p++)
  {
    std::cout << read_byte(p);
  }
}
