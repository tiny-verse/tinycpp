#include <memory>
#include <unordered_set>

#include "common/symbol.h"
#include "common/lexer.h"
#include "common/parser.h"
#include "common/ast.h"

#include "tinycpp/ast.h"

namespace tinycpp {

    using Lexer = tiny::Lexer;
    using Token = tiny::Token;
    using Symbol = tiny::Symbol;
    using ASTBase = tiny::ASTBase;
    using ParserBase = tiny::ParserBase;
    using ParserError = tiny::ParserError;

    namespace symbols 
    {
        static Symbol Namespace {"namespace"};
        static Symbol Class {"class"};
    }

    class Parser : public ParserBase {
    public:
        static std::unique_ptr<ASTBase> ParseFile(std::string const & filename) {
            Parser p{Lexer::TokenizeFile(filename)};
            std::unique_ptr<ASTBase> result{p.PROGRAM()};
            p.pop(Token::Kind::EoF);
            return result;
        }

        static std::unique_ptr<ASTBase> Parse(std::string const & code, std::string const & filename) {
            Parser p{Lexer::Tokenize(code, filename)};
            std::unique_ptr<ASTBase> result{p.REPL()};
            p.pop(Token::Kind::EoF);
            return result;
        }

    protected:

        Parser(std::vector<Token> && tokens):
            ParserBase{std::move(tokens)} {
        }

        /** Determines if given token is a valid user identifier.
         */
        bool isIdentifier(Token const & t) {
            if (t != Token::Kind::Identifier)
                return false;
            if (t == Symbol::KwBreak
                || t == Symbol::KwCase
                || t == Symbol::KwCast
                || t == Symbol::KwChar
                || t == Symbol::KwContinue
                || t == Symbol::KwDefault
                || t == Symbol::KwDo
                || t == Symbol::KwDouble
                || t == Symbol::KwElse
                || t == Symbol::KwFor
                || t == Symbol::KwIf
                || t == Symbol::KwInt
                || t == Symbol::KwReturn
                || t == Symbol::KwStruct
                || t == Symbol::KwSwitch
                || t == Symbol::KwTypedef
                || t == Symbol::KwVoid
                || t == Symbol::KwWhile)
                 return false;
            return true;
        }

        /** \name Types Ambiguity

            tinyC shares the unfortunate problem of C and C++ grammars which makes it impossible to determine whether an expression is type declaration or an expression simply from the grammar. take for instance

                foo * a;

            Is this declaration of variable of name `a` with type `foo*`, or is this multiplication of two variables `foo` and `a`. Ideally this ambiguity should be solved at the grammar level such as introducing `var` keyword, or some such, but for educational purposes we have decided to keep this "feature" in the language.

            The way to fix this is to make the parser track all possible type names so that an identifier can be resolved as being either variable, or a type, thus removing the ambiguity. In tiny's case this is further complicated by the parser being state-full in the repl mode.
         */

        std::unordered_set<Symbol> possibleTypes_;
        std::vector<Symbol> possibleTypesStack_;

        /** Returns true if given symbol is a type.

            Checks both own tentative records and the frontend's valid records.
         */
        bool isTypeName(Symbol name) const;

        /** Adds given symbol as a tentative typename.

            Note that same typename can be added multiple times for forward declared structs.
         */
        void addTypeName(Symbol name) {
            possibleTypes_.insert(name);
            possibleTypesStack_.push_back(name);
        }

        /** We need new position because when reverting, the tentative types names that were created *after* the savepoint must be unrolled as well.
         */
        class Position {
        private:
            friend class Parser;

            ParserBase::Position position_;
            size_t possibleTypesSize_;

            Position(ParserBase::Position position, size_t typesSize):
                position_{position},
                possibleTypesSize_{typesSize} {
            }
        };

        Position position() {
            return Position{ParserBase::position(), possibleTypesStack_.size()};
        }

        void revertTo(Position const & p) {
            ParserBase::revertTo(p.position_);
            while (possibleTypesStack_.size() > p.possibleTypesSize_) {
                possibleTypes_.erase(possibleTypesStack_.back());
                possibleTypesStack_.pop_back();
            }
        }

        void raiseError(const std::string & message) const {
            throw ParserError{message, top().location(), eof()};
        }

        void expect(Symbol & symbol) 
        {
            if (!condPop(symbol)) {
                raiseError(STR("Expected keyword \"" << symbols::Namespace.name() <<  "\", but found: " << top()));
            }
        }

        /*  Parsing

            Nothing fancy here, just a very simple recursive descent parser built on the basic framework.
         */
        std::unique_ptr<AST> PROGRAM()
        {
            while (!eof()) {
                expect(symbols::Namespace);
                return NAMESPACE();
            }
            return nullptr;
        }

        std::unique_ptr<AST> REPL() {

        }

        std::unique_ptr<AST> NAMESPACE() {
            std::unique_ptr<AST::Namespace> result{new AST::Namespace{pop()}};
            expect(Symbol::ParOpen);
            if (condPop(symbols::Class)) {
                result->add(CLASS());
            } else {
                result->add(std::make_unique<ASTRaw>(pop()));
            }
            expect(Symbol::ParClose);
        }

        std::unique_ptr<AST> CLASS() { 
            std::unique_ptr<AST::Class> result{new AST::Class{pop()}};
            expect(Symbol::ParOpen);
            /// TODO: continue
            expect(Symbol::ParClose);
            return result;
        }
    }; // class Parser

} // namespace tinycpp