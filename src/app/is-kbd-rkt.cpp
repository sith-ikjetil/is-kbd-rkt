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
#include <sstream>
#include <iostream>

//
// #define
//
#define VERSION_INFO   "1.2"

//
// using
//
using std::string;
using std::stringstream;
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
    cout << setw(80) << setfill('#') << std::left << "#" << endl;
    cout << setw(80) << setfill('#') << std::left << "## Is Keyboard Rootkitted App " << endl;
    cout << "## Author  : " << "Kjetil Kristoffer Solberg <post@ikjetil.no>" << endl;
    cout << "## Version : " << VERSION_INFO << endl;
    cout << "##" << endl;
}

//
// print driver retrieved data
//
void PrintData(IS_KEYBOARD_RKT_DATA* p) 
{
    //
    // Base address'
    //
    cout << setw(36) << setfill('#') << "## BASE ADDRESS' ##" << endl;
    cout << setfill(' ');
    cout << setw(16) << std::left << "APIC" << ": " << std::hex << std::showbase << p->dwApicBaseAddress << endl;
    cout << setw(16) << std::left << "IO APIC" << ": " << std::hex << std::showbase << p->dwIoApicBaseAddress << endl;
    cout << setw(16) << std::left << "Root Complex" << ": " << std::hex << std::showbase << p->dwRootComplexBaseAddress << endl;
    
    //
    // IOTRn
    //
    cout << setw(36) << setfill('#') << "## IOTRn ##" << endl;
    for (int i = 0; i < IOTRn_COUNT; i++ ) { 
        stringstream ss;
        ss << "IOTRn[" << i << "]";
        cout << setfill(' ') << setw(16) << std::left << ss.str() << ": " << std::hex << std::showbase << p->qwIOTRn[i] << endl;
    }
    
    //
    // IOAPIC_IRQn
    //
    cout << setw(36) << setfill('#') << "## IOAPIC_IRQn ##" << endl;
    for (int i = 0; i < IO_APIC_IRQ_COUNT; i++ ) {
        stringstream ss;
        ss << "IOAPIC_IRQn[" << i << "]";
        cout << setfill(' ') <<setw(16) << std::left << ss.str() << ": " << std::hex << std::showbase << p->qwIOAPIC_REDTBL[i] << endl;
    }
    cout << setfill(' ') <<setw(16) << std::left << "ErrorMessage" << ": " << p->szErrorMessage << endl;

    //
    // Conclution
    //
    PrintConclution(p);
}

//
// print conclution from driver provided data
//
void PrintConclution(IS_KEYBOARD_RKT_DATA* p) {
    cout << "## C O N C L U T I O N ##" << endl;
    cout << "NA" << endl;
}