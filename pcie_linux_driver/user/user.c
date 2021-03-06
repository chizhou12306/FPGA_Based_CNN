#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "../altera_dma_cmd.h"

#define BUFFER_LENGTH 40

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_LENGTH 40
#define DATA_SIZE 96 + 11 * 11 * 3 * 96 + 227 * 227 * 3

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if(ch == 27)
    {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
}

char *read_line (char *buf, size_t length, FILE *f)
{
    char *p;
    if (p = fgets (buf, length, f)) {
        size_t last = strlen(buf) - 1;
        if (buf[last] == '\n') {
            buf[last] = '\0';
        } else {
            fscanf (f, "%*[^\n]");
            (void) fgetc (f);
        }
    }
    return p;
}

void print_menu (char *buf, int base_addr, int data_size) {
        printf("\n***********************************************\n");
        printf("** ALTERA 256b DMA driver                    **\n");
        printf("** version %s                       **\n", ALTERA_DMA_DRIVER_VERSION);
        printf("** %d) start DMA                              **\n", ALTERA_CMD_START_DMA                         );
        printf("** %d) enable/disable read dma                **\n", ALTERA_CMD_ENA_DIS_READ                      );
        printf("** %d) enable/disable write dma               **\n", ALTERA_CMD_ENA_DIS_WRITE                     );
        printf("** %d) enable/disable simul dma               **\n", ALTERA_CMD_ENA_DIS_SIMUL                     );
        //printf("** %d) set num dwords (256 - 4096)            **\n", ALTERA_CMD_MODIFY_NUM_DWORDS                 );
        //printf("** %d) set num descriptors (1 - 127)          **\n", ALTERA_CMD_MODIFY_NUM_DESC                   );
        printf("** %d) set read base address                 **\n", ALTERA_CMD_SET_RD_BA                         );
        printf("** %d) set total words to rd/wr              **\n", ALTERA_CMD_SET_DATA_SZ                         );
        //printf("** %d) toggle on-chip or off-chip memory     **\n", ALTERA_CMD_ONCHIP_OFFCHIP            );
        printf("** %d) loop dma                               **\n", ALTERA_LOOP                                  );
        printf("** %d) Start custom write to FPGA            **\n", ALTERA_CMD_DMA_READ                          );
        printf("** %d) Start data read from FPGA             **\n", ALTERA_CMD_DMA_WRITE                          );
        printf("** %d) random                     **\n", ALTERA_CMD_RAND                );
        printf("** %d) exit                                  **\n", ALTERA_EXIT                                  );
        printf("***********************************************\n");
        //printf("Access On Chip RAM    ? %d\n", ((struct dma_status *)buf)->onchip);
        //printf("Random            ? %d\n", ((struct dma_status *)buf) -> rand);
        printf("Run Read                ? %d\n", ((struct dma_status *)buf)->run_read); 
        printf("Run Write               ? %d\n", ((struct dma_status *)buf)->run_write); 
        //printf("Run Simultaneous        ? %d\n", ((struct dma_status *)buf)->run_simul); 
        printf("DMA Read Passed         ? %d\n", ((struct dma_status *)buf)->pass_read); 
        printf("DMA Write Passed        ? %d\n", ((struct dma_status *)buf)->pass_write); 
        //printf("Simultaneous Passed     ? %d\n", ((struct dma_status *)buf)->pass_simul);
        printf("Read EPLast timeout     ? %d\n", ((struct dma_status *)buf)->read_eplast_timeout); 
        printf("Write EPLast timeout    ? %d\n", ((struct dma_status *)buf)->write_eplast_timeout);
        //printf("Number of Dwords/Desc   : %d\n", ((struct dma_status *)buf)->altera_dma_num_dwords); 
        //printf("Number of Descriptors   : %d\n", ((struct dma_status *)buf)->altera_dma_descriptor_num); 
        printf("Length of tx per itr    : %d KB\n", ((struct dma_status *)buf)->length_transfer);
        printf("Rootport address offset : %d\n", ((struct dma_status *)buf)->offset);
        printf("Rd/Wr Base Address      : %d\n", base_addr);
        printf("Rd/Wr Total Data Size   : %d\n", data_size);        
// Each loop contains 2 runs to test wrap around of last descriptor pointer
        printf("Read Time               : %ld s and %ld us\n", ((struct dma_status *)buf)->read_time.tv_sec, ((struct dma_status *)buf)->read_time.tv_usec);
        printf("Read Throughput         : %f GB/S\n", (((struct dma_status *)buf)->length_transfer*0.954)/(((struct dma_status *)buf)->read_time.tv_usec + 1000000*((struct dma_status *)buf)->read_time.tv_sec ));
        printf("Write Time              : %ld s and %ld us\n", ((struct dma_status *)buf)->write_time.tv_sec, ((struct dma_status *)buf)->write_time.tv_usec);
        printf("Write Throughput        : %f GB/S\n", (((struct dma_status *)buf)->length_transfer*0.954)/(((struct dma_status *)buf)->write_time.tv_usec + 1000000*((struct dma_status *)buf)->write_time.tv_sec ));
        printf("Simultaneous Time       : %ld s and %ld us\n", ((struct dma_status *)buf)->simul_time.tv_sec, ((struct dma_status *)buf)->simul_time.tv_usec);
        printf("Simultaneous Throughput : %f GB/S\n", (((struct dma_status *)buf)->length_transfer*0.954*2)/(((struct dma_status *)buf)->simul_time.tv_usec + 1000000*((struct dma_status *)buf)->simul_time.tv_sec ));

        printf("# ");
}

void main () {

    ssize_t f = open ("/dev/altera_dma", O_RDWR);
    if (f == -1) {
        printf ("Couldn't open the device.\n");
        return;
    } else {
        printf ("Opened the device: file handle #%lu!\n", (long unsigned int)f);
    }
    char *buf = malloc(sizeof(struct dma_status));
    struct dma_cmd cmd;
    int tx_num_words;
    int *tx_buf;
    cmd.usr_buf_size = sizeof(struct dma_status);
    cmd.offset = 0;
    int value, rc;
    int num_tx;
    char line[BUFFER_LENGTH];
    int num_input;
    int i, loop_num;
    int j=1;
    int base_addr = 0;
    int data_size = DATA_SIZE;
    int pad_count = 0;
    int flag = 0;
    int file_wr_count = 0;
    
    /*cmd.cmd = ALTERA_CMD_READ_STATUS;
    cmd.buf = buf;
    write (f, &cmd, 0);
    tx_num_words = ((struct dma_status *)buf)->altera_dma_num_dwords * ((struct dma_status *)buf)->altera_dma_descriptor_num;
    tx_buf = (int *)malloc(tx_num_words * sizeof(int));*/

    do {   
        cmd.cmd = ALTERA_CMD_READ_STATUS;
        cmd.buf = buf;
        cmd.offset = 0;
        write (f, &cmd, 0);
    
        tx_num_words = ((struct dma_status *)buf)->altera_dma_num_dwords * ((struct dma_status *)buf)->altera_dma_descriptor_num;
        if(tx_num_words == 0) {
            printf("Some Error in Kernel Module, 'dma_status' parameter values not initialized properly.\n");
            return;
        }
        
        tx_buf = (int *)malloc(tx_num_words * sizeof(int));
        cmd.data_ptr = tx_buf;

        //system("clear");
        print_menu(buf, base_addr, data_size);
        read_line(line, BUFFER_LENGTH, stdin);
        num_input = strtol(line, NULL, 10);
        switch (num_input)
        {
           case ALTERA_EXIT:
                break;
           case ALTERA_CMD_START_DMA:
                ioctl(f, ALTERA_IOCX_START);
                ioctl(f, ALTERA_IOCX_WAIT);
                cmd.cmd = ALTERA_CMD_READ_STATUS;
                cmd.buf = buf;
                write (f, &cmd, 0);
                break;
           case ALTERA_CMD_ENA_DIS_READ:
                cmd.cmd = ALTERA_CMD_ENA_DIS_READ;
                cmd.buf = buf;
                write (f, &cmd, 0);
                break;
           case ALTERA_CMD_ENA_DIS_WRITE:
                cmd.cmd = ALTERA_CMD_ENA_DIS_WRITE;
                cmd.buf = buf;
                write (f, &cmd, 0);
                break;
           case ALTERA_CMD_ENA_DIS_SIMUL:
                cmd.cmd = ALTERA_CMD_ENA_DIS_SIMUL;
                cmd.buf = buf;
                write (f, &cmd, 0);
                break;
          /*case ALTERA_CMD_MODIFY_NUM_DWORDS:
                cmd.cmd = ALTERA_CMD_MODIFY_NUM_DWORDS;
                cmd.buf = buf;
                printf("enter # dwords: ");
                read_line(line, BUFFER_LENGTH, stdin);
                num_input = strtol(line, NULL, 10);
                *(int *)buf = num_input;
                //if (num_input < 256 || num_input > 4096)
         if (num_input < 1 || num_input > 0x3FFFF){
            printf("the maximum transfer size of each descriptor is 0x3FFFF DW (1MB)\n");
                    break;}
                else
                    write (f, &cmd, 0);
                break;*/
           /*case ALTERA_CMD_MODIFY_NUM_DESC:
                cmd.cmd = ALTERA_CMD_MODIFY_NUM_DESC;
                cmd.buf = buf;
                printf("enter desc num: ");
                read_line(line, BUFFER_LENGTH, stdin);
                num_input = strtol(line, NULL, 10);
                *(int *)buf = num_input;
                if (num_input > 128 || num_input < 1)
                    break;
                else
                    write (f, &cmd, 0);
                break;*/
          case ALTERA_CMD_SET_RD_BA:
                cmd.cmd = ALTERA_CMD_SET_RD_BA;
                cmd.buf = buf;
                printf("Enter Read/Write Base Address (integer): ");
                read_line(line, BUFFER_LENGTH, stdin);
                num_input = strtol(line, NULL, 10);
                if (num_input < 0x3ffffff || num_input > 0){ // 2 GB DDR3 SDRAM
                    base_addr = num_input;
                    printf("Number entered is out of address Range\n");
                }
                break;
         case ALTERA_CMD_SET_DATA_SZ:
                cmd.cmd = ALTERA_CMD_SET_DATA_SZ;
                cmd.buf = buf;
                printf("Enter number of elements to Read/Write (integer): ");
                read_line(line, BUFFER_LENGTH, stdin);
                num_input = strtol(line, NULL, 10);
                if (num_input < 0x3ffffff || num_input > 0){ // 2 GB DDR3 SDRAM
                    data_size = num_input;
                    printf("Number entered is out of address Range\n");
                }
                break;
       case ALTERA_CMD_ONCHIP_OFFCHIP:
        cmd.cmd = ALTERA_CMD_ONCHIP_OFFCHIP;
                cmd.buf = buf;
                write (f, &cmd, 0);
                break;
      case ALTERA_CMD_RAND:
        cmd.cmd = ALTERA_CMD_RAND;
        cmd.buf = buf;
        write (f, &cmd, 0);
        break;
      case ALTERA_LOOP:
        printf("enter loop num (0 for infinite and press ESC to quit the loop): ");
        read_line(line, BUFFER_LENGTH, stdin);
        loop_num = strtol(line, NULL, 10);
        if(loop_num != 0) {
            for (i = 0; i < loop_num; i++) {
                ioctl(f, ALTERA_IOCX_START);
                ioctl(f, ALTERA_IOCX_WAIT);
                cmd.cmd = ALTERA_CMD_READ_STATUS;
                cmd.buf = buf;
                write (f, &cmd, 0);
                if ( (!((struct dma_status *)buf)->pass_read && ((struct dma_status *)buf)->run_read)  || 
                        (!((struct dma_status *)buf)->pass_write && ((struct dma_status *)buf)->run_write)  || 
                        (!((struct dma_status *)buf)->pass_simul && ((struct dma_status *)buf)->run_simul)) {
                    system("clear");
                    print_menu(buf, base_addr, data_size);
                    printf("DMA data error!\n");
                    printf("Type in dmesg to show more details!\n");
                    return;
                }
                system("clear");
                print_menu(buf, base_addr, data_size);
                printf("Press ESC to stop\n");
//                        usleep(1000*250);
                if(kbhit()) break;
            }
        }
        else{
            do{
                ioctl(f, ALTERA_IOCX_START);
                ioctl(f, ALTERA_IOCX_WAIT);
                cmd.cmd = ALTERA_CMD_READ_STATUS;
                cmd.buf = buf;
                write (f, &cmd, 0);
                if ( (!((struct dma_status *)buf)->pass_read && ((struct dma_status *)buf)->run_read)  || 
                        (!((struct dma_status *)buf)->pass_write && ((struct dma_status *)buf)->run_write)  || 
                        (!((struct dma_status *)buf)->pass_simul && ((struct dma_status *)buf)->run_simul)) {
                    system("clear");
                    print_menu(buf, base_addr, data_size);
                    printf("DMA data error!\n");
                    printf("Type in dmesg to show more details!\n");
                    return;
                }
                system("clear");
                print_menu(buf, base_addr, data_size);
                printf("Press ESC to stop\n");
//                        usleep(1000*250);
                if(kbhit()) break;
            }while(1);
        }
        break;

//----------------------------------------------------------------------------------
//============== Added Functionality to Dump AlexNet to DE5-net board ==============
//----------------------------------------------------------------------------------

        case ALTERA_CMD_DMA_READ:{
            printf("Writing Data into FPGA ... Wait or Press ESC to stop.\n");
            
            FILE *fp;
            printf("Here read 3a1!\n");
            fp = fopen("./AlexNet_data.txt", "r");
            printf("Here read 3a2!\n");
            if(!fp){
                printf("Could not open model file!\n");
                break;
            }
            printf("Here read 3a3!\n");
            // Perform operations block by block
            
            /*rc = fscanf(fp, "%d\n", &value);
            if(rc != 1) {
                printf("Returned %d elements, Error!\n", rc);
                break;
            }
            if(data_size > value){
                printf("Not enough Data in Model File, either reduce #elements to write or increase data in model file\n", rc);
                scanf("Press Enter to continue");
                break;
            }*/
            
            if(data_size % tx_num_words) num_tx = data_size/tx_num_words + 1;
            else num_tx = data_size/tx_num_words;
            
            //full_count = data_size / tx_num_words;
            //part_count = data_size % tx_num_words;
            //pad_count = tx_num_words - data_size % tx_num_words;
            flag = 0;
            for(i = 0; i < num_tx; i++){
                
                // Load new 64 blocks into host memory
                for(j = 0; j < tx_num_words; j++){
                    if(!flag){
                        if(fscanf(fp, "%d\n", &value) == EOF){
                            flag = 1;
                            *(tx_buf+j) = 0;
                            continue;
                        }    
                        
                        /*if(rc != 1) {
                            printf("Returned %d elements, Error!\n", rc);
                            break;
                        }*/
                        
                        
                        *(tx_buf+j) = value;
                    }
                    else *(tx_buf+j) = 0;
                }    

                // Pass the data in userspace to kernelspace
                printf("Here read 3a!\n");
                cmd.cmd = ALTERA_CMD_PREP_READ;
                cmd.buf = buf;
                printf("Here read 3b!\n");
                cmd.offset = tx_num_words*i + base_addr;
                printf("Here read 3c!\n");
                cmd.data_ptr = tx_buf; 
                printf("Here read 3d!\n");
                write (f, &cmd, 0);
                printf("Here 3!\n");

                /*cmd.cmd = ALTERA_CMD_READ_STATUS;
                cmd.buf = buf;
                write (f, &cmd, 0);*/

                // Now initiate the write process
                ioctl(f, ALTERA_IOCX_DMA_READ);
                printf("Here 3a!\n");
                ioctl(f, ALTERA_IOCX_WAIT);
                printf("Here 4!\n");
                // Update applicaiton Status fields
                cmd.cmd = ALTERA_CMD_READ_STATUS;
                printf("Here 5!\n");
                cmd.buf = buf;
                printf("Here 6!\n");
                write (f, &cmd, 0);
                printf("Here 7!\n");
                
                //free(tx_buf);
            }
            fclose(fp);
            break;
        }
        
        case ALTERA_CMD_DMA_WRITE:{
            printf("Reading Data from FPGA ... Wait or Press ESC to stop.\n");
            
            FILE *fp;
            printf("Here write 3a1!\n");
            fp = fopen("./AlexNet_data_rd.txt", "w");
            printf("Here write 3a2!\n");
            if(!fp){
                printf("Could not open model file!\n");
                break;
            }
            printf("Here write 3a3!\n");
            
            // Perform operations block by block
            if(data_size % tx_num_words) num_tx = data_size/tx_num_words + 1;
            else num_tx = data_size/tx_num_words;
            
            //pad_count = data_size % tx_num_words;
            for(i = 0; i < num_tx; i++){

                // Load new 64 blocks into host memory
                /*for(j = 0; j < tx_num_words; j++){
                    rc = fscanf(fp, "%d\n", &value);
                    if(rc != 1) {
                        printf("Returned %d elements, Error!\n", rc);
                        break;
                    }
                    *(tx_buf+j) = value;
                }    */

                // Pass the data in userspace to kernelspace
                printf("Here write 3a!\n");
                cmd.cmd = ALTERA_CMD_READ_STATUS;
                printf("Here write 3b!\n");
                cmd.buf = buf;
                printf("Here write 3c!\n");
                cmd.offset = tx_num_words*i + base_addr;
                printf("Here write 3d!\n");
                write (f, &cmd, 0);

                // Now initiate the write process
                ioctl(f, ALTERA_IOCX_DMA_WRITE);
                printf("Here 3ab!\n");
                ioctl(f, ALTERA_IOCX_WAIT);
                printf("Here 4b!\n");
                
                cmd.cmd = ALTERA_CMD_FINISH_WRITE;
                cmd.buf = buf;
                cmd.data_ptr = tx_buf;
                memset((char *)tx_buf, 0, tx_num_words*4);
                write (f, &cmd, 0);
                printf("Here 3b!\n");
                
                // Update applicaiton Status fields
                /*cmd.cmd = ALTERA_CMD_READ_STATUS;
                cmd.buf = buf;
                write (f, &cmd, 0);*/
                if(i < num_tx-1) file_wr_count = tx_num_words;
                else if(data_size % tx_num_words) file_wr_count = data_size % tx_num_words;
                else file_wr_count = tx_num_words;
                 
                for(j = 0; j < file_wr_count; j++){
                    fprintf(fp, "%u, 0x%08x\n", *(cmd.data_ptr+j), *(cmd.data_ptr+j));
                }
            }
            fclose(fp);
            break;
        }
//================================= X === X === X ==================================
            
        default:
            printf("%d is an invalid command\n", num_input);
        }
        //system("clear");
        free(tx_buf);
    } while (num_input != ALTERA_EXIT);
    free(buf);
    close (f);
}


