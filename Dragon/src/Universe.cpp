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
#include "Dragon/Universe.h"
#include "Dragon/Environment.h"

#include "Dragon/CallingContext.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Types/ValueType.h"


#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"
#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backway/Environment.h"
#include "Backway/StdLib.h"

#include <set>

namespace Dragon
 {

   static void processSingleList(CallingContext& context, std::list<std::shared_ptr<Backway::StateMachine> >& list,
      const std::shared_ptr<Backwards::Types::ValueType>& input, std::shared_ptr<Backway::Command>& output)
    {
      std::set<std::shared_ptr<Backway::StateMachine> > entitiesToKill;
      for (const std::shared_ptr<Backway::StateMachine>& entity : list)
       {
         entity->input = input;
         bool result = false;
         try
          {
            context.machine = entity.get();
            context.environment = context.machine->environment;
            result = entity->update(context);
          }
         catch (const Backwards::Types::TypedOperationException&)
          {
            // This entity is dead.
          }
         catch (const Backwards::Engine::FatalException&)
          {
            // This entity is dead.
          }
         if (nullptr != entity->output.get())
          {
            if (nullptr == output.get())
             {
               output = entity->output;
             }
            else
             {
               output->last->next = entity->output;
               output->last = entity->output->last;
             }
            entity->output = std::shared_ptr<Backway::Command>();
          }
         if (false == result)
          {
            entitiesToKill.insert(entity);
          }
       }
      for (const std::shared_ptr<Backway::StateMachine>& entity : entitiesToKill)
       {
         list.remove(entity);
       }
    }

   bool Universe::update(CallingContext& context)
    {
      outputList = std::shared_ptr<Backway::Command>();
      processSingleList(context, background, input, outputList);
      processSingleList(context, dynamic, input, outputList);
      processSingleList(context, player, input, outputList);
      return false == background.empty();
    }

   void Environment::loadEntity(CallingContext& context, const std::string& name, const std::string& states)
    {
      std::shared_ptr<Backway::StateMachine> machina = std::make_shared<Backway::StateMachine>(name);
      std::shared_ptr<Backway::Environment> env = std::make_shared<Backway::Environment>();

      machina->environment = env.get(); // Set this pointer here for use later.

      context.machine = machina.get();
      context.environment = env.get();

      Backwards::Input::StringInput string (states);
      Backwards::Input::Lexer lexer (string, name);

      Backwards::Parser::GetterSetter gs;
      Backwards::Parser::SymbolTable table (gs, *context.globalScope);

      std::shared_ptr<Backwards::Engine::Expression> res = Backwards::Parser::Parser::ParseExpression(lexer, table, *context.logger);

      if (nullptr != res.get())
       {

         try
          {
            std::shared_ptr<Backwards::Types::ValueType> val = res->evaluate(context);

            if (typeid(Backwards::Types::DictionaryValue) == typeid(*val))
             {
               for (std::map<std::shared_ptr<Backwards::Types::ValueType>, std::shared_ptr<Backwards::Types::ValueType>, Backwards::Types::ChristHowHorrifying>::const_iterator iter =
                  static_cast<const Backwards::Types::DictionaryValue&>(*val).value.begin();
                  static_cast<const Backwards::Types::DictionaryValue&>(*val).value.end() != iter; ++iter)
                {
                  Backway::CreateState(context, iter->first, iter->second);
                }
             }
            else
             {
               throw Backwards::Types::TypedOperationException("Load failed: loaded value was not a Dictionary.");
             }
          }
         catch (const Backwards::Types::TypedOperationException& e)
          {
            throw Backwards::Types::TypedOperationException(std::string("Load failed: ") + e.what());
          }
         catch (const Backwards::Engine::FatalException& e)
          {
            throw Backwards::Types::TypedOperationException(std::string("Load failed: ") + e.what());
          }
       }
      else
       {
         throw Backwards::Types::TypedOperationException("Load failed: parse failed.");
       }

      entities.emplace(std::make_pair(name, machina));
      environments.emplace(std::make_pair(name, env));
    }

 } // namespace Dragon
