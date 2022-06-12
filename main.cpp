#include <iostream>
#include "sim_mem.h"

using namespace std;

int main() {

    char file1[] = "exec_file1.txt";
    char file2[] = "exec_file2.txt";
    char swap[] = "swap_file1.txt";
    sim_mem mem_sm(file1, file2,swap,25, 50, 25,25, 25, 5,2);

    //insert k,m when the ram is full so with fifo we run over 0,1 pages because dirty=0
    char val = mem_sm.load (1, 5);//Bbbbb
    printf("the val is: %c\n",val);//return B
    val = mem_sm.load (1, 2);//aaAaa
    printf("the val is: %c\n",val);//return A
    char val1 = mem_sm.load (1, 13);//cccCc
    printf("the val is: %c\n",val1);//C
    val1 = mem_sm.load(1, 23);//eeeee
    printf("the val is: %c\n",val1);//e
    val1 = mem_sm.load(1, 32);//ggwgg
    printf("the val is: %c\n",val1);//w
    val1 = mem_sm.load (1, 44);//iiiii
    printf("the val is: %c\n",val1);//i
    val1 = mem_sm.load (1, 54);//kkkkk
    printf("the val is: %c\n",val1);//k
    val1 = mem_sm.load (1, 64);//mmmmM
    printf("the val is: %c\n",val1);//M
    mem_sm.print_memory();//with the run over
    mem_sm.print_swap(); //nothing in the swap

////////////////////////////////////////////////////////////start change and switch//////////////////////////////////////////////////////////

    mem_sm.store(1, 63,'L');//mmmLM
    mem_sm.store(1, 120,'X');//X0000 (instead ccccc)
    val1 = mem_sm.load(1, 120);//now we can take the char because ita not the first time
    printf("the val is: %c\n",val1);//X
    mem_sm.store(1, 44,'I');//iiiiI
    mem_sm.store(1, 52,'P');//kkPkk
    mem_sm.store(1, 33,'I');//ggwIg
    mem_sm.store(1, 73,'E');//switch eeeee and bring:nnnnn
    mem_sm.store(1, 82,'Q');//00Q00 instead ggw.. and now its need to move to the swap file and chang numPage 6
    mem_sm.store(1, 93,'A');//000A0 instead iiiii and not move to the swap file and chang numPage 8
    mem_sm.store(1, 46,'Q');//jQjjj instead kkkkk and now its need to move to the swap file and chang numPage 6
    mem_sm.print_memory();//with the run over

    ////check bss: between 15 to 20
    //first time that work - should put 0
    val1 = mem_sm.load(1, 76);//switch mmmLM and put:00000
    printf("the val is: %c\n",val1);//0
    //store should put char
    mem_sm.store(1, 76,'S');//put:0S000

    ////check heap stack: between 20 to 25
    //store should put char H
    mem_sm.store(1, 100,'H');//put:H0000 instead X0000

    /*full the swap memory with process 2 */
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


    val1 = mem_sm.load(1, 105);//switch jQjjj and put:00000
}