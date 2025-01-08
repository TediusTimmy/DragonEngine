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
#include "Backwards/Parser/ContextBuilder.h"
#include "Backwards/Engine/Statement.h"
#include "Backway/CallingContext.h"
#include "Backway/ContextBuilder.h"
#include "Backway/StdLib.h"
#include "Dragon/CallingContext.h"
#include "Dragon/ContextBuilder.h"
#include "Dragon/StdLib.h"

namespace Dragon
 {

   std::shared_ptr<Backwards::Engine::CallingContext> CallingContext::duplicate()
    {
      std::shared_ptr<CallingContext> result = std::make_shared<CallingContext>();
      duplicate(result);
      return result;
    }

   void CallingContext::duplicate(std::shared_ptr<CallingContext> result)
    {
      Backway::CallingContext::duplicate(result);
      result->universe = universe;
      result->viron = viron;
    }

   void ContextBuilder::createGlobalScope (Backwards::Engine::Scope& global)
    {
      Backway::ContextBuilder::createGlobalScope(global);

    // 4
      Backwards::Parser::ContextBuilder::addFunction("ClearDynamicLayer", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(ClearDynamicLayer), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("ClearPlayerLayer", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(ClearPlayerLayer), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("GetMyName", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(GetMyName), 0U, global);
      Backwards::Parser::ContextBuilder::addFunction("GetScreenXY", std::make_shared<Backwards::Engine::StandardConstantFunctionWithContext>(GetScreenXY), 0U, global);

    // 2
      Backwards::Parser::ContextBuilder::addFunction("RemoveEntity", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(RemoveEntity), 1U, global);
      Backwards::Parser::ContextBuilder::addFunction("SetScreenXY", std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(SetScreenXY), 1U, global);

    // 5
      Backwards::Parser::ContextBuilder::addFunction("CreateBackgroundEntity", std::make_shared<Backway::StandardBinaryFunctionWithContext>(CreateBackgroundEntity), 2U, global);
      Backwards::Parser::ContextBuilder::addFunction("CreateDynamicEntity", std::make_shared<Backway::StandardBinaryFunctionWithContext>(CreateDynamicEntity), 2U, global);
      Backwards::Parser::ContextBuilder::addFunction("CreatePlayerEntity", std::make_shared<Backway::StandardBinaryFunctionWithContext>(CreatePlayerEntity), 2U, global);
      Backwards::Parser::ContextBuilder::addFunction("DrawSprite", std::make_shared<Backway::StandardBinaryFunctionWithContext>(DrawSprite), 2U, global);
      Backwards::Parser::ContextBuilder::addFunction("SendMessage", std::make_shared<Backway::StandardBinaryFunctionWithContext>(SendMessage), 2U, global);
    }

 } // namespace Dragon
