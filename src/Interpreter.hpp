#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP


#include <vector>
#include <set>
#include <string>
#include <cstddef>

class Interpreter
{

public:

    void run(std::ifstream &sourceFile,std::istream &stdInput,std::size_t arraySize,bool debugMode);

private:

    bool parseFile(std::ifstream &sourceFile,bool debugMode);
    void executeCode(std::istream &stdInput);
    void init(std::size_t arraySize);
    void optimizeLoops();
    void performOptimizations();
    void stripMovePtr();


    enum Opcode
    {
        OPeditVal,
        OPmovePtr,
        OPjumpOnZero,
        OPjumpOnNonZero,
        OPloopAdd,
        OPsetZero,
        OPprint,
        OPread,
        OPdebug,
        OPend

    };

    struct Instruction
    {

        Opcode opcode;

        // OPeditVal, OPmovePtr, OPjumpOnZero, OPjumpOnNonZero,OPdebug
        int parameter;

        // OPloopAdd
        int parameter2;

        // Avoid decoding OPmovePtr
        int parameter3;

        Instruction():parameter3(0){}

    };

    void dumpCode(const std::vector<Instruction> &code,const std::string &filename);


    typedef unsigned int CellType;

    std::vector<Instruction> mCode;
    std::vector<CellType> mCellArray;
    std::set<int> mLoopsToOptimize;


};

#endif
