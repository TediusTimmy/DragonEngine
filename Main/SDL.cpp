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
#include <map>
#include <string>

#include "Settings.h"

#include "Dragon/Commands.h"
#include "Dragon/Universe.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace Main
 {

static SDL_Window* window = nullptr;
static SDL_Surface* hWind = nullptr;
static SDL_Surface* tempWind = nullptr;
static std::map<std::string, SDL_Surface*> spriteAssets;

static int spriteSize;
static int screenSize_x;
static int screenSize_y;
static int screenScale;

void LoadSprites(const std::map<std::string, std::string>& sprites)
 {
   for (const auto& sprite : sprites)
    {
      spriteAssets.emplace(std::make_pair(sprite.first, IMG_Load(sprite.second.c_str())));
    }
 }

bool SDL_Init(const Settings& settings)
 {
	if (::SDL_Init(SDL_INIT_VIDEO) < 0)
	 {
      return false;
	 }

	if (0 == IMG_Init(IMG_INIT_PNG))
	 {
      return false;
	 }

   spriteSize = settings.sprite;
   screenSize_x = settings.sx;
   screenSize_y = settings.sy;
   screenScale = settings.scale;

   window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenSize_x * spriteSize * screenScale, screenSize_y * spriteSize * screenScale, SDL_WINDOW_SHOWN);
   if (nullptr == window)
    {
      return false;
    }

   hWind = SDL_GetWindowSurface(window);
   tempWind = SDL_CreateRGBSurfaceWithFormat(0, screenSize_x * spriteSize, screenSize_y * spriteSize, hWind->pitch, hWind->format->format);

   return true;
 }

void SDL_Quit(void)
 {
	SDL_DestroyWindow(window);

   IMG_Quit();
	::SDL_Quit();
 }

static int screen_x = 0;
static int screen_y = 0;
static int screen_sx = 0;
static int screen_sy = 0;

void ProcessInput(Dragon::Universe* universe)
 {
   SDL_Event e;
   std::shared_ptr<Backwards::Types::DictionaryValue> input = std::make_shared<Backwards::Types::DictionaryValue>();
   std::shared_ptr<Backwards::Types::ArrayValue> result = std::make_shared<Backwards::Types::ArrayValue>();
   input->value[std::make_shared<Backwards::Types::StringValue>("PLAYER")] = result;
   while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
       {
         universe->background.clear();
         universe->dynamic.clear(); // Take no chances
         universe->player.clear();
       }
      else if (e.type == SDL_KEYDOWN)
       {
         switch (e.key.keysym.sym)
          {
            case SDLK_ESCAPE:
               universe->background.clear();
               universe->dynamic.clear(); // Take no chances
               universe->player.clear();
               break;

            case SDLK_0:
            case SDLK_KP_0:
            case SDLK_INSERT:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("0"));
               break;
            case SDLK_1:
            case SDLK_KP_1:
            case SDLK_END:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("1"));
               break;
            case SDLK_2:
            case SDLK_KP_2:
            case SDLK_DOWN:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("2"));
               break;
            case SDLK_3:
            case SDLK_KP_3:
            case SDLK_PAGEDOWN:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("3"));
               break;
            case SDLK_4:
            case SDLK_KP_4:
            case SDLK_LEFT:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("4"));
               break;
            case SDLK_5:
            case SDLK_KP_5:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("5"));
               break;
            case SDLK_6:
            case SDLK_KP_6:
            case SDLK_RIGHT:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("6"));
               break;
            case SDLK_7:
            case SDLK_KP_7:
            case SDLK_HOME:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("7"));
               break;
            case SDLK_8:
            case SDLK_KP_8:
            case SDLK_UP:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("8"));
               break;
            case SDLK_9:
            case SDLK_KP_9:
            case SDLK_PAGEUP:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("9"));
               break;
            case SDLK_KP_ENTER:
            case SDLK_RETURN:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("ENTER"));
               break;
            case SDLK_KP_PLUS:
            case SDLK_PLUS:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("+"));
               break;
            case SDLK_KP_PERIOD:
            case SDLK_PERIOD:
            case SDLK_DELETE:
               result->value.emplace_back(std::make_shared<Backwards::Types::StringValue>("+"));
               break;
          }
       }
    }
   universe->input = input;

   std::shared_ptr<Backwards::Types::ArrayValue> screenxy = std::make_shared<Backwards::Types::ArrayValue>();
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screen_x)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screen_y)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screen_sx)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screen_sy)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screenSize_x)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(screenSize_y)));
   screenxy->value.emplace_back(std::make_shared<Backwards::Types::FloatValue>(SlowFloat::SlowFloat(spriteSize)));
   universe->screenxy = screenxy;
 }

void ProcessCommands(const std::shared_ptr<Backway::Command>& outputList)
 {
   SDL_Rect dest;
   std::shared_ptr<Backway::Command> iter = outputList;
   while (nullptr != iter.get())
    {
      if (typeid(Dragon::Command_Move) == typeid(*iter.get()))
       {
         Dragon::Command_Move* command = static_cast<Dragon::Command_Move*>(iter.get());
         screen_x = command->x;
         screen_y = command->y;
         screen_sx = command->sx;
         screen_sy = command->sy;
       }
      else if (typeid(Dragon::Command_Draw) == typeid(*iter.get()))
       {
         Dragon::Command_Draw* command = static_cast<Dragon::Command_Draw*>(iter.get());
         dest.x = (command->x - screen_x) * spriteSize + (command->sx - screen_sx);
         dest.y = (command->y - screen_y) * spriteSize + (command->sy - screen_sy);
         const auto asset = spriteAssets.find(command->resource);
         if (spriteAssets.end() != asset)
          {
            SDL_BlitSurface(asset->second, NULL, tempWind, &dest);
          }
       }
      iter = iter->next;
    }
   dest.x = 0;
   dest.y = 0;
   dest.w = tempWind->w * screenScale;
   dest.h = tempWind->h * screenScale;
   SDL_BlitScaled(tempWind, nullptr, hWind, &dest);
   SDL_UpdateWindowSurface(window);
 }

 } // namespace Main
