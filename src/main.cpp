#include <cstring>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

namespace javsh
{
    // Beratement
    int nBerateTracker;
    int nBerateFrequency;

    std::vector<std::string> vQuotes;

    std::map<std::string,std::function<int(char**)>> mapBuiltInCommands;
    
    // argument collections
    std::vector<std::string> vArgs;
    char** ppArgs;
    
    // quit function used multiple times
    int QuitFunc(char** args)
    {
        return 0;
    }

    // https://stackoverflow.com/a/14678964/8465844
    void StringReplace(std::string& subject, const std::string& search, const std::string& replace)
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    int LoadQuotes(char** args)
    {
        std::ifstream ifs("quotes.txt");
        vQuotes.clear();
        for(std::string line; std::getline(ifs, line);)
        {
            StringReplace(line, "\\033", "\033");
            StringReplace(line, "\\n", "\n");

            vQuotes.push_back(line);
        }
        ifs.close();
        return 1;
    }

    void Berate()
    {
        nBerateTracker++;

        if((nBerateTracker % nBerateFrequency) == 0)
            std::cout << std::endl << vQuotes[rand() % vQuotes.size()] << std::endl << std::endl;
    }

    void DefineBuiltInCommands()
    {
        mapBuiltInCommands["cd"] = [&](char** args)
        {
            if (args[1] == NULL)
            {
                std::cerr << "javsh: expected argument to \"cd\"\n";
            }
            else
            {
                if(chdir(args[1]) != 0)
                {
                    perror("javsh");
                }
            }
            return 1;
        };
        
        mapBuiltInCommands["exit"] = &QuitFunc;
        mapBuiltInCommands["quit"] = &QuitFunc;

        mapBuiltInCommands["reload"] = &LoadQuotes;
        
        mapBuiltInCommands["quotes"] = [&](char** args)
        {
            std::cout << "\nQuotes\n----------------------\n\n";

            for(auto &quote : vQuotes)
                std::cout << quote << std::endl << std::endl;
            
            std::cout << std::endl;
            return 1;
        };

    }

    // https://stackoverflow.com/a/325000/8465844
    void TokenizeArgs(const std::string& string, const std::string& delimiter)
    {
        size_t  start = 0, end = 0;
        
        vArgs.clear();

        while(end != std::string::npos)
        {
            end = string.find(delimiter, start);

            // If at end, use length=maxLength.  Else use length=end-start.
            vArgs.push_back(
                string.substr(
                    start,
                    (end == std::string::npos) ? std::string::npos : end - start
                )
            );

            // If at end, use start=maxSize.  Else use start=end+delimiter.
            start = (( end > (std::string::npos - delimiter.size())) ? std::string::npos : end + delimiter.size());
        }
    }


    void GetArgs()
    {
        ppArgs = new char* [vArgs.size()+1];
        for(int i = 0; i < vArgs.size(); i++)
        {
            ppArgs[i] = (char*)calloc(1, vArgs[i].length() + 1);
            std::strcpy(ppArgs[i], vArgs[i].c_str());
        }
        
        // sentinel
        ppArgs[vArgs.size()] = NULL;
    }

    void FreeArgs()
    {
        for(size_t i = 0; i < vArgs.size(); i++)
            delete ppArgs[i];

        delete ppArgs;
    }
    
    //https://github.com/brenns10/lsh/blob/master/src/main.c#L101
    int LaunchExternalCommand()
    {
        pid_t pid;
        int status;

        pid = fork();
        if(pid == 0)
        {
            // Child process
            if(execvp(ppArgs[0], ppArgs) == -1)
            {
                perror("javsh");
            }
            exit(EXIT_FAILURE);
        }
        else if(pid < 0)
        {
            // Error forking
            perror("javsh");
        }
        else
        {
            // Parent process
            do
            {
                waitpid(pid, &status, WUNTRACED);
            }
            while(!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        return 1;
    }

    int Execute()
    {
        // try running built-in commands first
        if(mapBuiltInCommands.count(vArgs[0]) > 0)
            return mapBuiltInCommands[vArgs[0]](ppArgs);
        
        // launch external command
        return LaunchExternalCommand();
    }

    void Init()
    {
        // seed randomizer
        srand(time(NULL));
        
        int nStatus;

        nBerateTracker = 0;
        nBerateFrequency = 3;
       
        ppArgs = NULL;

        LoadQuotes(ppArgs);
        DefineBuiltInCommands();

        // Clear the Terminal On Start-Up
        TokenizeArgs("clear", " ");
        
        GetArgs();
        nStatus = Execute();
        FreeArgs();

        std::cout << "\033[0;35mWelcome to JavSH.\033[0m\n\nYOU are using \033[1;31mLinux!\033[0m\n\nWhy? Everybody knows \033[0;32mThe Great Machines\033[0m are \033[0;34mWindows\033[0m\n\n";
        // LOOP FOREVER
        do
        {
            std::cout << "\033[0;34m" << std::filesystem::current_path().c_str();
            std::cout << "\033[0m$ ";
            std::string sLine;
            std::getline(std::cin, sLine);
            
            Berate();

            TokenizeArgs(sLine, " ");

            GetArgs();
            nStatus = Execute();
            FreeArgs();
        }
        while(nStatus);
    }
}

int main(int argc, char* argv[])
{
    javsh::Init();

    return 0;
}