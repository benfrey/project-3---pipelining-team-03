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

int signExtend(int num){
        // convert a 16-bit number into a 32-bit integer
        if (num & (1<<15) ) {
                num -= (1<<16);
        }
        return num;
}

void printInstruction(int instr){
	char opcodestring[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodestring, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodestring, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodestring, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodestring, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodestring, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodestring, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodestring, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodestring, "noop");
	} else {
		strcpy(opcodestring, "data");
	}

	if(opcode(instr) == ADD || opcode(instr) == NAND){
		printf("%s %d %d %d\n", opcodestring, field2(instr), field0(instr), field1(instr));
	} else if(0 == strcmp(opcodestring, "data")){
		printf("%s %d\n", opcodestring, signExtend(field2(instr)));
	} else{
		printf("%s %d %d %d\n", opcodestring, field0(instr), field1(instr), signExtend(field2(instr)));
	}
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

void print_stats(statetype* state){
	printf("total of %d cycles executed\n", state->cycles);
	printf("total of %d instructions fetched\n", state->fetched);
	printf("total of %d instructions retired\n", state->retired);
	printf("total of %d branches executed\n", state->branches);
	printf("total of %d branch mispredictions\n", state->mispreds);
}

int* checkDataHazard(statetype* state, statetype* newstate){
        // Decode fields from IFID buffer
        int regA = field0(state->IDEX.instr);
        int regB = field1(state->IDEX.instr);

	int *output;	// Int array for checkDataHazard output
	output = (int*)malloc(3);
	output[0] = 0;	// data to be written in regA
	output[1] = 0;	// data to be written in regB
	output[2] = 0;	// If 0, don't forward into regA or regB,
			// If 1, forward into regA
			// If 2, forward into regB
			// If 3, forward into regA and regB
			// If 4, load stall occurred
	int useA = 0;	// If 0, need to look at only regB
			// If 1, may need to look at regA

	// Instructions that may require looking at regA.
	if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND || opcode(state->IDEX.instr) == BEQ){
		useA = 1;
	}

	// Bypassing/Forwarding. Go from buffers furthest down the pipeline (WBEND) to
	// earliest in pipeline that require inspection (EXMEM).
	if(opcode(state->WBEND.instr) == ADD || opcode(state->WBEND.instr) == NAND){
		if(useA == 1 && regA == field2(state->WBEND.instr)){
			output[0] = state->WBEND.writedata;
			output[2] = output[2] | 2;
		}
		if(regB == field2(state->WBEND.instr)){
			output[1] = state->WBEND.writedata;
			output[2] = output[2] | 1;
		}
	}
        if(opcode(state->WBEND.instr) == LW){
                if(useA == 1 && regA == field0(state->WBEND.instr)){
                        output[0] = state->WBEND.writedata;
                        output[2] = output[2] | 2;
                }
                if(regB == field0(state->WBEND.instr)){
                        output[1] = state->WBEND.writedata;
                        output[2] = output[2] | 1;
                }
        }
        if(opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND){
                if(useA == 1 && regA == field2(state->MEMWB.instr)){
                        output[0] = state->MEMWB.writedata;
                        output[2] = output[2] | 2;
                }
                if(regB == field2(state->MEMWB.instr)){
                        output[1] = state->MEMWB.writedata;
                        output[2] = output[2] | 1;
                }
        }
        if(opcode(state->MEMWB.instr) == LW){
                if(useA == 1 && regA == field0(state->MEMWB.instr)){
                        output[0] = state->MEMWB.writedata;
                        output[2] = output[2] | 2;
                }
                if(regB == field0(state->MEMWB.instr)){
                        output[1] = state->MEMWB.writedata;
                        output[2] = output[2] | 1;
                }
        }
        if(opcode(state->EXMEM.instr) == ADD || opcode(state->EXMEM.instr) == NAND){
                if(useA == 1 && regA == field2(state->EXMEM.instr)){
                        output[0] = state->EXMEM.aluresult;
                        output[2] = output[2] | 2;
                }
                if(regB == field2(state->EXMEM.instr)){
                        output[1] = state->EXMEM.aluresult;
                        output[2] = output[2] | 1;
                }
        }

	// Load stall
	if(opcode(state->EXMEM.instr) == LW){
                // Check if regA will be revalued in EXMEM
                if(field1(state->IDEX.instr) == field0(state->EXMEM.instr) || (useA == 1 && field0(state->IDEX.instr) == field0(state->EXMEM.instr))){
                        // 1. "Pause" PC, fetched, and retired.
                        newstate->pc = state->pc;
                        newstate->fetched = state->fetched;
                        newstate->retired = state->retired;

                        // 2. "Pause" buffers through IDEX.
                        newstate->IFID.instr = state->IFID.instr;
                        newstate->IFID.pcplus1 = state->IFID.pcplus1;
                        newstate->IDEX.instr = state->IDEX.instr;
                        newstate->IDEX.pcplus1 = state->IDEX.pcplus1;
                        if(output[2]==2 || output[2] == 3){
				newstate->IDEX.readregA = output[0];
			}else{
				newstate->IDEX.readregA = state->IDEX.readregA;
			}
                        if(output[2]==1 || output[2] == 3){
				newstate->IDEX.readregB = output[1];
			}else{
				newstate->IDEX.readregB = state->IDEX.readregB;
			}
                        newstate->IDEX.offset = state->IDEX.offset;

                        // 3. Insert noop (BUBBLE) into EXMEM.
			newstate->EXMEM.instr = NOOPINSTRUCTION;
                        newstate->EXMEM.branchtarget = 0;
                        newstate->EXMEM.aluresult = 0;
                        newstate->EXMEM.readreg = 0;

			// Indicate load stall in output.
			output[2]=4;
		}
	}

        //printf("DATA HAZARD: %d %d %d\n",output[0],output[1],output[2]);
	return output;
}

void checkControlHazard(statetype* state, statetype* newstate){
	// Branch occurred

	// Need to modify currState to flush IFID, IDEX, EXMEM buffers
        newstate->IFID.instr = NOOPINSTRUCTION;
        newstate->IFID.pcplus1 = 0;
	newstate->IDEX.instr = NOOPINSTRUCTION;
	newstate->IDEX.pcplus1 = 0;
       	newstate->IDEX.readregA = 0;
        newstate->IDEX.readregB = 0;
        newstate->IDEX.offset = 0;
	newstate->EXMEM.instr = NOOPINSTRUCTION;
       	newstate->EXMEM.branchtarget = 0;
       	newstate->EXMEM.aluresult = 0;
       	newstate->EXMEM.readreg = 0;

	// Increment mispredictions and decrease retired (they are flushed, can't retire)
	newstate->mispreds = state->mispreds+1;
        newstate->retired = state->retired-2;

	// Fix fetched
	//if(signExtend(field2(state->EXMEM.instr)) < 0){
	//	newstate->fetched = state->fetched+1;
	//}
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

		// Copy state into the mutable newstate and increment cycles.
		*newstate = *state;
		newstate->cycles++;

		/*------------------ IF stage ----------------- */

                // Fetch new instruction. Also assume that instruciton
		// will eventually be retired (we correct assumption later)
		newstate->fetched = state->fetched+1;
		newstate->retired = state->retired+1;
                newstate->IFID.instr = state->instrmem[state->pc];

		// Increment PC
                newstate->pc = state->pc+1;
		newstate->IFID.pcplus1 = state->pc+1;

		/*------------------ ID stage ----------------- */

		// Decode fields from instr in IFID buffer
                newstate->IDEX.instr = state->IFID.instr;
                newstate->IDEX.pcplus1 = state->IFID.pcplus1;
                newstate->IDEX.readregA = state->reg[field0(state->IFID.instr)];
                newstate->IDEX.readregB = state->reg[field1(state->IFID.instr)];
                newstate->IDEX.offset = signExtend(field2(state->IFID.instr));

		/*------------------ EX stage ----------------- */

		// Check for data errors
		int *check = checkDataHazard(state, newstate);

	       	// Temporary parameters (not used outside of EX stage);
	       	int instr = state->IDEX.instr;
	        int dataA = state->IDEX.readregA;
	        int dataB = state->IDEX.readregB;
	        int offset = state->IDEX.offset;
	        int aluResult = 0;

		// Based result of checkDataHazard, forwarding may occur.
		if(check[2]==1 || check[2]==3){
			dataB=check[1];
		}
		if(check[2]==2 || check[2]==3){
			dataA=check[0];
		}

                //printf("CHECKS: %d %d %d\n", check[0],check[1],check[2]);
                //printf("!!!%d %d %d %d\n", opcode(instr), dataA, dataB, offset);
		//printf("%d", check);
		//printf("%d", state->MEMWB.writedata);

		// ADD
                if(opcode(instr) == ADD){
                       	aluResult = dataA + dataB;
	        }
                // NAND
                else if(opcode(instr) == NAND){
               	        aluResult = ~(dataA & dataB);
	        }
        	// LW or SW
              	else if(opcode(instr) == LW || opcode(instr) == SW){
                       	aluResult = dataB + offset;
		}
       	        // BEQ
               	else if(opcode(instr) == BEQ){
			aluResult = (dataA - dataB);
	        }

        	// Advance buffers if load stall did not occur.
	       	if(check[2] != 4){
			newstate->EXMEM.instr = instr;
		        newstate->EXMEM.branchtarget = state->IDEX.pcplus1 + offset;
			newstate->EXMEM.aluresult = aluResult;
			newstate->EXMEM.readreg = dataA;
		}

		/*------------------ MEM stage ----------------- */

		// Check if BEQ was in EXMEM buffer.
		if(opcode(state->EXMEM.instr) == BEQ){
			// Increase tot amount of BEQs executed (does not mean that branch was taken).
                        newstate->branches = state->branches+1;

			// If branch was taken.
			if(state->EXMEM.aluresult == 0){
				// Correct pc to point to desired branch target
                        	newstate->pc = state->EXMEM.branchtarget;

				//


				// Check for control errors. Prof. Myre, is this valid location to call method to
                        	// flush registers? We thought HDU had to be at start of mem stage for control hazs.
                        	checkControlHazard(state, newstate);
			}
		}

                // Determine writeData
                int writeData = 0;

		// R-Type
                if(opcode(state->EXMEM.instr) == ADD || opcode(state->EXMEM.instr) == NAND){
			writeData = state->EXMEM.aluresult;
		}
		// LW
                else if(opcode(state->EXMEM.instr) == LW){
                        writeData = state->datamem[state->EXMEM.aluresult];
                }
		// SW
		else if(opcode(state->EXMEM.instr) == SW){
                        newstate->datamem[state->EXMEM.aluresult] = state->EXMEM.readreg;
                }

		// Correct retired and fetched if we are going to HALT next cycle
		if(opcode(state->EXMEM.instr) == HALT){
			newstate->retired = state->retired-2;
			newstate->fetched = state->fetched-2;
		}

		printf("FETCHED: %d", newstate->fetched);

		// Advance buffers
		newstate->MEMWB.instr = state->EXMEM.instr;
		newstate->MEMWB.writedata = writeData;

		/*------------------ WB stage ----------------- */

		// Determine what should be stored in write data depending on instr
		writeData = 0;

                // Determine destReg from MEMWB and pull data from WBEND
                int destReg;

                // R-Type
                if(opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND){
                        // Get destReg from field2
                        destReg = field2(state->MEMWB.instr);
                        // Update writeData
			writeData = state->MEMWB.writedata;
			// Result into reg
                        newstate->reg[destReg] = writeData;
                }
		// LW
		else if(opcode(state->MEMWB.instr) == LW){
                        // Get destReg from field0
                        destReg = field0(state->MEMWB.instr);
                        // Update writeData
                        writeData = state->MEMWB.writedata;
                        // Result into reg
                        newstate->reg[destReg] = writeData;
                }

		// Advance buffers
		newstate->WBEND.instr = state->MEMWB.instr;
		newstate->WBEND.writedata = writeData;

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

	// Setup state and newstate structs. In run, state is copied into
	// newsate, so save some lines in init here.
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
		state->datamem[i] = atoi(line);
		i++;
	}
	fclose(fp);

	/** Run the simulation **/
	run(state, newstate);

	free(state);
	free(newstate);
	free(fname);
}
