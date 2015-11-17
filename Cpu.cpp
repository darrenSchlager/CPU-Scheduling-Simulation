#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <cctype>
using namespace std;

#define NUM_ALGORITHMS 5

const string ALGORITHM[NUM_ALGORITHMS] = {"FCFS", "NPSJF", "PSJF", "RR", "RRP"};

enum algorithm {
	FCFS, NPSJF, PSJF, RR, RRP
};

typedef struct {
	int arrival;		//arrival time
	int burst;			//cpu burst length
} process;

typedef struct {
	int waiting;
	int turnAround;
} processStats;

//process block
//		-- useage: groups a process together with its recorded stats
typedef struct {
	process p;
	processStats s;
} processBlock;

//cpu scheduling options
typedef struct {
	algorithm alg;
	int slice;			//length of time slice
	int prioritySlice;  //lengh of priority time slice (RRP only)
	int switchTime;		//time it takes to perform a contet switch
} option;

void fcfs(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats);
void npsjf(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats);
void psjf(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats);
void rr(vector<process> ps, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats);
void rrp(vector<process> ps, int slice, int prioritySlice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats);
void addProcessByArrival(process & p,  vector<process> & ps);
void addProcessBlockByBurst(processBlock & b, deque<processBlock> & bs);
void addNewArrivals(vector<process> & ps, deque<processBlock> & ready);
void addNewArrivalsInOrder(vector<process> & ps, deque<processBlock> & ready);
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes, const vector<int> & idleTimes);
void readInProcesses(string filename, vector<process> & ps);
void readInOptions(string filename, vector<option> & opts);

int main(int argc, char *argv[]) 
{
	vector<process> processes;
	vector<option> options;
	readInProcesses("P.dat", processes);
	readInOptions("S.dat", options);
	vector< vector<processStats> > pStats(options.size(), vector<processStats>());
	vector<int> totalTimes(options.size(), 0);
	vector<int> idleTimes(options.size(), 0);
	for(int i=0; i<options.size(); i++)
	{
		switch (options[i].alg) 
		{
			case FCFS:
				fcfs(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case NPSJF:
				npsjf(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case PSJF:
				psjf(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case RR:
				rr(processes, options[i].slice, options[i].switchTime, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case RRP:
				rrp(processes, options[i].slice, options[i].prioritySlice, options[i].switchTime, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			default: 
				break;
		}
		
	}
	printReport(options, pStats, totalTimes, idleTimes);
}

/* Function:	fcfs
				int totalTime;
				int idleTime
				vector<processStats> pStats;
 *    Usage:	fcfs(ps, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the FCFS scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, 
 *	 the total time the cpu is idle is stored into idleTime,
 *	 and the timing statistics for each process are stored into pStats.
 */
void fcfs(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	deque<processBlock> ready;
	processBlock cpu;
	bool running = false;
	while(running || ps.size()+ready.size()>0)
	{
		if(!running && ready.size()==0 && totalTime<ps[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(ps.size()>0 && totalTime==ps[0].arrival)
			{
				addNewArrivals(ps, ready);
			}
			if(!running)
			{
				cpu = ready.front();
				ready.pop_front();
				running = true;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			cpu.p.burst--;
			cpu.s.turnAround++;
			if(cpu.p.burst == 0)
			{
				pStats.push_back(cpu.s);
				running = false;
			}
		}		
		totalTime++;
	}
}

/* Function:	npsjf
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	npsjf(ps, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the NPSJF scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, 
 *	 the total time the cpu is idle is stored into idleTime,
 *	 and the timing statistics for each process are stored into pStats.
 */
void npsjf(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	deque<processBlock> ready;
	processBlock cpu;
	bool running = false;
	while(running || ps.size()+ready.size()>0)
	{
		if(!running && ready.size()==0 && totalTime<ps[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(ps.size()>0 && totalTime==ps[0].arrival)
			{
				addNewArrivalsInOrder(ps, ready);
			}
			if(!running)
			{
				cpu = ready.front();
				ready.pop_front();
				running = true;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			cpu.p.burst--;
			cpu.s.turnAround++;
			if(cpu.p.burst==0)
			{
				pStats.push_back(cpu.s);
				running = false;
			}
		}
		totalTime++;
	}
}

/* Function:	psjf
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	psjf(ps, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the PSJF scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, 
 *	 the total time the cpu is idle is stored into idleTime,
 *	 and the timing statistics for each process are stored into pStats.
 */
void psjf(vector<process> ps, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	deque<processBlock> ready;
	processBlock cpu;
	bool running = false;
	while(running || ps.size()+ready.size()>0)
	{
		if(!running && ready.size()==0 && totalTime<ps[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(ps.size()>0 && totalTime==ps[0].arrival)
			{
				addNewArrivalsInOrder(ps, ready);
				if(running && cpu.p.burst>ready[0].p.burst)
				{
					addProcessBlockByBurst(cpu, ready);
					cpu = ready.front();
					ready.pop_front();
				}
			}
			if(!running)
			{
				cpu = ready.front();
				ready.pop_front();
				running = true;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			cpu.p.burst--;
			cpu.s.turnAround++;
			if(cpu.p.burst == 0)
			{
				pStats.push_back(cpu.s);
				running = false;
			}
		}		
		totalTime++;
	}
}

/* Function:	rr
				int switchTime;
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	rr(ps, slice, switchTime, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the RR scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - slice: the time quantum each process receives
 * - switchTime: the time it takes to switch processes
 * - The total time of execution is stored into totalTime, 
 *	 the total time the cpu is idle is stored into idleTime,
 *	 and the timing statistics for each process are stored into pStats.
 */
void rr(vector<process> ps, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	deque<processBlock> ready;
	int timeRunning = 0;
	processBlock cpu;
	bool running = false;
	while(running || ps.size()+ready.size()>0)
	{
		if(!running && ready.size()==0 && totalTime<ps[0].arrival)
		{
			idleTime++;
			totalTime++;
		}
		else
		{
			if(ps.size()>0 && totalTime==ps[0].arrival)
			{
				addNewArrivals(ps, ready);
			}
			if(!running)
			{
				for(int i=0; i<switchTime; i++)
				{
					for(int i=0; i<ready.size(); i++)
					{
						ready[i].s.waiting++;
						ready[i].s.turnAround++;
					}
					idleTime++;
					totalTime++;
					if(ps.size()>0 && totalTime==ps[0].arrival)
					{
						addNewArrivals(ps, ready);
					}
				}
				cpu = ready.front();
				ready.pop_front();
				running = true;
				timeRunning = 0;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			cpu.p.burst--;
			cpu.s.turnAround++;
			timeRunning++;
			totalTime++;
			if(cpu.p.burst==0)
			{	
				pStats.push_back(cpu.s);
				running=false;
			}
			else if(timeRunning==slice)
			{
				if(ps.size()>0 && totalTime==ps[0].arrival)
				{
					addNewArrivals(ps, ready);
				}
				ready.push_back(cpu);
				running = false;
			}
		}
	}
}

/* Function:	rrp
				int switchTime;
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	rrp(ps, slice, int prioritySlice, switchTime, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the RRP (round robin priority) scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - slice: the time quantum each process receives
 * - prioritySlice: if a process has burst<prioritySlice, it runs to completion
 * - switchTime: the time it takes to switch processes
 * - The total time of execution is stored into totalTime, 
 *	 the total time the cpu is idle is stored into idleTime,
 *	 and the timing statistics for each process are stored into pStats.
 */
void rrp(vector<process> ps, int slice, int prioritySlice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	deque<processBlock> ready;
	int timeRunning = 0;
	processBlock cpu;
	bool running = false;
	int currentSlice = slice;
	while(running || ps.size()+ready.size()>0)
	{
		if(!running && ready.size()==0 && totalTime<ps[0].arrival)
		{
			idleTime++;
			totalTime++;
		}
		else
		{
			if(ps.size()>0 && totalTime==ps[0].arrival)
			{
				addNewArrivals(ps, ready);
			}
			if(!running)
			{
				for(int i=0; i<switchTime; i++)
				{
					for(int i=0; i<ready.size(); i++)
					{
						ready[i].s.waiting++;
						ready[i].s.turnAround++;
					}
					idleTime++;
					totalTime++;
					if(ps.size()>0 && totalTime==ps[0].arrival)
					{
						addNewArrivals(ps, ready);
					}
				}
				cpu = ready.front();
				ready.pop_front();
				running = true;
				timeRunning = 0;
				if(cpu.p.burst <= prioritySlice) currentSlice=cpu.p.burst;
				else currentSlice = slice;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			cpu.p.burst--;
			cpu.s.turnAround++;
			timeRunning++;
			totalTime++;
			if(cpu.p.burst==0)
			{	
				pStats.push_back(cpu.s);
				running=false;
			}
			else if(timeRunning==currentSlice)
			{
				if(ps.size()>0 && totalTime==ps[0].arrival)
				{
					addNewArrivals(ps, ready);
				}
				ready.push_back(cpu);
				running = false;
			}
		}
	}
}

/* Function:	addProcessByArrival
 *    Usage:	vector<process> ps 
				addProcessByArrival(p, ps);
 * -------------------------------------------
 * Adds the process in the apporpriate position in the vector, keeps it sorted by arrival time from least to greatest.
 */
void addProcessByArrival(process & p,  vector<process> & ps)
{
	if(ps.size()==0 || p.arrival >= ps.back().arrival )
		ps.push_back(p);
	else 
	{
		int i = (int)ps.size()-1;
		ps.resize(ps.size()+1);
		do
		{
			ps[i+1] = ps[i];
			i--;
		} while(i>=0 && p.arrival<ps[i].arrival);
		ps[i+1] = p;
	}
}

/* Function:	addProcessBlockByBurst
 *    Usage:	deque<processBlock> p 
				addProcessBlockByBurst(b, bs);
 * -------------------------------------------
 * Adds the processBlock in the apporpriate position in the vector, keeps it sorted by the processescpu burst time from least to greatest.
 */
void addProcessBlockByBurst(processBlock & b, deque<processBlock> & bs)
{
	if(bs.size()==0 || b.p.burst >= bs.back().p.burst )
		bs.push_back(b);
	else 
	{
		int i = (int)bs.size()-1;
		bs.resize(bs.size()+1);
		do
		{
			bs[i+1] = bs[i];
			i--;
		} while(i>=0 && b.p.burst<bs[i].p.burst);
		bs[i+1] = b;
	}
}

/* Function:	addNewArrivals
 *    Usage:	deque<processBlock> bs
 addNewArrivals(ps, ready);
 * -------------------------------------------
 * For all processes in 'ps' that have the same arrival time as 'ps[0]', a new 'processBlock' is created, initialized,
 * and inserted at the end of 'ready'.
 */
void addNewArrivals(vector<process> & ps, deque<processBlock> & ready)
{
	int arrive = ps[0].arrival;
	do
	{
		processBlock b;
		b.p = ps[0];
		b.s = processStats();
		ready.push_back(b);
		ps.erase(ps.begin());
	} while(ps.size()>0 && ps[0].arrival==arrive);
}

/* Function:	addNewArrivalsInOrder
 *    Usage:	deque<processBlock> bs
 addNewArrivalsInOrder(ps, ready);
 * -------------------------------------------
 * For all processes in 'ps' that have the same arrival time as 'ps[0]', a new 'processBlock' is created, initialized,
 * and inserted at the correct position in 'ready'.
 */
void addNewArrivalsInOrder(vector<process> & ps, deque<processBlock> & ready)
{
	int arrive = ps[0].arrival;
	do
	{
		processBlock b;
		b.p = ps[0];
		b.s = processStats();
		addProcessBlockByBurst(b, ready);
		ps.erase(ps.begin());
	} while(ps.size()>0 && ps[0].arrival==arrive);
}

/* Function:	printReport
 *    Usage:	printReport(opts, pStats, totalTimes, idleTimes);
 * -------------------------------------------
 * Prints out the results of multiple cpu scheduling option simulations.
 * - opts: contains all of the simulated cpu scheduling options
 * - pStats: a 2d vector where each row contains the processStats for each simulated cpu scheduling option
 * - totalTimes: contains the total running time for each simulated cpu scheduling option
 * - idleTimes: contains the cpu idle time for each simulated cpu scheduling option
 */
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes, const vector<int> & idleTimes)
{
	stringstream ss;
	int w = 13;
	int ww = 13;
	bool toLong = false;
	do 
	{
		ss.str("");
		toLong = false;
		
		ss << left;
		ss << setw(ww) << "" << setw(w) << "Average" << setw(w) << "Average" << setw(w) << "CPU" << endl;
		ss << setw(ww) << "" << setw(w) << "Turnaround" << setw(w) << "CPU Waiting" << setw(w) << "Utilization" << endl;
		ss << setw(ww) << "Scheduler" << setw(w) << "Time" << setw(w) << "Time" << setw(w) << "%" << endl;
		for(int i=0; i<ww+w+w+w-2; i++) ss << "=";
		ss << endl;
		for(int i=0; i<opts.size(); i++)
		{
			int totalTurnAround=0;
			int totalWaiting=0;
			for(int j = 0; j<pStats[i].size(); j++)
			{
				totalTurnAround += pStats[i][j].turnAround;
				totalWaiting += pStats[i][j].waiting;
			}
			double avgTurnAround = (double)totalTurnAround/pStats[i].size();
			double avgWaiting = (double)totalWaiting/pStats[i].size();
			double cpuUtilization = ( (totalTimes[i]-idleTimes[i])/(double)totalTimes[i] ) * 100;

			string scheduler = ALGORITHM[opts[i].alg];
			if(opts[i].alg==RR) scheduler += "-" + to_string(opts[i].slice) + "/" + to_string(opts[i].switchTime);
			if(opts[i].alg==RRP) scheduler += "-" + to_string(opts[i].slice) + "/" + to_string(opts[i].prioritySlice) + "/" + to_string(opts[i].switchTime);
			if(scheduler.length()+2>ww)
			{
				toLong=true;
				break;
			}
			ss << fixed << setprecision(2) << setw(ww) << scheduler << setw(w) << avgTurnAround << setw(w) << avgWaiting << cpuUtilization << endl; 
		}
		ww++;
	} while(toLong);
	
	string output;
	while(!ss.eof())
	{
		string temp;
		getline(ss, temp);
		output += (temp+"\n");
	}
	cout << output;
}

/* Function:	readInProcesses
 *    Usage:	vector<process> ps;
				readInProcess("P.dat", ps);
 * -------------------------------------------
 * Saves the data in a formatted file (eg. "P.dat") into a vector of processes (eg. ps).
 * Each line of the file must contain two numbers separated by a space:
 * 		- The first number is the arrival time (in milliseconds),
 *		- The second number is the amount of time the process requires to complete (in milliseconds)
 *		eg. "30 2000"
 */
void readInProcesses(string filename, vector<process> & ps)
{
	ps.clear();
	ifstream p(filename, fstream::in);
	string line;
	while(p.good())
	{
		getline(p, line);
		if(line.length()>0 && isprint(line[0]))
		{
			int end = 0;
			while(isblank(line[end])) end++;
			/* if a number doesnt come next, error */
			if(!isdigit(line[end])) 
			{
				cerr << "ERROR-- readInProcesses: " << filename << " '"<< line << "' - Each line MUST contain two positive numbers separated by a single space." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			process pr;
			/* save the arrival time */
			for(; end<line.length() && isdigit(line[end]); end++) {}
			pr.arrival = stoi(line.substr(0,end));
			/**/
			while(isblank(line[end])) end++;
			/* if a number doesnt come next, error */
			if(!isdigit(line[end]))
			{
				cerr << "ERROR-- readInProcesses: " << filename << " '"<< line << "' - Each line MUST contain two positive numbers separated by a single space." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			/* save the burst time */
			int firstDigit = end;
			for(; end<line.length() && isdigit(line[end]); end++) {}
			pr.burst = stoi(line.substr(firstDigit, end-firstDigit+1));
			
			if(pr.burst==0) //error if the burst is 0
			{
				cerr << "ERROR-- readInProcesses: " << filename << " '"<< line << "' - Ensure that all process burst times are > 0." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			while(end<line.length() && isblank(line[end])) end++;
			/* if a something comes next, error */
			if(end<line.length() && isprint(line[end]))
			{
				cerr << "ERROR-- readInProcesses: " << filename << " '"<< line << "' - Each line MUST only contain two numbers separated by a single space. No lagging spaces." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			addProcessByArrival(pr, ps);
		}
	}
	p.close();
}

/* Function:	readInOptions
 *    Usage:	vector<option> opts;
				readInOptions("S.dat", opts);
 * -------------------------------------------
 * Saves the data from a properly formatted file (eg. "S.dat") into a vector of cpu scheduling options (eg. opts).
 * Each line of the file must contain the algorithm identifier, followed by an optional integer pair lead with a dash:
 * 		- The algorithm identifier must be one of the following: FCFS, PSJF, NPSJF, RR, RRP
 *		- If RR, the integer pair represents the Time Slice (S) and the Context Switching Time (T). (eg. S/T)
 *		- If RRP, the integer pair represents the Time Slice (S), the Priority Time Slice (PS) 
		  and the Context Switching Time (T). (eg. S/PS/T) 	 
 *		eg. "FCFS" "RR-100/10" "RRP-100/1000/10"
 */
void readInOptions(string filename, vector<option> & opts)
{
	opts.clear();
	ifstream s(filename, fstream::in);
	string line;
	while(s.good())
	{
		getline(s, line);
		if(line.length()>0 && isprint(line[0]))
		{
			int end = 0;
			while(isblank(line[end])) end++;
			/* if a letter doesnt come next, error */
			if(!isalpha(line[end]))
			{
				cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - Each line MUST start with a letter." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			
			option opt;
			/* save the option */
			int start = end;
			for(; end<line.length() && isalpha(line[end]); end++) {}
			string prospect = line.substr(start,end-start);
			for(int i=0; i<NUM_ALGORITHMS; i++)
			{
				if(prospect==ALGORITHM[i]) 
				{
					opt.alg = (algorithm)i;
					break;
				}
				else if(i==NUM_ALGORITHMS-1)
				{
					cerr << "ERROR-- readInOptions: " << filename << " - '"<< prospect << "' is not supported." << endl;
					exit(EXIT_FAILURE);
				}
			}
			/**/
			while(isblank(line[end])) end++;
			/* if RR or RRP and a dash comes next, read in the integer pair*/
			if((opt.alg==RR || opt.alg==RRP) && end<line.length() && line[end] == '-')
			{
				end++;
				while(isblank(line[end])) end++;
				/* if a number doesnt come next, error */
				if(!isdigit(line[end]))
				{
					cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - Each dash MUST be followed by a number." << endl;
					exit(EXIT_FAILURE);
				}
				/**/
				/* save the slice  */
				int firstDigit = end;
				for(; end<line.length() && isdigit(line[end]); end++) {}
				opt.slice = stoi(line.substr(firstDigit, end-firstDigit+1));
				if(opt.slice==0) // error if the slice is 0
				{
					cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - Ensure that all RR and RRP time slices are > 0." << endl;
					exit(EXIT_FAILURE);
				}
				/**/
				while(isblank(line[end])) end++;
				/* if a slash and a number dont come next, error */
				int slashLoc = end++;
				while(isblank(line[end])) end++;
				if(line[slashLoc]!='/' || !isdigit(line[end]))
				{
					if(opt.alg==RR) cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - For RR, there MUST be two numbers each separated by a slash ( TimeSlice/ContextSwitchTime )" << endl;
					else if(opt.alg==RRP) cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - For RRP, there MUST be three numbers each separated by a slash ( TimeSlice/PriorityTimeSlice/ContextSwitchTime )" << endl;
					exit(EXIT_FAILURE);
				}
				if(opt.alg==RR)
				{
					/**/
					/* save the switch time */
					firstDigit = end;
					for(; end<line.length() && isdigit(line[end]); end++) {}
					opt.switchTime = stoi(line.substr(firstDigit, end-firstDigit+1));
					/**/
					while(end<line.length() && isblank(line[end])) end++;
					/* if a something comes next, error */
					if(end<line.length() && isprint(line[end]))
					{
						cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - For RR, there MUST only be two numbers each separated by a slash ( TimeSlice/ContextSwitchTime )" << endl;
						exit(EXIT_FAILURE);
					}
					/**/
				}
				else if(opt.alg==RRP)
				{
					/**/
					/* save the priority slice */
					firstDigit = end;
					for(; end<line.length() && isdigit(line[end]); end++) {}
					opt.prioritySlice = stoi(line.substr(firstDigit, end-firstDigit+1));
					/**/
					while(isblank(line[end])) end++;
					/* if a slash and a number dont come next, error */
					slashLoc = end++;
					while(isblank(line[end])) end++;
					if(line[slashLoc]!='/' || !isdigit(line[end]))
					{
						cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - For RRP, there MUST be three numbers each separated by a slash ( TimeSlice/PriorityTimeSlice/ContextSwitchTime )" << endl;
						exit(EXIT_FAILURE);
					}
					/**/
					/* save the switch time */
					firstDigit = end;
					for(; end<line.length() && isdigit(line[end]); end++) {}
					opt.switchTime = stoi(line.substr(firstDigit, end-firstDigit+1));
					/**/
					while(end<line.length() && isblank(line[end])) end++;
					/* if a something comes next, error */
					if(end<line.length() && isprint(line[end]))
					{
						cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - For RRP, there MUST only be three numbers each separated by a slash ( TimeSlice/PriorityTimeSlice/ContextSwitchTime )" << endl;
						exit(EXIT_FAILURE);
					}
					/**/
				}
			}
			/* if nothing come next, save the default integer pair*/
			else if(opt.alg!=RR && opt.alg!=RRP && (end>=line.length() || !isprint(line[end])))
			{
				opt.slice = 0;
				opt.switchTime = 0;
			}
			/**/
			else
			{
				if(line[end]=='-') cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - Only RR and RRP support '-' modifiers." << endl;
				else if(opt.alg==RR || opt.alg==RRP) cerr << "ERROR-- readInOptions: " << filename << " '"<< line << "' - RR and RRP must be followed by a dash and the appropriate modifiers (eg. RR-TimeSlice/ContextSwitchTime , RRP-TimeSlice/PriorityTimeSlice/ContextSwitchTime)." << endl;
				else cerr << "ERROR-- readInOptions: S.dat '"<< line << "' - Ensure that each line ONLY contains a single valid entry." << endl;
				exit(EXIT_FAILURE);
			}
			opts.push_back(opt);
		}
	}
	s.close();
}