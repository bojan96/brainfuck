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


    void displayHelp(std::ostream &stream)
    {

        const std::string options[][2]=
        {
            { "-i <input>","Specifiy input"},
            { "-f <filename>", "Specifiy file as input"},
            { "-d","Enable debug mode"},
            { "-s","Specify array size"}


        };


        stream<<"Usage: bf filename [options]\n";
        stream<<"options: \n";

        for(int i=0; i<4; ++i)
        {
            stream<<std::setw(20)<<std::left<<options[i][0]<<options[i][1]<<'\n';
        }

        stream<<"NOTE: If you specify mutliple inputs or sizes the last one will be used\n";


        return;
    }

    bool parseArgs(int argc,char *argv[])
    {

        bool useFileInput=false;
        std::string stdinString;
        std::string stdinFilename;
        int arraySize;
        bool fileSpecified=false;

        if(argc < 2)
        {
            displayHelp(std::cerr);
            return false;
        }


        for(int i=1; i<argc; ++i)
        {


            if(argv[i][0]=='-')

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

                case 'h':

                    displayHelp(std::cout);
                    return false;

                    break;

                default:

                    std::cerr<<"Invalid option: "<<argv[i];
                    return false;

                    break;


                }

            else if(!fileSpecified)
            {
                fileSpecified=true;

                mSourceFile.open(argv[i]);

                if(! mSourceFile.is_open())
                {
                    std::cerr<<"Could not open the file: "<<argv[i];
                    return false;
                }

            }





        }


        if(useFileInput)
        {
            std::ifstream *file=new std::ifstream(stdinFilename.c_str());

            if(! file->is_open())
            {
                std::cerr<<"Could not open the file: "<<stdinFilename<<'\n';
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
