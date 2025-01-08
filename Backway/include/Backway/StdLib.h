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
#ifndef BACKWAY_STDLIB_H
#define BACKWAY_STDLIB_H

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Statement.h"
#include "Backwards/Types/ValueType.h"

namespace Backway
 {

#define STDLIB_CONSTANT_DECL_WITH_CONTEXT(x) \
   std::shared_ptr<Backwards::Types::ValueType> x (Backwards::Engine::CallingContext& context)

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetInput);
   STDLIB_CONSTANT_DECL_WITH_CONTEXT(GetName);
   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Leave);
   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Rand);
   STDLIB_CONSTANT_DECL_WITH_CONTEXT(Return);

#define STDLIB_UNARY_DECL_WITH_CONTEXT(x) \
   std::shared_ptr<Backwards::Types::ValueType> x (Backwards::Engine::CallingContext& context, const std::shared_ptr<Backwards::Types::ValueType>& arg)

   STDLIB_UNARY_DECL_WITH_CONTEXT(SetOutput);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Enqueue);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Finally);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Follow);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Inject);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Precede);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Push);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Skip);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Unwind);

   typedef std::shared_ptr<Backwards::Types::ValueType> (*BinaryFunctionPointerWithContext) (Backwards::Engine::CallingContext& context,
      const std::shared_ptr<Backwards::Types::ValueType>&, const std::shared_ptr<Backwards::Types::ValueType>&);

   class StandardBinaryFunctionWithContext final : public Backwards::Engine::Statement
    {
   public:
      BinaryFunctionPointerWithContext function;
      explicit StandardBinaryFunctionWithContext(BinaryFunctionPointerWithContext);
      std::shared_ptr<Backwards::Engine::FlowControl> execute (Backwards::Engine::CallingContext&) const override;
    };

#define STDLIB_BINARY_DECL_WITH_CONTEXT(x) \
   std::shared_ptr<Backwards::Types::ValueType> x (Backwards::Engine::CallingContext& context, \
      const std::shared_ptr<Backwards::Types::ValueType>& first, const std::shared_ptr<Backwards::Types::ValueType>& second)

   STDLIB_BINARY_DECL_WITH_CONTEXT(CreateState);

 } // namespace Backway

#endif /* BACKWAY_STDLIB_H */
