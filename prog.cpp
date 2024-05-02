
/**
 * Assignment 2: Simple UNIX Shell
 * @file prog.cpp
 * @author Nathaniel Vandenberg
 * @author Salman Burhan
 * @brief This is the main function of a simple UNIX Shell.
 * You may add additional functions in this file for your implementation.
 * @version 0.1
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

#define MAX_LINE 80 // The maximum length command

class UNIXShell
{
private:
    bool should_wait = true; /* flag to determine whether to wait for output */
    string last_command;     /* shell copy of last non-reserved command */

public:
    int parse_command(char command[], char *args[]);
    void exec_command(char *args[], int count);
    void exec_reserved_command(char *args[]);
    void save_last_command(char command[]);
    bool is_reserved_command(char *args[]);
    bool should_run = true; /* flag to determine when to exit program */
    void redirect_file_out(char *file_name);
    void redirect_file_in(char *file_name);
};

/**
 * @brief Parse The Arguments From The Input Command, Space Delimited.
 *
 * @param command
 * @param args
 * @return int
 */
int UNIXShell::parse_command(char command[], char *args[])
{
    int count = 0;//number of arguments in args[]
    
    //turn command string into indivdual words
    char *argument = strtok(command, " ");
    while (argument != NULL)
    {
        args[count++] = argument;
        argument = strtok(NULL, " ");
    }
    //check if parent should wait for child fork
    string arg = args[count-1];
    should_wait = (arg.back() == '&') ? false : true;
    //if no wait remove
    args[(should_wait) ? count : count - 1] = NULL;

    return count;
}

/**
 * @brief Execute The Parsed `Reserved` Command.
 *
 * @param args
 */
void UNIXShell::exec_reserved_command(char *args[])
{
    //cout<<"exec_reserved_command<<\n";
    string reserved_command(args[0]);
    if (reserved_command == "exit")
    {
        //cout<<"^^ exit ^^\n";
        should_run = false;
    }
    else if (reserved_command == "!!")
    {
        //cout<<"^^ !! ^^\n";
        should_run = true;
        if (last_command.length() == 0)
        {
            cout << "No command history." << endl;
            return;
        }

        char command[MAX_LINE];
        char *args[MAX_LINE / 2 + 1];

        // Set Copy As Main Command
        strcpy(command, last_command.c_str());
        cout << command << endl;
        // Parse The Newly Copied Main Command
        int count = parse_command(command, args);
        // Execute Newly Parsed Copy
        exec_command(args, count);
    }
}

/**
 * @brief Execute The Parsed Unix Command.
 *
 * @param args
 */
void UNIXShell::exec_command(char *args[], int count)
{
    //cout<<"exec_command: "<<*args<<endl;
    // Fork A New Process.
    pid_t pid = fork();

    // If The Process ID = 0, This Is A Child Process.
    if (pid == 0)
    {
        if(count >= 3)
        {   
            char *file_name = args[count-1];

            if (*args[count-2] == '>') //save command output to designated file
            {
                //cout<<"^^ > ^^\n"; //test output
                args[count-2] = NULL;
                if(file_name) 
                {
                    redirect_file_out(file_name);
                }
            }

            else if (*args[count-2] == '<') //get command input to designated file
            {
                //cout<<"^^ < ^^\n";
                args[count-2] = NULL;
                if(file_name) 
                {
                    redirect_file_in(file_name);
                }
                else{cout << endl<<"osh> No source file."<<endl;}
            }
        }
        if(execvp(args[0], args) < 0)
        {
            cout << "osh> command not found: " << args[0] << endl;
            
        }   
        exit(1);  
    }
    // If The Process ID > 0, This Is The Parent Process
    // And We Should Use The `should_wait` Flag To Decide
    // Whether Or Not To Wait For The Execution Of The
    // Children Processes To Complete Before Moving Forward.
    else if (pid > 0)
    {
        if (should_wait) //by default parent process will wait, unless "&" is placed at end of command
        {
            //cout<<"^^ wait ^^\n";
            //cout <<endl<<"Waiting for children..."<<endl;
            wait(NULL);
        }
        //cout<<"^^ no wait ^^\n";
        return;      
    }
    // If The Process ID Is Neither < 0, There Was An Issue
    // Forking A New Process.
    else
    {
        cout << args[0] << ": fork error" << endl;
        return;
    }
}

/**
 * @brief Updates The Last Command Executed By The Shell
 *
 * @param command
 */
void UNIXShell::save_last_command(char command[])
{
    //cout<<"save_last_command<<\n";
    last_command = string(command);
}

/**
 * @brief Checks Whether The Parsed Command Is Of Type `Reserved`.
 *
 * @param args
 * @return true
 * @return false
 */
bool UNIXShell::is_reserved_command(char *args[])
{
    //cout<<"is_reserved_command<<\n";
    string command(args[0]);
    return (command == "exit" || command == "!!") ? true : false;
}

/**
 * @brief redirects shell output to specified file location.
 *
 * @param args
 * @param count
 */
void UNIXShell::redirect_file_out(char *file_name)
{
    //open file @file_name and get file descriptor @FD_out
    int FD_out = open(file_name, O_WRONLY | O_CREAT | O_APPEND , 0466);
                    
    dup2(FD_out,STDOUT_FILENO);
    //write(FD_out, "TEST WRITE. TEST TEST", 46); //test code
    close(FD_out);
}

/**
 * @brief redirects shell input from specified file location.
 *
 * @param args
 * @param count
 */
void UNIXShell::redirect_file_in(char *file_name)
{
    //open file @file_name and get file descriptor @FD_in
    int FD_in = open(file_name, O_RDONLY);
                    
    dup2(FD_in,STDIN_FILENO);
    close(FD_in);
}

/**
 * @brief The main function of a simple UNIX Shell.
 * You may add additional functions in this file for your implementation.
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @return The exit status of the program
 */

int main(int argc, char const *argv[])
{
    // Instance Of Our Shell
    UNIXShell shell;

    char command[MAX_LINE];       /* the command that was entered */
    char last_command[MAX_LINE];  /* a copy of the command that was entered */
    char *args[MAX_LINE / 2 + 1]; /* parsed command line arguments */

    while (shell.should_run)
    {
        cout << "osh>";
        cout.flush();

        // Read User Input and Save A Local Copy Of The Full Command
        // In Case It Does Not End Up Being A `Reserved Command.
        cin.getline(command, MAX_LINE);
        strcpy(last_command, command);

        // Parse The Inputted Command
        int count = shell.parse_command(command, args);
        
        if(count == 1)
        {
            // Check If The Command Is A `Reserved` Command.
            if (shell.is_reserved_command(args))
            {
                shell.exec_reserved_command(args);
                continue;
            }
        }
        // The Command Is Not `Reserved`.
        // Save The Local Copy Of The Full Command In The Shell.
        // Proceed To Execute Command Like Normal.
        shell.save_last_command(last_command);
        shell.exec_command(args, count);
    }

    return 0;
}

/**
 * @brief Previously Used To Ensure Handling Of Whitespaces
 *        Before Switching to cin.getline()
 *
 * @return string
 */
string trim(const string &source)
{
    string s(source);
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
    return s;
}
