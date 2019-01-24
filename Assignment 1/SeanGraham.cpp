#include <iostream>
#include <string>
#include <queue>
//#include "ASSIGNMENT1.H"
#include <limits>
/*
Sean Graham
UH ID 1449668
Operating Systems Summer 2016
Professor Paris
04July2016
*/
using namespace std;
const int MAX_PROCESSES = 1000;
const int MAX_CORES = 100;
const int MAX_TASKS = 100;
const double INFINITY = std::numeric_limits<double>::infinity();
 
enum taskType {
    core, user, block, noblock
};
enum statusType {
    running, blocked, ready, terminated
};
struct ProcessTask {
    taskType type;
    double duration;
    ProcessTask(taskType _type, double _duration)
    {
        type = _type;
        duration = _duration;
    }
    ProcessTask(){
        type = core;
        duration = 0;
    }
};
struct Process {
    int pid;
    bool started;
    double startTime;
    double endTime;
    double allocatedTime;
    statusType status;
    queue<ProcessTask> tasks;
    Process(){
    status = blocked;
    }
};
struct Core {
    Process runningProcess;
    bool occupied;
    double completionTime;
};
queue<Process> readyQue;
queue<Process> diskQue;
queue<Process> userQue;
queue<Process> readProcessesFromFile;
//queue<Process> processList;
double timingClock = 0;
int terminatedProcesses = 0;
int totalCores;
int availableCores;
int pidCounter;
int indexNextCoreCompletion;
bool diskIdle = true;
Process diskProcess;
taskType diskTask;
bool userWait = false;
Process userProcess;
double diskCompletionTime = INFINITY;
double userCompletionTime = INFINITY;
Process processList[MAX_PROCESSES];
Core cores[MAX_CORES];
 
 
 
void readInput()
{
    string taskString;
    double duration;
    //int taskCounter;
    taskType task;
    //read how many cores there are
    cin >> taskString >> totalCores;
    availableCores = totalCores;
    //set core completion times for INFINITY for initializing program
    for (int i = 0; i < totalCores; i++)
    {
        cores[i].completionTime = INFINITY;
        cores[i].occupied = false;
    }
    //read the rest of the file
    while (cin >> taskString >> duration)
    {
            if (taskString == "START")
            {
                processList[pidCounter].startTime = duration;
                processList[pidCounter].pid = pidCounter;
                pidCounter++;
               // taskCounter = 0;
            }
            else
            {
                if (taskString == "CORE")
                {
                    task = core;
                } else if (taskString == "BLOCK")
                {
                    task = block;
                } else if (taskString == "NOBLOCK")
                {
 
                    task = noblock;
                } else if (taskString == "USER")
                {
                    task = user;
                }
                processList[pidCounter-1].tasks.push(ProcessTask(task, duration));
            }
    }
    //push the read processes into a queue for timing purposes
    for (int i = 0; i < pidCounter; i++)
    {
        readProcessesFromFile.push(processList[i]);
    }
}
 
void printProcess(Process x)
{
    string statusString;
    if (x.status == running)
    {
        statusString = "RUNNING";
    } else if (x.status == ready)
    {
        statusString = "READY";
    } else if (x.status == blocked)
    {
        statusString = "BLOCKED";
    } else if (x.status == terminated)
    {
        statusString = "TERMINATED";
    }
    cout << "PID: " << x.pid << " StartTime: " << x.startTime << " AllocatedTime: " << x.allocatedTime << " Status: " << statusString << endl;
};
void printTasks(Process x)
{
    cout << "Tasks Que Size: " <<  x.tasks.size() << endl;
    while (!x.tasks.empty())
    {
        ProcessTask temp = x.tasks.front();
        cout << temp.duration << endl;
        x.tasks.pop();
    }
}
void printProcessList()
{
    for (int i = 0; i < int(readProcessesFromFile.size()); i++)
    {
        printProcess(readProcessesFromFile.front());
        readProcessesFromFile.push(readProcessesFromFile.front());
        readProcessesFromFile.pop();
    }
}
 
void printQues()
{
    if (readyQue.empty() == false)
    {
        cout << "Contents of ready queue:" << endl;
        for (int i = 0; i < int(readyQue.size()); i++)
        {
            printProcess(readyQue.front());
            readyQue.push(readyQue.front());
            readyQue.pop();
        }
    }
    else
    {
        cout << "Ready Que is empty." << endl;
    }
    if (diskQue.empty() == false)
    {
        cout << "Contents of disk queue:" << endl;
        for (int i = 0; i < int(diskQue.size()); i++)
        {
            printProcess(diskQue.front());
            diskQue.push(diskQue.front());
            diskQue.pop();
        }
    }
    else
    {
        cout << "Disk Queue is empty." << endl;
    }
    if (userQue.empty() == false)
    {
        cout << "Contents of user queue:" << endl;
        for (int i = 0; i < int(userQue.size()); i++)
        {
            printProcess(userQue.front());
            userQue.push(userQue.front());
            userQue.pop();
        }
    }
    else
    {
        cout << "User Queue is empty." << endl;
    }
}
void printCoreProcesses()
{
    for (int i = 0; i < totalCores; i++)
    {
        if (cores[i].occupied == true)
        {
            cout << "Core " << i << ": ";
            printProcess(cores[i].runningProcess);
        }
    }
}
void printSummary(Process x)
{
        terminatedProcesses++;
        x.status = terminated;
        x.endTime = timingClock;
        cout << "Summary for PID: " << x.pid << " ending at " << x.endTime << endl;
        cout << "Number of busy cores: " << (totalCores - availableCores) << "." << endl;
        printCoreProcesses();
        printProcess(x);
        printQues();
        cout << endl;
}
 
void coreRequest(){
    //cout << "Core Request" << endl;
    if (!readyQue.empty())
    {
        Process x = readyQue.front();
        bool coreFound = false;
        int coreCounter = 0;
        if (availableCores >= 1)
        {
            while (coreFound != true && coreCounter != totalCores)
            {
                if (cores[coreCounter].occupied == false)
                {
                    cores[coreCounter].occupied = true;
                    double taskDuration = x.tasks.front().duration;
                    cores[coreCounter].completionTime = timingClock + taskDuration;
                    x.allocatedTime += taskDuration;
                    x.tasks.pop();
                    x.status = running;
                    cores[coreCounter].runningProcess = x;
                    coreFound = true;
                    readyQue.pop();
                    availableCores--;
                }
                coreCounter++;
            }
        }
    }
 
//    else if (coreFound == false && readyQue.front().pid != x.pid)
//    {
//            readyQue.push(x);
//    }
}
 
void coreCompletion(int finishedCore){
    //cout << "Core Completion" << endl;
    cores[finishedCore].occupied = false;
    timingClock = cores[finishedCore].completionTime;
    availableCores++;
    Process completedProcess = cores[finishedCore].runningProcess;
    if (completedProcess.tasks.empty())
    {
        printSummary(completedProcess);
//        terminatedProcesses++;
//        completedProcess.status = terminated;
//        completedProcess.endTime = timingClock;
//        printProcess(completedProcess);
//        printQues();
    }
    else
    {
        taskType nextTask = completedProcess.tasks.front().type;
        if (nextTask == block)
        {
            completedProcess.status = blocked;
            diskQue.push(completedProcess);
        }
        else if (nextTask == noblock)
        {
            completedProcess.status = ready;
            diskQue.push(completedProcess);
            completedProcess.tasks.pop();
            //cout << "Pushing to ready QUE from noblock" << endl;
            readyQue.push(completedProcess);
        }
        else if (nextTask == user)
        {
            completedProcess.status = blocked;
            userQue.push(completedProcess);
        }
    }
}
 
void diskRequest(){
    //cout << "Disk Request" << endl;
    if (diskIdle && (!diskQue.empty()))
    {
        diskIdle = false;
        diskProcess = diskQue.front();
        diskQue.pop();
        diskCompletionTime = timingClock + diskProcess.tasks.front().duration;
        diskTask = diskProcess.tasks.front().type;
        if (diskTask == block) //no block task was popped in the disk completion
        {
            diskProcess.tasks.pop();
        }
    }
}
//need to do some sort of checking for blocking/no blocking in the disk completion, do not push no blocking to ready que because it was already pushed there by the coreCompletion()
void diskCompletion(){
    //cout << "Disk Completion" << endl;
    timingClock = diskCompletionTime;
    diskIdle = true;
    diskCompletionTime = INFINITY;
    if (diskProcess.tasks.empty())
    {
        printSummary(diskProcess);
//        terminatedProcesses++;
//        diskProcess.status = terminated;
//        diskProcess.endTime = timingClock;
//        printProcess(diskProcess);
//        printQues();
    }
    else
    {
        taskType nextTask = diskProcess.tasks.front().type;
        if (nextTask == core && diskTask == block)
        {
            diskProcess.status = ready;
            //cout << "Pushing to ready Que from Disk Completion" << endl;
            readyQue.push(diskProcess);
        }
    }
 
}
void userRequest(){
    //cout << "User Request" << endl;
    if (!userWait && !userQue.empty())
    {
        userProcess = userQue.front();
        userQue.pop();
        userWait = true;
        userCompletionTime = timingClock + userProcess.tasks.front().duration;
        userProcess.tasks.pop();
    }
}
void userCompletion(){
    //cout << "User Completion" << endl;
    userWait = false;
    timingClock = userCompletionTime;
    userCompletionTime = INFINITY;
    if (userProcess.tasks.empty())
    {
        printSummary(userProcess);
//        terminatedProcesses++;
//        userProcess.status = terminated;
//        userProcess.endTime = timingClock;
//        printProcess(userProcess);
//        printQues();
    }
    else
    {
        taskType nextTask = userProcess.tasks.front().type;
        if (nextTask == core)
        {
            userProcess.status = ready;
            readyQue.push(userProcess);
        }
    }
}
//only the last case is working for starting a process!
void eventHandler(){
   // cout << "Event Handler" << endl;
    double nextProcessStartTime = INFINITY;
    if (!readProcessesFromFile.empty())
    {
        nextProcessStartTime = readProcessesFromFile.front().startTime;
    }
    if (availableCores > 0 && (!readyQue.empty()))
    {
        coreRequest();
    }
    else if (diskIdle == true && (!diskQue.empty()))
    {
        diskRequest();
    }
    else if (userWait == false && (!userQue.empty()))
    {
        userRequest();
    }
    else //this will check stuff after empty devices are filled up
    {
        //find the next core that will complete to compare with disk and user completion times
        double coreMin = INFINITY; //change to infinity?
        int minCore = 0;
        for (int i = 0; i < totalCores; i++)
        {
            if (cores[i].occupied == true)
            {
                if (cores[i].completionTime <= coreMin)
                {
                    coreMin = cores[i].completionTime;
                    minCore = i;
                }
                //coreMin = min(coreMin, cores[i].completionTime);
            }
        }
        if (coreMin <= diskCompletionTime && coreMin <= userCompletionTime && coreMin <= nextProcessStartTime)
        {
            coreCompletion(minCore);
            coreRequest();
        }
        else if (diskCompletionTime <= coreMin && diskCompletionTime <= userCompletionTime && diskCompletionTime <= nextProcessStartTime)
        {
            diskCompletion();
            diskRequest();
        }
        else if (userCompletionTime <= coreMin && userCompletionTime <= diskCompletionTime && userCompletionTime <= nextProcessStartTime)
        {
            userCompletion();
            userRequest();
        }
        else if (nextProcessStartTime <= coreMin && nextProcessStartTime <= diskCompletionTime && nextProcessStartTime <= userCompletionTime) //&& nextProcessStartTime < timingClock)//might need equals
        {
            //cout << "Next Process start" << endl;
            Process startingProcess = readProcessesFromFile.front();
            readProcessesFromFile.pop();
            timingClock = startingProcess.startTime;
            startingProcess.started = true;
            startingProcess.status = ready;
            readyQue.push(startingProcess);
            //none of the other cases are working!!!!!!
        }
    }
}
 
int main()
{
    readInput();
    while (terminatedProcesses < pidCounter)
    {
        eventHandler();
    }
    return 0;
}