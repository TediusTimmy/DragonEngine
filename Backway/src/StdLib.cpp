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
#include "Backway/CallingContext.h"
#include "Backway/StdLib.h"

#include "Backway/StateMachine.h"
#include "Backway/Environment.h"
#include "Backwards/Engine/ConstantsSingleton.h"

#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Engine/DebuggerHook.h"

#include "Backwards/Engine/ProgrammingException.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"


#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"


namespace Backway
 {

    /*
      Backway is a finite state machine for doing tasks. It is a stack of queues.
      In this diagram, X is the current state.

        +-+
        |A|
      +-+-+-+-+-+-+-+
      |B|X|C|1|2|3|D|
      +-+-+-+-+-+-+-+
        |E|
        +-+-+-+
        |4|5|6|
        +-+-+-+
        |7|8|
        +-+-+
        |F|
        +-+

      If we add a new state, we can add it in one of several locations.
      Many of these make it not a pure queue of pure stacks.
         A -> Push (pure)
         B -> Precede
         C -> Follow
         D -> Enqueue (pure)
         E -> Inject
         F -> Finally
      If we exit the current state, we can do so to a few places:
         1 -> Leave (pure)
         4 -> Return (pure)
      We can also exit to some "impure" places:
         Skip removes states from the queue until reaching a named state. If the queue is cleared, that is an error.
         Rewind removes queues from the stack until reaching a queue with a named state at its head. If the stack is cleared, that is an error.

      Rules:
         If we Leave and the top queue is now empty, pop the stack and continue on the next state.
         If we Leave or Return and the stack becomes empty, return false.
         If a state changes the next state to execute, run its update immediately.
            This supports Anchor states and Dispatch states:
               Anchor states are named states that Leave immediately to be found by Skip and Rewind.
               Dispatch states are states that "queue" up a collection of states with Follow and then Leave immediately.
    */

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetInput)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (nullptr != text.machine->input.get())
          {
            return text.machine->input;
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().EMPTY_DICTIONARY;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetName)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         return std::make_shared<Backwards::Types::StringValue>(text.topScope()->name);
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Leave)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (false == text.machine->states.empty())
          {
            text.machine->states.back().pop_front();
            if (true == text.machine->states.back().empty())
             {
               text.machine->states.pop_back();
             }
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Rand)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         return std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(text.machine->rng() / 18446744073709551616.0));
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Return)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (false == text.machine->states.empty())
          {
            text.machine->states.pop_back();
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(SetOutput)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         text.machine->result = arg;
         return arg;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Enqueue)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error enqueueing state: no such state.");
             }
            if (true == text.machine->states.empty())
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
             }
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            text.machine->states.back().emplace_back(added);
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            // Double loop : machine state is not modified if an exception is thrown.
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error enqueueing state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error enqueueing collection of names of states: item not a name.");
                }
             }
            if (true == text.machine->states.empty())
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
             }
            for (const auto& item : array)
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
               std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
               text.machine->states.back().emplace_back(added);
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error enqueueing neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Finally)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error making final state: no such state.");
             }
            text.machine->states.emplace_front(std::list<std::shared_ptr<State> >());
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            text.machine->states.front().emplace_back(added);
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            // Double loop : machine state is not modified if an exception is thrown.
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error making final state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error making final collection of names of states: item not a name.");
                }
             }
            text.machine->states.emplace_front(std::list<std::shared_ptr<State> >());
            for (const auto& item : array)
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
               std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
               text.machine->states.front().emplace_back(added);
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error making final neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Follow)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error following state: no such state.");
             }
            if (true == text.machine->states.empty())
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
             }
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            if (false == text.machine->states.back().empty())
             {
               text.machine->states.back().emplace(++text.machine->states.back().begin(), added);
             }
            else // Will only ever be true if the machine was empty and we just added this queue.
             {
               text.machine->states.back().emplace_back(added);
             }
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            // Double loop : machine state is not modified if an exception is thrown.
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error following state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error following collection of names of states: item not a name.");
                }
             }
            if (true == text.machine->states.empty()) // If the machine is empty, then a follow is just an enqueue.
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
               for (const auto& item : array)
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
                  text.machine->states.back().emplace_back(added);
                }
             }
            else
             {
               for (size_t iter = array.size(); iter > 0; --iter) // Looks a little strange, but iterate from size to one so we don't go negative.
                { // And go backwards so that the array is forwards in the queue.
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*array[iter - 1]).value;
                  std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
                  text.machine->states.back().emplace(++text.machine->states.back().begin(), added);
                }
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error following neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Inject)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error injecting state: no such state.");
             }
            std::list<std::shared_ptr<State> >* spot;
            if (false == text.machine->states.empty())
             {
               text.machine->states.emplace(--text.machine->states.end(), std::list<std::shared_ptr<State> >());
               spot = &(*--(--text.machine->states.end())); // Nope, can't do " - 2"
             }
            else
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
               spot = &(text.machine->states.back());
             }
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            spot->emplace_back(added);
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error injecting state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error injecting collection of names of states: item not a name.");
                }
             }
            std::list<std::shared_ptr<State> >* spot;
            if (false == text.machine->states.empty())
             {
               text.machine->states.emplace(--text.machine->states.end(), std::list<std::shared_ptr<State> >());
               spot = &(*--(--text.machine->states.end())); // Nope, can't do " - 2"
             }
            else
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
               spot = &(text.machine->states.back());
             }
            for (const auto& item : array)
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
               std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
               spot->emplace_back(added);
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error injecting neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Precede)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error preceding state: no such state.");
             }
            if (true == text.machine->states.empty())
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
             }
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            text.machine->states.back().emplace_front(added);
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            // Double loop : machine state is not modified if an exception is thrown.
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error preceding state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error preceding collection of names of states: item not a name.");
                }
             }
            if (true == text.machine->states.empty())
             {
               text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
             }
            for (size_t iter = array.size(); iter > 0; --iter) // Looks a little strange, but iterate from size to one so we don't go negative.
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*array[iter - 1]).value;
               std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
               text.machine->states.back().emplace_front(added);
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error preceding neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Push)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            if (text.environment->states.end() == text.environment->states.find(name))
             {
               throw Backwards::Types::TypedOperationException("Error pushing state: no such state.");
             }
            std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
            text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
            text.machine->states.back().emplace_back(added);
          }
         else if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
          {
            const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
            // Double loop : machine state is not modified if an exception is thrown.
            for (const auto& item : array)
             {
               if (typeid(Backwards::Types::StringValue) == typeid(*item))
                {
                  const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
                  if (text.environment->states.end() == text.environment->states.find(name))
                   {
                     throw Backwards::Types::TypedOperationException("Error pushing state: no such state.");
                   }
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error pushing collection of names of states: item not a name.");
                }
             }
            text.machine->states.emplace_back(std::list<std::shared_ptr<State> >());
            for (const auto& item : array)
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*item).value;
               std::shared_ptr<State> added = std::make_shared<State>(*(text.environment->states.find(name)->second));
               text.machine->states.back().emplace_back(added);
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error pushing neither name of state or collection of names of states.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Skip)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            bool found = false;
            for (const auto& item : text.machine->states.back())
             {
               if (name == item->scope.name)
                {
                  found = true;
                  break;
                }
             }
            if (false == found)
             {
               throw Backwards::Types::TypedOperationException("Error skipping to state: no such state.");
             }
            while (name != text.machine->states.back().front()->scope.name)
             {
               text.machine->states.back().pop_front();
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error skipping to not the name of state.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(Unwind)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            bool found = false;
            for (const auto& item : text.machine->states)
             {
               if (name == item.front()->scope.name)
                {
                  found = true;
                  break;
                }
             }
            if (false == found)
             {
               throw Backwards::Types::TypedOperationException("Error unwinding to state: no such state.");
             }
            while (name != text.machine->states.back().front()->scope.name)
             {
               text.machine->states.pop_back();
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error unwinding to not the name of state.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(CreateState)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*first))
          {
            if (typeid(Backwards::Types::StringValue) == typeid(*second))
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*first).value;
               const std::string& functions = static_cast<const Backwards::Types::StringValue&>(*second).value;
               bool overwritten = false;
               if (text.environment->states.end() != text.environment->states.find(name))
                {
                  overwritten = true;
                }

               std::shared_ptr<State> newState = std::make_shared<State>();
               newState->scope.name = name;

               Backwards::Input::StringInput string (functions);
               Backwards::Input::Lexer lexer (string, "CreateState Functions Argument");

               Backwards::Parser::GetterSetter gs;
               Backwards::Parser::SymbolTable table (gs, *context.globalScope);
               table.pushScope(&newState->scope);

               std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseFunctions(lexer, table, *context.logger);

               if (nullptr != res.get())
                {
                  context.pushScope(&newState->scope);
                  try
                   {
                     std::shared_ptr<Backwards::Engine::FlowControl> result = res->execute(context);
                     if (nullptr != result)
                      {
                        throw Backwards::Engine::ProgrammingException("Result was not null in CreateState.");
                      }

                     if (newState->scope.var.end() == newState->scope.var.find("Update"))
                      {
                        throw Backwards::Types::TypedOperationException("Error creating state: no Update function.");
                      }
                     newState->updateFun = std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), table.getVariableGetter("Update"));

                     std::map<std::string, std::shared_ptr<State> >::iterator iter = text.environment->states.find(name);
                     if (text.environment->states.end() != iter)
                        iter->second = newState;
                     else
                        text.environment->states.emplace(name, newState);
                   }
                  catch (...)
                   {
                     context.popScope();
                     throw;
                   }
                  context.popScope();
                }
               else
                {
                  throw Backwards::Types::TypedOperationException("Error creating state: could not parse functions.");
                }

               return (true == overwritten) ? Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ZERO :
                  Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
             }
            else
             {
               throw Backwards::Types::TypedOperationException("Error creating state: functions not String.");
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error creating state: name not String.");
          }
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Backway Context.");
       }
    }

   StandardBinaryFunctionWithContext::StandardBinaryFunctionWithContext(BinaryFunctionPointerWithContext function) : Backwards::Engine::Statement(Backwards::Input::Token()), function(function)
    {
    }

   std::shared_ptr<Backwards::Engine::FlowControl> StandardBinaryFunctionWithContext::execute (Backwards::Engine::CallingContext& context) const
    {
      std::shared_ptr<Backwards::Types::ValueType> lhs = context.currentFrame->args[0U];
      std::shared_ptr<Backwards::Types::ValueType> rhs = context.currentFrame->args[1U];
      try
       {
         return std::make_shared<Backwards::Engine::FlowControl>(token, Backwards::Engine::FlowControl::RETURN, Backwards::Engine::FlowControl::NO_TARGET, function(context, lhs, rhs));
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }

 } // namespace Backway
