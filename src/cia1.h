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
#ifndef EMUDORE_CIA1_H
#define EMUDORE_CIA1_H

#include "io.h"
#include "cpu.h"

/**
 * @brief MOS 6526 Complex Interface Adapter #1
 *
 * - Memory area : $DC00-$DCFF
 * - Tasks       : Keyboard, Joystick, Paddles, Datasette, IRQ control
 */
class Cia1
{
  private:
    Cpu *cpu_;
    IO *io_;
    int16_t timer_a_latch_;
    int16_t timer_b_latch_;
    int16_t timer_a_counter_;
    int16_t timer_b_counter_;
    bool timer_a_enabled_;
    bool timer_b_enabled_;
    bool timer_a_irq_enabled_;
    bool timer_b_irq_enabled_;
    bool timer_a_irq_triggered_;
    bool timer_b_irq_triggered_;
    uint8_t timer_a_run_mode_;
    uint8_t timer_b_run_mode_;
    uint8_t timer_a_input_mode_;
    uint8_t timer_b_input_mode_;
    unsigned int prev_cpu_cycles_;
    uint8_t pra_, prb_;
  public:
    Cia1();
    void cpu(Cpu *v){ cpu_ = v;};
    void io(IO *v){ io_ = v;};
    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);
    void reset_timer_a();
    void reset_timer_b();
    bool emulate();
    /* constants */
    enum kInputMode
    {
      kModeProcessor,
      kModeCNT,
      kModeTimerA,
      kModeTimerACNT
    };
    enum kRunMode
    {
      kModeRestart,
      kModeOneTime
    };
};

#endif
