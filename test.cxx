/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>

#include <argsplus.hxx>

int main(int argc, char **argv) {
    argsplus::ArgumentParser<> parser("This is a test program", "This is the big epilogue");
    auto &boolflag = parser.AddOption<bool>("BOOLFLAG", {'b', "bool"}).Help("This is a boolean flag");
    auto &invboolflag = parser.AddOption<bool>("INVBOOLFLAG", {'i', "inverse"}).Default(false).Help("This is an inverse boolean flag");
    auto &doubleflag = parser.AddOption<double>("DUBFLAG", {'d', "double"}).Default(25.0).Help("This is some double flag");
    return 0;
}
