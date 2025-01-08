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
#include "Backway/State.h"

namespace Backway
 {

   std::shared_ptr<Backwards::Types::ValueType> State::update (Backwards::Engine::CallingContext& context, const std::shared_ptr<Backwards::Types::ValueType>& arg)
    {
      context.pushScope(&scope);
      try
       {
          // Build a function call and call it. I don't really have a better way of constantly updating the constant value.
         std::vector<std::shared_ptr<Backwards::Engine::Expression> > args;
         args.emplace_back(std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), arg));
         Backwards::Engine::FunctionCall call (Backwards::Input::Token(), updateFun, args);
         std::shared_ptr<Backwards::Types::ValueType> result = call.evaluate(context);
         context.popScope();
         return result;
       }
      catch (...)
       {
         context.popScope();
         throw;
       }
    }

 } // namespace Backway
