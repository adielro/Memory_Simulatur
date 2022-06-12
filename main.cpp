#include <iostream>
#include "sim_mem.h"

using namespace std;

int main() {

    char file1[] = "exec_file1.txt";
    char file2[] = "exec_file2.txt";
    char swap[] = "swap_file1.txt";
    sim_mem mem_sm(file1, file2,swap,25, 50, 25,25, 25, 5,2);

    char val = mem_sm.load (1, 5);
    printf("the val is: %c\n",val);
    val = mem_sm.load (1, 2);
    printf("the val is: %c\n",val);
    char val1 = mem_sm.load (1, 13);
    printf("the val is: %c\n",val1);
    val1 = mem_sm.load(1, 23);
    printf("the val is: %c\n",val1);
    val1 = mem_sm.load(1, 32);
    printf("the val is: %c\n",val1);
    val1 = mem_sm.load (1, 44);
    printf("the val is: %c\n",val1);
    val1 = mem_sm.load (1, 54);
    printf("the val is: %c\n",val1);
    val1 = mem_sm.load (1, 64);
    printf("the val is: %c\n",val1);
    mem_sm.print_memory();
    mem_sm.print_swap(); 

    mem_sm.store(1, 63,'L');
    mem_sm.store(1, 120,'X');
    val1 = mem_sm.load(1, 120);
    printf("the val is: %c\n",val1);
    mem_sm.store(1, 44,'I');
    mem_sm.store(1, 52,'P');
    mem_sm.store(1, 33,'I');
    mem_sm.store(1, 73,'E');
    mem_sm.store(1, 82,'Q');    mem_sm.store(1, 93,'A');
    mem_sm.store(1, 46,'Q');
    mem_sm.print_memory();//with the run over

    val1 = mem_sm.load(1, 76);
    printf("the val is: %c\n",val1);
    mem_sm.store(1, 76,'S');

    mem_sm.store(1, 100,'H');

    char valFile2 = mem_sm.load (2, 1);
    printf("the val is: %c\n",val);
    mem_sm.store(2, 44,'I');
    mem_sm.store(2, 52,'P');
    mem_sm.store(2, 33,'I');
    mem_sm.store(2, 73,'E');
    mem_sm.store(2, 82,'Q');
    mem_sm.store(2, 93,'A');
    mem_sm.store(2, 46,'Q');
    mem_sm.store(1, 44,'I');
    mem_sm.store(2, 57,'P');
    mem_sm.store(2, 38,'I');
    mem_sm.store(2, 77,'E');
    mem_sm.store(2, 87,'Q');
    mem_sm.store(2, 97,'A');
    mem_sm.store(2, 110,'Q');
    mem_sm.store(2, 115,'Q');
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();
    val1 = mem_sm.load(1, 105);
