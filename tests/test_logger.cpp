
#include <gtest/gtest.h>
#include <fstream>
#include "../logger/logger.h"

TEST(LoggerBasic, FlagsEnableDisable) {
    Logger log("tmp_log.txt");
    log.disable(Logger::DEBUG | Logger::INFO | Logger::WARN | Logger::ERROR);
    EXPECT_FALSE(log.isEnabled(Logger::DEBUG));
    log.enable(Logger::DEBUG);
    EXPECT_TRUE(log.isEnabled(Logger::DEBUG));
    log.disable(Logger::DEBUG);
    EXPECT_FALSE(log.isEnabled(Logger::DEBUG));
}

TEST(LoggerBasic, WritesToFile) {
    const char* path = "tmp_log.txt";
    std::remove(path);
    Logger log(path);
    log.enable(Logger::DEBUG | Logger::INFO | Logger::WARN | Logger::ERROR);
    log.debug("hello");
    log.info("world");
    std::ifstream in(path);
    ASSERT_TRUE(in.good());
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("DEBUG"), std::string::npos);
    EXPECT_NE(contents.find("INFO"), std::string::npos);
}
