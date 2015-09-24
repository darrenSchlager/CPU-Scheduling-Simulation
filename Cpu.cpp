#include <iostream>
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

//cpu scheduling options
typedef struct {
	algorithm alg;
	int slice;			//length of time slice
	int switchTime;		//time it takes to perform a contet switch
} option;

//process stats
typedef struct {
	int waitingTime;
	int turnarountTime;
} processStats;

void readInProcesses(string filename, vector<process> & prs);
void readInOptions(string filename, vector<option> & opts);
void sortProcessesByArrival(const vector<process> & prses, vector<process> & pOut);
void sortProcessesByBurst(const vector<process> & prses, vector<process> & pOut);
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes);
void fcfs(const vector<process> & prses, int & time, vector<processStats> & pStats);

int main(int argc, char *argv[]) 
{
	vector<process> processes, sortedProcesses1, sortedProcesses2;
	vector<option> options;
	readInProcesses("P.dat", processes);
	readInOptions("S.dat", options);
	vector< vector<processStats> > pStats(options.size(), vector<processStats>());
	vector<int> totalTimes(options.size(), 0);
	for(int i=0; i<options.size(); i++)
	{
		switch (options[i].alg) 
		{
			
			case FCFS:
				fcfs(processes, totalTimes[i], pStats[i]);
				break;
			default: 
				break;
		}
		
	}
	printReport(options, pStats, totalTimes);

	
}

/* Function:	readInProcesses
 *    Usage:	vector<process> prses;
				readInProcess("P.dat", prses);
 * -------------------------------------------
 * Saves the data in a formatted file (eg. "P.dat") into a vector of processes (eg. prses).
 * Each line of the file must contain two numbers separated by a space:
 * 		- The first number is the arrival time (in milliseconds),
 *		- The second number is the amount of time the process requires to complete (in milliseconds)
 *		eg. "30 2000"
 */
void readInProcesses(string filename, vector<process> & prses)
{
	ifstream p(filename, fstream::in);
	string line;
	while(p.good())
	{
		getline(p, line);
		/* if a number doesnt come next, error */
		if(!isdigit(line[0])) 
		{
			cerr << "ERROR-- readInProcesses: P.dat is not formatted correctly." << endl;
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
			cerr << "ERROR-- readInProcesses: P.dat is not formatted correctly." << endl;
			exit(EXIT_FAILURE);
		}
		/**/
		/* save the second number */
		int firstDigit = end;
		for(; end<line.length() && isdigit(line[end]); end++) {}
		pr.burst = stoi(line.substr(firstDigit, end-firstDigit+1));
		/**/
		prses.push_back(pr);
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
	ifstream s(filename, fstream::in);
	string line;
	while(s.good())
	{
		getline(s, line);
		/* if a letter doesnt come next, error */
		if(!isalpha(line[0]))
		{
			cerr << "ERROR-- readInOptions: S.dat is not formatted correctly." << endl;
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
			if(prospect==ALGORITHM[i]) opt.alg = (algorithm)i;
		}
		/**/
		/* if a dash comes next, read in the integer pair*/
		if(end<line.length() && line[end] == '-')
		{
			/* if a number doesnt come next, error */
			if(!isdigit(line[++end]))
			{
				cerr << "ERROR-- readInOptions: S.dat is not formatted correctly." << endl;
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
				cerr << "ERROR-- readInOptions: S.dat is not formatted correctly." << endl;
				exit(EXIT_FAILURE);
			}
			/**/
			/* save the second number */
			firstDigit = end;
			for(; end<line.length() && isdigit(line[end]); end++) {}
			opt.switchTime = stoi(line.substr(firstDigit, end-firstDigit+1));
			/**/
		}
		else if(end>=line.length())
		{
			opt.slice = 0;
			opt.switchTime = 0;
		}
		else
		{
			cerr << "ERROR-- readInOptions: S.dat is not formatted correctly." << endl;
			exit(EXIT_FAILURE);
		}
		/**/
		opts.push_back(opt);
	}
	s.close();
}
	
/* Function:	sortProcessesByArrival
 *    Usage:	vector<process> p 
				sortProcessesByArrival(prses, p);
 * -------------------------------------------
 * Returns a vector of proceeses sorted by arrival time from least to greatest
 */
void sortProcessesByArrival(const vector<process> & prses, vector<process> & pOut)
{
	pOut.clear();
	pOut.resize(prses.size());
	pOut[0] = prses[0];
	for(int i=1; i<prses.size(); i++)
	{
		for(int j=i-1; j>=0; j--)
		{
			if(prses[i].arrival>=pOut[j].arrival)
			{
				pOut[j+1] = prses[i];
				break;
			}
			pOut[j+1] = pOut[j];
			if(j==0) pOut[j] = prses[i];
		}
	}
}

/* Function:	sortProcessesByBurst
 *    Usage:	vector<process> p 
				sortProcessesByBurst(prses, p);
 * -------------------------------------------
 * Returns a vector of proceeses sorted by cpu burst time from least to greatest
 */
void sortProcessesByBurst(const vector<process> & prses, vector<process> & pOut)
{
	pOut.clear();
	pOut.resize(prses.size());
	pOut[0] = prses[0];
	for(int i=1; i<prses.size(); i++)
	{
		for(int j=i-1; j>=0; j--)
		{
			if(prses[i].burst>=pOut[j].burst)
			{
				pOut[j+1] = prses[i];
				break;
			}
			pOut[j+1] = pOut[j];
			if(j==0) pOut[j] = prses[i];
		}
	}
}

/* Function:	printReport
 *    Usage:	printReport(opts, pStats, totalTimes)
 * -------------------------------------------
 * Prints out the results of multiple cpu scheduling option simulations.
 * - opts: contains all of the simulated cpu scheduling options
 * - pStats: a 2d vector where each row contains the processStats for each simulated cpu scheduling option
 * - totalTimes: contains the total time for each simulated cpu scheduling option
 */
void printReport(const vector<option> & opts, const vector< vector<processStats> > & pStats, const vector<int> & totalTimes)
{
	for(int i=0; i<opts.size(); i++)
	{
		int total=0;
		for(int j = 0; j<pStats[i].size(); j++)
		{
			total += pStats[i][j].turnarountTime;
		}
		double avgTurnAroundTime = (double)total/pStats[i].size();
		
		total=0;
		for(int j = 0; j<pStats[i].size(); j++)
		{
			total += pStats[i][j].waitingTime;
		}
		double avgWaitingTime = (double)total/pStats[i].size();

		cout << ALGORITHM[opts[i].alg] << " " << avgTurnAroundTime << " " << avgWaitingTime << endl; 
	}
}

/* Function:	fcfs
				int totalTime;
				vecotr<processStats> pStats;
 *    Usage:	fcfs(prses, totalTime, pStats
 * -------------------------------------------
 * Runs a simulation of the FCFS scheduling algorithm. 
 * - prses: contains the processes to schedule and execute
 * - The total time of execution is stored into totalTime, and the timing statistics for each process are stored into pStats.
 */
void fcfs(const vector<process> & prses, int & totalTime, vector<processStats> & pStats)
{
	pStats.clear();
	pStats.resize(0);
	vector<process> p;
	sortProcessesByArrival(prses, p);
	
	totalTime = 0;
	while(p.size()>0)
	{
		if(totalTime<p[0].arrival) totalTime += p[0].arrival-totalTime;
		processStats pS;
		pS.waitingTime = totalTime-p[0].arrival;
		pS.turnarountTime = totalTime - p[0].arrival + p[0].burst;
		pStats.push_back(pS);
		totalTime += p[0].burst;
		p.erase(p.begin());
	}
	
}