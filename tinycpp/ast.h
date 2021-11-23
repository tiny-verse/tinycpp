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
        void print(ASTPrettyPrinter & printer) const override {
            assert(false && "not implemented");
        }
    public:
        class Variable;
        class FunctionPointer;
        class Struct;
        class Function;
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


    class AST::Variable : public AST {
    private:
        Symbol type_;
        Symbol name_;
        std::unique_ptr<AST> assignment_;
        uint arraySize_;
    public:
        Variable(Token const & nameToken, Symbol type, Symbol name, uint arraySize): AST{nameToken}
            ,type_{type}
            ,name_{name}
            ,arraySize_{arraySize}
        { }
    public:
        void print(ASTContext & context) {
            assert(false && "not implemented");
        }
        void takeAsAssignemnt(std::unique_ptr<AST> & assignment) {
            assignment_ = std::move(assignment);
        }
    }; // class AST::Variable


    class AST::FunctionPointer : public AST {
    private:
        Symbol returnType_;
        Symbol name_;
        std::vector<std::unique_ptr<AST>> parameters_;
    public:
        FunctionPointer(
            Token const & nameToken,
            Symbol returnType,
            Symbol name
        ) : AST(nameToken)
            ,returnType_{returnType_}
            ,name_{name}
        { }
    public:
        void print(ASTContext & context) {
            assert(false && "not implemented");
        }
        void takeAsParameter(std::unique_ptr<AST> & ast) {
            parameters_.push_back(std::move(ast));
        }
    }; // class AST::FunctionPointer


    class AST::Struct : public AST {
    private:
        Symbol name_;
        std::vector<std::unique_ptr<AST>> fields_;
    public:
        Struct(Token const & token, Symbol name) : AST{token}, name_{name} { }
    public:
        void print(ASTContext & context) override {
            auto printer = context.getPrinter();
            printer << "struct ";
            context.printName(name_, printer.keyword);
            printer << "{\n";
            printer.indent();
            for (auto & ast : fields_)
            {
                ast->print(context);
                printer.newline();
            }
            printer.dedent();
            printer << "}\n";
        }

        void takeAsField(std::unique_ptr<AST> & ast) {
            fields_.push_back(std::move(ast));
        }
    }; // class AST::Struct


    class AST::Function : public AST {
    private:
        Symbol type_;
        Symbol name_;
        std::vector<std::unique_ptr<AST>> parameters_;
    public:
        Function(Token const & token, Symbol type, Symbol name): AST{token}
            ,type_{type}
            ,name_{name}
        { }
    public:
        void takeAsParameter(std::unique_ptr<AST> & ast) {
            parameters_.push_back(std::move(ast));
        }
    }; // class AST::Function


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
            printer << "struct ";
            context.printName(name_, printer.keyword);
            printer << "{\n";
            printer.indent();
            for (auto & ast : fields_)
            {
                ast->print(context);
                printer.newline();
            }
            printer.dedent();
            printer << "}\n";
            for (auto & ast : functions_)
            {
                ast->print(context);
                printer.newline();
            }
        }

        void takeAsField(std::unique_ptr<AST> & ast) {
            fields_.push_back(std::move(ast));
        }

        void takeAsMethod(std::unique_ptr<AST> & ast) {
            fields_.push_back(std::move(ast));
        }
    }; // class AST::Class

} // namespace tinycpp