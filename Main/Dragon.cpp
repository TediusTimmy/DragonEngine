/*
BSD 3-Clause License

Copyright (c) 2025, Thomas DiModica
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
#include "Settings.h"

#include <string>
#include <fstream>
#include <iterator>

#include "Dragon/CallingContext.h"
#include "Dragon/ContextBuilder.h"
#include "Dragon/Environment.h"
#include "Dragon/StdLib.h"

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Parser/ContextBuilder.h"

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/Statement.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/DictionaryValue.h"


namespace Main
 {

static std::string getString(const std::shared_ptr<Backwards::Types::ValueType>& from)
 {
   std::string result;
   if (typeid(Backwards::Types::StringValue) == typeid(*from.get()))
    {
      result = static_cast<Backwards::Types::StringValue*>(from.get())->value;
    }
   return result;
 }

static int getInt(const std::shared_ptr<Backwards::Types::ValueType>& from)
 {
   int result = -1;
   if (typeid(Backwards::Types::FloatValue) == typeid(*from.get()))
    {
      result = static_cast<int>(static_cast<double>(static_cast<Backwards::Types::FloatValue*>(from.get())->value));
    }
   return result;
 }

static void populateMap(std::map<std::string, std::string>& dest, const std::shared_ptr<Backwards::Types::ValueType>& src)
 {
   if (typeid(Backwards::Types::DictionaryValue) == typeid(*src.get()))
    {
      for (std::map<std::shared_ptr<Backwards::Types::ValueType>, std::shared_ptr<Backwards::Types::ValueType>, Backwards::Types::ChristHowHorrifying>::const_iterator iter =
         static_cast<const Backwards::Types::DictionaryValue&>(*src).value.begin();
         static_cast<const Backwards::Types::DictionaryValue&>(*src).value.end() != iter; ++iter)
       {
         dest[getString(iter->first)] = getString(iter->second);
       }
    }
 }

void LoadSettings (const std::string& fileName, Settings& settings, Backwards::Engine::Logger* logger)
 {
   std::ifstream file (fileName.c_str(), std::ios_base::in);
   std::string settingFile (std::istreambuf_iterator<char>{file}, {});

   Backwards::Input::StringInput string (settingFile);
   Backwards::Input::Lexer lexer (string, fileName);

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;

   context.logger = logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Expression> res = Backwards::Parser::Parser::ParseFullExpression(lexer, table, *logger);

   if (nullptr != res.get())
    {
      try
       {
         std::shared_ptr<Backwards::Types::ValueType> val = res->evaluate(context);

         if (nullptr != val.get())
          {
            if (typeid(Backwards::Types::DictionaryValue) == typeid(*val))
             {
               for (std::map<std::shared_ptr<Backwards::Types::ValueType>, std::shared_ptr<Backwards::Types::ValueType>, Backwards::Types::ChristHowHorrifying>::const_iterator iter =
                  static_cast<const Backwards::Types::DictionaryValue&>(*val).value.begin();
                  static_cast<const Backwards::Types::DictionaryValue&>(*val).value.end() != iter; ++iter)
                {
                  std::string key = getString(iter->first);
                  if ("screen_x" == key)
                   {
                     settings.sx = getInt(iter->second);
                   }
                  else if ("screen_y" == key)
                   {
                     settings.sy = getInt(iter->second);
                   }
                  else if ("sprite" == key)
                   {
                     settings.sprite = getInt(iter->second);
                   }
                  else if ("scale" == key)
                   {
                     settings.scale = getInt(iter->second);
                   }
                  else if ("fps" == key)
                   {
                     settings.fps = getInt(iter->second);
                   }
                  else if ("entities" == key)
                   {
                     populateMap(settings.entities, iter->second);
                   }
                  else if ("sprites" == key)
                   {
                     populateMap(settings.sprites, iter->second);
                   }
                  else if ("songs" == key)
                   {
                     populateMap(settings.songs, iter->second);
                   }
                  else if ("sounds" == key)
                   {
                     populateMap(settings.sounds, iter->second);
                   }
                  else if ("initial_entity" == key)
                   {
                     settings.entity = getString(iter->second);
                   }
                  else if ("initial_state" == key)
                   {
                     settings.state = getString(iter->second);
                   }
                }
             }
            else
             {
               logger->log("Loading settings failed: loaded value was not a Dictionary.");
             }
          }
         else
          {
            logger->log("Loading settings failed: evaluate failed.");
          }
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
         logger->log(std::string("Loading settings failed: ") + e.what());
       }
      catch (const Backwards::Engine::FatalException& e)
       {
         logger->log(std::string("Loading settings failed: ") + e.what());
       }
    }
   else
    {
      logger->log("Loading settings failed: parse failed.");
    }
 }

void LoadEntities (Dragon::CallingContext& context, const std::map<std::string, std::string>& entities, const std::string& initialEntity, const std::string& initialState)
 {
   Dragon::ContextBuilder::createGlobalScope(*context.globalScope);
   try
    {
      for (const auto& entity : entities)
       {
         std::ifstream file (entity.second.c_str(), std::ios_base::in);
         std::string states (std::istreambuf_iterator<char>{file}, {});
         context.viron->loadEntity(context, entity.first, states);
       }

      Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>(initialEntity), std::make_shared<Backwards::Types::StringValue>(initialState));
    }
   catch (const Backwards::Types::TypedOperationException& e)
    {
      context.logger->log(std::string("Load failed: ") + e.what());
    }
 }

 } // namespace Main
