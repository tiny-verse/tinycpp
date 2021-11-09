#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cctype>

namespace tinycpp
{
    namespace keywords
    {
        // tinyc
        const std::string struct_ = "struct";
        const std::string integer_ = "int";
        const std::string character_ = "char";
        const std::string double_ = "double";
        // tinycpp
        const std::string namespace_ = "namespace";
        const std::string class_ = "class";
        const std::string trait_ = "trait";
    }

    class KeywordCase
    {
    private:
        bool isValid_ = true;
        const std::string & text_;
    public:
        KeywordCase(const std::string & text) : text_{text} { }
    public:
        bool isValid() const
        {
            return isValid_;
        }

        void reset()
        {
            isValid_ = true;
        }

        void check(char c, int index)
        {
            if (!isValid_) return;
            if (index > text_.length())
            {
                isValid_ = false;
                return;
            }
            isValid_ = text_[index] == c;
        }
    };

    class IdentifierCase
    {
    private:
        bool isValid_ = true;
    public:
        bool isValid() const
        {
            return isValid_;
        }

        void reset()
        {
            isValid_ = true;
        }

        void check(char c, int index)
        {
            isValid_ = (index == 0 ? std::isalpha(c) : isValid_ && std::isalnum(c)) || c == '_';
        }
    };

    class NumberCase 
    {
    private:
        bool isValid_ = true;
        int separations_ = 0;
    public:
        bool isInteger() const
        {
            return isValid_ && separations_ == 0;
        }

        bool isDouble() const 
        {
            return isValid_ && separations_ == 1;
        }

        void reset()
        {
            isValid_ = true;
            separations_ = 0;
        }

        void check(char c)
        {
            if (!isValid_) return;
            if (c == '.') 
            {
                separations_++;
                return;
            }
            isValid_ = isValid_ && std::isdigit(c);
        }
    };

    class CharacterCase 
    {
    private:
        bool isValid_ = true;
    public:
        bool isValid() const
        {
            return isValid_;
        }

        void reset()
        {
            isValid_ = true;
        }

        void check(char c, int index)
        {
            switch (index)
            {
                case 0:
                case 2:
                    isValid_ = isValid_ && c == '\'';
                    break;
                case 1: break;
                default: isValid_ = false;
            }
        }
    };

    class AST
    {
    public:
        // types
        class Node {};
        class Root {};
        class Variable {};
        class Namespace {};
        class Class {};
        class Struct {};
    };



    class ParseData
    {
    public:
        std::istream & input;
        std::stringstream buffer;
        AST::Node * currentNode;
        KeywordCase caseNamespace;
        KeywordCase caseClass;
        IdentifierCase caseIdentifier;
        CharacterCase caseCharacter;
        NumberCase caseNumber;

    public:
        ParseData(std::istream & input) : input{input}
            , caseNamespace{keywords::namespace_}
            , caseClass{keywords::class_}
        {   }

    public:
        void resetWord()
        {
            buffer.str("");
            caseNamespace.reset();
            caseClass.reset();
            caseIdentifier.reset();
            caseCharacter.reset();
            caseNumber.reset();
        }
    };


    void parseNamespace(ParseData & data)
    {
        data.resetWord();
    }

    void parseVariable(ParseData & data)
    {
        /// TODO: parse variable
    }

    // returns { Parse Error | { Namespace | Class | Struct | Trait | Variable  } Syntax Tree }
    void parseRoot(ParseData & data)
    {
        data.resetWord();
        for (int letterCount = 0; !data.input.eof();)
        {
            char c;
            data.input >> c;
            if (std::isspace(c))
            {
                letterCount = 0;
                // what is the word ?
                if (data.buffer.tellp() == 0) continue; // didnt found word yet
                if (data.caseNamespace.isValid()) parseNamespace(data);
                if (data.caseIdentifier.isValid()) parseVariable(data);
                else assert(false && "failed to parse root.");
            }
            else
            {
                data.buffer << c;
                data.caseNamespace.check(c, letterCount);
                data.caseClass.check(c, letterCount);
                data.caseIdentifier.check(c, letterCount);
                letterCount++;
            }
        }
    }

    void parse(std::istream & input)
    {
        ParseData data {input};
        parseRoot(data);
    }
}