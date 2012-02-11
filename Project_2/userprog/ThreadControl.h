#ifndef THREADCONTROL_H
#define THREADCONTROL_H


class ThreadControlBlock
{
	private:
		int thread;
		int joinSem;
		int StatusLock;
		int stackPageStart;
		bool IsExiting;

	public:
		ThreadControlBlock(int);
		int GetSynchParam();
		void SetStackStart(int startStackPage);
		int GetStackStart();
		void SetExitStatus();
		bool GetExitStatus();
};
#endif
