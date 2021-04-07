#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR – not implemented in this project */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{
	int instr;
	int writedata;
} WBENDType;

typedef struct statestruct{
	int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles;       /* Number of cycles run so far */
	int fetched;     /* Total number of instructions fetched */
	int retired;      /* Total number of completed instructions */
	int branches;  /* Total number of branches executed */
	int mispreds;  /* Number of branch mispredictions*/
} statetype;

int field0(int instruction){
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){
	return(instruction & 0xFFFF);
}

int opcode(int instruction){
	return(instruction>>22);
}

void printInstruction(int instr){
	char opcodeString[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodeString, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodeString, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodeString, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodeString, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodeString, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodeString, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodeString, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodeString, "noop");
	} else {
		strcpy(opcodeString, "data");
	}

	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
			field2(instr));
}

void printstate(statetype* stateptr){
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
    printf("\tpc %d\n", stateptr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<stateptr->numMemory; i++) {
	    printf("\t\tdatamem[ %d ] %d\n", i, stateptr->datamem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
	}
    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->IFID.instr);
	printf("\t\tpcplus1 %d\n", stateptr->IFID.pcplus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->IDEX.instr);
	printf("\t\tpcplus1 %d\n", stateptr->IDEX.pcplus1);
	printf("\t\treadregA %d\n", stateptr->IDEX.readregA);
	printf("\t\treadregB %d\n", stateptr->IDEX.readregB);
	printf("\t\toffset %d\n", stateptr->IDEX.offset);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->EXMEM.instr);
	printf("\t\tbranchtarget %d\n", stateptr->EXMEM.branchtarget);
	printf("\t\taluresult %d\n", stateptr->EXMEM.aluresult);
	printf("\t\treadreg %d\n", stateptr->EXMEM.readreg);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->MEMWB.instr);
	printf("\t\twritedata %d\n", stateptr->MEMWB.writedata);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->WBEND.instr);
	printf("\t\twritedata %d\n", stateptr->WBEND.writedata);
}

int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

void print_stats(statetype* state){
	printf("total of %d cycles executed\n", state->cycles);
	printf("total of %d instructions fetched\n", state->fetched);
	printf("total of %d instructions retired\n", state->retired);
	printf("total of %d branches executed\n", state->branches);
	printf("total of %d branch mispredictions\n", state->mispreds);
}

void run(statetype* state, statetype* newstate){

	// Initialize buffers
	IFIDType* IFID = (IFIDType*)malloc(sizeof(IFIDType));
	IFID->instr = NOOPINSTRUCTION;
	IFID->pcplus1 = 0;
	state->IFID = *IFID;

	IDEXType* IDEX = (IDEXType*)malloc(sizeof(IDEXType));
	IDEX->instr = NOOPINSTRUCTION;
	IDEX->pcplus1 = 0;
	IDEX->readregA = 0;
	IDEX->readregB = 0;
	IDEX->offset = 0;
	state->IDEX = *IDEX;

	EXMEMType* EXMEM = (EXMEMType*)malloc(sizeof(EXMEMType));
	EXMEM->instr = NOOPINSTRUCTION;
	EXMEM->branchtarget = 0;
	EXMEM->aluresult = 0;
	EXMEM->readreg = 0;
	state->EXMEM = *EXMEM;

	MEMWBType* MEMWB = (MEMWBType*)malloc(sizeof(MEMWBType));
	MEMWB->instr = NOOPINSTRUCTION;
	MEMWB->writedata = 0;
	state->MEMWB = *MEMWB;

	WBENDType* WBEND = (WBENDType*)malloc(sizeof(WBENDType));
	WBEND->instr = NOOPINSTRUCTION;
	WBEND->writedata = 0;
	state->WBEND = *WBEND;

	// Primary loop
	while(1){
		printstate(state);

		/* check for halt */
		if(HALT == opcode(state->MEMWB.instr)) {
			printf("machine halted\n");
			print_stats(state);
			break;
		}
		*newstate = *state;
		newstate->cycles++;

		/*------------------ IF stage ----------------- */

		// Fetch new instructions
		newstate->fetched++;

		newstate->IFID.instr = state->instrmem[state->pc];
		newstate->IDEX.pcplus1 = (state->pc)+1;

		/*------------------ ID stage ----------------- */

                newstate->IDEX.instr = state->IFID.instr;
                newstate->IDEX.pcplus1 = 99;    // Not sure...
                newstate->IDEX.readregA = field0(state->IFID.instr);
                newstate->IDEX.readregB = field1(state->IFID.instr);
                newstate->IDEX.offset = field2(state->IFID.instr);

		/*------------------ EX stage ----------------- */

		newstate->EXMEM.instr = state->IDEX.instr;
		newstate->EXMEM.branchtarget = 99; // Not sure...
		newstate->EXMEM.aluresult = 99; // Perform alu operation here...
		newstate->EXMEM.readreg = 99; // Not sure...

		/*------------------ MEM stage ----------------- */

		newstate->MEMWB.instr = state->IDEX.instr;
		newstate->MEMWB.writedata = 99; // Not sure...

		/*------------------ WB stage ----------------- */

		newstate->WBEND.instr = state->MEMWB.instr;
		newstate->WBEND.writedata = 99; // Not sure...

		*state = *newstate; 	/* this is the last statement before the end of the loop. 
					It marks the end of the cycle and updates the current
					state with the values calculated in this cycle
					– AKA “Clock Tick”. */
	}

}

int main(int argc, char** argv){

	/** Get command line arguments **/
	char* fname;

	opterr = 0;

	int cin = 0;

	while((cin = getopt(argc, argv, "i:")) != -1){
		switch(cin)
		{
			case 'i':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				break;
			case '?':
				if(optopt == 'i'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option `-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				abort();
		}
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
	while (EOF != (c=getc(fp))) {
		if ( c == '\n' ){
			line_count++;
		}
	}
	// reset fp to the beginning of the file
	rewind(fp);

	statetype* state = (statetype*)malloc(sizeof(statetype));
	state->pc = 0;
	memset(state->instrmem, 0, NUMMEMORY*sizeof(int));
        memset(state->datamem, 0, NUMMEMORY*sizeof(int));
	memset(state->reg, 0, NUMREGS*sizeof(int));
        state->numMemory = line_count;

	statetype* newstate = (statetype*)malloc(sizeof(statetype));


	char line[256];

	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
		/* note that fgets doesn't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		state->instrmem[i] = atoi(line);
		i++;
	}
	fclose(fp);

	/** Run the simulation **/
	run(state, newstate);

	free(state);
	free(newstate);
	free(fname);

}
