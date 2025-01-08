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
#include "Backwards/Parser/ContextBuilder.h"
#include "Backwards/Engine/Statement.h"
#include "Backway/CallingContext.h"
#include "Backway/ContextBuilder.h"
#include "Backway/StdLib.h"

namespace Backway
 {

   std::shared_ptr<Backwards::Engine::CallingContext> CallingContext::duplicate()
    {
      std::shared_ptr<CallingContext> result = std::make_shared<CallingContext>();
      duplicate(result);
      return result;
    }

   void CallingContext::duplicate(std::shared_ptr<CallingContext> result)
    {
      Backwards::Engine::CallingContext::duplicate(result);
      result->machine = machine;
      result->environment = environment;
    }

   void ContextBuilder::createGlobalScope (Backwards::Engine::Scope& global)
    {
      Backwards::Parser::ContextBuilder::createGlobalScope(global);

    // 5
      Backwards::Parser::ContextBuilder::addFunction("GetInput", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(GetInput), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("GetName", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(GetName), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("Leave", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(Leave), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("Rand", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(Rand), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("Return", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(Return), 0U, global);

    // 9
      Backwards::Parser::ContextBuilder::addFunction("SetOutput", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(SetOutput), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Enqueue", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Enqueue), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Finally", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Finally), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Follow", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Follow), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Inject", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Inject), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Precede", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Precede), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Push", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Push), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Skip", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Skip), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("Unwind", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Unwind), 1U, global);

    // 1
      Backwards::Parser::ContextBuilder::addFunction("CreateState", std::make_shared<StandardBinaryFunctionWithContext>(CreateState), 2U, global);
    }

 } // namespace Backway
