#include "Interpreter.hpp"
#include <fstream>
#include <iostream>
#include <stack>
#include <map>
#include <algorithm>
#include <cassert>



void Interpreter::dumpCode(const std::vector<Instruction> &code,const std::string &filename)
{
    std::ofstream file(filename.c_str());

    int codeSize = code.size();

    for(int i = 0; i<codeSize ;i++)
    {

        switch(code[i].opcode)
        {

        case OPeditVal:

            file<<"editVal, "<<code[i].parameter;

            break;

        case OPmovePtr:

            file<<"movePtr, "<<code[i].parameter;

            break;

        case OPjumpOnZero:

            file<<"jumpOnZero, "<<code[i].parameter;

            break;

        case OPjumpOnNonZero:

            file<<"jumpOnNonZero, "<<code[i].parameter;

            break;

        case OPloopAdd:

            file<<"loopAdd, "<<code[i].parameter<<", "<<code[i].parameter2;

            break;

        case OPsetZero:

            file<<"setZero";

            break;

        case OPprint:

            file<<"print";

            break;

        case OPread:

            file<<"read";

            break;

        case OPdebug:

            file<<"debug";

            break;

        case OPend:

            file<<"end";

            break;


        }

        file<<'\n';
    }

    file<<"------------------------\nCode size: "<<codeSize;



    return;
}


Interpreter::Interpreter()
{
    mCode.reserve(1000);
}

void Interpreter::run(std::ifstream &sourceFile,std::istream &stdInput,std::size_t arraySize,bool debugMode)
{

    init(arraySize);

    if(parseFile(sourceFile,debugMode))
    {
        performOptimizations(debugMode);
        executeCode(stdInput);
    }


    return;
}



bool Interpreter::parseFile(std::ifstream &sourceFile,bool debugMode)
{

    int ch;
    int codePos = 0;
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

        switch( static_cast<char>(ch) )
        {

        case '+':

            if(mCode.empty() || mCode.back().opcode != OPeditVal)
            {
                instr.opcode=OPeditVal;
                instr.parameter=1;
                mCode.push_back(instr);
            }
            else if( ++mCode.back().parameter == 0)
                mCode.pop_back();


            //Loop optimization related code
            loopCounter += relativePointer == 0 ? 1:0;



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
            loopCounter -= relativePointer == 0 ? 1:0;

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

            if(mCode.empty() || mCode.back().opcode !=OPmovePtr)
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
            {
                std::cout<<"Unbalanced brackets\n";
                return false;
            }
            else
            {
                //Loop optimization related code
                if(!recentPop && loopCounter==-1 && relativePointer==0 && !hasPrintRead)
                {
                    mLoopsToOptimize.insert(loopStack.top());
                }

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
    {
        std::cout<<"Unbalanced brackets\n";
        return false;
    }

    instr.opcode = OPend;
    mCode.push_back(instr);


    return true;
}


void Interpreter::executeCode(std::istream &stdInput)
{

    int stdinChar;
    Instruction *code = &mCode.front();
    Instruction *toExecute = code;

    CellType *cellArray = &mCellArray.front();
    unsigned int dataPtr = 0;

    while(true)
    {

        switch(toExecute->opcode)
        {

        case OPeditVal:

            cellArray[dataPtr] += toExecute->parameter;

            break;

        case OPmovePtr:

            dataPtr += toExecute->parameter;

            break;

        case OPjumpOnZero:

            if(!cellArray[dataPtr])
                toExecute = &code[toExecute->parameter];

                break;

        case OPjumpOnNonZero:

            if(cellArray[dataPtr])
                toExecute = &code[toExecute->parameter];

            break;

        //Loop optimization related code

        case OPloopAdd:

            cellArray[dataPtr + toExecute->parameter] += cellArray[dataPtr] * toExecute->parameter2;

            break;

        case OPsetZero:

            cellArray[dataPtr] = 0;

            break;


        case OPprint:

            std::cout<<static_cast<char>(cellArray[dataPtr]);

            break;

        case OPread:

            stdinChar = stdInput.get();

            if(stdinChar != std::ifstream::traits_type::eof())
                cellArray[dataPtr] = stdinChar;


            break;

        case OPdebug:


            std::cout<<"Position within the code: "<<toExecute->parameter<<'\n';
            std::cout<<"Pointer value: "<<dataPtr<<'\n';
            std::cout<<"Value at pointer: "<<cellArray[dataPtr]<<'\n';

            std::cin.get();

            break;

        case OPend:
            goto finish;


        default:
            break;


        }

        ++toExecute;

    }

finish:

    return;
}

void Interpreter::init(std::size_t arraySize)
{

    mCellArray=std::vector<CellType>(arraySize);
    std::fill(mCellArray.begin(),mCellArray.end(),0);

    return;
}


void Interpreter::optimizeLoops()
{

    std::vector<Instruction> optimizedCode;
    std::set<int>::iterator mLoopsToOptimizeEnd = mLoopsToOptimize.end();
    std::map<int,int> loopAddOpcodes;
    std::map<int,int>::iterator loopAddEnd = loopAddOpcodes.end();
    std::map<int,int>::iterator loopAddBegin;
    std::stack<int> loopStack;

    Instruction instr;
    Instruction *toInspect;
    int codeSize = mCode.size();
    int relativePointer = 0;
    bool scanLoop = false;

    optimizedCode.reserve(mCode.size());

    for(int i = 0; i<codeSize; ++i)
    {

        toInspect = &mCode[i];


        if(!scanLoop)
        {
            if(toInspect->opcode == OPjumpOnZero )
            {

                //Is this loop to optimize, if yes start scan
                if(mLoopsToOptimize.find(i) != mLoopsToOptimizeEnd)
                scanLoop=true;

                else
                {
                    optimizedCode.push_back(*toInspect);
                    loopStack.push(optimizedCode.size() - 1);
                }
            }

            //Correct loop jumps
            else if(toInspect->opcode == OPjumpOnNonZero)
            {
                instr.opcode = OPjumpOnNonZero;
                instr.parameter = loopStack.top();
                optimizedCode.push_back(instr);
                optimizedCode[loopStack.top()].parameter = optimizedCode.size() - 1;
                loopStack.pop();


            }
            else
            {
                optimizedCode.push_back(*toInspect);

            }
        }

        //Scan loop to optimize
        else
        {

            switch(toInspect->opcode)
            {

            case OPeditVal:


                //Dont care about counter variable
                if(relativePointer != 0)
                {
                    if(loopAddOpcodes.find(relativePointer) != loopAddEnd)
                        loopAddOpcodes[relativePointer] += toInspect->parameter;
                    else
                        loopAddOpcodes[relativePointer] = toInspect->parameter;
                }

                break;

            case OPmovePtr:

                relativePointer += toInspect->parameter;

                break;

            case OPjumpOnNonZero:


                scanLoop = false;
                relativePointer = 0;


                for(loopAddBegin = loopAddOpcodes.begin(); loopAddBegin != loopAddEnd ; ++loopAddBegin)
                {

                    /*

                    OPloopAdd param, param2
                    param - relative pointer
                    param2 - increment

                    */

                    if(loopAddBegin->second != 0)
                    {
                    instr.opcode = OPloopAdd;
                    instr.parameter = loopAddBegin->first;
                    instr.parameter2 = loopAddBegin->second;
                    optimizedCode.push_back(instr);
                    }


                }

                loopAddOpcodes.clear();
                instr.opcode = OPsetZero;
                optimizedCode.push_back(instr);


                break;


            // Supress warnings
            default:

                break;


            }


        }

    }


    dumpCode(optimizedCode,"optimizedCode.txt");
    dumpCode(mCode,"code.txt");

    mCode = optimizedCode;


    return;
}


void Interpreter::performOptimizations(bool debugMode)
{
    if(debugMode)
        return;

    optimizeLoops();

    return;
}
