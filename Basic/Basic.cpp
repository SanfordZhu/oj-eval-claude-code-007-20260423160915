/*
 * File: Basic.cpp
 * ---------------
 * This file is the main implementation of the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <set>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"
#include "statement.hpp"

/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state, bool &shouldExit);
void executeProgram(Program &program, EvalState &state);
Statement *parseStatement(TokenScanner &scanner, std::string keyword);

/* Set of keywords that cannot be used as variable names */
const std::set<std::string> KEYWORDS = {
    "REM", "LET", "PRINT", "INPUT", "END", "GOTO", "IF", "THEN", "RUN", "LIST", "CLEAR", "QUIT", "HELP"
};

/* Main program */

int main() {
    EvalState state;
    Program program;
    bool shouldExit = false;
    while (!shouldExit) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state, shouldExit);
        } catch (ProgramEndException&) {
            // Program ended normally, continue to next command
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state, shouldExit);
 * -----------------------------------------
 * Processes a single line entered by the user.
 */

void processLine(std::string line, Program &program, EvalState &state, bool &shouldExit) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) return;

    std::string firstToken = scanner.nextToken();

    // Check if it's a line number
    TokenType type = scanner.getTokenType(firstToken);
    if (type == NUMBER) {
        int lineNumber = stringToInteger(firstToken);

        // Check if it's just a line number (delete line)
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }

        // Get the rest of the line and store it
        std::string rest = line.substr(line.find(firstToken) + firstToken.length());
        program.addSourceLine(lineNumber, line);

        // Parse the statement
        std::string nextToken = scanner.nextToken();

        try {
            Statement *stmt = parseStatement(scanner, nextToken);
            program.setParsedStatement(lineNumber, stmt);
        } catch (ErrorException &ex) {
            // Remove the line if parsing failed
            program.removeSourceLine(lineNumber);
            throw;
        }
        return;
    }

    // Direct commands
    std::string keyword = firstToken;

    if (keyword == "REM") {
        // Do nothing for REM
        return;
    } else if (keyword == "LET") {
        std::string var = scanner.nextToken();
        if (KEYWORDS.count(var)) {
            error("SYNTAX ERROR");
        }
        std::string eq = scanner.nextToken();
        if (eq != "=") {
            error("SYNTAX ERROR");
        }
        Expression *exp = parseExp(scanner);
        int value;
        try {
            value = exp->eval(state);
        } catch (...) {
            delete exp;
            throw;
        }
        state.setValue(var, value);
        delete exp;
    } else if (keyword == "PRINT") {
        Expression *exp = parseExp(scanner);
        int value;
        try {
            value = exp->eval(state);
        } catch (...) {
            delete exp;
            throw;
        }
        std::cout << value << std::endl;
        delete exp;
    } else if (keyword == "INPUT") {
        std::string var = scanner.nextToken();
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
        state.setValue(var, value);
    } else if (keyword == "END") {
        throw ProgramEndException();
    } else if (keyword == "GOTO") {
        std::string lineStr = scanner.nextToken();
        int lineNumber = stringToInteger(lineStr);
        if (!program.hasLine(lineNumber)) {
            error("LINE NUMBER ERROR");
        }
        throw GotoException(lineNumber);
    } else if (keyword == "IF") {
        Expression *exp1 = readE(scanner);
        std::string op = scanner.nextToken();
        Expression *exp2 = readE(scanner);
        std::string thenToken = scanner.nextToken();
        if (toUpperCase(thenToken) != "THEN") {
            delete exp1;
            delete exp2;
            error("SYNTAX ERROR");
        }
        std::string lineStr = scanner.nextToken();
        int lineNumber = stringToInteger(lineStr);

        int left = exp1->eval(state);
        int right = exp2->eval(state);
        bool condition = false;
        if (op == "<") condition = left < right;
        else if (op == "<=") condition = left <= right;
        else if (op == ">") condition = left > right;
        else if (op == ">=") condition = left >= right;
        else if (op == "=") condition = left == right;
        else if (op == "<>") condition = left != right;

        delete exp1;
        delete exp2;

        if (condition) {
            if (!program.hasLine(lineNumber)) {
                error("LINE NUMBER ERROR");
            }
            throw GotoException(lineNumber);
        }
    } else if (keyword == "RUN") {
        executeProgram(program, state);
    } else if (keyword == "LIST") {
        int line = program.getFirstLineNumber();
        while (line != -1) {
            std::cout << program.getSourceLine(line) << std::endl;
            line = program.getNextLineNumber(line);
        }
    } else if (keyword == "CLEAR") {
        program.clear();
        state.Clear();
    } else if (keyword == "QUIT") {
        shouldExit = true;
    } else if (keyword == "HELP") {
        // Optional, not tested
    } else {
        error("SYNTAX ERROR");
    }
}

/*
 * Function: executeProgram
 * Usage: executeProgram(program, state);
 * -------------------------------------
 * Executes the stored BASIC program.
 */

void executeProgram(Program &program, EvalState &state) {
    int currentLine = program.getFirstLineNumber();
    while (currentLine != -1) {
        Statement *stmt = program.getParsedStatement(currentLine);
        if (stmt == nullptr) {
            // Parse the statement on the fly
            std::string sourceLine = program.getSourceLine(currentLine);
            TokenScanner scanner;
            scanner.ignoreWhitespace();
            scanner.scanNumbers();
            scanner.setInput(sourceLine);

            scanner.nextToken(); // Skip line number
            std::string keyword = scanner.nextToken();
            stmt = parseStatement(scanner, keyword);
            program.setParsedStatement(currentLine, stmt);
        }

        try {
            stmt->execute(state, program);
            currentLine = program.getNextLineNumber(currentLine);
        } catch (GotoException &ex) {
            currentLine = ex.getLineNumber();
        }
    }
}

/*
 * Function: parseStatement
 * Usage: Statement *stmt = parseStatement(scanner, keyword);
 * ----------------------------------------------------------
 * Parses a statement based on the keyword.
 */

Statement *parseStatement(TokenScanner &scanner, std::string keyword) {
    std::string upperKeyword = toUpperCase(keyword);

    if (upperKeyword == "REM") {
        return new RemStatement();
    } else if (upperKeyword == "LET") {
        std::string var = scanner.nextToken();
        if (KEYWORDS.count(toUpperCase(var))) {
            error("SYNTAX ERROR");
        }
        std::string eq = scanner.nextToken();
        if (eq != "=") {
            error("SYNTAX ERROR");
        }
        Expression *exp = parseExp(scanner);
        return new LetStatement(var, exp);
    } else if (upperKeyword == "PRINT") {
        Expression *exp = parseExp(scanner);
        return new PrintStatement(exp);
    } else if (upperKeyword == "INPUT") {
        std::string var = scanner.nextToken();
        return new InputStatement(var);
    } else if (upperKeyword == "END") {
        return new EndStatement();
    } else if (upperKeyword == "GOTO") {
        std::string lineStr = scanner.nextToken();
        int lineNumber = stringToInteger(lineStr);
        return new GotoStatement(lineNumber);
    } else if (upperKeyword == "IF") {
        Expression *exp1 = readE(scanner);
        std::string op = scanner.nextToken();
        Expression *exp2 = readE(scanner);
        std::string thenToken = scanner.nextToken();
        if (toUpperCase(thenToken) != "THEN") {
            delete exp1;
            delete exp2;
            error("SYNTAX ERROR");
        }
        std::string lineStr = scanner.nextToken();
        int lineNumber;
        try {
            lineNumber = stringToInteger(lineStr);
        } catch (...) {
            delete exp1;
            delete exp2;
            throw;
        }
        return new IfStatement(exp1, op, exp2, lineNumber);
    } else {
        error("SYNTAX ERROR");
    }
    return nullptr;
}
