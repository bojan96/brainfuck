#include "Interpreter.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstddef>
#include <string>
#include <memory>
#include <utility>
#include <stdexcept>

class Bf
{

public:

    Bf():mArraySize(10000),mDebug(false),mHelp(false)
    {}

    void run(int argc,char *argv[])
    {

        parseArgs(argc,argv);

        if(mHelp)
            displayHelp();

        else
        {

            Interpreter interpreter;
            interpreter.run(mSourceFile,*mStdin,mArraySize,mDebug);

        }

    }


private:

    bool strToInt(const std::string &str,int &n)
    {

        bool success = true;

        try
        {

            n = std::stoi(str);

        }
        catch(...)
        {
            success = false;
        }

        return success;

    }


    void displayHelp()
    {

        const std::string options[][2]=
        {

            { "-i <input>","Specify input"},
            { "-f <filename>", "Specify file as input"},
            { "-d","Enable debug mode"},
            { "-s","Specify array size"}

        };


        std::cout << "Usage: bf filename [options]\n";
        std::cout << "options: \n";

        for(const auto &option : options)
            std::cout << std::setw(20) << std::left << option[0] << option[1]<<"\n";

        std::cout << "NOTE: If you specify multiple options the last one will be used\n";

    }

    void parseArgs(int argc,char *argv[])
    {

        bool useFileInput = false;
        std::string stdinString;
        std::string stdinFilename;
        int arraySize;
        bool fileSpecified = false;

        if(argc < 2)
            throw std::runtime_error("No input file specified");

        for(int i = 1; i < argc; ++i)
        {

            if(argv[i][0] == '-' && argv[i][2] == '\0')
                switch(argv[i][1])
                {

                case 'i':

                    if(i + 1 < argc)
                    {

                        useFileInput = false;
                        stdinString = argv[++i];

                    }
                    else
                        throw std::runtime_error("Missing input after '-i'");

                    break;

                case 'f':

                    if(i + 1 < argc)
                    {

                        useFileInput = true;
                        stdinFilename = argv[++i];

                    }
                    else
                        throw std::runtime_error("Missing filename after '-f'");

                    break;

                case 'd':

                    mDebug = true;

                    break;

                case 's':

                    if(i + 1 < argc)
                    {

                        if(!strToInt(argv[++i],arraySize) || arraySize <= 0)
                            throw std::runtime_error(std::string("Invalid size ") + argv[i]);

                        else
                            mArraySize = arraySize;

                    }
                    else
                        throw std::runtime_error("Missing size after '-s'");

                    break;

                case 'h':

                    mHelp = true;
                    // User wants to see help text
                    return;

                    break;

                default:

                    throw std::runtime_error(std::string("Invalid option ") + argv[i]);

                    break;

                }

            else
            {

                fileSpecified = true;
                mSourceFile.open(argv[i]);

                if(!mSourceFile.is_open())
                    throw std::runtime_error(std::string("Could not open the file: ") + argv[i]);

            }

        }

        if(!fileSpecified)
            throw std::runtime_error("No input file specified");

        if(useFileInput)
        {

            std::unique_ptr<std::ifstream> stdin(new std::ifstream(stdinFilename));

            if(!stdin->is_open())
                throw std::runtime_error(std::string("Could not open the file: ") + stdinFilename);

            mStdin = std::move(stdin);

        }
        else
            mStdin = std::unique_ptr<std::istream> (new std::istringstream(stdinString));

    }

    std::ifstream mSourceFile;
    std::unique_ptr<std::istream> mStdin;
    std::size_t mArraySize;
    bool mDebug;
    bool mHelp;

};


int main(int argc,char *argv[])
{

    try
    {

        Bf bf;
        bf.run(argc,argv);

    }
    catch(const std::exception &ex)
    {

        std::cerr << "Error: " << ex.what();

    }

    std::cout << "\n";

    return 0;
}
