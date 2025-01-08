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
#include "gtest/gtest.h"

#include "Dragon/CallingContext.h"
#include "Dragon/Commands.h"
#include "Dragon/ContextBuilder.h"
#include "Dragon/Environment.h"
#include "Dragon/StdLib.h"
#include "Dragon/Universe.h"

#include "Backway/Environment.h"
#include "Backway/StateMachine.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/ProgrammingException.h"

class ConsoleLogger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message) { std::cout << message << std::endl; }
   std::string get () { std::string result; std::getline(std::cin, result); return result; }
 };

class DummyDebugger final : public Backwards::Engine::DebuggerHook
 {
public:
   bool entered;
   virtual void EnterDebugger(const std::string&, Backwards::Engine::CallingContext&) { entered = true; }
 };

TEST(DragonTests, testDuplicate)
 {
   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;
   Dragon::Environment viron;
   context.viron = &viron;
   Backway::StateMachine machine ("machina");
   context.machine = &machine;
   Backway::Environment environment;
   context.environment = &environment;
   machine.environment = &environment;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Dragon::ContextBuilder::createGlobalScope(global);

   std::shared_ptr<Backwards::Engine::CallingContext> result = context.duplicate();
   std::shared_ptr<Dragon::CallingContext> res = std::dynamic_pointer_cast<Dragon::CallingContext>(result);

   ASSERT_NE(nullptr, res.get());
   EXPECT_EQ(&logger, res->logger);
   EXPECT_EQ(&global, res->globalScope);
   EXPECT_EQ(nullptr, res->topScope());
   EXPECT_EQ(nullptr, res->debugger);
   EXPECT_EQ(&machine, res->machine);
   EXPECT_EQ(&environment, res->environment);
   EXPECT_EQ(&universe, res->universe);
   EXPECT_EQ(&viron, res->viron);
 }

TEST(DragonTests, testFunctionsWithoutContext)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   Backwards::Engine::CallingContext badContext;
   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;

   res = Dragon::ClearDynamicLayer(context);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));

   EXPECT_THROW(Dragon::ClearDynamicLayer(badContext), Backwards::Engine::ProgrammingException);

   res = Dragon::ClearPlayerLayer(context);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));

   EXPECT_THROW(Dragon::ClearPlayerLayer(badContext), Backwards::Engine::ProgrammingException);

   Backway::StateMachine machine ("machine1");
   context.machine = &machine;

   res = Dragon::GetMyName(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));

   EXPECT_THROW(Dragon::GetMyName(badContext), Backwards::Engine::ProgrammingException);

   universe.screenxy = res;

   res = Dragon::GetScreenXY(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));

   EXPECT_THROW(Dragon::GetScreenXY(badContext), Backwards::Engine::ProgrammingException);
 }

TEST(DragonTests, testEnvironmentExceptions)
 {
   Dragon::CallingContext context;
   Dragon::Environment viron;

   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Dragon::ContextBuilder::createGlobalScope(global);

   EXPECT_THROW(viron.loadEntity(context, "one", "{ ;; }"), Backwards::Types::TypedOperationException);
   EXPECT_THROW(viron.loadEntity(context, "two", "{ 'hello'; 'world' }"), Backwards::Types::TypedOperationException);
   EXPECT_THROW(viron.loadEntity(context, "three", "2 + 'hello'"), Backwards::Types::TypedOperationException);
   EXPECT_THROW(viron.loadEntity(context, "four", "3 + Fatal('you')"), Backwards::Types::TypedOperationException);
 }

TEST(DragonTests, testEntityFunctions)
 {
   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;
   Dragon::Environment viron;
   context.viron = &viron;

   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Dragon::ContextBuilder::createGlobalScope(global);

   viron.loadEntity(context, "Larry", "{ 'Start' : 'set Update to function update (arg) is return arg end' }");
   viron.loadEntity(context, "Moe", "{ 'Start' : 'set Update to function update (arg) is return arg end' }");
   viron.loadEntity(context, "Curly", "{ 'Start' : 'set Update to function update (arg) is return arg end' }");

   std::shared_ptr<Backwards::Types::ValueType> res;

   res = Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));

   res = Dragon::CreateDynamicEntity(context, std::make_shared<Backwards::Types::StringValue>("Moe"), std::make_shared<Backwards::Types::StringValue>("Start"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));

   res = Dragon::CreatePlayerEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Start"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));


   EXPECT_TRUE(universe.update(context)); // Push the button, Max


   res = Dragon::SendMessage(context, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start"));
   ASSERT_TRUE(typeid(Backwards::Types::DictionaryValue) == typeid(*res.get()));


   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Larry"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(1.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Moe"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(1.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(1.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

      // I'm going to have this return zero, but it should throw.
   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Zippy"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(0.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

      // Curly should still be present.
   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(1.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);


   EXPECT_FALSE(universe.update(context)); // Now remove the entities.

      // Now Curly should be gone.
   res = Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(SlowFloat::SlowFloat(0.0), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
 }

TEST(DragonTests, testManyExceptions)
 {
   Backwards::Engine::CallingContext badContext;

   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;
   Dragon::Environment viron;
   context.viron = &viron;

   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Dragon::ContextBuilder::createGlobalScope(global);

   viron.loadEntity(context, "Larry", "{ 'Start' : 'set Update to function update (arg) is return arg end' }");
   viron.loadEntity(context, "Curly", "{ 'Start' : 'set Update to function update (arg) is return 2 + update end' }");
   viron.loadEntity(context, "Gene", "{ 'Start' : 'set Update to function update (arg) is return Fatal(GetInput()) end' }");

   EXPECT_THROW(Dragon::RemoveEntity(context, std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(1.0))), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::RemoveEntity(badContext, std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Engine::ProgrammingException);

   EXPECT_THROW(Dragon::CreateBackgroundEntity(badContext, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start")), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Dragon::CreateDynamicEntity(badContext, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start")), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Dragon::CreatePlayerEntity(badContext, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start")), Backwards::Engine::ProgrammingException);

   EXPECT_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Moe"), std::make_shared<Backwards::Types::StringValue>("Start")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(1.0)), std::make_shared<Backwards::Types::StringValue>("Start")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(1.0))), Backwards::Types::TypedOperationException);

      // I can create a bad entity.
   EXPECT_NO_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Start")));
   EXPECT_NO_THROW(Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene"), std::make_shared<Backwards::Types::StringValue>("Start")));

   EXPECT_THROW(Dragon::SendMessage(badContext, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Dragon::SendMessage(context, std::make_shared<Backwards::Types::StringValue>("Moe"), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::SendMessage(context, std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(1.0)), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::SendMessage(context, std::make_shared<Backwards::Types::StringValue>("Moe"), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::SendMessage(context, std::make_shared<Backwards::Types::StringValue>("Gene"), std::make_shared<Backwards::Types::StringValue>("Go")), Backwards::Engine::FatalException);

   EXPECT_FALSE(universe.update(context)); // Bad entities should get removed.
   EXPECT_TRUE(universe.background.empty());

   EXPECT_THROW(Dragon::SetScreenXY(badContext, std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Dragon::SetScreenXY(context, std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::DrawSprite(badContext, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Dragon::DrawSprite(context, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Dragon::DrawSprite(context, std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(1.0)), std::make_shared<Backwards::Types::StringValue>("Curly")), Backwards::Types::TypedOperationException);
 }

TEST(DragonTests, testOutputskies)
 {
   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;
   Dragon::Environment viron;
   context.viron = &viron;

   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Dragon::ContextBuilder::createGlobalScope(global);

   viron.loadEntity(context, "Larry", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({0; 0; 0; 0}) return arg end' }");
   viron.loadEntity(context, "Moe", "{ 'Start' : 'set Update to function update (arg) is call DrawSprite(\"You\"; {0; 0; 0; 0}) return arg end' }");
   viron.loadEntity(context, "Curly", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({1; 1; 1; 1}) + DrawSprite(\"Me\"; {0; 0; 0; 0}) return arg end' }");

   viron.loadEntity(context, "Gene0", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({0; 0; 0}) return arg end' }");
   viron.loadEntity(context, "Gene1", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({arg; 0; 0; 0}) return arg end' }");
   viron.loadEntity(context, "Gene2", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({0; arg; 0; 0}) return arg end' }");
   viron.loadEntity(context, "Gene3", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({0; 0; arg; 0}) return arg end' }");
   viron.loadEntity(context, "Gene4", "{ 'Start' : 'set Update to function update (arg) is call SetScreenXY({0; 0; 0; arg}) return arg end' }");
   
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Larry"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Curly"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Moe"), std::make_shared<Backwards::Types::StringValue>("Start"));

   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene0"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene1"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene2"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene3"), std::make_shared<Backwards::Types::StringValue>("Start"));
   Dragon::CreateBackgroundEntity(context, std::make_shared<Backwards::Types::StringValue>("Gene4"), std::make_shared<Backwards::Types::StringValue>("Start"));

   EXPECT_TRUE(universe.update(context));

   ASSERT_NE(nullptr, universe.outputList.get());
   EXPECT_EQ(typeid(Dragon::Command_Move), typeid(*universe.outputList.get()));
   EXPECT_EQ(0, static_cast<Dragon::Command_Move*>(universe.outputList.get())->x);
   ASSERT_NE(nullptr, universe.outputList->next.get());
   EXPECT_EQ(typeid(Dragon::Command_Move), typeid(*universe.outputList->next.get()));
   EXPECT_EQ(1, static_cast<Dragon::Command_Move*>(universe.outputList->next.get())->x);
   ASSERT_NE(nullptr, universe.outputList->next->next.get());
   EXPECT_EQ(typeid(Dragon::Command_Draw), typeid(*universe.outputList->next->next.get()));
   EXPECT_EQ("Me", static_cast<Dragon::Command_Draw*>(universe.outputList->next->next.get())->resource);
   ASSERT_NE(nullptr, universe.outputList->next->next->next.get());
   EXPECT_EQ(typeid(Dragon::Command_Draw), typeid(*universe.outputList->next->next->next.get()));
   EXPECT_EQ("You", static_cast<Dragon::Command_Draw*>(universe.outputList->next->next->next.get())->resource);
   EXPECT_EQ(nullptr, universe.outputList->next->next->next->next.get());
 }
