#include "sim_mem.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fcntl.h>
using namespace std;
char main_memory[MEMORY_SIZE];

/*
 * sim_mem constructor, initialize program_fd array (num of proc size) by opening the exec files and the sizes
 * the funciong receive.
 * create a swap file and initialize it with '0' * swapSize (page_size * (num_of_pages - text_pages) * num_of_process).
 * create a page_table (struct) two-dimensional array (size of num of process) and initialize the struct elements
 * (V = D = 0, P = 0/1 (text/rest).
 * initialize the main memory with '0';
 * create a swap_link and mem struct of memory_link (holding info about the memory and swap).
 */

sim_mem::sim_mem (char exe_file_name1[], char exe_file_name2[],
                  char swap_file_name[], int text_size, int data_size,
                  int bss_size, int heap_stack_size, int num_of_pages,
                  int page_size, int num_of_process)
{
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->num_of_proc = num_of_process;
    int text_pages = (this->text_size) / this->page_size;
    this->used_memory = 0;
    this->swapSize = (int) (page_size * (num_of_pages - text_pages) * num_of_process);
    this->mem = new memory_link[MEMORY_SIZE / this->page_size];
    this->swap_link = new memory_link[swapSize / this->page_size];
    this->program_fd[0] = open (exe_file_name1, O_RDONLY);
    if (this->program_fd[0] == -1)
    {
        perror ("open file");
        exit (1);
    }
    if (num_of_process == 2)
    {
        this->program_fd[1] = open (exe_file_name2, O_RDONLY);
        if (this->program_fd[1] == -1)
        {
            perror ("open file 2");
            exit (1);
        }
    }
    for (int i = 0; i < MEMORY_SIZE; i++)
        main_memory[i] = '0';
    if ((swapfile_fd = open (swap_file_name, O_CREAT | O_RDWR, 0644)) == -1)
    {
        perror ("swap file");
        exit (1);
    }
    char init[swapSize];
    for (int i = 0; i < swapSize; i++)
    {
        init[i] = '0';
    }
    init[swapSize] = '\0';
    lseek (swapfile_fd, 0, SEEK_SET);
    int wr = write (swapfile_fd, init, swapSize);
    if (wr == -1)
    {
        perror ("write to swap");
        exit (1);
    }
    this->page_table =
            (page_descriptor **) malloc (num_of_process * sizeof (page_descriptor *));
    for (int i = 0; i < num_of_process; i++)
    {
        this->page_table[i] =
                (page_descriptor *) malloc (sizeof (page_descriptor) * num_of_pages);
        for (int j = 0; j < num_of_pages; j++)
        {
            this->page_table[i][j].V = 0;
            this->page_table[i][j].D = 0;
            this->page_table[i][j].P = 1;
            this->page_table[i][j].frame = -1;
            this->page_table[i][j].swap_index = -1;
            if (j < this->text_size / page_size)
                this->page_table[i][j].P = 0;
        }
    }
}

/*
 * Destructor
 */
sim_mem::~sim_mem ()
{
    for (int i = 0; i < this->num_of_proc; i++) {
        free(page_table[i]);
    }
    free(page_table);
    delete[] mem;
    delete[] swap_link;
    for (int i = 0; i < this->num_of_proc; i++) {
        close(this->program_fd[i]);
    }
    close(this->swapfile_fd);
}

/*
 * Function receive process_id (1/2) and address.
 * process_id-- (0 is process 1, 1 is process 2), initilaize offset and page_num.
 * initilaize int ind (frame) = used_memory % (MEMORY_SIZE / this->page_size).
 * from now on consider page as page_table[process_id][page_num]
 * check if the page.V == 1, if 1 (page is in main memory) return the requested char.
 * else check if the page.P == 0, if 0 (need to bring page from exe file), check if memory is full
 * if memory is full use swap_memory() function, then use bring_from_exe() function and return the requested char.
 * if page.P == 1, check if page.D == 1, if 1 (bring page from swap) check if memory full, use swap_memory() if needed.
 * read swap_file (if fail->exit), bring page from swap file (chars in page.swap_index*page_size+i (0<=i<=page_size).
 * insert the page to main_memory and update mem, swap_link and page_table, return the requested char.
 * finally, if V == 0, P == 1, D == 1, check if the requested page is heap, stack (print error and return)
 * else check if memory full (use swap_memory() if needed), check if requested page is bss
 * if bss, initialize new page in main_memory with '0', update mem and page_table and return requested char.
 * else, bring page from the execute file (bring_from_exe()) and return requested char.
 */
char
sim_mem::load (int process_id, int address)
{
    process_id--;
    int offset = address % this->page_size;
    int page_num = address / page_size;
    int ind = this->used_memory % (MEMORY_SIZE / this->page_size);
    if (this->page_table[process_id][page_num].V == 1)
    {
        return main_memory[this->page_table[process_id][page_num].frame * page_size +
                           offset];
    }
    if (this->page_table[process_id][page_num].P == 0)
    {
        if (this->mem[ind].page != -1)
            swap_memory (ind);
        bring_from_exe(ind, process_id, page_num);
        return main_memory[ind * page_size + offset];
    }
    if (this->page_table[process_id][page_num].D == 1)
    {
        if (this->mem[ind].page != -1)
            swap_memory (ind);
        char swap_text[this->swapSize];
        lseek (swapfile_fd, 0, SEEK_SET);
        if (read (this->swapfile_fd, swap_text, swapSize) == -1)
        {
            perror ("read swap_file failed");
            exit (1);
        }
        char page_from_swap[this->page_size];
        for (int i = 0; i < page_size; i++) {
            page_from_swap[i] = swap_text[page_table[process_id][page_num].swap_index * page_size + i];
            swap_text[page_table[process_id][page_num].swap_index * page_size + i] = '0';
        }
        lseek (swapfile_fd, 0, SEEK_SET);
        int wr = write (this->swapfile_fd, swap_text, swapSize);
        if (wr == -1)
        {
            perror ("write to swap");
            exit (1);
        }
        for (int i = 0; i < page_size; i++)
            main_memory[ind + i] = page_from_swap[i];
        this->swap_link[page_table[process_id][page_num].swap_index].proc_id = -1;
        this->swap_link[page_table[process_id][page_num].swap_index].page = -1;
        this->mem[ind].page = page_num;
        this->mem[ind].proc_id = process_id;
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = ind;
        this->page_table[process_id][page_num].swap_index = -1;
        used_memory++;
        return main_memory[ind * page_size + offset];
    }
    else {
        if (page_num >= this->num_of_pages - (this->heap_stack_size / this->page_size)){
            fprintf(stderr, "Invalid memory\n");
            return '\0';
        }
        if (this->mem[ind].page != -1)
            swap_memory (ind);
        if (page_num >= this->num_of_pages - ((this->bss_size + this->heap_stack_size) / page_size)){
            for (int i = 0; i < page_size; i++)
                main_memory[ind * page_size + i] = '0';
            this->mem[ind].page = page_num;
            this->mem[ind].proc_id = process_id;
            this->page_table[process_id][page_num].V = 1;
            this->page_table[process_id][page_num].frame = ind;
            this->page_table[process_id][page_num].swap_index = -1;
            used_memory++;
            return main_memory[ind * page_size + offset];
        }
        else
            bring_from_exe(ind, process_id, page_num);
        return main_memory[ind * page_size + offset];
    }
}

/*
 * Function receive process_id (1/2), address and value.
 * process_id-- (0 is process 1, 1 is process 2), initilaize offset and page_num.
 * initilaize int ind (frame) = used_memory % (MEMORY_SIZE / this->page_size).
 * from now on consider page as page_table[process_id][page_num]
 * check if page.P == 0, if 0 print error (no permission) and return.
 * check if the page.V == 1, if 1 (page is in main memory) insert the value into the requested memory, update page.D = 1
 * and return.
 * check if page.D == 1, if 1 (bring page from swap) check if memory full, use swap_memory() if needed.
 * read swap_file (if fail->exit), bring page from swap file (chars in page.swap_index*page_size+i (0<=i<=page_size).
 * insert the page to main_memory and update mem, swap_link and page_table, insert the value into the requested memory.
 * if D == 0, check if memory full (use swap_memory() if needed), check if the requested page is heap stack or bss
 * if it is,  initialize new page in main_memory with '0', update mem and page_table.
 * else, bring page from the execute file (bring_from_exe()) and insert the value into the requested memory.
 * if store() worked (P != 0), update page.D = 1.
 */

void
sim_mem::store (int process_id, int address, char value)
{
    process_id--;
    int offset = address % page_size;
    int page_num = address / page_size;
    int ind = this->used_memory % (MEMORY_SIZE / this->page_size);
    if (this->page_table[process_id][page_num].P == 0)
    {
        fprintf(stderr, "No permission for this file\n");
        return;
    }
    if (this->page_table[process_id][page_num].V == 1)
    {
        main_memory[this->page_table[process_id][page_num].frame * page_size + offset] = value;
        this->page_table[process_id][page_num].D = 1;
        return;
    }

    if (this->page_table[process_id][page_num].D == 1)
    {   
        if (this->mem[ind].page != -1)
            swap_memory (ind);
        char swap_text[this->swapSize];
        lseek (swapfile_fd, 0, SEEK_SET);
        if (read (this->swapfile_fd, swap_text, swapSize) == -1)
        {
            perror ("read swap_file failed");
            exit (1);
        }
        char page_from_swap[this->page_size];
        for (int i = 0; i < page_size; i++) {
            page_from_swap[i] = swap_text[page_table[process_id][page_num].swap_index * page_size + i];
            swap_text[page_table[process_id][page_num].swap_index * page_size + i] = '0';
        }
        lseek (swapfile_fd, 0, SEEK_SET);
        int wr = write (this->swapfile_fd, swap_text, swapSize);
        if (wr == -1)
        {
            perror ("write to swap");
            exit (1);
        }
        for (int i = 0; i < page_size; i++)
            main_memory[ind * page_size + i] = page_from_swap[i];
        this->swap_link[page_table[process_id][page_num].swap_index].proc_id = -1;
        this->swap_link[page_table[process_id][page_num].swap_index].page = -1;
        this->mem[ind].page = page_num;
        this->mem[ind].proc_id = process_id;
        this->page_table[process_id][page_num].D = 1;
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = ind;
        this->page_table[process_id][page_num].swap_index = -1;
        used_memory++;
        main_memory[(ind * page_size) + offset] = value;
        return;
    }
    else {
        if (this->mem[ind].page != -1)
            swap_memory (ind);
        if (page_num >= this->num_of_pages - ((this->bss_size + this->heap_stack_size) / page_size)){
            for (int i = 0; i < page_size; i++)
                main_memory[ind * page_size + i] = '0';
            this->mem[ind].page = page_num;
            this->mem[ind].proc_id = process_id;
            this->page_table[process_id][page_num].V = 1;
            this->page_table[process_id][page_num].D = 1;
            this->page_table[process_id][page_num].frame = ind;
            this->page_table[process_id][page_num].swap_index = -1;
            used_memory++;
            main_memory[ind * page_size + offset] = value;
            return;
        }
        bring_from_exe(ind, process_id, page_num);
        this->page_table[process_id][page_num].D = 1;
        main_memory[ind * page_size + offset] = value;
        return;
    }

}

/**************************************************************************************/
void
sim_mem::print_memory ()
{
    int i;
    printf ("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        printf ("[%c]\t", main_memory[i]);
        if ((i + 1) % this->page_size == 0)
            printf("\n");
    }
}

/************************************************************************************/
void
sim_mem::print_swap ()
{
    char *str = (char *) malloc (this->page_size * sizeof (char));
    int i;
    printf ("\n Swap memory\n");
    lseek (swapfile_fd, 0, SEEK_SET);	// go to the start of the file
    while (read (swapfile_fd, str, this->page_size) == this->page_size)
    {
        for (i = 0; i < page_size; i++)
        {
            printf ("%d - [%c]\t", i, str[i]);
        }
        printf ("\n");
    }
    free (str);
}

/***************************************************************************************/
void
sim_mem::print_page_table ()
{
    int i;
    for (int j = 0; j < num_of_proc; j++)
    {
        printf ("\n page table of process: %d \n", j);
        printf ("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
        for (i = 0; i < num_of_pages; i++)
        {
            printf ("[%d]\t[%d]\t[%d]\t\t[%d]\t[%d]\n",
                    page_table[j][i].V,
                    page_table[j][i].D,
                    page_table[j][i].P,
                    page_table[j][i].frame, page_table[j][i].swap_index);
        }
    }
}

/*
 * function receive the frame index, check if the page needed to pull out is dirty, if it is, insert to swap_file
 * and update swap_link struct (V = 0, frame = -1, swap_index = first free page slot) and page_table.
 * if the page.D == 0 (not dirty), can be override and update page_table.
 */
void
sim_mem::swap_memory (int frame)
{
    if (this->page_table[this->mem[frame].proc_id][this->mem[frame].page].D == 1)
    {
        char *swap_text = (char *) malloc (this->swapSize);
        lseek (swapfile_fd, 0, SEEK_SET);
        if (read (this->swapfile_fd, swap_text, swapSize) == -1)
        {
            perror ("read swap_file failed");
            free(swap_text);
            exit (1);
        }
        int link_ind = 0;
        while (this->swap_link[link_ind].page != -1)
            link_ind++;
        for (int i = 0; i < page_size; i++)
        {
            swap_text[link_ind * page_size + i] = main_memory[frame * page_size + i];
        }
        lseek (swapfile_fd, 0, SEEK_SET);
        int wr = write (this->swapfile_fd, swap_text, swapSize);
        if (wr == -1)
        {
            free(swap_text);
            perror ("write to swap");
            exit (1);
        }
        this->swap_link[link_ind].proc_id = mem[frame].proc_id;
        this->swap_link[link_ind].page = mem[frame].page;
        this->page_table[this->swap_link[link_ind].proc_id][swap_link[link_ind].page].frame = -1;
        this->page_table[this->swap_link[link_ind].proc_id][swap_link[link_ind].page].V = 0;
        this->page_table[this->swap_link[link_ind].proc_id][swap_link[link_ind].page].swap_index = link_ind;
        free(swap_text);
        return;
    }
    page_table[mem[frame].proc_id][mem[frame].page].frame = -1;
    page_table[mem[frame].proc_id][mem[frame].page].V = 0;
}

/*
 * function receive frame, process_id and page_num, read the desired exe file and insert the requested page into the
 * main memory
 */
void sim_mem::bring_from_exe(int frame, int process_id, int page_num){
    lseek (program_fd[process_id], page_num * page_size, SEEK_SET);
    char temp[page_size];
    if (read (program_fd[process_id], temp, this->page_size) == -1)
    {
        perror ("read failed");
        exit (1);
    }
    for (int i = 0; i < page_size; i++)
        main_memory[(frame * page_size) + i] = temp[i];
    this->mem[frame].page = page_num;
    this->mem[frame].proc_id = process_id;
    this->page_table[process_id][page_num].V = 1;
    this->page_table[process_id][page_num].frame = frame;
    used_memory++;
}