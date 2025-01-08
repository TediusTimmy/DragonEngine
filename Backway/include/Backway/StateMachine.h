/*
BSD 3-Clause License

Copyright (c) 2022, Thomas DiModica
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
#ifndef BACKWAY_STATEMACHINE_H
#define BACKWAY_STATEMACHINE_H

#include "Backwards/Types/ValueType.h"
#include "Backway/CallingContext.h"
#include "Backway/State.h"
#include "../external/pcg_random.hpp"

#include <list>

namespace Backway
 {

   class Command
    {
   public:
      std::shared_ptr<Command> next;
      Command* last;
      Command() : last(nullptr) { }
      virtual ~Command() = default;
    };

   class StateMachine final
    {
   public:
      std::string name;
      std::list<std::list<std::shared_ptr<State> > > states;
      std::shared_ptr<Backwards::Types::ValueType> last;
      std::shared_ptr<Backwards::Types::ValueType> input;
      std::shared_ptr<Backwards::Types::ValueType> result;
      std::shared_ptr<Command> output;
      pcg64 rng;
      Environment* environment;

      StateMachine(const std::string& name);

      bool update(CallingContext&);

      void addOutput(std::shared_ptr<Command> next);
    };

 } // namespace Backway

#endif /* BACKWAY_STATEMACHINE_H */
