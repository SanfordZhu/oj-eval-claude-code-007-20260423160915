/*
 * File: program.cpp
 * -----------------
 * This file implements the program.h interface.
 */

#include "program.hpp"

Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    for (auto& entry : parsedStatements) {
        delete entry.second;
    }
    sourceLines.clear();
    parsedStatements.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
        parsedStatements.erase(lineNumber);
    }
    sourceLines[lineNumber] = line;
}

void Program::removeSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        sourceLines.erase(it);
    }
    auto stmtIt = parsedStatements.find(lineNumber);
    if (stmtIt != parsedStatements.end()) {
        delete stmtIt->second;
        parsedStatements.erase(stmtIt);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        return it->second;
    }
    return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    auto it = parsedStatements.find(lineNumber);
    if (it != parsedStatements.end()) {
        delete it->second;
    }
    parsedStatements[lineNumber] = stmt;
}

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = parsedStatements.find(lineNumber);
    if (it != parsedStatements.end()) {
        return it->second;
    }
    return nullptr;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) {
        return -1;
    }
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.upper_bound(lineNumber);
    if (it != sourceLines.end()) {
        return it->first;
    }
    return -1;
}

bool Program::hasLine(int lineNumber) {
    return sourceLines.find(lineNumber) != sourceLines.end();
}
