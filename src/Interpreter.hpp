#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <vector>
#include <set>
#include <stack>
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
    void findZeroOptimize();

    enum Opcode
    {

        OPeditVal, // parameter1 - offset
        OPmovePtr, // parameter1 - offset
        OPjumpOnZero, // parameter1 - index of next instr on branch
        OPjumpOnNonZero, // parameter1 - index of next inst on branch
        OPmulAdd, // parameter1 - relative offset, parameter2 - increment
        OPmulAddZero,
        OPsetZero,
        OPfindZero,
        OPprint,
        OPread,
        OPdebug,
        OPend

    };

    struct Instruction
    {

        Opcode opcode;
        std::int32_t parameter;
        decltype(parameter) parameter2;

        // Avoid decoding OPmovePtr
        decltype(parameter)  parameter3;
        // Avoid decoding OPeditVal
        decltype(parameter)  parameter4;

        Instruction():parameter(0),parameter2(0),parameter3(0),parameter4(0){}

        Instruction(Opcode op,decltype(parameter) parameter)
                    :opcode(op),parameter(parameter),parameter2(0),parameter3(0),
                    parameter4(0){}

        explicit Instruction(Opcode op):opcode(op),parameter(0),parameter2(0),parameter3(0),parameter4(0){}

    };

    using Code = std::vector <Instruction>;
    using CellType = std::uint32_t;
    using LoopStack = std::stack <decltype(Instruction::parameter)>;
    using CellArray = std::vector <CellType>;

    void dumpCode(const Code &code,const std::string &filename);

    Code mCode;
    CellArray mCellArray;
    std::set <decltype(Instruction::parameter)> mLoopsToOptimize;

};

#endif
