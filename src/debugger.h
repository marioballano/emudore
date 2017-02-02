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
#ifndef EMUDORE_DEBUGGER_H
#define EMUDORE_DEBUGGER_H

/**
 * @brief Debugger
 *
 * Sponsored by radare2
 * 
 * https://github.com/radare/radare2
 */

#include "cpu.h"
#include "memory.h"

#include <vector>
#include <string>

class Debugger
{
  private:
    int ss_;
    uint16_t offset_;
    Cpu *cpu_;
    Memory *mem_;
    uint16_t emu_seek(uint16_t offset, int whence);
    uint8_t * emu_read_mem(size_t sz);
    std::string emu_handle_cmd(const std::string &s);
    void emu_write_mem(uint8_t *mem, size_t sz);
    std::vector<std::string> split_cmd(const std::string &s);
    std::string regs_cmd();
  public:
    Debugger();
    ~Debugger();
    void cpu(Cpu *v){cpu_=v;};
    void memory(Memory *v){mem_=v;};
    bool emulate();
    static const int kDbgPort = 9999;
    static const int kMaxCmdLen = 256;
    enum kDbgCommands{
      RAP_RMT_OPEN = 0x01,
      RAP_RMT_READ,
      RAP_RMT_WRITE,
      RAP_RMT_SEEK,
      RAP_RMT_CLOSE,
      RAP_RMT_CMD,
      RAP_RMT_REPLY = 0x80,
      RAP_RMT_MAX = 4096
    };
};

#endif
