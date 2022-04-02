#ifndef _COMPILER_HPP_
#define _COMPILER_HPP_
#include <string>
#include <functional>
#include <array>
#include <optional>
#include <tuple>
#include <limits>
#include "token.hpp"
#include "scanner.hpp"
#include "chunk.hpp"
#include "object.hpp"
#include "common.hpp"

enum class Precedence : std::uint32_t;

enum class CompileResult
{
    COMPILE_OK,
    COMPILE_ERROR,
    COMPILE_INTERNAL_ERROR,
};
enum class FunctionType
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT,
};
using CompileReturn = std::tuple<CompileResult, std::optional<std::shared_ptr<FunctionObject>>>;
using TryFindInternedStringFunc = std::function<std::optional<std::shared_ptr<StringObject>>(std::string &)>;
using AddStringToInternFunc = std::function<void(std::shared_ptr<StringObject>)>;

struct StringInternProps
{
    TryFindInternedStringFunc tryFindInternedString;
    AddStringToInternFunc addStringToIntern;
};

class Compiler
{
public:
    using ParseFn = std::function<void(bool)>;
    explicit Compiler();
    explicit Compiler(StringInternProps);
    Compiler(const Compiler &&other);
    Compiler(Compiler *other, FunctionType);

    CompileReturn compile(std::string &);

private:
    struct Parser
    {
        Token current;
        Token previous;
        bool hadError = false;
        bool panicMode = false;
    };

    explicit Compiler(FunctionType);
    explicit Compiler(Parser *, Scanner *, FunctionType);

    struct Local
    {
        Token name;
        int depth = 0;
        bool isCaptured = false;
    };
    struct Internals
    {
        std::shared_ptr<FunctionObject> function;
        FunctionType type;
        std::array<Local, UINT8_COUNT> locals;
        int localCount;
        std::array<Upvalue, UINT8_COUNT> upvalues;
        int scopeDepth;
    };
    struct ParseRule
    {
        std::optional<ParseFn> prefix;
        std::optional<ParseFn> infix;
        Precedence precedence;
    };
    struct ClassCompiler
    {
        ClassCompiler *enclosing = nullptr;
        bool hasSuperclass = false;
    };

    void expression();
    void block();
    void function(FunctionType);
    void functionParameters();
    void method();
    void classDeclaration();
    void funDeclaration();
    void varDeclaration();
    void statement();
    void declaration();
    void printStatement();
    void returnStatement();
    void whileStatement();
    void forStatement();
    void synchronize();
    void expressionStatement();
    void ifStatement();
    void number(bool);
    void grouping(bool);
    void unary(bool);
    void binary(bool);
    void call(bool);
    void dot(bool);
    void literal(bool);
    void string(bool);
    void variable(bool);
    void super_(bool);
    void this_(bool);
    void namedVariable(const Token &, bool);
    void advance();
    void errorAtCurrent(const char *) const;
    void error(const char *) const;
    void errorAt(const Token &, const char *) const;
    void consume(TokenType, const char *message);
    bool check(TokenType);
    bool match(TokenType);
    std::shared_ptr<FunctionObject> endCompiler();
    void beginScope();
    void endScope();
    void emitByte(std::uint8_t);
    void emitByte(Opcode);
    void emitBytes(std::uint8_t, std::uint8_t);
    void emitBytes(Opcode, std::uint8_t);
    void emitBytes(Opcode, Opcode);
    void emitLoop(int);
    int emitJump(Opcode);
    void emitReturn();
    void emitConstant(Value);
    void patchJump(int);
    ParseRule &getRule(TokenType &);
    std::uint8_t makeConstant(Value);
    void parsePrecedence(Precedence);
    std::uint8_t identifierConstant(const Token &);
    bool identifiersEqual(const Token &, const Token &) const;
    int resolveLocal(const Token &) const;
    int addUpvalue(std::uint8_t, bool);
    int resolveUpvalue(const Token &);
    void addLocal(const Token);
    void addConstant(const Token);
    void declareVariable();
    std::uint8_t parseVariable(const char *);
    void markInitialized();
    void defineVariable(std::uint8_t);
    std::uint8_t argumentList();
    void and_(bool);
    void or_(bool);
    std::shared_ptr<StringObject> copyString(const char *, int);
    Chunk *currentChunk();
    void initInternals(FunctionType type);
    void initRules();
    Parser *parser;
    Scanner *scanner;
    Internals internals;
    Compiler *const enclosing = nullptr;
    std::array<ParseRule, static_cast<int>(TokenType::EOF_) + 1> rules;
    std::optional<StringInternProps> stringInternProps = std::nullopt;
    ClassCompiler *currentClass = nullptr;
};
#endif