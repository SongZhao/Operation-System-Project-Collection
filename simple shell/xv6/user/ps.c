#include "pstat.h"
#include "param.h"
int main(int argc, char *argv[])
{
	struct pstat p;
	if(getpinfo(&p) < 0)
	{
		printf(1, "getpinfo failed!\n");
		exit();
	}

	int i;
	printf(1, "Slot\tPID\t#chosen\tRuntime\tCharge\n");

	for (i = 0; i< NPROC; i++)
	{
		if(p.inuse[i])
		{
			printf(1, "%d\t%d\t%d\t%d\t%d\n", i, p.pid[i], p.chosen[i], p.time[i], p.charge[i]);
		}
	}
	exit();
}
