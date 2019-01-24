////////////////////////////////
//        Ryan Gonzalez       //
//   PeopleSoft ID: 1330002   //
//      Operating Systems     // 
////////////////////////////////
#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
using namespace std; 

//global variables 
int number; 
int slice; 
int numberOfCores; 
int line = -2;
int processNum = 0;
int globalTime = 0;
bool BTdisk = false; 
string command;

//struct for my process table. 
struct processVector{
    int processNumber; 
    int startLine;
    int endLine;
    int currentLine;
    int startTime;
    string state;
};

//struct for my event list.
struct eventVector{
	int processNumber; 
	int startTime; 
	int coreNumber; 
	string eventName; 
};

//struct for my data table.
struct dataVector{
	string name; 
	int time; 
	int line; 
}; 

//function declarations. 
void addEvent(eventVector Event);
void printInfo(int value1, int value2);

vector <dataVector> dataTable; //vector to process my dataTable of dataVector type.  
vector <processVector> processTable; //vector to process my processTable of processVector type. 
vector <eventVector> eventList; //vector to process my eventList of eventList type. 
queue <int> coreQueue; 
queue <int> diskQueue;

//function returns core with lowest busy time. 
int checkFreeCore(bool array[]){ 
    for(int i = 0; i < numberOfCores; i++){
        if(array[i] == false){ return i;}
    }
    return -1;
}

//function returns the number of busy cores by checking if array at index is set to boolean true. 
int numberOfBusyCores(bool array[]){
    int counter = 0;
    for(int i = 0; i < numberOfCores; i++){
        if(array[i] == true){ counter++; }
    }
    return counter;
}

int main(){
//three declarations to store data from input file accordingly. 
dataVector dataHolder; 
processVector processHolder;
eventVector eventHolder;         

    while (cin >> command){
    	cin >> number;
        if(command == "NCORES"){ numberOfCores = number; } //tells simulation how many cores we have. 
        else if(command == "SLICE"){ slice = number; } //tells simulation how much time we are assigned. 
        else if(command == "CORE"){ //split the core input into seperate core requests if time is less than slice.  
            if(number > slice){
                while(number > slice){
                    dataHolder.name = command;
                    dataHolder.time = slice;
                    dataHolder.line = line;
                    dataTable.push_back(dataHolder);
                    number = number - slice;
                    line++;
                }
            }
            dataHolder.name = command;
            dataHolder.time = number;
            dataHolder.line = line;
            dataTable.push_back(dataHolder);
        }

        //adds input data into appropriate sections of vector dataVector
        else if(command == "DISK" || command == "INPUT"|| command == "DISPLAY" || command == "NEW"){
            dataHolder.name = command;
            dataHolder.time = number;
            dataHolder.line = line;
            dataTable.push_back(dataHolder);
        }
        
        //Adds new processes immediately.
        if(command == "NEW"){
            if(processTable.size() != 0){ processTable.back().endLine = line-1; }
            processHolder.processNumber = processNum++;
            processHolder.startLine = line;
            processHolder.currentLine = line;
            processHolder.startTime = number;
            processHolder.state = "BLOCKED";
            processTable.push_back(processHolder);
            eventHolder.processNumber = processHolder.processNumber;
            eventHolder.startTime = processHolder.startTime;
            eventHolder.eventName = "NEW";
            eventHolder.coreNumber = -1;
            addEvent(eventHolder);
        }
        line++;   
    }

    processTable[processTable.size()-1].endLine = line-1;

    //boolean array im going to use to be able to check and see if core is in use or not. 
    bool coreInUse[numberOfCores];
    for(int i = 0; i < numberOfCores; i++){ coreInUse[i] = false; }

    //inputting data is now complete, now starting the simulation. 
    eventVector currentT;
	eventVector nextE;
    processVector processT;

    //while loop that will run until there are no events left to process 
    while(eventList.size() != 0){
        nextE = eventList.back();
        eventList.pop_back(); 
        globalTime = nextE.startTime; //keeps the clock running by setting global time equal to the event time.
        processT = processTable[nextE.processNumber];
        currentT.processNumber = nextE.processNumber; //set process number of the next event to process number of the current event. 
        currentT.coreNumber = nextE.coreNumber; //keep track of the coreNumber in order to free it when necessary. 

        if(nextE.eventName == "NEW"){ //if new, shift the eventName of event to be CORE, since we know a core request will follow.
            nextE.eventName = "CORE";
            processT.currentLine++; }

        if(nextE.eventName == "END"){ //handle event if the current processing event is ending. 
            if(dataTable[processT.currentLine].name == "CORE"){ //if a core is free'd, set core to now free or immediately load up the next process. 
                processT.currentLine++;
                if(coreQueue.empty()){
                    coreInUse[nextE.coreNumber] = false; }
                else {
                    currentT.processNumber = coreQueue.front();
                    coreQueue.pop();
                    currentT.startTime = globalTime + dataTable[processTable[currentT.processNumber].currentLine].time;
                    currentT.eventName = "END";
                    addEvent(currentT);
                    processTable[currentT.processNumber].state = "RUNNING";
                    currentT.processNumber = nextE.processNumber; }

                if(processT.currentLine > processT.endLine){
                    currentT.eventName = "TERM";
                    currentT.startTime = globalTime;
                    addEvent(currentT); } 

                else if(dataTable[processT.currentLine].name == "CORE"){ //check if more CPU time is required if there are more things required in the current process.
                    int value1 = checkFreeCore(coreInUse);

                    if(value1 != -1){
                        currentT.eventName = "END";
                        currentT.startTime = globalTime+dataTable[processT.currentLine].time;
                        currentT.coreNumber = value1;
                        addEvent(currentT);
                        coreInUse[nextE.coreNumber] = true; }
                    else{
                        coreQueue.push(nextE.processNumber);
                        processT.state = "READY"; }
                }

                else{
                    currentT.eventName = dataTable[processT.currentLine].name;
                    currentT.startTime = globalTime;
                    addEvent(currentT);
                    processT.state = "BLOCKED"; }
            }

            else if(dataTable[processT.currentLine].name == "DISK"){ //if the disk is free'd, load in the next process to use the disk, if there aren't any processes waiting to use the disk, set status to IDLE
                processT.currentLine++;
                if(diskQueue.empty()){ BTdisk = false; }
                else{
                    currentT.processNumber = diskQueue.front();
                    diskQueue.pop();
                    currentT.startTime = globalTime + dataTable[processTable[currentT.processNumber].currentLine].time;
                    currentT.eventName = "END";
                    addEvent(currentT);
                    currentT.processNumber = nextE.processNumber; }
                if(processT.currentLine > processT.endLine){
                    currentT.eventName = "TERM";
                    currentT.startTime = globalTime;
                    addEvent(currentT); }
                else{
                    currentT.eventName = dataTable[processT.currentLine].name;
                    currentT.startTime = globalTime;
                    addEvent(currentT); }
                
            }
            else if(dataTable[processT.currentLine].name == "DISPLAY" || dataTable[processT.currentLine].name == "INPUT"){ //if a process finishes DISPLAY/INPUT command, check if the process is completed, else add next step into eventList
                processT.currentLine++;
                if(processT.currentLine > processT.endLine){
                    currentT.eventName = "TERM";
                    currentT.startTime = globalTime;
                    addEvent(currentT); }
                else{
                    currentT.eventName = dataTable[processT.currentLine].name;
                    currentT.startTime = globalTime;
                    addEvent(currentT); }
            }
        }

        if(nextE.eventName == "TERM"){ //handle processes that need to be terminated
            processT.state = "TERMINATED";
            processTable[nextE.processNumber] = processT;
            printInfo(nextE.processNumber, numberOfBusyCores(coreInUse));
        }

        if(nextE.eventName == "CORE"){ //handle processes that requests cores
            int value2 = checkFreeCore(coreInUse);
            if(value2 != -1){
                coreInUse[value2] = true;
                currentT.eventName = "END";
                currentT.startTime = globalTime + dataTable[processT.currentLine].time;
                currentT.coreNumber = value2;
                addEvent(currentT);
                processT.state = "RUNNING"; }
            else{
                coreQueue.push(nextE.processNumber);
                processT.state = "READY"; }
        }

        if(nextE.eventName == "DISK"){ //handle processes that requests disk 
            if(dataTable[processT.currentLine].time == 0){
                processT.currentLine++;
                if(processT.currentLine > processT.endLine){
                    currentT.eventName = "TERM";
                    currentT.startTime = globalTime;
                    addEvent(currentT); }
                else{
                    currentT.eventName = dataTable[processT.currentLine].name;
                    currentT.startTime = globalTime;
                    addEvent(currentT); }   
            }
            else if(BTdisk == false){
                BTdisk = true;
                currentT.eventName = "END";
                currentT.startTime = globalTime + dataTable[processT.currentLine].time;
                addEvent(currentT); }
            else{ diskQueue.push(nextE.processNumber); }
        }

        if(nextE.eventName == "DISPLAY" || nextE.eventName == "INPUT"){ //handle all display and input requests. 
            currentT.eventName = "END";
            currentT.startTime = globalTime + dataTable[processT.currentLine].time;
            addEvent(currentT); }

        processTable[nextE.processNumber] = processT; //update changes to processTable. 
    }
    return 0;
}

void addEvent(eventVector Event){
    //addEvent adds event E into its appropriate position. Highest priority is time that the event is going to end, if an event time matches another event, then event name is checked. 
    //END events occur first, TERM events occur last, if event names match then it sorts in order of process number. 
    int counter = 0;
    while(counter <= eventList.size()){
        if(counter == eventList.size()){
            eventList.push_back(Event);
            counter = eventList.size()+1; }

        else if(Event.startTime > eventList[counter].startTime){
            eventList.insert(eventList.begin() + counter, Event);
            counter = eventList.size() + 1; }

        else if(Event.startTime == eventList[counter].startTime){ //handles events where times match and seperates accordingly. 
            if(Event.eventName == "END"){
                while(counter != eventList.size() && (eventList[counter].eventName != Event.eventName || eventList[counter].eventName == Event.eventName && eventList[counter].startTime > Event.startTime) && Event.startTime == eventList[counter].startTime){ 
                    counter++; }
                
                if(counter == eventList.size() || Event.startTime != eventList[counter].startTime){                                                     
                    counter--; }
                else{
                    eventList.insert(eventList.begin()+counter,Event);
                    counter = eventList.size() + 1; }
            }

            else if(Event.eventName == "TERM"){
                eventList.insert(eventList.begin() + counter, Event);
                counter = eventList.size() + 1; }

            else if((Event.processNumber > eventList[counter].processNumber || eventList[counter].eventName == "END") && eventList[counter].eventName != "TERM"){
                eventList.insert(eventList.begin() + counter, Event);
                counter = eventList.size() + 1; }   
        }
        counter++;
    }
}

//function that prints to screen all relevant information about the system. 
void printInfo(int value1, int value2){
    cout << endl << "CURRENT STATE OF THE SYSTEM AT t = " << globalTime << " ms";
    cout << endl << "Current number of busy cores: " << value2;
    cout << endl << "Disk is "; 

    if (BTdisk > globalTime){ cout << "BUSY"; } 
    else { cout << "IDLE"; }

    cout << endl << "READY QUEUE:" << endl;
    queue<int> temp = coreQueue;
    if(temp.empty()){ cout << "empty"; }
    else{
        while(!temp.empty()){
            cout << "Process " << temp.front();
            temp.pop();
            if(!temp.empty()){ cout << ", "; }
        }
    }

    cout << endl << "DISK QUEUE:" << endl;
    temp = diskQueue;
    if(temp.empty()){ cout << "empty"; }
    else{
        while(!temp.empty()){
            cout << "Process " << temp.front();
            temp.pop();
            if(!temp.empty()){ cout << ", "; }
        }
    }

    cout << endl << "PROCESS TABLE:" << endl;
    for(int i = 0; i < processTable.size(); i++){
        processVector p = processTable[i];
        if(p.state != "TERMINATED" || value1 == i){
            cout << "Process " << i << " started at " << p.startTime << " ms and is " << p.state << endl;
        }
    }
}