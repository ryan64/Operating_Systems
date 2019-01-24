/*
    Nathan Pham 1346757
    COSC 3360 SPRING 2017
*/

//imports
#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;

// Structs used to create my dataTable, processTable, and eventList
struct data{
    string name;
    int time,line;
};
struct process{
    int pnum,sline,eline,cline,stime; // process number, start line, end line, current line, start time(of process)
    string state; //current status
};
struct event{
    int pnum, stime,corenum; // process number, start time, core number
    string ename; // event name
};

//Methods to be used
void inputData();
void insertEvent(event e);
void print(int n, int c);
int getFreeCore(bool bt[]);
int numBusyCores(bool bt[]);
void showQueue(queue<int> q);
void showProcessTable(int n);
/*
For debugging purposes
void showEventList();
void showDataTable();
void showProcessTable();
*/

//Global Variables
string action; //Holds input data names
int num, slice, nCores; // num is container for input data times
int line = -2; //line = -2 so we don't count NCORE and SLICE into our data
int pronum = 0; //a counter to establish process numbers when inserting into process tables
int globalTime = 0; // overall clock
bool diskBusy = false; //checks to see if disk is currently busy
vector<data> dataTable; //acts as table for input data
vector<process> processTable; //acts as table for processes
vector<event> eventList; //orignally used as a table but changed into a priority queue over time
queue<int> coreQueue;
queue<int> diskQueue;


int main(){
    
    //processing input data into tables
    inputData(); 
    bool busyTimes[nCores]; //An bool array that contains whether a core is being used or not
    for(int x = 0; x < nCores; x++){ 
        busyTimes[x] = false;
    }
    
    //temporary containers used in the simulation
    event tmp; 
    process p; 
    event next;

    //start simulation
    while(eventList.size()!=0){ //The program will end when there are no events left to process 
                                //The eventList vector ended up being a makeshift priority queue with the back being where the earliest event is
        next = eventList.back();
        eventList.pop_back();
        globalTime = next.stime; // when we get the earliest event in the list we set the global time equal to the event time to keep the clock running
        p = processTable[next.pnum]; //This just makes typing out dataTable[processTable[next.pnum]] a bit easier
        tmp.pnum = next.pnum; // setting the process number of the future event to the process number of the current event
        tmp.corenum = next.corenum; //Also need to keep track of the corenum in order to free it

        //We then deal with each event name accordingly
        if(next.ename == "NEW") // protocol for a completely new process
        {
            next.ename = "CORE"; //Just shift the ename of current event to be core since a core request always follows
            p.cline++; //we must also increment the current line since we're going from NEW to the next line
        } //Using this way, the CORE if statement will catch this event in the same iteration rather than creating a new event and waiting

        if(next.ename == "END") // The protocol if the current processing event is freeing up something
        {
            if(dataTable[p.cline].name == "CORE") //if a core frees up we either claim that the core is now free or we immediately load up the next process in coreQueue
            {
                p.cline++; //Since process finishes up one of it's steps we must increment the current line'
                if(coreQueue.empty())
                {
                    busyTimes[next.corenum] = false;
                }
                else
                {
                    //process number of process next in line
                    tmp.pnum = coreQueue.front();
                    coreQueue.pop();
                    tmp.stime = globalTime+dataTable[processTable[tmp.pnum].cline].time;
                    tmp.ename = "END";
                    insertEvent(tmp);
                    processTable[tmp.pnum].state = "RUNNING";
                    tmp.pnum = next.pnum; //ensure that the tmp is on the same pnum for current event instead of future event
                }
                if(p.cline>p.eline) //A check to see if the current process is now finished and needs to be terminated
                {
                    tmp.ename = "TERM";
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                else if(dataTable[p.cline].name == "CORE") //if there are more steps in the current process we must then check to see if it still need CPU time
                {
                    //essentially do a CORE protocol within the END protocol
                    int a = getFreeCore(busyTimes);
                    if(a!=-1)
                    {
                        tmp.ename = "END";
                        tmp.stime = globalTime+dataTable[p.cline].time;
                        tmp.corenum = a;
                        insertEvent(tmp);
                        busyTimes[next.corenum] = true;
                    }
                    else
                    {
                        coreQueue.push(next.pnum);
                        p.state = "READY";
                    }
                    
                }
                else // if it doesn't need anymore CPU time, we then insert the next step
                {
                    tmp.ename = dataTable[p.cline].name;
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                    p.state = "BLOCKED";
                }
            }
            else if(dataTable[p.cline].name == "DISK") // if the disk frees up we must load in the next process to use the disk.
            {                                          // if there aren't any processes waiting to use the disk, we set it to IDLE
                p.cline++;
                if(diskQueue.empty())
                {
                    diskBusy = false;
                }
                else
                {
                    tmp.pnum = diskQueue.front();
                    diskQueue.pop();
                    tmp.stime = globalTime+dataTable[processTable[tmp.pnum].cline].time;
                    tmp.ename = "END";
                    insertEvent(tmp);
                    tmp.pnum = next.pnum; //ensure that the tmp is on the same pnum for current event
                }
                if(p.cline>p.eline) //checks to see if current process needs to be terminated
                {
                    tmp.ename = "TERM";
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                else //otherwise we insert the process's next step
                {
                    tmp.ename = dataTable[p.cline].name;
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                
            }
            else if(dataTable[p.cline].name == "DISPLAY"||dataTable[p.cline].name == "INPUT") //if a process finishes a DISPLAY or INPUT we check to see if the process is done 
            {                                                                                 //otherwise we insert the next step into eventList
                p.cline++;
                if(p.cline>p.eline)
                {
                    tmp.ename = "TERM";
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                else
                {
                    tmp.ename = dataTable[p.cline].name;
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                
            }
        }

        if(next.ename == "TERM") //protocol if a process is to be terminated
        {
            p.state = "TERMINATED";
            processTable[next.pnum] = p;
            print(next.pnum,numBusyCores(busyTimes));
        }

        if(next.ename=="CORE") //protocol if a process requests a core
        {
            int c = getFreeCore(busyTimes); //checks to see if a core is free, if there isn't a free core c's value is set to -1
            if(c != -1) //if there is a free core, set it's state to busy and insert the time that it will be freed up
            {
                busyTimes[c] = true;
                tmp.ename = "END";
                tmp.stime = globalTime+dataTable[p.cline].time;
                tmp.corenum = c;
                insertEvent(tmp);
                p.state = "RUNNING";
            }
            else //if all cores are busy then add process number into the coreQueue
            {
                coreQueue.push(next.pnum);
                p.state = "READY";
            }
        }

        if(next.ename == "DISK") // protocol if a process requests the disk
        {
            if(dataTable[p.cline].time==0) //checks for special case of 0ms disk access
            {                              //if 0ms disk access then immediately skip to the next step
                p.cline++;
                if(p.cline>p.eline) //checks to see if process needs to be terminated
                {
                    tmp.ename = "TERM";
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                else //otherwise insert next step
                {
                    tmp.ename = dataTable[p.cline].name;
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                
            }
            else if(diskBusy == false) //if disk is busy then give the process the slot
            {
                diskBusy = true;
                tmp.ename = "END";
                tmp.stime = globalTime+dataTable[p.cline].time;
                insertEvent(tmp);
            }
            else //otherwise add it to the queue
            {
                diskQueue.push(next.pnum);
            }
        }
        if(next.ename == "DISPLAY"||next.ename == "INPUT") // protocol for display and input requests
        {                                                  // basically just keep the process on hold until display/input is complete
            tmp.ename = "END";
            tmp.stime = globalTime+dataTable[p.cline].time;
            insertEvent(tmp);
        }
        processTable[next.pnum] = p; //commit all changes to processTable
    }
    return 0;
}
void inputData()
{
        // This part enters all the data from any input file into the vector dataTable.name
    // It also will split all events of CORE that take up more time than the given slice into
    // separate events inorder to bring it back to queue in round robin fashion
    data dtmp;
    process ptmp;
    event etmp;
    while(cin>>action)
    {
        if(action=="NCORES"||action=="SLICE"||action=="CORE"||action=="DISK"||action=="INPUT"||action=="DISPLAY"||action=="NEW"){
            cin>>num;
            if(cin.fail())
            {
                action = "ERROR";
            }
        }
        else
        {
            cout<<action;
            action = "ERROR";
        }
        // assigns number of cores
        if(action=="NCORES"){
            nCores = num;
        }
        // assigns length of time slice
        else if(action == "SLICE"){
            slice = num;
        }
        // splits the core input into seperate core requests of time less than or equal to slice
        else if(action == "CORE"){
            if(num>slice)
            {
                while(num>slice) //loops until num<slice
                {
                    dtmp.name = action;
                    dtmp.time = slice;
                    dtmp.line = line;
                    dataTable.push_back(dtmp);
                    num-=slice;
                    line++;
                }
            }//a new data line with time num is then added to the data table
            dtmp.name = action;
            dtmp.time = num;
            dtmp.line = line;
            dataTable.push_back(dtmp);
        }
        // adds other data into input while also ignoring unknown words
        else if(action=="NEW"||action=="DISPLAY"||action=="INPUT"||action=="DISK")
        {
            dtmp.name = action;
            dtmp.time = num;
            dtmp.line = line;
            dataTable.push_back(dtmp);
        }
        // Seeding in events for each process being used
        if(action=="NEW")
        {
            if(processTable.size()!=0) //This portion inserts the eline of the previous process
            {
                processTable.back().eline = line-1;
            }
            ptmp.pnum = pronum++;
            ptmp.sline = line;
            ptmp.cline = line;
            ptmp.stime = num;
            ptmp.state = "BLOCKED";
            processTable.push_back(ptmp);
            etmp.pnum = ptmp.pnum;
            etmp.stime = ptmp.stime;
            etmp.ename = "NEW";
            etmp.corenum = -1;
            insertEvent(etmp);

        }
        if(action != "ERROR")
        {
            line++;
        }
    }
    processTable[processTable.size()-1].eline = line-1; //This one is to insert the eline for the last process

}
void insertEvent(event e)
{
    //insertEvent inserts e into the sorted position based on many factors
    //the highest priority is the time the event is going to end
    //if an event ties another event it then checks the event name
    //END events occur first while TERM events occur last
    //if the event names are the same then it sorts in order of process number
    int cnt = 0;
    while(cnt<=eventList.size())
    {
        if(cnt==eventList.size()) //if the cnt exceeds the size that means that it must be the earliest event in the list
        {                         //my vector puts the earliest events in the back so I can just use pop_back
            eventList.push_back(e);
            cnt = eventList.size()+1; //this stops the loop, used later on a lot too
        }
        else if(e.stime>eventList[cnt].stime) //if e comes later than eventList[cnt] then it is inserted in it's sorted spot
        {
            eventList.insert(eventList.begin()+cnt,e);
            cnt = eventList.size()+1;
        }
        else if(e.stime == eventList[cnt].stime) //this is where the tie breakers happen
        {
            if(e.ename == "END") //I make END events occur first in order to release the core and get the next event into the core before this one terminates and prints output
            {
                while(cnt!=eventList.size()&&(eventList[cnt].ename!=e.ename||eventList[cnt].ename==e.ename&&eventList[cnt].stime>e.stime)&&e.stime==eventList[cnt].stime)
                {
                    //the while checks that cnt hasn't exceeded the list size, checks to make sure e is grouped with events of the same kind and in process number order
                    //and it also checks to make sure that the start times are always equal
                    cnt++;
                }
                if(cnt==eventList.size()||e.stime!=eventList[cnt].stime) //if the loop ends where either size is exceeded or start times are no longer the same
                {                                                        //it decrements cnt to allow the earlier if conditions to take care of it
                    cnt--;
                }
                else
                {
                    //otherwise this else means that the event is in the correct spot
                    eventList.insert(eventList.begin()+cnt,e);
                    cnt = eventList.size()+1;
                }
            }
            else if(e.ename == "TERM") //I decided to let terms end as they came
            {
                eventList.insert(eventList.begin()+cnt,e);
                cnt = eventList.size()+1;
            }
            else if((e.pnum>eventList[cnt].pnum||eventList[cnt].ename=="END") && eventList[cnt].ename != "TERM") //this is for the other cases which organizes them in process order 
            {
                eventList.insert(eventList.begin()+cnt,e);
                cnt = eventList.size()+1;
            }   
        }
        cnt++;
    }
}
void print(int n,int c) //When a process is terminated, this is called to display the output in an easy to read format
{
    cout<<endl<<"CURRENT STATE OF THE SYSTEM AT t = "<<globalTime<<" ms";
    cout<<endl<<"Current number of busy cores: "<<c;
    cout<<endl<<"Disk is "<< (diskBusy>globalTime ? "BUSY" : "IDLE");
    cout<<endl<<"READY QUEUE:"<<endl;
    showQueue(coreQueue);
    cout<<endl<<"DISK QUEUE:"<<endl;
    showQueue(diskQueue);
    cout<<endl<<"PROCESS TABLE:"<<endl;
    showProcessTable(n);
}
//returns the position of the first core found to be free, if none are free then returns -1
int getFreeCore(bool bt[])
{
    for(int x = 0; x<nCores;x++)
    {
        if(bt[x]==false)
        {
            return x;
        }
    }
    return -1;
}
//displays the number of busy cores, only used for ease of print();
int numBusyCores(bool bt[])
{
    int cnt = 0;
    for(int x = 0;x<nCores;x++)
    {
        if(bt[x]==true)
        {
            cnt++;
        }
    }
    return cnt;
}
// shows the contents of a queue of integers, used for debugging as well as ease of print()
void showQueue(queue<int> q)
{
    queue<int> tmp = q;
    if(tmp.empty())
    {
        cout<<"empty";
    }
    else
    {
        while(!tmp.empty())
        {
            cout<<"Process "<<tmp.front();
            tmp.pop();
            if(!tmp.empty())
            {
                cout<<", ";
            }
        }
    }
}
//displays the contents of processTable excluding the terminated processes that aren't process n
void showProcessTable(int n)
{
    for(int x = 0; x<processTable.size();x++)
    {
        process p = processTable[x];
        if(p.state!="TERMINATED"||n==x)
        {
            cout<<"Process "<<x<<" started at "<<p.stime<<" ms and is "<<p.state<<endl;
        }
    }
}
//Methods below used for Debugging
/*
void showEventList()
{
    cout<<endl;
    for(int x = 0; x < eventList.size(); x++)
    {
        cout<<eventList[x].pnum<<" "<<eventList[x].ename<<" "<<eventList[x].stime<<endl;
    }
}
void showDataTable()
{
    cout<<endl;
    for(int x = 0; x < dataTable.size(); x++)
    {
        cout<<dataTable[x].line<<" "<<dataTable[x].name<<" "<<dataTable[x].time<<endl;
    }
}
void showProcessTable()
{
    cout<<endl;
    for(int x = 0; x < processTable.size(); x++)
    {
        process p = processTable[x];
        cout<<x<<" "<<p.sline<<" "<<p.cline<<" "<<p.eline<<" "<<p.stime<<" "<<p.state<<endl;
    }   
}
*/