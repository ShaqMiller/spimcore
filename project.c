#include "spimcore.h"
// I have not used C language code obtained from other students, the Internet, and any other unauthorized sources, either modified or unmodified. If any code in my program was obtained from an authorized source, such as textbook or course notes, that has been clearly noted as a citation in the comments of the program.
// Shaquille Miller
// sh110063@ucf.edu


/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    if (ALUControl == 0)
        *ALUresult = A + B;
    else if (ALUControl == 1)
        *ALUresult = A - B;
    else if (ALUControl == 2)
        *ALUresult = (int) A < (int) B ? 1 : 0;
    else if (ALUControl == 3)
        *ALUresult = A < B ? 1 : 0;
    else if (ALUControl == 4)
        *ALUresult = A & B;
    else if (ALUControl == 5)
        *ALUresult = A | B;
    else if (ALUControl == 6)
        *ALUresult = B << 16;
    else if (ALUControl == 7)
        *ALUresult = ~A;

    *Zero = *ALUresult == 0 ? 1 : 0;
    return;
}

/* Instruction Fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    //first i check word alignedness since this is 32bits
    if( PC % 4 == 0 ) 		
        //Extracts the instruction by shifting right	
		*instruction = Mem[PC/4]; //divide by 4 to get index
	else{
        //not aligned so return 1
        return 1; 	
    }							
    
	return 0;
}

/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op     = (instruction & 0xfc000000) >> 26; // 11111100 00000000 00000000 00000000  
	*r1     = (instruction & 0x03e00000) >> 21; // 00000011 11100000 00000000 00000000
	*r2     = (instruction & 0x001f0000) >> 16; // 00000000 00011111 00000000 00000000
	*r3     = (instruction & 0x0000f800) >> 11; // 00000000 00000000 11111000 00000000
	*funct  =  instruction & 0x0000003f; 		// 00000000 00000000 00000000 00111111
	*offset =  instruction & 0x0000ffff; 		// 00000000 00000000 11111111 11111111
	*jsec   =  instruction & 0x03ffffff; 		// 00000011 11111111 11111111 11111111

}

/* Instruction Decode */
/* 30 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
switch(op){
            
		//Struct instantiation pass format
		// (struct name){RegDst,Jump,Branch,MemRead,MemtoReg,ALUOp,MemWrite,ALUSrc,RegWrite}

		case 0x0: // add,sub, and, or, slt, sltu
		 	*controls = (struct_controls) {1,0,0,0,0,7,0,0,1};
			break;
            
		case 0x8: // addi
			*controls = (struct_controls) {0,0,0,0,0,0,0,1,1};
			break;
            
		case 0x23: // lw
			*controls = (struct_controls) {0,0,0,1,1,0,0,1,1};
			break;
            
		case 0x2b: // sw
			*controls = (struct_controls) {0,0,0,0,0,0,1,1,0};
			break;
            
		case 0xf:  // lui
			*controls = (struct_controls) {0,0,0,0,0,6,0,1,1};
			break;
            
		case 0x4:  // beq
			*controls = (struct_controls) {2,0,1,0,2,1,0,2,0};
			break;
            
		case 0xa:  // slti
			*controls = (struct_controls) {0,0,0,0,0,2,0,1,1};
			break;
            
		case 0xb:  // sltiu
			*controls = (struct_controls) {0,0,0,0,0,3,0,1,1};
			break;
            
		case 0x2:  // j
			*controls = (struct_controls) {2,1,2,0,2,0,0,2,0};
			break;
            
		default:
			return 1;
	}
    
	return 0;   
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];	//Extract content from register in address r1
	*data2 = Reg[r2];	//Extract content from register in address r2   
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    //shifts right by 15 so i can get the signbit which is the 16th
    if((offset >> 15) == 1){
        //negative so then make the last 16 bits 1s so it keeps neg value
        //doing an | (or) will set those to 1					
		*extended_value = offset | 0xffff0000;	
    }
	else{
        //If it is postitive then make the last 16 bits to maintain positive value
        //using the & would make the last 4 bit mainatin their state  but turn off the others
		*extended_value = offset & 0x0000ffff;	
    }
}

/* ALU Operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	// Handle second parameter to ALU
    unsigned B = ALUSrc == 1 ? extended_value : data2;

    // Handle ALU control unit
    if (ALUOp == 7) {
        // Add
        if (funct == 0b100000)
            ALUOp = 0;
        // Subtract
        else if (funct == 0b100010)
            ALUOp = 1;
        // And
        else if (funct == 0b100100)
            ALUOp = 4;
        // Or
        else if (funct == 0b100101)
            ALUOp = 5;
        // Slt
        else if (funct == 0b101010)
            ALUOp = 2;
        // Sltu
        else if (funct == 0b101011)
            ALUOp = 3;
        // Unknown funct code
        else
            return 1;
    }

    ALU(data1, B, ALUOp, ALUresult, Zero);
    return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    //Ensure  its aligned and does not exceed 64kb 
    if(ALUresult % 4 == 0 && ALUresult < 65536){

        if(MemRead  == 1){
            *memdata = Mem[ALUresult >> 2];
        }

        if(MemWrite == 1){
            Mem[ALUresult >> 2] = data2;  
        }
    }else{
        return 1;
    }

	return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    //If info is being written to reg
    if(RegWrite == 1){					
        //If its from memmory to register
		if(MemtoReg == 1){				
			Reg[r2] = memdata;
		}								
		//the info is coming from the register insetad
		else if(MemtoReg == 0){		
            //If RegDst is 1 the instruction is R-type. if not then its an I-Type 
			if(RegDst == 1){
                //R-Type so we write to register r3
				Reg[r3] = ALUresult;
            }				
			else{
                //I-Type so we write to register r2
				Reg[r2] = ALUresult;
            }				
            
		}
        
	}
}

/* PC Update */
/* 15 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Increment the program counter by 4 to prepare for the next instruction
    *PC += 4;
    
    // Check if the Branch condition is met and the Zero flag is set
    if (Zero == 1 && Branch == 1) {
        // If both conditions are true, update the program counter using the branch offset
        // The branch offset is left-shifted by 2 to convert from words to bytes
        *PC = *PC + (extended_value << 2);
    }
    
    // Check if the Jump condition is met
    if (Jump == 1) {
        // If the Jump condition is true, update the program counter using the jump target address
        // The upper 28 bits of the program counter are preserved, while the lower 2 bits are set to 0
        *PC = (*PC & 0xF0000000) | (jsec << 2);
    }
}
