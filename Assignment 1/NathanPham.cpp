#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;
// A struct used to contain my input data
struct data
{
    string name;
    int time,line;
};
//A struct used to contain my data for processTable
struct process
{
    int pnum,sline,eline,cline,stime;
    string state;
};
//A struct used for eventList
struct event
{
    int pnum, stime,corenum;
    string ename;
};
//Methods to be used
void insertEvent(event e);
void print(int n, int c);
int getFreeCore(bool bt[]);
int numBusyCores(bool bt[]);
void showQueue(queue<int> q);
void showProcessTable(int n);
//For debugging purposes
//void showEventList();
//void showDataTable();
//void showProcessTable();
//Global Variables
string action;
int num, slice, nCores;
//line = -2 so we don't count NCORE and SLICE into our data
int line = -2;
int pronum = 0;
int globalTime = 0;
bool diskBusyTime = false;
vector<data> dataTable;
vector<process> processTable;
vector<event> eventList;
queue<int> coreQueue;
queue<int> diskQueue;
//queue<int> tempQueue;
int main()
{
    // This part enters all the data from any input file into the vector dataTable.name
    // It also will split all events of CORE that take up more time than the given slice into
    // separate events inorder to bring it back to queue in round robin fashion
    data dtmp;
    process ptmp;
    event etmp;
    while(cin>>action)
    {
        cin>>num;
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
                while(num>slice)
                {
                    dtmp.name = action;
                    dtmp.time = slice;
                    dtmp.line = line;
                    dataTable.push_back(dtmp);
                    num-=slice;
                    line++;
                }
            }
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
        // Adding new processes immediately as well as creating initial event list
        if(action=="NEW")
        {
            if(processTable.size()!=0)
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
        line++;   
    }
    processTable[processTable.size()-1].eline = line-1;
    bool busyTimes[nCores];
    for(int x = 0; x < nCores; x++)
    {
        busyTimes[x] = false;
    }
    // Start simulation
    event tmp;
    process p;
    event next;
    while(eventList.size()!=0)
    {
        next = eventList.back();
        eventList.pop_back();
        globalTime = next.stime;
        p = processTable[next.pnum];
        tmp.pnum = next.pnum;
        tmp.corenum = next.corenum;
        if(next.ename == "NEW")
        {
            next.ename = "CORE";
            p.cline++;
        }
        if(next.ename == "END")
        {
            if(dataTable[p.cline].name == "CORE")
            {
                p.cline++;
                if(coreQueue.empty())
                {
                    busyTimes[next.corenum] = false;
                }
                else
                {
                    //process number
                    tmp.pnum = coreQueue.front();
                    coreQueue.pop();
                    tmp.stime = globalTime+dataTable[processTable[tmp.pnum].cline].time;
                    tmp.ename = "END";
                    insertEvent(tmp);
                    processTable[tmp.pnum].state = "RUNNING";
                    tmp.pnum = next.pnum; //ensure that the tmp is on the same pnum for current event
                }
                if(p.cline>p.eline)
                {
                    tmp.ename = "TERM";
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                }
                else if(dataTable[p.cline].name == "CORE")
                {
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
                else
                {
                    tmp.ename = dataTable[p.cline].name;
                    tmp.stime = globalTime;
                    insertEvent(tmp);
                    p.state = "BLOCKED";
                }
            }
            else if(dataTable[p.cline].name == "DISK")
            {
                p.cline++;
                if(diskQueue.empty())
                {
                    diskBusyTime = false;
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
            else if(dataTable[p.cline].name == "DISPLAY"||dataTable[p.cline].name == "INPUT")
            {
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
        if(next.ename == "TERM")
        {
            p.state = "TERMINATED";
            processTable[next.pnum] = p;
            print(next.pnum,numBusyCores(busyTimes));
        }
        if(next.ename=="CORE")
        {
            int c = getFreeCore(busyTimes);
            if(c != -1)
            {
                busyTimes[c] = true;
                tmp.ename = "END";
                tmp.stime = globalTime+dataTable[p.cline].time;
                tmp.corenum = c;
                insertEvent(tmp);
                p.state = "RUNNING";
            }
            else
            {
                coreQueue.push(next.pnum);
                p.state = "READY";
            }
        }
        if(next.ename == "DISK")
        {
            if(dataTable[p.cline].time==0)
            {
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
            else if(diskBusyTime == false)
            {
                diskBusyTime = true;
                tmp.ename = "END";
                tmp.stime = globalTime+dataTable[p.cline].time;
                insertEvent(tmp);
            }
            else
            {
                diskQueue.push(next.pnum);
            }
        }
        if(next.ename == "DISPLAY"||next.ename == "INPUT")
        {
            tmp.ename = "END";
            tmp.stime = globalTime+dataTable[p.cline].time;
            insertEvent(tmp);
        }
        processTable[next.pnum] = p;
    }
    return 0;
}
//Inserts events into eventList in the sorted position as it goes
void insertEvent(event e)
{
    int cnt = 0;
    while(cnt<=eventList.size())
    {
        if(cnt==eventList.size())
        {
            eventList.push_back(e);
            cnt = eventList.size()+1;
        }
        else if(e.stime>eventList[cnt].stime)
        {
            eventList.insert(eventList.begin()+cnt,e);
            cnt = eventList.size()+1;
        }
        else if(e.stime == eventList[cnt].stime)
        {
            if(e.ename == "END")
            {

                while(cnt!=eventList.size()&&eventList[cnt].ename!=e.ename&&e.stime==eventList[cnt].stime)
                {
                    cnt++;
                }
                if(cnt==eventList.size())
                {
                    eventList.push_back(e);
                    cnt = eventList.size()+1;
                }
                else if(e.stime==eventList[cnt].stime)
                {
                    eventList.insert(eventList.begin()+cnt,e);
                    cnt = eventList.size()+1;
                }
                else
                {
                    eventList.insert(eventList.begin()+cnt-1,e);
                    cnt = eventList.size()+1;
                }
            }
            else if(e.ename == "TERM")
            {
                eventList.insert(eventList.begin()+cnt,e);
                cnt = eventList.size()+1;
            }
            else if(e.pnum>eventList[cnt].pnum||eventList[cnt].ename=="END")
            {
                eventList.insert(eventList.begin()+cnt,e);
                cnt = eventList.size()+1;
            }   
        }
        cnt++;
    }
}
void print(int n,int c)
{
    cout<<endl<<"CURRENT STATE OF THE SYSTEM AT t = "<<globalTime<<" ms";
    cout<<endl<<"Current number of busy cores: "<<c;
    cout<<endl<<"Disk is "<< (diskBusyTime>globalTime ? "BUSY" : "IDLE");
    cout<<endl<<"READY QUEUE:"<<endl;
    showQueue(coreQueue);
    cout<<endl<<"DISK QUEUE:"<<endl;
    showQueue(diskQueue);
    cout<<endl<<"PROCESS TABLE:"<<endl;
    showProcessTable(n);
}
//Grabs the core with lowest busy time and defaults to the earlier core in case of ties
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