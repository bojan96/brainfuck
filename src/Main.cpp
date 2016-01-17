#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstddef>
#include "Interpreter.hpp"

class Bf
{

public:

    Bf():mStdin(0),mArraySize(10000),mDebug(false)
    {}

    ~Bf()
    {
        delete mStdin;
    }

    void run(int argc,char *argv[])
    {
        if(parseArgs(argc,argv))
        {

            Interpreter interpreter;
            interpreter.run(mSourceFile,*mStdin,mArraySize,mDebug);

        }

        return;
    }


private:

    bool strToInt(const char *str,int &n)
    {
        std::istringstream stream(str);
        char ch;

        stream>>n;

        return !(stream.fail() || stream.get(ch));

    }


    void displayHelp(char *argv[])
    {

        const std::string options[][2]=
        {
            { "-i <input>","Specifiy input"},
            { "-f <filename>", "Specifiy file as input"},
            { "-d","Enable debug mode"},
            { "-s","Specify array size"}


        };


        std::cout<<"Usage: bf filename [options]\n";
        std::cout<<"options: \n";

        for(int i=0; i<4; ++i)
        {
            std::cout<<std::setw(20)<<std::left<<options[i][0]<<options[i][1]<<'\n';
        }

        std::cout<<"NOTE: If you specify mutliple inputs or sizes the last one will be used\n";


        return;
    }

    bool parseArgs(int argc,char *argv[])
    {

        bool useFileInput=false;
        std::string stdinString;
        std::string stdinFilename;
        int arraySize;

        if(argc < 2)
        {
            displayHelp(argv);
            return false;
        }

        mSourceFile.open(argv[1]);

        if(! mSourceFile.is_open())
        {
            std::cout<<"Could not open the file: "<<argv[1]<<'\n';
            return false;
        }

        for(int i=2; i<argc; ++i)
        {

            if(argv[i][0]=='-')
            {
                switch(argv[i][1])
                {

                case 'i':

                    if(i+1<argc)
                    {
                        useFileInput=false;
                        stdinString=argv[++i];
                    }
                    else
                    {
                        std::cout<<"Input not specified, last specified input will be used\n";
                    }


                    break;

                case 'f':

                    if(i+1<argc)
                    {
                        useFileInput=true;
                        stdinFilename=argv[++i];
                    }
                    else
                    {
                        std::cout<<"Filename for input not specified, last specified input will be used\n";
                    }

                    break;

                case 'd':

                    mDebug=true;

                    break;

                case 's':

                    if(i+1<argc)
                    {
                        if( !strToInt(argv[++i],arraySize) || arraySize<=0)
                        {
                            std::cout<<"Invalid size value: "<<argv[i]<<", last specified value will be used"<<'\n';

                        }
                        else
                            mArraySize=arraySize;
                    }
                    else
                        std::cout<<"Size not specified, last specified size will be used\n";

                    break;


                }
            }
            else
            {
                std::cout<<"Invalid option: "<<argv[i]<<'\n';
            }


        }


        if(useFileInput)
        {
            std::ifstream *file=new std::ifstream(stdinFilename.c_str());

            if(! file->is_open())
            {
                std::cout<<"Could not open the file: "<<stdinFilename<<'\n';
                return false;
            }

            mStdin=file;


        }
        else
        {
            mStdin=new std::istringstream(stdinString);
        }



        return true;
    }

    std::ifstream mSourceFile;
    std::istream *mStdin;
    std::size_t mArraySize;
    bool mDebug;


};


int main(int argc,char *argv[])
{

    Bf bf;
    bf.run(argc,argv);

    return 0;
}
