//This is the old core test.
//SEQUENTIAL TEST
#include "core.hpp"
#include "globaldef.hpp"
#include "memory.hpp"
#include "parser.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
using namespace std;
Parser P;

TEST(CoreTest, TickTest_Immidiates) {
    //This is aimed to test
    //commands interacting with immidiates.
    core_session C("dataset/basic-testset/test-1.data");
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00020000);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00020020);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x0002001D);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00000210);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000000);
    EXPECT_EQ(C.reg[3], 0x00000210);

    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000000);
    EXPECT_EQ(C.reg[3], 0x00000210);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000001);
    EXPECT_EQ(C.reg[3], 0x00000210);
    C.cycle();
    EXPECT_EQ(C.reg[5], 0xFFFFF000);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000001);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000000);

    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000001);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000000);
    EXPECT_EQ(C.reg[5], 0xFFFFF000);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0xFFFFF00F);
    EXPECT_EQ(C.reg[5], 0xFFFFF000);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0xFFFFF0FF);
    EXPECT_EQ(C.reg[5], 0xFFFFF000);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0xFFFFE000);

    C.cycle();
    EXPECT_EQ(C.reg[2], 0x7FFFF800);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0xFFFFF800);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0xFFFFF044);
}

TEST(CoreTest, TickTest_Registers) {
    //This is aimed to test
    //commands interacting with immidiates.
    core_session C("dataset/basic-testset/test-2.data");
    C.cycle();
    EXPECT_EQ(C.reg[1], 0xFFFFFFFF);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x0000000F);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x0000000E);
    EXPECT_EQ(C.reg[1], 0xFFFFFFFF);
    EXPECT_EQ(C.reg[2], 0x0000000F);
    C.cycle();
    EXPECT_EQ(C.reg[0], 0x00000000);
    EXPECT_EQ(C.reg[1], 0xFFFFFFFF);
    EXPECT_EQ(C.reg[2], 0x0000000F);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0xFFFFFFF0);
    EXPECT_EQ(C.reg[1], 0xFFFFFFFF);
    EXPECT_EQ(C.reg[2], 0x0000000F);

    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00000010);
    EXPECT_EQ(C.reg[1], 0xFFFFFFFF);
    EXPECT_EQ(C.reg[2], 0x0000000F);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00000001);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00000000);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x0000000F);
    C.cycle();
    EXPECT_EQ(C.reg[4], 0xFFFFFFF0);

    C.cycle();
    EXPECT_EQ(C.reg[5], 0x0000000E);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0xFFFFFFFE);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00000001);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x80000000);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0x00010000);

    C.cycle();
    EXPECT_EQ(C.reg[3], 0x0001FFFF);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0xFFFFFFF0);
    C.cycle();
    EXPECT_EQ(C.reg[2], 0x00000002);
    C.cycle();
    EXPECT_EQ(C.reg[3], 0xFFFFFFFC);
}


TEST(CoreTest,TickTest_Reg_ADD_SUB) {
    core_session C("dataset/basic-testset/test-C.data");
    EXPECT_EQ(C.run(), 176);
}

TEST(CoreTest, TickTest_Jump_1) {
    //This is aimed to test
    //jumping commands.
    core_session C("dataset/basic-testset/test-3.data");
    EXPECT_EQ(C.pc, 0x0000000);
    C.cycle();
    EXPECT_EQ(C.pc, 0x0000010);
    C.cycle();
    EXPECT_EQ(C.pc, 0x0000014);
    C.cycle();
    EXPECT_EQ(C.pc, 0x0000004);
    C.cycle();
    EXPECT_EQ(C.pc, 0x000000C);
    C.cycle();
    EXPECT_EQ(C.pc, 0x0000008);
    C.cycle();
    EXPECT_EQ(C.pc, 0x0000018);
}

TEST(CoreTest, MemoryWriteTest) {
    core_session C("dataset/basic-testset/test-5.data");
    for (int i = 0; i < 7; i++)
        C.cycle();
    EXPECT_EQ(C.M.get(0x00001000), 0xEFBEADDE);
    EXPECT_EQ(C.M.get(0x00001020), 0xEFBE0000);
    EXPECT_EQ(C.M.get(0x00001040), 0xEF000000);
    C.cycle();
    EXPECT_EQ(C.M.get(0x00001000), 0xFFBEADDE);
}

TEST(CoreTest, MemoryReadTest) {
    core_session C("dataset/basic-testset/test-6.data");
    EXPECT_EQ(C.M.get(0xDEADBEEF), P.rearrange(0xDEADC0DE));
    for (int i = 0; i < 7; i++)
        C.cycle();
    EXPECT_EQ(C.M.get(0x00001000), 0xEFBEADDE);
    EXPECT_EQ(C.M.get(0x00001020), 0xEFBE0000);
    EXPECT_EQ(C.M.get(0x00001040), 0xEF000000);
    C.cycle();
    EXPECT_EQ(C.M.get(0x00001000), 0xFFBEADDE);
    for (int i = 0; i < 9; i++)
        C.cycle();
    EXPECT_EQ(C.reg[13], 0xFFFFFFFF);
    EXPECT_EQ(C.reg[14], 0x000000FF);
    EXPECT_EQ(C.reg[15], 0xDEADBEFF);
    EXPECT_EQ(C.reg[16], 0xFFFFBEEF);
    EXPECT_EQ(C.reg[17], 0x0000BEEF);
    EXPECT_EQ(C.M.get(0xDEADBEEF), P.rearrange(0xDEADC0DE));
    EXPECT_GE(C.pc, 0x3c + 4);
    EXPECT_EQ(C.reg[23], 0xDEADC0DE);
}

TEST(CoreTest, JumpTest) {
    core_session C("dataset/basic-testset/test-7.data");
    taddr pc[] = {0, 0x10, 0x14, 0x4, 0xC, 0x8, 0x18};
    for (int i = 0; i < 7; i++) {
        EXPECT_EQ(C.pc, pc[i]);
        C.cycle();
    }
}

TEST(CoreTest, BranchTest_1_BEQ) {
    core_session C("dataset/basic-testset/test-8.data");
    taddr pc[] = {0x0, 0x4, 0x8, 0x10, 0x14, 0x18, 0xc};
    for (int i = 0; i < 7; i++) {
        EXPECT_EQ(C.pc, pc[i]);
        C.cycle();
    }
}

TEST(CoreTest, BranchTest_2_BNE) {
    core_session C("dataset/basic-testset/test-9.data");
    taddr pc[] = {0x0, 0x4, 0x8, 0x10, 0x14, 0x18, 0x1c};
    for (int i = 0; i < 7; i++) {
        EXPECT_EQ(C.pc, pc[i]);
        C.cycle();
    }
}

TEST(CoreTest, BranchTest_3_BLT_U) {
    core_session C("dataset/basic-testset/test-A.data");
    taddr pc[] = {0x0, 0x4, 0x8, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x0c};
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(C.pc, pc[i]);
        C.cycle();
    }
}

TEST(CoreTest, BranchTest_4_BGE_U) {
    core_session C("dataset/basic-testset/test-B.data");
    taddr pc[] = {0x0, 0x4, 0x8, 0x10, 0x14, 0x18, 0x20, 0x28, 0x2c, 0x0c};
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(C.pc, pc[i]);
        C.cycle();
    }
}

// TEST(CoreTest, ReturnTest_1) {
//     core_session C("dataset/sample.data");
//     EXPECT_EQ(C.run(), 94);
// };


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}