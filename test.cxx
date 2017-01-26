/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>

#include <argsplus.hxx>

int main(int argc, char **argv) {
    argsplus::ArgumentParser<> parser("This is a test program", "This is the big epilogue");
    auto &doubleflag = parser.AddOption<double>("DUBFLAG", {'d', "double"});
    doubleflag.Default(25.0).Help("This is some double flag");
    auto &pos = parser.AddPositional<unsigned int>("MyPos");
    pos.Default(17).Help("This is some positional flag");
    if (!parser.ParseCLI(argc, argv)) {
        std::cerr << "Error encountered:\n" << parser.Error() << std::endl;
    }
    std::cout << doubleflag.Value() << std::endl;
    std::cout << pos.Value() << std::endl;
    return 0;
}
