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
    void stripEditVal();

    enum Opcode
    {

        OPeditVal, // parameter1 - offset
        OPmovePtr, // parameter1 - offset
        OPjumpOnZero, // parameter1 - index of next instr on branch
        OPjumpOnNonZero, // parameter1 - index of next inst on branch
        OPmulAdd, // parameter1 - relative offset, parameter2 - increment
        OPmulAddZero,
        OPsetZero,
        OPprint,
        OPread,
        OPdebug,
        OPend

    };

    struct Instruction
    {

        Opcode opcode;
        std::int32_t parameter;
        std::int32_t parameter2;

        // Avoid decoding OPmovePtr
        std::int32_t parameter3;
        // Avoid decoding OPeditVal
        std::int32_t parameter4;

        Instruction():parameter(0),parameter2(0),parameter3(0),parameter4(0){}

    };

    void dumpCode(const std::vector<Instruction> &code,const std::string &filename);

    typedef std::uint32_t CellType;

    std::vector<Instruction> mCode;
    std::vector<CellType> mCellArray;
    std::set<int> mLoopsToOptimize;

};

#endif
