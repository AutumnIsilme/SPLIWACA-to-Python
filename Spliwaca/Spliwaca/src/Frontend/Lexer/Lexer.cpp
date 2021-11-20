#include "Lexer.h"
#include <regex>
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
//#include "Instrumentor.h"
#include "Log.h"
#include "UtilFunctions.h"

namespace Spliwaca
{
    bool itemInVect(const std::vector<std::string>& v, std::string t)
    {
        for (int i = 0; i < v.size(); i++)
        {
            if (v.at(i) == t)
            {
                return true;
            }
        }
        return false;
    }

    bool itemInVect(const std::vector<char>& v, char t)
    {
        for (int i = 0; i < v.size(); i++)
        {
            if (v.at(i) == t)
            {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<Lexer> Lexer::Create(std::string file)
    {
        return std::shared_ptr<Lexer>(new Lexer(file));
    }

    Lexer::Lexer(std::string fileLocation)
        : m_Tokens(new std::vector<std::shared_ptr<Token>>()), m_FileLocation(fileLocation)
    {
        PROFILE_FUNCTION();
        SPLW_INFO("Beginning file open");
        std::ifstream file;
        file.open(m_FileLocation);
        //char* fileContents;

        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                m_FileString.append(line + "\n");
            }
            //SPLW_TRACE("File contents: {0}", m_FileString);
            //std::cout << fileContents << "\n";
        }
        else
        {
            SPLW_ERROR("Could not open source file: {0}", m_FileLocation);
            system("PAUSE");
            exit(2);
        }

        file.close();
        SPLW_INFO("Closed file");

        //std::shared_ptr<Token> token;
        //token.reset(new Token(TokenType::UnfinishedToken, fileContents.c_str(), 0, 0));

        //m_Tokens->push_back(token);
    }

    void Lexer::makeToken(std::string tokenContents)
    {
        if (tokenContents == "\x04")
        {
            //End of file. Final cleanup time, and return error if we are missing something.
            if (flags & 16)
            {
                SPLW_ERROR("Missing double quote at end of file.");
                return;
            }
            else if (flags & 8)
            {
                SPLW_ERROR("Missing single quote at end of file.");
                return;
            }
            else if (flags & 4)
            {
                flags &= 0b11111011;
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Raw, persistent_contents.c_str(), m_StoredLineNumber, m_StoredColumnNumber)));
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Newline, "\n", m_LineNumber, m_ColumnNumber)));
                return;
            }
            else if (flags & 2)
            {
                if (tokenContents == "*/")
                {
                    flags &= 0b11111101;
                }
                else
                    SPLW_ERROR("Missing end of block comment at end of file.");
                return;
            }
            else if (flags & 1)
            {
                flags &= 0b11111110;
                return;
            }
            m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::eof, "", m_LineNumber, m_ColumnNumber)));
            return;
        }/* ---------------------------------END CLEANUP--------------------------------- */
        else if (flags == 0)
        {
            if (s_KeywordDict.find(tokenContents) != s_KeywordDict.end() && tokenContents != "RAW" && tokenContents != "OUTPUT" && tokenContents != "/*" && tokenContents != "//")
            {
                //We have a keyword!
                m_Tokens->push_back(std::make_shared<Token>(Token(s_KeywordDict.at(tokenContents), tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));

            }
            else if (tokenContents == "/*")
            {
                flags |= 0b00000010;
            }
            else if (tokenContents == "//")
            {
                flags |= 0b00000001;
            }
            else if (tokenContents == "OUTPUT")
            {
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Output, tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));
                flags |= 0b00100100;
                m_StoredColumnNumber = m_ColumnNumber;
                m_StoredLineNumber = m_LineNumber;
            }
            else if (tokenContents == "RAW")
            {
                flags |= 0b00100100;
                m_StoredColumnNumber = m_ColumnNumber;
                m_StoredLineNumber = m_LineNumber;
            }
            else if (tokenContents == "\"")
            {
                flags |= 0b00010000;
                m_StoredColumnNumber = m_ColumnNumber;
                m_StoredLineNumber = m_LineNumber;
            }
            else if (tokenContents == "'")
            {
                flags |= 0b00001000;
                m_StoredColumnNumber = m_ColumnNumber;
                m_StoredLineNumber = m_LineNumber;
            }
            else if (tokenContents == std::string(1, ' ') || tokenContents == "\t" || tokenContents == "\f" || tokenContents == "")// || tokenContents == std::string(1, '\ufeff')) // Whitespace
            {
            }
            else
            {
                //PROFILE_SCOPE("Catch idents and numbers");
                std::smatch m;
                //Use regexes
                bool number = false;
                if ((tokenContents[tokenContents.size() - 1] == 'i' || (tokenContents[tokenContents.size() - 1] <= '9' && tokenContents[tokenContents.size() - 1] >= '0')) && tokenContents[0] <= '9' && tokenContents[0] >= '0')
                {
                    //PROFILE_SCOPE("Check for number");
                    
                    if (std::regex_search(tokenContents, m, std::regex("(\\d+_*)+")) && m[0] == tokenContents) // Matches int regex
                    {
                        if (std::regex_search(tokenContents, m, std::regex("\\d{1,3}(_\\d{3})+|\\d+")) && m[0] != tokenContents)
                        {
                            SPLW_WARN("Style Warning, line {0}, char {1}: Integer literals should have underscores treated as commas.");
                        }
                        m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Int, tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));
                        number = true;
                    }
                    else if (std::regex_search(tokenContents, m, std::regex("(\\d|_)+\\.\\d+")) && m[0] == tokenContents) // Matches float regex
                    {
                        if (std::regex_search(tokenContents, m, std::regex("(\\d{1,3}(_\\d{3})+|\\d+)\\.[0-9]+")) && m[0] != tokenContents)
                        {
                            SPLW_WARN("Style Warning, line {0}, char {1}: Float literals should have underscores treated as commas.");
                        }
                        m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Float, tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));
                        number = true;
                    }
                    else if (std::regex_search(tokenContents, m, std::regex("(\\d|_)+(\\.\\d+)?i")) && m[0] == tokenContents) // Matches complex regex
                    {
                        if (std::regex_search(tokenContents, m, std::regex("(\\d{1,3}(_\\d{3})+|\\d+)(\\.[0-9]+)?i")) && m[0] != tokenContents)
                        {
                            SPLW_WARN("Style Warning, line {0}, char {1}: Complex literals should have underscores treated as commas.");
                        }
                        m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Complex, tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));
                        number = true;
                    }
                }
                if (!number) {
                    //PROFILE_SCOPE("Check ident validity");
                    char invalidChars[] = { '~', '\\', ';', '#', '$', '@', '`', ',', '?', '!', '%', '^', '<', '|', '\'', '&', ')', '*', '/', '+', '[', ']', '.', '"', '=', '{', '}', ':', '>', '(', '-'};
                    int i = -1;
                    for (char c : tokenContents) {
                        i++;
                        for (char d : invalidChars) {
                            if (c == d) {
                                goto invalid;
                            }
                        }
                    }
                    {
                        m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Identifier, tokenContents.c_str(), m_LineNumber, m_ColumnNumber)));
                        return;
                    }
                    {
                    invalid:
                        //Error unexpected characters.
                        SPLW_ERROR("Lexical Error: Unexpected character {0} in string: {1}", tokenContents[i], tokenContents);
                        RegisterLexicalError(0, m_LineNumber, m_ColumnNumber, (uint32_t)tokenContents.size());
                    }
                }
            }
        }
        else if (flags & 32) // First char of RAW
        {
            flags &= 0b11011111;
            if (tokenContents != " ")
            {
                if (tokenContents == "\n")
                {
                    flags &= 0b11111011;
                    m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Raw, persistent_contents.c_str(), m_StoredLineNumber, m_StoredColumnNumber)));
                    m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Newline, "\n", m_LineNumber, m_ColumnNumber)));
                    persistent_contents = "";
                }
                else
                    persistent_contents.append(tokenContents);
            }
        }
        else if (flags & 16) // Double quote
        {
            if (tokenContents == "\"")
            {
                flags &= 0b11101111;
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::String, persistent_contents.c_str(), m_StoredLineNumber, m_StoredColumnNumber)));
                persistent_contents = "";
            }
            else
                persistent_contents.append(tokenContents);
        }
        else if (flags & 8) // Single quote
        {
            if (tokenContents == "'")
            {
                flags &= 0b11110111;
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::String, persistent_contents.c_str(), m_StoredLineNumber, m_StoredColumnNumber)));
                persistent_contents = "";
            }
            else
                persistent_contents.append(tokenContents);
        }
        else if (flags & 4) // Raw
        {
            if (tokenContents == "\n")
            {
                flags &= 0b11111011;
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Raw, persistent_contents.c_str(), m_StoredLineNumber, m_StoredColumnNumber)));
                m_Tokens->push_back(std::make_shared<Token>(Token(TokenType::Newline, "\n", m_LineNumber, m_ColumnNumber)));
                persistent_contents = "";
            }
            else
                persistent_contents.append(tokenContents);
        }
        else if (flags & 2) // Block comment
        {
            if (tokenContents == "*/")
            {
                flags &= 0b11111101;
            }
        }
        else if (flags & 1)  // Comment
        {
            if (tokenContents == "\n")
            {
                flags &= 0b11111110;
            }
        }
        else
        {
            SPLW_CRITICAL("WHY ARE WE IN THE ELSE? THIS SHOULD BE IMPOSSIBLE!!!!!!!!");
        }
        if (tokenContents == "\n")
        {
            m_LineNumber += 1;
            m_ColumnNumber = 0;
        }
        else
            m_ColumnNumber += (uint32_t)tokenContents.size();
    }

    std::shared_ptr<std::vector<std::shared_ptr<Token>>> Lexer::MakeTokens()
    {
        std::string s = m_FileString;

        std::string intermediate = "";
        std::vector<char> splitChars = { ' ', '\n', '\t', '\f', '(', ')', '[', ']', '+', '-', '/', '*', '!', '=', '%', '^', '&', '|', '<', '>', ',', '"', '\'', '.', ':' };
        std::vector<std::string> splitDuoStrings = { "/*", "*/", "//", "**", "??", "&&", "==", "!=", "<=", ">=", "<<", ">>", "<-", "->", "||", "\\'", "\\\"" };
        std::vector<std::string> splitTrioStrings = { "=/=", "===" };

        int i = 0;
        while ((s[i] & 0xffff) == 0xffef || (s[i] & 0xffff) == 0xffbb || (s[i] & 0xffff) == 0xffbf)
        {
            i++;
        }

        while (true)
        {
            std::string c = std::string(1, s[i]);
            std::string duo = c; (i < s.size() - 1) ? duo += s[i + 1] : duo += "";
            std::string trio = duo; (i < s.size() - 2) ? trio += s[i + 2] : trio += "";

            if (trio == "=/=" || trio == "===")//itemInVect(splitTrioStrings, trio))
            {
                if (intermediate != "")
                    makeToken(intermediate);
                intermediate = c + s[i + 1] + s[i + 2];
                //intermediate += s[i + 1];
                //intermediate += s[i + 2];
                makeToken(intermediate);
                intermediate = "";
                i += 2;
            }
            else if (itemInVect(splitDuoStrings, duo))
            {
                if (intermediate != "")
                    makeToken(intermediate);
                intermediate = c + s[i+1];
                //intermediate += s[i + 1];
                makeToken(intermediate);
                intermediate = "";
                i++;
            }
            else if (itemInVect(splitChars, c[0]) || s_KeywordDict.find(c) != s_KeywordDict.end())
            {
                if (c != "." || charInStr(alphabetCharacters, intermediate[0]))
                {
                    if (intermediate != "")
                        makeToken(intermediate);
                    intermediate = c;
                    makeToken(intermediate);
                    intermediate = "";
                }
                else
                {
                    intermediate += c;
                }
            }
            else
            {
                intermediate += c;
            }
            i++;
            if (i >= s.size())
                break;
        }
        makeToken(intermediate);
        makeToken("\x004");
        return m_Tokens;
    }
}
