#include <gtest/gtest.h>

#include "AppCore/Parser.h"
#include "AppCore/RuntimeInfo.h"

using namespace FileSearch::AppCore;

class ParserTest : public ::testing::Test {

};

TEST_F(ParserTest, Parse)
{
    try {
        Parser p;
        auto expr = p.compile("\"test string\" contains \"str\"");
        auto result = expr->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
    } catch (ParserException& ex) {
        std::cerr << "Error at pos " << ex.pos() << ": " << ex.what() << std::endl;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    
    try {
        FileSearch::AppCore::Parser p;
        auto result = p.compile("\"test\"==\"test\"")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("\"test\"!=\"test\"")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("\"test\"==\"test2\"")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));
        
        result = p.compile("123==123")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("123==567")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("1 > 2")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));
        result = p.compile("1 > 1")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));
        result = p.compile("100 > -17")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("5 < 7")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("4 < 4")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));
        result = p.compile("-6 < -4")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("4 < 2")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("7 >= 2")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("7 >= 7")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("7 >= 8")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("4 <= 4")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("14 <= -4")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("14 <= -4")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));
        result = p.compile("14 <= 14")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        result = p.compile("76 <= 99")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("4 == 4")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("(1 == 1) and (2 == 2)")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("1 + 2 == 3")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("1==2 or 2==2")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        result = p.compile("1==7 or 8==-3")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("1==7 or 8==-3")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("7==7 or 8==-3")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));
        
        result = p.compile("7==7 and 8==-3")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("9==9 and 10==10")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));

        /*result = p.compile("9==9 and not(10==10)")->evaluate();
        ASSERT_EQ(false, boost::get<bool>(result));

        result = p.compile("not(1!=1)")->evaluate();
        ASSERT_EQ(true, boost::get<bool>(result));*/
    } catch (ParserException& ex) {
        std::cerr << "Error at pos " << ex.pos() << ": " << ex.what() << std::endl;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

TEST_F(ParserTest, InvalidExpressions) {
    FileSearch::AppCore::Parser p;
    ASSERT_THROW(p.compile(""), ParserException);
    ASSERT_THROW(p.compile("("), ParserException);
    ASSERT_NO_THROW(p.compile("1"));
    ASSERT_THROW(p.compile("(((())))"), ParserException);
    ASSERT_THROW(p.compile("((((1)))"), ParserException);
    ASSERT_THROW(p.compile("1==1 and 2==and"), ParserException);
    ASSERT_THROW(p.compile("or and is"), ParserException);
}
