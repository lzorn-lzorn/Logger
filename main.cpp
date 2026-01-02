#include <iostream>
#include "SimpleLog.hpp"

int main() {
    Report::Message msg1("This is an info message.", Report::Level::Info);
    Report::Message msg2("This is a warning message.", Report::Level::Warning);
    Report::Message msg3("This is an error message.", Report::Level::Error);
    Report::Message msg4("This is a fatal message.", Report::Level::Fatal);
    Report::PrintMessageToConsole(msg1);
    Report::PrintMessageToConsole(msg2);
    Report::PrintMessageToConsole(msg3);
    Report::PrintMessageToConsole(msg4);

    Report::Buffer log_buffer;
    Report::Buffer log_buffer1;
    Report::Buffer log_buffer2;
    return 0;
}