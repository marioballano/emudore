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
 
#ifndef EMUDORE_MEMORY_H
#define EMUDORE_MEMORY_H 

#include <iostream>
#include <cstdint>

/* forward declarations */

class Vic;
class Cia1;
class Cia2;
class Sid;

/**
 * @brief DRAM
 *
 * - @c $0000-$00FF  Page 0        Zeropage addressing
 * - @c $0100-$01FF  Page 1        Enhanced Zeropage contains the stack
 * - @c $0200-$02FF  Page 2        Operating System and BASIC pointers
 * - @c $0300-$03FF  Page 3        Operating System and BASIC pointers
 * - @c $0400-$07FF  Page 4-7      Screen Memory
 * - @c $0800-$9FFF  Page 8-159    Free BASIC program storage area (38911 bytes)
 * - @c $A000-$BFFF  Page 160-191  Free machine language program storage area (when switched-out with ROM)
 * - @c $C000-$CFFF  Page 192-207  Free machine language program storage area
 * - @c $D000-$D3FF  Page 208-211  
 * - @c $D400-$D4FF  Page 212-215  
 * - @c $D800-$DBFF  Page 216-219  
 * - @c $DC00-$DCFF  Page 220  
 * - @c $DD00-$DDFF  Page 221  
 * - @c $DE00-$DFFF  Page 222-223  Reserved for interface extensions
 * - @c $E000-$FFFF  Page 224-255  Free machine language program storage area (when switched-out with ROM)
 */
class Memory
{
  private:
    uint8_t *mem_ram_;
    uint8_t *mem_rom_;
    uint8_t banks_[7];
    Vic *vic_;
    Cia1 *cia1_;
    Cia2 *cia2_;
    Sid *sid_;
  public:
    Memory();
    ~Memory();
    void vic(Vic *v){vic_ = v;};
    void cia1(Cia1 *v){cia1_ = v;};
    void cia2(Cia2 *v){cia2_ = v;};
    /* bank switching */
    enum kBankCfg
    {
      kROM,
      kRAM,
      kIO
    };
    enum Banks
    {
      kBankBasic =  3,
      kBankCharen = 5,
      kBankKernal =  6,
    };
    void setup_memory_banks(uint8_t v);
    /* read/write memory */
    uint8_t read_byte(uint16_t addr);
    uint8_t read_byte_no_io(uint16_t addr);
    void write_byte(uint16_t addr, uint8_t v);
    void write_byte_no_io(uint16_t addr, uint8_t v);
    uint16_t read_word(uint16_t addr);
    uint16_t read_word_no_io(uint16_t);
    void write_word(uint16_t addr, uint16_t v);
    void write_word_no_io(uint16_t addr, uint16_t v);
    /* vic memory access */
    uint8_t vic_read_byte(uint16_t addr);
    uint8_t read_byte_rom(uint16_t addr);
    /* load external binaries */
    void load_rom(const std::string &f, uint16_t baseaddr);
    void load_ram(const std::string &f, uint16_t baseaddr);
    /* debug */
    void dump();
    void print_screen_text();
    /* constants */
    static const size_t kMemSize = 0x10000;
    /* memory addresses  */
    static const uint16_t kBaseAddrBasic  = 0xa000;
    static const uint16_t kBaseAddrKernal = 0xe000;
    static const uint16_t kBaseAddrStack  = 0x0100;
    static const uint16_t kBaseAddrScreen = 0x0400;
    static const uint16_t kBaseAddrChars  = 0xd000;
    static const uint16_t kBaseAddrBitmap = 0x0000;
    static const uint16_t kBaseAddrColorRAM = 0xd800;
    static const uint16_t kAddrResetVector = 0xfffc;
    static const uint16_t kAddrIRQVector = 0xfffe;
    static const uint16_t kAddrNMIVector = 0xfffa;
    static const uint16_t kAddrDataDirection = 0x0000;
    static const uint16_t kAddrMemoryLayout  = 0x0001;
    static const uint16_t kAddrColorRAM = 0xd800;
    /* memory layout */
    static const uint16_t kAddrZeroPage     = 0x0000;
    static const uint16_t kAddrVicFirstPage = 0xd000;
    static const uint16_t kAddrVicLastPage  = 0xd300;
    static const uint16_t kAddrCIA1Page = 0xdc00;
    static const uint16_t kAddrCIA2Page = 0xdd00;
    static const uint16_t kAddrBasicFirstPage = 0xa000; 
    static const uint16_t kAddrBasicLastPage  = 0xbf00;
    static const uint16_t kAddrKernalFirstPage = 0xe000;
    static const uint16_t kAddrKernalLastPage = 0xff00;
    /* bank switching */
    static const uint8_t kLORAM  = 1 << 0;
    static const uint8_t kHIRAM  = 1 << 1;
    static const uint8_t kCHAREN = 1 << 2;
};

#endif
