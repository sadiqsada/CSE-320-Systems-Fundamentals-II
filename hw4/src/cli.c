/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <debug.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"
#include "header.h"

int numPrinters = 0;
int numJobs = 0;
int globalPid = 0;

sig_atomic_t volatile job_finished = 0;
sig_atomic_t volatile job_aborted = 0;
sig_atomic_t volatile job_paused = 0;

// counts the number of arguments in given input
int count_args(char *input, char *delim)
{
    int count = 0;
    char *ptr = input;
    char *prevPtr;
    while ((ptr = strchr(ptr, ' ')) != NULL)
    {
        count++;
        prevPtr = ptr;
        while (*ptr == ' ')
        {
            ptr++;
        }
    }

    if (count == 0)
        return count + 1;

    int increment = 0;

    while (*prevPtr != '\0')
    {
        if (*prevPtr != ' ')
        {
            increment = 1;
            break;
        }
        prevPtr++;
    }

    return count + increment;
}

void print_arg_error(int expectedArgs, int argCount, FILE *out)
{
    fprintf(out, "Wrong number of args for command: help; Expected Args: %d, Provided Args: %d\n", expectedArgs, argCount - 1);
    sf_cmd_error("arg count");
}

void free_printer_list()
{
    for (int i = 0; i < numPrinters; i++)
    {
        PRINTER *printer = &(printer_array[i]);
        free(printer->printerName);
    }
}

void free_job_list()
{
    for (int i = 0; i < numJobs; i++)
    {
        JOB *job = &(job_array[i]);
        free(job->jobFileName);
    }
}

int find_printer(char *name)
{
    for (int i = 0; i < MAX_PRINTERS; i++)
    {
        char *printerName = printer_array[i].printerName;
        if (strcmp(printerName, name) == 0)
        {
            // printer found - return the index
            return i;
        }
    }
    return -1; // printer not found
}

char *get_printer_status(int status)
{
    char *statusString;
    if (status == 0)
    {
        statusString = "disabled";
    }
    if (status == 1)
    {
        statusString = "idle";
    }
    if (status == 2)
    {
        statusString = "busy";
    }

    return statusString;
}

// iterates through all jobs and determines if any is
// ready to be printed
// job is ready to be printed if
// 1) A queued job exists
// 2) A conversion path from the job type to the printer type exists
// 3) Printer is in IDLE
int is_job_ready()
{
    for (int i = 0; i < numJobs; i++)
    {
        JOB currentJob = job_array[i];
        char *jobFileType = currentJob.jobFileType->name;
        for (int j = 0; j < numPrinters; j++)
        {
            PRINTER printer = printer_array[j];
            PRINTER_STATUS printerStatus = printer.printerStatus;
            char *printerFileType = printer.printerFileType->name;
            // call find_conversion_path
            CONVERSION **path = find_conversion_path(jobFileType, printerFileType);

            // no conversion
            if (path == NULL && printerStatus == PRINTER_IDLE)
            {
                // return correct job
                return i;
            }

            // some conversion happens
            else {
                return i;
            }
        }
    }
    return -1;
}

int find_printer_from_pid(pid_t pid) {
    for(int i = 0; i < numPrinters; i++) {
        if(printer_array[i].printerPid == pid) {
            return i;
        }
    }
    return -1;
}

int find_job_from_pid(pid_t pid) {
    for(int i = 0; i < numJobs; i++) {
        if(job_array[i].jobPid == pid) {
            return i;
        }
    }
    return -1;
}

void readline_callback() {
    pid_t pid = globalPid;
    int jobIndex = find_job_from_pid(pid);
    int printerIndex = find_printer_from_pid(pid);

    if(job_finished) {
        sf_job_status(job_array[jobIndex].jobId, JOB_FINISHED);
        sf_printer_status(printer_array[printerIndex].printerName, PRINTER_IDLE);
        job_finished = 0;
    }

    if(job_aborted) {
        sf_job_status(job_array[jobIndex].jobId, JOB_ABORTED);
        sf_printer_status(printer_array[printerIndex].printerName, PRINTER_IDLE);
        job_aborted = 0;
    }
}

void handler(int sig) {
    int handlerChildStatus = 0;
    waitpid(-1, &handlerChildStatus, 0);

    if(WIFEXITED(handlerChildStatus)) {
        job_aborted = 1;
    }

    printf("%s %d\n", "Child terminated", sig);
}

void create_conversion_pipeline(PRINTER *printer, JOB *job) {
    // connect to the specified printer
    int connect = imp_connect_to_printer(printer->printerName, printer->printerFileType->name, 0);
    if(connect == -1) {
        printf("%s\n", "Error connecting to printer");
    }
    else {
        printf("%s %d\n", "Connected to the printer, descriptor: ", connect);
    }
    int childStatus = 0;

    // set printer and job status
    sf_job_status(job->jobId, JOB_RUNNING);
    sf_printer_status(printer->printerName, PRINTER_BUSY);

    pid_t pid = fork(); // create master process

    if(pid == -1) {
        debug("Error");
        sf_job_status(job->jobId, JOB_ABORTED);
        sf_printer_status(printer->printerName, PRINTER_IDLE);
    }
    else if(pid == 0) { // child process (Master)
        debug("Master Process");
        setpgid(pid, pid); // set pgid of master process
        job->jobPid = pid;
        printer->printerPid = pid;
        globalPid = pid;

        // find conversion**, and get number of elements
        CONVERSION **argv = find_conversion_path(job->jobFileType->name, printer->printerFileType->name);
        int argSize = 0;
        CONVERSION **copyArgv = argv;
        while(*copyArgv != NULL) {
            argSize++;
            copyArgv++;
        }

        char **copyArgv2[argSize + 1];
        copyArgv = argv;
        while(*copyArgv != NULL) {
            char **currArgs = (*copyArgv) -> cmd_and_args;
            int index = 0;
            copyArgv2[index++] = currArgs;

            copyArgv++;
        }

        copyArgv2[argSize] = NULL;

        char *convPrograms[argSize + 1];
        for(int i = 0; i < argSize; i++) {
            convPrograms[i] = *(copyArgv2[i]);
        }
        convPrograms[argSize] = NULL;
        // set job to started
        sf_job_started(job->jobId, printer->printerName, getpgid(pid), convPrograms);


        if(*argv == NULL) {
            int fd = open(job->jobFileName, O_RDONLY);
            if(fd < 0) {
                printf("%s\n", "Error opening the file");
            }
            char *a[] = {"/bin/cat", NULL};
            int waitStatus = 0;
            pid_t childPid = fork(); // only child of master
            if(childPid == -1) {
                printf("%s\n", "Error");
                close(fd);
            }
            else if(childPid == 0) {
                // child process
                dup2(fd, STDIN_FILENO);
                dup2(connect, STDOUT_FILENO);
                fflush(stdout);
                close(fd);
                close(connect);
                execvp(a[0], a);
                perror("execvp failed");
                printf("%d\n", fd);
                exit(1);
            }
            else {
                // master process
                close(fd);
                close(connect);
                waitpid(-1, &waitStatus, 0);
                exit(0);
            }

        }

        else {
            // create the argv to pass into execvp
            copyArgv = argv;
            int fd = open(job->jobFileName, O_RDONLY);
            int pipefd[2];
            int prevPipe = fd;
            int counter = 0;

            while(counter < argSize - 1) {
                // set up pipe
                pipe(pipefd);

                pid_t childPid = fork();

                if(childPid == -1) {
                    printf("%s\n", "Error");
                    exit(1);
                }
                else if(childPid == 0) {
                    // child process

                    if(prevPipe != STDIN_FILENO) {
                        dup2(prevPipe, STDIN_FILENO);
                        close(prevPipe);
                    }

                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);

                    execvp(copyArgv2[counter][0], copyArgv2[counter]);

                    perror("execvp failed");
                    exit(1);
                }

                close(prevPipe);

                close(pipefd[1]);

                prevPipe = pipefd[0];

                counter++;
            }

            if(prevPipe != STDIN_FILENO) {
                dup2(prevPipe, STDIN_FILENO);
                close(prevPipe);
            }

            dup2(connect, STDOUT_FILENO);
            close(connect);

            // last command
            execvp(copyArgv2[counter][0], copyArgv2[counter]);

            perror("execvp failed");
            exit(1);

        }
    }
    else {
        waitpid(-1, &childStatus, 0);
        printf("%s\n", "Main process");
        exit(0);
    }
}

int handle_input(char *input, char *delim, FILE *out, int quit)
{
    char *helpMessage = "Commands are: help quit type printer conversion printers jobs print cancel disable enable pause resume";
    if (*input != '\0')
    {
        int argCount = count_args(input, delim);
        char *token = strtok(input, delim);
        while (token != NULL)
        {
            // Miscellaneous commands

            // help command
            if (strcmp(token, "help") == 0)
            {
                if (argCount != 1)
                {
                    print_arg_error(0, argCount, out);
                    break;
                }
                fprintf(out, "%s\n", helpMessage);
                sf_cmd_ok();
            }
            // quit command
            if (strcmp(token, "quit") == 0)
            {
                if (argCount != 1)
                {
                    print_arg_error(0, argCount, out);
                    break;
                }
                quit = 1;
                sf_cmd_ok();
                break;
            }

            // Configuration commands

            // type
            if (strcmp(token, "type") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                token = strtok(NULL, delim);
                define_type(token);
                sf_cmd_ok();
                break;
            }

            // printer
            if (strcmp(token, "printer") == 0)
            {
                if (argCount != 3)
                {
                    print_arg_error(2, argCount, out);
                    break;
                }
                PRINTER *newPrinter = &printer_array[numPrinters];
                token = strtok(NULL, delim); // printer_name
                char *printerName = token;

                token = strtok(NULL, delim); // printer file_type
                FILE_TYPE *fileType = find_type(token);

                if (fileType == NULL)
                {
                    fprintf(out, "Unknown file type %s\n", token);
                    sf_cmd_error("printer");
                    break;
                }

                newPrinter->printerId = numPrinters;
                newPrinter->printerName = malloc(strlen(printerName) + 1);
                strcpy(newPrinter->printerName, printerName);
                newPrinter->printerStatus = PRINTER_DISABLED;
                newPrinter->printerFileType = fileType;

                numPrinters++;
                sf_printer_defined(newPrinter->printerName, token);
                sf_printer_status(newPrinter->printerName, newPrinter->printerStatus);

                char *printerStatus = get_printer_status(newPrinter->printerStatus);
                fprintf(out, "PRINTER: id=%d, name=%s, type=%s, status=%s\n", newPrinter->printerId, newPrinter->printerName, (char *)(newPrinter->printerFileType->name), printerStatus);
                sf_cmd_ok();
                break;
            }

            // conversion
            if (strcmp(token, "conversion") == 0)
            {
                if (argCount < 4)
                {
                    print_arg_error(3, argCount, out);
                    break;
                }
                token = strtok(NULL, delim); // first file type

                if(find_type(token) == NULL) {
                    exit(1);
                }

                char *type1 = find_type(token)->name;

                if (type1 == NULL)
                {
                    sf_cmd_error("conversion");
                }

                token = strtok(NULL, delim); // second file type

                if(find_type(token) == NULL) {
                    exit(1);
                }

                char *type2 = find_type(token)->name;

                if (type2 == NULL)
                {
                    sf_cmd_error("conversion");
                }

                int count = argCount - 3;
                token = strtok(NULL, delim); // cmd args start

                char *cmdArgs[count];
                cmdArgs[0] = token;
                count = 1;
                while (token != NULL)
                {
                    token = strtok(NULL, delim);
                    cmdArgs[count] = token;
                    count++;
                }

                CONVERSION *conv = define_conversion(type1, type2, cmdArgs);

                if (conv == NULL)
                {
                    sf_cmd_error("conversion");
                }

                sf_cmd_ok();
                break;
            }

            // informational commands
            // printers
            if (strcmp(token, "printers") == 0)
            {
                if (argCount != 1)
                {
                    print_arg_error(0, argCount, out);
                    break;
                }
                for (int i = 0; i < numPrinters; i++)
                {
                    PRINTER printer = printer_array[i];
                    int printerStatus = (int)(printer.printerStatus);
                    char *printerStatusString = get_printer_status(printerStatus);

                    fprintf(out, "PRINTER: id=%d, name=%s, type=%s, status=%s\n", printer.printerId, printer.printerName, printerStatusString, printerStatusString);
                }
                sf_cmd_ok();
                break;
            }
            // jobs
            if (strcmp(token, "jobs") == 0)
            {
                if (argCount != 1)
                {
                    print_arg_error(0, argCount, out);
                    break;
                }
                for (int i = 0; i < numJobs; i++)
                {
                    JOB *newJob = &job_array[i];
                    fprintf(out, "JOB[%d]: file=%s, type=%s, status=%d, eligible=%08x\n", newJob->jobId, newJob->jobFileName, newJob->jobFileType->name, newJob->jobStatus, newJob->eligiblePrinters);
                }
                sf_cmd_ok();
                break;
            }

            // spooling commands
            // print
            if (strcmp(token, "print") == 0)
            {
                if (argCount != 2 && argCount != 3)
                {
                    print_arg_error(2, argCount, out);
                    break;
                }
                token = strtok(NULL, delim); // filename
                FILE_TYPE *fileType = infer_file_type(token);
                if (fileType == NULL)
                {
                    fprintf(out, "Invalid file type");
                    sf_cmd_error("print");
                    break;
                }
                JOB *newJob = &job_array[numJobs];
                char *jobFileName = token;
                newJob->jobId = numJobs++;
                newJob->jobFileName = malloc(strlen(jobFileName) + 1);
                strcpy(newJob->jobFileName, jobFileName);
                newJob->jobStatus = 0;
                newJob->jobFileType = fileType;

                token = strtok(NULL, delim); // eligible printers

                // if token is NULL all printers are eligible
                if (token == NULL)
                {
                    newJob->eligiblePrinters = 0xffffffff;
                }

                // there is a series of printer names that are eligible
                else
                {
                    int printerCount = 0;
                    newJob->eligiblePrinters = 0;
                    while (token != NULL)
                    {
                        int printerIndex = find_printer(token);
                        if (printerIndex != -1)
                        {
                            printerCount++;
                            // set eligiblePrinters bit
                            newJob->eligiblePrinters = newJob->eligiblePrinters | (1 << (31 - printerIndex));
                        }
                        token = strtok(NULL, delim);
                    }
                    if (printerCount == 0)
                    {
                        sf_cmd_error("Printers not specified properly.");
                    }
                }

                sf_job_created(numJobs, token, fileType->name);
                fprintf(out, "JOB[%d]: file=%s, type=%s, status=%d, eligible=%08x\n", newJob->jobId, newJob->jobFileName, newJob->jobFileType->name, newJob->jobStatus, newJob->eligiblePrinters);
                sf_cmd_ok();
                break;
            }

            // cancel
            if (strcmp(token, "cancel") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                sf_cmd_ok();
                break;
            }

            // pause
            if (strcmp(token, "pause") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                sf_cmd_ok();
                break;
            }

            // resume
            if (strcmp(token, "resume") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                sf_cmd_ok();
                break;
            }

            // disbale
            if (strcmp(token, "disable") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                sf_cmd_ok();
                break;
            }

            // enable
            if (strcmp(token, "enable") == 0)
            {
                if (argCount != 2)
                {
                    print_arg_error(1, argCount, out);
                    break;
                }
                token = strtok(NULL, delim); // printer name
                int index = find_printer(token);
                if (index == -1)
                {
                    sf_cmd_error("enable");
                }
                else
                {
                    PRINTER *printer = &printer_array[index];
                    printer->printerStatus = PRINTER_IDLE;
                    sf_printer_status(printer->printerName, PRINTER_IDLE);
                    int jobReadyStatus = is_job_ready();
                    // no conversion
                    if (jobReadyStatus >= 0)
                    {
                        create_conversion_pipeline(printer, &job_array[jobReadyStatus]);
                    }
                }
                sf_cmd_ok();
                break;
            }

            token = strtok(NULL, delim);
        }
    }
    return quit;
}

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if (in == NULL)
        return -1;

    int quit = 0;

    signal(SIGCHLD, handler);
    sf_set_readline_signal_hook(readline_callback);

    // -i was specified - batch input
    char *lineptr = NULL;
    size_t n = 0;
    if (in != stdin)
    {
        size_t read = getline(&lineptr, &n, in);
        char *delim = " ";
        while (read != -1)
        {
            if (read - 1 >= 0 && lineptr[read - 1] == '\n')
            {
                lineptr[read - 1] = '\0';
            }
            quit = handle_input(lineptr, delim, out, quit);
            read = getline(&lineptr, &n, in);
        }
        free(lineptr);
        free_printer_list();
        free_job_list();
        close(2);

        return quit ? -1 : 0;
    }

    while (!EOF)
    {
        // if out != stdout, suppress the prompt
        char *prompt = "";
        if (out == stdout)
        {
            prompt = "imp>";
        }
        char *input = sf_readline(prompt);
        char *delim = " ";

        quit = handle_input(input, delim, out, quit);

        // non-empty input
        if (quit)
        {
            free_printer_list();
            free_job_list();
            free(input);
            break;
        }

        free(input);
    }

    close(1);
    close(0);
    return quit ? -1 : 0;
}

// fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
// abort();
