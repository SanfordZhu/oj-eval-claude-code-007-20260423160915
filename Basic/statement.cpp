/*
 * File: statement.cpp
 * -------------------
 * This file implements the Statement class and its subclasses.
 */

#include "statement.hpp"

/* Implementation of the Statement class */

Statement::Statement() = default;

Statement::~Statement() = default;

/* RemStatement - Comment statement */

RemStatement::RemStatement() = default;

RemStatement::~RemStatement() = default;

void RemStatement::execute(EvalState &state, Program &program) {
    // Do nothing - REM is a comment
}

/* LetStatement - Variable assignment */

LetStatement::LetStatement(std::string var, Expression *exp) : variable(var), exp(exp) {}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(variable, value);
}

/* PrintStatement - Print expression value */

PrintStatement::PrintStatement(Expression *exp) : exp(exp) {}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

/* InputStatement - Read input to variable */

InputStatement::InputStatement(std::string var) : variable(var) {}

InputStatement::~InputStatement() = default;

void InputStatement::execute(EvalState &state, Program &program) {
    std::string input;
    int value;
    while (true) {
        std::cout << " ? ";
        std::getline(std::cin, input);
        try {
            value = stringToInteger(input);
            break;
        } catch (...) {
            std::cout << "INVALID NUMBER" << std::endl;
        }
    }
    state.setValue(variable, value);
}

/* EndStatement - Terminate program */

EndStatement::EndStatement() = default;

EndStatement::~EndStatement() = default;

void EndStatement::execute(EvalState &state, Program &program) {
    throw ProgramEndException();
}

/* GotoStatement - Jump to line number */

GotoStatement::GotoStatement(int line) : lineNumber(line) {}

GotoStatement::~GotoStatement() = default;

void GotoStatement::execute(EvalState &state, Program &program) {
    if (!program.hasLine(lineNumber)) {
        error("LINE NUMBER ERROR");
    }
    throw GotoException(lineNumber);
}

/* IfStatement - Conditional jump */

IfStatement::IfStatement(Expression *exp1, std::string op, Expression *exp2, int line)
    : exp1(exp1), op(op), exp2(exp2), lineNumber(line) {}

IfStatement::~IfStatement() {
    delete exp1;
    delete exp2;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int left = exp1->eval(state);
    int right = exp2->eval(state);
    bool condition = false;
    if (op == "<") condition = left < right;
    else if (op == "<=") condition = left <= right;
    else if (op == ">") condition = left > right;
    else if (op == ">=") condition = left >= right;
    else if (op == "=") condition = left == right;
    else if (op == "<>") condition = left != right;

    if (condition) {
        if (!program.hasLine(lineNumber)) {
            error("LINE NUMBER ERROR");
        }
        throw GotoException(lineNumber);
    }
}

/* Exception classes */

ProgramEndException::ProgramEndException() = default;

const char* ProgramEndException::what() const noexcept {
    return "Program ended";
}

GotoException::GotoException(int line) : lineNumber(line) {}

int GotoException::getLineNumber() const {
    return lineNumber;
}
