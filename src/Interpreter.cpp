#include "Interpreter.hpp"
#include <fstream>
#include <iostream>
#include <stack>
#include <map>
#include <utility>
#include <stdexcept>
#include <cassert>
#include <iomanip>

#if !defined(NDEBUG)

void Interpreter::dumpCode(const std::vector<Instruction> &code,const std::string &filename)
{

    std::ofstream file(filename.c_str());
    assert(file.is_open());

    file << "Size: " << code.size() << "\n";
    file << "Instructions:\n";


    for(std::size_t i = 0; i < code.size(); i++)
    {

        file << "0x" << std::setw(8) << std::setfill('0') << std::hex << std::uppercase << i << ": ";
        file << std::dec;

        switch(code[i].opcode)
        {

        case OPeditVal:

            file << "editVal " << code[i].parameter;

            break;

        case OPmovePtr:

            file << "movePtr " << code[i].parameter;

            break;

        case OPjumpOnZero:

            file << std::hex;
            file << "jumpOnZero 0x" << code[i].parameter;

            break;

        case OPjumpOnNonZero:

            file << std::hex;
            file << "jumpOnNonZero 0x" << code[i].parameter;

            break;

        case OPmulAdd:

            file << "mulAdd " << code[i].parameter << ", " << code[i].parameter2;

            break;

        case OPmulAddZero:

            file << "mulAddZero " << code[i].parameter << ", " << code[i].parameter2;

            break;

        case OPsetZero:

            file << "setZero";

            break;

        case OPprint:

            file << "print";

            break;

        case OPread:

            file << "read";

            break;

        case OPdebug:

            file << "debug";

            break;

        case OPend:

            file << "end";

            break;

        }

        file << std::dec;
        file << ", " << code[i].parameter3 << "\n";

    }

}

#endif


void Interpreter::run(std::ifstream &sourceFile,std::istream &stdInput,std::size_t arraySize,bool debugMode)
{

    init(arraySize);
    parseFile(sourceFile,debugMode);

    if(!debugMode)
        performOptimizations();

    executeCode(stdInput);

}


/*

Except parsing this function does:
- Optimizes series of "+" and "-"
- Check for loops we can optimize

*/
void Interpreter::parseFile(std::ifstream &sourceFile,bool debugMode)
{

    int ch;
    std::size_t codePos = 0;
    Instruction instr;
    std::stack<int> loopStack;

    //Variables related to loop optimizations
    int relativePointer = 0;
    int loopCounter = 0;
    bool hasPrintRead = false;
    bool recentPop = false;

    while(true)
    {

        ch = sourceFile.get();

        if(ch == std::ifstream::traits_type::eof())
            break;

        switch(static_cast<char>(ch))
        {

        case '+':

            if(mCode.empty() || mCode.back().opcode != OPeditVal)
            {

                instr.opcode = OPeditVal;
                instr.parameter = 1;
                mCode.push_back(instr);

            }
            else if( ++mCode.back().parameter == 0)
                mCode.pop_back();

            //Loop optimization related code
            loopCounter += relativePointer == 0;

            break;

        case '-':

            if(mCode.empty() || mCode.back().opcode != OPeditVal )
            {

                instr.opcode = OPeditVal;
                instr.parameter = -1;
                mCode.push_back(instr);

            }
            else if( --mCode.back().parameter == 0)
                mCode.pop_back();

            //Loop optimization related code
            loopCounter -= relativePointer == 0;

            break;

        case '>':

            if(mCode.empty() || mCode.back().opcode != OPmovePtr)
            {

                instr.opcode = OPmovePtr;
                instr.parameter = 1;
                mCode.push_back(instr);

            }
            else if( ++mCode.back().parameter == 0)
                mCode.pop_back();

            //Loop optimization related code
            ++relativePointer;

            break;

        case '<':

            if(mCode.empty() || mCode.back().opcode != OPmovePtr)
            {

                instr.opcode = OPmovePtr;
                instr.parameter = -1;
                mCode.push_back(instr);

            }
            else if ( --mCode.back().parameter == 0)
                mCode.pop_back();

            //Loop optimization related code
            --relativePointer;

            break;

        case '[':

            instr.opcode = OPjumpOnZero;
            mCode.push_back(instr);
            loopStack.push(mCode.size() - 1);

            //Loop optimzation related code
            relativePointer = 0;
            loopCounter = 0;
            hasPrintRead = recentPop = false;

            break;

        case ']':

            if(loopStack.empty())
                throw std::runtime_error("Unbalanced brackets");

            else
            {
                //Loop optimization related code
                if(!recentPop && loopCounter == -1 && relativePointer == 0 && !hasPrintRead)
                    mLoopsToOptimize.insert(loopStack.top());

                instr.opcode = OPjumpOnNonZero;
                instr.parameter = loopStack.top();
                mCode.push_back(instr);
                mCode[loopStack.top()].parameter = mCode.size() - 1;
                loopStack.pop();
                recentPop = true;

            }


            break;

        case '.':

            instr.opcode = OPprint;
            mCode.push_back(instr);
            hasPrintRead = true;

            break;

        case ',':

            instr.opcode = OPread;
            mCode.push_back(instr);
            hasPrintRead = true;

            break;

        case '#':

            if(debugMode)
            {

                instr.opcode = OPdebug;
                instr.parameter = codePos;
                mCode.push_back(instr);

            }

            break;

        default:
            break;
        }

        ++codePos;

    }

    if(!loopStack.empty())
        throw std::runtime_error("Unbalanced brackets");

    instr.opcode = OPend;
    mCode.push_back(instr);

}


void Interpreter::executeCode(std::istream &stdInput)
{

    int stdinChar;
    Instruction *code = &mCode.front();
    Instruction *toExecute = code;

    CellType *cellArray = &mCellArray.front();
    std::uint32_t dataPtr = 0;

    while(true)
    {

        assert(dataPtr >= 0);
        assert(dataPtr < mCellArray.size());

        dataPtr += toExecute->parameter3;

        switch(toExecute->opcode)
        {

        case OPeditVal:

            cellArray[dataPtr] += toExecute->parameter;

            break;

        case OPmovePtr:

            dataPtr += toExecute->parameter;

            break;

        case OPjumpOnZero:

            assert(toExecute->parameter >= 0);
            assert(static_cast<std::size_t>(toExecute->parameter) < mCellArray.size());

            if(!cellArray[dataPtr])
                toExecute = &code[toExecute->parameter];

            break;

        case OPjumpOnNonZero:

            assert(toExecute->parameter >= 0);
            assert(static_cast<std::size_t>(toExecute->parameter) < mCellArray.size());

            if(cellArray[dataPtr])
                toExecute = &code[toExecute->parameter];

            break;


        case OPmulAdd:

            /*If statement used to prevent out of range indexing when cellArray[dataPtr] == 0
              and dataPtr + toExecute->parameter < 0 */
            if(cellArray[dataPtr])
                cellArray[dataPtr + toExecute->parameter] += cellArray[dataPtr] * toExecute->parameter2;

            break;

        case OPmulAddZero:

            if(cellArray[dataPtr])
                cellArray[dataPtr + toExecute->parameter] += cellArray[dataPtr] * toExecute->parameter2;

            cellArray[dataPtr] = 0;

            break;

        case OPsetZero:

            cellArray[dataPtr] = 0;

            break;

        case OPprint:

            std::cout << static_cast<char>(cellArray[dataPtr]) << std::flush;

            break;

        case OPread:

            stdinChar = stdInput.get();

            if(stdinChar != std::ifstream::traits_type::eof())
                cellArray[dataPtr] = stdinChar;

            break;

        case OPdebug:

            std::cerr << "Position within the code: " << toExecute->parameter << "\n";
            std::cerr << "Pointer value: " << dataPtr << "\n";
            std::cerr << "Value at pointer: " << cellArray[dataPtr] << "\n";
            std::cin.get();

            break;

        case OPend:
            goto finish;


        default:
            break;

        }

        ++toExecute;

    }

finish:;

}

void Interpreter::init(std::size_t arraySize)
{

    mCellArray = std::vector<CellType>(arraySize);

}


void Interpreter::optimizeLoops()
{

    std::vector<Instruction> optimizedCode;
    std::map<int,int> mulAddOpcodes;
    std::stack<int> loopStack;

    Instruction instr;
    int relativePointer = 0;
    bool scanLoop = false;


    for(std::size_t i = 0; i < mCode.size(); ++i)
    {

        if(!scanLoop)
        {

            if(mCode[i].opcode == OPjumpOnZero )
            {

                //Is this loop to optimize, if yes start scan
                if(mLoopsToOptimize.find(i) != mLoopsToOptimize.end())
                    scanLoop = true;

                else
                {

                    optimizedCode.push_back(mCode[i]);
                    loopStack.push(optimizedCode.size() - 1);

                }

            }

            //Correct loop jumps
            else if(mCode[i].opcode == OPjumpOnNonZero)
            {

                instr.opcode = OPjumpOnNonZero;
                instr.parameter = loopStack.top();
                optimizedCode.push_back(instr);
                optimizedCode[loopStack.top()].parameter = optimizedCode.size() - 1;
                loopStack.pop();

            }
            else
                optimizedCode.push_back(mCode[i]);

        }

        //Scan loop to optimize
        else
        {

            switch(mCode[i].opcode)
            {

            case OPeditVal:

                //Dont care about counter variable
                if(relativePointer != 0)
                   mulAddOpcodes[relativePointer] += mCode[i].parameter;

                break;

            case OPmovePtr:

                relativePointer += mCode[i].parameter;

                break;

            case OPjumpOnNonZero:

                scanLoop = false;
                relativePointer = 0;

                /*

                    Map stores elements in asscending ordered
                    Based on that, this code [<->-<<+>>] is optimized to

                    mulAdd -2, 1, 0
                    mulAddZero -1, -1, 0

                    Order does not affect code equivalence

                */

                if(mulAddOpcodes.size())
                    for(auto iter = mulAddOpcodes.cbegin(); iter != mulAddOpcodes.cend(); ++iter)
                    {

                        //Skip any instructions with zero increment
                        if(iter->second != 0)
                        {

                            instr.parameter = iter->first;
                            instr.parameter2 = iter->second;

                            if(iter != --mulAddOpcodes.cend())
                                instr.opcode = OPmulAdd;

                            else
                                instr.opcode = OPmulAddZero;

                            optimizedCode.push_back(instr);

                        }

                    }

                else
                {

                    instr.opcode = OPsetZero;
                    optimizedCode.push_back(instr);

                }

                mulAddOpcodes.clear();

                break;

            // Supress warnings
            default:
                break;


            }

        }

    }

    mCode = std::move(optimizedCode);

}


void Interpreter::performOptimizations()
{

    optimizeLoops();
    stripMovePtr();

}


void Interpreter::stripMovePtr()
{

    int movePtrVal = 0;
    std::vector<Instruction> optimizedCode;
    std::stack<int> loopStack;
    Instruction instr;

    for(const auto &currentInstr : mCode)
    {

        switch(currentInstr.opcode)
        {

            case OPmovePtr:

                movePtrVal = currentInstr.parameter;

                break;

            default:

                instr = currentInstr;
                instr.parameter3 = movePtrVal;
                movePtrVal = 0;

                switch(instr.opcode)
                {

                    case OPjumpOnZero:

                        optimizedCode.push_back(instr);
                        loopStack.push(optimizedCode.size() - 1);

                        break;

                    case OPjumpOnNonZero:

                        instr.parameter = loopStack.top();
                        optimizedCode.push_back(instr);
                        optimizedCode[loopStack.top()].parameter = optimizedCode.size() - 1;
                        loopStack.pop();

                        break;

                    default:

                        optimizedCode.push_back(instr);

                        break;

                }

                break;

        }

    }


#if !defined(NDEBUG)

    dumpCode(mCode,"code.txt");
    dumpCode(optimizedCode,"optimizedCode.txt");

#endif

    mCode = std::move(optimizedCode);

}
