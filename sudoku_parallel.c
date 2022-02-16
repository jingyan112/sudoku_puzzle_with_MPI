#include "sudoku_parallel.h"

/*
 *
 * map[81]: Store the values of 81 cells in the sudoku map
 * ------------------------- ------------------------- -------------------------
 * | map[0] map[1] map[2]    | map[3] map[4] map[5]    | map[6] map[7] map[8]    | 
 * | map[9] map[10] map[11]  | map[12] map[13] map[14] | map[15] map[16] map[17] | 
 * | map[18] map[19] map[20] | map[21] map[22] map[23] | map[24] map[25] map[26] | 
 * ------------------------- ------------------------- -------------------------
 * | map[27] map[28] map[29] | map[30] map[31] map[32] | map[33] map[34] map[35] | 
 * | map[36] map[37] map[38] | map[39] map[40] map[41] | map[42] map[43] map[44] | 
 * | map[45] map[46] map[47] | map[48] map[49] map[50] | map[51] map[52] map[53] | 
 * ------------------------- ------------------------- -------------------------
 * | map[54] map[55] map[56] | map[57] map[58] map[59] | map[60] map[61] map[62] | 
 * | map[63] map[64] map[65] | map[66] map[67] map[68] | map[69] map[70] map[71] | 
 * | map[72] map[73] map[74] | map[75] map[76] map[77] | map[78] map[79] map[80] | 
 * ------------------------- ------------------------- -------------------------
 * 
 * (x, y): Coordinate of each cell in sudoku map
 *  ------------------- ------------------- -------------------
 * | (1,1) (1,2) (1,3) | (1,4) (1,5) (1,6) | (1,7) (1,8) (1,9) | 
 * | (2,1) (2,2) (2,3) | (2,4) (2,5) (2,6) | (2,7) (2,8) (2,9) | 
 * | (3,1) (3,2) (3,3) | (3,4) (3,5) (3,6) | (3,7) (3,8) (3,9) | 
 *  ------------------- ------------------- -------------------
 * | (4,1) (4,2) (4,3) | (4,4) (4,5) (4,6) | (4,7) (4,8) (4,9) | 
 * | (5,1) (5,2) (5,3) | (5,4) (5,5) (5,6) | (5,7) (5,8) (5,9) | 
 * | (6,1) (6,2) (6,3) | (6,4) (6,5) (6,6) | (6,7) (6,8) (6,9) | 
 *  ------------------- ------------------- -------------------
 * | (7,1) (7,2) (7,3) | (7,4) (7,5) (7,6) | (7,7) (7,8) (7,9) | 
 * | (8,1) (8,2) (8,3) | (8,4) (8,5) (8,6) | (8,7) (8,8) (8,9) | 
 * | (9,1) (9,2) (9,3) | (9,4) (9,5) (9,6) | (9,7) (9,8) (9,9) | 
 *  ------------------- ------------------- -------------------
 * 
 * (i, j): Divide the sudoku map into 9 small boxes
 *  -------- -------- --------
 * | (0, 0) | (0, 1) | (0, 2) |
 *  -------- -------- --------
 * | (1, 0) | (1, 1) | (1, 2) |
 *  -------- -------- --------
 * | (2, 0) | (2, 1) | (2, 2) |
 *  -------- -------- --------
*/

/*
 * Function: SetCellValue
 * --------------------
 * Set value for the cell (x,y) in sudoku map
 * 
 * x, y: coordinate of cell (x, y)
 * value: Set the value of cell (x, y) as value
 * map[]: sudoku map array
*/
void SetCellValue(int x, int y, char value, char map[])
{
    int index = (x - 1) * 9 + y - 1;
    map[index] = value;
}

/*
 * Function: GetCellValue
 * --------------------
 * Get value from the cell (x,y) in sudoku map
 * 
 * x, y: coordinate of cell (x, y)
 * map[]: sudoku map array
 * 
 * returns: the value of cell (x, y)
*/
char GetCellValue(int x, int y, char map[])
{
    int index = (x - 1) * 9 + y - 1;
    return map[index];
}

/*
 * Function: RowColumnCheckDup 
 * --------------------
 * Check whether there are duplicated values in row x or column y in sudoku map
 *
 * x, y: coordinate of cell (x, y)
 * value: the value of cell (x, y)
 * map[]: sudoku map array
 *
 * returns: return 0 if there are duplicated values in row x or column y in sudoku map, which means the sudoku is invalid. Otherwise, return 1
*/
int RowColumnCheckDup(int x, int y, char value, char map[])
{
    int i = 0;

    // If there are at least 2 cells in the column y with same value, return 0
    for (i = 1; i <= 9; i++)
    {
        if (GetCellValue(i, y, map) == value)
        {
            if (x != i)
                return 0;
        }
    }

    // If there are at least 2 cells in the row x with same value, return 0
    for (i = 1; i <= 9; i++)
    {
        if (GetCellValue(x, i, map) == value)
        {
            if (i != y)
                return 0;
        }
    }
    return 1;
}

/*
 * Function: BoxCheckDup 
 * --------------------
 * Check whether there are duplicated values in each small box (1 sudoku has 9 small boxes)
 *
 * x, y: coordinate of cell (x, y)
 * value: the value of cell (x, y)
 * map[]: sudoku map array
 *
 * returns: return 0 if there are duplicated values in the small box that (x, y) is located which means the sudoku is invalid. Otherwise, return 1
*/
int BoxCheckDup(int x, int y, int value, char map[])
{
    int i = (x - 1) / 3;
    int j = (y - 1) / 3;
    for (int a = i * 3 + 1; a < i * 3 + 3 + 1; a++)
    {
        for (int b = j * 3 + 1; b < j * 3 + 3 + 1; b++)
        {
            if (GetCellValue(a, b, map) == value)
            {
                if (x != a && b != y)
                    return 0;
            }
        }
    }
    return 1;
}

/*
 * Function: IsValid 
 * --------------------
 * AND operation for the results of BoxCheckDup function and RowColumnCheckDup function
 *
 * x, y: coordinate of cell (x, y)
 * value: the value of cell (x, y)
 * map[]: sudoku map array
 *
 * returns: return 0 if there are duplicated values in the small box that (x, y) is located, or there are duplicated values in row x or column y in sudoku map, which means the sudoku is invalid. Otherwise, return 1.
*/
int IsValid(int x, int y, int value, char map[])
{
    return BoxCheckDup(x, y, value, map) && RowColumnCheckDup(x, y, value, map);
}

/*
 * Function: SudokuMapCheck 
 * --------------------
 * Check the result of AND operation of the results of BoxCheckDup function and RowColumnCheckDup function, namely IsValid function above for each cell in sudoku map
 *
 * map[]: sudoku map array
 *
 * returns: return 0 if there are duplicated values in each small box, or there are duplicated values in each row or each column in sudoku map, which means the sudoku is invalid. Otherwise, return 1.
*/
int SudokuMapCheck(char map[])
{
    int i, j;
    i = 0;
    j = 0;
    for (i = 1; i < 10; i++)
    {
        for (j = 1; j < 10; j++)
        {
            int value = GetCellValue(i, j, map);
            // In ParseArgv function, the cells without inputting values from the user will be filled in 0.
            // In this function, only the cells with inputting values from the user need to be checked
            if (value != 0)
            {
                if (!IsValid(i, j, value, map))
                    return 0;
            }
        }
    }
    return 1;
}

/*
 * Function: SudokuPrint 
 * --------------------
 * Print the solution of sudoku map with some format
 *
 * map[]: sudoku map array
*/
void SudokuPrint(char map[])
{
    int i, j;
    i = j = 0;
    for (j = 1; j <= 9; j++)
    {
        printf("---");
        if (j % 3 == 0)
        {
            printf("|");
        }
    }
    printf("\n");
    for (i = 1; i <= 9; i++)
    {
        for (j = 1; j <= 9; j++)
        {
            int value = GetCellValue(i, j, map);
            // In ParseArgv function, the cells without inputting values from the user will be filled in 0, which should not be printed
            if (value == 0)
                printf("   ");
            else
                printf(" %d ", GetCellValue(i, j, map));
            if (j % 3 == 0)
            {
                printf("|");
            }
        }
        if (i % 3 == 0)
        {
            printf("\n");
            for (j = 1; j <= 9; j++)
            {
                printf("---");
                if (j % 3 == 0)
                {
                    printf("|");
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

/*
 * Function: SudokuSolution 
 * --------------------
 * Figure out the possible solution for sudoku puzzle by travesing all the blank cells, trying to fill 1-9 in the blank cells one by one, and checking whether the numbers are reasonable. 
 *
 * map[]: sudoku map array
 *
 * returns: return 0 if there is no reasonable solution to the sudoku puzzle, otherwise, return 1.
*/
int SudokuSolution(char map[])
{
    int count = 0;
    if (!SudokuMapCheck(map))
    {
        return count;
    }
    // Define a two-dimensional array with 81 rows (as there are 81 cells at maximum) and 2 columns, to record the coordinate of the cells without inputting values from the user in the two-dimensional array
    int as[81][2] = {{0}};
    // Define a variable, to record the number of cells which need to be filled in
    int step = 0;
    // Traverse all the cells in the sudoku map and store the corresponding info in the as[81][2] and step
    for (int i = 1; i < 10; i++)
    {
        for (int j = 1; j < 10; j++)
        {
            if (GetCellValue(i, j, map) == 0)
            {
                as[step][0] = i;
                as[step][1] = j;
                step++;
            }
        }
    }
    // Use "pos" to traverse all the blank cells and try to fill cur+1 or 0 in, cur range from 0 to 8
    int pos = 0;
    while (pos >= 0 && pos <= step)
    {
        int x = as[pos][0];
        int y = as[pos][1];
        int cur = GetCellValue(x, y, map);

        if (cur < 9)
        {
            // Try to get a reasonable cur through IsValid function
            while (IsValid(x, y, cur + 1, map) == 0)
            {
                cur++;
            }
            // If there is a possibly reasonable cur found, fill cur+1 in the current cell, and repeat same process for the next cell after pos++
            if (cur < 9)
            {
                SetCellValue(x, y, cur + 1, map);
                pos++;
            }
            // If there is no possibly reasonable cur found, fill 0 in the current cell and which also means the number filled in last cell was wrong, will try the next possibly reasonable value (namely the number + 1) in the last cell
            else
            {
                SetCellValue(x, y, 0, map);
                pos--;
            }
        }
        // If the value in the current cell is 9, which means there is no possible values could be tried anymore, fill 0 in the current cell and which also means the number filled in last cell was wrong, will try the next possibly reasonable value (namely the number + 1) in the last cell
        else
        {
            SetCellValue(x, y, 0, map);
            pos--;
        }
        // When pos equals step, a valid solution for the sudoku puzzle is found successfully 
        if (pos == step)
        {
            count++;
            // Print the full sudoku map after the calculation
            // SudokuPrint(map);
        }
    }
    return count;
}

/*
 * Function: ParseArgv 
 * --------------------
 * Fill in the corresponding cells in sudoku map according to the coordinate and value inputted by the user when executing the program, for example: when the user executing ./sudoku_serial 1 1 2 1 4 6, means fill 2 into the cell (1, 1) and fill 6 into the cell (1, 4)
 * 
 * argc: the number of the parameters when the user executes the program
 * argv: the parameters when the user executes the program
 * map[]: sudoku map array
 * 
 * returns: return 0 if argc, argv (including both coordinates and values) or map[] is not valid, otherwise, fill the corresponding values into the cells in the sudoku map and return 1
*/
int ParseArgv(int argc, char **argv, char map[])
{
    // Check if argc is valid
    if ((argc - 1) % 3 != 0)
    {
        return 0;
    }

    // Check if map is valid
    if (!map)
    {
        return 0;
    }

    // Initiate map with 0
    memset(map, 0, sizeof(char) * 81);

    // Check if argv is valid
    for (int i = 1; i < argc; i += 3)
    {
        int x = atoi(argv[i]);
        int y = atoi(argv[i + 1]);
        char value = atoi(argv[i + 2]);
        if (x < 1 || x > 9 || y < 1 || y > 9 || value < 1 || value > 9)
        {
            return 0;
        }
        // If argc, map and argv are valid, fill the corresponding values into the cells in the sudoku map
        SetCellValue(x, y, value, map);
    }
    return 1;
}

/*
 * Function: GetTime 
 * --------------------
 * Get the current time
 * 
 * returns: the current time
*/
long long GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}