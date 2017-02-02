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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "util.h"
#include "debugger.h"

Debugger::Debugger()
{
  int e = 1;
  offset_ = 0;
  struct sockaddr_in serv_addr;
  /* SOCK_NONBLOCK requires a Linux kernel >= 2.6.27 */
#ifdef SOCK_NONBLOCK
  ss_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#else
  ss_ = socket(AF_INET, SOCK_STREAM,0);
  fcntl(ss_, F_SETFL, fcntl(ss_, F_GETFL, 0) | O_NONBLOCK);
#endif
  if(ss_ < 0)
  {
    D("Debuggger: Error opening socket\n");
    exit(1);
  }
  setsockopt(ss_, SOL_SOCKET, SO_REUSEADDR, &e, sizeof(int)); 
  bzero((char *) &serv_addr, sizeof(serv_addr));
  /* init socket struct */
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons(kDbgPort);
  /* bind */
  if (bind(ss_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    D("Debugger: ERROR binding\n");
    exit(1);
  }
  /* listen */
  listen(ss_ , 3);
}

Debugger::~Debugger()
{
  close(ss_);
}

std::vector<std::string> Debugger::split_cmd(const std::string &s)
{
  const char c = ' '; 
  std::string buff{""};
  std::vector<std::string> v;
  for(auto n:s)
  {
    if(n != c) buff+=n; else
    if(n == c && buff != "") { v.push_back(buff); buff = ""; }
  }
  if(buff != "") v.push_back(buff);
  return v;
}

uint16_t Debugger::emu_seek(uint16_t offset, int whence)
{
  switch(whence)
  {
  case SEEK_SET:
    offset_ = offset; 
    break;
  case SEEK_CUR:
    offset_ = offset + offset_;
    break;
  case SEEK_END:
    offset_ = 0xffff + offset;
    break;
  }
  return offset_;
}

uint8_t * Debugger::emu_read_mem(size_t sz)
{
  uint8_t *mem = (uint8_t *) malloc(sz);
  for(size_t i = 0 ; i < sz ; i++)
    mem[i] = mem_->read_byte(offset_ + i);
  return mem;
}

void Debugger::emu_write_mem(uint8_t *mem, size_t sz)
{
  for(size_t i = 0 ; i < sz ; i++)
  {
    mem_->write_byte(offset_++,mem[i]);
  }
}

std::string Debugger::regs_cmd()
{
  std::stringstream v;
  v << std::hex << std::setfill('0');
  v << "a = 0x" << std::setw(2) << (unsigned int) cpu_->a() << "\n";
  v << "x = 0x" << std::setw(2) << (unsigned int) cpu_->x() << "\n";
  v << "y = 0x" << std::setw(2) << (unsigned int) cpu_->y() << "\n";
  v << "sp = 0x" << std::setw(2) << (unsigned int) cpu_->sp() << "\n";
  v << "pc = 0x" << std::setw(4) << (unsigned int) cpu_->pc() << "\n";
  v << "flags = ";
  if(cpu_->cf())  v << "cf ";
  if(cpu_->zf())  v << "zf ";
  if(cpu_->idf()) v << "idf ";
  if(cpu_->dmf()) v << "dmf ";
  if(cpu_->bcf()) v << "bcf ";
  if(cpu_->of())  v << "of ";
  if(cpu_->nf())  v << "nf ";
  v << "\n";
  return v.str();
}

std::string Debugger::emu_handle_cmd(const std::string &s)
{
  std::vector<std::string> args = split_cmd(s);
  std::string r("");
  try
  {
    if(args.at(0) == "dr")
      r = regs_cmd();
  }
  catch(const std::out_of_range){}
  return r;
}

bool Debugger::emulate()
{ 
  int sockfd;
  struct sockaddr_in sa;
  socklen_t sl = sizeof(sa);
  /* non blocking */
  sockfd = accept(ss_, (struct sockaddr *)&sa, &sl);
  if(sockfd > 0)
  {
    /* unset fd's O_NONBLOCK flag
     * on Linux reading from the fd seems to block while
     * on OSX it seems to inherit the socket's non-blocking flag
     */
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) &~ O_NONBLOCK);
    D("Debugger: client connected\n");
    /* emulation paused while debugger is connected */
    while(1)
    {    
      int whence;
      uint8_t *mem;
      uint32_t sz;
      uint64_t offset;
      uint8_t buff[kMaxCmdLen];
      std::string r;
      /* continue emulation if debugger disconnected */
      if(!recv(sockfd,buff,1,0))
      {
        D("Debugger: client disconnected\n");
        break;
      }
      switch(buff[0])
      {
      case RAP_RMT_OPEN:
        recv(sockfd,&buff[1],2,0);
        recv(sockfd,&buff[3],(int)buff[2],0);
        buff[0] = RAP_RMT_OPEN | RAP_RMT_REPLY;
        send(sockfd,buff,5,0);
        break;
      case RAP_RMT_SEEK:
        recv(sockfd,&buff[1],9,0);
        whence = buff[1];
        offset = ntohll(*(uint64_t*)&buff[2]);
        offset = htonll(emu_seek((uint16_t)offset,whence));
        buff[0] = RAP_RMT_SEEK | RAP_RMT_REPLY;
        send(sockfd,buff,1,0);
        send(sockfd,&offset,8,0);
        break;
      case RAP_RMT_READ:
        recv(sockfd,&buff[1],4,0);
        sz = ntohl(*(uint32_t*)&buff[1]);
        buff[0] = RAP_RMT_READ | RAP_RMT_REPLY;
        mem = emu_read_mem(sz);
        send(sockfd,buff,5,0);
        send(sockfd,mem,sz,0);
        free(mem);
        break;
      case RAP_RMT_WRITE:
        recv(sockfd,&buff[1],4,0);
        sz = ntohl(*(uint32_t*)&buff[1]);
        recv(sockfd,&buff[5],sz,0);
        emu_write_mem(&buff[5],sz);
        buff[0] = RAP_RMT_WRITE | RAP_RMT_REPLY;
        send(sockfd,buff,5,0);
        break;
      case RAP_RMT_CMD:
        recv(sockfd,&buff[1],4,0);
        sz = ntohl(*(uint32_t*)&buff[1]);
        recv(sockfd,&buff[5],sz,0);
        r = emu_handle_cmd(std::string((char*)&buff[5],(size_t)(sz-1)));
        *(uint32_t*)(&buff[1]) = htonl(r.length());
        buff[0] = RAP_RMT_CMD | RAP_RMT_REPLY;
        send(sockfd,buff,5,0);
        send(sockfd,r.c_str(),r.length(),0);
        break;
      default:
        D("Debugger: Unknown command %x\n",buff[0]);
        break;
      }
    }
  }
  return true;
}
