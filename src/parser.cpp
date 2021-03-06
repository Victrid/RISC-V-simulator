#include "parser.hpp"
#include "globaldef.hpp"
#include <cstring>

void Parser::padimm(taddr& imm, int digit) {
    if (getdigits(imm, digit - 2, digit - 1)) {
        imm = (0xFFFFFFFF << digit) | imm;
    }
    return;
}

int Parser::fint(const taddr& t) {
    int ret;
    memcpy(&ret, &t, sizeof(taddr));
    return ret;
}

taddr Parser::ftaddr(const int& t) {
    taddr ret;
    memcpy(&ret, &t, sizeof(taddr));
    return ret;
}

command Parser::Splitter(unsigned int operation, unsigned int baseaddr) {
    operation = rearrange(operation);
    if (operation == 0x0ff00513)
        return command(baseaddr, false);
    taddr opcode = (operation - (operation >> 7 << 7)) >> 2;
    switch (opcode) {
    case 0b01100:
        return RSplitter(operation, baseaddr);
        break;
    case 0b00100: {
        int p = getdigits(operation, 11, 14);
        if (p != 0b001 && p != 0b101)
            return ISplitter(operation, baseaddr);
        else
            return IsSplitter(operation, baseaddr);
        break;
    }
    case 0b11001:
        return ISplitter(operation, baseaddr);
    case 0b00000:
        return IlSplitter(operation, baseaddr);
    case 0b01000:
        return SSplitter(operation, baseaddr);
    case 0b01101:
    case 0b00101:
        return USplitter(operation, baseaddr);
    case 0b11000:
        return BSplitter(operation, baseaddr);
    case 0b11011:
        return JSplitter(operation, baseaddr);
    }
    return command();
}
taddr Parser::getdigits(taddr content, int l, int r) {
    if (r == 31)
        return content >> (l + 1);
    return (content - (content >> (r + 1) << (r + 1))) >> (l + 1);
};
command Parser::RSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::R;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.rs2         = getdigits(operation, 19, 24);
    c.funct7      = getdigits(operation, 24, 31);
    return c;
}
command Parser::ISplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = getdigits(operation, 1, 6) == 0b11001 ? instr::Ij : instr::I;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.imm         = getdigits(operation, 19, 31);
    padimm(c.imm, 12);
    return c;
}
command Parser::IlSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::Il;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.imm         = getdigits(operation, 19, 31);
    padimm(c.imm, 12);
    return c;
}
command Parser::IsSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::I;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.imm         = getdigits(operation, 19, 24);
    c.funct7      = getdigits(operation, 24, 31);
    return c;
}
command Parser::SSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::S;
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.rs2         = getdigits(operation, 19, 24);
    c.imm         = (getdigits(operation, 24, 31) << 5) | getdigits(operation, 6, 11);
    padimm(c.imm, 12);
    return c;
}
command Parser::USplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = getdigits(operation, 1, 6) == 0b00101 ? instr::Ua : instr::U;
    c.rd          = getdigits(operation, 6, 11);
    c.imm         = getdigits(operation, 11, 31) << 12;
    return c;
}
command Parser::BSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::B;
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.rs2         = getdigits(operation, 19, 24);
    c.imm         = getdigits(operation, 7, 11) << 1 | getdigits(operation, 24, 30) << 5 | getdigits(operation, 6, 7) << 11 | getdigits(operation, 30, 31) << 12;
    padimm(c.imm, 13);
    return c;
}

command Parser::JSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = instr::J;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.imm         = (getdigits(operation, 20, 30) << 1) | (getdigits(operation, 19, 20) << 11) | (getdigits(operation, 11, 19) << 12) | (getdigits(operation, 30, 31) << 20);
    padimm(c.imm, 20);
    return c;
}

std::ostream& Parser::displayer(command& z, std::ostream& os) {
    const char* cc[] = {
        "R ",
        "I ",
        "Il",
        "Ij",
        "S ",
        "U ",
        "Ua",
        "J ",
        "B "};
    const char* reg[] = {
        "zero",
        "ra",
        "sp",
        "gp",
        "tp",
        "t0",
        "t1",
        "t2",
        "s0",
        "s1",
        "a0",
        "a1",
        "a2",
        "a3",
        "a4",
        "a5",
        "a6",
        "a7",
        "s2",
        "s3",
        "s4",
        "s5",
        "s6",
        "s7",
        "s8",
        "s9",
        "s10",
        "s11",
        "t3",
        "t4",
        "t5",
        "t6"};
    // os << std::hex << z.addr << '\t';
    // os << cc[z.instruction] << ' ';
    switch (z.instruction) {
    case instr::T:
        return os; 
    case instr::B:
        switch (z.funct3) {
        case 0b000:
            os << "beq\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        case 0b001:
            os << "bne\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        case 0b100:
            os << "blt\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        case 0b101:
            os << "bge\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        case 0b110:
            os << "bltu\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        case 0b111:
            os << "bgeu\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::hex << z.addr + fint(z.imm);
            break;
        }
        break;
    case instr::J:
        os << "jal\t" << reg[z.rd] << ',' << std::hex << z.addr + fint(z.imm);
        break;
    case instr::U:
        os << "lui\t" << reg[z.rd] << "\t0x" << std::hex << (z.imm >> 12);
        break;
    case instr::Ua:
        os << "auipc\t" << reg[z.rd] << ',' << std::hex << z.imm;
        break;
    case instr::S:
        switch (z.funct3) {
        case 0b000:
            os << "sb\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::dec << fint(z.imm);
            break;
        case 0b001:
            os << "sh\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::dec << fint(z.imm);
            break;
        case 0b010:
            os << "sw\t" << reg[z.rs1] << ',' << reg[z.rs2] << ',' << std::dec << fint(z.imm);
            break;
        }
        break;
    case instr::Il:
        switch (z.funct3) {
        case 0b000:
            os << "lb\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b001:
            os << "lh\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b010:
            os << "lw\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b100:
            os << "lbu\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b101:
            os << "lhu\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        }
        break;
    case instr::I: {
        switch (z.funct3) {
        case 0b000:
            os << "addi\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b010:
            os << "slti\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b100:
            os << "xori\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b110:
            os << "ori\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b111:
            os << "andi\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << fint(z.imm);
            break;
        case 0b001:
            os << "slli\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << z.imm;
            break;
        case 0b101:
            if (z.funct7 == 0)
                os << "srli\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << z.imm;
            else
                os << "srai\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << z.imm;
            break;
        }
        break;
    }
    case instr::Ij:
        os << "jalr\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::hex << fint(z.imm);
        break;
    case instr::R:
        switch (z.funct3) {
        case 0b000:
            if (z.funct7 == 0)
                os << "add\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            else
                os << "sub\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b001:
            os << "sll\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b010:
            os << "slt\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b011:
            os << "sltu\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b100:
            os << "xor\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b101:
            if (z.funct7 == 0)
                os << "srl\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            else
                os << "sra\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b110:
            os << "or\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        case 0b111:
            os << "and\t" << reg[z.rd] << ',' << reg[z.rs1] << ',' << std::dec << reg[z.rs2];
            break;
        }
    }
    return os;
};

taddr Parser::rearrange(const taddr& t) {
    return getdigits(t, -1, 7) << 24 | getdigits(t, 7, 15) << 16 | getdigits(t, 15, 23) << 8 | getdigits(t, 23, 31);
};