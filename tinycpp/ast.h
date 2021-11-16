#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include "common/ast.h"

namespace tinycpp {

    using color = tiny::color;
    using Token = tiny::Token;
    using Symbol = tiny::Symbol;
    using ASTBase = tiny::ASTBase;
    using ASTPrettyPrinter = tiny::ASTPrettyPrinter;

    struct ASTContext {
    private:
        ASTPrettyPrinter printer_;
        std::vector<Symbol> currentDomain_;
    public:
        const ASTPrettyPrinter & getPrinter() const {
            return printer_;
        }
    public:
        void printName(Symbol name, color color) {
            printer_ << color;
            for (auto & name : currentDomain_) {
                printer_ << name.name() << "_";
            }
            printer_ << name.name();
            printer_ << color::reset;
        }

        void enterNamespace(Symbol name){
            currentDomain_.push_back(name);
        }

        void exitNamespace() {
            currentDomain_.pop_back();
        }
    }; // struct ASTContext


    class AST : public ASTBase {
    protected:
        AST(Token const & token) : ASTBase{token} { }
    public:
        virtual void print(ASTContext & context) = 0;
    public:
        class Namespace;
        class Class;
    }; // class AST


    class ASTRaw : public AST {
    private:
        std::stringstream merge_;
    public:
        ASTRaw(Token const & token) : AST{token} { }
    public:
        void print(ASTContext & context) override {
            auto printer = context.getPrinter();
            printer << merge_.str();
        }

        void add(Token & token) {
            merge_ << token;
        }
    }; // class ASTRaw


    class AST::Namespace : public AST {
    private:
        std::vector<std::unique_ptr<AST>> content_;
        Symbol name_;
    public:
        Namespace(Token const & token) : AST{token}, name_{token.valueSymbol()} { }
    public:
        void print(ASTContext & context) override {
            context.enterNamespace(name_);
            for (auto & ast : content_) {
                ast->print(context);
            }
            context.exitNamespace();
        }

        void add(std::unique_ptr<AST> && ast) {
            content_.push_back(ast);
        }
    }; // class AST::Namespace


    class AST::Class : public AST {
    private:
        Symbol name_;
        std::vector<std::unique_ptr<AST>> fields_;
        std::vector<std::unique_ptr<AST>> functions_;
    public:
        Class(Token const & token) : AST{token}, name_{token.valueSymbol()} { }
    public:
        void print(ASTContext & context) override {
            auto printer = context.getPrinter();
            printer << "struct " << name_.name() << "{";
            printer.indent();
            printer.newline();
            for (auto & ast : fields_)
            {
                ast->print(context);
                printer.newline();
            }
            printer.dedent();
            printer << "}";
            for (auto & ast : functions_)
            {
                ast->print(context);
                printer.newline();
            }
        }

        void addField(std::unique_ptr<AST> & ast) {
            fields_.push_back(ast);
        }

        void addFunction(std::unique_ptr<AST> & ast) {
            fields_.push_back(ast);
        }
    }; // class AST::Class

} // namespace tinycpp