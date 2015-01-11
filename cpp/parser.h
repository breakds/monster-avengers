#include <cstdio>
#include <cwchar>
#include <string>
#include <locale>
#include <type_traits>

namespace monster_avengers {
  
  namespace parser {
    
    enum TokenName {
      OPEN_PARENTHESIS = 0, // (
      CLOSE_PARENTHESIS, // )
      KEYWORD, // :keyword
      STRING, // "string"
      TRUE, // T
      NUMBER, // 1234
      NIL, // NIL
      INVALID_TOKEN, 
      NUMBER_OF_TOKEN, // number of valid tokens
    };

    struct Token {
      TokenName name;
      std::wstring value;
      
      void DebugPrint() {
        wprintf(L"Token: ");
        switch (name) {
        case OPEN_PARENTHESIS:
          wprintf(L"(\n");
          break;
        case CLOSE_PARENTHESIS:
          wprintf(L")\n");
          break;
        case KEYWORD:
          wprintf(L":%ls\n", value.c_str());
          break;
        case STRING:
          wprintf(L"\"%Ls\"\n", value.c_str());
          break;
        case TRUE:
          wprintf(L"T\n");
          break;
        case NUMBER:
          wprintf(L"%ls\n", value.c_str());
          break;
        case NIL:
          wprintf(L"NIL\n");
          break;
        default:
          wprintf(L"INVALID_TOKEN\n");
        };
      }
    };

    class Tokenizer {
    public:
      Tokenizer(const std::string &file_name) 
        : input_stream_(file_name), buffer_(), end_of_file_(false) {
        if (!input_stream_.good()) {
          Log(ERROR, L"error while opening %s.", file_name.c_str());
          exit(-1);
        }
        input_stream_.imbue(std::locale("en_US.UTF-8"));
        GetChar();
      }

      // Returns false if end of file. Returns true otherwise, even if
      // the read token may be invalid.
      bool Next(Token *token) {
        while (!end_of_file_ && std::isspace(buffer_)) GetChar();
        if (!end_of_file_) {
          if (!(Read<OPEN_PARENTHESIS>(token) ||
                Read<CLOSE_PARENTHESIS>(token) ||
                Read<KEYWORD>(token) ||
                Read<STRING>(token) ||
                Read<TRUE>(token) ||
                Read<NUMBER>(token) ||
                Read<NIL>(token))) {
            token->name = INVALID_TOKEN;
          }
          return true;
        }
        return false;
      }

      bool Expect(TokenName name) {
        Token token;
        if (Next(&token) && token.name == name) {
          return true;
        }
        return false;
      }
      
      TokenName ExpectOpenParenOrNil() {
        Token token;
        CHECK(Next(&token));
        CHECK(NIL == token.name || OPEN_PARENTHESIS == token.name);
        return token.name;
      }
      
      int ExpectNumber() {
        Token token;
        CHECK(Next(&token) && NUMBER == token.name);
        return std::stoi(token.value);
      }

      std::wstring ExpectString() {
        Token token;
        CHECK(Next(&token) && STRING == token.name);
        return token.value;
      }
      
    private:
      template <TokenName T, TokenName H>
      using Enabler = typename std::enable_if<(T==H), void>::type;
      
      inline wchar_t GetChar() {
        if (!input_stream_.get(buffer_)) {
          end_of_file_ = true;
          buffer_ = L' ';
        }
        return buffer_;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, OPEN_PARENTHESIS> *unused = nullptr) {
        if (L'(' == buffer_) {
          GetChar();
          result->name = OPEN_PARENTHESIS;
          result->value = L"";
          return true;
        }
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, CLOSE_PARENTHESIS> *unused = nullptr) {
        if (L')' == buffer_) {
          GetChar();
          result->name = CLOSE_PARENTHESIS;
          result->value = L"";
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, KEYWORD> *unused = nullptr) {
        if (L':' == buffer_) {
          result->name = KEYWORD;
          result->value = L"";
          while (!std::isspace(GetChar())) {
            result->value += buffer_;
          }
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, STRING> *unused = nullptr) {
        if (L'"' == buffer_) {
          result->name = STRING;
          result->value = L"";
          while (!end_of_file_ && L'"' != GetChar()) {
            result->value += buffer_;
          }
          if (L'"' != buffer_) {
            Log(ERROR, L"Invalid String.");
          }
          GetChar();
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, TRUE> *unused = nullptr) {
        if (L'T' == buffer_) {
          GetChar();
          result->name = TRUE;
          result->value = L"";
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, NUMBER> *unused = nullptr) {
        if (std::isdigit(buffer_) || L'-' == buffer_) {
          result->name = NUMBER;
          result->value = L"";
          do {
            result->value += buffer_;
            GetChar();
          } while (std::isdigit(buffer_));
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, NIL> *unused = nullptr) {
        if (L'N' == buffer_) {
          result->name = NIL;
          result->value = L"";
          CHECK(L'I' == GetChar());
          CHECK(L'L' == GetChar());
          GetChar();
          return true;
        }
        
        return false;
      }

      // Rely on wifstream to close itself automatically on destruction.
      std::wifstream input_stream_;
      wchar_t buffer_;
      bool end_of_file_;
    };

    template <typename Parsable>
    struct ParseList {
      static std::vector<Parsable> Do(parser::Tokenizer *tokenizer,
                                      bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<Parsable>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<Parsable> result;
      
        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case OPEN_PARENTHESIS:
            result.emplace_back(Parsable(tokenizer, false));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };

    template <>
    struct ParseList<int> {
      static std::vector<int> Do(parser::Tokenizer *tokenizer,
                                 bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<int>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<int> result;

        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case NUMBER:
            result.push_back(std::stoi(token.value));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };

    template <>
    struct ParseList<std::wstring> {
      static std::vector<std::wstring> Do(parser::Tokenizer *tokenizer,
                                          bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<std::wstring>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<std::wstring> result;

        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case STRING:
            result.push_back(std::move(token.value));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };
    
  }  // namespace parser
}  // namespace monster_avengers
