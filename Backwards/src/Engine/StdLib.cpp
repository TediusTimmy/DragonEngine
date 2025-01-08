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
#include "Backwards/Engine/StdLib.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/ConstantsSingleton.h"

#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/DebuggerHook.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <ctime>

namespace Backwards
 {

namespace Engine
 {

   //////////
   // Expression and Statement are built on these first 7 functions.
   //////////

   STDLIB_CONSTANT_DECL(NewArray)
    {
      return ConstantsSingleton::getInstance().EMPTY_ARRAY;
    }

   STDLIB_CONSTANT_DECL(NewDictionary)
    {
      return ConstantsSingleton::getInstance().EMPTY_DICTIONARY;
    }

   STDLIB_BINARY_DECL(PushBack)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         // Yes, construct a new container on modification.
         // All operations treat ValueTypes as immutable, so this is safe.
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         result->value = static_cast<const Types::ArrayValue&>(*first).value;
         result->value.push_back(second);
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error pushing back to non-Array.");
       }
    }

   STDLIB_TERNARY_DECL(Insert)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         // Yes, construct a new container on modification.
         std::shared_ptr<Types::DictionaryValue> result = std::make_shared<Types::DictionaryValue>();
         result->value = static_cast<const Types::DictionaryValue&>(*first).value;
         result->value[second] = third;
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error inserting into non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(GetValue)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            return iter->second;
          }
         else
          {
            throw Types::TypedOperationException("Key not found in Dictionary.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to retrieve key from non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(GetIndex)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            double index = static_cast<double>(static_cast<const Types::FloatValue&>(*second).value);
            if ((index >= 0.0) && (index < static_cast<double>(static_cast<const Types::ArrayValue&>(*first).value.size())))
             {
               return static_cast<const Types::ArrayValue&>(*first).value[static_cast<size_t>(index)];
             }
            else
             {
               throw Types::TypedOperationException("Array Index Out-of-Bounds.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error indexing with non-Float.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error indexing into non-Array.");
       }
    }

   STDLIB_TERNARY_DECL(SetIndex)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            double index = static_cast<double>(static_cast<const Types::FloatValue&>(*second).value);
            if ((index >= 0.0) && (index < static_cast<double>(static_cast<const Types::ArrayValue&>(*first).value.size())))
             {
               // Yes, construct a new container on modification.
               std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
               result->value = static_cast<const Types::ArrayValue&>(*first).value;
               result->value[static_cast<size_t>(index)] = third;
               return result;
             }
            else
             {
               throw Types::TypedOperationException("Array Index Out-of-Bounds.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error indexing with non-Float.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error indexing into non-Array.");
       }
    }

   //////////
   // These next 5 are the most basic in telling if it works.
   //////////

#define LOGGINGFUNCTIONDEFN(x,y) \
   STDLIB_UNARY_DECL_WITH_CONTEXT(x) \
    { \
      if (typeid(Types::StringValue) == typeid(*arg)) \
       { \
         context.logger->log(y + static_cast<const Types::StringValue&>(*arg).value); \
         return arg; \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error logging non-String."); \
       } \
    }

   LOGGINGFUNCTIONDEFN(Error, "ERROR: ")
   LOGGINGFUNCTIONDEFN(Warn, "WARN: ")
   LOGGINGFUNCTIONDEFN(Info, "INFO: ")

   STDLIB_UNARY_DECL_WITH_CONTEXT(Fatal)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         context.logger->log("FATAL: " + static_cast<const Types::StringValue&>(*arg).value);
         throw FatalException(static_cast<const Types::StringValue&>(*arg).value);
       }
      else
       {
         throw FatalException("Error logging non-String while trying to generate a Fatal message.");
       }
    }

   STDLIB_UNARY_DECL(ToString)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         return std::make_shared<Types::StringValue>(SlowFloat::toString(static_cast<const Types::FloatValue&>(*arg).value));
       }
      else
       {
         throw Types::TypedOperationException("Error converting non-Float to String.");
       }
    }

   //////////
   // The other 51 functions of the Standard Library in no particular order. (Eval and EnterDebugger are not counted here.)
   //////////

   STDLIB_BINARY_DECL(PushFront)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         result->value.push_back(second);
         const std::vector<std::shared_ptr<Types::ValueType> >& source = static_cast<const Types::ArrayValue&>(*first).value;
         result->value.insert(result->value.end(), source.begin(), source.end());
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error pushing front to non-Array.");
       }
    }

   STDLIB_UNARY_DECL(PopBack)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         if (false == static_cast<const Types::ArrayValue&>(*arg).value.empty())
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            result->value = static_cast<const Types::ArrayValue&>(*arg).value;
            result->value.pop_back();
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error popping back of empty Array.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error popping back of non-Array.");
       }
    }

   STDLIB_UNARY_DECL(PopFront)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         if (false == static_cast<const Types::ArrayValue&>(*arg).value.empty())
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            const std::vector<std::shared_ptr<Types::ValueType> >& source = static_cast<const Types::ArrayValue&>(*arg).value;
            result->value.assign(source.begin() + 1U, source.end());
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error popping front of empty Array.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error popping front of non-Array.");
       }
    }

#define RTTIFUNCTIONDEFN(x,y) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(y) == typeid(*arg)) \
       { \
         return ConstantsSingleton::getInstance().FLOAT_ONE; \
       } \
      else \
       { \
         return ConstantsSingleton::getInstance().FLOAT_ZERO; \
       } \
    }

   RTTIFUNCTIONDEFN(IsFloat, Types::FloatValue)
   RTTIFUNCTIONDEFN(IsString, Types::StringValue)
   RTTIFUNCTIONDEFN(IsArray, Types::ArrayValue)
   RTTIFUNCTIONDEFN(IsDictionary, Types::DictionaryValue)
   RTTIFUNCTIONDEFN(IsFunction, Types::FunctionValue)

   STDLIB_UNARY_DECL(Length)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(static_cast<const Types::StringValue&>(*arg).value.size())));
       }
      else
       {
         throw Types::TypedOperationException("Error getting length of non-String.");
       }
    }

   STDLIB_UNARY_DECL(Size)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(static_cast<const Types::ArrayValue&>(*arg).value.size())));
       }
      else if (typeid(Types::DictionaryValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(static_cast<const Types::DictionaryValue&>(*arg).value.size())));
       }
      else
       {
         throw Types::TypedOperationException("Error getting size of non-Collection.");
       }
    }

   STDLIB_BINARY_DECL(NewArrayDefault)
    {
      if (typeid(Types::FloatValue) == typeid(*first))
       {
         double size = static_cast<double>(static_cast<const Types::FloatValue&>(*first).value);
         if ((size >= 0.0) && (size < static_cast<double>(std::numeric_limits<size_t>::max())))
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            result->value.resize(static_cast<size_t>(size), second);
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error creating Array size either negative or too big.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error creating Array with non-Float size.");
       }
    }

   STDLIB_TERNARY_DECL(SubString)
    {
      if (typeid(Types::StringValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            if (typeid(Types::FloatValue) == typeid(*third))
             {
               double stringLength = static_cast<double>(static_cast<const Types::StringValue&>(*first).value.length());
               double startIndex = static_cast<double>(static_cast<const Types::FloatValue&>(*second).value);
               double endIndex = static_cast<double>(static_cast<const Types::FloatValue&>(*third).value);
               if ((startIndex >= 0.0) && (startIndex <= stringLength) &&
                  (endIndex >= 0.0) && (endIndex <= stringLength) &&
                  (endIndex >= startIndex))
                {
                  std::shared_ptr<Types::StringValue> result = std::make_shared<Types::StringValue>();
                  result->value = static_cast<const Types::StringValue&>(*first).value.substr(static_cast<size_t>(startIndex), static_cast<size_t>(endIndex - startIndex));
                  return result;
                }
               else
                {
                  throw Types::TypedOperationException("Error getting substring with either beginning or ending index not in String, or ending before beginning.");
                }
             }
            else
             {
               throw Types::TypedOperationException("Error getting substring with non-Float ending position.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error getting substring with non-Float starting position.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error getting substring of non-String.");
       }
    }

   STDLIB_BINARY_DECL(ContainsKey)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            return ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            return ConstantsSingleton::getInstance().FLOAT_ZERO;
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to determine if key is in non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(RemoveKey)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            std::shared_ptr<Types::DictionaryValue> result = std::make_shared<Types::DictionaryValue>();
            result->value = static_cast<const Types::DictionaryValue&>(*first).value;
            result->value.erase(second);
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Key not found in Dictionary.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to remove key from non-Dictionary.");
       }
    }

   STDLIB_UNARY_DECL(GetKeys)
    {
      if (typeid(Types::DictionaryValue) == typeid(*arg))
       {
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         for (std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*arg).value.begin();
            static_cast<const Types::DictionaryValue&>(*arg).value.end() != iter; ++iter)
          {
            result->value.push_back(iter->first);
          }
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error trying to get all keys from non-Dictionary.");
       }
    }

   STDLIB_CONSTANT_DECL(PI)
    {
      return ConstantsSingleton::getInstance().FLOAT_PI;
    }

   STDLIB_CONSTANT_DECL(Date)
    {
      std::stringstream str;
      std::time_t nowTime;
      std::tm* timeToDecompose;
      std::time(&nowTime);
      timeToDecompose = std::localtime(&nowTime);
      str << std::setw(2) << std::setfill('0') << (timeToDecompose->tm_mon + 1) << '/' <<
         std::setw(2) << std::setfill('0') << timeToDecompose->tm_mday << '/' <<
         std::setw(4) << std::setfill('0') << (timeToDecompose->tm_year + 1900);
      return std::make_shared<Types::StringValue>(str.str());
    }

   STDLIB_CONSTANT_DECL(Time)
    {
      std::stringstream str;
      std::time_t nowTime;
      std::tm* timeToDecompose;
      std::time(&nowTime);
      timeToDecompose = std::localtime(&nowTime);
      str << std::setw(2) << std::setfill('0') << timeToDecompose->tm_hour << ':' <<
         std::setw(2) << std::setfill('0') << timeToDecompose->tm_min << ':' <<
         std::setw(2) << std::setfill('0') << timeToDecompose->tm_sec;
      return std::make_shared<Types::StringValue>(str.str());
    }

   static const double RTD = 180.0 / M_PI;
   static const double DTR = M_PI / 180.0;

   STDLIB_BINARY_DECL(Atan2)
    {
      if (typeid(Types::FloatValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::atan2(
               static_cast<double>(static_cast<const Types::FloatValue&>(*first).value), static_cast<double>(static_cast<const Types::FloatValue&>(*second).value)) * RTD));
          }
         else
          {
            throw Types::TypedOperationException("Error computing atan2 with non-Float second argument.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error computing atan2 with non-Float first argument.");
       }
    }

   STDLIB_BINARY_DECL(Hypot)
    {
      if (typeid(Types::FloatValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::hypot(
               static_cast<double>(static_cast<const Types::FloatValue&>(*first).value), static_cast<double>(static_cast<const Types::FloatValue&>(*second).value))));
          }
         else
          {
            throw Types::TypedOperationException("Error computing hypot with non-Float second argument.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error computing hypot with non-Float first argument.");
       }
    }

   STDLIB_BINARY_DECL(Log)
    {
      if (typeid(Types::FloatValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::log(static_cast<double>(static_cast<const Types::FloatValue&>(*second).value)) /
               std::log(static_cast<double>(static_cast<const Types::FloatValue&>(*first).value))));
          }
         else
          {
            throw Types::TypedOperationException("Error computing log with non-Float second argument.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error computing log with non-Float first argument.");
       }
    }

#define MINMAXDEFN(x,y,z) \
   STDLIB_BINARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*first)) \
       { \
         if (typeid(Types::FloatValue) == typeid(*second)) \
          { \
            double fVal = static_cast<double>(static_cast<const Types::FloatValue&>(*first).value); \
            double sVal = static_cast<double>(static_cast<const Types::FloatValue&>(*second).value); \
            if (true == std::isnan(fVal)) \
             { \
               return first; \
             } \
            else if (true == std::isnan(sVal)) \
             { \
               return second; \
             } \
            return (fVal y sVal) ? first : second; \
          } \
         else \
          { \
            throw Types::TypedOperationException("Error computing " z " with non-Float second argument."); \
          } \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error computing " z " with non-Float first argument."); \
       } \
    }

   MINMAXDEFN(Max, >=, "max")
   MINMAXDEFN(Min, <=, "min")

#define TRIGFUNCTIONDEFN(x,y,z) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*arg)) \
       { \
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::y(static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value) * DTR))); \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error trying to compute " z " of non-Float."); \
       } \
    }

   TRIGFUNCTIONDEFN(Sin, sin, "sine")
   TRIGFUNCTIONDEFN(Cos, cos, "cosine")
   TRIGFUNCTIONDEFN(Tan, tan, "tangent")

#define INVTRIGFUNCTIONDEFN(x,y,z) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*arg)) \
       { \
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::y(static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value)) * RTD)); \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error trying to compute " z " of non-Float."); \
       } \
    }

   INVTRIGFUNCTIONDEFN(Asin, asin, "arcsine")
   INVTRIGFUNCTIONDEFN(Acos, acos, "arccosine")
   INVTRIGFUNCTIONDEFN(Atan, atan, "arctangent")

#define BASICONEARGMATHDEFN(x,y,z) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*arg)) \
       { \
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(std::y(static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value)))); \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error trying to compute " z " of non-Float."); \
       } \
    }

   BASICONEARGMATHDEFN(Exp, exp, "exponential")
   BASICONEARGMATHDEFN(Ln, log, "natural logarithm")
   BASICONEARGMATHDEFN(Sqrt, sqrt, "square root")
   BASICONEARGMATHDEFN(Cbrt, cbrt, "cube root")
   BASICONEARGMATHDEFN(Abs, fabs, "absolute value")
   BASICONEARGMATHDEFN(Round, round, "rounded value")
   BASICONEARGMATHDEFN(Floor, floor, "rounded to negative infinity")
   BASICONEARGMATHDEFN(Ceil, ceil, "rounded to positive infinity")
   BASICONEARGMATHDEFN(IsInfinity, isinf, "is infinity")
    // Well, technically, I guess it SHOULD return true if the argument is not a Float....
   BASICONEARGMATHDEFN(IsNaN, isnan, "is special not-a-number value")
   BASICONEARGMATHDEFN(Sinh, sinh, "hyperbolic sine")
   BASICONEARGMATHDEFN(Cosh, cosh, "hyperbolic cosine")
   BASICONEARGMATHDEFN(Tanh, tanh, "hyperbolic tangent")

   STDLIB_UNARY_DECL(Sqr)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         SlowFloat::SlowFloat x = static_cast<const Types::FloatValue&>(*arg).value;
         return std::make_shared<Types::FloatValue>(x * x);
       }
      else
       {
         throw Types::TypedOperationException("Error trying to square non-Float.");
       }
    }

#define DTRRTDDEFN(x,y) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*arg)) \
       { \
         return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value) * y)); \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error trying to convert non-Float between Radians and Degrees."); \
       } \
    }

   DTRRTDDEFN(DegToRad, DTR)
   DTRRTDDEFN(RadToDeg, RTD)

   STDLIB_UNARY_DECL(ValueOf)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         std::stringstream str (static_cast<const Types::StringValue&>(*arg).value);
         double val;
         str >> val;
         if (!str.fail() && (str.get() == std::char_traits<char>::eof()))
          {
            return std::make_shared<Types::FloatValue>(SlowFloat::fromString(static_cast<const Types::StringValue&>(*arg).value));
          }
         else
          {
            throw Types::TypedOperationException("String did not contain valid Float value.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to get value of non-String.");
       }
    }

   STDLIB_UNARY_DECL(FromCharacter)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         const std::string& str (static_cast<const Types::StringValue&>(*arg).value);
         if (1U == str.size())
          {
            return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(str[0])));
          }
         else
          {
            throw Types::TypedOperationException("String was not single character.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-String to Float character code point.");
       }
    }

   STDLIB_UNARY_DECL(ToCharacter)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         double val = static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value);
         if ((val > static_cast<double>(std::numeric_limits<char>::min())) &&
            (val < static_cast<double>(std::numeric_limits<char>::max())))
          {
            std::string str;
            str += static_cast<char>(val);
            return std::make_shared<Types::StringValue>(str);
          }
         else
          {
            throw Types::TypedOperationException("Float is not a valid character code point.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-Float to single character String.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(DebugPrint)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         context.logger->log(static_cast<const Types::StringValue&>(*arg).value);
         return arg;
       }
      else
       {
         throw Types::TypedOperationException("Error logging non-String.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(EnterDebugger)
    {
      if (nullptr != context.debugger)
       {
         context.debugger->EnterDebugger("", context);
       }
      return ConstantsSingleton::getInstance().FLOAT_ZERO;
    }

   STDLIB_CONSTANT_DECL(GetRoundMode)
    {
      return std::make_shared<Types::FloatValue>(SlowFloat::SlowFloat(static_cast<double>(SlowFloat::mode)));
    }

   STDLIB_UNARY_DECL(SetRoundMode)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         double val = static_cast<double>(static_cast<const Types::FloatValue&>(*arg).value);
         if ((val >= static_cast<double>(SlowFloat::ROUND_TIES_EVEN)) &&
            (val <= static_cast<double>(SlowFloat::ROUND_AWAY)))
          {
            SlowFloat::mode = static_cast<SlowFloat::SlowFloat_Round_Mode>(val);
            return arg;
          }
         else
          {
            throw Types::TypedOperationException("Float is not a valid rounding mode.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-Float to rounding mode.");
       }
    }

 } // namespace Engine

 } // namespace Backwards
