using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace winbind_wrapper
{
    namespace parser
    {
        enum TokenName
        {
            OPEN_PARENTHESIS = 0, // "("
            CLOSE_PARENTHESIS, // ")"
            KEYWORD, // ":[a-zA-z0-9].+"
            STRING, // "\".*\""
            TRUE_VALUE, // "[Tt]"
            NUMBER, // "[0-9]+"
            NIL, // "[Nn][Ii][Ll]"
            INVALID_TOKEN,
            NUMBER_OF_TOKEN
        }

        struct Token
        {
            public TokenName name;
            public string value;

            public Token(TokenName name_, string value_)
            {
                name = name_;
                value = value_;
            }

            public void DebugPrint()
            {
                Console.WriteLine("{0} ({1})",
                    Enum.GetName(typeof(TokenName), name),
                    value);
            }
        }

        class Tokenizer
        {
            public Tokenizer(string input_string)
            {
                text = input_string;
            }

            public bool Next(out Token token)
            {
                while (pos < text.Length && Char.IsWhiteSpace(text[pos]))
                {
                    ++pos;
                }
                token = new Token();
                if (pos >= text.Length) return false;
                if (!(ReadOpenParenthesis(out token) ||
                      ReadCloseParenthesis(out token) ||
                      ReadKeyword(out token) ||
                      ReadString(out token) ||
                      ReadTrueValue(out token) ||
                      ReadNumber(out token) ||
                      ReadNil(out token)))
                {
                    token.name = TokenName.INVALID_TOKEN;
                }
                return true;
            }

            public bool Expect(TokenName name)
            {
                Token token = new Token();
                return this.Next(out token) && token.name == name;
            }

            public bool ExpectNumber(out int result)
            {
                Token token = new Token();
                if (this.Next(out token) && token.name == TokenName.NUMBER)
                {
                    result = Convert.ToInt32(token.value);
                    return true;
                }
                result = 0;
                return false;
            }

            public bool ExpectIntegerList(out List<int> result)
            {
                Expect(TokenName.OPEN_PARENTHESIS);
                Token token = new Token();
                result = new List<int>();
                while (Next(out token))
                {
                    if (token.name == TokenName.CLOSE_PARENTHESIS)
                    {
                        return true;
                    }
                    else if (token.name != TokenName.NUMBER)
                    {
                        return false;
                    }
                    result.Add(Convert.ToInt32(token.value));
                }
                return false;
            }

            public bool Empty()
            {
                return pos >= text.Length;
            }

            private bool ReadOpenParenthesis(out Token result)
            {
                if ('(' == text[pos])
                {
                    pos++;
                    result = new Token(TokenName.OPEN_PARENTHESIS, "");
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadCloseParenthesis(out Token result)
            {
                if (')' == text[pos])
                {
                    pos++;
                    result = new Token(TokenName.CLOSE_PARENTHESIS, "");
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadKeyword(out Token result)
            {
                if (':' == text[pos])
                {
                    result = new Token(TokenName.KEYWORD, "");
                    pos++;
                    while (pos < text.Length && (!Char.IsWhiteSpace(text[pos])))
                    {
                        result.value += text[pos];
                        pos++;
                    }
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadString(out Token result)
            {
                if ('"' == text[pos])
                {
                    result = new Token(TokenName.STRING, "");
                    bool escape = false;
                    while (pos < text.Length)
                    {
                        pos++;
                        if ('"' == text[pos] && !escape)
                        {
                            break;
                        }
                        if ('\\' == text[pos])
                        {
                            escape = true;
                        }
                        else
                        {
                            escape = false;
                            if ('"' == text[pos])
                            {
                                result.value += '|';
                            }
                            else
                            {
                                result.value += text[pos];
                            }
                        }
                    }

                    if ('"' != text[pos])
                    {
                        return false;
                    }
                    pos++;
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadTrueValue(out Token result)
            {
                if ('T' == text[pos])
                {
                    pos++;
                    result = new Token(TokenName.TRUE_VALUE, "");
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadNumber(out Token result)
            {
                if (Char.IsDigit(text[pos]) || '-' == text[pos])
                {
                    result = new Token(TokenName.NUMBER, "");
                    do
                    {
                        result.value += text[pos];
                        pos++;
                    } while (pos < text.Length && Char.IsDigit(text[pos]));
                    return true;
                }
                result = new Token();
                return false;
            }

            private bool ReadNil(out Token result)
            {
                if ('N' == text[pos])
                {
                    result = new Token(TokenName.NIL, "");
                    pos += 3;
                    return true;
                }
                result = new Token();
                return false;
            }

            private string text;
            private int pos;
        }
    }
}
