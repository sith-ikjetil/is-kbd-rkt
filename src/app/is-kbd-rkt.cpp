//////////////////////////////////////////////////////////////
//: Filename    : is-kbd-rkt.cpp
//: Date        : 2021-10-18
//: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version     : 1.7
//: Description : Application that calls a Linux Kernel Module 
//:               for detection of an SMM keyboard rootkit.
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
#define VERSION_INFO        "1.7"
#define MAX_N               1'000
#define MIN_N               1
#define DEFAULT_N           50
#define DEFAULT_NO_COLOR    false
#define DEFAULT_RESULT_ONLY false
#define CLR_GREEN           "\033[32m"
#define CLR_WHITE           "\033[37;1m"
#define CLR_RESET           "\033[0m"

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
// (i): data collected during ProcessResult function call
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
// struct: AppSettings
//
// (i): application settings
//
typedef struct AppSettings 
{
    int argc;
    const char** argv;
    int n;
    bool no_color;
    bool result_only;
} AppSettings, *LPAppSettings;

//
// function prototypes
//
void PrintHeader(AppSettings& settings);
void PrintData(AppSettings& settings, IS_KEYBOARD_RKT_DATA* p);
void PrintConclusion(AppSettings& settings, IS_KEYBOARD_RKT_RESULT* r);
void PrintError(AppSettings& settings, string msg);
bool ProcessResult(IS_KEYBOARD_RKT_DATA *p, IS_KEYBOARD_RKT_RESULT *r);
void UpdateAppSettings(AppSettings& settings);
string GetArgVal(string arg, int argc, const char* argv[]);
bool GetHasArg(string arg, int argc, const char* argv[]);

//
// Function: main
//
// (i): entry point
//
int main(int argc, const char* argv[])
{
    AppSettings settings {
        .argc = argc,
        .argv = argv,
        .n = DEFAULT_N,
        .no_color = DEFAULT_NO_COLOR,
        .result_only = DEFAULT_RESULT_ONLY,
    };

    UpdateAppSettings(settings);

    PrintHeader(settings);

    string deviceName("/dev/");
    deviceName += DEVICE_NAME;

    unique_file_descriptor file = open(deviceName.c_str(), O_RDONLY);
    if ( file.IsInvalid() ) {
        stringstream ss;
        ss << "Could not open device: " << deviceName << endl;
        ss << "Error: " << strerror(errno) << endl;
        ss << "Have you checked that device exist?" << endl;
        PrintError(settings, ss.str());
        return EXIT_FAILURE;
    }

    IS_KEYBOARD_RKT_DATA    data{0};
    IS_KEYBOARD_RKT_RESULT  result{0};
    size_t bytesRead(0);
    bool bResult = false;
    int n = settings.n;
    do
    {    
        auto size = read(file, static_cast<void*>(&data), sizeof(IS_KEYBOARD_RKT_DATA));
        if ( size != sizeof(IS_KEYBOARD_RKT_DATA) ) {
            stringstream ss;
            ss << "Not able to read from " << deviceName << endl;
            PrintError(settings, ss.str());
            return EXIT_FAILURE;
        }

        bResult = ProcessResult(&data, &result);

    } while ((--n > 0) && !bResult);
 
    PrintData(settings, &data);

    PrintConclusion(settings, &result);

    return EXIT_SUCCESS;
}

//
// Function: PrintError
//
// (i): prints error message to stdout
//
void PrintError(AppSettings& settings, string msg)
{
    if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
    cout << std::left << setw(36) << setfill('#') << "## ERROR ##" << endl;
    if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
    cout << msg;
}

//
// Function: GetHasArg
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
void UpdateAppSettings(AppSettings& settings) 
{
    //
    // Update .n
    //
    string newN = GetArgVal("-n", settings.argc, settings.argv);
    if ( newN.size() > 0 ) {
        try
        {
            int nn = ItsConvert::ToNumber<int>(ItsString::Replace(newN, "'", ""));
            if (nn > MAX_N ) {
                nn = MAX_N;
            }
            else if ( nn < MIN_N) {
                nn = MIN_N;
            }
            settings.n = nn;
        }
        catch(const std::exception& e)
        {

        }
    }

    //
    // no_color
    //
    settings.no_color = GetHasArg("--no-color", settings.argc, settings.argv);

    //
    // result_only
    //
    settings.result_only = GetHasArg("--result-only", settings.argc, settings.argv);
}

//
// print app ui header
//
void PrintHeader(AppSettings& settings)
{
    if (settings.result_only) {
        return;
    }

    if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
    cout << setw(80) << setfill('#') << std::left << "#" << endl;
    cout << setw(80) << setfill(' ') << std::left << "## Is Keyboard Rootkitted App " << endl;
    cout << "## Author  : " << "Kjetil Kristoffer Solberg <post@ikjetil.no>" << endl;
    cout << "## Version : " << VERSION_INFO << endl;
    cout << "## Usage   : " << endl;
    cout << "##           -n " << settings.n << " = Number of times to run the test." << endl;
    cout << "##           (default = 50, max = 1'000, min = 1)" << endl;
    cout << "##           --no-color = no colored output" << endl;
    cout << "##" << endl;
    if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
}

//
// print driver retrieved data
//
void PrintData(AppSettings& settings, IS_KEYBOARD_RKT_DATA* p) 
{
    if (settings.result_only) {
        return;
    }

    //
    // Base address'
    //
    if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
    cout << std::left << setw(36) << setfill('#') << "## BASE ADDRESS ##" << endl;
    if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
    cout << setfill(' ') << setw(16) << std::left << "APIC" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwApicBaseAddress << endl;
    cout << setfill(' ') << setw(16) << std::left << "IO APIC" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwIoApicBaseAddress << endl;
    cout << setfill(' ') << setw(16) << std::left << "Root Complex" << ": 0x" << std::right << setw(8) << setfill('0') << std::hex << p->dwRootComplexBaseAddress << endl;
    
    //
    // IOTRn
    //
    if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
    cout << std::left << setw(36) << setfill('#') << "## IOTRn ##" << endl;
    if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
    cout << setfill(' ') << setw(16) << std::left << "IOTR0" << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOTRn[0] << ((p->qwIOTRn[0] & 1) ? " TRSE-bit SET" : " TRSE-bit NOT SET") << endl;
    cout << setfill(' ') << setw(16) << std::left << "IOTR1" << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOTRn[1] << ((p->qwIOTRn[1] & 1) ? " TRSE-bit SET" : " TRSE-bit NOT SET") << endl;
    cout << setfill(' ') << setw(16) << std::left << "IOTR2" << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOTRn[2] << ((p->qwIOTRn[2] & 1) ? " TRSE-bit SET" : " TRSE-bit NOT SET") << endl;
    cout << setfill(' ') << setw(16) << std::left << "IOTR3" << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOTRn[3] << ((p->qwIOTRn[3] & 1) ? " TRSE-bit SET" : " TRSE-bit NOT SET") << endl;

    //
    // IOAPIC_IRQn
    //
    if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
    cout << std::left << setw(36) << setfill('#') << "## IOAPIC_IRQn ##" << endl;    
    if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
    cout << setfill(' ') <<setw(16) << std::left << "IO APIC IRQ1" << ": 0x" << std::right << setw(16) << setfill('0') << std::hex << p->qwIOAPIC_REDTBL[1] << (((p->qwIOAPIC_REDTBL[1] & 0b1'0000'0000'0000'0000) == 0) ? " Interrupt Mask-bit NOT SET" : " Interrupt Mask-bit SET") << endl;

    if ( strnlen(p->szErrorMessage, MAX_STRING_BUFFER_SIZE) > 0 ) {
        if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
        cout << std::left << setw(36) << setfill('#') << "## ERROR MESSAGE ##" << CLR_RESET << endl;    
        cout << p->szErrorMessage << endl;
        if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
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
    if (((uint32_t)p->qwIOTRn[0]) == 0x61)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x60;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    if (((uint32_t)p->qwIOTRn[0]) == 0x65)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x64;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    // IOTR1
    if (((uint32_t)p->qwIOTRn[1]) == 0x61)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x60;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    if (((uint32_t)p->qwIOTRn[1]) == 0x65)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x64;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    // IOTR2
    if (((uint32_t)p->qwIOTRn[2]) == 0x61)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x60;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    if (((uint32_t)p->qwIOTRn[2]) == 0x65)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x64;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    // IOTR3
    if (((uint32_t)p->qwIOTRn[3]) == 0x61)
    {
        r->bHitIOTR3 = true;
        r->wHitPortIOTR3 = 0x60;
        r->qwHitIOTR3 = p->qwIOTRn[3];
    }

    if (((uint32_t)p->qwIOTRn[3]) == 0x65)
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
void PrintConclusion(AppSettings& settings, IS_KEYBOARD_RKT_RESULT* r) {
    if (!settings.result_only) {    
        if (!settings.no_color) { cout << CLR_RESET << CLR_GREEN; }
        cout << std::left << setw(36) << setfill('#') << "## CONCLUSION ##" << endl;
        if (!settings.no_color) { cout << CLR_RESET << CLR_WHITE; }
    }

    bool bSmiHandlerFound(false);
    if (r->bHitIOTR0) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR0 port " << std::hex << std::showbase << r->wHitPortIOTR0 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR1) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR1 port " << std::hex << std::showbase << r->wHitPortIOTR1 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR2) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR2 port " << std::hex << std::showbase << r->wHitPortIOTR2 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIOTR3) {
        cout << "Keyboard Is Trapped by SMI Handler on IOTR3 port " << std::hex << std::showbase << r->wHitPortIOTR3 << endl;
        bSmiHandlerFound = true;
    }
    if (r->bHitIoApicIRQ1) {
        cout << "Keyboard Is Trapped by SMI Handler on I/O APIC IRQ1. DELMOD-bit SMI SET." << endl;
        bSmiHandlerFound = true;
    }

    if (!bSmiHandlerFound)
    {
        cout << "No SMI Handler trapping the keyboard on IOTR0-IOTR3 or IRQ1" << endl;
    }
}