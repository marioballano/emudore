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

#ifndef EMUDORE_CPU_H
#define EMUDORE_CPU_H

#include <cstdint>
#include "memory.h"

/**
 * @brief MOS 6510 microprocessor
 */
class Cpu
{
  private:
    /* registers */
    uint16_t pc_;
    uint8_t sp_, a_, x_, y_;
    /* flags (p/status reg) */
    bool cf_,zf_,idf_,dmf_,bcf_,of_,nf_;
    /* memory and clock */
    Memory *mem_;
    unsigned int cycles_;
    /* helpers */
    inline uint8_t load_byte(uint16_t addr);
    inline void push(uint8_t);
    inline uint8_t pop();
    inline uint8_t fetch_op();
    inline uint16_t fetch_opw();
    inline uint16_t addr_zero();
    inline uint16_t addr_zerox();
    inline uint16_t addr_zeroy();
    inline uint16_t addr_abs();
    inline uint16_t addr_absy();
    inline uint16_t addr_absx();
    inline uint16_t addr_indx();
    inline uint16_t addr_indy();
    inline uint8_t rol(uint8_t v);
    inline uint8_t ror(uint8_t v);
    inline uint8_t lsr(uint8_t v);
    inline uint8_t asl(uint8_t v);
    inline void tick(uint8_t v){cycles_+=v;};
    inline uint8_t flags();
    inline void flags(uint8_t v);
    /* instructions : data handling and memory operations */
    inline void sta(uint16_t addr, uint8_t cycles);
    inline void stx(uint16_t addr, uint8_t cycles);
    inline void sty(uint16_t addr, uint8_t cycles);
    inline void lda(uint8_t v, uint8_t cycles);
    inline void ldx(uint8_t v, uint8_t cycles);
    inline void ldy(uint8_t v, uint8_t cycles);
    inline void txs();
    inline void tsx();
    inline void tax();
    inline void txa();
    inline void tay();
    inline void tya();
    inline void pha();
    inline void pla();
    /* instructions: logic operations */
    inline void ora(uint8_t v, uint8_t cycles);
    inline void _and(uint8_t v, uint8_t cycles);
    inline void bit(uint16_t addr, uint8_t cycles);
    inline void rol_a();
    inline void rol_mem(uint16_t addr, uint8_t cycles);
    inline void ror_a();
    inline void ror_mem(uint16_t addr, uint8_t cycles);
    inline void asl_a();
    inline void asl_mem(uint16_t addr, uint8_t cycles);
    inline void lsr_a();
    inline void lsr_mem(uint16_t addr, uint8_t cycles);
    inline void eor(uint8_t v, uint8_t cycles);
    /* instructions: arithmetic operations */
    inline void inc(uint16_t addr, uint8_t cycles);
    inline void dec(uint16_t addr, uint8_t cycles);
    inline void inx();
    inline void iny();
    inline void dex();
    inline void dey();
    inline void adc(uint8_t v, uint8_t cycles);
    inline void sbc(uint8_t v, uint8_t cycles);
    /* instructions: flag access */
    inline void sei();
    inline void cli();
    inline void sec();
    inline void clc();
    inline void sed();
    inline void cld();
    inline void clv();
    inline void php();
    inline void plp();
    /* instructions: control flow */
    inline void cmp(uint8_t v, uint8_t cycles);
    inline void cpx(uint8_t v, uint8_t cycles);
    inline void cpy(uint8_t v, uint8_t cycles);
    inline void rts();
    inline void jsr();
    inline void bne();
    inline void beq();
    inline void bcs();
    inline void bcc();
    inline void bpl();
    inline void bmi();
    inline void bvc();
    inline void bvs();
    inline void jmp();
    inline void jmp_ind();
    /* instructions: misc */
    inline void nop();
    inline void brk();
    inline void rti();
  public:
    /* cpu state */
    void reset();
    bool emulate();
    /* memory */
    void memory(Memory *v){mem_ = v;};
    Memory* memory(){return mem_;};
    /* register access */
    inline uint16_t pc() {return pc_;};
    inline void pc(uint16_t v) {pc_=v;};
    inline uint8_t sp() {return sp_;};
    inline void sp(uint8_t v) {sp_=v;};
    inline uint8_t a() {return a_;};
    inline void a(uint8_t v) {a_=v;};
    inline uint8_t x() {return x_;};
    inline void x(uint8_t v) {x_=v;};
    inline uint8_t y() {return y_;};
    inline void y(uint8_t v) {y_=v;};
    /* flags */
    inline bool cf() {return cf_;};
    inline void cf(bool v) { cf_=v;};
    inline bool zf() {return zf_;};
    inline void zf(bool v) {zf_=v;};
    inline bool idf() {return idf_;};
    inline void idf(bool v) { idf_=v;};
    inline bool dmf() {return dmf_;};
    inline void dmf(bool v) { dmf_=v;};
    inline bool bcf() {return bcf_;};
    inline void bcf(bool v) { bcf_=v;};
    inline bool of() {return of_;};
    inline void of(bool v) { of_=v;};
    inline bool nf() {return nf_;};
    inline void nf(bool v) {nf_=v;};
    /* clock */
    inline unsigned int cycles(){return cycles_;};
    inline void cycles(unsigned int v){cycles_=v;};
    /* interrupts */
    void nmi();
    void irq();
    /* debug */
    void dump_regs();
    void dump_regs_json();
};

/* macro helpers */

#define SET_ZF(val)     (zf(!(uint8_t)(val)))
#define SET_NF(val)     (nf(((uint8_t)(val)&0x80)!=0))

#endif
