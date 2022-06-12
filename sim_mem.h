#ifndef SIM_MEM_H
#define SIM_MEM_H
#define MEMORY_SIZE 30
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor
{
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;
/*
 * struct is used for linking info from main_memory and swap_file
 */
typedef struct memory_link{
    int proc_id = -1;
    int page = -1;
} memory_link;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd[2]; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    int used_memory;
    int swapSize;
    page_descriptor **page_table; //pointer to page table
    memory_link *mem;
    memory_link *swap_link;
public:
    sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name[], int
    text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int
            page_size, int num_of_process);
    ~sim_mem();
    char load(int process_id, int address);
    void store(int process_id, int address, char value);
    void print_memory();
    void print_swap ();
    void print_page_table();
protected:
    void swap_memory(int frame); // insert page from memory into swap_file
    void bring_from_exe(int frame, int proc, int page_num); // bring page from exe file
};



#endif