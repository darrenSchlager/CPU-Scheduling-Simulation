#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
using namespace std;

#define NUM_ALGORITHMS 7

const string ALGORITHM[NUM_ALGORITHMS] = {"FCFS", "PSJF", "NPSJF", "RR", "RRS", "A", "RRPB"};

enum algorithm {
	FCFS, PSJF, NPSJF, RR, RRS, A, RRPB
};

//process timing
typedef struct {
	int arrival;		//arrival time
	int burst;			//cpu burst length
} process;

//process stats
typedef struct {
	int waiting;
	int turnAround;
	int runCount;
} processStats;

//process block
typedef struct {
	process p;
	processStats s;
} processBlock;

//cpu scheduling options
typedef struct {
	algorithm alg;
	int slice;			//length of time slice
	int switchTime;		//time it takes to perform a contet switch
} option;

void readInProcesses(string filename, vector<process> & prs);
void readInOptions(string filename, vector<option> & opts);
void addProcessByArrival(process & p,  vector<process> & ps);
void addProcessBlockByBurst(processBlock & b, vector<processBlock> & bs);
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes, const vector<int> & idleTimes);
void fcfs(const vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void npsjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void psjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void rr(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats);
void rrs(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats);
void a(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void rrpb(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats);

int main(int argc, char *argv[]) 
{
	vector<process> processes, sortedProcesses1, sortedProcesses2;
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
			case PSJF:
				psjf(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case NPSJF:
				npsjf(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case RR:
				rr(processes, options[i].slice, options[i].switchTime, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case RRS:
				rrs(processes, options[i].slice, options[i].switchTime, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case A:
				a(processes, totalTimes[i], idleTimes[i], pStats[i]);
				break;
			case RRPB:
				rrpb(processes, options[i].slice, options[i].switchTime, totalTimes[i], idleTimes[i], pStats[i]);
			default: 
				break;
		}
		
	}
	printReport(options, pStats, totalTimes, idleTimes);
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
			/* if a number doesnt come next, error */
			if(!isdigit(line[0])) 
			{
				cerr << "ERROR-- readInProcesses: P.dat - Each line MUST contain two numbers separated by a space." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			process pr;
			/* save the first number */
			int end;
			for(end=0; end<line.length() && isdigit(line[end]); end++) {}
			pr.arrival = stoi(line.substr(0,end));
			/**/
			/* if a number doesnt come next, error */
			if(!isdigit(line[++end])) //skip over the space
			{
				cerr << "ERROR-- readInProcesses: P.dat - Each line MUST contain two numbers separated by a space." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			/* save the second number */
			int firstDigit = end;
			for(; end<line.length() && isdigit(line[end]); end++) {}
			pr.burst = stoi(line.substr(firstDigit, end-firstDigit+1));
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
 * 		- The algorithm identifier must be one of the following: FCFS, PSJF, NPSJF, RR
 *		- The integer pair represents the Time Slice (S) and the Context Switching Time (T). (eg. S/T) 
 *		eg. "FCFS" "RR-100/10"
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
			/* if a letter doesnt come next, error */
			if(!isalpha(line[0]))
			{
				cerr << "ERROR-- readInOptions: S.dat - Each line MUST start with a letter." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			
			option opt;
			/* save the option */
			int end;
			for(end=0; end<line.length() && isalpha(line[end]); end++) {}
			string prospect = line.substr(0,end);
			for(int i=0; i<NUM_ALGORITHMS; i++)
			{
				if(prospect==ALGORITHM[i]) 
				{
					opt.alg = (algorithm)i;
					break;
				}
				else if(i==NUM_ALGORITHMS-1)
				{
					cerr << "ERROR-- readInOptions: S.dat - '"<< prospect << "' is not supported." << endl;
					exit(EXIT_FAILURE);
				}
			}
			/**/
			/* if RR, RRS, or RRPB and a dash comes next, read in the integer pair*/
			if((opt.alg==RR || opt.alg==RRS || opt.alg==RRPB) && end<line.length() && line[end] == '-')
			{
				/* if a number doesnt come next, error */
				if(!isdigit(line[++end]))
				{
					cerr << "ERROR-- readInOptions: S.dat - Each dash MUST be followed by a number." << endl;
					exit(EXIT_FAILURE);
				}
				/**/
				/* save the first number */
				int firstDigit = end;
				for(; end<line.length() && isdigit(line[end]); end++) {}
				opt.slice = stoi(line.substr(firstDigit, end-firstDigit+1));
				/**/
				/* if a number doesnt come next, error */
				if(!isdigit(line[++end]))
				{
					cerr << "ERROR-- readInOptions: S.dat - Each slash MUST be followed by a number." << endl;
					exit(EXIT_FAILURE);
				}
				/**/
				/* save the second number */
				firstDigit = end;
				for(; end<line.length() && isdigit(line[end]); end++) {}
				opt.switchTime = stoi(line.substr(firstDigit, end-firstDigit+1));
				/**/
			}
			/* if nothing come next, save the default integer pair*/
			else if(end>=line.length() || !isprint(line[end]))
			{
				opt.slice = 0;
				opt.switchTime = 0;
			}
			/**/
			else
			{
				cerr << "ERROR-- readInOptions: S.dat - Ensure that each line ONLY contains a single entry." << endl;
				exit(EXIT_FAILURE);
			}
			opts.push_back(opt);
		}
	}
	s.close();
}

/* Function:	addProcessByArrival
 *    Usage:	vector<process> p 
				addProcessByArrival(p, ps);
 * -------------------------------------------
 * Adds the process in the apporpriate position in the vector, keeps it sorted by arrival time from least to greatest.
 */
void addProcessByArrival(process & p,  vector<process> & ps)
{
	if(ps.size()==0 || p.arrival >= ps[ps.size()-1].arrival )
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
 *    Usage:	vector<processBlock> p 
				addProcessBlockByBurst(b, bs);
 * -------------------------------------------
 * Adds the processBlock in the apporpriate position in the vector, keeps it sorted by the processescpu burst time from least to greatest.
 */
void addProcessBlockByBurst(processBlock & b, vector<processBlock> & bs)
{
	if(bs.size()==0 || b.p.burst >= bs[bs.size()-1].p.burst )
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
	int w = 13;
	cout << left;
	cout << setw(w) << "" << setw(w) << "Average" << setw(w) << "Average" << setw(w) << "CPU" << endl;
	cout << setw(w) << "" << setw(w) << "Turnaround" << setw(w) << "CPU Waiting" << setw(w) << "Utilization" << endl;
	cout << setw(w) << "Scheduler" << setw(w) << "Time" << setw(w) << "Time" << setw(w) << "%" << endl;
	cout << "==================================================" << endl;
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
		if(opts[i].alg == RR || opts[i].alg==RRS || opts[i].alg==RRPB) scheduler += "-" + to_string(opts[i].slice) + "/" + to_string(opts[i].switchTime);
		cout << setw(w) << scheduler << setw(w) << avgTurnAround << setw(w) << avgWaiting << cpuUtilization << endl; 
	}
}

/* Function:	fcfs
				int totalTime;
				int idleTime
				vector<processStats> pStats;
 *    Usage:	fcfs(ps, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the FCFS scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void fcfs(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	while(p.size()+ready.size()>0)
	{
		if(ready.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					ready.push_back(b);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			for(int i=1; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			ready[0].p.burst--;
			ready[0].s.turnAround++;
			if(ready[0].p.burst == 0)
			{
				pStats.push_back(ready[0].s);
				ready.erase(ready.begin());
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
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void npsjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	processBlock running;
	processBlock *runningPtr = NULL;
	while(p.size()+ready.size()>0 || runningPtr!=NULL)
	{
		if(runningPtr==NULL && ready.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					addProcessBlockByBurst(b, ready);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			if(runningPtr==NULL)
			{
				running = ready[0];
				runningPtr = &running;
				ready.erase(ready.begin());
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			running.p.burst--;
			running.s.turnAround++;
			if(running.p.burst==0)
			{
				pStats.push_back(running.s);
				runningPtr = NULL;
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
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void psjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	while(p.size()+ready.size()>0)
	{
		if(ready.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					addProcessBlockByBurst(b, ready);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			for(int i=1; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			ready[0].p.burst--;
			ready[0].s.turnAround++;
			if(ready[0].p.burst == 0)
			{
				pStats.push_back(ready[0].s);
				ready.erase(ready.begin());
			}
		}		
		totalTime++;
	}
}

/* Function:	rr
				int slice;
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
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void rr(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	int timeRunning = 0;
	bool running = false;
	while(p.size()+ready.size() || running)
	{
		if(!running && ready.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					ready.push_back(b);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			if(!running)
			{
				idleTime += switchTime;
				totalTime += switchTime;
				for(int i=0; i<ready.size(); i++)
				{
					ready[i].s.waiting += switchTime;
					ready[i].s.turnAround += switchTime;
				}
				running = true;
				timeRunning = 0;
			}
			for(int i=1; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			ready[0].p.burst--;
			ready[0].s.turnAround++;
			timeRunning++;
			if(ready[0].p.burst==0)
			{	
				running=false;			
				pStats.push_back(ready[0].s);
				ready.erase(ready.begin());
			}
			else if(timeRunning==slice)
			{
				running = false;
				ready.push_back(ready[0]);
				ready.erase(ready.begin());
			}
		}
		totalTime++;
	}
}

/* Function:	rrs
				int slice;
				int switchTime;
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	rrs(ps, slice, switchTime, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the RRS scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - slice: the time quantum each process receives
 * - switchTime: the time it takes to switch processes
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void rrs(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	vector<processBlock> preempted;
	int timeRunning = 0;
	bool running = false;
	while(p.size()+ready.size()+preempted.size()>0 || running)
	{
		if(!running && ready.size()==0 && preempted.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					ready.push_back(b);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			if(!running)
			{
				if(ready.size()==0) ready.swap(preempted);
				idleTime += switchTime;
				totalTime += switchTime;
				for(int i=0; i<ready.size(); i++)
				{
					ready[i].s.waiting += switchTime;
					ready[i].s.turnAround += switchTime;
				}
				for(int i=0; i<preempted.size(); i++)
				{
					preempted[i].s.waiting += switchTime;
					preempted[i].s.turnAround += switchTime;
				}
				running = true;
				timeRunning = 0;
			}
			for(int i=1; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			for(int i=0; i<preempted.size(); i++)
			{
				preempted[i].s.waiting++;
				preempted[i].s.turnAround++;
			}
			ready[0].p.burst--;
			ready[0].s.turnAround++;
			timeRunning++;
			if(ready[0].p.burst==0)
			{	
				running=false;			
				pStats.push_back(ready[0].s);
				ready.erase(ready.begin());
			}
			else if(timeRunning==slice)
			{
				running = false;
				preempted.push_back(ready[0]);
				ready.erase(ready.begin());
			}
		}
		totalTime++;
	}
}

/* Function:	a
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	a(ps, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the Alternator scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void a(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	processBlock running;
	processBlock *runningPtr = NULL;
	int position = 0;
	const int POSITION_LIMIT = 10;
	int timeRunning = 0;
	const int SLICE = 1000;
	while(p.size()+ready.size()>0 || runningPtr!=NULL)
	{
		if(runningPtr==NULL && ready.size()==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					addProcessBlockByBurst(b, ready);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			if(runningPtr==NULL)
			{
				if(position<9)
				{
					running = ready[0];
					runningPtr = &running;
					ready.erase(ready.begin());
					position = (position+1)%POSITION_LIMIT;
				}
				else if(position<10){
					running = ready[ready.size()-1];
					runningPtr = &running;
					ready.erase(ready.begin() + ready.size()-1);
					position = (position+1)%POSITION_LIMIT;
				}
				timeRunning=0;
			}
			for(int i=0; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			running.p.burst--;
			running.s.turnAround++;
			timeRunning++;
			if(running.p.burst==0)
			{
				pStats.push_back(running.s);
				runningPtr = NULL;
			}
			if(position==9 && timeRunning==SLICE)
			{
				addProcessBlockByBurst(running, ready);
				runningPtr = NULL;
			}
		}
		totalTime++;
	}
}

/* Function:	rrpb
				int slice;
				int switchTime;
				int totalTime;
				int idleTime;
				vector<processStats> pStats;
 *    Usage:	rrpb(ps, slice, switchTime, totalTime, idleTime, pStats);
 * -------------------------------------------
 * Runs a simulation of the RRPB (round robin push-back) scheduling algorithm. 
 * - ps: contains the processes to schedule and execute
 * - slice: the time quantum each process receives
 * - switchTime: the time it takes to switch processes
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void rrpb(vector<process> p, int slice, int switchTime, int & totalTime, int & idleTime, vector<processStats> & pStats)
{
	pStats.clear();
	totalTime = 0;
	idleTime = 0;
	vector<processBlock> ready;
	vector< vector<processBlock> > preempted(10, vector<processBlock>());
	int preemptedSize=0;
	int timeRunning = 0;
	bool running = false;
	while(p.size()+ready.size()+preemptedSize>0 || running)
	{
		if(!running && ready.size()==0 && preemptedSize==0 && totalTime<p[0].arrival)
		{
			idleTime++;
		}
		else
		{
			if(totalTime==p[0].arrival)
			{
				int arrive = p[0].arrival;
				do
				{
					processBlock b;
					b.p = p[0];
					b.s = processStats();
					b.s.runCount=0;
					ready.push_back(b);
					p.erase(p.begin());
				} while(p.size()>0 && p[0].arrival==arrive);
			}
			if(!running)
			{
				if(ready.size()==0)
				{
					for(int i=0; i<preempted.size(); i++)
					{
						if(preempted[i].size()>0)
						{
							ready.swap(preempted[i]);
							preemptedSize -= ready.size();
							break;
						}
					}
					
				}
				idleTime += switchTime;
				totalTime += switchTime;
				for(int i=0; i<ready.size(); i++)
				{
					ready[i].s.waiting += switchTime;
					ready[i].s.turnAround += switchTime;
				}
				for(int i=0; i<preempted.size(); i++)
				{
					for(int j=0; j<preempted[i].size(); j++)
					{
						preempted[i][j].s.waiting += switchTime;
						preempted[i][j].s.turnAround += switchTime;
					}
				}
				running = true;
				timeRunning = 0;
			}
			for(int i=1; i<ready.size(); i++)
			{
				ready[i].s.waiting++;
				ready[i].s.turnAround++;
			}
			for(int i=0; i<preempted.size(); i++)
			{
				for(int j=0; j<preempted[i].size(); j++)
				{
					preempted[i][j].s.waiting += switchTime;
					preempted[i][j].s.turnAround += switchTime;
				}
			}
			ready[0].p.burst--;
			ready[0].s.turnAround++;
			timeRunning++;
			ready[0].s.runCount++;
			if(ready[0].p.burst==0)
			{	
				running=false;			
				pStats.push_back(ready[0].s);
				ready.erase(ready.begin());
			}
			else if(timeRunning==slice)
			{
				running = false;
				if(ready[0].s.runCount<20) preempted[0].push_back(ready[0]);
				else if(ready[0].s.runCount<30) preempted[1].push_back(ready[0]);
				else if(ready[0].s.runCount<40) preempted[2].push_back(ready[0]);
				else if(ready[0].s.runCount<50) preempted[3].push_back(ready[0]);
				else if(ready[0].s.runCount<60) preempted[4].push_back(ready[0]);
				else if(ready[0].s.runCount<70) preempted[5].push_back(ready[0]);
				else if(ready[0].s.runCount<80) preempted[6].push_back(ready[0]);
				else if(ready[0].s.runCount<90) preempted[7].push_back(ready[0]);
				else if(ready[0].s.runCount<100) preempted[8].push_back(ready[0]);
				else if(ready[0].s.runCount>=100) preempted[9].push_back(ready[0]);
				preemptedSize++;
				ready.erase(ready.begin());
			}
		}
		totalTime++;
	}
}