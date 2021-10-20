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
#define MAX_N           1'000
#define MIN_N           1
#define DEFAULT_N       200

//
// using
//
using std::string;
using std::stringstream;
using std::cout;
using std::setw;
using std::setfill;
using std::endl;
using ItSoftware::Linux::ItsConvert;
using ItSoftware::Linux::ItsString;
using ItSoftware::Linux::Core::ItsFile;
using ItSoftware::Linux::Core::unique_file_descriptor;

//
// struct: IS_KEYBOARD_RKT_RESULT
//
typedef struct IS_KEYBOARD_RKT_RESULT {
    bool bHitIOTR0;
    bool bHitIOTR1;
    bool bHitIOTR2;
    bool bHitIOTR3;

    unsigned short wHitPortIOTR0;
    unsigned short wHitPortIOTR1;
    unsigned short wHitPortIOTR2;
    unsigned short wHitPortIOTR3;

    bool bHitIoApicIRQ1;

    unsigned long long qwHitIOTR0;
    unsigned long long qwHitIOTR1;
    unsigned long long qwHitIOTR2;
    unsigned long long qwHitIOTR3;
} IS_KEYBOARD_RKT_RESULT, *PIS_KEYBOARD_RKT_RESULT;

//
// function prototypes
//
void PrintHeader();
void PrintData(IS_KEYBOARD_RKT_DATA* p);
void PrintConclusion(IS_KEYBOARD_RKT_RESULT* r);
bool ProcessResult(IS_KEYBOARD_RKT_DATA *p, IS_KEYBOARD_RKT_RESULT *r);
void UpdateN(int argc, const char* argv[]);
void UpdateVerbose(int argc, const char* argv[]);

//
// global data
//
int     g_n = DEFAULT_N;
bool    g_verbose = false;

//
// Function: main
//
// (i): entry point
//
int main(int argc, const char* argv[])
{
    UpdateN(argc, argv);
    UpdateVerbose(argc,argv);

    PrintHeader();

    string deviceName("/dev/");
    deviceName += DEVICE_NAME;

    unique_file_descriptor file = open(deviceName.c_str(), O_RDONLY);
    if ( file.IsInvalid() ) {
        cout << "Could not open: " << deviceName << endl;
        cout << "Error: " << strerror(errno) << endl;
        cout << "Have you checked if device exists?" << endl;
        return EXIT_FAILURE;
    }

    IS_KEYBOARD_RKT_DATA    data{0};
    IS_KEYBOARD_RKT_RESULT  result{0};
    size_t bytesRead(0);
    bool bResult = false;
    int n = g_n;
    do
    {    
        auto size = read(file, static_cast<void*>(&data), sizeof(IS_KEYBOARD_RKT_DATA));
        if ( size != sizeof(IS_KEYBOARD_RKT_DATA) ) {
            cout << "Not able to read from " << deviceName << endl;
            return EXIT_FAILURE;
        }

        bResult = ProcessResult(&data, &result);

    } while ((--n > 0) && !bResult);
 
    PrintData(&data);

    PrintConclusion(&result);
}

//
// Function: GetIsArg
//
// (i): Gets bool true if we have the argument. false otherwise.
//
bool GetHasArg(string arg, int argc, const char* argv[])
{
    for ( int i = 0; i < argc; i++ ) {
        if ( argv[i] == arg ) {
            return true;
        }
    }
    return false;
}

//
// Function: GetArgVal
//
// (i): Gets argument value if there is one.
//
string GetArgVal(string arg, int argc, const char* argv[])
{
    for ( int i = 0; i < argc; i++ ) {
        if ( argv[i] == arg ) {
            if ( i < (argc-1)) {
                return string(argv[i+1]);
            }
        }
    }

    return string("");
}

//
// Function: UpdateN
//
// (i): Updates g_n global variable.
//
void UpdateN(int argc, const char* argv[]) 
{
    string newN = GetArgVal("-n", argc, argv);
    if ( newN.size() > 0 ) {
        try
        {
            g_n = ItsConvert::ToNumber<int>(ItsString::Replace(newN, "'", ""));
            if (g_n > MAX_N ) {
                g_n = MAX_N;
            }
            else if ( g_n < MIN_N) {
                g_n = MIN_N;
            }
        }
        catch(const std::exception& e)
        {

        }
    }
}

//
// Function: UpdateVerbose
//
// (i): Updates g_verbose global variable.
//
void UpdateVerbose(int argc, const char* argv[]) 
{
    g_verbose = (GetHasArg("-v",argc,argv) || GetHasArg("--verbose", argc, argv));
}

//
// print app ui header
//
void PrintHeader()
{
    cout << setw(80) << setfill('#') << std::left << "#" << endl;
    cout << setw(80) << setfill('#') << std::left << "## Is Keyboard Rootkitted App " << endl;
    cout << "## Author    : " << "Kjetil Kristoffer Solberg <post@ikjetil.no>" << endl;
    cout << "## Version   : " << VERSION_INFO << endl;
    cout << "## Arguments : " << endl;
    cout << "##             -n <count> = Number of times to run the test." << endl;
    cout << "##                          default = 200, max = 1'000, min = 1" << endl;
    cout << "##             -v         = verbose output" << endl;
    cout << "##             --verbose  = verbose output" << endl;
    cout << "## Using     : " << endl;
    cout << "##              n = " << g_n << endl;
    cout << "##              v = " << ((g_verbose) ? "true" : "false") << endl;
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
    cout << std::left << setw(36) << setfill('#') << "## BASE ADDRESS ##" << endl;
    cout << setfill(' ') << setw(16) << std::left << "APIC" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwApicBaseAddress << endl;
    cout << setfill(' ') << setw(16) << std::left << "IO APIC" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwIoApicBaseAddress << endl;
    cout << setfill(' ') << setw(16) << std::left << "Root Complex" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwRootComplexBaseAddress << endl;
    
    if ( g_verbose ) 
    {
        //
        // IOTRn
        //
        cout << std::left << setw(36) << setfill('#') << "## IOTRn ##" << endl;
        for (int i = 0; i < IOTRn_COUNT; i++ ) { 
            stringstream ss;
            ss << "IOTRn[" << i << "]";
            cout << setfill(' ') << setw(16) << std::left << ss.str() << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOTRn[i] << ((p->qwIOTRn[0] & 1) ? " TRSE-bit SET" : " TRSE-bit NOT SET") << endl;
        }
        
        //
        // IOAPIC_IRQn
        //
        cout << std::left << setw(36) << setfill('#') << "## IOAPIC_IRQn ##" << endl;
        for (int i = 0; i < IO_APIC_IRQ_COUNT; i++ ) {
            stringstream ss;
            ss << "IOAPIC_IRQn[" << i << "]";
            cout << setfill(' ') <<setw(16) << std::left << ss.str() << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOAPIC_REDTBL[i] << (((p->qwIOAPIC_REDTBL[i] & 0b1'0000'0000'0000'0000) == 0) ? " Interrupt Mask-bit NOT SET" : " Interrupt Mask-bit SET") << endl;
        }

        cout << setfill(' ') <<setw(16) << std::left << "ErrorMessage" << ": " << p->szErrorMessage << endl;
    }
}

//
// Function: ProcessResult
//
// (i): Process data and set result.
//
bool ProcessResult(IS_KEYBOARD_RKT_DATA *p, IS_KEYBOARD_RKT_RESULT *r)
{
    // IOTR0
    if (p->qwIOTRn[0] == 0x61)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x60;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    if (p->qwIOTRn[0] == 0x65)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x64;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    // IOTR1
    if (p->qwIOTRn[1] == 0x61)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x60;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    if (p->qwIOTRn[1] == 0x65)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x64;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    // IOTR2
    if (p->qwIOTRn[2] == 0x61)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x60;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    if (p->qwIOTRn[2] == 0x65)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x64;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    // IOTR3
    if (p->qwIOTRn[3] == 0x61)
    {
        r->bHitIOTR3 = true;
        r->wHitPortIOTR3 = 0x60;
        r->qwHitIOTR3 = p->qwIOTRn[3];
    }

    if (p->qwIOTRn[3] == 0x65)
    {
        r->bHitIOTR3 = true;
        r->wHitPortIOTR3 = 0x64;
        r->qwHitIOTR3 = p->qwIOTRn[3];
    }

    if (((p->qwIOAPIC_REDTBL[1] & 0b0111'0000'0000) == 0b0010'0000'0000)) {
        r->bHitIoApicIRQ1 = true;
    }

    return (r->bHitIOTR0 || r->bHitIOTR1 || r->bHitIOTR2 || r->bHitIOTR3 || r->bHitIoApicIRQ1);
}

//
// Function: PrintConclution
//
// (i): Print conclution from result.
//
void PrintConclusion(IS_KEYBOARD_RKT_RESULT* r) {
    cout << std::left << setw(36) << setfill('#') << "## CONCLUSION ##" << endl;

    bool bSmiHandlerFound(false);
    if (r->bHitIOTR0) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR0 I/O Address(IOAD) 0x" << std::hex << std::uppercase << r->qwHitIOTR0 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR1) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR1 I/O Address(IOAD) 0x" << std::hex << std::uppercase << r->qwHitIOTR1 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR2) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR2 I/O Address(IOAD) 0x" << std::hex << std::uppercase << r->qwHitIOTR2 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR3) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR3 I/O Address(IOAD) 0x" << std::hex << std::uppercase << r->qwHitIOTR3 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIoApicIRQ1) {
        cout << "I/O APIC - IRQ 1 - DELMOD-bit SMI SET" << endl;
        bSmiHandlerFound = true;
    }

    if (!bSmiHandlerFound)
    {
        cout << "No SMI Handler trapping the keyboard on IOTR0-IOTR3 or IRQ1" << endl;
    }
}