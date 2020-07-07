#include "parser.hpp"
#include "globaldef.hpp"
#include <cstring>

Parser::Parser(const char* filepath) : file(new fstream(filepath)) {
    if (!(*file))
        throw runtime_error("File error");
}

int Parser::hextoint(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else
        return c - 'A' + 10;
}

void Parser::padimm(taddr& imm, int digit) {
    if (getdigits(imm, digit - 2, digit - 1)) {
        imm = (0xFFFFFFFF << digit) | imm;
    }
    return;
}

mempair Parser::getline() {
    if (!((*file) >> inputline))
        return mempair{0, 0};
    if (inputline[0] == '@') {
        baseaddr = 0;
        for (int i = 0; i < 8; i++) {
            baseaddr <<= 4;
            baseaddr += inputline[i + 1] - '0';
        }
        return getline();
    }
    (*file) >> (inputline + 2) >> (inputline + 4) >> (inputline + 6);
    unsigned int operation = 0;
    for (int i = 3; i >= 0; i--) {
        operation <<= 8;
        operation += (hextoint(inputline[i << 1]) << 4) + hextoint(inputline[i << 1 | 1]);
    }
    mempair ret{baseaddr, operation};
    baseaddr += 4;
    return ret;
}

command Parser::Splitter(unsigned int operation, unsigned int baseaddr) {
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
    case 0b00000:
        return ISplitter(operation, baseaddr);
        break;
    case 0b01000:
        return SSplitter(operation, baseaddr);
        break;
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
    c.instruction = command::R;
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
    c.instruction = getdigits(operation, 1, 6) == 0b11001 ? command::Ij : command::I;
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
    c.instruction = command::I;
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
    c.instruction = command::S;
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
    c.instruction = getdigits(operation, 1, 6) == 0b00101 ? command::Ua : command::U;
    c.rd          = getdigits(operation, 6, 11);
    c.imm         = getdigits(operation, 11, 31) << 12;
    return c;
}
command Parser::BSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = command::B;
    c.funct3      = getdigits(operation, 11, 14);
    c.rs1         = getdigits(operation, 14, 19);
    c.rs2         = getdigits(operation, 19, 24);
    c.imm         = getdigits(operation, 7, 11) << 1 | getdigits(operation, 24, 30) << 5 | getdigits(operation, 6, 7) << 11 | getdigits(operation, 24, 25) << 12;
    padimm(c.imm, 13);
    return c;
}

command Parser::JSplitter(unsigned int operation, unsigned int baseaddr) {
    command c;
    c.addr        = baseaddr;
    c.instruction = command::J;
    c.rd          = getdigits(operation, 6, 11);
    c.funct3      = getdigits(operation, 11, 14);
    c.imm         = (getdigits(operation, 20, 30) << 1) | (getdigits(operation, 19, 20) << 11) | (getdigits(operation, 11, 19) << 12) | (getdigits(operation, 30, 31) << 20);
    padimm(c.imm, 20);
    return c;
}

ostream& displayer(command& z, ostream& os) {
    const char* cc[] = {
        "R ",
        "I ",
        "Ij",
        "S ",
        "U ",
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
    os << z.addr
       << "\t";
    os << cc[z.instruction] << ' ';
    switch (z.instruction) {
    case command::B:
        switch (z.funct3) {
        case 0b000: {
            os << "beq\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        case 0b001: {
            os << "bne\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        case 0b100: {
            os << "blt\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        case 0b101: {
            os << "bge\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        case 0b110: {
            os << "bltu\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        case 0b111: {
            os << "bgeu\t" << reg[z.rs1] << "\t" << reg[z.rs2] << "\t";
            int imp;
            memcpy(&imp, &(z.imm), sizeof(imp));
            os << imp;
            break;
        }
        }
        break;
    case command::J: {
        os << "jal\t" << reg[z.rd] << "\t";
        int imp;
        memcpy(&imp, &(z.imm), sizeof(imp));
        os << imp;
    } break;
    case command::U: {
    }
    }
    cout << z.funct3 << ' ' << z.funct7 << ' ' << z.imm << ' ' << z.rs1 << ' ' << z.rs2 << ' ' << z.rd << endl;
};