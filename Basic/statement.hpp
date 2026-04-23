/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type and its subclasses.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "program.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

/*
 * Class: Statement
 * ----------------
 * This class is used to represent a statement in a program.
 */

class Statement {

public:

    Statement();

    virtual ~Statement();

    virtual void execute(EvalState &state, Program &program) = 0;

};

/*
 * Subclass: RemStatement
 * -----------------------
 * REM statement - comment, does nothing
 */

class RemStatement : public Statement {
public:
    RemStatement();
    ~RemStatement() override;
    void execute(EvalState &state, Program &program) override;
};

/*
 * Subclass: LetStatement
 * -----------------------
 * LET statement - variable assignment
 */

class LetStatement : public Statement {
public:
    LetStatement(std::string var, Expression *exp);
    ~LetStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    std::string variable;
    Expression *exp;
};

/*
 * Subclass: PrintStatement
 * -------------------------
 * PRINT statement - print expression value
 */

class PrintStatement : public Statement {
public:
    PrintStatement(Expression *exp);
    ~PrintStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp;
};

/*
 * Subclass: InputStatement
 * -------------------------
 * INPUT statement - read input to variable
 */

class InputStatement : public Statement {
public:
    InputStatement(std::string var);
    ~InputStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    std::string variable;
};

/*
 * Subclass: EndStatement
 * -----------------------
 * END statement - terminate program
 */

class EndStatement : public Statement {
public:
    EndStatement();
    ~EndStatement() override;
    void execute(EvalState &state, Program &program) override;
};

/*
 * Subclass: GotoStatement
 * ------------------------
 * GOTO statement - jump to line number
 */

class GotoStatement : public Statement {
public:
    GotoStatement(int line);
    ~GotoStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    int lineNumber;
};

/*
 * Subclass: IfStatement
 * ----------------------
 * IF-THEN statement - conditional jump
 */

class IfStatement : public Statement {
public:
    IfStatement(Expression *exp1, std::string op, Expression *exp2, int line);
    ~IfStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp1, *exp2;
    std::string op;
    int lineNumber;
};

/*
 * Exception: ProgramEndException
 * -------------------------------
 * Thrown when END statement is executed
 */

class ProgramEndException : public std::exception {
public:
    ProgramEndException();
    const char* what() const noexcept override;
};

/*
 * Exception: GotoException
 * -------------------------
 * Thrown when GOTO or IF-THEN needs to jump to a line
 */

class GotoException : public std::exception {
public:
    explicit GotoException(int line);
    int getLineNumber() const;
private:
    int lineNumber;
};

#endif
