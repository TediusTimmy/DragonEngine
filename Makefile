#Makefile for Dragon-Engine

CCP := g++
AR := ar

CFLAGS += -Wall -Wextra -Wpedantic

B_INCLUDE := -ISlowFloat -IBackwards/include
F_INCLUDE := $(B_INCLUDE) -IBackway/include
D_INCLUDE := $(F_INCLUDE) -IDragon/include

ifeq "$(MAKECMDGOALS)" "release"
   CCP := x86_64-w64-mingw32-g++.exe
   CITY_FLAG := -D_MSC_VER
   FINAL_BINARY := bin/Engine.exe
   LINKING_FLAGS := -lSDL2_mixer -lSDL2_image -lSDL2 -Wl,-subsystem,windows

   CFLAGS += -O2
   BFLAGS += -s
endif

ifeq "$(MAKECMDGOALS)" "debug"
   CCP := x86_64-w64-mingw32-g++.exe
   CITY_FLAG := -D_MSC_VER
   FINAL_BINARY := bin/Engine.exe
   LINKING_FLAGS := -lSDL2_mixer -lSDL2_image -lSDL2 -Wl,-subsystem,windows

   CFLAGS += -O0 -g
endif

ifeq "$(MAKECMDGOALS)" "test"
   CFLAGS += -O2
endif

ifeq "$(MAKECMDGOALS)" "emscripten"
   CCP := em++
   AR := emar
   FINAL_BINARY := bin/Engine.html
   LIBRARY_FLAGS := -s USE_SDL=2 --use-port=sdl2_image:formats=png --use-port=sdl2_mixer
   LINKING_FLAGS := $(LIBRARY_FLAGS) --use-preload-plugins --embed-file game@/ -s SINGLE_FILE

# potentially-evaluated-expression -- I'm not sold that this is a useful diagnostic, or a useful rule from the standard it comes out of.
   CFLAGS += -O2 -Wno-potentially-evaluated-expression
endif

.PHONY: all clean release debug emscripten
all: bin/Engine.exe


clean:
	rm bin/*.exe | true
	rm lib/* | true
	rm -rf obj/* | true

release: all


debug: all


emscripten: all


test: bin/SlowFloatTest.exe bin/BackwardsTest.exe bin/BackwayTest.exe bin/DragonTest.exe


bin/Engine.exe: lib/Dragon.a lib/Backway.a lib/Backwards.a lib/SlowFloat.a obj/city.o obj/main.o obj/Dragon.o obj/MainLoop.o obj/SDL.o | bin
	$(CCP) $(CFLAGS) $(BFLAGS) $(D_INCLUDE) -o $(FINAL_BINARY) obj/main.o obj/Dragon.o obj/MainLoop.o obj/SDL.o lib/Dragon.a lib/Backway.a lib/Backwards.a lib/SlowFloat.a obj/city.o $(LINKING_FLAGS)

obj/main.o: Main/main.cpp
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/main.o Main/main.cpp

obj/Dragon.o: Main/Dragon.cpp
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/Dragon.o Main/Dragon.cpp

obj/MainLoop.o: Main/MainLoop.cpp
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/MainLoop.o Main/MainLoop.cpp

obj/SDL.o: Main/SDL.cpp
	$(CCP) $(CFLAGS) $(D_INCLUDE) $(LIBRARY_FLAGS) -c -o obj/SDL.o Main/SDL.cpp


bin/SlowFloatTest.exe: lib/SlowFloat.a | bin
	$(CCP) $(CFLAGS) $(BFLAGS) -o bin/SlowFloatTest.exe -I../External/googletest/include SlowFloat/SlowFloatTest.cpp lib/SlowFloat.a ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a
	./bin/SlowFloatTest.exe

lib/SlowFloat.a: obj/SlowFloat/SlowFloat.o | lib
	$(AR) -rsc lib/SlowFloat.a obj/SlowFloat/*.o

obj/SlowFloat/SlowFloat.o: SlowFloat/SlowFloat.cpp | obj/SlowFloat
	$(CCP) $(CFLAGS) -c -o obj/SlowFloat/SlowFloat.o SlowFloat/SlowFloat.cpp


bin/BackwardsTest.exe: lib/Backwards.a lib/SlowFloat.a | bin
	$(CCP) $(CFLAGS) $(BFLAGS) $(B_INCLUDE) -o bin/BackwardsTest.exe -I../External/googletest/include Backwards/Tests/DebuggerTest.cpp Backwards/Tests/ExecutionTest.cpp Backwards/Tests/ExpressionTest.cpp Backwards/Tests/LexerTest.cpp Backwards/Tests/ParserTest.cpp Backwards/Tests/StatementTest.cpp Backwards/Tests/StdLibTest.cpp Backwards/Tests/TypesTest.cpp lib/Backwards.a ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a lib/SlowFloat.a
	./bin/BackwardsTest.exe

lib/Backwards.a: obj/Backwards/CallingContext.o obj/Backwards/ConstantsSingleton.o obj/Backwards/Expression.o obj/Backwards/Statement.o obj/Backwards/StdLib.o obj/Backwards/BufferedGenericInput.o obj/Backwards/Lexer.o obj/Backwards/LineBufferedStreamInput.o obj/Backwards/StringInput.o obj/Backwards/ContextBuilder.o obj/Backwards/DebuggerHook.o obj/Backwards/Eval.o obj/Backwards/Parser.o obj/Backwards/SymbolTable.o obj/Backwards/ArrayValue.o obj/Backwards/DictionaryValue.o obj/Backwards/FloatValue.o obj/Backwards/FunctionValue.o obj/Backwards/StringValue.o obj/Backwards/ValueType.o | lib
	$(AR) -rsc lib/Backwards.a obj/Backwards/*.o

obj/Backwards/CallingContext.o: Backwards/src/Engine/CallingContext.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/CallingContext.o Backwards/src/Engine/CallingContext.cpp

obj/Backwards/ConstantsSingleton.o: Backwards/src/Engine/ConstantsSingleton.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/ConstantsSingleton.o Backwards/src/Engine/ConstantsSingleton.cpp

obj/Backwards/Expression.o: Backwards/src/Engine/Expression.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/Expression.o Backwards/src/Engine/Expression.cpp

obj/Backwards/Statement.o: Backwards/src/Engine/Statement.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/Statement.o Backwards/src/Engine/Statement.cpp

obj/Backwards/StdLib.o: Backwards/src/Engine/StdLib.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/StdLib.o Backwards/src/Engine/StdLib.cpp

obj/Backwards/BufferedGenericInput.o: Backwards/src/Input/BufferedGenericInput.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/BufferedGenericInput.o Backwards/src/Input/BufferedGenericInput.cpp

obj/Backwards/Lexer.o: Backwards/src/Input/Lexer.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/Lexer.o Backwards/src/Input/Lexer.cpp

obj/Backwards/LineBufferedStreamInput.o: Backwards/src/Input/LineBufferedStreamInput.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/LineBufferedStreamInput.o Backwards/src/Input/LineBufferedStreamInput.cpp

obj/Backwards/StringInput.o: Backwards/src/Input/StringInput.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/StringInput.o Backwards/src/Input/StringInput.cpp

obj/Backwards/ContextBuilder.o: Backwards/src/Parser/ContextBuilder.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/ContextBuilder.o Backwards/src/Parser/ContextBuilder.cpp

obj/Backwards/DebuggerHook.o: Backwards/src/Parser/DebuggerHook.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/DebuggerHook.o Backwards/src/Parser/DebuggerHook.cpp

obj/Backwards/Eval.o: Backwards/src/Parser/Eval.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/Eval.o Backwards/src/Parser/Eval.cpp

obj/Backwards/Parser.o: Backwards/src/Parser/Parser.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/Parser.o Backwards/src/Parser/Parser.cpp

obj/Backwards/SymbolTable.o: Backwards/src/Parser/SymbolTable.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/SymbolTable.o Backwards/src/Parser/SymbolTable.cpp

obj/Backwards/ArrayValue.o: Backwards/src/Types/ArrayValue.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/ArrayValue.o Backwards/src/Types/ArrayValue.cpp

obj/Backwards/DictionaryValue.o: Backwards/src/Types/DictionaryValue.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/DictionaryValue.o Backwards/src/Types/DictionaryValue.cpp

obj/Backwards/FloatValue.o: Backwards/src/Types/FloatValue.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/FloatValue.o Backwards/src/Types/FloatValue.cpp

obj/Backwards/FunctionValue.o: Backwards/src/Types/FunctionValue.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/FunctionValue.o Backwards/src/Types/FunctionValue.cpp

obj/Backwards/StringValue.o: Backwards/src/Types/StringValue.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/StringValue.o Backwards/src/Types/StringValue.cpp

obj/Backwards/ValueType.o: Backwards/src/Types/ValueType.cpp | obj/Backwards
	$(CCP) $(CFLAGS) $(B_INCLUDE) -c -o obj/Backwards/ValueType.o Backwards/src/Types/ValueType.cpp


bin/BackwayTest.exe: lib/Backway.a lib/Backwards.a lib/SlowFloat.a obj/city.o | bin
	$(CCP) $(CFLAGS) $(BFLAGS) $(F_INCLUDE) -o bin/BackwayTest.exe -I../External/googletest/include Backway/Tests/StdLibTest.cpp lib/Backway.a ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a lib/Backwards.a lib/SlowFloat.a obj/city.o
	./bin/BackwayTest.exe

lib/Backway.a: obj/Backway/ContextBuilder.o obj/Backway/State.o obj/Backway/StateMachine.o obj/Backway/StdLib.o  | lib
	$(AR) -rsc lib/Backway.a obj/Backway/*.o

obj/Backway/ContextBuilder.o: Backway/src/ContextBuilder.cpp | obj/Backway
	$(CCP) $(CFLAGS) $(F_INCLUDE) -c -o obj/Backway/ContextBuilder.o Backway/src/ContextBuilder.cpp

obj/Backway/State.o: Backway/src/State.cpp | obj/Backway
	$(CCP) $(CFLAGS) $(F_INCLUDE) -c -o obj/Backway/State.o Backway/src/State.cpp

obj/Backway/StateMachine.o: Backway/src/StateMachine.cpp | obj/Backway
	$(CCP) $(CFLAGS) $(F_INCLUDE) -c -o obj/Backway/StateMachine.o Backway/src/StateMachine.cpp

obj/Backway/StdLib.o: Backway/src/StdLib.cpp | obj/Backway
	$(CCP) $(CFLAGS) $(F_INCLUDE) -c -o obj/Backway/StdLib.o Backway/src/StdLib.cpp


obj/city.o: external/city.cc | obj
	$(CCP) $(CFLAGS) $(CITY_FLAG) -c -o obj/city.o external/city.cc


bin/DragonTest.exe: lib/Dragon.a lib/Backway.a lib/Backwards.a lib/SlowFloat.a obj/city.o | bin
	$(CCP) $(CFLAGS) $(BFLAGS) $(D_INCLUDE) -o bin/DragonTest.exe -I../External/googletest/include Dragon/Tests/StdLibTest.cpp lib/Dragon.a ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a lib/Backway.a lib/Backwards.a lib/SlowFloat.a obj/city.o
	./bin/DragonTest.exe

lib/Dragon.a: obj/Dragon/ContextBuilder.o obj/Dragon/StdLib.o obj/Dragon/Universe.o  | lib
	$(AR) -rsc lib/Dragon.a obj/Dragon/*.o

obj/Dragon/ContextBuilder.o: Dragon/src/ContextBuilder.cpp | obj/Dragon
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/Dragon/ContextBuilder.o Dragon/src/ContextBuilder.cpp

obj/Dragon/StdLib.o: Dragon/src/StdLib.cpp | obj/Dragon
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/Dragon/StdLib.o Dragon/src/StdLib.cpp

obj/Dragon/Universe.o: Dragon/src/Universe.cpp | obj/Dragon
	$(CCP) $(CFLAGS) $(D_INCLUDE) -c -o obj/Dragon/Universe.o Dragon/src/Universe.cpp


bin:
	mkdir bin

lib:
	mkdir lib

obj/SlowFloat:
	mkdir -p obj/SlowFloat

obj/Backwards:
	mkdir -p obj/Backwards

obj/Backway:
	mkdir -p obj/Backway

obj/Dragon:
	mkdir -p obj/Dragon
