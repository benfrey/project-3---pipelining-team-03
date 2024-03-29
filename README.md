# UST-3400 Rip Van saWinkle Simulator

Ben Frey and Joe Lambrecht <br />
CISC340 - Professor Myre <br />
Project 3 - Pipelined Implementation

## Project Overview
Project Assignment Outline PDF: [project_assignment.pdf](docs/project_assignment.pdf) </br>
Simulator Documentation PDF: [simulator_documentation.pdf](docs/simulator_documentation.pdf) </br>
</br>
This project relates to my CISC 480: Senior Capstone course because it was the most difficult assignment I worked on during my Computer Science curriculum at the University of St. Thomas. While my partner and I were successfully able to complete all project components, the Rip Van saWinkle Simulator had dozens of intricate design considerations that required implementation to pass all of the validation tests.
</br></br>
NOTE: All code in this repository has been cleaned up since the completion of this course to be featured in my senior portfolio. Additional documentation has been provided under "/docs" for future usage.

## Project Description
**Rip Van saWinkle (Third Generation UST-3400)**</br>
The Rip Van saWinkle architecture implements a pipelined CPU design. A pipelined CPU implementation segments the data path into distinct stages that allows for multiple instructions to be in flight simultaneously. Staging of the data path has an immense comparative advantage to non-pipelined implementations in terms of performance at the cost of added complexity handled by our hazard detection systems outlined in the simulator documentation.

| Stage | Instruction Fetch | Instruction Decode | Execute | Memory | Write-back | End |
| ----- | ----------------- | ------------------ | --------| ------ | ---------- | --- |
| Function |  Pull instruction from memory and update PC  |  Decode instruction word and read from register file | Carry out instruction | Load and store information from memory |  Write information back to register file  | Allows for easier data forwarding across buffers |

## UST-3400 Pipelined Datapath
![UST-3400 Pipelined Datapath Diagram](docs/datapath.png)

## UST-3400 Functional Overview
First the file input lines are swept to store the program into UST-3400 memory. The pipeline buffers: IF/ID, ID/EX, EX/MEM, MEM/WB, WB/END between each stage are initialized to contain noop instructions and parameter fields of 0.

Second, the program is started at address 0 in memory. Depending on the program counter, an instruction is evaluated and the UST-3400 CPU orchestrates flight of instructions across the discrete stages. The program is run until a halt instruction is stored in the MEM/WB buffer.

## Running a Program
Once the program .zip has been decompressed, in the simulator directory simply run:<br />
<br />
$ make<br />
$ ./sim -i {inputfile.mc}<br />
<br />
To clean up the repository, run: </br>
$ make clean </br>
</br>
Simulator execution of class.mc (see [class.log](docs/class.log) for full output)</br>
![Simulator execution of class.mc](docs/class_example.png)

## Test Suite Descriptions
| Assembly File (Machine Code)          | Description |
| ------------- | :---------------------|
| addForwardTest.asm (addForwardTest.mc) | Presents a data hazard that requires proper implementation of a multi-buffer selection data forwarding. |
| beqFetchTest.asm (beqFetchTest.mc) | Tests proper accounting of fetched instructions for multiple control hazards. |
| beqHalt.asm (beqHalt.mc) | Tests scenario when BEQ is followed by a HALT, special considertations required when accounting for fetches. |
| class.asm (class.mc) | Example of a while loop presented in class. Presents multiple data and control hazards. |
| classNoHazard.asm (classNoHazard.mc) | Similar to class.asm except has noop instructions between each instruction to prevent hazards. Minimum noop between each instruction was not calculated, as 4 will be sufficient for any hazard. |
| lwStallTest.asm (lwStallTest.mc) | Tests data hazards associated with multiple load word instructions contiguously executed. Observable load stall (bubble) insertion. |
| multiDataTest.asm (multiDataTest.mc) | Robust test of nand, lw, and add instruction sequence that will result in a complex data hazard requiring forwarding across multiple buffers. |
| sample1.asm (sample1.mc) | Sample program provided with project specifications. Used to check proper initialization of pipelined CPU simulator. |

## Known Issues
- One test case in the autograder (Multihazard #3) in which the cycle count for our simulator was incorrect. We were unable to replicate this in any test case and currently consider it an unidentified corner case.

