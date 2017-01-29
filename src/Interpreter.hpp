#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <vector>
#include <set>
#include <string>
#include <cstddef>
#include <cstdint>

class Interpreter
{

public:

    void run(std::ifstream &sourceFile,std::istream &stdInput,std::size_t arraySize,bool debugMode);

private:

    void parseFile(std::ifstream &sourceFile,bool debugMode);
    void executeCode(std::istream &stdInput);
    void init(std::size_t arraySize);
    void optimizeLoops();
    void performOptimizations();
    void stripMovePtr();

    enum Opcode
    {

        OPeditVal, // parameter1 - offset
        OPmovePtr, // parameter1 - offset
        OPjumpOnZero, // parameter1 - index of next instr on branch
        OPjumpOnNonZero, // parameter1 - index of next inst on branch
        OPmulAdd, // parameter1 - relative offset, parameter2 - increment
        OPsetZero,
        OPprint,
        OPread,
        OPdebug,
        OPend

    };

    struct Instruction
    {

        Opcode opcode;
        int32_t parameter;
        int32_t parameter2;

        // Avoid decoding OPmovePtr
        int32_t parameter3;

        Instruction():parameter3(0){}

    };

    void dumpCode(const std::vector<Instruction> &code,const std::string &filename);

    typedef unsigned int CellType;

    std::vector<Instruction> mCode;
    std::vector<CellType> mCellArray;
    std::set<int> mLoopsToOptimize;

};

#endif
