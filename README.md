# UST-3400 Rip Van saWinkle Simulator

Ben Frey and Joe Lambrecht <br />
CISC340 - Professor Myre <br />
Project 3 - Pipelined Implementation

## Rip Van saWinkle (Third Generation UST-3400)
The Rip Van saWinkle architecture implements a pipelined CPU design. A pipelined CPU implementation segments the data path into distinct stages that allows for multiple instructions to be in flight simultaneously. Staging of the data path has an immense comparative advantage to non-pipelined implementations in terms of performance at the cost of added complexity handled by our hazard detection systems outlined in the simulator documentation.

| Stage | Instruction Fetch | Instruction Decode | Execute | Memory | Write-back | End |
| ----- | ----------------- | ------------------ | --------| ------ | ---------- | --- |
| Function |  Pull instruction from memory and update PC  |  Decode instruction word and read from register file | Carry out instruction | Load and store information from memory |  Write information back to register file  | Allows for easier data forwarding across buffers |

## UST-3400 Functional Overview
!Replace with pipelined function!

First the file input lines are swept to store the program into UST-3400 memory. The pipeline buffers: IF/ID, ID/EX, EX/MEM, MEM/WB, WB/END between each stage are initialized to contain noop instructions and parameter fields of 0.

Second, the program is started at address 0 in memory. Depending on the program counter, an instruction is evaluated and the UST-3400 CPU orchestrates flight of instructions across the discrete stages. The program is run until a halt instruction is stored in the MEM/WB buffer.

## Running a Program
Once the program .zip has been decompressed, in the simulator directory simply run:<br />
<br />
$ make<br />
$ ./sim -i {inputfile.mc}<br />
<br />

## Test Suite Descriptions
| Assembly File (Machine Code)          | Description |
| ------------- | :---------------------|
| addForwardTest.asm (addForwardTest.mc) | Presents a data hazard that requires proper implementation of a multi-buffer selection data forwarding. |
| beqFetchTest.asm (beqFetchTest.mc) | Tests proper accounting of fetched instructions for multiple control hazards. |
| class.asm (class.mc) | Example of a while loop presented in class. Presents multiple data and control hazards. |
| classNoHazard.asm (classNoHazard.mc) | Similar to class.asm except has noop instructions between each instruction to prevent hazards. Minimum noop between each instruction was not calculated, as 4 will be sufficient for any hazard. |
| lwStallTest.asm (lwStallTest.mc) | Tests data hazards associated with multiple load word instructions contiguously executed. Observable load stall (bubble) insertion. |
| mult.asm (mult.mc) | Working example of fast multiplication using multiplication procedure outline on pg. 185 of Computer Organization and Design - Patterson and Hennessy. Presents multiple hazards that prove to be a robust test case for a pipelined CPU. |
| sample1.asm (sample1.mc) | Sample program provided with project specifications. Used to check proper initialization of pipelined CPU simulator. |

## Known Issues
- Fetch instruction accounting anomaly with branches taken.
