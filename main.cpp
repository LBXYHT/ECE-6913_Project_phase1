#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

const int MemSize = 1000;
/*
R-TYPE (0-6: funct7, 7-11: rs2, 12-16: rs1, 17-19: funct3, 20-24: rd, 25-31: opcode)
I-TYPE (0-11: imme, 12-16: rs1, 17-19: funct3, 20-24: rd, 25-31: opcode)

*/

class InsMem {
private:
    vector<bitset<8> > IMem;

public:
    string id, ioDir;
    InsMem(string name, string ioDir) {
        id = name;
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open(ioDir + "\\imem.txt");
        if (imem.is_open()) {
            //cout << "open successfully" << endl; 
            while (getline(imem, line)) {
                if (imem.peek() != EOF) line.pop_back();
                IMem[i] = bitset<8>(line);
                i++;
            }
        } else {
            cout << "Unable to open IMEM input file.";
        }
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress) {
        //read instruction memory
        string instruction = "";
        instruction += IMem[ReadAddress.to_ulong()].to_string();
        instruction += IMem[ReadAddress.to_ulong() + 1].to_string();
        instruction += IMem[ReadAddress.to_ulong() + 2].to_string();
        instruction += IMem[ReadAddress.to_ulong() + 3].to_string();
        return bitset<32>(instruction);
    }
};

class DataMem {
private:
    vector<bitset<8> > DMem;
public:
    string id, opFilePath, ioDir;
    DataMem(string name, string ioDir) : id(name), ioDir(ioDir) {
        DMem.resize(MemSize);
        opFilePath = ioDir + "\\" + name + "_DMEMResult.txt";
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open(ioDir + "\\dmem.txt");
        if (dmem.is_open()) {
            while (getline(dmem, line)) {
                if (dmem.peek() != EOF) line.pop_back();
                DMem[i] = bitset<8>(line);
                i++;
            }
        } else {
            cout << "Unable to open DMEM input file.";
        }
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address) {
        //read data memory
        string data = "";
        data += DMem[Address.to_ulong()].to_string();
        data += DMem[Address.to_ulong() + 1].to_string();
        data += DMem[Address.to_ulong() + 2].to_string();
        data += DMem[Address.to_ulong() + 3].to_string();
        return bitset<32>(data);
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
        //write into memory
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
    }

    void outputDataMem() {
        ofstream dmemout;
        dmemout.open(opFilePath, ios_base::trunc);
        if (dmemout.is_open()) {
            for (int j = 0; j < 1000; j++) {
                dmemout << DMem[j] << endl;
            }
        } else {
            cout << "Unable to open " << id << " DMEM result file." << endl;
        }
        dmemout.close();
    }
};

class RegisterFile {
private: 
    vector<bitset<32> > Registers;
public:
    string outputFile;
    RegisterFile(string ioDir) : outputFile(ioDir + "RFResult.txt") {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr) {
        //Fill in
        bitset<32> reg_data = Registers[Reg_addr.to_ulong()];
        return reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
        //Fill in
        if (Reg_addr.to_ulong() != 0) {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
    }

    void outputRF(int cycle) {
        ofstream rfout;
        if (cycle == 0) {
            rfout.open(outputFile, ios_base::trunc);
        } else {
            rfout.open(outputFile, ios_base::app);
        }
        if (rfout.is_open()) {
            rfout << "----------------------------------------------------------------------" << endl;
            rfout << "State of RF after executing cycle:\t" << cycle << endl;
            for (int j = 0; j < 32; ++j) {
                rfout << Registers[j] << endl;
            }
        } else {
            cout << "Unable to open RF output file." << endl;
        }
        rfout.close();
    }
};

struct IFStruct {
    bool nop;
    bitset<32> PC;
};

struct IDStruct {
    bool nop;
    bitset<32> Instr;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<32>  instr;
    bitset<24>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bitset<4>   alucontrol;
    string funct3;
    bool branch;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        stall;
    bool        nop;     
};


class stateStruct {
public:
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
    
};

class Core {
public:
    RegisterFile myRF;
    uint32_t cycle;
    bool halted;
    string ioDir;
    struct stateStruct state, nextstate;
    InsMem ext_imem;
    DataMem ext_dmem;
    uint32_t instructionCount;

    Core(string ioDir, InsMem &imem, DataMem &dmem) : myRF(ioDir), ioDir(ioDir), ext_imem (imem), ext_dmem (dmem), instructionCount(0), cycle(0), halted(false) {}

    virtual void step() {}

    virtual void printState() {}

    virtual void printPerformanceMetrics() {}
};


class SingleStageCore : public Core {
private:
    string opFilePath;
public: 
    SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "\\SS_", imem, dmem), opFilePath(ioDir + "\\StateResult_SS.txt") {}

    //IF STAGE
    void InstructionFetch() {
        state.ID.Instr = ext_imem.readInstr(state.IF.PC);
        string opcode = state.ID.Instr.to_string().substr(25, 7);
        if (opcode == "1111111") {
            nextstate.IF.PC = state.IF.PC;
            nextstate.IF.nop = true;
        } else {
            nextstate.IF.nop = false;
            nextstate.IF.PC = state.IF.PC.to_ulong() + 4;
            state.ID.nop = false;
            instructionCount++;
        }
    }
    
    //ID STAGE
    //rs --> rs1, rt --> rs2
    void InstructionDecode() {
        state.EX.nop = state.ID.nop;
        if (!state.ID.nop) {
            string line = state.ID.Instr.to_string();
            string OPCODE = line.substr(25, 7);
            if (OPCODE == "0110011") {
                //R-TYPE
                string funct7 = line.substr(0, 7);
                string funct3 = line.substr(17, 3);
                funct3 += funct7[1];
                //Get rd, rs1, rs2
                state.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                state.EX.Rt = bitset<5>(line.substr(7, 5));
                state.EX.Read_data2 = myRF.readRF(state.EX.Rt);
                state.EX.Rs = bitset<5>(line.substr(12, 5));
                state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
                state.EX.rd_mem = false;
                state.EX.wrt_mem = false;
                state.EX.is_I_type = false;

                unordered_map<string, string> ALUcontrol;
                ALUcontrol["0000"] = "0010";
                ALUcontrol["0001"] = "0110";
                ALUcontrol["1110"] = "0000";
                ALUcontrol["1100"] = "0001";
                ALUcontrol["1000"] = "0011";
                state.EX.wrt_enable = true;
                //state.EX.branch = 0;
                state.EX.alucontrol = bitset<4>(ALUcontrol[funct3]);
                //add, sub, xor, or, and
            } else if (OPCODE == "0010011") {
                //I-TYPE
                string funct3 = line.substr(17, 3);
                //Get rd, rs1, imme
                state.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                state.EX.Rt = 0;
                state.EX.Read_data2 = 0;
                state.EX.Rs = bitset<5>(line.substr(12, 5));
                state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
                state.EX.Imm = bitset<24>(signextend(line.substr(0, 12), 24));
                state.EX.rd_mem = false;
                state.EX.wrt_mem = false;
                state.EX.is_I_type = true;

                unordered_map<string, string> ALUcontrol;
                ALUcontrol["000"] = "0010";
                ALUcontrol["111"] = "0000";
                ALUcontrol["110"] = "0001";
                ALUcontrol["100"] = "0011";
                state.EX.wrt_enable = true;
                //state.EX.branch = 0;
                state.EX.alucontrol = bitset<4>(ALUcontrol[funct3]);
                //addi, xori, ori, andi
            } else if (OPCODE == "1101111") {
                //JAL
                state.EX.funct3 = "111";
                //Get rd, imme
                state.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                state.EX.Read_data1 = 0;
                state.EX.Read_data2 = 0;
                //imme [20|10:1|11|19:12]
                state.EX.Imm = bitset<24>(line.substr(0, 1) + line.substr(12, 8) + line.substr(11, 1) + line.substr(1, 10) + '0');
                state.EX.rd_mem = false;
                state.EX.wrt_mem = false;
                state.EX.is_I_type = false;

                state.EX.wrt_enable = true;
                state.EX.branch = true;
                state.EX.alucontrol = bitset<4>("1111");
                //jal
            } else if (OPCODE == "1100011") {
                //B-TYPE
                state.EX.funct3 = line.substr(17, 3);
                //Get rs1, rs2, imme
                state.EX.Wrt_reg_addr = 0;
                state.EX.Rs = bitset<5>(line.substr(12, 5));
                state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
                state.EX.Rt = bitset<5>(line.substr(7, 5));
                state.EX.Read_data2 = myRF.readRF(state.EX.Rt);
                state.EX.Imm = bitset<24>(signextend(line.substr(0, 1) + line.substr(24, 1) + line.substr(1, 6) + line.substr(20, 4) + '0', 24));
                state.EX.rd_mem = false;
                state.EX.wrt_mem = false;
                state.EX.is_I_type = false;

                state.EX.wrt_enable = false;
                state.EX.branch = true;
                state.EX.alucontrol = bitset<4>("0110");
                //beq, bne
            } else if (OPCODE == "0000011") {
                //LW (I-TYPE)
                string funct3 = line.substr(17, 3);
                //Get rd, rs1, imme
                state.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                state.EX.Rt = 0;
                state.EX.Read_data2 = 0;
                state.EX.Rs = bitset<5>(line.substr(12, 5));
                state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
                state.EX.Imm = bitset<24>(signextend(line.substr(0, 12), 24));
                state.EX.rd_mem = true;
                state.EX.wrt_mem = false;
                state.EX.is_I_type = true;

                state.EX.wrt_enable = true;
                state.EX.branch = false;
                state.EX.alucontrol = bitset<4>("0010");
                //lw
            } else if (OPCODE == "0100011") {
                //SW (S-TYPE)
                string funct3 = line.substr(17, 3);
                //Get rs1, rs2, imme
                state.EX.Wrt_reg_addr = 0;
                state.EX.Rs = bitset<5>(line.substr(12, 5));
                state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
                state.EX.Rt = bitset<5>(line.substr(7, 5));
                state.EX.Read_data2 = myRF.readRF(state.EX.Rt);
                state.EX.Imm = bitset<24>(signextend(line.substr(0, 7) + line.substr(20, 5), 24));
                state.EX.rd_mem = false;
                state.EX.wrt_mem = true;
                state.EX.is_I_type = true;

                state.EX.wrt_enable = false;
                state.EX.branch = false;
                state.EX.alucontrol = bitset<4>("0010");
                //sw
            } 
        }
        
    }

    //EX STAGE
    void InstructionExecute() {
        state.MEM.nop = state.EX.nop;
        if (!state.EX.nop) {
            long Aluin2;
            if (state.EX.is_I_type) {
                if (state.EX.Imm[23] == 0) {
                    Aluin2 = state.EX.Imm.to_ulong();
                } else {
                    unsigned long value = state.EX.Imm.to_ulong();
                    Aluin2 = static_cast<long>(static_cast<int16_t>(value));
                }
            } else {
                Aluin2 = state.EX.Read_data2.to_ulong();
            }

            if (state.EX.alucontrol.to_string() == "0010") {
                //add operation
                state.MEM.ALUresult = state.EX.Read_data1.to_ulong() + Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0110") {
                //sub operation
                state.MEM.ALUresult = state.EX.Read_data1.to_ulong() - Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0000") {
                //and operationw
                state.MEM.ALUresult = state.EX.Read_data1.to_ulong() & Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0001") {
                //or operation
                state.MEM.ALUresult = state.EX.Read_data1.to_ulong() | Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0011") {
                //xor operation
                state.MEM.ALUresult = state.EX.Read_data1.to_ulong() ^ Aluin2;
            }

            if (state.EX.branch == true) {
                if (state.EX.funct3 == "000" && state.MEM.ALUresult.to_ulong() == 0) {
                    //beq
                    nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(state.EX.Imm.to_ulong())); //may need modifying
                    nextstate.IF.nop = false;
                    state.MEM.nop = true;
                    //cout << state.EX.Read_data1.to_ulong() << " " << Aluin2 << endl;
                } else if (state.EX.funct3 == "001" && state.MEM.ALUresult.to_ulong() != 0) {
                    //bne
                    nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(state.EX.Imm.to_ulong())); //may need modifying
                    nextstate.IF.nop = false;
                    state.MEM.nop = true;
                } else if (state.EX.funct3 == "111") {
                    nextstate.IF.nop = false;
                    state.MEM.ALUresult = state.IF.PC.to_ulong() + 4;
                    nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(state.EX.Imm.to_ulong())); //may need modifying
                }
            }
            state.MEM.rd_mem = state.EX.rd_mem;
            state.MEM.wrt_mem = state.EX.wrt_mem;
        }
        
    }

    void LoadStore() {
        state.WB.nop = state.MEM.nop;
        if (!state.MEM.nop) {
            if (state.MEM.rd_mem == true) {
                state.MEM.Store_data = ext_dmem.readDataMem(state.MEM.ALUresult);
            } else if (state.MEM.wrt_mem == true) {
                ext_dmem.writeDataMem(state.MEM.ALUresult, state.EX.Read_data2);
                //write in memory file?
            }
            state.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            state.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            state.WB.wrt_enable = state.EX.wrt_enable;
        } else {
            state.WB.nop = true;
        }
    }

    void WriteBack() {
        if (!state.WB.nop) {
            if (state.WB.wrt_enable == true) {
                if (state.EX.rd_mem == true) {
                    myRF.writeRF(state.WB.Wrt_reg_addr, state.MEM.Store_data);
                } else {
                    myRF.writeRF(state.WB.Wrt_reg_addr, state.MEM.ALUresult);
                }
            }
        }
    }

    void step() {
        //IF STAGE
        InstructionFetch();
        //ID STAGE
        InstructionDecode();
        //EX STAGE
        InstructionExecute();
        //MEM STAGE
        LoadStore();
        //WB STAGE
        WriteBack();
        if (state.IF.nop) {
            halted = true;
        }
        myRF.outputRF(cycle); // dump RF
        printState(nextstate, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        state = nextstate; // The end of the cycle and updates the current state with the values calculated in this cycle
        cycle++;
    }

    void printState(stateStruct state, int cycle) {
        ofstream printstate;
        if (cycle == 0) {
            printstate.open(opFilePath, std::ios_base::trunc);
        } else {
            printstate.open(opFilePath, std::ios_base::app);
        }
        if (printstate.is_open()) {
            printstate<<"----------------------------------------------------------------------" << endl;
            printstate<<"State after executing cycle:\t"<<cycle<<endl; 

            printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
            printstate<<"IF.nop:\t"<<boolalpha<<state.IF.nop<<endl;
        }
        else cout<<"Unable to open SS StateResult output file." << endl;
        printstate.close();
    }

    void printPerformanceMetrics() {

        ofstream Metrics_Result(ioDir + "PerformanceMetrics_Result.txt");
        if (Metrics_Result.is_open()) {
            Metrics_Result << "-----------------------------Single Stage Core Performance Metrics-----------------------------" << endl;
            Metrics_Result << "Number of cycles taken: " << cycle << endl;
            Metrics_Result << "Total Number of Instructions: " << instructionCount+1 << endl;
            Metrics_Result << "Cycles per instruction: " << to_string((float)cycle / (instructionCount + 1)) << endl;
            Metrics_Result << "Instructions per cycle: " << to_string((float)(instructionCount+1) / cycle) << endl;
            Metrics_Result.close();
        }
    }

    string signextend(string bin, int bits) {
        string res = bin;
        char sign = bin[0];
        while (res.size() < bits) {
            res = sign + res;
        }
        return res;
    }
};


//Five-stage processor
class FiveStageCore : public Core{
private:
		string opFilePath;
public:
    FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "\\FS_", imem, dmem), opFilePath(ioDir + "\\StateResult_FS.txt") {}

    void WriteBack() {
        if (!state.WB.nop) {
            if (state.WB.wrt_enable == true) {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
            if (state.WB.stall == true) {
                myRF.outputRF(cycle); // dump RF
                printState(nextstate, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
                cycle++;
            }
            
        }
    }

    //there is RAW Hazards here, need forwarding

    void LoadStore() {
        nextstate.WB.nop = state.MEM.nop;
        if (!state.MEM.nop) {
            if ((state.MEM.Wrt_reg_addr == state.EX.Rs || state.MEM.Wrt_reg_addr == state.EX.Rt) && state.MEM.rd_mem == true && state.EX.Rs != 0 && state.EX.Rt != 0) {
                nextstate.WB.stall = true;
            } else {
                nextstate.WB.stall = false;
            }
            if (state.MEM.rd_mem == true) {
                nextstate.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
            } else if (state.MEM.wrt_mem == true) {
                ext_dmem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);    //should have nextstate.MEM.Store_data = state.EX.Read_data2
                //write in memory file
            } else {
                nextstate.WB.Wrt_data = state.MEM.ALUresult;
            }
            nextstate.WB.Rs = state.MEM.Rs;
            nextstate.WB.Rt = state.MEM.Rt;
            nextstate.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            nextstate.WB.wrt_enable = state.MEM.wrt_enable;
        } 
    }

    void InstructionExecute() {
        nextstate.MEM.nop = state.EX.nop;
        if (!state.EX.nop) {
            
            long Aluin1 = state.EX.Read_data1.to_ulong();
            long Aluin2;
            if (state.EX.is_I_type) {
                if (state.EX.Imm[23] == 0) {
                    Aluin2 = state.EX.Imm.to_ulong();
                } else {
                    unsigned long value = state.EX.Imm.to_ulong();
                    Aluin2 = static_cast<long>(static_cast<int16_t>(value));
                }
            } else {
                Aluin2 = state.EX.Read_data2.to_ulong();
            }
            if (state.MEM.wrt_enable) {
                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.EX.Rs.to_ulong() != 0) {
                    if (state.MEM.rd_mem) {
                        Aluin1 = nextstate.WB.Wrt_data.to_ulong();
                    } 
                } else if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.EX.Rt.to_ulong() != 0) {
                    if (state.MEM.rd_mem) {
                        Aluin2 = nextstate.WB.Wrt_data.to_ulong();
                    } 
                }
            }

            if (state.EX.alucontrol.to_string() == "0010") {
                //add operation
                nextstate.MEM.ALUresult = Aluin1 + Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0110") {
                //sub operation
                nextstate.MEM.ALUresult = Aluin1 - Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0000") {
                //and operationw
                nextstate.MEM.ALUresult = Aluin1 & Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0001") {
                //or operation
                nextstate.MEM.ALUresult = Aluin1 | Aluin2;
            } else if (state.EX.alucontrol.to_string() == "0011") {
                //xor operation
                nextstate.MEM.ALUresult = Aluin1 ^ Aluin2;
            }

            if (state.EX.branch == true) {
                if (state.EX.funct3 == "111") {
                    nextstate.MEM.ALUresult = state.IF.PC.to_ulong() + 4 - state.EX.Imm.to_ulong();
                } else {
                    state.EX.nop = true;
                    nextstate.MEM.nop = true;
                }
            }

            nextstate.MEM.rd_mem = state.EX.rd_mem;
            nextstate.MEM.wrt_mem = state.EX.wrt_mem;
            nextstate.MEM.Store_data = state.EX.Read_data2;
            nextstate.MEM.Rs = state.EX.Rs;
            nextstate.MEM.Rt = state.EX.Rt;
            nextstate.MEM.wrt_enable = state.EX.wrt_enable;
            nextstate.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
        }
    }

    void InstructionDecode() {
        nextstate.EX.nop = state.ID.nop;
        if (!state.ID.nop) {
            nextstate.EX.instr = state.ID.Instr;
            string line = state.ID.Instr.to_string();
            string OPCODE = line.substr(25, 7);
            if (OPCODE == "0110011") {
                //R-TYPE
                string funct7 = line.substr(0, 7);
                string funct3 = line.substr(17, 3);
                funct3 += funct7[1];
                //Get rd, rs1, rs2
                nextstate.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                nextstate.EX.Rt = bitset<5>(line.substr(7, 5));
                nextstate.EX.Read_data2 = myRF.readRF(nextstate.EX.Rt);
                nextstate.EX.Rs = bitset<5>(line.substr(12, 5));
                nextstate.EX.Read_data1 = myRF.readRF(nextstate.EX.Rs);
                nextstate.EX.rd_mem = false;
                nextstate.EX.wrt_mem = false;
                nextstate.EX.is_I_type = false;

                unordered_map<string, string> ALUcontrol;
                ALUcontrol["0000"] = "0010";
                ALUcontrol["0001"] = "0110";
                ALUcontrol["1110"] = "0000";
                ALUcontrol["1100"] = "0001";
                ALUcontrol["1000"] = "0011";
                nextstate.EX.wrt_enable = true;
                nextstate.EX.branch = false;
                nextstate.EX.alucontrol = bitset<4>(ALUcontrol[funct3]);
                //add, sub, xor, or, and
                
            } else if (OPCODE == "0010011") {
                //I-TYPE
                string funct3 = line.substr(17, 3);
                //Get rd, rs1, imme
                nextstate.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                nextstate.EX.Rt = 0;
                nextstate.EX.Read_data2 = 0;
                nextstate.EX.Rs = bitset<5>(line.substr(12, 5));
                nextstate.EX.Read_data1 = myRF.readRF(nextstate.EX.Rs);
                nextstate.EX.Imm = bitset<24>(signextend(line.substr(0, 12), 24));
                nextstate.EX.rd_mem = false;
                nextstate.EX.wrt_mem = false;
                nextstate.EX.is_I_type = true;

                unordered_map<string, string> ALUcontrol;
                ALUcontrol["000"] = "0010";
                ALUcontrol["111"] = "0000";
                ALUcontrol["110"] = "0001";
                ALUcontrol["100"] = "0011";
                nextstate.EX.wrt_enable = true;
                nextstate.EX.branch = false;
                //state.EX.branch = 0;
                nextstate.EX.alucontrol = bitset<4>(ALUcontrol[funct3]);
                //addi, xori, ori, andi
            } else if (OPCODE == "1101111") {
                //JAL
                nextstate.EX.funct3 = "111";
                //Get rd, imme
                nextstate.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                nextstate.EX.Rs = 0;
                nextstate.EX.Rt = 0;
                nextstate.EX.Read_data1 = 0;
                nextstate.EX.Read_data2 = 0;
                //imme [20|10:1|11|19:12]
                nextstate.EX.Imm = bitset<24>(line.substr(0, 1) + line.substr(12, 8) + line.substr(11, 1) + line.substr(1, 10) + '0');
                nextstate.EX.rd_mem = false;
                nextstate.EX.wrt_mem = false;
                nextstate.EX.is_I_type = false;

                nextstate.EX.wrt_enable = true;
                nextstate.EX.branch = true;
                nextstate.EX.alucontrol = bitset<4>("1111");
                //jal
            } else if (OPCODE == "1100011") {
                //B-TYPE
                nextstate.EX.funct3 = line.substr(17, 3);
                //Get rs1, rs2, imme
                nextstate.EX.Wrt_reg_addr = 0;
                nextstate.EX.Rs = bitset<5>(line.substr(12, 5));
                nextstate.EX.Read_data1 = myRF.readRF(nextstate.EX.Rs);
                nextstate.EX.Rt = bitset<5>(line.substr(7, 5));
                nextstate.EX.Read_data2 = myRF.readRF(nextstate.EX.Rt);
                nextstate.EX.Imm = bitset<24>(signextend(line.substr(0, 1) + line.substr(24, 1) + line.substr(1, 6) + line.substr(20, 4) + '0', 24));
                nextstate.EX.rd_mem = false;
                nextstate.EX.wrt_mem = false;
                nextstate.EX.is_I_type = false;

                nextstate.EX.wrt_enable = false;
                nextstate.EX.branch = true;
                nextstate.EX.alucontrol = bitset<4>("0110");
                //b-type test branch hazard here
            
                //beq, bne
            } else if (OPCODE == "0000011") {
                //LW (I-TYPE)
                string funct3 = line.substr(17, 3);
                //Get rd, rs1, imme
                nextstate.EX.Wrt_reg_addr = bitset<5>(line.substr(20, 5));
                nextstate.EX.Rt = 0;
                nextstate.EX.Read_data2 = 0;
                nextstate.EX.Rs = bitset<5>(line.substr(12, 5));
                nextstate.EX.Read_data1 = myRF.readRF(nextstate.EX.Rs);
                nextstate.EX.Imm = bitset<24>(signextend(line.substr(0, 12), 24));
                nextstate.EX.rd_mem = true;
                nextstate.EX.wrt_mem = false;
                nextstate.EX.is_I_type = true;

                nextstate.EX.wrt_enable = true;
                nextstate.EX.branch = false;
                //state.EX.branch = 0;
                nextstate.EX.alucontrol = bitset<4>("0010");
                //lw
            } else if (OPCODE == "0100011") {
                //SW (S-TYPE)
                string funct3 = line.substr(17, 3);
                //Get rs1, rs2, imme
                nextstate.EX.Wrt_reg_addr = 0;
                nextstate.EX.Rs = bitset<5>(line.substr(12, 5));
                nextstate.EX.Read_data1 = myRF.readRF(nextstate.EX.Rs);
                nextstate.EX.Rt = bitset<5>(line.substr(7, 5));
                nextstate.EX.Read_data2 = myRF.readRF(nextstate.EX.Rt);
                nextstate.EX.Imm = bitset<24>(signextend(line.substr(0, 7) + line.substr(20, 5), 24));
                nextstate.EX.rd_mem = false;
                nextstate.EX.wrt_mem = true;
                nextstate.EX.is_I_type = true;

                nextstate.EX.wrt_enable = false;
                nextstate.EX.branch = false;
                //state.EX.branch = 0;
                nextstate.EX.alucontrol = bitset<4>("0010");
                //sw
            } 
            if (state.EX.wrt_enable) {
                if (state.EX.Wrt_reg_addr == nextstate.EX.Rs && nextstate.EX.Rs.to_ulong() != 0) {
                    if (state.EX.rd_mem) {
                        //nextstate.EX.Read_data1 = ext_dmem.readDataMem(nextstate.MEM.ALUresult);

                    } else {
                        nextstate.EX.Read_data1 = nextstate.MEM.ALUresult;
                    }
                    //cout << "Read_data1: " << nextstate.EX.Read_data1 << endl;
                } else if (state.EX.Wrt_reg_addr == nextstate.EX.Rt && nextstate.EX.Rt.to_ulong() != 0) {
                    if (state.EX.rd_mem) {
                        //nextstate.EX.Read_data2 = ext_dmem.readDataMem(nextstate.MEM.ALUresult);

                    } else {
                        nextstate.EX.Read_data2 = nextstate.MEM.ALUresult;
                    }
                    //cout << "Read_data2: " << nextstate.EX.Read_data2 << endl;
                }
            }
            

            if (state.MEM.wrt_enable) {
                if (state.MEM.Wrt_reg_addr == nextstate.EX.Rs && nextstate.EX.Rs.to_ulong() != 0) {
                    nextstate.EX.Read_data1 = nextstate.WB.Wrt_data;
                    //cout << "Read_data1: " << nextstate.EX.Read_data1 << endl;
                } else if (state.MEM.Wrt_reg_addr == nextstate.EX.Rt && nextstate.EX.Rt.to_ulong() != 0) {
                    nextstate.EX.Read_data2 = nextstate.WB.Wrt_data;
                    //cout << "Read_data2: " << nextstate.EX.Read_data2 << endl;
                }
            }

            if (OPCODE == "1100011") {
                if (nextstate.EX.funct3 == "000") {
                    if (nextstate.EX.Read_data1.to_ulong()-nextstate.EX.Read_data2.to_ulong() == 0) {
                        nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(nextstate.EX.Imm.to_ulong())) - 4;
                        nextstate.IF.nop = false;
                        state.IF.nop = true;
                    }
                } else if (nextstate.EX.funct3 == "001") {
                    if (nextstate.EX.Read_data1.to_ulong()-nextstate.EX.Read_data2.to_ulong() !=0) {
                        nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(nextstate.EX.Imm.to_ulong())) - 4;
                        nextstate.IF.nop = false;
                        state.IF.nop = true;
                    } 
                }
            } else if (OPCODE == "1101111") {
                nextstate.IF.PC = state.IF.PC.to_ulong() + static_cast<long>(static_cast<int16_t>(nextstate.EX.Imm.to_ulong())) - 4;
                nextstate.IF.nop = false;
                state.IF.nop = true;
            }
        }

        //cout << cycle << " " << nextstate.EX.Rs << nextstate.EX.Rt << endl;
    }

    void InstructionFetch() {
        nextstate.ID.nop = state.IF.nop;
        if (!state.IF.nop) {
            nextstate.ID.Instr = ext_imem.readInstr(state.IF.PC);
            string opcode = nextstate.ID.Instr.to_string().substr(25, 7);
            if (opcode == "1111111") {
                nextstate.IF.PC = state.IF.PC;
                nextstate.IF.nop = true;
                nextstate.ID.nop = true;
            } else {
                nextstate.IF.nop = false;
                nextstate.IF.PC = state.IF.PC.to_ulong() + 4;
                nextstate.ID.nop = false;
                instructionCount++;
            }
        }
        
    }

    void step() {
        /* Your implementation */
        /* --------------------- WB stage --------------------- */
        WriteBack();
        
        
        /* --------------------- MEM stage -------------------- */
        LoadStore();
        
        
        /* --------------------- EX stage --------------------- */
        InstructionExecute();
        
        
        /* --------------------- ID stage --------------------- */
        InstructionDecode();
        
        
        /* --------------------- IF stage --------------------- */
        InstructionFetch();
        
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            halted = true;
    
        myRF.outputRF(cycle); // dump RF
        printState(nextstate, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
    
        state = nextstate; //The end of the cycle and updates the current state with the values calculated in this cycle
        cycle++;
    }

    void printState(stateStruct state, int cycle) {
        ofstream printstate;
        if (cycle == 0)
            printstate.open(opFilePath, std::ios_base::trunc);
        else 
            printstate.open(opFilePath, std::ios_base::app);
        if (printstate.is_open()) {
            printstate<<"----------------------------------------------------------------------" << endl;
            printstate<<"State after executing cycle:\t"<<cycle<<endl; 

            printstate<<"IF.nop:\t"<<boolalpha<<state.IF.nop<<endl; 
            printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        

            printstate<<"ID.nop:\t"<<boolalpha<<state.ID.nop<<endl;
            printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 

            printstate<<"EX.nop:\t"<<boolalpha<<state.EX.nop<<endl;  
            printstate<<"EX.instr:\t"<<boolalpha<<state.EX.instr<<endl;  
            printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
            printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
            printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
            printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
            printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
            printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
            printstate<<"EX.is_I_type:\t"<<boolalpha<<state.EX.is_I_type<<endl; 
            printstate<<"EX.rd_mem:\t"<<boolalpha<<state.EX.rd_mem<<endl;
            printstate<<"EX.wrt_mem:\t"<<boolalpha<<state.EX.wrt_mem<<endl;        
            printstate<<"EX.alu_op:\t"<<boolalpha<<state.EX.alu_op<<endl;
            printstate<<"EX.wrt_enable:\t"<<boolalpha<<state.EX.wrt_enable<<endl;      

            printstate<<"MEM.nop:\t"<<boolalpha<<state.MEM.nop<<endl; 
            printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
            printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
            printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
            printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
            printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
            printstate<<"MEM.rd_mem:\t"<<boolalpha<<state.MEM.rd_mem<<endl;
            printstate<<"MEM.wrt_mem:\t"<<boolalpha<<state.MEM.wrt_mem<<endl; 
            printstate<<"MEM.wrt_enable:\t"<<boolalpha<<state.MEM.wrt_enable<<endl;                

            printstate<<"WB.nop:\t"<<boolalpha<<state.WB.nop<<endl; 
            printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
            printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
            printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
            printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
            printstate<<"WB.wrt_enable:\t"<<boolalpha<<state.WB.wrt_enable<<endl;
        }
        else cout<<"Unable to open FS StateResult output file." << endl;
        printstate.close();
    }

    void printPerformanceMetrics() {

        ofstream Metrics_Result(ioDir + "PerformanceMetrics_Result.txt");
        if (Metrics_Result.is_open()) {
            Metrics_Result << "-----------------------------Five Stage Core Performance Metrics-----------------------------" << endl;
            Metrics_Result << "Number of cycles taken: " << cycle << endl;
            Metrics_Result << "Total Number of Instructions: " << instructionCount + 1 << endl;
            Metrics_Result << "Cycles per instruction: " << to_string((float)cycle / (instructionCount + 1)) << endl;
            Metrics_Result << "Instructions per cycle: " << to_string((float)(instructionCount+1) / cycle) << endl;
            Metrics_Result.close();
        }
    }

    string signextend(string bin, int bits) {
        string res = bin;
        char sign = bin[0];
        while (res.size() < bits) {
            res = sign + res;
        }
        return res;
    }
};


int main(int argc, char* argv[]) {
    string ioDir = "";
    if (argc == 1) {
        cout << "Enter path containing the memory files: ";
        cin >> ioDir;
    } else if (argc > 2) {
        cout << "Invalid number of arguments. Machine stopped." << endl;
        return -1;
    } else {
        ioDir = argv[1];
        cout << "IO Directory: " << ioDir << endl;
    }

    InsMem imem = InsMem("Imem", ioDir);
    DataMem dmem_ss = DataMem("SS", ioDir);
	DataMem dmem_fs = DataMem("FS", ioDir);

	SingleStageCore SSCore(ioDir, imem, dmem_ss);
    FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (1) {
        if (!SSCore.halted) {
            SSCore.step();
        }
		
		if (!FSCore.halted) {
            FSCore.step();
        }
        
        
		if (SSCore.halted && FSCore.halted) {
			break;
        }
    }

    //print Performance Metrics Result
    SSCore.printPerformanceMetrics();
    FSCore.printPerformanceMetrics();

    //dump SS and FS data mem.
    SSCore.ext_dmem.outputDataMem();
    FSCore.ext_dmem.outputDataMem();
    return 0;
}