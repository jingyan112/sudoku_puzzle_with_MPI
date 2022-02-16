#include "sudoku_parallel.h"
#include <mpi.h>

const int MAX_STRING = 100;

// Store the basic infomation to devide the computing workload to multiple processes
struct Params
{
    int workID;     // Process ID
    int comm_sz;    // comm_sz: [3, 10], [11, 82], [83, 730]
    int level;      // level: 1, 2, 3; which is calculated by GetLevel function based on the value of comm_sz
    int taskNum;    // taskNum: 9, 81, 729; which is calculated by GetTaskNum function based on the value of level
    char param[3];  // params: (param[0]), (param[0], param[1]), (param[0], param[1], param[2])
};

// Store the number of the solutions to soduku puzzle calculated by the current process ( process ID: workID)
struct Result
{
    int workID; // Process ID
    int count;  // The number of the solutions calculated by the current process
};

/*
 * Function: GetLevel 
 * --------------------
 * Set three levels for the sudoku solution algorithm based on the setting for the number of MPI processes
 * level = 1, fill the first blank cell with [1, 9]; if comm_sz belongs to [3, 10]
 * level = 2, fill the first two blank cells with [1,9]; if comm_sz belongs to [11, 82]
 * level = 3, fill the first three blank cells with [1,9]; if comm_sz belongs to [83, 730]
 * Note, the limitation for 730 is set in the main function, actually there is a limitation in HPC too, which is less than 730.
 * 
 * int comm_sz: The number of MPI processes
 * 
 * returns: level (1, 2, 3)
*/
int GetLevel(int comm_sz)
{
    if (comm_sz >= 3 && comm_sz <= 10)
    {
        return 1;
    }
    if (comm_sz >= 11 && comm_sz <= 82)
    {
        return 2;
    }
    return 3;
}

/*
 * Function: GetTaskNum 
 * --------------------
 * Calculate the number of the tasks according to the value of variable "level"
 * level = 1, fill the first blank cell with [1, 9], there will be 9 tasks
 * level = 2, fill the first two blank cells with [1,9], there will be 81 tasks
 * level = 3, fill the first three blank cells with [1,9], there will be 729 tasks
 * 
 * int level: calculated by GetLevel function based on the value of comm_sz
 * 
 * returns: the number of tasks
*/
int GetTaskNum(int level)
{
    if (level == 1)
    {
        return 9;
    }
    if (level == 2)
    {
        return 81;
    }
    return 729;
}

/*
 * Function: SudokuSolutionWithParams 
 * --------------------
 * Fill in the first several blank cells (Note: the number of blank cells filled in should equal to level variable.) with workInfo.param,
 * then call SudokuSolution function to calculate the number of solutions
 * 
 * struct Params workInfo: the basic infomation used to devide the computing workload to multiple processes, including workID, comm_sz, level, taskNum, param[3]
 * char *map: sudoku map array
 * 
 * returns: the solutions to the sudoku calculated by the current process
*/
long long SudokuSolutionWithParams(struct Params workInfo, char *map)
{
    if (!SudokuMapCheck(map))
    {
        return 0;
    }
    int setCount = 0;

    // Fill in the first several(workInfo.level) blank cells with workInfo.param
    for (int i = 1; i < 10; i++)
    {
        if (setCount == workInfo.level)
        {
            break;
        }
        for (int j = 1; j < 10; j++)
        {
            if (GetCellValue(i, j, map) == 0)
            {
                SetCellValue(i, j, workInfo.param[setCount], map);
                setCount++;
                if (setCount == workInfo.level)
                {
                    break;
                }
            }
        }
    }
    if (setCount != workInfo.level)
    {
        if (!SudokuMapCheck(map))
        {
            return 0;
        }
        return 1;
    }
    //SudokuPrint(map);
    // Call SudokuSolution function in sudoku_20211125.c to calculate the number of solutions after filling in the first several(workInfo.level) blank cells with workInfo.param
    return SudokuSolution(map);
}

/*
 * Function: slave 
 * --------------------
 * When my_rank != 0, all the processes calculte the possible solutions to sudoku puzzle and work as the senders, send the calculating results to the master process (namely my_rank = 0)
 * 
 * struct Params workInfo: the basic infomation used to devide the computing workload to multiple processes, including workID, comm_sz, level, taskNum, param[3]
 * char *map: sudoku map array
 * 
 * returns: the  number of the solutions to the sudoku puzzle calculated by each slave process seperately
*/
void slave(struct Params workInfo, char map[])
{
    struct Result result;
    result.workID = workInfo.workID;
    result.count = 0;
    int pall = workInfo.comm_sz - 1;
    char curMap[81];
    for (int cur_rank = workInfo.workID - 1; cur_rank < workInfo.taskNum; cur_rank += pall)
    {
        memcpy(curMap, map, 81);
        // The three for loops are used to generate the corresponding tasks (9, 81, 729)
        // When workInfo.level = 1, assign 9 tasks (filling in the first blank cell with [1,9] to all the slave processes evenly), workInfo.param[0] is used to store the value for the first blank cell
        // When workInfo.level = 2, assign 81 tasks (filling in the first two blank cells with [1,9]*[1,9] to all slave processes evenly), workInfo.param[0] and workInfo.param[1] are used to store the values for the first two blank cells
        // When workInfo.level = 3, assign 729 tasks (filling in the first three blank cells with [1,9]*[1,9]*[1,9] to all slave processes evenly), workInfo.param[0], workInfo.param[1] and workInfo.param[2] are used to store the values for the first three blank cells
        for (int i = 0; i < workInfo.level; i++)
        {
            int subNum = cur_rank;
            int dev = 1;
            for (int j = 0; j < i; j++)
            {
                dev = dev * 9;
            }
            workInfo.param[i] = subNum / dev % 9 + 1;
        }
        result.count += SudokuSolutionWithParams(workInfo, curMap);
    }
    printf("workID is %d, the number of solutions is %d!\n", workInfo.workID, result.count);
    MPI_Send((char *)&result, MAX_STRING, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

/*
 * Function: master 
 * --------------------
 * When my_rank = 0, it works as the receiver, waiting for the rest processes' calculating results, and add all of them to get the final number of the solutions to sudoku puzzle.
 * 
 * int comm_sz: the number of all the processes which is set in the PBS file when submitting the job
 * 
 * returns: the total number of the solutions to the sudoku puzzle
*/
long long master(int comm_sz)
{
    char buf[1024];
    long long count = 0;
    // Receive the rest processes' (from 1 to comm_sz-1) calculating results
    for (int i = 1; i < comm_sz; i++)
    {
        MPI_Recv(buf, MAX_STRING, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        struct Result *ret = (struct Result *)buf;
        count += ret->count;
    }
    return count;
}

int main(int argc, char **argv)
{
    char map[81];

    // Checking whether the input for sudoku puzzle is valid
    if (!ParseArgv(argc, argv, map))
    {
        printf("Wrong input for Sudoku puzzle!\n");
        return 0;
    }

    // MPI init
    int comm_sz;
    int my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Restriction for the setting of comm_sz
    if (comm_sz <= 2 || comm_sz >= 730)
    {
        printf("Wrong setting for the number of processes!\n");
        return 0;
    }

    // Set the values for struct Params variable
    struct Params workInfo;
    workInfo.workID = my_rank;
    workInfo.level = GetLevel(comm_sz-1);
    workInfo.comm_sz = comm_sz;
    workInfo.taskNum = GetTaskNum(workInfo.level);

    // Master process, which is used to add the number of solutions to sudoku puzzle from each salve process, and evaluate the time cost for the whole program
    if (my_rank == 0)
    {
       long long start = GetTime();
       int total_num_solutions = master(comm_sz);
       long long end = GetTime();
       printf("The num of processes is %d, the num of solutions is %d, total time is %lld ms.\n", comm_sz - 1, total_num_solutions, end - start);
    }
    // Slave processes, which are used to calculate the number of solutions to sudoku puzzle seperately
    else
    {
        slave(workInfo, map);
    }

    MPI_Finalize();
    return 0;
}