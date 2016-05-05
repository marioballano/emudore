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
 
#include <iostream>
#include <string>
#include <algorithm>

#include "c64.h"
#include "loader.h"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

C64 *c64;
Loader *loader;

bool loader_cb()
{
  if(!loader->emulate())
    c64->callback(nullptr);
  return true;
}

void emscripten_loop()
{
  c64->emscripten_loop();
}

int main(int argc, char **argv)
{
  c64 = new C64();
  /* check if asked load a program */
  if(argc != 1)
  {
    std::string f(argv[1]);
    size_t ext_i = f.find_last_of(".");
    if(ext_i != std::string::npos)
    {
      std::string ext(f.substr(ext_i + 1));
      std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
      if(ext == "bas"){
        loader = new Loader(c64);
        loader->bas(f);
        c64->callback(loader_cb);
      }
      else if(ext == "prg"){
        loader = new Loader(c64);
        loader->prg(f);
        c64->callback(loader_cb);
      }     
    }
  }
#ifdef EMSCRIPTEN
  emscripten_set_main_loop(emscripten_loop,0,0);
#else
  c64->start();
#endif
  return 0;
}
