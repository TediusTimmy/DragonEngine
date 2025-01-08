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
#include "Backwards/Engine/StdLib.h"
#include "Backwards/Engine/FunctionContext.h"

#include <sstream>

namespace Backwards
 {

namespace Parser
 {

   void ContextBuilder::createGlobalScope(Engine::Scope& global)
    {
    // 6
      addFunction("PI", std::make_shared<Engine::StandardConstantFunction>(Engine::PI), 0U, global);
      addFunction("Date", std::make_shared<Engine::StandardConstantFunction>(Engine::Date), 0U, global);
      addFunction("Time", std::make_shared<Engine::StandardConstantFunction>(Engine::Time), 0U, global);
      addFunction("NewArray", std::make_shared<Engine::StandardConstantFunction>(Engine::NewArray), 0U, global);
      addFunction("NewDictionary", std::make_shared<Engine::StandardConstantFunction>(Engine::NewDictionary), 0U, global);
      addFunction("GetRoundMode", std::make_shared<Engine::StandardConstantFunction>(Engine::GetRoundMode), 0U, global);

    // 1
      addFunction("EnterDebugger", std::make_shared<Engine::StandardConstantFunctionWithContext>(Engine::EnterDebugger), 0U, global);

    // 37
      addFunction("Sin", std::make_shared<Engine::StandardUnaryFunction>(Engine::Sin), 1U, global);
      addFunction("Cos", std::make_shared<Engine::StandardUnaryFunction>(Engine::Cos), 1U, global);
      addFunction("Tan", std::make_shared<Engine::StandardUnaryFunction>(Engine::Tan), 1U, global);
      addFunction("Asin", std::make_shared<Engine::StandardUnaryFunction>(Engine::Asin), 1U, global);
      addFunction("Acos", std::make_shared<Engine::StandardUnaryFunction>(Engine::Acos), 1U, global);
      addFunction("Atan", std::make_shared<Engine::StandardUnaryFunction>(Engine::Atan), 1U, global);
      addFunction("Sinh", std::make_shared<Engine::StandardUnaryFunction>(Engine::Sinh), 1U, global);
      addFunction("Cosh", std::make_shared<Engine::StandardUnaryFunction>(Engine::Cosh), 1U, global);
      addFunction("Tanh", std::make_shared<Engine::StandardUnaryFunction>(Engine::Tanh), 1U, global);
      addFunction("Exp", std::make_shared<Engine::StandardUnaryFunction>(Engine::Exp), 1U, global);
      addFunction("Ln", std::make_shared<Engine::StandardUnaryFunction>(Engine::Ln), 1U, global);
      addFunction("Sqrt", std::make_shared<Engine::StandardUnaryFunction>(Engine::Sqrt), 1U, global);
      addFunction("Cbrt", std::make_shared<Engine::StandardUnaryFunction>(Engine::Cbrt), 1U, global);
      addFunction("Sqr", std::make_shared<Engine::StandardUnaryFunction>(Engine::Sqr), 1U, global);
      addFunction("Abs", std::make_shared<Engine::StandardUnaryFunction>(Engine::Abs), 1U, global);
      addFunction("Round", std::make_shared<Engine::StandardUnaryFunction>(Engine::Round), 1U, global);
      addFunction("Floor", std::make_shared<Engine::StandardUnaryFunction>(Engine::Floor), 1U, global);
      addFunction("Ceil", std::make_shared<Engine::StandardUnaryFunction>(Engine::Ceil), 1U, global);
      addFunction("ToString", std::make_shared<Engine::StandardUnaryFunction>(Engine::ToString), 1U, global);
      addFunction("Length", std::make_shared<Engine::StandardUnaryFunction>(Engine::Length), 1U, global);
      addFunction("DegToRad", std::make_shared<Engine::StandardUnaryFunction>(Engine::DegToRad), 1U, global);
      addFunction("RadToDeg", std::make_shared<Engine::StandardUnaryFunction>(Engine::RadToDeg), 1U, global);
      addFunction("IsNaN", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsNaN), 1U, global);
      addFunction("IsInfinity", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsInfinity), 1U, global);
      addFunction("ValueOf", std::make_shared<Engine::StandardUnaryFunction>(Engine::ValueOf), 1U, global);
      addFunction("ToCharacter", std::make_shared<Engine::StandardUnaryFunction>(Engine::ToCharacter), 1U, global);
      addFunction("FromCharacter", std::make_shared<Engine::StandardUnaryFunction>(Engine::FromCharacter), 1U, global);
      addFunction("IsFloat", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsFloat), 1U, global);
      addFunction("IsString", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsString), 1U, global);
      addFunction("IsArray", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsArray), 1U, global);
      addFunction("IsDictionary", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsDictionary), 1U, global);
      addFunction("IsFunction", std::make_shared<Engine::StandardUnaryFunction>(Engine::IsFunction), 1U, global);
      addFunction("Size", std::make_shared<Engine::StandardUnaryFunction>(Engine::Size), 1U, global);
      addFunction("PopFront", std::make_shared<Engine::StandardUnaryFunction>(Engine::PopFront), 1U, global);
      addFunction("PopBack", std::make_shared<Engine::StandardUnaryFunction>(Engine::PopBack), 1U, global);
      addFunction("GetKeys", std::make_shared<Engine::StandardUnaryFunction>(Engine::GetKeys), 1U, global);
      addFunction("SetRoundMode", std::make_shared<Engine::StandardUnaryFunction>(Engine::SetRoundMode), 1U, global);

    // 6
      addFunction("Error", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::Error), 1U, global);
      addFunction("Warn", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::Warn), 1U, global);
      addFunction("Info", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::Info), 1U, global);
      addFunction("Fatal", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::Fatal), 1U, global);
      addFunction("DebugPrint", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::DebugPrint), 1U, global);
      addFunction("Eval", std::make_shared<Engine::StandardUnaryFunctionWithContext>(Engine::Eval), 1U, global);

    // 12
      addFunction("Atan2", std::make_shared<Engine::StandardBinaryFunction>(Engine::Atan2), 2U, global);
      addFunction("Hypot", std::make_shared<Engine::StandardBinaryFunction>(Engine::Hypot), 2U, global);
      addFunction("Log", std::make_shared<Engine::StandardBinaryFunction>(Engine::Log), 2U, global);
      addFunction("Min", std::make_shared<Engine::StandardBinaryFunction>(Engine::Min), 2U, global);
      addFunction("Max", std::make_shared<Engine::StandardBinaryFunction>(Engine::Max), 2U, global);
      addFunction("GetIndex", std::make_shared<Engine::StandardBinaryFunction>(Engine::GetIndex), 2U, global);
      addFunction("NewArrayDefault", std::make_shared<Engine::StandardBinaryFunction>(Engine::NewArrayDefault), 2U, global);
      addFunction("PushBack", std::make_shared<Engine::StandardBinaryFunction>(Engine::PushBack), 2U, global);
      addFunction("PushFront", std::make_shared<Engine::StandardBinaryFunction>(Engine::PushFront), 2U, global);
      addFunction("ContainsKey", std::make_shared<Engine::StandardBinaryFunction>(Engine::ContainsKey), 2U, global);
      addFunction("RemoveKey", std::make_shared<Engine::StandardBinaryFunction>(Engine::RemoveKey), 2U, global);
      addFunction("GetValue", std::make_shared<Engine::StandardBinaryFunction>(Engine::GetValue), 2U, global);

    // 3
      addFunction("SubString", std::make_shared<Engine::StandardTernaryFunction>(Engine::SubString), 3U, global);
      addFunction("SetIndex", std::make_shared<Engine::StandardTernaryFunction>(Engine::SetIndex), 3U, global);
      addFunction("Insert", std::make_shared<Engine::StandardTernaryFunction>(Engine::Insert), 3U, global);
    }

   static const char* names [4] = { "Automatic_Argument_1", "Automatic_Argument_2", "Automatic_Argument_3", "Automatic_Argument_4" };

   void ContextBuilder::addFunction(const std::string& name, const std::shared_ptr<Engine::Statement>& function, size_t nargs, Engine::Scope& global)
    {
      std::shared_ptr<Engine::FunctionContext> fun = std::make_shared<Engine::FunctionContext>();
      fun->name = name;
      fun->nargs = nargs;
      fun->nlocals = 0;
      fun->function = function;

      global.names.emplace_back(name);
      global.var.emplace(std::make_pair(name, global.vars.size()));
      global.vars.emplace_back(std::make_shared<Types::FunctionValue>(fun, std::vector<std::shared_ptr<Types::ValueType> >()));

      for (size_t arg = 0U; arg < nargs; ++arg)
       {
         fun->args.emplace(std::make_pair(names[arg], arg));
         fun->argNames.emplace_back(names[arg]);
       }
    }

 } // namespace Parser

 } // namespace Backwards
