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

#include "Dragon/CallingContext.h"
#include "Dragon/Environment.h"
#include "Dragon/Universe.h"

#include "Backwards/Engine/Logger.h"

#ifdef __EMSCRIPTEN__
   #include <emscripten/emscripten.h>
#endif /* __EMSCRIPTEN__ */

namespace Main
 {
   void Init(Dragon::CallingContext& context);
   void Loop(int micros);
   void Update(void);

   void LoadSettings(const std::string&, Settings&, Backwards::Engine::Logger*);

   void LoadEntities(Dragon::CallingContext& context, const std::map<std::string, std::string>& entities, const std::string& initialEntity, const std::string& initialState);

   void LoadSprites(const std::map<std::string, std::string>& sprites);
   bool SDL_Init(const Settings& settings);
   void SDL_Quit(void);
 }

#ifdef __EMSCRIPTEN__
class Logger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string&) { }
   std::string get () { return std::string(); }
 };
#else /* ! __EMSCRIPTEN__ */
#include <fstream>

class Logger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message)
    {
      std::ofstream file ("Engine_Log.txt", std::ios_base::out | std::ios_base::app);
      file << message << std::endl;
    }
   std::string get () { return std::string(); }
 };
#endif /* __EMSCRIPTEN__ */


#ifdef __EMSCRIPTEN__
int main (int /*argc*/, char ** /*argv*/)
#else /* ! __EMSCRIPTEN__ */
int WinMain (int /*argc*/, char ** /*argv*/)
#endif /* __EMSCRIPTEN__ */
 {
   Main::Settings settings;
   Logger logger;
   Main::LoadSettings("settings.txt", settings, &logger);

   if (false == Main::SDL_Init(settings))
    {
      return 1;
    }

   Dragon::CallingContext context;
   Dragon::Universe universe;
   context.universe = &universe;
   Dragon::Environment environment;
   context.viron = &environment;
   context.logger = &logger;
   Backwards::Engine::Scope global;
   context.globalScope = &global;

   Main::LoadSprites(settings.sprites);
   Main::LoadEntities(context, settings.entities, settings.entity, settings.state);

   Main::Init(context);
#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(Main::Update, settings.fps, true);
#else /* ! __EMSCRIPTEN__ */
   Main::Loop(1000000 / settings.fps);

	Main::SDL_Quit();
#endif /* __EMSCRIPTEN__ */

	return 0;
 }
