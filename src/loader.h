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

#ifndef EMUDORE_LOADER_H
#define EMUDORE_LOADER_H

#include <fstream>
#include "c64.h"

/**
 * @brief Program loader
 */
class Loader
{
  private:
    bool booted_up_;
    C64 *c64_;
    IO *io_;
    Cpu *cpu_;
    Memory *mem_;
    std::ifstream is_;
    enum kFormat
    {
      kNone,
      kBasic,
      kPRG
    };
    kFormat format_;
    void load_basic();
    void load_prg();
    uint16_t read_short_le();
  public:
    Loader(C64 *c64);
    void bas(const std::string &f);
    void prg(const std::string &f);
    bool emulate();
    /* constants */
    static const uint16_t kBasicPrgStart = 0x0801;
    static const uint16_t kBasicTxtTab   = 0x002b;
    static const uint16_t kBasicVarTab   = 0x002d; 
    static const uint16_t kBasicAryTab   = 0x002f;
    static const uint16_t kBasicStrEnd   = 0x0031;
};

#endif
