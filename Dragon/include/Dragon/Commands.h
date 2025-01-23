/*
BSD 3-Clause License

Copyright (c) 2024, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DRAGON_COMMANDS_H
#define DRAGON_COMMANDS_H

#include "Backway/StateMachine.h"

namespace Dragon
 {

class Command_Draw final : public Backway::Command
 {
public:
   std::string resource;
   int x, y, sx, sy;
   Command_Draw(const std::string& resource, int x, int y, int sx, int sy) : resource(resource), x(x), y(y), sx(sx), sy(sy) { }
 };

class Command_Move final : public Backway::Command
 {
public:
   int x, y, sx, sy;
   Command_Move(int x, int y, int sx, int sy) : x(x), y(y), sx(sx), sy(sy) { }
 };

class Command_PlayMusic final : public Backway::Command
 {
public:
   std::string song;
   Command_PlayMusic(const std::string& song) : song(song) { }
 };

class Command_PlaySound final : public Backway::Command
 {
public:
   std::string sound;
   Command_PlaySound(const std::string& sound) : sound(sound) { }
 };

 } // namespace Dragon

#endif /* DRAGON_COMMANDS_H */
