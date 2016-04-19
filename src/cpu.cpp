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

#include "cpu.h"
#include "util.h"
#include <sstream>

/**
 * @brief Cold reset
 *
 * https://www.c64-wiki.com/index.php/Reset_(Process)
 */
void Cpu::reset()
{
  a_ = x_ = y_ = sp_ = 0;
  cf_ = zf_ = idf_ = dmf_ = bcf_ = of_ = nf_ = false;
  pc(mem_->read_word(Memory::kAddrResetVector));
  cycles_ = 6;
}

/** 
 * @brief emulate instruction 
 * @return returns false if something goes wrong (e.g. illegal instruction)
 *
 * Current limitations:
 * 
 * - Illegal instructions are not implemented
 * - Excess cycles due to page boundary crossing are not calculated
 * - Some known architectural bugs are not emulated
 */
bool Cpu::emulate()
{
  /* fetch instruction */
  uint8_t insn = fetch_op();
  bool retval = true;
  /* emulate instruction */
  switch(insn)
  {
  /* BRK */
  case 0x0: brk(); break;
  /* ORA (nn,X) */
  case 0x1: ora(load_byte(addr_indx()),6); break;
  /* ORA nn */
  case 0x5: ora(load_byte(addr_zero()),3); break;
  /* ASL nn */
  case 0x6: asl_mem(addr_zero(),5); break;
  /* PHP */
  case 0x8: php(); break;
  /* ORA #nn */
  case 0x9: ora(fetch_op(),2); break;
  /* ASL A */
  case 0xA: asl_a(); break;
  /* ORA nnnn */
  case 0xD: ora(load_byte(addr_abs()),4); break;
  /* ASL nnnn */
  case 0xE: asl_mem(addr_abs(),6); break; 
  /* BPL nn */
  case 0x10: bpl(); break;
  /* ORA (nn,Y) */
  case 0x11: ora(load_byte(addr_indy()),5); break;
  /* ORA nn,X */
  case 0x15: ora(load_byte(addr_zerox()),4); break;
  /* ASL nn,X */
  case 0x16: asl_mem(addr_zerox(),6); break;
  /* CLC */
  case 0x18: clc(); break;
  /* ORA nnnn,Y */
  case 0x19: ora(load_byte(addr_absy()),4); break;
  /* ORA nnnn,X */
  case 0x1D: ora(load_byte(addr_absx()),4); break;
  /* ASL nnnn,X */
  case 0x1E: asl_mem(addr_absx(),7); break;
  /* JSR */
  case 0x20: jsr(); break;
  /* AND (nn,X) */
  case 0x21: _and(load_byte(addr_indx()),6); break;
  /* BIT nn */
  case 0x24: bit(addr_zero(),3); break;
  /* AND nn */
  case 0x25: _and(load_byte(addr_zero()),3); break;
  /* ROL nn */
  case 0x26: rol_mem(addr_zero(),5); break;
  /* PLP */
  case 0x28: plp(); break;
  /* AND #nn */
  case 0x29: _and(fetch_op(),2); break;
  /* ROL A */
  case 0x2A: rol_a(); break;
  /* BIT nnnn */
  case 0x2C: bit(addr_abs(),4); break;
  /* AND nnnn */
  case 0x2D: _and(load_byte(addr_abs()),4); break;
  /* ROL nnnn */
  case 0x2E: rol_mem(addr_abs(),6); break;
  /* BMI nn */
  case 0x30: bmi(); break;
  /* AND (nn,Y) */
  case 0x31: _and(load_byte(addr_indy()),5); break;               
  /* AND nn,X */
  case 0x35: _and(load_byte(addr_zerox()),4); break;
  /* ROL nn,X */
  case 0x36: rol_mem(addr_zerox(),6); break;
  /* SEC */
  case 0x38: sec(); break;
  /* AND nnnn,Y */
  case 0x39: _and(load_byte(addr_absy()),4); break;
  /* AND nnnn,X */
  case 0x3D: _and(load_byte(addr_absx()),4); break;
  /* ROL nnnn,X */
  case 0x3E: rol_mem(addr_absx(),7); break;
  /* RTI */
  case 0x40: rti(); break;
  /* EOR (nn,X) */
  case 0x41: eor(load_byte(addr_indx()),6); break;
  /* EOR nn */
  case 0x45: eor(load_byte(addr_zero()),3); break;
  /* LSR nn */
  case 0x46: lsr_mem(addr_zero(),5); break;
  /* PHA */
  case 0x48: pha(); break;
  /* EOR #nn */
  case 0x49: eor(fetch_op(),2); break;
  /* BVC */
  case 0x50: bvc(); break;
  /* JMP nnnn */
  case 0x4C: jmp(); break;
  /* EOR nnnn */
  case 0x4D: eor(load_byte(addr_abs()),4); break;
  /* LSR A */
  case 0x4A: lsr_a(); break;
  /* LSR nnnn */
  case 0x4E: lsr_mem(addr_abs(),6); break;
  /* EOR (nn,Y) */
  case 0x51: eor(load_byte(addr_indy()),5); break;
  /* EOR nn,X */
  case 0x55: eor(load_byte(addr_zerox()),4); break;
  /* LSR nn,X */
  case 0x56: lsr_mem(addr_zerox(),6); break;
  /* CLI */
  case 0x58: cli(); break;
  /* EOR nnnn,Y */
  case 0x59: eor(load_byte(addr_absy()),4); break;
  /* EOR nnnn,X */
  case 0x5D: eor(load_byte(addr_absx()),4); break;
  /* LSR nnnn,X */
  case 0x5E: lsr_mem(addr_absx(),7); break;
  /* RTS */
  case 0x60: rts(); break;
  /* ADC (nn,X) */
  case 0x61: adc(load_byte(addr_indx()),6); break;
  /* ADC nn */
  case 0x65: adc(load_byte(addr_zero()),3); break;
  /* ROR nn */
  case 0x66: ror_mem(addr_zero(),5); break;
  /* PLA */
  case 0x68: pla(); break;
  /* ADC #nn */
  case 0x69: adc(fetch_op(),2); break;
  /* ROR A */
  case 0x6A: ror_a(); break;
  /* JMP (nnnn) */
  case 0x6C: jmp_ind(); break;
  /* ADC nnnn */
  case 0x6D: adc(load_byte(addr_abs()),4); break;
  /* ROR nnnn */
  case 0x6E: ror_mem(addr_abs(),6); break;
  /* BVS */
  case 0x70: bvs(); break;
  /* ADC (nn,Y) */
  case 0x71: adc(load_byte(addr_indy()),5); break;
  /* ADC nn,X */
  case 0x75: adc(load_byte(addr_zerox()),4); break;
  /* ROR nn,X */
  case 0x76: ror_mem(addr_zerox(),6); break;
  /* SEI */
  case 0x78: sei(); break;
  /* ADC nnnn,Y */
  case 0x79: adc(load_byte(addr_absy()),4); break;
  /* ADC nnnn,X */
  case 0x7D: adc(load_byte(addr_absx()),4); break;
  /* ROR nnnn,X */
  case 0x7E: ror_mem(addr_absx(),7); break;
  /* STA (nn,X) */
  case 0x81: sta(addr_indx(),6); break;
  /* STY nn */
  case 0x84: sty(addr_zero(),3); break;
  /* STA nn */
  case 0x85: sta(addr_zero(),3); break;
  /* STX nn */
  case 0x86: stx(addr_zero(),3); break;
  /* DEY */
  case 0x88: dey(); break;
  /* TXA */
  case 0x8A: txa(); break;
  /* STY nnnn */
  case 0x8C: sty(addr_abs(),4); break;
  /* STA nnnn */
  case 0x8D: sta(addr_abs(),4); break;
  /* STX nnnn */
  case 0x8E: stx(addr_abs(),4); break;
  /* BCC nn */
  case 0x90: bcc(); break;
  /* STA (nn,Y) */
  case 0x91: sta(addr_indy(),6); break;
  /* STY nn,X */
  case 0x94: sty(addr_zerox(),4); break;
  /* STA nn,X */
  case 0x95: sta(addr_zerox(),4); break;
  /* STX nn,Y */
  case 0x96: stx(addr_zeroy(),4); break;
  /* TYA */
  case 0x98: tya(); break;
  /* STA nnnn,Y */
  case 0x99: sta(addr_absy(),5); break;
  /* TXS */
  case 0x9A: txs(); break;
  /* STA nnnn,X */
  case 0x9D: sta(addr_absx(),5); break;
  /* LDY #nn */
  case 0xA0: ldy(fetch_op(),2); break; 
  /* LDA (nn,X) */
  case 0xA1: lda(load_byte(addr_indx()),6); break;
  /* LDX #nn */
  case 0xA2: ldx(fetch_op(),2); break;
  /* LDY nn */
  case 0xA4: ldy(load_byte(addr_zero()),3); break;
  /* LDA nn */
  case 0xA5: lda(load_byte(addr_zero()),3); break;
  /* LDX nn */
  case 0xA6: ldx(load_byte(addr_zero()),3); break;
  /* TAY */
  case 0xA8: tay(); break;
  /* LDA #nn */
  case 0xA9: lda(fetch_op(),2); break;
  /* TAX */
  case 0xAA: tax(); break;
  /* LDY nnnn */
  case 0xAC: ldy(load_byte(addr_abs()),4); break;
  /* LDA nnnn */
  case 0xAD: lda(load_byte(addr_abs()),4); break;
  /* LDX nnnn */
  case 0xAE: ldx(load_byte(addr_abs()),4); break;
  /* BCS nn */
  case 0xB0: bcs(); break;
  /* LDA (nn,Y) */
  case 0xB1: lda(load_byte(addr_indy()),5); break;
  /* LDY nn,X */
  case 0xB4: ldy(load_byte(addr_zerox()),3); break;
  /* LDA nn,X */
  case 0xB5: lda(load_byte(addr_zerox()),3); break;
  /* LDX nn,Y */
  case 0xB6: ldx(load_byte(addr_zeroy()),3); break;
  /* CLV */
  case 0xB8: clv(); break;
  /* LDA nnnn,Y */
  case 0xB9: lda(load_byte(addr_absy()),4); break;
  /* TSX */
  case 0xBA: tsx(); break;
  /* LDY nnnn,X */
  case 0xBC: ldy(load_byte(addr_absx()),4); break;
  /* LDA nnnn,X */
  case 0xBD: lda(load_byte(addr_absx()),4); break;
  /* LDX nnnn,Y */
  case 0xBE: ldx(load_byte(addr_absy()),4); break;
  /* CPY #nn */
  case 0xC0: cpy(fetch_op(),2); break;
  /* CMP (nn,X) */
  case 0xC1: cmp(load_byte(addr_indx()),6); break;
  /* CPY nn */
  case 0xC4: cpy(load_byte(addr_zero()),3); break;
  /* CMP nn */
  case 0xC5: cmp(load_byte(addr_zero()),3); break;
  /* DEC nn */
  case 0xC6: dec(addr_zero(),5); break;
  /* INY */
  case 0xC8: iny(); break;
  /* CMP #nn */
  case 0xC9: cmp(fetch_op(),2); break;
  /* DEX */
  case 0xCA: dex(); break;
  /* CPY nnnn */
  case 0xCC: cpy(load_byte(addr_abs()),4); break;
  /* CMP nnnn */
  case 0xCD: cmp(load_byte(addr_abs()),4); break;
  /* DEC nnnn */
  case 0xCE: dec(addr_abs(),6); break;
  /* BNE nn */
  case 0xD0: bne(); break;
  /* CMP (nn,Y) */
  case 0xD1: cmp(load_byte(addr_indy()),5); break;
  /* CMP nn,X */
  case 0xD5: cmp(load_byte(addr_zerox()),4); break;
  /* DEC nn,X */
  case 0xD6: dec(addr_zerox(),6); break;
  /* CLD */
  case 0xD8: cld(); break;
  /* CMP nnnn,Y */
  case 0xD9: cmp(load_byte(addr_absy()),4); break;
  /* CMP nnnn,X */
  case 0xDD: cmp(load_byte(addr_absx()),4); break;
  /* DEC nnnn,X */
  case 0xDE: dec(addr_absx(),7); break;
  /* CPX #nn */
  case 0xE0: cpx(fetch_op(),2); break;
  /* SBC (nn,X) */
  case 0xE1: sbc(load_byte(addr_indx()),6); break;
  /* CPX nn */
  case 0xE4: cpx(load_byte(addr_zero()),3); break;
  /* SBC nn */
  case 0xE5: sbc(load_byte(addr_zero()),3); break;
  /* INC nn */
  case 0xE6: inc(addr_zero(),5); break;
  /* INX */
  case 0xE8: inx(); break;
  /* SBC #nn */
  case 0xE9: sbc(fetch_op(),2); break;
  /* NOP */
  case 0xEA: nop(); break;
  /* CPX nnnn */
  case 0xEC: cpx(load_byte(addr_abs()),4); break;
  /* SBC nnnn */
  case 0xED: sbc(load_byte(addr_abs()),4); break;
  /* INC nnnn */
  case 0xEE: inc(addr_abs(),6); break;
  /* BEQ nn */
  case 0xF0: beq(); break;
  /* SBC (nn,Y) */
  case 0xF1: sbc(load_byte(addr_indy()),5); break;
  /* SBC nn,X */
  case 0xF5: sbc(load_byte(addr_zerox()),4); break;
  /* INC nn,X */
  case 0xF6: inc(addr_zerox(),6); break;
  /* SED */
  case 0xF8: sed(); break;
  /* SBC nnnn,Y */
  case 0xF9: sbc(load_byte(addr_absy()),4); break;
  /* SBC nnnn,X */
  case 0xFD: sbc(load_byte(addr_absx()),4); break;
  /* INC nnnn,X */
  case 0xFE: inc(addr_absx(),7); break;
  /* Unknown or illegal instruction */
  default:
    D("Unknown instruction: %X at %04x\n", insn,pc());
    retval = false;
  }
  return retval;
}

// helpers ///////////////////////////////////////////////////////////////////

uint8_t Cpu::load_byte(uint16_t addr)
{
  return mem_->read_byte(addr);
}

void Cpu::push(uint8_t v)
{
  uint16_t addr = Memory::kBaseAddrStack+sp_;
  mem_->write_byte(addr,v);
  sp_--;
}

uint8_t Cpu::pop()
{
  uint16_t addr = ++sp_+Memory::kBaseAddrStack;
  return load_byte(addr);
}
 
uint8_t Cpu::fetch_op()
{
  return load_byte(pc_++);
}

uint16_t Cpu::fetch_opw()
{
  uint16_t retval = mem_->read_word(pc_);
  pc_+=2;
  return retval;
}

uint16_t Cpu::addr_zero()
{
  uint16_t addr = fetch_op();
  return addr;
}

uint16_t Cpu::addr_zerox()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + x()) & 0xff ;
  return addr;
}
 
uint16_t Cpu::addr_zeroy()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + y()) & 0xff;
  return addr;
}

uint16_t Cpu::addr_abs()
{
  uint16_t addr = fetch_opw();
  return addr;
}

uint16_t Cpu::addr_absy()
{
  uint16_t addr = fetch_opw() + y();
  return addr;
}

uint16_t Cpu::addr_absx()
{
  uint16_t addr = fetch_opw() + x();
  return addr;  
}

uint16_t Cpu::addr_indx()
{
  /* wraps around the zeropage */
  uint16_t addr = mem_->read_word((addr_zero() + x()) & 0xff);
  return addr;
}

uint16_t Cpu::addr_indy()
{
  uint16_t addr = mem_->read_word(addr_zero()) + y();
  return addr;
}

// Instructions: data handling and memory operations  ////////////////////////

/**
 * @brief STore Accumulator
 */
void Cpu::sta(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,a());
  tick(cycles);
}

/**
 * @brief STore X
 */
void Cpu::stx(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,x());
  tick(cycles);
}

/**
 * @brief STore Y
 */
void Cpu::sty(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,y());
  tick(cycles);
}

/**
 * @brief Transfer X to Stack pointer
 */
void Cpu::txs()
{
  sp(x());
  tick(2);
}

/**
 * @brief Transfer Stack pointer to X
 */
void Cpu::tsx()
{
  x(sp());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief LoaD Accumulator
 */
void Cpu::lda(uint8_t v, uint8_t cycles)
{
  a(v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief LoaD X
 */
void Cpu::ldx(uint8_t v, uint8_t cycles)
{
  x(v);
  SET_ZF(x());
  SET_NF(x());
  tick(cycles);
}

/**
 * @brief LoaD Y
 */
void Cpu::ldy(uint8_t v, uint8_t cycles)
{
  y(v);
  SET_ZF(y());
  SET_NF(y());
  tick(cycles);
}

/**
 * @brief Transfer X to Accumulator 
 */
void Cpu::txa()
{
  a(x());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief Transfer Accumulator to X 
 */
void Cpu::tax()
{
  x(a());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief Transfer Accumulator to Y 
 */
void Cpu::tay()
{
  y(a());
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief Transfer Y to Accumulator 
 */
void Cpu::tya()
{
  a(y());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief PusH Accumulator 
 */
void Cpu::pha()
{
  push(a());
  tick(3);
}

/**
 * @brief PuLl Accumulator 
 */
void Cpu::pla()
{
  a(pop());
  SET_ZF(a());
  SET_NF(a());
  tick(4);
}
 
// Instructions: logic operations  ///////////////////////////////////////////

/**
 * @brief Logical OR on Accumulator
 */
void Cpu::ora(uint8_t v, uint8_t cycles)
{
  a(a()|v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief Logical AND
 */
void Cpu::_and(uint8_t v, uint8_t cycles)
{
  a(a()&v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief BIT test
 */
void Cpu::bit(uint16_t addr, uint8_t cycles)
{
  uint8_t t = load_byte(addr);
  of((t&0x40)!=0);
  SET_NF(t);
  SET_ZF(t&a());
  tick(cycles);
}
 
/**
 * @brief ROtate Left
 */
uint8_t Cpu::rol(uint8_t v)
{
  uint16_t t = (v << 1) | (uint8_t)cf();
  cf((t&0x100)!=0);
  SET_ZF(t);
  SET_NF(t);
  return (uint8_t)t;
}

/** 
 * @brief ROL A register
 */
void Cpu::rol_a()
{
  a(rol(a()));
  tick(2);
}

/**
 * @brief ROL mem 
 */
void Cpu::rol_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,rol(v));
  tick(cycles);
}

/**
 * @brief ROtate Right
 */
uint8_t Cpu::ror(uint8_t v)
{
  uint16_t t = (v >> 1) | (uint8_t)(cf() << 7);
  cf((v&0x1)!=0);
  SET_ZF(t);
  SET_NF(t);
  return (uint8_t)t;
}

/** 
 * @brief ROR A register
 */
void Cpu::ror_a()
{
  a(ror(a()));
  tick(2);
}

/**
 * @brief ROR mem 
 */
void Cpu::ror_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,ror(v));
  tick(cycles);
}       

/**
 * @brief Logic Shift Right
 */
uint8_t Cpu::lsr(uint8_t v)
{
  uint8_t t = v >> 1;
  cf((v&0x1)!=0);
  SET_ZF(t);
  SET_NF(t);
  return t;
}

/** 
 * @brief LSR A
 */
void Cpu::lsr_a()
{
  a(lsr(a()));
  tick(2);
}

/**
 * @brief LSR mem
 */
void Cpu::lsr_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,lsr(v));
  tick(cycles);
}

/**
 * @brief Arithmetic Shift Left
 */
uint8_t Cpu::asl(uint8_t v)
{
  uint8_t t = (v << 1) & 0xff;
  cf((v&0x80)!=0);
  SET_ZF(t);
  SET_NF(t);
  return t;
}

/** 
 * @brief ASL A
 */
void Cpu::asl_a()
{
  a(asl(a()));
  tick(2);
}

/**
 * @brief ASL mem 
 *
 * ASL and the other read-modify-write instructions contain a bug (wikipedia):
 *
 * --
 * The 6502's read-modify-write instructions perform one read and two write 
 * cycles. First the unmodified data that was read is written back, and then 
 * the modified data is written. This characteristic may cause issues by 
 * twice accessing hardware that acts on a write. This anomaly continued 
 * through the entire NMOS line, but was fixed in the CMOS derivatives, in 
 * which the processor will do two reads and one write cycle. 
 * --
 *
 * I have come across code that uses this side-effect as a feature, for
 * instance, the following instruction will acknowledge VIC interrupts 
 * on the first write cycle:
 *
 * ASL $d019
 *
 * So.. we need to mimic the behaviour.
 */
void Cpu::asl_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  mem_->write_byte(addr,v); 
  mem_->write_byte(addr,asl(v));
  tick(cycles);
} 

/**
 * @brief Exclusive OR 
 */
void Cpu::eor(uint8_t v, uint8_t cycles)
{
  a(a()^v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}
 
// Instructions: arithmetic operations  //////////////////////////////////////

/**
 * @brief INCrement
 */
void Cpu::inc(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  v++;
  mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
}

/**
 * @brief DECrement
 */
void Cpu::dec(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  v--;
  mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
}

/**
 * @brief INcrement X
 */
void Cpu::inx()
{
  x_+=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief INcrement Y
 */
void Cpu::iny()
{
  y_+=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief DEcrement X
 */
void Cpu::dex()
{
  x_-=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief DEcrement Y
 */
void Cpu::dey()
{
  y_-=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief ADd with Carry
 */
void Cpu::adc(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  if(dmf())
  {
    t = (a()&0xf) + (v&0xf) + (cf() ? 1 : 0);
    if (t > 0x09) 
      t += 0x6;
    t += (a()&0xf0) + (v&0xf0);
    if((t & 0x1f0) > 0x90) 
      t += 0x60;
  }
  else
  {
    t = a() + v + (cf() ? 1 : 0);
  }
  cf(t>0xff);
  t=t&0xff;
  of(!((a()^v)&0x80) && ((a()^t) & 0x80));
  SET_ZF(t);
  SET_NF(t);
  a((uint8_t)t);
}

/**
 * @brief SuBstract with Carry
 */
void Cpu::sbc(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  if(dmf())
  {
    t = (a()&0xf) - (v&0xf) - (cf() ? 0 : 1);
    if((t & 0x10) != 0)
      t = ((t-0x6)&0xf) | ((a()&0xf0) - (v&0xf0) - 0x10);
    else
      t = (t&0xf) | ((a()&0xf0) - (v&0xf0));
    if((t&0x100)!=0)
      t -= 0x60;
  }
  else
  {
    t = a() - v - (cf() ? 0 : 1);
  }
  cf(t<0x100);
  t=t&0xff;
  of(((a()^t)&0x80) && ((a()^v) & 0x80));
  SET_ZF(t);
  SET_NF(t);
  a((uint8_t)t);
}
 
// Instructions: flag access ///////////////////////////////////////////////// 

/** 
 * @brief SEt Interrupt flag
 */
void Cpu::sei()
{
  idf(true);
  tick(2);
}

/**
 * @brief CLear Interrupt flag
 */
void Cpu::cli()
{
  idf(false);
  tick(2);
}

/**
 * @brief SEt Carry flag 
 */
void Cpu::sec()
{
  cf(true);
  tick(2);
}
 
/**
 * @brief CLear Carry flag 
 */
void Cpu::clc()
{
  cf(false);
  tick(2);
}

/**
 * @brief SEt Decimal flag 
 */
void Cpu::sed()
{
  dmf(true);
  tick(2);
}
 
/**
 * @brief CLear Decimal flag 
 */
void Cpu::cld()
{
  dmf(false);
  tick(2);
}

/**
 * @brief CLear oVerflow flag 
 */
void Cpu::clv()
{
  of(false);
  tick(2);
}

uint8_t Cpu::flags()
{
  uint8_t v=0;
  v |= cf()  << 0;
  v |= zf()  << 1;
  v |= idf() << 2;
  v |= dmf() << 3;
  /* brk & php instructions push the bcf flag active */
  v |= 1 << 4;
  /* unused, always set */
  v |= 1     << 5;
  v |= of()  << 6;
  v |= nf()  << 7;
  return v;
}

void Cpu::flags(uint8_t v)
{
  cf(ISSET_BIT(v,0));
  zf(ISSET_BIT(v,1));
  idf(ISSET_BIT(v,2));
  dmf(ISSET_BIT(v,3));
  of(ISSET_BIT(v,6));
  nf(ISSET_BIT(v,7));     
}

/**
 * @brief PusH Processor flags
 */
void Cpu::php()
{
  push(flags());
  tick(3);
}

/**
 * @brief PuLl Processor flags
 */
void Cpu::plp()
{
  flags(pop());
  tick(4);
}

/**
 * @brief Jump to SubRoutine
 * 
 * Note that JSR does not push the address of the next instruction
 * to the stack but the address to the last byte of its own 
 * instruction.
 */
void Cpu::jsr()
{
  uint16_t addr = addr_abs();
  push(((pc()-1) >> 8) & 0xff);
  push(((pc()-1) & 0xff));
  pc(addr);
  tick(6);
}

/**
 * @brief JuMP
 */
void Cpu::jmp()
{
  uint16_t addr = addr_abs();
  pc(addr);
  tick(3);
}

/**
 * @brief JuMP (indirect)
 */
void Cpu::jmp_ind()
{
  uint16_t addr = mem_->read_word(addr_abs());
  pc(addr);
  tick(3);    
}
 
/**
 * @brief ReTurn from SubRoutine
 */
void Cpu::rts()
{
  uint16_t addr = (pop() + (pop() << 8)) + 1;
  pc(addr);
  tick(6);
}

/** 
 * @brief Branch if Not Equal
 */
void Cpu::bne()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!zf()) pc(addr);
  tick(2);
}

/** 
 * @brief CoMPare
 */
void Cpu::cmp(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  t = a() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

/** 
 * @brief CoMPare X
 */
void Cpu::cpx(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  t = x() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

/** 
 * @brief CoMPare Y
 */
void Cpu::cpy(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  t = y() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}
 
/** 
 * @brief Branch if Equal
 */
void Cpu::beq()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(zf()) pc(addr);
  tick(2);
}

/** 
 * @brief Branch if Carry is Set
 */
void Cpu::bcs()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(cf()) pc(addr);
  tick(2);
}

/** 
 * @brief Branch if Carry is Clear
 */
void Cpu::bcc()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!cf()) pc(addr);
  tick(2);
}
 
/**
 * @brief, Branch if PLus
 */
void Cpu::bpl()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!nf()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if MInus
 */
void Cpu::bmi()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(nf()) pc(addr);
  tick(2);    
}

/**
 * @brief Branch if oVerflow Clear
 */
void Cpu::bvc()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!of()) pc(addr);
  tick(2);    
}

/**
 * @brief Branch if oVerflow Set
 */
void Cpu::bvs()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(of()) pc(addr);
  tick(2);    
}

// misc //////////////////////////////////////////////////////////////////////

/**
 * @brief No OPeration
 */
void Cpu::nop()
{
  tick(2);
}

/**
 * @brief BReaKpoint
 */
void Cpu::brk()
{
  push(((pc()+1) >> 8) & 0xff);
  push(((pc()+1) & 0xff));
  push(flags());
  pc(mem_->read_word(Memory::kAddrIRQVector));
  idf(true);
  bcf(true);
  tick(7);
}

/**
 * @brief ReTurn from Interrupt 
 */
void Cpu::rti()
{
  flags(pop());
  pc(pop() + (pop() << 8));
  tick(7);
}

// interrupts  ///////////////////////////////////////////////////////////////

/**
 * @brief Interrupt ReQuest
 */
void Cpu::irq()
{
  if(!idf())
  {
    push(((pc()) >> 8) & 0xff);
    push(((pc()) & 0xff));
    /* push flags with bcf cleared */
    push((flags()&0xef));
    pc(mem_->read_word(Memory::kAddrIRQVector));
    idf(true);
    tick(7);
  }
}

/**
 * @brief Non Maskable Interrupt
 */
void Cpu::nmi()
{
  push(((pc()) >> 8) & 0xff);
  push(((pc()) & 0xff));
  /* push flags with bcf cleared */
  push((flags() & 0xef));
  pc(mem_->read_word(Memory::kAddrNMIVector));
  tick(7);
}

// debugging /////////////////////////////////////////////////////////////////

void Cpu::dump_regs()
{
  std::stringstream sflags;
  if(cf())  sflags << "CF ";
  if(zf())  sflags << "ZF ";
  if(idf()) sflags << "IDF ";
  if(dmf()) sflags << "DMF ";
  if(bcf()) sflags << "BCF ";
  if(of())  sflags << "OF ";
  if(nf())  sflags << "NF ";
  D("pc=%04x, a=%02x x=%02x y=%02x sp=%02x flags= %s\n",
    pc(),a(),x(),y(),sp(),sflags.str().c_str());
}

void Cpu::dump_regs_json()
{
  D("{");
  D("\"pc\":%d,",pc());
  D("\"a\":%d,",a());
  D("\"x\":%d,",x());
  D("\"y\":%d,",y());
  D("\"sp\":%d",sp());
  D("}\n");
}
