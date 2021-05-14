#define DEBUG
 
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
 
//files for temporary saving pids 
#define ZERO_PID_FILE "zero.txt"
#define FIRST_PID_FILE "first.txt"
#define SECOND_PID_FILE "second.txt"
#define THIRD_PID_FILE "third.txt"
#define FOURTH_PID_FILE "fourth.txt"
#define FIFTH_PID_FILE "fifth.txt"
#define SIXTH_PID_FILE "sixth.txt"
#define SEVENTH_PID_FILE "seventh.txt"
#define EIGHTH_PID_FILE "eighth.txt"
 
//pointer for program name
char* programName;
int receivedByFirstProcess = 0;
int currentProcess = 0;
int signalUSR1 = 0;
 
 
//files array
const char *FILES[] = { ZERO_PID_FILE, FIRST_PID_FILE, SECOND_PID_FILE,
             THIRD_PID_FILE, FOURTH_PID_FILE, FIFTH_PID_FILE,
             SIXTH_PID_FILE, SEVENTH_PID_FILE, EIGHTH_PID_FILE 
            };
        
 
//delete all files after using them
void deleteFiles() {
    for (int i = 0; i < 9; i++) {
        errno = 0;
            if (remove(FILES[i]) == -1) {
                    fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), FILES[i]);
            }
        }
}
 
int readPidFromFile(char *filename) {
    FILE *file = fopen(filename, "r+");
        //error durign opening
        if (file == NULL) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), filename);
            exit(EXIT_FAILURE);
    }
        //getting pid from file
        char buf[64];
        fgets(buf, 64, file);
        errno = 0;
        //string to long int
        long pid = strtol(buf, NULL, 10);
        //if some errors
        if (errno != 0) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), filename);
            exit(EXIT_FAILURE);
        }
        //error during closing file
        if (fclose(file) == EOF) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), filename);
            exit(EXIT_FAILURE);
        }
        return (int) pid;
}
 
//writing pid to file
void writePidToFile(int pid, char *filename) {
        FILE *file = fopen(filename, "w+");
        //if file equal NULL
        if (file == NULL) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), filename);
            exit(EXIT_FAILURE);
        }
    
        fprintf(file, "%d", pid);
    
        //if closing error
        if (fclose(file) == EOF) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), programName, strerror(errno), filename);
            exit(EXIT_FAILURE);
        }
}
 
//getting time in milliseconds
long getTime() {
        struct timeval time;
        gettimeofday(&time, NULL);
        return time.tv_usec;
}
 
void receiveTERM() {
        fprintf(stdout, "%d %d terminated after %d SIGUSR1\n", getpid(), getppid(), signalUSR1);
        fflush(stdout);
}
 
void receiveUSR1() {
        signalUSR1++;
        fprintf(stdout, "%d %d %d received SIGUSR1 %ld\n", currentProcess, getpid(), getppid(), getTime());
        fflush(stdout);
}
 
void sendUSR1ToGroup(int processNumber, int group) {
        fprintf(stdout, "%d %d %d sent SIGUSR1 %ld\n", processNumber, getpid(), getppid(), getTime());
        fflush(stdout);
        killpg(group, SIGUSR1);
}
 
void sendUSR1(int processNumber, int pid) {
        fprintf(stdout, "%d %d %d sent SIGUSR1 %ld\n", processNumber, getpid(), getppid(), getTime());
        fflush(stdout);
        kill(pid, SIGUSR1);
}
 
void bindSignalAction(int signal, void (*function)(int)) {
        //memory allocation
        struct sigaction* action;
        action = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    
        //attach function to action handler
        action->sa_handler = function;
        sigaction(signal, action, 0);
}
 
void USR1EightProcessHandler(int sig_num) {
        receiveUSR1();
        int firstPid = readPidFromFile(FIRST_PID_FILE);
        sendUSR1(currentProcess, firstPid);
}
 
void USR1SeventhProcessHandler(int sig_num) {
        receiveUSR1();
}
 
 
void USR1SixthProcessHandler(int sig_num) {
        receiveUSR1();
        int seventhPid = readPidFromFile(SEVENTH_PID_FILE);
        sendUSR1ToGroup(currentProcess, seventhPid);
}
 
void USR1FifthProcessHandler(int sig_num) {
        receiveUSR1();
        int sixthPid = readPidFromFile(SIXTH_PID_FILE);
        sendUSR1(currentProcess, sixthPid); 
}
 
void USR1FourthProcessHandler(int sig_num) {
        receiveUSR1();
}
 
void USR1ThirdProcessHandler(int sig_num) {
        receiveUSR1();
}
 
void USR1SecondProcessHandler(int sig_num) {
        receiveUSR1();
        int fourthPid = readPidFromFile(FOURTH_PID_FILE);
        sendUSR1ToGroup(currentProcess, fourthPid);
}
 
void USR2FirstProcessHandler(int signal) {
    //killing processes
    int secondPid = readPidFromFile(SECOND_PID_FILE);
        kill(secondPid, SIGTERM);
        wait(0);
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void USR1FirstProcessHandler(int signal) {
        receiveUSR1();
        receivedByFirstProcess++;
        //receivedByFirstProcess = 101;
        
    int secondPid = readPidFromFile(SECOND_PID_FILE);
        if (receivedByFirstProcess == 101) {
            //send signal to zeroth process for printing processes tree
            //before other signals are dead 
            int zeroPid = readPidFromFile(ZERO_PID_FILE);
            kill(zeroPid, SIGUSR1);
            pause();
        }
        puts("");
        sendUSR1ToGroup(currentProcess, secondPid);
}
 
 
void USR1ZeroProcessHandler(int signal) {
#ifdef DEBUG
    int zerothPid = readPidFromFile(ZERO_PID_FILE);
    char psTreeCommand[64] = { 0 };
        sprintf(psTreeCommand, "pstree -p %d", zerothPid);
        system(psTreeCommand);
        //tell first process to start terminate processes
        kill(readPidFromFile(FIRST_PID_FILE), SIGUSR2);
#endif
}
 
void TERMEightProcessHandler(int sig_num) {
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMSeventhProcessHandler(int sig_num) {
        int eighthPid = readPidFromFile(EIGHTH_PID_FILE);
        kill(eighthPid, SIGTERM);
        wait(0);
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMSixthProcessHandler(int sig_num) {
        int seventhPid = readPidFromFile(SEVENTH_PID_FILE);
        kill(seventhPid, SIGTERM);
        wait(0);
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMFifthProcessHandler(int sig_num) {
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMFourthProcessHandler(int sig_num) {
        int fifthPid = readPidFromFile(FIFTH_PID_FILE);
        kill(fifthPid, SIGTERM);
        wait(0);
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMThirdProcessHandler(int sig_num) {
        int sixthPid = readPidFromFile(SIXTH_PID_FILE);
        kill(sixthPid, SIGTERM);
        wait(0);
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void TERMSecondProcessHandler(int sig_num) {
        int thirdPid = readPidFromFile(THIRD_PID_FILE);
        int forthPid = readPidFromFile(FOURTH_PID_FILE);
        kill(thirdPid, SIGTERM);
        kill(forthPid, SIGTERM);
        for (int i = 0; i < 2; i++) {
            wait(0);
        }
        receiveTERM();
        exit(EXIT_SUCCESS);
}
 
void initEighthProcess(){
    currentProcess = 8;
    
    bindSignalAction(SIGTERM, TERMEightProcessHandler);
    bindSignalAction(SIGUSR1, USR1EightProcessHandler);
 
    writePidToFile(getpid(), EIGHTH_PID_FILE);
        
    while (1) {
            pause();
        }
}
 
void initSeventhProcess(){
    currentProcess = 7;
    
    bindSignalAction(SIGTERM, TERMSeventhProcessHandler);
    bindSignalAction(SIGUSR1, USR1SeventhProcessHandler);
    
    if (setpgrp() == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
    
    int eighth_pid = fork();
    if (eighth_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (eighth_pid == 0) {
            initEighthProcess();
        }
        
        writePidToFile(getpid(), SEVENTH_PID_FILE);
        
        if (setpgid(eighth_pid, getpid()) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
        
    while (1) {
            pause();
        }
}
 
void initSixthProcess(){
    currentProcess = 6;
    
    bindSignalAction(SIGTERM, TERMSixthProcessHandler);
    bindSignalAction(SIGUSR1, USR1SixthProcessHandler);
    
    if (setpgid(getpid(), getpid()) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
    
    int seventh_pid = fork();
    if (seventh_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (seventh_pid == 0) {
            initSeventhProcess();
        }
        
        
        writePidToFile(getpid(), SIXTH_PID_FILE);
        
        
    while (1) {
            pause();
        }
}
 
void initFifthProcess(){
    currentProcess = 5;
    
    bindSignalAction(SIGTERM, TERMFifthProcessHandler);
    bindSignalAction(SIGUSR1, USR1FifthProcessHandler);
    
    writePidToFile(getpid(), FIFTH_PID_FILE);
    
    while (1) {
            pause();
        }
}
 
void initFourthProcess(){
    currentProcess = 4;
    bindSignalAction(SIGTERM, TERMFourthProcessHandler);
    bindSignalAction(SIGUSR1, USR1FourthProcessHandler);
    
    int fifth_pid = fork();
    if (fifth_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (fifth_pid == 0) {
            initFifthProcess();
        }
        
        //same as setpgid(getpid(), getpid()
        if (setpgrp() == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
 
        if (setpgid(fifth_pid, getpid()) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
        
        writePidToFile(getpid(), FOURTH_PID_FILE);
        
    while (1) {
            pause();
        }
}
 
void initThirdProcess(){
    currentProcess = 3;
    
    bindSignalAction(SIGUSR1, USR1ThirdProcessHandler);
    bindSignalAction(SIGTERM, TERMThirdProcessHandler);
    
    int sixth_pid = fork();
    if (sixth_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (sixth_pid == 0) {
            initSixthProcess();
        }
        
        writePidToFile(getpid(), THIRD_PID_FILE);
        
    while (1) {
            pause();
        }
}
 
void initSecondProcess(){
    currentProcess = 2;
    
    bindSignalAction(SIGUSR1, USR1SecondProcessHandler);
    bindSignalAction(SIGTERM, TERMSecondProcessHandler);
    
    
    int third_pid = fork();
    if (third_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (third_pid == 0) {
            initThirdProcess();
        }
        
        if (setpgrp() == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
 
        if (setpgid(third_pid, getpid()) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
        
        int fourth_pid = fork();
    if (fourth_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (fourth_pid == 0) {
            initFourthProcess();
        }
        
                
        writePidToFile(getpid(), SECOND_PID_FILE);
    
        while (1) {
            pause();
        }
}
 
void initFirstProcess(){
    currentProcess = 1;
    
    bindSignalAction(SIGUSR1, USR1FirstProcessHandler);
    bindSignalAction(SIGUSR2, USR2FirstProcessHandler);
    
    
    if (setpgid(getpid(), getpid()) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
    
    int second_pid = fork();
    if (second_pid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
            exit(EXIT_FAILURE);
        } else if (second_pid == 0) {
            initSecondProcess();
        }
    
    writePidToFile(getpid(), FIRST_PID_FILE);
    
    //while 8th file is empty
    struct stat buf;
        while (lstat(EIGHTH_PID_FILE, &buf) == -1)
            continue;
            
        int zero_pid = readPidFromFile(ZERO_PID_FILE);
        
        if (setpgid(getpid(), zero_pid) == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), programName, strerror(errno));
        }
        
        sendUSR1ToGroup(currentProcess, second_pid);
        
        while (1) {
            pause();
        }
}
//process tree
// 1->2 2->(3,4) 4->5 3->6 6->7 7->8
//interprocess signals
// 1->(2,3) SIGUSR1 2->(4,5) SIGUSR1 5->6 SIGUSER1 6->(7,8) SIGUSER1 8->1 SIGUSER1
 
int main(int argc, char* argv[]) {
    programName = (char *) calloc(15, sizeof(char));
    //getting programm name without / 'symbol'
    //strrchr find last symbol index in string
    strcpy(programName, strrchr(argv[0], '/') + 1);
    if (argc != 1) {
        fprintf(stderr, "%s: Error: Wrong amount of arguments.\n", programName);
        return 1;
    }
    
    bindSignalAction(SIGUSR1, USR1ZeroProcessHandler);
    
    //writing zero pid to file
    writePidToFile(getpid(), ZERO_PID_FILE);
    
    int firstPid = fork();  
    
    switch (firstPid) {
            case -1: {
                fprintf(stderr, "%s: %s\n", programName, strerror(errno));
                break;
            }
        case 0: {
                initFirstProcess();
                break;
        }
            default: {
                    wait(0);
            }
    }
 
    //delete all files with pids after execution
        deleteFiles();
        return 0;
}
