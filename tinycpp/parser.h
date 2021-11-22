// standard
#include <memory>
#include <unordered_set>

// external
#include "common/symbol.h"
#include "common/lexer.h"
#include "common/parser.h"
#include "common/ast.h"

// internal
#include "tinycpp/ast.h"

namespace tinycpp {

    using Lexer = tiny::Lexer;
    using Token = tiny::Token;
    using Symbol = tiny::Symbol;
    using ASTBase = tiny::ASTBase;
    using ParserBase = tiny::ParserBase;
    using ParserError = tiny::ParserError;

    namespace symbols {
        static Symbol Class {"class"};
        static Symbol This {"this"};
        static Symbol Base {"base"};
        static Symbol Trait {"trait"};
    }

    class Parser : public ParserBase {
    public:
        static std::unique_ptr<ASTBase> ParseFile(std::string const & filename) {
            Parser p{Lexer::TokenizeFile(filename)};
            std::unique_ptr<ASTBase> result{p.PROGRAM()};
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

        std::unordered_set<Symbol> possibleTypes_;
        std::vector<Symbol> possibleTypesStack_;

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

        // === Case Rollback ===
        /// TODO: possibleTypesStack, position(), revretTo(): are helpful if a case could fail,
        ///       so we may return and try another case.
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
        // =====================


        void throwError(const std::string & message) const {
            throw ParserError{message, top().location(), eof()};
        }

        Symbol popIdentifier() {
            auto token = pop(Token::Kind::Identifier);
            return token.valueSymbol();
        }

        int popInteger(bool isSigned) {
            auto token = pop(Token::Kind::Integer);
            int value = token.valueInt();
            if (!isSigned && value < 0) {
                throwError(STR("Expected unsigned integer, but got signed: " << value));
            }
            return value;
        }

        std::unique_ptr<AST> FIELD() {
            if (condPop(Symbol::KwTypedef)) { // function ptr
                // * type
                auto sReturnType = popIdentifier();
                // * identifier
                pop(Symbol::ParOpen);
                pop(Symbol::Mul);
                auto tName = top();
                auto sName = popIdentifier();
                pop(Symbol::ParClose);
                // * parameters
                auto astFuncPtr = std::make_unique<AST::FunctionPointer>(tName, sReturnType, sName);
                pop(Symbol::ParOpen);
                do {
                    auto astParam = FIELD();
                    astFuncPtr->addParameter(astParam);
                } while(condPop(Symbol::Colon));
                pop(Symbol::ParClose);
                return astFuncPtr;
            } else {
                auto sType = popIdentifier();
                addTypeName(sType);
                while (condPop(Symbol::Mul)); // skip pointer part declaration
                auto tName = top();
                auto sName = popIdentifier();
                uint arraySize = 0;
                if (condPop(Symbol::SquareOpen)) {
                    arraySize = popInteger(false);
                    pop(Symbol::SquareClose);
                }
                auto ast = std::make_unique<AST::Variable>(tName, sType, sName, arraySize);
                pop(Symbol::Semicolon);
                return ast;
            }
        }

        std::unique_ptr<AST> STRUCT() {
            pop(Symbol::KwStruct);
            auto name = popIdentifier();
            addTypeName(name);
            pop(Symbol::ParOpen);
            assert(false && "not implemented");
            pop(Symbol::ParClose);
        }

        std::unique_ptr<AST> FUNCTION() {
            assert(false && "not implemented");
        }

        std::unique_ptr<AST> CLASS() { 
            std::unique_ptr<AST::Class> result(new AST::Class{ pop() });
            pop(Symbol::ParOpen);
            assert(false && "not implemented");
            pop(Symbol::ParClose);
            return result;
        }

        std::unique_ptr<AST> PROGRAM()
        {
            while (!eof()) {
                if (condPop(Symbol::KwStruct)) {
                    assert(false && "not implemented");
                }
                else if (condPop(symbols::Class)) {
                    assert(false && "not implemented");
                }
                else if (condPop(symbols::Trait)) {
                    assert(false && "not implemented");
                }
                else // pop type
                {
                    assert(false && "not implemented");
                    // pop name
                    // if '(' -> method
                    // else if ';' -> variable
                }
            }
            return nullptr;
        }

    }; // class Parser

} // namespace tinycpp