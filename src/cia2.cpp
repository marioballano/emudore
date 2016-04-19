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

#include "cia2.h"

// ctor  /////////////////////////////////////////////////////////////////////

Cia2::Cia2()
{
  timer_a_latch_ = timer_b_latch_ = timer_a_counter_ = timer_b_counter_ = 0;
  timer_a_enabled_ = timer_b_enabled_ = timer_a_irq_enabled_ = timer_b_irq_enabled_ = false;
  timer_a_irq_triggered_ = timer_b_irq_triggered_ = false;
  timer_a_input_mode_ = timer_b_input_mode_ = kModeProcessor;
  timer_a_run_mode_ = timer_b_run_mode_ = kModeRestart;
  pra_ = prb_ = 0xff;
  prev_cpu_cycles_ = 0;
}

// DMA register access  //////////////////////////////////////////////////////

void Cia2::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
  /* data port a (PRA) */
  case 0x0:
    pra_ = v;
    break;
  /* data port b (PRB) */
  case 0x1:
    prb_ = v;
    break;
  /* data direction port a (DDRA) */
  case 0x2:
    break;
  /* data direction port b (DDRB) */
  case 0x3:
    break;
  /* timer a low byte */
  case 0x4:
    timer_a_latch_ &= 0xff00;
    timer_a_latch_ |= v;
    break;
  /* timer a high byte */
  case 0x5:
    timer_a_latch_ &= 0x00ff;
    timer_a_latch_ |= v << 8;
    break;
  /* timer b low byte */
  case 0x6:
    timer_b_latch_ &= 0xff00;
    timer_b_latch_ |= v;
    break;
  /* timer b high byte */
  case 0x7: 
    timer_b_latch_ &= 0x00ff;
    timer_b_latch_ |= v << 8;
    break;
  /* RTC 1/10s  */
  case 0x8:
    break;
  /* RTC seconds */
  case 0x9:
    break;
  /* RTC minutes */
  case 0xa:
    break;
  /* RTC hours */
  case 0xb:
    break;
  /* shift serial */
  case 0xc:
    break;
  /* interrupt control and status */
  case 0xd:
    /**
     * if bit 7 is set, enable selected mask of 
     * interrupts, else disable them
     */
    if(ISSET_BIT(v,0)) timer_a_irq_enabled_ = ISSET_BIT(v,7);
    if(ISSET_BIT(v,1)) timer_b_irq_enabled_ = ISSET_BIT(v,7);
    break;
  /* control timer a */
  case 0xe:
    timer_a_enabled_ = ((v&(1<<0))!=0);
    timer_a_input_mode_ = (v&(1<<5)) >> 5;
    /* load latch requested */
    if((v&(1<<4))!=0)
      timer_a_counter_ = timer_a_latch_;
    break;
  /* control timer b */
  case 0xf:
    timer_b_enabled_ = ((v&0x1)!=0);
    timer_b_input_mode_ = (v&(1<<5)) | (v&(1<<6)) >> 5;
    /* load latch requested */
    if((v&(1<<4))!=0)
      timer_b_counter_ = timer_b_latch_;
    break;
  }
}

uint8_t Cia2::read_register(uint8_t r)
{
  uint8_t retval = 0;

  switch(r)
  {
  /* data port a (PRA) */
  case 0x0:
    retval = pra_;
    break;
  /* data port b (PRB) */
  case 0x1:
    retval = prb_;
    break;
  /* data direction port a (DDRA) */
  case 0x2:
    break;
  /* data direction port b (DDRB) */
  case 0x3:
    break;
  /* timer a low byte */
  case 0x4:
    retval = (uint8_t)(timer_a_counter_ & 0x00ff);
    break;
  /* timer a high byte */
  case 0x5:
    retval = (uint8_t)((timer_a_counter_ & 0xff00) >> 8);
    break;
  /* timer b low byte */
  case 0x6:
    retval = (uint8_t)(timer_b_counter_ & 0x00ff);
    break;
  /* timer b high byte */
  case 0x7: 
    retval = (uint8_t)((timer_b_counter_ & 0xff00) >> 8);
    break;
  /* RTC 1/10s  */
  case 0x8:
    break;
  /* RTC seconds */
  case 0x9:
    break;
  /* RTC minutes */
  case 0xa:
    break;
  /* RTC hours */
  case 0xb:
    break;
  /* shift serial */
  case 0xc:
    break;
  /* interrupt control and status */
  case 0xd:
    if(timer_a_irq_triggered_ ||
       timer_b_irq_triggered_)
    {
      retval |= (1 << 7); // IRQ occured
      if(timer_a_irq_triggered_) retval |= (1 << 0);
      if(timer_b_irq_triggered_) retval |= (1 << 1);
    }
    break;
  /* control timer a */
  case 0xe:
    break;
  /* control timer b */
  case 0xf:
    break;
  }
  return retval;
}

// timer reset ///////////////////////////////////////////////////////////////

void Cia2::reset_timer_a()
{
  switch(timer_a_run_mode_)
  {
  case kModeRestart:
    timer_a_counter_ = timer_a_latch_;
    break;
  case kModeOneTime:
    timer_a_enabled_ = false;
    break;
  }
}

void Cia2::reset_timer_b()
{
  switch(timer_b_run_mode_)
  {
  case kModeRestart:
    timer_b_counter_ = timer_b_latch_;
    break;
  case kModeOneTime:
    timer_b_enabled_ = false;
    break;
  }      
}

// VIC banking ///////////////////////////////////////////////////////////////

/**
 * @brief retrieves vic base address
 *
 * PRA bits (0..1)
 *
 *  %00, 0: Bank 3: $C000-$FFFF, 49152-65535
 *  %01, 1: Bank 2: $8000-$BFFF, 32768-49151
 *  %10, 2: Bank 1: $4000-$7FFF, 16384-32767
 *  %11, 3: Bank 0: $0000-$3FFF, 0-16383 (standard)
 */
uint16_t Cia2::vic_base_address()
{
  return ((~pra_&0x3) << 14);
}

// emulation  ////////////////////////////////////////////////////////////////

bool Cia2::emulate()
{
  /* timer a */
  if(timer_a_enabled_)
  {
    switch(timer_a_input_mode_)
    {
    case kModeProcessor:
      timer_a_counter_ -= cpu_->cycles() - prev_cpu_cycles_;
      if (timer_a_counter_ <= 0)
      {
        if(timer_a_irq_enabled_) 
        {
          timer_a_irq_triggered_ = true;
          cpu_->nmi();
        }
        reset_timer_a();
      }
      break;
    case kModeCNT:
      break;
    }
  }
  /* timer b */
  if(timer_b_enabled_)
  {
    switch(timer_b_input_mode_)
    {
    case kModeProcessor:
      timer_b_counter_ -= cpu_->cycles() - prev_cpu_cycles_;
      if (timer_b_counter_ <= 0)
      {
        if(timer_b_irq_enabled_)
        {
          timer_b_irq_triggered_ = true;
          cpu_->nmi();
        }
        reset_timer_b();
      }   
      break;
    case kModeCNT:
      break;
    case kModeTimerA:
      break;
    case kModeTimerACNT:
      break;
    }
  }
  prev_cpu_cycles_ = cpu_->cycles();
  return true;
}
