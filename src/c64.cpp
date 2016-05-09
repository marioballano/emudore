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
#include "c64.h"
#include "util.h"

C64::C64()
{
  /* create chips */
  cpu_  = new Cpu();
  mem_  = new Memory();
  cia1_ = new Cia1();
  cia2_ = new Cia2();
  vic_  = new Vic();
  sid_  = new Sid();
  io_   = new IO();
  /* init cpu */
  cpu_->memory(mem_);
  cpu_->reset();
  /* init vic-ii */
  vic_->memory(mem_);
  vic_->cpu(cpu_);
  vic_->io(io_);
  /* init cia1 */
  cia1_->cpu(cpu_);
  cia1_->io(io_);
  /* init cia2 */
  cia2_->cpu(cpu_);
  /* init io */
  io_->cpu(cpu_);
  /* DMA */
  mem_->vic(vic_);
  mem_->cia1(cia1_);
  mem_->cia2(cia2_);
 /* r2 support */
#ifdef DEBUGGER_SUPPORT
  debugger_ = new Debugger();
  debugger_->memory(mem_);
  debugger_->cpu(cpu_);
#endif   
}

C64::~C64()
{
  delete cpu_;
  delete mem_;
  delete cia1_;
  delete cia2_;
  delete vic_;
  delete sid_;
  delete io_;
#ifdef DEBUGGER_SUPPORT
  delete debugger_;
#endif   
}

void C64::start()
{
  /* main emulator loop */
  while(true)
  {
#ifdef DEBUGGER_SUPPORT
    if(!debugger_->emulate())
      break;
#endif
    /* CIA1 */
    if(!cia1_->emulate())
      break;
    /* CIA2 */
    if(!cia2_->emulate())
      break;
    /* CPU */
    if(!cpu_->emulate())
      break;
    /* VIC-II */
    if(!vic_->emulate())
      break;
    /* IO */
    if(!io_->emulate())
      break;
    /* callback */
    if(callback_ && !callback_())
      break;
  }
}

/**
 * @brief emscripten's main loop
 */
void C64::emscripten_loop()
{
  unsigned int frame = vic_->frames();
  while(frame == vic_->frames())
  {
    /* CIA1 */
    cia1_->emulate();
    /* CIA2 */
    cia2_->emulate();
    /* CPU */
    cpu_->emulate();
    /* VIC-II */
    vic_->emulate();
    /* IO */
    io_->emulate();
    /* callback */
    if(callback_) callback_();
  }
}
 
/**
 * @brief runs Klaus Dormann's 6502 test suite 
 *
 * https://github.com/Klaus2m5/6502_65C02_functional_tests
 */
void C64::test_cpu()
{
  uint16_t pc=0;
  /* unmap C64 ROMs */
  mem_->write_byte(Memory::kAddrMemoryLayout, 0);
  /* load tests into RAM */
  mem_->load_ram("tests/6502_functional_test.bin",0x400);
  cpu_->pc(0x400);
  while(true)
  {
    if(pc == cpu_->pc())
    {
      D("infinite loop at %x\n",pc);
      break;
    }
    else if(cpu_->pc() == 0x3463)
    {
      D("test passed!\n");
      break;
    }
    pc = cpu_->pc();
    if(!cpu_->emulate())
      break;
  }
}
