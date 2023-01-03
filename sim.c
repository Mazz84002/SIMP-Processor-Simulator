#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define LINE_LENGHT 10
#define REGSIZE 16
static int REG[REGSIZE]; // register to store operations signed integers
static unsigned IOREG[23] = {0};
const char *OPCODES[] = {"add", "sub", "mul",
                         "and", "or", "xor",
                         "sll", "sra", "srl",
                         "beq", "bne", "blt", "bgt", "ble", "bge",
                         "jal", "lw", "sw", "reti",
                         "in", "out", "halt"
};
const char *REGNAMES[16] = {
        "$zero", "$imm", "$v0",
        "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2",
        "$s0", "$s1", "$s2",
        "$gp", "$sp", "$ra"
};
const char *IOREGNAMES[23] = { "irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status", "irq2status", "irqhandler", "irqreturn",
                               "clks", "leds", "display7seg",
                               "timerenable", "timercurrent", "timermax",
                               "diskcmd", "disksector", "diskbuffer", "diskstatus", "reserved18", "reserved19",
                               "monitoraddr", "monitordata", "monitorcmd"
};
void print_REG(){
    for (int i = 0; i < 16; ++i) {
        printf("(%d,%d),", i, REG[i]);
    }
    printf("\n");
    for (int i = 0; i < 16; ++i) {
        printf("(%d,%X)", i, REG[i]);
    }
    printf("\n");
}
void print_IOREG(){
    printf("IOREG - ");
    for (int i = 0; i < 23; ++i) {
        if (i == 8 || i == 11 || i == 14 || i == 18 || i == 20){
            printf(" | ");
        }
        printf("(%d,%X),", i, IOREG[i]);
    }
    printf("\n");
}

// ---------------------------------- OPERATIONS -----------------------------------------
struct instruction{
    unsigned rs;
    unsigned rt;
    unsigned rd;
    unsigned op;
    unsigned inst_type;
    int imm;
};


// ---------------------------------------------------------------------------------------------

int perform_op(struct instruction inst, int pc, int MEM[], int MONITOR[], int DISK[][128], int* irq2, FILE*leds, FILE* display7seg, FILE* hwregtrace);


int h2d(char line[]){ // converts hexadecimal to integer
    if (line[0] == '8' || line[0] == '9' || line[0] == 'A' || line[0] == 'B' || line[0] == 'C' || line[0] == 'D' || line[0] == 'E'  | line[0] == 'F'){
        return (int)(0xFFF00000 + strtol(line, (char **)NULL, 16));
    }

    return (int)strtol(line, (char **)NULL, 16);
}

int is_itype(struct instruction inst){ // find out the type of instruction
    if ((inst.rt == 1) || (inst.rs == 1) || (inst.rd == 1)){
        return 1;
    }
    return 0;
}

struct instruction decode(unsigned machine_code, int pc, int i, int MEM[]){
    struct instruction inst;
    inst.rt = machine_code & 0x0000000F;
    inst.rs = (machine_code & 0x000000F0)/(16);
    inst.rd = (machine_code & 0x00000F00)/(256);
    inst.op = (machine_code & 0x000FF000)/(4096);
    inst.inst_type = is_itype(inst);
    if (inst.inst_type == 1){
        inst.imm = MEM[i+1];
        REG[1] = inst.imm;
    }
    return inst;
}

FILE* open_file(char name[], char mode[]){ // loads a file
    FILE *f = fopen(name, mode);
    if (f == NULL){
        printf("Unable to open file\n");
        exit(1);
    }
    return f;
}

void create_temp_memory(FILE* memin, int MEM[]){
    fseek(memin, 0, SEEK_SET);
    char line[LINE_LENGHT];
    int i = 0;
    while (fgets(line, LINE_LENGHT, memin)){
        MEM[i] = h2d(line);
        i++;
    }
}

void print_memory(int MEM[]){
    for (int i = 0; i < 4096; ++i) {
        if (MEM[i] != 0)
            printf("(%d,%05X) ", i, MEM[i]);
    }
    printf("\n");
}

void create_temp_diskout(FILE* diskin, int DISK[][128]){
    fseek(diskin, 0, SEEK_SET);
    char line[LINE_LENGHT];
    int i = 0;
    while (fgets(line, LINE_LENGHT, diskin)){
        DISK[i/128][i%128] = h2d(line);
        i++;
    }
}

void print_disk(int DISK[][128]){
    for (int i = 0; i < 128; ++i) {
        for (int j = 0; j < 128; ++j) {
            printf("%05X ", DISK[i][j]);
        }
        printf("\n");
    }
}


// ------------------------------------------- TRACE --------------------------------------------
void write_trace(unsigned machine_code, FILE* trace, int pc){ // writes the instruction to trace file BEFORE every instruction
    fprintf(trace,"%03X %05X ",pc, machine_code);
    for (int i = 0; i <= 15; ++i) {
        fprintf(trace, "%08X ", REG[i]);
    }
    fprintf(trace, "\n");
}


// ------------------------------------ OPERATIONS -------------------------------------------------

void monitor_update(int MONITOR[], struct instruction inst);
void disk_update(int DISK[][128], int MEM[], struct instruction inst, int*irq2);
void write_leds(struct instruction inst, FILE* leds);
void write_display7seg(struct instruction inst, FILE* display7seg);
void write_hwregtrace(struct instruction inst, FILE* hwregtrace);

int perform_op(struct instruction inst, int pc, int MEM[], int MONITOR[], int DISK[][128], int *irq2, FILE* leds, FILE* display7seg, FILE* hwregtrace){ // takes in struct instruction, performs the required operations and returns the pc increment
    int pc_inc = 0;
    if (inst.op == 0){ // add
        REG[inst.rd] = REG[inst.rs] + REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 1){ // sub
        REG[inst.rd] = REG[inst.rs] - REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 2){ // mul
        REG[inst.rd] = REG[inst.rs] * REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 3){ // and
        REG[inst.rd] = REG[inst.rs] & REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 4){ // or
        REG[inst.rd] = REG[inst.rs] | REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 5){ // xor
        REG[inst.rd] = REG[inst.rs] ^ REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 6){ // sll
        REG[inst.rd] = REG[inst.rs] << REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 7){ // sra (CHANGE TO ARITHMETIC SHIFT)
        REG[inst.rd] = (int)(REG[inst.rs] >> REG[inst.rt]) + (int)pow(2, 20)*(REG[inst.rs]%10);
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 8){ // srl
        REG[inst.rd] = REG[inst.rs] >> REG[inst.rt];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 9){ // beq
        if (REG[inst.rs] == REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 10){ // bne
        if (REG[inst.rs] != REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 11){ // blt
        if (REG[inst.rs] < REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 12){ // bgt
        if (REG[inst.rs] > REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 13){ // ble
        if (REG[inst.rs] <= REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 14){ // bge
        if (REG[inst.rs] >= REG[inst.rt]){
            pc_inc = REG[inst.rd] - pc;
        } else{
            pc_inc = 2;
        }
    }
    else if (inst.op == 15){ // jal
        REG[inst.rd] = pc + 2;
        pc_inc = REG[inst.rs] - pc;
        printf("increment for jump = %d\n", pc_inc);
    }
    else if (inst.op == 16){ // lw
        REG[inst.rd] = MEM[REG[inst.rs] + REG[inst.rt]];
        pc_inc = 2;
    }
    else if (inst.op == 17){ // sw
        printf("Storing at %d the value %d\n", REG[inst.rs] + REG[inst.rt], REG[inst.rd]);
        MEM[REG[inst.rs] + REG[inst.rt]] = REG[inst.rd];
        pc_inc = 2;
    }
    else if (inst.op == 18){ // reti
        pc = IOREG[7];
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 19){ // in - to read from IOREG
        printf("->IN (read) REQUEST\n");
        printf("DESTINATION - %s(%d) and value is %d\n", REGNAMES[inst.rd], inst.rd, IOREG[REG[inst.rs]+REG[inst.rt]]);
        REG[inst.rd] = (int)IOREG[REG[inst.rs]+REG[inst.rt]];
        monitor_update(MONITOR, inst);
        disk_update(DISK, MEM, inst, irq2);
        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    }
    else if (inst.op == 20){ // out - to write IOREG
        printf("->OUT (write) REQUEST\n");
        printf("DESTINATION - %s(%d) and value is %d\n", IOREGNAMES[REG[inst.rs] + REG[inst.rt]], REG[inst.rs] + REG[inst.rt], REG[inst.rd]);
        IOREG[REG[inst.rs] + REG[inst.rt]] = (unsigned)REG[inst.rd];
        monitor_update(MONITOR, inst);
        disk_update(DISK, MEM, inst, irq2);

        pc_inc = 1;
        if (inst.inst_type == 1){
            pc_inc = 2;
        }
    } else{ // halt - handled in void simulator()
        pc_inc = 1;
    }
    printf("pc-increment = %d\n", pc_inc);
    return pc_inc;
}


// ------------------------------------ INTERRUPTS ----------------------------------------

void reset_clk();
void increment_clk(struct instruction inst, int * irq2);
void update_timer();

// irq0 is integrated in the timer


// This code is used for detecting irq2
// It will be executed after every clock update, and set IOREG[2]
int h2ud(char line[]){ // converts hexadecimal to unsigned integer
    return (int)strtol(line, (char **)NULL, 16);
}

int* store_irq2_in_array(FILE* irq2in){
    char line[LINE_LENGHT];
    int count = 0;
    while (fgets(line, LINE_LENGHT, irq2in)){
        count++;
    }
    fseek(irq2in,0,SEEK_SET);
    int* irq2 = (int*)malloc(sizeof(int) * count);
    int i = 0;
    while(!feof(irq2in)){
        fgets(line, LINE_LENGHT, irq2in);
        irq2[i] = atoi(line);
        //irq2[i] = strtol()
        i++;
    }
    irq2[i] = -1;
    return irq2;
}

void turn_on_irq2(int *irq2){ // turns on interrupt bit if current clk belongs to irq2in.txt
    //int l = sizeof(irq2)/sizeof(irq2[0]);
    int i = 0;
    while (irq2[i] != -1){
        if (irq2[i] == IOREG[8]){
            printf("We found irqstatus2 at %d\n", IOREG[8]);
            IOREG[5] = 1; // turn on the status bit
        }
        i++;
    }
}

void interrupt_off(){
    for (int i = 3; i < 6; ++i) {
        IOREG[i] = 0;
    }
}

int check_irq(){ // gives one if we must go into an ISR
    return((IOREG[0] && IOREG[3]) || (IOREG[1] && IOREG[4]) || (IOREG[2] && IOREG[5]));
}

int ISR(int pc, int MEM[], int MONITOR[], int DISK[][128], int* irq2, FILE* trace, FILE* leds, FILE* display7seg, FILE* hwregtrace){ // handles the entire interrupt service routine and returns the pc after executing reti
    interrupt_off();
    IOREG[7] = pc; // store previous pc
    pc = IOREG[6]; // get the pc from `irqhandler`
    struct instruction inst;
    int i = pc;
    inst.op = 0;
    while (inst.op != 18){ // keep iterating till you reach reti
        printf("-----------------------------------------------------------------------------\n");
        printf("%05X\n", MEM[i]);
        printf("PC - %d\n", pc);
        printf("CLOCK - %d/%X\n", IOREG[8], IOREG[8]);
        unsigned machine_code = MEM[i];
        inst = decode(machine_code, pc, i, MEM);
        printf("DECODED: op=%s(%d), rd=%s(%d), rs=%s(%d), rt=%s(%d), type=%d, imm=%d\n",
               OPCODES[inst.op], inst.op, REGNAMES[inst.rd], inst.rd, REGNAMES[inst.rs], inst.rs, REGNAMES[inst.rt], inst.rt, inst.inst_type, inst.imm);
        printf("Before Operations - \n");
        print_IOREG();
        print_REG();
        write_trace(machine_code, trace, pc);

        pc += perform_op(inst, pc, MEM, MONITOR, DISK, irq2, leds, display7seg, hwregtrace);
        i = pc;
        increment_clk(inst, irq2);
        printf("After Operations - \n");
        write_leds(inst, leds);
        write_display7seg(inst, display7seg);
        write_hwregtrace(inst, hwregtrace);
        print_IOREG();
        print_REG();
    }
    return IOREG[7];
}


// ---------------------------------------------- TIMER ---------------------------------------

void update_timer(){ // updates the timer for a line in file (plus handeling the interrupts) - to be run after very clock update
    IOREG[3] = 0; // restting irqstatus0
    IOREG[12]++;
    if (IOREG[11] == 1){ // enable interrupt when timerenable = 1
        IOREG[0] = 1;
    }

    if (IOREG[12] == IOREG[13]){ // checking for the irqstatus0
        IOREG[3] = 1;
        IOREG[12] = 0;
    }
}

// ------------------------------------ CLOCK -----------------------------------

void reset_clk(){ // checks and resets clk to zero - to be used before every update of clk
    if (IOREG[8] == 0xffffffff){
        IOREG[3] = 1;
        IOREG[8] = 0;
    }
}

void increment_clk(struct instruction inst, int * irq2){
    reset_clk();
    IOREG[8]++;
    update_timer();
    turn_on_irq2(irq2);
    if (inst.inst_type == 1){
        reset_clk();
        IOREG[8]++;
        update_timer();
        turn_on_irq2(irq2);
        if (inst.op == 16 || inst.op == 17){
            reset_clk();
            IOREG[8]++;
            update_timer();
            turn_on_irq2(irq2);
        }
    }
}

// ------------------------------------- DISPLAY7SEG -------------------------------------------

// no need for extra function
void write_display7seg(struct instruction inst, FILE* display7seg){
    if (inst.op == 20 && REG[inst.rs] + REG[inst.rt] == 10){
        fprintf(display7seg, "%d %08X\n", IOREG[8]-1, IOREG[10]);
    }
}

// ---------------------------- HWREGTRACE ---------------------------------

void write_hwregtrace(struct instruction inst, FILE* hwregtrace){
    if (inst.op == 20){
        fprintf(hwregtrace, "%d WRITE %s %08X\n", IOREG[8]-1, IOREGNAMES[REG[inst.rs] + REG[inst.rt]], IOREG[REG[inst.rs] + REG[inst.rt]]);
    }
    if (inst.op == 19){
        fprintf(hwregtrace, "%d READ %s %08X\n", IOREG[8]-1, IOREGNAMES[REG[inst.rs] + REG[inst.rt]], IOREG[REG[inst.rs] + REG[inst.rt]]);
    }
}
// ------------------------------------------- DISK --------------------------------------------

void disk_update(int DISK[][128], int MEM[], struct instruction inst, int*irq2){
    int destination = REG[inst.rs]+REG[inst.rt];
    // disksector, diskbuffer, diskstatus are simple IO instructions already running using existing commands
    // disk_update will be run when we have an `out` instruction with diskcmd, which will tell us to read/write from the disk
    if (IOREG[14] == 1){ // read sector
        printf("WE ARE IN DISK READING\n");
        // we need to read sector `disksector` into MEM[`diskbuffer`]
        int buffer = IOREG[16];
        int sector_num = IOREG[15];
        for (int j = 0; j<128; j++){
            MEM[j+buffer] = DISK[sector_num][j];
        }
        // incrementing the clock
        for (int k = 0; k<1024; k++){
            increment_clk(inst, irq2);
        }
        // After 1024 clock cycles the hardware registers “diskstatus” and “diskcmd” will be set to 0
        IOREG[14] = 0; IOREG[17] = 0;
        IOREG[4] = 1; //irq1status = 1
    }
    if (IOREG[14] == 2){ // write sector
        printf("WE ARE IN DISK WRITING\n");
        // we need to write MEM[`disksector`] into DISK[`disksector`][0->127]
        int buffer = IOREG[16];
        int sector_num = IOREG[15];
        for (int j = 0; j<128; j++){
            DISK[sector_num][j] = MEM[j+buffer];
        }
        // incrementing the clock
        for (int k = 0; k<1024; k++){
            increment_clk(inst, irq2);
        }
        // After 1024 clock cycles the hardware registers “diskstatus” and “diskcmd” will be set to 0
        IOREG[14] = 0; IOREG[17] = 0;
        IOREG[4] = 1; //irq1status = 1
    }
}


// ---------------------------------------- LEDS ----------------------------------------------

// No need for seperate function
void write_leds(struct instruction inst, FILE* leds){
    if (inst.op == 20 && REG[inst.rs] + REG[inst.rt] == 9){
        fprintf(leds, "%d %08X\n", IOREG[8]-1, IOREG[9]);
    }
}

// ------------------------------------ MONITOR -----------------------------------------

void monitor_update(int MONITOR[], struct instruction inst){
    int destination = REG[inst.rs] + REG[inst.rt];
    if (inst.op == 19){ // in - read
        if (destination == 22){ // monitorcmd
            IOREG[destination] = 0;
        }
    }
    if (inst.op == 20){ // out - write
        if (destination == 22){ // monitorcmd
            IOREG[destination] = 1;
            printf("WRITTING TO MONITOR - address is %d and value is %d\n", IOREG[20], IOREG[21]);
            MONITOR[IOREG[20]] = IOREG[21];
        }
    }
}



// ---------------------------------------- SIMULATOR --------------------------------------

void simulator(int MEM[], int MONITOR[], int DISK[][128], int* irq2, FILE* trace, FILE* cycles, FILE* leds, FILE* irq2in, FILE* display7seg, FILE* hwregtrace){
    struct instruction inst;
    int pc = 0; // the program counter stores the line number of the instruction that is to be executed
    int i = pc;
    int j = 0;
    inst.op = 0;
    while (inst.op != 21){
        printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("%05X\n", MEM[i]);
        printf("PC - %d/%03X\n", pc, pc);
        printf("CLOCK - %d/%X\n", IOREG[8], IOREG[8]);
        unsigned machine_code = MEM[i];
        inst = decode(machine_code, pc, i, MEM);
        printf("DECODED: op=%s(%d), rd=%s(%d), rs=%s(%d), rt=%s(%d), type=%d, imm=%d\n",
               OPCODES[inst.op], inst.op, REGNAMES[inst.rd], inst.rd, REGNAMES[inst.rs], inst.rs, REGNAMES[inst.rt], inst.rt, inst.inst_type, inst.imm);
        printf("Before Operations - \n");
        print_IOREG();
        print_REG();
        write_trace(machine_code, trace, pc);

        pc += perform_op(inst, pc, MEM, MONITOR, DISK, irq2, leds, display7seg, hwregtrace);

        increment_clk(inst, irq2);
        printf("\n");
        printf("After Operations - \n");
        write_leds(inst, leds);
        write_display7seg(inst, display7seg);
        write_hwregtrace(inst, hwregtrace);
        print_IOREG();
        print_REG();

        // checking ISR - ISR won't work for CLK 0 (but you need >1 clk to set irqenable so it doesn't matter)
        while (check_irq()){ // if we have an interrupt
            printf("----------------------------------------------------- RUNNING ISR -----------------------------------------------------\n");
            pc = ISR(pc, MEM, MONITOR, DISK, irq2, trace, leds, display7seg, hwregtrace);
            printf("----------------------------------------------------- FINISHED ISR ----------------------------------------------------\n");
        }


        i = pc;
    }
    fprintf(cycles, "%d", IOREG[8]);

}

// ----------------------------- GENERATING OUTPUT FILES ------------------------------

int last_non_zero_element(int A[], int size){ // takes in an array and returns the position of last non-zero element
    int pos = size-1;
    while (A[pos] == 0){
        pos--;
    }
    return pos;
}


void generate_memout(int MEM[], FILE* memout){
    int end = last_non_zero_element(MEM, 4096);
    for (int i = 0; i <= end; ++i) {
        fprintf(memout, "%05X\n", MEM[i] & 0x000FFFFF);
    }
}

void print_arr(int irq2[]){
    int i = 0;
    for (i = 0; i <= 9; i++){
        printf("%d, ", irq2[i]);
    }
    printf("\n");
}

void generate_monitor(int MONITOR[], FILE* monitor){
    for (int i = 0; i < 256*256; i++){
        fprintf(monitor, "%02X\n", MONITOR[i]);
    }

}

void generate_diskout(int DISK[][128], FILE* diskout){
    for (int i = 0; i < 128; ++i) {
        for (int j = 0; j < 128; ++j) {
            fprintf(diskout, "%05X\n", DISK[i][j]);
        }
    }
}

void generate_regout(FILE* regout){
    for (int i = 2; i < 16; ++i) {
        fprintf(regout, "%05X\n", REG[i]);
    }
}

int main(){
    IOREG[13] = 0xffffffff; // timermax - the only value in IOREG that is not initialised to 0
    FILE *diskin, *irq2in, *memin, *memout, *regout, *trace, *hwregtrace, *cycles, *leds, *display7seg, *diskout, *monitor;
    memin = open_file("Outputs/memin.txt", "r");
    irq2in = open_file("Inputs/irq2in.txt", "r");
    diskin = open_file("Inputs/diskin.txt", "r");
    int MEM[4096] = {0}; // initialise a temporary array that holds the memory
    int MONITOR[256*256] = {0};
    int DISK[128][128] = {0};
    int *irq2 = store_irq2_in_array(irq2in); //this array contains all the irq2
    print_arr(irq2);

    create_temp_memory(memin, MEM);
    create_temp_diskout(diskin, DISK);

    trace = open_file("Outputs/trace.txt", "w");
    cycles = open_file("Outputs/cycles.txt", "w");
    leds = open_file("Outputs/leds.txt", "w");
    hwregtrace = open_file("Outputs/hwregtrace.txt", "w");
    monitor = open_file("Outputs/monitor.txt", "w");
    diskout = open_file("Outputs/diskout.txt", "w");
    regout = open_file("Outputs/regout.txt", "w");
    display7seg = open_file("Outputs/display7seg.txt", "w");

    simulator(MEM, MONITOR, DISK, irq2, trace, cycles, leds, irq2in, display7seg, hwregtrace);
    //print_memory(MEM);
    //print_disk(DISK);
    //print_monitor(MONITOR);

    // Generating output files
    memout = open_file("Outputs/memout.txt", "w");
    generate_memout(MEM, memout);
    generate_monitor(MONITOR, monitor);
    generate_diskout(DISK, diskout);
    generate_regout(regout);


    fclose(diskin);
    fclose(irq2in);
    fclose(memin);
    fclose(memout);
    fclose(regout);
    fclose(trace);
    fclose(hwregtrace);
    fclose(cycles);
    fclose(leds);
    fclose(display7seg);
    fclose(diskout);
    fclose(monitor);
    free(irq2);
    return 0;
}