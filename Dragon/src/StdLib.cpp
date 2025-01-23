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
#include "Dragon/CallingContext.h"
#include "Dragon/StdLib.h"

#include "Dragon/Commands.h"
#include "Dragon/Environment.h"
#include "Dragon/Universe.h"
#include "Backwards/Engine/ConstantsSingleton.h"

#include "Backwards/Engine/ProgrammingException.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"

#include "Backway/Environment.h"


namespace Dragon
 {

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(ClearDynamicLayer)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         text.universe->dynamic.clear();
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(ClearPlayerLayer)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         text.universe->player.clear();
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetMyName)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         return std::make_shared<Backwards::Types::StringValue>(text.machine->name);
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetScreenXY)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         return text.universe->screenxy;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }


   static std::shared_ptr<Backway::StateMachine> findFirstEntity (const std::string& name, CallingContext& text)
    {
      for (const std::shared_ptr<Backway::StateMachine>& entity : text.universe->background)
       {
         if (name == entity->name)
          {
            return entity;
          }
       }
      for (const std::shared_ptr<Backway::StateMachine>& entity : text.universe->dynamic)
       {
         if (name == entity->name)
          {
            return entity;
          }
       }
      for (const std::shared_ptr<Backway::StateMachine>& entity : text.universe->player)
       {
         if (name == entity->name)
          {
            return entity;
          }
       }
      return std::shared_ptr<Backway::StateMachine>();
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(RemoveEntity)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            std::shared_ptr<Backway::StateMachine> entity = findFirstEntity(name, text);
            if (nullptr != entity.get())
             {
               // Remove an entity by blowing away its state list so that it can be garbage collected later.
               entity->states.clear();
               return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
             }
            return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ZERO;
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error removing entity: name is not a string.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   static std::shared_ptr<Command_Move> convertToCoordinates (const std::shared_ptr<Backwards::Types::ValueType>& arg)
    {
      if (typeid(Backwards::Types::ArrayValue) == typeid(*arg))
       {
         const std::vector<std::shared_ptr<Backwards::Types::ValueType> >& array = static_cast<const Backwards::Types::ArrayValue&>(*arg).value;
         if (array.size() < 4U)
          {
            throw Backwards::Types::TypedOperationException("Position array too small.");
          }
         if ( (typeid(Backwards::Types::FloatValue) != typeid(*array[0])) ||
              (typeid(Backwards::Types::FloatValue) != typeid(*array[1])) ||
              (typeid(Backwards::Types::FloatValue) != typeid(*array[2])) ||
              (typeid(Backwards::Types::FloatValue) != typeid(*array[3])) )
          {
            throw Backwards::Types::TypedOperationException("Position elements not numeric.");
          }
         int x = static_cast<double>(static_cast<const Backwards::Types::FloatValue&>(*array[0]).value);
         int y = static_cast<double>(static_cast<const Backwards::Types::FloatValue&>(*array[1]).value);
         int sx = static_cast<double>(static_cast<const Backwards::Types::FloatValue&>(*array[2]).value);
         int sy = static_cast<double>(static_cast<const Backwards::Types::FloatValue&>(*array[3]).value);
         return std::make_shared<Command_Move>(x, y, sx, sy);
       }
      else
       {
         throw Backwards::Types::TypedOperationException("Position array was not an array.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(SetScreenXY)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         text.machine->addOutput(convertToCoordinates(arg));
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(PlayMusic)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            text.machine->addOutput(std::make_shared<Command_PlayMusic>(name));
            return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error playing music: song name is not a string.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(PlaySound)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*arg))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*arg).value;
            text.machine->addOutput(std::make_shared<Command_PlaySound>(name));
            return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error playing sound: effect name is not a string.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }


   static std::shared_ptr<Backway::StateMachine> makeEntity (CallingContext& text,
      const std::shared_ptr<Backwards::Types::ValueType>& first, const std::shared_ptr<Backwards::Types::ValueType>& second)
    {
      if (typeid(Backwards::Types::StringValue) == typeid(*first))
       {
         const std::string& name = static_cast<const Backwards::Types::StringValue&>(*first).value;

         if (text.viron->entities.end() == text.viron->entities.find(name))
          {
            throw Backwards::Types::TypedOperationException("Error creating entity: entity not found.");
          }
         std::shared_ptr<Backway::StateMachine> result = std::make_shared<Backway::StateMachine>(name);
         result->environment = text.viron->entities[name]->environment;

         if (typeid(Backwards::Types::StringValue) == typeid(*second))
          {
            const std::string& state = static_cast<const Backwards::Types::StringValue&>(*second).value;
            // I could twiddle with Context and call Enqueue (or some other) directly
            if (result->environment->states.end() == result->environment->states.find(state))
             {
               throw Backwards::Types::TypedOperationException("Error creating entity: no such initial state.");
             }
            result->states.emplace_back(std::list<std::shared_ptr<Backway::State> >());
            std::shared_ptr<Backway::State> added = std::make_shared<Backway::State>(*(result->environment->states.find(state)->second));
            result->states.back().emplace_back(added);
            // Final step : set the arg to Update to empty dictionary or bad things happen.
            result->last = Backwards::Engine::ConstantsSingleton::getInstance().EMPTY_DICTIONARY;
            return result;
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error creating entity: initial state is not a string.");
          }
       }
      else
       {
         throw Backwards::Types::TypedOperationException("Error creating entity: name is not a string.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(CreateBackgroundEntity)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         std::shared_ptr<Backway::StateMachine> newEntity = makeEntity(text, first, second);
         text.universe->background.push_back(newEntity);
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(CreateDynamicEntity)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         std::shared_ptr<Backway::StateMachine> newEntity = makeEntity(text, first, second);
         text.universe->dynamic.push_back(newEntity);
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(CreatePlayerEntity)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         std::shared_ptr<Backway::StateMachine> newEntity = makeEntity(text, first, second);
         text.universe->player.push_back(newEntity);
         return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(DrawSprite)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*first))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*first).value;
            std::shared_ptr<Command_Move> coords = convertToCoordinates(second);
            text.machine->addOutput(std::make_shared<Command_Draw>(name, coords->x, coords->y, coords->sx, coords->sy));
            return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error drawing sprite: resource is not a string.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

   STDLIB_BINARY_DECL_WITH_CONTEXT(SendMessage)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*first))
          {
            const std::string& name = static_cast<const Backwards::Types::StringValue&>(*first).value;
            std::shared_ptr<Backway::StateMachine> entity = findFirstEntity(name, text);
            if (nullptr != entity.get())
             {
               entity->input = second;
               Backway::StateMachine* saveMachine = text.machine;
               Backway::Environment* saveEnvironment = text.environment;
               try
                {
                  text.machine = entity.get();
                  text.environment = text.machine->environment;
                  entity->update(text);
                  text.machine = saveMachine;
                  text.environment = saveEnvironment;
                }
               catch (...)
                {
                  text.machine = saveMachine;
                  text.environment = saveEnvironment;
                  throw;
                }
               return entity->result;
             }
            else
             {
               throw Backwards::Types::TypedOperationException("Error sending message: no such entity.");
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error sending message: entity name is not a string.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards Context wasn't a Dragon Context.");
       }
    }

 } // namespace Dragon
