#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
using namespace std;

#define NUM_ALGORITHMS 4

const string ALGORITHM[NUM_ALGORITHMS] = {"FCFS", "PSJF", "NPSJF", "RR"};

enum algorithm {
	FCFS, PSJF, NPSJF, RR
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
void addProcessByBurst(process & p,  vector<process> & ps);
void addProcessBlockByBurst(processBlock & b, vector<processBlock> & bs);
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes, const vector<int> & idleTimes);
void fcfs(const vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void npsjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);
void psjf(vector<process> p, int & totalTime, int & idleTime, vector<processStats> & pStats);

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
		if(line.length()>0)
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
		if(line.length()>0)
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
			/* if a dash comes next, read in the integer pair*/
			if(end<line.length() && line[end] == '-')
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
		int i = ps.size()-1;
		ps.resize(ps.size()+1);
		do
		{
			ps[i+1] = ps[i];
			i--;
		} while(i>=0 && p.arrival<ps[i].arrival);
		ps[i+1] = p;
	}
}

/* Function:	addProcessByBurst
 *    Usage:	vector<process> p 
				addProcessByBurst(p, ps);
 * -------------------------------------------
 * Adds the process in the apporpriate position in the vector, keeps it sorted by cpu burst time from least to greatest.
 */
void addProcessByBurst(process & p,  vector<process> & ps)
{
	
	if(ps.size()==0 || p.burst >= ps[ps.size()-1].burst )
		ps.push_back(p);
	else 
	{
		int i = ps.size()-1;
		ps.resize(ps.size()+1);
		do
		{
			ps[i+1] = ps[i];
			i--;
		} while(i>=0 && p.burst<ps[i].burst);
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
		int i = bs.size()-1;
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
		if(opts[i].alg == RR) scheduler += "-" + to_string(opts[i].slice) + "/" + to_string(opts[i].switchTime);
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
	while(p.size()>0)
	{
		if(totalTime<p[0].arrival)
		{
			idleTime += p[0].arrival-totalTime;
			totalTime += p[0].arrival-totalTime;
		}
		processStats pS;
		pS.waiting = totalTime-p[0].arrival;
		pS.turnAround = totalTime - p[0].arrival + p[0].burst;
		pStats.push_back(pS);
		totalTime += p[0].burst;
		p.erase(p.begin());

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