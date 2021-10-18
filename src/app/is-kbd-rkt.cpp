//////////////////////////////////////////////////////////////
//: Filename    : is-kbd-rkt.cpp
//: Date        : 2021-10-18
//: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version     : 1.2
//: Description : A Linux Kernel Module that detects an SMM keyboard rootkit.
//
// #include
//
#include "../include/is-kbd-rkt.h"
#include "../include/is-kbd-rkt-data.h"
#include "../include/itsoftware-linux.h"
#include "../include/itsoftware-linux-core.h"
#include <string>
#include <iostream>

//
// #define
//
#define VERSION_INFO   "1.2"

//
// using
//
using std::string;
using std::cout;
using std::setw;
using std::setfill;
using std::endl;
using ItSoftware::Linux::Core::ItsFile;
using ItSoftware::Linux::Core::unique_file_descriptor;

//
// function prototypes
//
void PrintHeader();
void PrintData(IS_KEYBOARD_RKT_DATA* p);
void PrintConclution(IS_KEYBOARD_RKT_DATA* p);

//
// main
//
int main(int argc, const char* argv[])
{
    PrintHeader();

    string deviceName("/dev/");
    deviceName += DEVICE_NAME;

    unique_file_descriptor file = open(deviceName.c_str(), O_RDONLY);
    if ( file.IsInvalid() ) {
        cout << "Could not open: " << deviceName << endl;
        cout << "Error: " << strerror(errno) << endl;
        cout << "Have you checked if device exists?" << endl;
        cout << "Are you running as root?" << endl;
        return EXIT_FAILURE;
    }

    IS_KEYBOARD_RKT_DATA data{0};
    size_t bytesRead(0);
    auto size = read(file, static_cast<void*>(&data), sizeof(IS_KEYBOARD_RKT_DATA));
    if ( size != sizeof(IS_KEYBOARD_RKT_DATA) ) {
        cout << "Not able to read from " << deviceName << endl;
        return EXIT_FAILURE;
    }

    PrintData(&data);
}

//
// print app ui header
//
void PrintHeader()
{
    cout << setw(80) << setfill('#') << std::left << "## Is Keyboard Rootkitted UI App " << endl;
    cout << "## Author  : " << "Kjetil Kristoffer Solberg <post@ikjetil.no>" << endl;
    cout << "## Version : " << VERSION_INFO << endl;
    cout << "##" << endl;
}

//
// print driver retrieved data
//
void PrintData(IS_KEYBOARD_RKT_DATA* p) 
{
    cout << setfill(' ');
    cout << setw(26) << std::left << "APIC Base Address" << ": " << p->dwApicBaseAddress << endl;
    cout << setw(26) << std::left << "IO APIC Base Address" << ": " << p->dwIoApicBaseAddress << endl;
    cout << setw(26) << std::left << "Root Complex Base Address" << ": " << p->dwRootComplexBaseAddress << endl;
    for (int i = 0; i < IOTRn_COUNT; i++ ) { 
        string s("IOTRn[");
        s += std::to_string((i+1));
        s += "]";
        cout << setw(16) << std::left << s << ": " << p->qwIOTRn[i] << endl;
    }
    for (int i = 0; i < IO_APIC_IRQ_COUNT; i++ ) {
        string s("IOAPIC_IRQn[");
        s += std::to_string((i+1));
        s += "]";
        cout << setw(16) << std::left << s << ": " << p->qwIOAPIC_REDTBL[i] << endl;
    }
    cout << setw(16) << std::left << "ErrorMessage" << ": " << p->szErrorMessage << endl;

    PrintConclution(p);
}

//
// print conclution from driver provided data
//
void PrintConclution(IS_KEYBOARD_RKT_DATA* p) {
    cout << "## C O N C L U T I O N ##" << endl;
    cout << "NA" << endl;
}