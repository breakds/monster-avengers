#ifndef _MONSTER_AVENGERS_LISP_READER_
#define _MONSTER_AVENGERS_LISP_READER_

#include <vector>
#include <sstream>

#include "supp/helpers.h"
#include "lisp_object.h"
#include "parser.h"

namespace monster_avengers {
  namespace lisp {
    
    class ObjectReader {
    public:
      ObjectReader(const std::string &file_name)
        : tokenizer_(Tokenizer::FromFile(file_name)) {
        // ObjectReader expects the file contains a list of
        // Objects. Therefore, we expect the first token to be an
        // OPEN_PARENTHESIS (and the last to be a corresponding
        // CLOSE_PARENTHESIS).
        tokenizer_.Expect(OPEN_PARENTHESIS);
      }
      
      Object Read() {
        // Feeding ReadObject with INVALID_TOKEN hints ReadObject to
        // not epxect a cached token.
        return ReadObject(Token {INVALID_TOKEN, L""});
      }

    private:
      Object ReadObject(Token cache) {

        Token token;

        // Get the first token, so that we can decide the type of the
        // resulting Object.
        if (INVALID_TOKEN != cache.name) {
          token.Acquire(std::move(cache));
        } else {
          tokenizer_.Next(&token);
        }

        switch (token.name) {
        case NIL:
          return Object::List();
        case CLOSE_PARENTHESIS:
          return Object::Nil();
        case NUMBER:
          return Object::Number(std::stoi(token.value));
        case STRING:
          return Object::String(token.value);
        default:
          CHECK(OPEN_PARENTHESIS == token.name);
          // Based on the second token, we will decide whether it is
          // an object or a list.
          CHECK(tokenizer_.Next(&token));
          if (KEYWORD == token.name) {
            // If we are getting a keyword, it must be an object. We
            // do not epxect list with a keyword as the first element.

            // The Object that will be returned as result.
            Object result = Object::Struct();

            // The characters we are reading into token are utf-8
            // wchars. We need a converter to convert the wchar
            // strings to normal strings for keys.
            std::string key = CoerceUTF8ToAscii(token.value);

            bool complete = false;
            while (!complete) {
              if ("OBJ" == key) {
                // We should skip ":OBJ T".
                tokenizer_.Expect(lisp::TRUE_VALUE);
              } else {
                result[key] = ReadObject(Token {INVALID_TOKEN, L""});
              }
	      
              CHECK(tokenizer_.Next(&token));
              if (CLOSE_PARENTHESIS == token.name) {
                complete = true;
              } else {
                CHECK(KEYWORD == token.name);
                key = CoerceUTF8ToAscii(token.value);
              }
            }
            return result;
          } else {  
            // Now we are pretty sure it is a list.
            Object result = Object::List();
            result.Push(ReadObject(token));
            while (true) {
              CHECK(tokenizer_.Next(&token));
              if (CLOSE_PARENTHESIS == token.name) {
                break;
              } 
	      
              result.Push(ReadObject(token));
            }
	    
            return result;
          }  // if (KEYWORD == token.name)
        }
      }
    
      Tokenizer tokenizer_;
    };

    template <typename ElementType>
    std::vector<ElementType> ReadList(const std::string &file_name) {
      std::vector<ElementType> result;
      lisp::ObjectReader reader(file_name);
      lisp::Object object = reader.Read();
      while (!object.IsNil()) {
        result.emplace_back(object);
        object = reader.Read();
      }
      return result;
    }
    
  }  // namespace lisp
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_LISP_READER_
