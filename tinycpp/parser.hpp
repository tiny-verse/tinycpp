#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cctype>

namespace tinycpp {
    namespace keywords {
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

    struct ParseInput
    {
    private:
        std::istream & stream_;
    public:
        ParseInput(std::istream & stream) : stream_{stream} { }
    public:
        bool isComplete() const { return stream_.eof(); }

        int readCurrent() { return stream_.peek(); }

        char takeCurrent() {
            char c;
            stream_ >> c;
        }

        void skipWhitespaces() {
            do {
                int code = readCurrent();
                if (code == EOF || !std::isspace(code)) return;
                takeCurrent();
            } while(true);
        }
    };

    class KeywordCase {
    private:
        bool isValid_ = true;
        const std::string & text_;
    public:
        KeywordCase(const std::string & text) : text_{text} { }
    public:
        bool isValid() const { return isValid_; }

        void reset() { isValid_ = true; }

        void check(char c, int index) {
            if (!isValid_) return;
            if (index > text_.length()) {
                isValid_ = false;
                return;
            }
            isValid_ = text_[index] == c;
        }
    };

    class IdentifierCase {
    private:
        bool isValid_ = true;
    public:
        bool isValid() const { return isValid_; }

        void reset() { isValid_ = true; }

        void check(char c, int index) {
            isValid_ = (index == 0 ? std::isalpha(c) : isValid_ && std::isalnum(c)) || c == '_';
        }
    };

    class NumberCase {
    private:
        bool isValid_ = true;
        int separations_ = 0;
    public:
        bool isInteger() const { return isValid_ && separations_ == 0; }

        bool isDouble() const { return isValid_ && separations_ == 1; }

        void reset() {
            isValid_ = true;
            separations_ = 0;
        }

        void check(char c) {
            if (!isValid_) return;
            if (c == '.') {
                separations_++;
                return;
            }
            isValid_ = isValid_ && std::isdigit(c);
        }
    };

    class CharacterCase {
    private:
        bool isValid_ = true;
    public:
        bool isValid() const { return isValid_; }

        void reset() { isValid_ = true; }

        void check(char c, int index) {
            switch (index) {
                case 0:
                case 2:
                    isValid_ = isValid_ && c == '\'';
                    break;
                case 1: break;
                default: isValid_ = false;
            }
        }
    };

    class AST {
    public:
        // types
        class Node {};
        class Root {};
        class Variable {};
        class Namespace {};
        class Class {};
        class Struct {};
    };



    class Parser {
    private:
        ParseInput input_;

        KeywordCase caseNamespace_;
        KeywordCase caseClass_;

        IdentifierCase caseIdentifier_;
        CharacterCase caseCharacter_;
        NumberCase caseNumber_;

    private:
        void resetCases() {
            caseNamespace_.reset();
            caseClass_.reset();
            caseIdentifier_.reset();
            caseCharacter_.reset();
            caseNumber_.reset();
        }

        void reach(char target) 
        {
            input_.skipWhitespaces();
            assert(!input_.isComplete() && "Parse expectation was not fullfilled.");
            char c = input_.takeCurrent();
            assert(c == target && "Parse expectation was not fullfilled.");
        }

        void processNamespace() {
            /* Draft:
             [*] skip whitespaces and expect 'namespace'
             [*] skip whitespaces and expect(and save) identifier
             [*] skip whitespaces and expect '{'
             [A] try parse global variable, or class, or function, or struct.
             [*] skip whitespaces and expect '}' or go to [A]
             [END]
             */

            resetCases();
            reach('{');
            // TODO: continue parse
            reach('}');
        }

        void processField() {
            /* Draft:
             [*] skip whitespaces and expect(and save) type
             [*] skip whitespaces and expect(and save) identifier
             [*] skip whitespaces and expect ';'
             [END]
             */
        }

    public:
        Parser(std::istream & input) : input_{input}
            , caseNamespace_{keywords::namespace_}
            , caseClass_{keywords::class_}
        {
            /// TODO: continue parsing

            /* Draft:
             [1] process input as namespace
             [2] go to [1] if input is not empty
             [3] end
             */

            // do 
            // {
            //     input_.skipWhitespaces();
            //     caseNamespace_.
            // }
            // for () {
            //     if () {
            //         // what is the word ?
            //         if (caseNamespace_.isValid()) processNamespace();
            //         if (caseIdentifier_.isValid()) processVariable();
            //         else assert(false && "failed to parse root.");
            //     } else {
            //         caseNamespace_.check(c);
            //         caseClass_.check(c);
            //         caseIdentifier_.check(c);
            //     }
            // }
        }
    };
}