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

#include "cia1.h"

// ctor  /////////////////////////////////////////////////////////////////////

Cia1::Cia1()
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

void Cia1::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
  /* data port a (PRA), keyboard matrix cols and joystick #2 */
  case 0x0:
    pra_ = v;
    break;
  /* data port b (PRB), keyboard matrix rows and joystick #1 */
  case 0x1:
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

uint8_t Cia1::read_register(uint8_t r)
{
  uint8_t retval = 0;

  switch(r)
  {
  /* data port a (PRA), keyboard matrix cols and joystick #2 */
  case 0x0:
    break;
  /* data port b (PRB), keyboard matrix rows and joystick #1 */
  case 0x1:
    if (pra_ == 0xff) retval = 0xff;
    else if(pra_)
    {
      int col = 0;
      uint8_t v = ~pra_;
      while (v >>= 1)col++;
      retval = io_->keyboard_matrix_row(col);
    }
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

void Cia1::reset_timer_a()
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

void Cia1::reset_timer_b()
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

// emulation  ////////////////////////////////////////////////////////////////

bool Cia1::emulate()
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
          cpu_->irq();
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
          cpu_->irq();
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
