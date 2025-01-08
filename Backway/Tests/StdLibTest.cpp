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
#include "gtest/gtest.h"

#include "Backway/StdLib.h"
#include "Backway/CallingContext.h"
#include "Backway/StateMachine.h"
#include "Backway/State.h"
#include "Backway/Environment.h"
#include "Backway/ContextBuilder.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Engine/ProgrammingException.h"
#include "Backwards/Engine/Scope.h"
#include "Backwards/Engine/ConstantsSingleton.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/DebuggerHook.h"

class ConsoleLogger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message) { std::cout << message << std::endl; }
   std::string get () { std::string result; std::getline(std::cin, result); return result; }
 };

TEST(BackwayTests, testFunctionsWithContext)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   Backwards::Engine::CallingContext badContext;
   Backway::CallingContext context;
   Backway::StateMachine machine ("machine1");
   context.machine = &machine;

   Backwards::Engine::Scope scape;
   scape.name = "Bathys";

   res = Backway::Rand(context);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));

   EXPECT_THROW(Backway::Rand(badContext), Backwards::Engine::ProgrammingException);


   context.pushScope(&scape);

   res = Backway::GetName(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("Bathys", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   EXPECT_THROW(Backway::GetName(badContext), Backwards::Engine::ProgrammingException);


   res = Backway::GetInput(context);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().EMPTY_DICTIONARY.get(), res.get());

   machine.input = std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(6.0));
   res = Backway::GetInput(context);
   EXPECT_EQ(machine.input.get(), res.get());

   EXPECT_THROW(Backway::GetInput(badContext), Backwards::Engine::ProgrammingException);


   EXPECT_EQ(nullptr, machine.result.get());
   res = Backway::SetOutput(context, machine.input);
   EXPECT_EQ(machine.input.get(), res.get());
   EXPECT_EQ(machine.input.get(), machine.result.get());

   EXPECT_THROW(Backway::SetOutput(badContext, machine.input), Backwards::Engine::ProgrammingException);
 }

bool checkState(const std::vector<std::vector<std::string> >& expected, const std::list<std::list<std::shared_ptr<Backway::State> > >& actual)
 {
   if (expected.size() != actual.size())
    {
      return false;
    }
   size_t i = 0U;
   for (const auto& queue : actual)
    {
      if (expected[i].size() != queue.size())
       {
         return false;
       }
      size_t j = 0U;
      for (const auto& item : queue)
       {
         if (expected[i][j] != item->scope.name)
          {
            return false;
          }
         ++j;
       }
      ++i;
    }
   return true;
 }

TEST(BackwayTests, testMostExceptions)
 {
   std::shared_ptr<Backwards::Types::FloatValue> number = std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(6.0));
   std::shared_ptr<Backwards::Types::StringValue> nosuch = std::make_shared<Backwards::Types::StringValue>("H");
   std::shared_ptr<Backwards::Types::ArrayValue> arrayNumber = std::make_shared<Backwards::Types::ArrayValue>();
   arrayNumber->value.push_back(number);
   std::shared_ptr<Backwards::Types::ArrayValue> arrayInvalid = std::make_shared<Backwards::Types::ArrayValue>();
   arrayInvalid->value.push_back(std::make_shared<Backwards::Types::StringValue>("A"));
   arrayInvalid->value.push_back(nosuch);

   Backwards::Engine::CallingContext badContext;
   Backway::CallingContext context;
   Backway::StateMachine machine ("machine2");
   context.machine = &machine;
   Backway::Environment environment;
   context.environment = &environment;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;

   std::vector<std::vector<std::string> > expected = { { "A", "B", "C" }, { "1", "2", "3" } };

   std::shared_ptr<Backway::State> s_A = std::make_shared<Backway::State>();
   s_A->scope.name = "A";
   std::shared_ptr<Backway::State> s_B = std::make_shared<Backway::State>();
   s_B->scope.name = "B";
   std::shared_ptr<Backway::State> s_C = std::make_shared<Backway::State>();
   s_C->scope.name = "C";
   std::shared_ptr<Backway::State> s_1 = std::make_shared<Backway::State>();
   s_1->scope.name = "1";
   std::shared_ptr<Backway::State> s_2 = std::make_shared<Backway::State>();
   s_2->scope.name = "2";
   std::shared_ptr<Backway::State> s_3 = std::make_shared<Backway::State>();
   s_3->scope.name = "3";

   environment.states.emplace("A", s_A);

   machine.states.emplace_back(std::list<std::shared_ptr<Backway::State> >{s_A, s_B, s_C}); // Now this is podracing!
   machine.states.emplace_back(std::list<std::shared_ptr<Backway::State> >{s_1, s_2, s_3});

   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Leave(badContext), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Return(badContext), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Enqueue(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Enqueue(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Enqueue(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Enqueue(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Enqueue(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Finally(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Finally(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Finally(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Finally(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Finally(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Follow(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Follow(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Follow(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Follow(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Follow(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Inject(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Inject(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Inject(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Inject(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Inject(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Precede(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Precede(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Precede(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Precede(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Precede(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Push(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Push(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Push(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Push(context, arrayNumber), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Push(context, arrayInvalid), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Skip(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Skip(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Skip(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::Unwind(badContext, number), Backwards::Engine::ProgrammingException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Unwind(context, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));
   EXPECT_THROW(Backway::Unwind(context, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(checkState(expected, machine.states));

   EXPECT_THROW(Backway::CreateState(badContext, number, number), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(Backway::CreateState(context, number, number), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backway::CreateState(context, nosuch, number), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(environment.states.end() == environment.states.find("H"));
   EXPECT_THROW(Backway::CreateState(context, nosuch, nosuch), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(environment.states.end() == environment.states.find("H"));
   EXPECT_EQ(nullptr, context.topScope());
   EXPECT_THROW(Backway::CreateState(context, nosuch, std::make_shared<Backwards::Types::StringValue>("set Y to 1.0")), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(environment.states.end() == environment.states.find("H"));
   EXPECT_EQ(nullptr, context.topScope());
 }

TEST(BackwayTests, testEmptyMachine)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   std::shared_ptr<Backwards::Types::StringValue> state = std::make_shared<Backwards::Types::StringValue>("A");
   std::shared_ptr<Backwards::Types::ArrayValue> array = std::make_shared<Backwards::Types::ArrayValue>();
   array->value.push_back(state);
   array->value.push_back(std::make_shared<Backwards::Types::StringValue>("B"));

   Backway::CallingContext context;
   Backway::StateMachine machine ("machine3");
   context.machine = &machine;
   Backway::Environment environment;
   context.environment = &environment;

   std::vector<std::vector<std::string> > expected0;
   std::vector<std::vector<std::string> > expected1 = { { "A" } };
   std::vector<std::vector<std::string> > expected2 = { { "A", "B" } };

   std::shared_ptr<Backway::State> s_A = std::make_shared<Backway::State>();
   s_A->scope.name = "A";
   std::shared_ptr<Backway::State> s_B = std::make_shared<Backway::State>();
   s_B->scope.name = "B";

   environment.states.emplace("A", s_A);
   environment.states.emplace("B", s_B);

   machine.states.clear();
   res = Backway::Leave(context);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected0, machine.states));

   machine.states.clear();
   res = Backway::Return(context);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected0, machine.states));

   machine.states.clear();
   res = Backway::Enqueue(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Enqueue(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));

   machine.states.clear();
   res = Backway::Finally(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Finally(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));

   machine.states.clear();
   res = Backway::Follow(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Follow(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));

   machine.states.clear();
   res = Backway::Inject(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Inject(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));

   machine.states.clear();
   res = Backway::Precede(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Precede(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));

   machine.states.clear();
   res = Backway::Push(context, state);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected1, machine.states));

   machine.states.clear();
   res = Backway::Push(context, array);
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(), res.get());
   EXPECT_TRUE(checkState(expected2, machine.states));
 }

class DummyDebugger final : public Backwards::Engine::DebuggerHook
 {
public:
   bool entered;
   virtual void EnterDebugger(const std::string&, Backwards::Engine::CallingContext&) { entered = true; }
 };

TEST(BackwayTests, testGoldenPath)
 {
   Backway::CallingContext context;
   Backway::StateMachine machine ("machine4");
   context.machine = &machine;
   Backway::Environment environment;
   context.environment = &environment;
   Backwards::Engine::Scope global;
   context.globalScope = &global;

    // It shouldn't be called for working code. But my code doesn't work, yet.
   //ConsoleLogger logger;
   //context.logger = &logger;

   Backway::ContextBuilder::createGlobalScope(global);

   std::vector<std::pair<std::string, std::string> > states = 
   {
         // WHY DID I USE THE SEMICOLON?
      { "Start", "set Update to function update (arg) is call Push('C') call Push({'A'; 'B'}) return arg end" },
      { "A", "set Pass to 0.0 set Update to function update (arg) is if Pass = 1.0 then call Precede('D') call Precede({'E'; 'F'}) call Follow('G') call Follow({'H'; 'I'}) call Enqueue('J') call Enqueue({'K'; 'L'}) else set Pass to 1.0 end return arg end" },
      { "B", "set Pass to 0.0 set Update to function update (arg) is if Pass = 1.0 then call Inject('F') call Inject({'H'; 'I'}) call Finally('K') call Finally({'L'; 'M'}) call Leave() else set Pass to 1.0 end return arg end" },
      { "C", "set Update to function update (arg) is return arg end" },
      { "D", "set Update to function update (arg) is return arg end" },
      { "E", "set Pass to 0.0 set Update to function update (arg) is if Pass = 1.0 then call Skip('B') else set Pass to 1.0 end return arg end" },
      { "F", "set Update to function update (arg) is call Unwind('L') return arg end" },
      { "G", "set Update to function update (arg) is return arg end" },
      { "H", "set Update to function update (arg) is call Leave() return arg end" },
      { "I", "set Update to function update (arg) is call Leave() return arg end" },
      { "J", "set Pass to 0.0 set Update to function update (arg) is if Pass = 1.0 then call Return() else set Pass to 1.0 end return arg end" },
      { "K", "set Update to function update (arg) is return arg end" },
      { "L", "set Pass to 0.0 set Update to function update (arg) is if Pass = 1 then call CreateState('N'; 'set Update to function (arg) is end') call Return() else set Pass to 1.0 end return arg end" },
      { "M", "set Update to function update (arg) is call CreateState('N'; 5) return arg end" }
   };

   for (const auto& state : states)
    {
      EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ONE.get(),
         Backway::CreateState(context, std::make_shared<Backwards::Types::StringValue>(state.first), std::make_shared<Backwards::Types::StringValue>(state.second)).get());
    }
   EXPECT_EQ(Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ZERO.get(),
      Backway::CreateState(context, std::make_shared<Backwards::Types::StringValue>(states[0].first), std::make_shared<Backwards::Types::StringValue>(states[0].second)).get());

   EXPECT_FALSE(machine.update(context));
   EXPECT_TRUE(checkState({ }, machine.states));

   Backway::Push(context, std::make_shared<Backwards::Types::StringValue>("Start"));
   EXPECT_TRUE(checkState({ { "Start" } }, machine.states));

   EXPECT_TRUE(machine.update(context));
   EXPECT_TRUE(checkState({ { "Start" }, { "C" }, { "A", "B"} }, machine.states));

   EXPECT_TRUE(machine.update(context));
   EXPECT_TRUE(checkState({ { "Start" }, { "C" }, { "E", "H", "I", "G", "F", "D", "A", "B", "J", "K", "L"} }, machine.states));

   EXPECT_TRUE(machine.update(context));
   EXPECT_TRUE(checkState({ { "Start" }, { "C" }, { "B", "J", "K", "L"} }, machine.states));

   EXPECT_TRUE(machine.update(context));
   EXPECT_TRUE(checkState({ { "L", "M" }, { "K" }, { "Start" }, { "C" }, { "F" }, { "H", "I" }, { "J", "K", "L"} }, machine.states));

   EXPECT_TRUE(machine.update(context));
   EXPECT_TRUE(checkState({ { "L", "M" } }, machine.states));

   EXPECT_FALSE(machine.update(context));
   EXPECT_TRUE(checkState({ }, machine.states));

   EXPECT_FALSE(machine.update(context));
   EXPECT_TRUE(checkState({ }, machine.states));

   Backway::Push(context, std::make_shared<Backwards::Types::StringValue>("N"));
   EXPECT_TRUE(checkState({ { "N" } }, machine.states));

   EXPECT_THROW(machine.update(context), Backwards::Engine::FatalException);

   DummyDebugger debugger;
   context.debugger = &debugger;

   machine.states.clear();
   Backway::Push(context, std::make_shared<Backwards::Types::StringValue>("M"));
   EXPECT_TRUE(checkState({ { "M" } }, machine.states));

   debugger.entered = false;
   EXPECT_THROW(machine.update(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);
 }

TEST(BackwayTests, testDuplicate)
 {
   Backway::CallingContext context;
   Backway::StateMachine machine ("machine5");
   context.machine = &machine;
   Backway::Environment environment;
   context.environment = &environment;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Backway::ContextBuilder::createGlobalScope(global);

   std::shared_ptr<Backwards::Engine::CallingContext> result = context.duplicate();
   std::shared_ptr<Backway::CallingContext> res = std::dynamic_pointer_cast<Backway::CallingContext>(result);

   ASSERT_NE(nullptr, res.get());
   EXPECT_EQ(&logger, res->logger);
   EXPECT_EQ(&global, res->globalScope);
   EXPECT_EQ(nullptr, res->topScope());
   EXPECT_EQ(nullptr, res->debugger);
   EXPECT_EQ(&machine, res->machine);
   EXPECT_EQ(&environment, res->environment);
 }

class CommandA final : public Backway::Command
 {
 };

class CommandB final : public Backway::Command
 {
 };

TEST(BackwayTests, testAddCommand)
 {
   Backway::StateMachine machine ("deus_ex_machina");
   std::shared_ptr<CommandA> commandA = std::make_shared<CommandA>();
   std::shared_ptr<CommandB> commandB = std::make_shared<CommandB>();

   machine.addOutput(commandA);
   machine.addOutput(commandB);

   ASSERT_NE(nullptr, machine.output.get());
   EXPECT_EQ(commandA.get(), machine.output.get());
   EXPECT_EQ(commandB.get(), machine.output->next.get());
   ASSERT_EQ(commandB.get(), machine.output->last);
   EXPECT_EQ(nullptr, machine.output->last->next.get());
   EXPECT_EQ(nullptr, machine.output->last->last);
 }
