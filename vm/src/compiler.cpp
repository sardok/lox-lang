#include <string>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <exception>
#include <algorithm>
#include <sstream>
#include "compiler.hpp"
#include "scanner.hpp"
#include "chunk.hpp"
#include "object.hpp"
#ifdef DEBUG_PRINT_CODE
#include "disassembler.hpp"
#endif

using std::bind;
using std::make_optional;
using std::make_tuple;
using std::move;
using std::nullopt;
using std::shared_ptr;
using std::uint8_t;

enum class Precedence : std::uint32_t
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
};

template <typename E>
constexpr std::underlying_type_t<E> to_underlying(E enumerator)
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

Precedence operator+(const Precedence &precedence, int addend)
{
    auto res = static_cast<int>(precedence) + addend;
    auto normalized = std::min(std::max(res, 0), static_cast<int>(Precedence::PREC_PRIMARY));
    return static_cast<Precedence>(normalized);
}

Compiler::Compiler() : Compiler{FunctionType::TYPE_SCRIPT}
{
}

Compiler::Compiler(FunctionType type)
{
    initRules();
    initInternals(type);
}

Compiler::Compiler(StringInternProps stringInternProps) : Compiler(FunctionType::TYPE_SCRIPT)
{
    this->stringInternProps = move(stringInternProps);
}

Compiler::Compiler(const Compiler &&other)
{
    this->initRules();
    parser = move(other.parser);
    scanner = move(other.scanner);
    stringInternProps = move(other.stringInternProps);
    currentClass = other.currentClass;
    this->initInternals(other.internals.type);
}

Compiler::Compiler(Compiler *other, FunctionType type) : enclosing{other}
{
    initRules();
    parser = other->parser;
    scanner = other->scanner;
    stringInternProps = other->stringInternProps;
    currentClass = other->currentClass;
    initInternals(type);
}

CompileReturn Compiler::compile(std::string &source)
{
    if (this->scanner || this->parser)
        return make_tuple(CompileResult::COMPILE_INTERNAL_ERROR, nullopt);

    Scanner scanner{source};
    this->scanner = &scanner;

    Parser parser;
    this->parser = &parser;

    advance();
    while (!match(TokenType::EOF_))
    {
        declaration();
    }

    if (this->parser->hadError)
        return make_tuple(CompileResult::COMPILE_ERROR, nullopt);

    auto function = endCompiler();

    this->scanner = nullptr;
    this->parser = nullptr;

    return make_tuple(CompileResult::COMPILE_OK, move(function));
}

void Compiler::expression()
{
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void Compiler::block()
{
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::EOF_))
    {
        declaration();
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::function(FunctionType type)
{
    Compiler compiler{this, type};
    compiler.beginScope();
    compiler.functionParameters();
    compiler.block();
    auto function = compiler.endCompiler();
    emitBytes(Opcode::OP_CLOSURE, makeConstant(objectValue(function)));
    for (int i = 0; i < function->upvalueCount; i++)
    {
        auto &upvalue = compiler.internals.upvalues[i];
        emitBytes(upvalue.isLocal ? 1 : 0, upvalue.index);
    }
}

void Compiler::functionParameters()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            internals.function->arity++;
            if (internals.function->arity > 255)
                errorAtCurrent("Can't have more than 255 parameters.");

            auto constant = parseVariable("Expect parameter name.");
            defineVariable(move(constant));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
}

void Compiler::method()
{
    consume(TokenType::IDENTIFIER, "Expect method name.");
    auto constant = identifierConstant(parser->previous);
    auto type = FunctionType::TYPE_METHOD;

    if (parser->previous.length == 4 && memcmp(parser->previous.start, "init", 4) == 0)
        type = FunctionType::TYPE_INITIALIZER;

    function(type);
    emitBytes(Opcode::OP_METHOD, constant);
}

void Compiler::classDeclaration()
{
    consume(TokenType::IDENTIFIER, "Expect class name.");
    auto className = parser->previous;
    auto nameConstant = identifierConstant(parser->previous);
    declareVariable();

    emitBytes(Opcode::OP_CLASS, nameConstant);
    defineVariable(nameConstant);

    auto classCompiler = ClassCompiler{currentClass};
    currentClass = &classCompiler;

    if (match(TokenType::LESS))
    {
        consume(TokenType::IDENTIFIER, "Expect superclass name.");
        variable(false);

        if (identifiersEqual(parser->previous, className))
            error("A class can't inherit from itself.");

        beginScope();
        addLocal(Token{TokenType::SUPER, "super", 5, -1});
        defineVariable(0);

        namedVariable(className, false);
        emitByte(Opcode::OP_INHERIT);
        currentClass->hasSuperclass = true;
    }

    namedVariable(className, false);
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::EOF_))
    {
        method();
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(Opcode::OP_POP);

    if (currentClass->hasSuperclass)
        endScope();

    currentClass = currentClass->enclosing;
}

void Compiler::funDeclaration()
{
    auto global = parseVariable("Expect function name.");
    markInitialized();
    function(FunctionType::TYPE_FUNCTION);
    defineVariable(global);
}

void Compiler::varDeclaration()
{
    auto global = parseVariable("Expect variable name.");

    if (match(TokenType::EQUAL))
        expression();
    else
        emitByte(Opcode::OP_NIL);

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    defineVariable(global);
}

void Compiler::statement()
{
    if (match(TokenType::PRINT))
        printStatement();
    else if (match(TokenType::FOR))
        forStatement();
    else if (match(TokenType::IF))
        ifStatement();
    else if (match(TokenType::RETURN))
        returnStatement();
    else if (match(TokenType::WHILE))
        whileStatement();
    else if (match(TokenType::LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
        expressionStatement();
}

void Compiler::declaration()
{
    if (match(TokenType::CLASS))
        classDeclaration();
    else if (match(TokenType::FUN))
        funDeclaration();
    else if (match(TokenType::VAR))
        varDeclaration();
    else
        statement();

    if (parser->panicMode)
        synchronize();
}

void Compiler::printStatement()
{
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(Opcode::OP_PRINT);
}

void Compiler::returnStatement()
{
    if (internals.type == FunctionType::TYPE_SCRIPT)
        error("Can't return from top-level code.");

    if (match(TokenType::SEMICOLON))
        emitReturn();
    else
    {
        if (internals.type == FunctionType::TYPE_INITIALIZER)
            error("Can't return a value from an initializer.");

        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after return value.");
        emitByte(Opcode::OP_RETURN);
    }
}

void Compiler::whileStatement()
{
    int loopStart = currentChunk()->size();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
    emitByte(Opcode::OP_POP);
    statement();

    emitLoop(loopStart);
    patchJump(exitJump);
    emitByte(Opcode::OP_POP);
}

void Compiler::forStatement()
{
    beginScope();
    consume(TokenType::LEFT_PAREN, "Expect ')' after 'for'.");
    if (match(TokenType::SEMICOLON))
    {
        // no initializer
    }
    else if (match(TokenType::VAR))
    {
        varDeclaration();
    }
    else
    {
        expressionStatement();
    }
    auto chunk = currentChunk();
    int loopStart = chunk->size();
    int exitJump = -1;

    if (!match(TokenType::SEMICOLON))
    {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

        exitJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
        emitByte(Opcode::OP_POP);
    }

    if (!match(TokenType::RIGHT_PAREN))
    {
        int bodyJump = emitJump(Opcode::OP_JUMP);
        int incrementStart = chunk->size();
        expression();
        emitByte(Opcode::OP_POP);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(Opcode::OP_POP);
    }
    endScope();
}

void Compiler::synchronize()
{
    parser->panicMode = false;
    while (parser->current.type != TokenType::EOF_)
    {
        if (parser->previous.type == TokenType::SEMICOLON)
            return;
        switch (parser->current.type)
        {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default:
            break;
        }

        advance();
    }
}

void Compiler::expressionStatement()
{
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emitByte(Opcode::OP_POP);
}

void Compiler::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
    emitByte(Opcode::OP_POP);
    statement();

    int elseJump = emitJump(Opcode::OP_JUMP);

    patchJump(thenJump);
    emitByte(Opcode::OP_POP);

    if (match(TokenType::ELSE))
        statement();

    patchJump(elseJump);
}

void Compiler::number(bool canAssign)
{
    auto value = std::strtod(parser->previous.start, nullptr);
    emitConstant(numberValue(value));
}

void Compiler::grouping(bool canAssign)
{
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(bool canAssign)
{
    auto operatorType = parser->previous.type;
    parsePrecedence(Precedence::PREC_UNARY);

    switch (operatorType)
    {
    case TokenType::MINUS:
        emitByte(Opcode::OP_NEGATE);
        break;
    case TokenType::BANG:
        emitByte(Opcode::OP_NOT);
        break;
    default:
        break;
    }
}

void Compiler::binary(bool canAssign)
{
    auto operatorType = parser->previous.type;
    auto rule = getRule(operatorType);
    parsePrecedence(rule.precedence + 1);

    switch (operatorType)
    {
    case TokenType::BANG_EQUAL:
        emitBytes(Opcode::OP_EQUAL, Opcode::OP_NOT);
        break;
    case TokenType::EQUAL_EQUAL:
        emitByte(Opcode::OP_EQUAL);
        break;
    case TokenType::GREATER:
        emitByte(Opcode::OP_GREATER);
        break;
    case TokenType::GREATER_EQUAL:
        emitBytes(Opcode::OP_LESS, Opcode::OP_NOT);
        break;
    case TokenType::LESS:
        emitByte(Opcode::OP_LESS);
        break;
    case TokenType::LESS_EQUAL:
        emitBytes(Opcode::OP_GREATER, Opcode::OP_NOT);
        break;
    case TokenType::PLUS:
        emitByte(Opcode::OP_ADD);
        break;
    case TokenType::MINUS:
        emitByte(Opcode::OP_SUBTRACT);
        break;
    case TokenType::STAR:
        emitByte(Opcode::OP_MULTIPLY);
        break;
    case TokenType::SLASH:
        emitByte(Opcode::OP_DIVIDE);
        break;
    default:
        break;
    }
}

void Compiler::call(bool canAssign)
{
    auto argCount = argumentList();
    emitBytes(Opcode::OP_CALL, argCount);
}

void Compiler::dot(bool canAssign)
{
    consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
    auto name = identifierConstant(parser->previous);

    if (canAssign && match(TokenType::EQUAL))
    {
        expression();
        emitBytes(Opcode::OP_SET_PROPERTY, name);
    }
    else if (match(TokenType::LEFT_PAREN))
    {
        auto argCount = argumentList();
        emitBytes(Opcode::OP_INVOKE, name);
        emitByte(argCount);
    }
    else
        emitBytes(Opcode::OP_GET_PROPERTY, name);
}

void Compiler::literal(bool canAssign)
{
    switch (parser->previous.type)
    {
    case TokenType::TRUE:
        emitByte(Opcode::OP_TRUE);
        break;
    case TokenType::FALSE:
        emitByte(Opcode::OP_FALSE);
        break;
    case TokenType::NIL:
        emitByte(Opcode::OP_NIL);
        break;
    default:
        break;
    }
}

void Compiler::string(bool canAssign)
{
    auto stringObject = copyString(parser->previous.start + 1, parser->previous.length - 2);
    emitConstant(objectValue(move(stringObject)));
}

void Compiler::variable(bool canAssign)
{
    namedVariable(parser->previous, canAssign);
}

void Compiler::super_(bool canAssign)
{
    if (!currentClass)
        error("Can't use 'super' outside of a class.");
    else if (!currentClass->hasSuperclass)
        error("Can't use 'super' in a class with no superclass.");

    consume(TokenType::DOT, "Expect '.' after 'super'.");
    consume(TokenType::IDENTIFIER, "Expect superclass method name.");
    auto name = identifierConstant(parser->previous);

    namedVariable(Token{TokenType::THIS, "this", 4, -1}, false);
    if (match(TokenType::LEFT_PAREN))
    {
        auto argCount = argumentList();
        namedVariable(Token{TokenType::SUPER, "super", 5, -1}, false);
        emitBytes(Opcode::OP_INVOKE_SUPER, name);
        emitByte(argCount);
    }
    else
    {
        namedVariable(Token{TokenType::SUPER, "super", 5, -1}, false);
        emitBytes(Opcode::OP_GET_SUPER, name);
    }
}

void Compiler::this_(bool canAssign)
{
    if (!currentClass)
    {
        error("Can't use 'this' outside of a class.");
        return;
    }
    variable(false);
}

void Compiler::namedVariable(const Token &name, bool canAssign)
{
    Opcode getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1)
    {
        getOp = Opcode::OP_GET_LOCAL;
        setOp = Opcode::OP_SET_LOCAL;
    }
    else if ((arg = resolveUpvalue(name)) != -1)
    {
        getOp = Opcode::OP_GET_UPVALUE;
        setOp = Opcode::OP_SET_UPVALUE;
    }
    else
    {
        getOp = Opcode::OP_GET_GLOBAL;
        setOp = Opcode::OP_SET_GLOBAL;
        arg = identifierConstant(name);
    }

    if (canAssign && match(TokenType::EQUAL))
    {
        expression();
        emitBytes(setOp, arg);
    }
    else
        emitBytes(getOp, arg);
}

void Compiler::advance()
{
    parser->previous = parser->current;

    for (;;)
    {
        parser->current = scanner->scanToken();
        if (parser->current.type != TokenType::ERROR)
            break;

        errorAtCurrent(parser->current.start);
    }
}

void Compiler::errorAtCurrent(const char *message) const
{
    errorAt(parser->current, message);
}

void Compiler::error(const char *message) const
{
    errorAt(parser->previous, message);
}

void Compiler::errorAt(const Token &token, const char *message) const
{
    if (parser->panicMode)
        return;

    parser->panicMode = true;
    std::cerr << "[line " << token.line << "] Error";

    if (token.type == TokenType::EOF_)
    {
        std::cerr << " at end";
    }
    else if (token.type == TokenType::ERROR)
    {
        // pass
    }
    else
    {
        fprintf(stderr, " at '%.*s'", static_cast<int>(token.length), token.start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

void Compiler::consume(TokenType expected, const char *message)
{
    if (parser->current.type == expected)
        advance();
    else
        errorAtCurrent(message);
}

bool Compiler::check(TokenType expected)
{
    return parser->current.type == expected;
}

bool Compiler::match(TokenType expected)
{
    if (parser->current.type != expected)
        return false;

    advance();
    return true;
}

shared_ptr<FunctionObject> Compiler::endCompiler()
{
    emitReturn();
    auto function = internals.function;

#ifdef DEBUG_PRINT_CODE
    if (!parser->hadError)
    {
        Disassembler disasm{currentChunk()};
        disasm.disassembleChunk(function->name ? function->name->str : "<script>");
    }
#endif

    emitBytes(Opcode::OP_CLOSURE, makeConstant(objectValue(function)));
    return function;
}

void Compiler::beginScope()
{
    internals.scopeDepth++;
}

void Compiler::endScope()
{
    internals.scopeDepth--;
    while (internals.scopeDepth > 0 &&
           internals.locals[internals.localCount - 1].depth > internals.scopeDepth)
    {
        if (internals.locals[internals.localCount - 1].isCaptured)
            emitByte(Opcode::OP_CLOSE_UPVALUE);
        else
            emitByte(Opcode::OP_POP);
        internals.localCount--;
    }
}

void Compiler::emitByte(uint8_t byte)
{
    currentChunk()->write(byte, parser->current.line);
}

void Compiler::emitByte(Opcode opcode)
{
    emitByte(static_cast<uint8_t>(opcode));
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitBytes(Opcode opcode, uint8_t byte)
{
    emitByte(opcode);
    emitByte(byte);
}

void Compiler::emitBytes(Opcode opcode1, Opcode opcode2)
{
    emitByte(opcode1);
    emitByte(opcode2);
}

void Compiler::emitLoop(int loopStart)
{
    emitByte(Opcode::OP_LOOP);

    int offset = currentChunk()->size() - loopStart + 2;
    if (offset > UINT16_MAX)
        error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

int Compiler::emitJump(Opcode opcode)
{
    emitByte(opcode);
    emitBytes(0xff, 0xff);
    return currentChunk()->size() - 2;
}

void Compiler::emitReturn()
{
    if (internals.type == FunctionType::TYPE_INITIALIZER)
        emitBytes(Opcode::OP_GET_LOCAL, 0);
    else
        emitByte(Opcode::OP_NIL);

    emitByte(Opcode::OP_RETURN);
}

void Compiler::emitConstant(Value value)
{
    auto index = makeConstant(value);
    emitBytes(Opcode::OP_CONSTANT, index);
}

void Compiler::patchJump(int offset)
{
    auto chunk = currentChunk();
    int jump = chunk->size() - offset - 2;

    if (jump > UINT16_MAX)
        error("Too much code to jump over.");

    (*chunk)[offset] = (jump >> 8) & 0xff;
    (*chunk)[offset + 1] = jump & 0xff;
}

uint8_t Compiler::makeConstant(Value value)
{
    auto index = currentChunk()->addConstant(value);
    if (index > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<uint8_t>(index);
}

void Compiler::parsePrecedence(Precedence precedence)
{
    advance();
    auto prefixRule = getRule(parser->previous.type).prefix;
    if (!prefixRule.has_value())
    {
        error("Expect expression");
        return;
    }

    auto canAssign = precedence <= Precedence::PREC_ASSIGNMENT;
    auto prefixFunc = prefixRule.value();
    prefixFunc(canAssign);

    while (precedence <= getRule(parser->current.type).precedence)
    {
        advance();
        auto infixRule = getRule(parser->previous.type).infix;
        auto infixFunc = infixRule.value();
        infixFunc(canAssign);
    }

    if (canAssign && match(TokenType::EQUAL))
        error("Invalid assignment target.");
}

uint8_t Compiler::identifierConstant(const Token &token)
{
    auto s = copyString(token.start, token.length);
    return makeConstant(objectValue(move(s)));
}

bool Compiler::identifiersEqual(const Token &t1, const Token &t2) const
{
    if (t1.length != t2.length)
        return false;
    return memcmp(t1.start, t2.start, t1.length) == 0;
}

int Compiler::resolveLocal(const Token &name) const
{
    for (int i = internals.localCount - 1; i >= 0; i--)
    {
        auto &local = internals.locals[i];
        if (identifiersEqual(local.name, name))
        {
            if (local.depth == -1)
                error("Can't read variable in its own initializer.");

            return i;
        }
    }

    return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal)
{
    int upvalueCount = internals.function->upvalueCount;
    for (int i = 0; i < upvalueCount; i++)
    {
        auto &upvalue = internals.upvalues[i];
        if (upvalue.index == index && upvalue.isLocal == isLocal)
            return i;
    }

    if (upvalueCount == UINT8_COUNT)
    {
        error("Too many closure variables in function.");
        return 0;
    }

    auto &upvalue = internals.upvalues[upvalueCount];
    upvalue.index = index;
    upvalue.isLocal = isLocal;
    return internals.function->upvalueCount++;
}

int Compiler::resolveUpvalue(const Token &name)
{
    if (!enclosing)
        return -1;

    int local = enclosing->resolveLocal(name);
    if (local != -1)
    {
        enclosing->internals.locals[local].isCaptured = true;
        return addUpvalue(static_cast<uint8_t>(local), true);
    }

    int upvalue = enclosing->resolveUpvalue(name);
    if (upvalue != -1)
    {
        return addUpvalue(static_cast<uint8_t>(local), false);
    }

    return -1;
}

void Compiler::addLocal(const Token name)
{
    if (internals.localCount == UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }
    auto &local = internals.locals[internals.localCount++];
    local.name = move(name);
    local.depth = -1;
}

void Compiler::declareVariable()
{
    if (internals.scopeDepth == 0)
        return;
    auto name = parser->previous;
    for (int i = internals.localCount - 1; i >= 0; i--)
    {
        auto &local = internals.locals[i];
        if (local.depth != -1 && local.depth < internals.scopeDepth)
            break;

        if (identifiersEqual(name, local.name))
            error("Already a variable with this name in this scope.");
    }
    addLocal(name);
}

uint8_t Compiler::parseVariable(const char *errorMsg)
{
    consume(TokenType::IDENTIFIER, errorMsg);
    declareVariable();
    if (internals.scopeDepth > 0)
        return 0;
    return identifierConstant(parser->previous);
}

void Compiler::markInitialized()
{
    if (internals.scopeDepth == 0)
        return;
    internals.locals[internals.localCount - 1].depth = internals.scopeDepth;
}

void Compiler::defineVariable(uint8_t global)
{
    if (internals.scopeDepth > 0)
    {
        markInitialized();
        return;
    }

    emitBytes(Opcode::OP_DEFINE_GLOBAL, global);
}

uint8_t Compiler::argumentList()
{
    auto argCount = 0;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if (argCount == 255)
                error("Can't have more than 255 arguments.");

            expression();
            argCount++;
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

void Compiler::and_(bool canAssign)
{
    int endJump = emitJump(Opcode::OP_JUMP_IF_FALSE);

    emitByte(Opcode::OP_POP);
    parsePrecedence(Precedence::PREC_AND);

    patchJump(endJump);
}

void Compiler::or_(bool canAssign)
{
    int elseJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
    int endJump = emitJump(Opcode::OP_JUMP);

    patchJump(elseJump);
    emitByte(Opcode::OP_POP);

    parsePrecedence(Precedence::PREC_OR);
    patchJump(endJump);
}

Compiler::ParseRule &Compiler::getRule(TokenType &type)
{
    return rules[to_underlying(type)];
}

shared_ptr<StringObject> Compiler::copyString(const char *ptr, int len)
{
    std::string str(ptr, len);

    if (stringInternProps)
    {
        auto props = stringInternProps.value();
        auto found = props.tryFindInternedString(str);
        if (found)
            return found.value();

        auto stringObject = newString(str);
        props.addStringToIntern(stringObject);
        return stringObject;
    }

    return newString(str);
}

Chunk *Compiler::currentChunk()
{
    return &internals.function->chunk;
}

void Compiler::initInternals(FunctionType type)
{
    internals.type = type;
    internals.localCount = 0;
    internals.scopeDepth = 0;
    internals.function = newFunction();
    if (type != FunctionType::TYPE_SCRIPT)
    {
        internals.function->name = copyString(parser->previous.start, parser->previous.length);
    }
    internals.locals = std::array<Local, UINT8_COUNT>{};
    auto &local = internals.locals[internals.localCount++];
    if (type != FunctionType::TYPE_FUNCTION)
    {
        local.name = Token{TokenType::THIS, "this", 4, -1};
    }
    else
    {
        local.name.start = "";
        local.name.length = 0;
    }
}

void Compiler::initRules()
{
    auto grouping = make_optional(
        [this](bool val)
        { this->grouping(val); });
    auto unary = make_optional(
        [this](bool val)
        { this->unary(val); });
    auto binary = make_optional(
        [this](bool val)
        { this->binary(val); });
    auto call = make_optional(
        [this](bool val)
        { this->call(val); });
    auto dot = make_optional(
        [this](bool val)
        { this->dot(val); });
    auto number = make_optional(
        [this](bool val)
        { this->number(val); });
    auto literal = make_optional(
        [this](bool val)
        { this->literal(val); });
    auto string = make_optional(
        [this](bool val)
        { this->string(val); });
    auto variable = make_optional(
        [this](bool val)
        { this->variable(val); });
    auto super_ = make_optional(
        [this](bool val)
        { this->super_(val); });
    auto this_ = make_optional(
        [this](bool val)
        { this->this_(val); });
    auto and_ = make_optional(
        [this](bool val)
        { this->and_(val); });
    auto or_ = make_optional(
        [this](bool val)
        { this->or_(val); });

    rules[to_underlying(TokenType::LEFT_PAREN)] = {grouping, call, Precedence::PREC_CALL};
    rules[to_underlying(TokenType::RIGHT_PAREN)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::LEFT_BRACE)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::RIGHT_BRACE)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::COMMA)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::DOT)] = {nullopt, dot, Precedence::PREC_CALL};
    rules[to_underlying(TokenType::MINUS)] = {unary, binary, Precedence::PREC_TERM};
    rules[to_underlying(TokenType::PLUS)] = {nullopt, binary, Precedence::PREC_TERM};
    rules[to_underlying(TokenType::SEMICOLON)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::SLASH)] = {nullopt, binary, Precedence::PREC_FACTOR};
    rules[to_underlying(TokenType::STAR)] = {nullopt, binary, Precedence::PREC_FACTOR};
    rules[to_underlying(TokenType::BANG)] = {unary, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::BANG_EQUAL)] = {nullopt, binary, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::EQUAL)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::EQUAL_EQUAL)] = {nullopt, binary, Precedence::PREC_EQUALITY};
    rules[to_underlying(TokenType::GREATER)] = {nullopt, binary, Precedence::PREC_COMPARISON};
    rules[to_underlying(TokenType::GREATER_EQUAL)] = {nullopt, binary, Precedence::PREC_COMPARISON};
    rules[to_underlying(TokenType::LESS)] = {nullopt, binary, Precedence::PREC_COMPARISON};
    rules[to_underlying(TokenType::LESS_EQUAL)] = {nullopt, binary, Precedence::PREC_COMPARISON};
    rules[to_underlying(TokenType::IDENTIFIER)] = {variable, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::STRING)] = {string, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::NUMBER)] = {number, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::AND)] = {nullopt, and_, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::CLASS)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::ELSE)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::FALSE)] = {literal, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::FOR)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::FUN)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::IF)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::NIL)] = {literal, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::OR)] = {nullopt, or_, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::PRINT)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::RETURN)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::SUPER)] = {super_, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::THIS)] = {this_, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::TRUE)] = {literal, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::VAR)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::WHILE)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::ERROR)] = {nullopt, nullopt, Precedence::PREC_NONE};
    rules[to_underlying(TokenType::EOF_)] = {nullopt, nullopt, Precedence::PREC_NONE};
}