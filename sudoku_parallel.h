#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

void SetCellValue(int x, int y, char value, char map[]);

char GetCellValue(int x, int y, char map[]);

int RowColumnCheckDup(int x, int y, char value, char map[]);

int BoxCheckDup(int x, int y, int value, char map[]);

int IsValid(int x, int y, int value, char map[]);

int SudokuMapCheck(char map[]);

void SudokuPrint(char map[]);

int SudokuSolution( char map[]);

int ParseArgv(int argc, char **argv, char map[]);

long long GetTime();