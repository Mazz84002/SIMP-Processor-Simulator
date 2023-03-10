#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// --------------------------------------- Defining constants ------------------------------------

#define REGSIZE 16
#define MEMSIZE 4096
#define MAX_LINE_ASM 100
#define MAX_LINE_LENGTH 100
#define true 1
#define false 0
static int is_Rtype = 1; // 1 if the instruction is R-type

const char *REGNAMES[REGSIZE] = {
        "zero", "imm", "v0",
        "a0", "a1", "a2", "a3",
        "t0", "t1", "t2",
        "s0", "s1", "s2",
        "gp", "sp", "ra"
};
const char *OPCODES[] = {"add", "sub", "mul",
                         "and", "or", "xor",
                         "sll", "sra", "srl",
                         "beq", "bne", "blt", "bgt", "ble", "bge",
                         "jal", "lw", "sw", "reti",
                         "in", "out", "halt"
};

// ----------------------------------- Remove space from string -----------------------------

// removes all the \t, spaces, and all the comments from a line
char *remove_white_spaces(char *str){
    int i = 0, j = 0;
    while (str[i] != '#' && str[i] != '\0'){
        if (str[i] != ' ' && str[i] != '\t')
            str[j++] = str[i];
        i++;
    }
    str[j] = '\0';
    return str;
}
// converts 5 byte hexadecimal to integer with sign extension
int h2d(char line[]){
    if (line[0] == '8' || line[0] == '9' || line[0] == 'A' || 
        line[0] == 'B' || line[0] == 'C' || line[0] == 'D' || 
        line[0] == 'E'  | line[0] == 'F'){
        return (0xFFF00000 + strtol(line, (char **)NULL, 16));
    }

    return strtol(line, (char **)NULL, 16);
}

// --------------------------------------- Custom structure - labels ------------------------------------
// Using BST instead of array for dynamic memory allocation
// The arrangement will be according to the label-name
struct label{
    int label_loc;
    char label_name[100];
    struct label *left, *right;
};
// Function to create a new node
struct label* new_label(int label_loc, char label_name[]){
    struct label* temp
            = (struct label*)malloc(sizeof(struct label));
    temp->label_loc = label_loc;
    strcpy(temp->label_name, label_name);
    temp->left = temp->right = NULL;
    return temp;
}
// Function to add new element to the BST-label using the precedence in names
struct label* insert_label(struct label* label, int label_loc, char *label_name){
    if (label == NULL)
        return new_label(label_loc, label_name);
    if (strcmp(label_name, label->label_name) < 0)
        label->left = insert_label(label->left, label_loc, label_name);
    else if (strcmp(label_name, label->label_name) > 0)
        label->right = insert_label(label->right, label_loc, label_name);
    return label;
}
// finds a label_name in the BST
int label_loc(struct label* label, char *label_name){
    if (strcmp(label_name, label->label_name) < 0)
        return label_loc(label->left, label_name);
    if (strcmp(label_name, label->label_name) > 0)
        return label_loc(label->right, label_name);
    else
        return label->label_loc;
}

// free the memory from BST
void free_tree(struct label* root){
    if (root == NULL){
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// -------------------------------- TOKEN EXTRACTOR ----------------------------------

// define a structure that will store the opcode and registers in an instruction
struct instruction{
    char* op;
    char* rd;
    char* rs;
    char* rt;
    char* imm;
};

int has_label(char *token); // a function to check if a line has a label defined later
int c2i(char *str); // fucntion to convert charater at imm value to integer if it is a number

struct instruction inst_fetch(char line[], struct instruction inst){ // Extract all the op, regs and imm from instruction
    strcmp(line, remove_white_spaces(line));
    char* token = NULL;
    token = strtok(line, "$");
    inst.op = token;
    token = strtok(NULL, ",$");
    inst.rd = token;
    token = strtok(NULL, ",$");
    inst.rs = token;
    token = strtok(NULL, ",$");
    inst.rt = token;
    token = strtok(NULL, ",");
    inst.imm = token;
    return inst;
}

// ------------------------------------ OPCODE -----------------------------------------------
int op_num(char *token){ // Gives the number of given opcode
    int i = -1;
    for (i = 0; i < 22; i++) {
        if (strstr(token, OPCODES[i]) != NULL)
            return i;
    }
    return -1; // If not a valid token
}
int op_mask(int hex, char *token){// masks the hex bit according to given opcode
    int i = op_num(token);
    if ((token!=NULL) && i!=-1){
        hex = hex + i*(4096);
    }
    return hex;
}
// ------------------------------------ REGISTERS ---------------------------------------------
int reg_num(char *token){ // Gives the number of given REGISTER
    int i = -1;
    for (i = 0; i < 16; i++) {
        if (strstr(token, REGNAMES[i]) != NULL)
            return i;
    }
    return -1; // if token is not in official reg list
}


int reg_mask(int hex, char*token, int num){ // creates a bitmask for the registers
    int i = reg_num(token);
    if (num == 1){
        if ((token != NULL) && (i!=-1)) {// Check if we indeed have a string to compare to
            hex = hex + i*(256);
            return hex;
        }
    }
    if (num == 2){
        if ((token != NULL) && (i!=-1)) {// Check if we indeed have a string to compare to
            hex = hex + i*(16);
            return hex;
        }
    }
    if (num == 3){
        if ((token != NULL) && (i!=-1)) {// Check if we indeed have a string to compare to
            hex = hex + i*(1);
            return hex;
        }
    }

    return 0;
}

// ------------------------------------- LABELS --------------------------------------

int has_label(char *token){ // returns 1 if we have a label and 0 otherwise
    int i = 0;
    while ((token[i] != '\0') && (token[i] != '#')){
        if ((token[i] >= 'a' && token[i] <= 'z') || (token[i] >= 'A' && token[i] <= 'Z'))
            return 1;
        i++;
    }
    return 0;
}

int enter_label_if_found(char line[], struct label* lroot, int pc){ // enters label into BST and returns the label_counter_increment
    if (strstr(line, ":") != NULL) {
        strcmp(line, strtok(line, ":"));
        lroot = insert_label(lroot, pc, line);
        return 1;
    }
    return 0;
}



// --------------------------- IMM FIELD -------------------------------------

int c2i(char *str){ // converts the last token into integer
    int sign = 1, number = 0, index = 0;
    while((str[index] != '\0')){
        if (str[index] == '#'){
            break;
        }
        if(str[index] == '-'){
            sign = -1;
            //index = 1;
        }
        if(str[index] >= '0' && str[index] <= '9'){
            number = number*10 + str[index] - '0';
        }
        index++;
    }
    number = number * sign;
    return number;
}

// -------------------------------- WORD -----------------------------------

int has_word(char line[]){ // checks if a line has .word
    if (strstr(".word", line)){
        return 1;
    }
    return 0;
}

// --------------------------- ENCODED INSTRUCTION ------------------------------
// holds the conoded instruction, where each part is converted to the corresponsing integer
struct encoded_instruction{
    int op;
    int rd;
    int rs;
    int rt;
    int imm;
};

// Converts all the instrcution to theor corresponding numbers
struct encoded_instruction encode_inst(struct instruction inst, struct encoded_instruction enc_inst, struct label* lroot){
    enc_inst.op = op_num(inst.op);
    enc_inst.rd = reg_num(inst.rd);
    enc_inst.rs = reg_num(inst.rs);
    enc_inst.rt = reg_num(inst.rt);
    if (has_label(inst.imm)){
        enc_inst.imm = label_loc(lroot, inst.imm);
    }else{
        enc_inst.imm = c2i(inst.imm);
    }
    return enc_inst;
}

int is_itype(struct encoded_instruction enc_inst){ // returns 1 for i_type inst, 0 else
    return ( (enc_inst.rt==1)||(enc_inst.rs==1)||(enc_inst.rd==1) );
}

int generte_machine_code(struct encoded_instruction enc_inst){ // gives back the machine code for encoded instruction
    int hex = 0;
    hex = enc_inst.op*4096 + enc_inst.rd*256 + enc_inst.rs*16 + enc_inst.rt*1;
    return hex;
}

// --------------------------------------- Assembler -------------------------------------------

/*
 * In First pass
 *  We extract all the labels - finding their address
 * In Second pass
 *  We go through all the lines again, but this time we find labels in each line, and replace
 *  them by their address
 *  We bit mask all the opcodes
 *  We bit mask all the registers (after checking I-Type/ R-Type)
 *  Finally, we also write bits into `memin.txt` in the second pass (for each line)
 */

void assembler(int MEM[], int pc, int label_counter, FILE* asmfile, FILE* memin){
    char line[100]; //  This will store the contents of the line
    char *token = NULL; // to store the tokens from a line
    struct instruction inst;
    struct encoded_instruction enc_inst;
    int increment_label = 0;
    static struct label* lroot = NULL;
    // ---------------------- FIRST PASS ----------------------
    while (fgets(line, MAX_LINE_ASM, asmfile)){
        strcmp(line, remove_white_spaces(line));
        if (strstr(line, ".word") == NULL){
            if (strstr(line, ":") != NULL) {
                strcmp(line, strtok(line, ":"));
                lroot = insert_label(lroot, pc, line);
                label_counter++;
            }else{ // we did not have a label, then update pc
                pc++;
                inst = inst_fetch(line, inst);
                pc += (reg_num(inst.rd)==1 || reg_num(inst.rs)==1 || reg_num(inst.rt)==1);
            }
        }
    }
    fseek(asmfile, 0, SEEK_SET);
    pc = 0;
    int line_counter = 0;
    int memory_loc = 0;
    int memory_val = 0;
    char* mem_loc= NULL;
    char* mem_value = NULL;
    // ---------------------- SECOND PASS ----------------------
    while (fgets(line, MAX_LINE_ASM, asmfile)){
        if (strstr(line, ".word") != NULL){
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            if (strstr(token, "x")){ // hex
                memory_loc = h2d(token);
            }else{
                memory_loc = c2i(token);
            }
            token = strtok(NULL, " ");
            if (strstr(token, "x")){ // hex
                memory_val = h2d(token);
            }else{
                memory_val = c2i(token);
            }
            MEM[memory_loc] = memory_val;
            continue;
        }
        strcmp(line, remove_white_spaces(line));
        if (strstr(line, ":") != NULL){ // We have a label
            pc++; // increase counter by one more to skip the row
            continue;
        }else{
            inst = inst_fetch(line, inst);
            enc_inst = encode_inst(inst, enc_inst, lroot);
            MEM[line_counter] = generte_machine_code(enc_inst);
            line_counter++;
            if (is_itype(enc_inst)){
                MEM[line_counter] = enc_inst.imm;
                line_counter++;
            }
        }
    }
    free_tree(lroot);
}

FILE* open_file(char name[], char mode[]){ // loads a file
    FILE *f = fopen(name, mode);
    if (f == NULL){
        printf("Unable to open file\n");
        exit(1);
    }
    return f;
}

int last_non_zero_element(int A[], int size){ // takes in an array and returns the position of last non-zero element
    int pos = size-1;
    while (A[pos] == 0){
        pos--;
    }
    return pos;
}

void print_to_memin(int MEM[], FILE* memin){ // write to memin from MEM[] at the end
    int size = last_non_zero_element(MEM, 4096);
    for (int i = 0; i <= size; ++i) {
        fprintf(memin, "%05X\n", MEM[i] & 0x000FFFFF);
    }
}

int main(int argc, char *argv[]){
    FILE* asmfile, *memin;
    char path[500];
    asmfile = open_file(argv[1], "r");
    memin = open_file(argv[2], "w");
    int pc = 0;
    int label_counter = 0;
    int MEM[4096]={0};
    assembler(MEM, pc, label_counter, asmfile, memin);
    print_to_memin(MEM, memin);
    return 0;
}