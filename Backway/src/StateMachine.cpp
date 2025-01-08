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
#include "Backway/StateMachine.h"
#include "../external/city.h"

#include "Backwards/Engine/ConstantsSingleton.h"

namespace Backway
 {

   inline static __uint128_t UnPack(const uint128& src)
    {
      return static_cast<__uint128_t>(Uint128Low64(src)) | ((static_cast<__uint128_t>(Uint128High64(src))) << 64);
    }

   StateMachine::StateMachine(const std::string& name) : name(name), rng(UnPack(CityHash128(name.c_str(), name.length()))), environment(nullptr)
    {
    }

   bool StateMachine::update(CallingContext& context)
    {
      std::shared_ptr<State> currentState;
      output = std::shared_ptr<Command>();
      result = Backwards::Engine::ConstantsSingleton::getInstance().EMPTY_DICTIONARY;
      if (false == states.empty())
       {
         do
          {
             // State to process is the front of the list (queue) of the top of the stack.
            currentState = states.back().front();
            last = currentState->update(context, last);
          }
          // If the current active state has changed, run its update function NOW.
          // The idea here is to support dispatch states: a state which enqueues an activity to run,
          // so that the activity has a stable name, even if its components are not.
         while ((false == states.empty()) && (currentState.get() != states.back().front().get()));
       }
      return false == states.empty();
    }

   void StateMachine::addOutput(std::shared_ptr<Command> next)
    {
      if (nullptr == output.get())
       {
         output = next;
       }
      else
       {
         output->last->next = next;
       }
      output->last = next.get();
    }

 } // namespace Backway
