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
                // Throw error for jalr? After playing with provided ref_sim, just do nothing...

		// Copy state into the mutable newstate and increment cycles.
		*newstate = *state;

		/*------------------ IF stage ----------------- */

		// Fetch new instruction
		newstate->fetched++;
		newstate->IFID.instr = state->instrmem[state->pc];
		newstate->IFID.pcplus1 = (state->pc)+1;

		/*------------------ ID stage ----------------- */

		// Decode fields from instr
		int regA = field0(state->IFID.instr);
		int regB = field1(state->IFID.instr);
		int offset = signExtend(field2(state->IFID.instr));

		// Determine destReg from MEMWB and pull data from WBEND
		int destReg;

                // R-Type
                if(opcode(state->IFID.instr) == ADD || opcode(state->IFID.instr) == NAND){
                        // Get destReg from field2
                        destReg = field2(state->MEMWB.instr)
                        // Result into reg
                        newstate->reg[destReg] = state->WBEND.writeData;
                }
                // LW
                else if(opcode(instr) == LW || opcode(instr) == SW){
                        // Get destReg from field0
                        destReg = field0(state->MEMWB.instr)
                        // Result into reg
                        newstate->reg[destReg] = state->WBEND.writeData;
                }

                newstate->IDEX.instr = state->IFID.instr;
                newstate->IDEX.pcplus1 = state->IFID.pcplus1;
                newstate->IDEX.readregA = state->reg[regA];
                newstate->IDEX.readregB = state->reg[regB];
                newstate->IDEX.offset = offset;

		/*------------------ EX stage ----------------- */

		// Check for data errors


		// ALU operation

	        // Reused variables;
        	int instr = newstate->IDEX.instr;
	        int regA = newstate->IDEX.readregA;
	        int regB = newstate->IDEX.readregB;
        	int offset = newstate->IDEX.offset;
	        int branchTarget = newstate->pc+1; // Assume branch not taken, go to next line.
	        int aluResult = 0;

		// ADD
                if(opcode(instr) == ADD){
                        // Add
                        aluResult = regA + regB;
                        // Save result
                        newstate->reg[field2(instr)] = aluResult;
                }
                // NAND
                else if(opcode(instr) == NAND){
                        // NAND
                        aluResult = ~(regA & regB);
                        // Save result
                        newstate->reg[field2(instr)] = aluResult;
                }
                // LW or SW
                else if(opcode(instr) == LW || opcode(instr) == SW){
                        // Calculate memory address
                        aluResult = regB + offset;
                        if(opcode(instr) == LW){
                                // Load
                                newstate->reg[field0(instr)] = newstate->mem[aluResult];
                        }else if(opcode(instr) == SW){
                                // Store
                                newstate->mem[aluResult] = regA;
                        }
                }
                // BEQ
                else if(opcode(instr) == BEQ){
                        // Calculate condition
                        aluResult = (regA - regB);

			// Increment branches executed;
			newstate->branches++;

                        // ZD
                        if(aluResult==0){
                                // Branch taken, will be caught as control error
                                branchTarget = state->IDEX.pcplus1 + state->IDEX.offset;
                        }
                }
                // NOOP
                else if(opcode(instr) == NOOP){
			// Do nothing...
                }

                // Advance buffers
                newstate->EXMEM.instr = state->IDEX.instr;
                newstate->EXMEM.branchtarget = branchTarget;
		newstate->EXMEM.aluresult = aluresult;
		newstate->EXMEM.readreg = newstate->reg[regA];

		/*------------------ MEM stage ----------------- */

		// Check for control errors
		// 1. Assume branch not taken, otherwise will have to flush.

		// Determine writeData
		int writeData = 0;

		// R-Type
                if(opcode(state->EXMEM.instr) == ADD || opcode(state->EXMEM.instr) == NAND){
			writeData = state->EXMEM.aluresult;
		}
		// LW or SW
                else if(opcode(state->EXMEM.instr) == LW){
                        // Load
                        writeData = state->datamem[state->EXMEM.aluresult];
                }else if(opcode(state->EXMEM.instr) == SW){
                        // Store
                        newstate->datamem[state->EXMEM.aluResult] = state->EXMEM.readreg;
                }

		// Advance buffers
		newstate->MEMWB.instr = state->IDEX.instr;
		newstate->MEMWB.writedata = writeData;

		/*------------------ WB stage ----------------- */

		// Determine what should be stored in write data depending on instr
		int writeData = 0;

		// R-Type
                if(opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND){
                        // Want to store basic ALU result
                        writeData = state->MEMWB.writedata;
                }
                // LW
                else if(opcode(state->MEMWB.instr) == LW){
                        // Want to store result read from datamem
                        writeData = state->MEMWB.writedata;
                }
		// SW or BEW
		else if(opcode(state->MEMWB.instr) == SW || opcode(state->MEMWB.instr) == BEQ){
			// Want to store ALU result
			writeData = state->MEMWB.writedata;
		}

		// Advance buffers
		newstate->WBEND.instr = state->MEMWB.instr;
		newstate->WBEND.writedata = writeData; // Not sure...

                newstate->retired++;
                newstate->cycles++;

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
        statetype* newstate = (statetype*)malloc(sizeof(statetype));

	state->pc = 0;
	memset(state->instrmem, 0, NUMMEMORY*sizeof(int));
        memset(state->datamem, 0, NUMMEMORY*sizeof(int));
	memset(state->reg, 0, NUMREGS*sizeof(int));
        state->numMemory = line_count;

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
