// Copyright (c) 2017 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "specabstract.h"


SpecAbstract::SIGNATURE_RECORD _header_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_LINKER,       SpecAbstract::RECORD_NAME_TURBOLINKER,                  "",             "",                     "4D5A50000200000004000F00FFFF0000B80000000000000040001A000000000000000000000000000000000000000000000000000000000000000000....0000BA10000E1FB409CD21B8014CCD219090546869732070726F6772616D206D7573742062652072756E20756E6465722057696E33320D0A24370000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_LINKER,       SpecAbstract::RECORD_NAME_TURBOLINKER,                  "",             "patched",              "4D5A............................................................................................................................BA10000E1FB409CD21B8014CCD219090546869732070726F6772616D206D7573742062652072756E20756E6465722057696E33320D0A24370000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LINKER,       SpecAbstract::RECORD_NAME_MICROSOFTLINKER,              "",             "",                     "4D5A90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000....00000E1FBA0E00B409CD21B8014CCD21546869732070726F6772616D2063616E6E6F742062652072756E20696E20444F53206D6F64652E0D0D0A2400000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LINKER,       SpecAbstract::RECORD_NAME_GENERICLINKER,                "",             "",                     "4D5A90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21546869732070726F6772616D2063616E6E6F742062652072756E20696E20444F53206D6F64652E0D0D0A2400000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LINKER,       SpecAbstract::RECORD_NAME_MICROSOFTLINKER,              "",             "patched",              "4D5A90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000....000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_FASM,                         "",             "",                     "'MZ'80000100000004001000FFFF00004001000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21'This program cannot be run in DOS mode.\r\n$'0000000000000000'PE'0000"}, // TODO patched
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_FASM,                         "",         "",     "4D5A80000100000004001000FFFF00004001000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21546869732070726F6772616D2063616E6E6F742062652072756E20696E20444F53206D6F64652E0D0A240000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_FORMAT,       SpecAbstract::RECORD_NAME_HXS,                          "",             "",                     "4D5A0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000000504500004C010200000000000000000000000000E00001200B010000000000000000000000000000000000000000000000000000000040000000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_FORMAT,        SpecAbstract::RECORD_NAME_MICROSOFTLINKER,              "",             "",                     "4D5A90000300000004000000FFFF0000B8000000000000004000000000000000000000000000000000000000000000000000000000000000000000004000000050450000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_WINUPACK,                     "",             "",                     "4D5A4B45524E454C33322E444C4C0000504500004C010300BEB0114000AD50FF7634EB7C48010F010B014C6F61644C6962726172794100001810000010000000003000000000400000100000000200000400000000003A0004000000000000000060010000020000000000000200000000001000001000000000100000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_MPRESS,                       "",             "Win32",                "4D5A40000100000002000000FFFF0000B8000000000000000A000000000000000E1FBA0E00B409CD21B8014CCD2157696E3332202E4558452E0D0A2440000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,     SpecAbstract::RECORD_NAME_INNOSETUP,                    "1.XX-5.1.X",   "Install",              "4D5A............................................................................................496E6E6F"}, // TODO Versions
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,     SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Uninstall",            "4D5A............................................................................................496E556E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_ANDPAKK2,                     "0.18",         "",                     "'MZ'00'ANDpakk2'00'PE'0000"}
};

SpecAbstract::SIGNATURE_RECORD _entrypoint_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.59",         "exe",                  "60E8000000005883E83D50"}, // mb TODO
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.60-0.69",    "exe",                  "60E8........68........8BE88DBD........33DB033C248BF7"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.71-0.72",    "exe",                  "60E80000000083CDFF31DB5E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.71-0.72",    "dll",                  "807C2408010F85........60E80000000083CDFF31DB5E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.81-3.81+",   "exe",                  "60BE........8DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "0.81-3.81+",   "dll",                  "807C2408010F85........60BE........8DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "3.81+",        "exe",                  "53565755488D35........488DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "3.81+",        "dll",                  "48894C240848895424104C8944241880FA010F85........53565755488D35........488DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_WWPACK32,                     "1.01-1.12",    "",                     "53558be833dbeb60'\r\n\r\nWWPack32 decompression routine version '........'\r\n(c) 1998 Piotr Warezak and Rafal Wierzbicki\r\n\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_BORLANDCPP,                   "",             "",                     "EB10'fb:C++HOOK'90"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_ANDPAKK2,                     "0.18",         "",                     "FCBE........BF........5783CDFF33C9F9EB05A402DB7505"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_ASDPACK,                      "2.0",          "",                     "8B442404565753E8CD010000C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_PEX,                          "0.99",         "",                     "E9"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PACKER,        SpecAbstract::RECORD_NAME_32LITE,                       "0.03a",        "",                     "6006FC1E07BE........6A0468........68"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PROTECTOR,     SpecAbstract::RECORD_NAME_ACPROTECT,                    "2.0.X",        "",                     "68........68........C3C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PROTECTOR,     SpecAbstract::RECORD_NAME_ALEXPROTECTOR,                "1.0",          "",                     "60E8000000005D81ED........E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_PROTECTOR,     SpecAbstract::RECORD_NAME_ALLOY,                        "4.X",          "",                     "9C60E8........33C08BC483C0..938BE3"}

};

SpecAbstract::SIGNATURE_RECORD _overlay_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Install",              "'idska32'1A"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Install",              "'zlb'1A"}, // TODO none
    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Uninstall",            "'zlb'1A"}
};

//SpecAbstract::SIGNATURE_RECORD _import_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_QT,                           "4.X",          "Dynamic",              "'QTCORE4.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_QT,                           "4.X",          "Dynamic/Debug",        "'QTCORED4.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_QT,                           "5.X",          "Dynamic",              "'QT5CORE.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_QT,                           "5.X",          "Dynamic/Debug",        "'QT5CORED.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_TOOL,         SpecAbstract::RECORD_NAME_MINGW,                        "",             "Dynamic",              "'LIBGCC_S_DW2-1.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_VISUALBASIC,                  "6.0",          "",                     "'MSVBVM60.DLL'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_VISUALCPP,                    "",             "",                     "'MSVC"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_C,                            "",             "",                     "'MSVCRT.DLL"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_MFC,                          "",             "Dynamic",              "'MFC'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_DOTNET,                       "",             ""                      "'MSCOREE.DLL'"}
//};

//SpecAbstract::SIGNATURE_RECORD _export_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE64,    SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_EMBARCADEROCPP,               "",             "",                     "'__CPPdebugHook'"}
//};

//// mb TODO Borland DElphi
//SpecAbstract::SCANMEMORY_RECORD _codesectionscan_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_OBJECTPASCAL,                 "",             "",                     "\x06\x73\x74\x72\x69\x6e\x67"                         ,7}, // string
//    {1, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_OBJECTPASCAL,                 "",             "",                     "\x07\x42\x6f\x6f\x6c\x65\x61\x6e"                     ,8}, // Boolean
//    {2, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_OBJECTPASCAL,                 "",             "",                     "\x06\x53\x74\x72\x69\x6e\x67"                         ,7}  // String
//};

//SpecAbstract::SCANMEMORY_RECORD _datasectionscan_records[]={
//    // TODO XXX
//    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_BORLANDCPP,                   "XXXX",         "",                     "\x42\x6f\x72\x6c\x61\x6e\x64\x20\x43\x2b\x2b\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20"                             ,24},                    // Borland C++ - Copyright 1994 Borland Intl.
//    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_EMBARCADEROCPP,               "XXXX",         "",                     "\x45\x6d\x62\x61\x72\x63\x61\x64\x65\x72\x6f\x20\x52\x41\x44\x20\x53\x74\x75\x64\x69\x6f\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20" ,35},    // Embarcadero RAD Studio - Copyright 2009 Embarcadero Technologies, Inc.
//    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_COMPILER,     SpecAbstract::RECORD_NAME_CODEGEARCPP,                  "XXXX",         "",                     "\x43\x6f\x64\x65\x47\x65\x61\x72\x20\x43\x2b\x2b\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20" ,25},                                             // CodeGear C++ - Copyright 2008 Embarcadero Technologies
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_MFC,                          "",             "Static",               "\x43\x4d\x46\x43\x43\x6f\x6d\x4f\x62\x6a\x65\x63\x74" ,13},                                                     // CMFCComObject
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_FLEXLM,                       "",             "",                     "\x40\x28\x23\x29\x20\x46\x4c\x45\x58\x6c\x6d\x20\x76\x20" ,12},                                                 // "@(#) FLEXlm "
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_FLEXNET,                      "",             "",                     "\x40\x28\x23\x29\x20\x46\x4c\x45\x58\x6e\x65\x74\x20\x4c\x69\x63\x65\x6e\x73\x69\x6e\x67\x20\x76" ,24},         // "@(#) FLEXnet Licensing v"
//    {1, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_FLEXNET,                      "",             "",                     "\x40\x28\x23\x29\x20\x46\x6c\x65\x78\x4e\x65\x74\x20\x4c\x69\x63\x65\x6e\x73\x69\x6e\x67\x20\x76" ,24}          // "@(#) FlexNet Licensing v"
//};

//SpecAbstract::SCANMEMORY_RECORD _headerscan_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "",             "",                     "\x24\x49\x64\x3a\x20\x55\x50\x58\x20"                 ,9}, // "$Id: UPX "
//    {1, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_PACKER,       SpecAbstract::RECORD_NAME_UPX,                          "",             "",                     "\x55\x50\x58\x21"                                     ,4} // UPX!

//};


//SpecAbstract::RESOURCES_RECORD _resources_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_VCL,                          "",             "",                     false,          "",         10,     true,       "PACKAGEINFO",      0},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_UNKNOWN,      SpecAbstract::RECORD_NAME_DVCLAL,                       "",             "",                     false,          "",         10,     true,       "DVCLAL",           0},
//    {0, SpecAbstract::RECORD_FILETYPE_PE32,    SpecAbstract::RECORD_TYPE_INSTALLER,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "5.1.X-X.X.X",  "Install",              false,          "",         10,     false,       0,                 11111}
//};


//SpecAbstract::SIGNATURE_RECORD _dot_ansistrings_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_PROTECTOR,    SpecAbstract::RECORD_NAME_DOTFUSCATOR,                   "",             "",                     "'DotfuscatorAttribute'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_LIBRARY,      SpecAbstract::RECORD_NAME_VCL,                           "",             ".NET",                 "'Borland.Vcl.'"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_TOOL,         SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET,       "",             "",                     "'Embarcadero.'"}
//};

//SpecAbstract::SIGNATURE_RECORD _dot_unicodestrings_records[]={
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_PROTECTOR,    SpecAbstract::RECORD_NAME_DOTFUSCATOR,                  "",             "",                     "'DotfuscatorAttribute'"}
//};


SpecAbstract::SpecAbstract(QObject *parent)
{
    Q_UNUSED(parent);
}

QString SpecAbstract::append(QString sResult, QString sString)
{
    if(sString!="")
    {
        if(sResult!="")
        {
            sResult+=",";
        }

        sResult+=sString;
    }

    return sResult;
}

QString SpecAbstract::recordFiletypeIdToString(RECORD_FILETYPES id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_FILETYPE_BINARY:
            sResult=QString("Binary");
            break;

        case RECORD_FILETYPE_MSDOS:
            sResult=QString("MSDOS");
            break;

        case RECORD_FILETYPE_PE:
            sResult=QString("PE");
            break;

        case RECORD_FILETYPE_PE32:
            sResult=QString("PE32");
            break;

        case RECORD_FILETYPE_PE64:
            sResult=QString("PE64");
            break;

        case RECORD_FILETYPE_PEOVERLAY:
            sResult=QString("PE Overlay");
            break;
    }

    return sResult;
}

QString SpecAbstract::recordTypeIdToString(RECORD_TYPES id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_TYPE_PACKER:
            sResult=tr("Packer");
            break;

        case RECORD_TYPE_ARCHIVE:
            sResult=tr("Archive");
            break;

        case RECORD_TYPE_FORMAT:
            sResult=tr("Format");
            break;

        case RECORD_TYPE_PROTECTOR:
            sResult=tr("Protector");
            break;

        case RECORD_TYPE_LINKER:
            sResult=tr("Linker");
            break;

        case RECORD_TYPE_COMPILER:
            sResult=tr("Compiler");
            break;

        case RECORD_TYPE_INSTALLER:
            sResult=tr("Installer");
            break;

        case RECORD_TYPE_CONVERTER:
            sResult=tr("Converter");
            break;

        case RECORD_TYPE_LIBRARY:
            sResult=tr("Library");
            break;

        case RECORD_TYPE_TOOL:
            sResult=tr("Tool");
            break;
    }

    return sResult;
}

QString SpecAbstract::recordNameIdToString(RECORD_NAMES id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_NAME_C:
            sResult=QString("C");
            break;

        case RECORD_NAME_CPP:
            sResult=QString("C++");
            break;

        case RECORD_NAME_MICROSOFTC:
            sResult=QString("Microsoft C");
            break;

        case RECORD_NAME_MICROSOFTCPP:
            sResult=QString("Microsoft C++");
            break;

        case RECORD_NAME_VISUALCPP:
            sResult=QString("Visual C/C++");
            break;

        case RECORD_NAME_VISUALBASIC:
            sResult=QString("Visual Basic");
            break;

        case RECORD_NAME_VBNET:
            sResult=QString("VB .NET");
            break;

        case RECORD_NAME_DOTNET:
            sResult=QString(".NET");
            break;

        case RECORD_NAME_MFC:
            sResult=QString("MFC");
            break;

        case RECORD_NAME_ASM:
            sResult=QString("Asm");
            break;

        case RECORD_NAME_RESOURCE:
            sResult=QString("Resource");
            break;

        case RECORD_NAME_EXPORT:
            sResult=QString("Export");
            break;

        case RECORD_NAME_IMPORT:
            sResult=QString("Import");
            break;

        case RECORD_NAME_UPX:
            sResult=QString("UPX");
            break;

        case RECORD_NAME_YANO:
            sResult=QString("Yano");
            break;

        case RECORD_NAME_ANDPAKK2:
            sResult=QString("ANDpakk2");
            break;

        case RECORD_NAME_ASDPACK:
            sResult=QString("ASDPack");
            break;

        case RECORD_NAME_32LITE:
            sResult=QString("32Lite");
            break;

        case RECORD_NAME_DOTFUSCATOR:
            sResult=QString("Dotfuscator");
            break;

        case RECORD_NAME_CIL:
            //        sResult=QString("Common Intermediate Language");
            sResult=QString("cil");
            break;

        case RECORD_NAME_BORLANDDELPHIDOTNET:
            sResult=QString("Borland Delphi .NET");
            break;

        case RECORD_NAME_EMBARCADERODELPHIDOTNET:
            sResult=QString("Embarcadero Delphi .NET");
            break;

        case RECORD_NAME_DEB:
            sResult=QString("DEB");
            break;

        case RECORD_NAME_ZIP:
            sResult=QString("ZIP");
            break;

        case RECORD_NAME_GZIP:
            sResult=QString("GZIP");
            break;

        case RECORD_NAME_7Z:
            sResult=QString("7-Zip");
            break;

        case RECORD_NAME_PKLITE32:
            sResult=QString("PKLITE32");
            break;

        case RECORD_NAME_MICROSOFTLINKER:
            sResult=QString("Microsoft linker");
            break;

        //    case RECORD_NAME_MICROSOFTCPPCOMPILER:
        //        sResult=QString("Microsoft C/C++ compiler"); // TODO
        //        break;
        case RECORD_NAME_MICROSOFTVISUALSTUDIO:
            sResult=QString("Microsoft Visual Studio");
            break;

        case RECORD_NAME_GENERIC:
            sResult=QString("Generic");
            break;

        case RECORD_NAME_TURBOLINKER:
            sResult=QString("Turbo linker");
            break;

        case RECORD_NAME_BORLANDCPP:
            sResult=QString("Borland C++");
            break;

        case RECORD_NAME_BORLANDCPPBUILDER:
            sResult=QString("Borland C++ Builder");
            break;

        case RECORD_NAME_CODEGEARCPP:
            sResult=QString("CodeGear C++");
            break;

        case RECORD_NAME_CODEGEARCPPBUILDER:
            sResult=QString("CodeGear C++ Builder");
            break;

        case RECORD_NAME_EMBARCADEROCPP:
            sResult=QString("Embarcadero C++");
            break;

        case RECORD_NAME_EMBARCADEROCPPBUILDER:
            sResult=QString("Embarcadero C++ Builder");
            break;

        case RECORD_NAME_BORLANDDELPHI:
            sResult=QString("Borland Delphi");
            break;

        case RECORD_NAME_CODEGEARDELPHI:
            sResult=QString("CodeGear Delphi");
            break;

        case RECORD_NAME_EMBARCADERODELPHI:
            sResult=QString("Embarcadero Delphi");
            break;

        case RECORD_NAME_GNULINKER:
            sResult=QString("GNU ld");
            break;

        case RECORD_NAME_ASPACK:
            sResult=QString("ASPack");
            break;

        case RECORD_NAME_FSG:
            sResult=QString("FSG");
            break;

        case RECORD_NAME_NSPACK:
            sResult=QString("NsPack");
            break;

        case RECORD_NAME_PEX:
            sResult=QString("PeX");
            break;

        case RECORD_NAME_INNOSETUP:
            sResult=QString("Inno Setup");
            break;

        case RECORD_NAME_FASM:
            sResult=QString("FASM");
            break;

        case RECORD_NAME_GCC:
            sResult=QString("GCC");
            break;

        case RECORD_NAME_FPC:
            sResult=QString("Free Pascal");
            break;

        case RECORD_NAME_MASM:
            sResult=QString("MASM");
            break;

        case RECORD_NAME_MASM32:
            sResult=QString("MASM32");
            break;

        case RECORD_NAME_HXS:
            sResult=QString("HXS");
            break;

        case RECORD_NAME_WINUPACK:
            sResult=QString("(Win)Upack");
            break;

        case RECORD_NAME_MPRESS:
            sResult=QString("MPRESS");
            break;

        case RECORD_NAME_VCL:
            sResult=QString("Visual Component Library");
            break;

        case RECORD_NAME_VCLPACKAGEINFO:
            sResult=QString("VCL PackageInfo");
            break;

        case RECORD_NAME_DVCLAL:
            sResult=QString("DVCLAL");
            break;

        case RECORD_NAME_OBJECTPASCAL:
            sResult=QString("Object Pascal");
            break;

        case RECORD_NAME_BORLANDOBJECTPASCAL:
            sResult=QString("Borland Object Pascal");
            break;

        case RECORD_NAME_CODEGEAROBJECTPASCAL:
            sResult=QString("Codegear Object Pascal");
            break;

        case RECORD_NAME_EMBARCADEROOBJECTPASCAL:
            sResult=QString("Embarcadero Object Pascal");
            break;

        case RECORD_NAME_QT:
            sResult=QString("Qt");
            break;

        case RECORD_NAME_PYTHON:
            sResult=QString("Python");
            break;

        case RECORD_NAME_MINGW:
            sResult=QString("MinGW");
            break;

        case RECORD_NAME_CYGWIN:
            sResult=QString("Cygwin");
            break;

        case RECORD_NAME_DYAMAR:
            sResult=QString("DYAMAR");
            break;

        case RECORD_NAME_MSYS:
            sResult=QString("Msys");
            break;

        case RECORD_NAME_MSYS2:
            sResult=QString("MSYS2");
            break;

        case RECORD_NAME_WWPACK32:
            sResult=QString("WWPack32");
            break;

        case RECORD_NAME_FLEXLM:
            sResult=QString("Flex License Manager");
            break;

        case RECORD_NAME_FLEXNET:
            sResult=QString("FlexNet Licensing");
            break;

        case RECORD_NAME_ACPROTECT:
            sResult=QString("ACProtect");
            break;

        case RECORD_NAME_ALEXPROTECTOR:
            sResult=QString("Alex Protector");
            break;

        case RECORD_NAME_ALLOY:
            sResult=QString("Alloy");
            break;

        case RECORD_NAME_ENIGMA:
            sResult=QString("ENIGMA");
            break;
    }

    return sResult;
}

QList<SpecAbstract::SCAN_STRUCT> SpecAbstract::scanBinary(QBinary *pBinary, SpecAbstract::SCAN_OPTIONS *pOptions,qint64 nStartOffset,SpecAbstract::ID parentId)
{
    Q_UNUSED(pBinary);
    Q_UNUSED(pOptions);
    Q_UNUSED(nStartOffset);
    Q_UNUSED(parentId);

    QList<SpecAbstract::SCAN_STRUCT> list;

    return list;
}

QList<SpecAbstract::SCAN_STRUCT> SpecAbstract::scanPE(QPE *pPE, SpecAbstract::SCAN_OPTIONS *pOptions,qint64 nStartOffset,SpecAbstract::ID parentId)
{
    Q_UNUSED(pOptions);
    Q_UNUSED(nStartOffset);
    Q_UNUSED(parentId);

    QList<SpecAbstract::SCAN_STRUCT> list;

    SpecAbstract::SCAN_STRUCT record= {0};

    record.id.filetype=pPE->is64()?RECORD_FILETYPE_PE64:RECORD_FILETYPE_PE32;
    record.name=RECORD_NAME_UNKNOWN;

    list.append(record);

    return list;
}

bool SpecAbstract::unpack(SpecAbstract::UNPACK_OPTIONS *pUnpOptions, QIODevice *pDevice, QString sOutFileName)
{
    Q_UNUSED(pUnpOptions);
    Q_UNUSED(pDevice);
    Q_UNUSED(sOutFileName);
    return false;
}

SpecAbstract::UNPACK_OPTIONS SpecAbstract::getPossibleUnpackOptions(QIODevice *pDevice)
{
    UNPACK_OPTIONS result= {0};

    QSet<QBinary::FILE_TYPES> setFileTypes=QBinary::getFileTypes(pDevice);

    if(setFileTypes.contains(QBinary::FILE_TYPE_PE32)||setFileTypes.contains(QBinary::FILE_TYPE_PE64))
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            if(pe.isValid())
            {
                result.bCopyOverlay=pe.isOverlayPresent();
            }
        }
    }

    return result;
}

QString SpecAbstract::createResultString(const SpecAbstract::SCAN_STRUCT *pScanStruct)
{
    return QString("%1: %2(%3)[%4]").arg(SpecAbstract::recordTypeIdToString(pScanStruct->type)).arg(SpecAbstract::recordNameIdToString(pScanStruct->name)).arg(pScanStruct->sVersion).arg(pScanStruct->sInfo);
}

QString SpecAbstract::createResultString2(const SpecAbstract::SCAN_STRUCT *pScanStruct)
{
    QString sResult=QString("%1: %2").arg(SpecAbstract::recordTypeIdToString(pScanStruct->type)).arg(SpecAbstract::recordNameIdToString(pScanStruct->name));

    if(pScanStruct->sVersion!="")
    {
        sResult+=QString("(%1)").arg(pScanStruct->sVersion);
    }

    if(pScanStruct->sInfo!="")
    {
        sResult+=QString("[%1]").arg(pScanStruct->sInfo);
    }

    return sResult;
}

// TODO VI
QString SpecAbstract::findEnigmaVersion(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    QString sResult;

    QBinary binary(pDevice);

    qint64 _nOffset=binary.find_array(nOffset,nSize,"\x00\x00\x00\x45\x4e\x49\x47\x4d\x41",9); // \x00\x00\x00ENIGMA

    if(_nOffset!=-1)
    {
        quint8 nMajor=binary.read_uint8(_nOffset+9);
        quint8 nMinor=binary.read_uint8(_nOffset+10);
        quint16 nYear=binary.read_uint16(_nOffset+11);
        quint16 nMonth=binary.read_uint16(_nOffset+13);
        quint16 nDay=binary.read_uint16(_nOffset+15);
        quint16 nHour=binary.read_uint16(_nOffset+17);
        quint16 nMin=binary.read_uint16(_nOffset+19);
        quint16 nSec=binary.read_uint16(_nOffset+21);

        sResult=QString("%1.%2 build %3.%4.%5 %6:%7:%8").arg(nMajor).arg(nMinor,2,10,QChar('0')).arg(nYear,4,10,QChar('0')).arg(nMonth,2,10,QChar('0')).arg(nDay,2,10,QChar('0')).arg(nHour,2,10,QChar('0')).arg(nMin,2,10,QChar('0')).arg(nSec,2,10,QChar('0'));
    }

    return sResult;
}

SpecAbstract::PEINFO_STRUCT SpecAbstract::getPEInfo(QIODevice *pDevice)
{
    QElapsedTimer timer;
    timer.start();

    PEINFO_STRUCT result= {0};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        result.bIs64=pe.is64();

        result.basic_info.parentId.nOffset=-1; // TODO
        result.basic_info.parentId.filetype=SpecAbstract::RECORD_FILETYPE_UNKNOWN;
        result.basic_info.id.nOffset=0;
        result.basic_info.id.filetype=result.bIs64?RECORD_FILETYPE_PE64:RECORD_FILETYPE_PE32;
        result.basic_info.nSize=pDevice->size();
        result.basic_info.sHeaderSignature=pe.getSignature(0,150);

        result.sEntryPointSignature=pe.getSignature(pe.getEntryPointOffset(),150);

        result.fileHeader=pe.getFileHeader();
        result.nOverlaySize=pe.getOverlaySize();

        if(result.nOverlaySize)
        {
            result.sOverlaySignature=pe.getSignature(pe.getOverlayOffset(),150);
        }

        if(result.bIs64)
        {
            result.optional_header.optionalHeader64=pe.getOptionalHeader64();
        }
        else
        {
            result.optional_header.optionalHeader32=pe.getOptionalHeader32();
        }

        result.listSections=pe.getSectionHeaders();
        result.listImports=pe.getImports();
        result.export_header=pe.getExport();
        result.listResources=pe.getResources();
        result.listRichSignatures=pe.getRichSignatureRecords();
        result.cliInfo=pe.getCliInfo();

        //        if(result.listSections.count()>=1)
        //        {
        //            result.nCodeSectionOffset=result.listSections.at(0).PointerToRawData;
        //            result.nCodeSectionSize=result.listSections.at(0).SizeOfRawData;
        //        }

        //        result.nHeaderOffset=0;
        //        result.nHeaderSize=qMin(result.basic_info.nSize,(qint64)1024);

        result.nEntryPointAddress=result.bIs64?result.optional_header.optionalHeader64.AddressOfEntryPoint:result.optional_header.optionalHeader32.AddressOfEntryPoint;
        result.nImageBaseAddress=result.bIs64?result.optional_header.optionalHeader64.ImageBase:result.optional_header.optionalHeader32.ImageBase;
        result.nMinorLinkerVersion=result.bIs64?result.optional_header.optionalHeader64.MinorLinkerVersion:result.optional_header.optionalHeader32.MinorLinkerVersion;
        result.nMajorLinkerVersion=result.bIs64?result.optional_header.optionalHeader64.MajorLinkerVersion:result.optional_header.optionalHeader32.MajorLinkerVersion;


        result.nEntryPointSection=pe.getEntryPointSection();
        result.nResourceSection=pe.getResourcesSection();
        result.nImportSection=pe.getImportSection();
        result.nCodeSection=pe.getNormalCodeSection();
        result.nDataSection=pe.getNormalDataSection();
        result.nConstDataSection=pe.getConstDataSection();

        if(result.nEntryPointSection!=-1)
        {
            result.sEntryPointSectionName=QString((char *)result.listSections.at(result.nEntryPointSection).Name);
        }

        //        result.mmCodeSectionSignatures=memoryScan(pDevice,nFirstSectionOffset,qMin((qint64)0x10000,nFirstSectionSize),_memory_records,sizeof(_memory_records),_filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        if(result.nCodeSection!=-1)
        //        {
        //            memoryScan(&result.mapCodeSectionScanDetects,pDevice,result.listSections.at(result.nCodeSection).PointerToRawData,result.listSections.at(result.nCodeSection).SizeOfRawData,_codesectionscan_records,sizeof(_codesectionscan_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        result.nHeaderOffset=0;
        result.nHeaderSize=qMin(result.basic_info.nSize,(qint64)2048);

        if(result.nCodeSection!=-1)
        {
            result.nCodeSectionOffset=result.listSections.at(result.nCodeSection).PointerToRawData; // mb TODO for image
            result.nCodeSectionSize=result.listSections.at(result.nCodeSection).SizeOfRawData; // TODO limit?
        }

        if(result.nDataSection!=-1)
        {
            result.nDataSectionOffset=result.listSections.at(result.nDataSection).PointerToRawData;
            result.nDataSectionSize=result.listSections.at(result.nDataSection).SizeOfRawData;
        }

        if(result.nConstDataSection!=-1)
        {
            result.nConstDataSectionOffset=result.listSections.at(result.nConstDataSection).PointerToRawData;
            result.nConstDataSectionSize=result.listSections.at(result.nConstDataSection).SizeOfRawData;
        }

        if(result.nEntryPointSection!=-1)
        {
            result.nEntryPointSectionOffset=result.listSections.at(result.nEntryPointSection).PointerToRawData;
            result.nEntryPointSectionSize=result.listSections.at(result.nEntryPointSection).SizeOfRawData;
        }

        if(result.nImportSection!=-1)
        {
            result.nImportSectionOffset=result.listSections.at(result.nImportSection).PointerToRawData;
            result.nImportSectionSize=result.listSections.at(result.nImportSection).SizeOfRawData;
        }

        //        if(result.nCodeSectionSize)
        //        {
        //            memoryScan(&result.mapCodeSectionScanDetects,pDevice,result.nCodeSectionOffset,result.nCodeSectionSize,_codesectionscan_records,sizeof(_codesectionscan_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        if(result.nDataSectionSize)
        //        {
        //            memoryScan(&result.mapDataSectionScanDetects,pDevice,result.nDataSectionOffset,result.nDataSectionSize,_datasectionscan_records,sizeof(_datasectionscan_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        // TODO Check if resources exists

        //        memoryScan(&result.mapHeaderScanDetects,pDevice,0,qMin(result.basic_info.nSize,(qint64)1024),_headerscan_records,sizeof(_headerscan_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);


        signatureScan(&result.mapHeaderDetects,result.basic_info.sHeaderSignature,_header_records,sizeof(_header_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        signatureScan(&result.mapEntryPointDetects,result.sEntryPointSignature,_entrypoint_records,sizeof(_entrypoint_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        signatureScan(&result.mapOverlayDetects,result.sOverlaySignature,_overlay_records,sizeof(_overlay_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);

        //        for(int i=0;i<result.listImports.count();i++)
        //        {
        //            signatureScan(&result._mapImportDetects,QBinary::stringToHex(result.listImports.at(i).sName.toUpper()),_import_records,sizeof(_import_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        for(int i=0;i<result.export_header.listPositions.count();i++)
        //        {
        //            signatureScan(&result.mapExportDetects,QBinary::stringToHex(result.export_header.listPositions.at(i).sFunctionName),_export_records,sizeof(_export_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        resourcesScan(&result.mapResourcesDetects,&result.listResources,_resources_records,sizeof(_resources_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);


        //        for(int i=0;i<result.cliInfo.listAnsiStrings.count();i++)
        //        {
        //            signatureScan(&result.mapDotAnsistringsDetects,QBinary::stringToHex(result.cliInfo.listAnsiStrings.at(i)),_dot_ansistrings_records,sizeof(_dot_ansistrings_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        for(int i=0;i<result.cliInfo.listUnicodeStrings.count();i++)
        //        {
        //            signatureScan(&result.mapDotUnicodestringsDetects,QBinary::stringToHex(result.cliInfo.listUnicodeStrings.at(i)),_dot_unicodestrings_records,sizeof(_dot_unicodestrings_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        SpecAbstract::SCAN_STRUCT _record={0};

        //        _record.id=result.basic_info.id;
        //        _record.nSize=result.basic_info.nSize;
        ////        _record.parentId=parentId; // TODO
        //        _record.parentId=result.basic_info.parentId;
        //        _record.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
        //        _record.name=SpecAbstract::RECORD_NAME_UNKNOWN;
        //        result.listDetects.append(_record);

        //        SpecAbstract::BASIC_PE_INFO bpi={0};

        //        bpi.nEntryPoint=result.nEntryPointAddress;

        //        _record.baExtra=SpecAbstract::_BasicPEInfoToArray(&bpi);

        handle_import(pDevice,&result);


        handle_protection(pDevice,&result);
        // TODO Free Pascal Compiler

        handle_libraries(pDevice,&result);

        handle_Microsoft(pDevice,&result);
        handle_Borland(pDevice,&result);
        handle_Tools(pDevice,&result);
        handle_Installers(pDevice,&result);

        // TODO
        // Filter

        // http://www.on-time.com/rtos-32-docs/rttarget-32/programming-manual/compiling/microsoft-visual-c.htm
        //        handle_Rich(&result);
        //        handle_ENIGMA(pDevice,&result);

        //        fixDetects(&result);



        //        // https://github.com/tmcdos/VCL-explorer
        //        // https://en.wikipedia.org/wiki/C%2B%2BBuilder
        //        // http://stackoverflow.com/questions/3357485/delphi-signature-in-exe-files
        //        // http://stackoverflow.com/questions/18720045/what-are-the-list-of-all-possible-values-for-dvclal
        //        // https://synopse.info/forum/viewtopic.php?id=83
        //        // sysutils.pas TPackageInfoHeader
        //        handle(&result,SpecAbstract::RECORD_NAME_MICROSOFTLINKER,RESULT_PRIO_RICH,RESULT_PRIO_HEADER);
        //        handle(&result,SpecAbstract::RECORD_NAME_VISUALCPP,RESULT_PRIO_RICH,RESULT_PRIO_IMPORT);
        //        handle(&result,SpecAbstract::RECORD_NAME_MASM,RESULT_PRIO_RICH);
        //        handle(&result,SpecAbstract::RECORD_NAME_VISUALBASIC,RESULT_PRIO_RICH,RESULT_PRIO_IMPORT);
        //        handle(&result,SpecAbstract::RECORD_NAME_MFC,RESULT_PRIO_IMPORT,RESULT_PRIO_DATASECTIONSCAN);
        //        handle(&result,SpecAbstract::RECORD_NAME_OBJECTPASCAL,RESULT_PRIO_CODESECTIONSCAN);

        //        handle(&result,SpecAbstract::RECORD_NAME_QT,RESULT_PRIO_IMPORT);
        //        handle(&result,SpecAbstract::RECORD_NAME_FASM,RESULT_PRIO_HEADER);

        //        handle(&result,SpecAbstract::RECORD_NAME_MINGW,RESULT_PRIO_IMPORT);
        //        handle(&result,SpecAbstract::RECORD_NAME_UPX,RESULT_PRIO_ENTRYPOINT,RESULT_PRIO_HEADERSCAN);
        //        handle(&result,SpecAbstract::RECORD_NAME_WWPACK32,RESULT_PRIO_ENTRYPOINT);

        //        // mb TODO 1 function for Borland
        //        handle(&result,SpecAbstract::RECORD_NAME_TURBOLINKER,RESULT_PRIO_HEADER);
        //        handle(&result,SpecAbstract::RECORD_NAME_BORLANDCPP,RESULT_PRIO_DATASECTIONSCAN,RESULT_PRIO_ENTRYPOINT);
        //        handle(&result,SpecAbstract::RECORD_NAME_CODEGEARCPP,RESULT_PRIO_DATASECTIONSCAN);
        //        handle(&result,SpecAbstract::RECORD_NAME_EMBARCADEROCPP,RESULT_PRIO_DATASECTIONSCAN,RESULT_PRIO_EXPORT);
        //        handle(&result,SpecAbstract::RECORD_NAME_VCL,RESULT_PRIO_RESOURCES,RESULT_PRIO_DOTANSISTRINGS);
        //        handle(&result,SpecAbstract::RECORD_NAME_BORLANDDELPHIDOTNET,RESULT_PRIO_DOTANSISTRINGS,RESULT_PRIO_DOTUNICODESTRINGS);
        //        handle(&result,SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET,RESULT_PRIO_DOTANSISTRINGS,RESULT_PRIO_DOTUNICODESTRINGS);


        //        handle(&result,SpecAbstract::RECORD_NAME_INNOSETUP,RESULT_PRIO_HEADER,RESULT_PRIO_RESOURCES,RESULT_PRIO_OVERLAY);

        //        handle(&result,SpecAbstract::RECORD_NAME_DOTFUSCATOR,RESULT_PRIO_DOTANSISTRINGS);


        //        handle(&result,SpecAbstract::RECORD_NAME_FLEXLM,RESULT_PRIO_DATASECTIONSCAN);
        //        handle(&result,SpecAbstract::RECORD_NAME_FLEXNET,RESULT_PRIO_DATASECTIONSCAN);


        ////        handle(&result,SpecAbstract::RECORD_NAME_ENIGMA,RESULT_PRIO_SPECIAL);





        //        fixResult(pDevice,&result);

        //        if(QPE::isResourcePresent(10,"DVCLAL",&result.listResources)||QPE::isResourcePresent(10,"PACKAGEINFO",&result.listResources))
        //        {
        //            SpecAbstract::SCAN_STRUCT record={0};

        //            record.id.filetype=_record.id.filetype;
        //            record.id.nOffset=_record.id.nOffset;
        //            record.nSize=_record.nSize;
        //            record.parentId=_record.parentId;
        //            record.type=SpecAbstract::RECORD_TYPE_LIBRARY;
        //            record.name=SpecAbstract::RECORD_NAME_VCL;

        //            result.listResultLibraries.append(record);

        //            qDebug("DVCLAL|PACKAGEINFO");
        //        }



        result.listDetects.append(result.mapResultLinkers.values());
        result.listDetects.append(result.mapResultCompilers.values());
        result.listDetects.append(result.mapResultLibraries.values());
        result.listDetects.append(result.mapResultTools.values());
        result.listDetects.append(result.mapResultProtectors.values());
        result.listDetects.append(result.mapResultPackers.values());
        result.listDetects.append(result.mapResultInstallers.values());

        //        if(!result.listDetects.count())
        //        {
        //            SCANS_STRUCT ssUnknown={0};

        //            ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
        //            ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

        //            result.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
        //        }
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

void SpecAbstract::handle_Rich(SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    int nRichSignaturesCount=pPEInfo->listRichSignatures.count();

    if(nRichSignaturesCount>=1)
    {
        quint32 _nRich1=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-1).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-1).nVersion;
        quint32 _nRich2=0;
        quint32 _nRich3=0;
        quint32 _nRich4=0;

        if(nRichSignaturesCount>=2)
        {
            _nRich2=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-2).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-2).nVersion;
        }

        if(nRichSignaturesCount>=3)
        {
            _nRich3=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-3).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-3).nVersion;
        }

        if(nRichSignaturesCount>=4)
        {
            _nRich4=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-4).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-4).nVersion;
        }

        SpecAbstract::SCANS_STRUCT ssRich1=SpecAbstract::getRichSignatureDescription(_nRich1);
        SpecAbstract::SCANS_STRUCT ssRich2=SpecAbstract::getRichSignatureDescription(_nRich2);
        SpecAbstract::SCANS_STRUCT ssRich3=SpecAbstract::getRichSignatureDescription(_nRich3);
        SpecAbstract::SCANS_STRUCT ssRich4=SpecAbstract::getRichSignatureDescription(_nRich4);

        SpecAbstract::SCANS_STRUCT *pssLinker=0;
        SpecAbstract::SCANS_STRUCT *pssCompiler=0;

        if(ssRich1.type==SpecAbstract::RECORD_TYPE_LINKER)
        {
            pssLinker=&ssRich1;
        }
        else if(ssRich2.type==SpecAbstract::RECORD_TYPE_LINKER)
        {
            pssLinker=&ssRich2;
        }
        else if(ssRich3.type==SpecAbstract::RECORD_TYPE_LINKER)
        {
            pssLinker=&ssRich3;
        }
        else if(ssRich4.type==SpecAbstract::RECORD_TYPE_LINKER)
        {
            pssLinker=&ssRich4;
        }

        if(ssRich1.type==SpecAbstract::RECORD_TYPE_COMPILER)
        {
            pssCompiler=&ssRich1;
        }
        else if(ssRich2.type==SpecAbstract::RECORD_TYPE_COMPILER)
        {
            pssCompiler=&ssRich2;
        }
        else if(ssRich3.type==SpecAbstract::RECORD_TYPE_COMPILER)
        {
            pssCompiler=&ssRich3;
        }
        else if(ssRich4.type==SpecAbstract::RECORD_TYPE_COMPILER)
        {
            pssCompiler=&ssRich4;
        }

        SpecAbstract::SCANS_STRUCT recordLinker= {0};

        //        recordLinker.id=pPEInfo->basic_info.id;
        //        recordLinker.nSize=pPEInfo->basic_info.nSize;
        //        recordLinker.parentId=pPEInfo->basic_info.parentId;
        recordLinker.type=SpecAbstract::RECORD_TYPE_LINKER;
        recordLinker.name=SpecAbstract::RECORD_NAME_MICROSOFTLINKER;

        if(pssLinker)
        {
            recordLinker.sVersion=pssLinker->sVersion;
            recordLinker.sInfo=pssLinker->sInfo;
        }
        else
        {
            //            recordLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
        }

        pPEInfo->mapRichDetects.insert(recordLinker.name,recordLinker);

        if(pssCompiler)
        {
            SpecAbstract::SCANS_STRUCT recordCompiler= {0};

            //            recordCompiler.id=pPEInfo->basic_info.id;
            //            recordCompiler.nSize=pPEInfo->basic_info.nSize;
            //            recordCompiler.parentId=pPEInfo->basic_info.parentId;
            recordCompiler.type=pssCompiler->type;
            recordCompiler.name=pssCompiler->name;
            recordCompiler.sVersion=pssCompiler->sVersion;
            recordCompiler.sInfo=pssCompiler->sInfo;

            // VB 6.0
            if(recordCompiler.name==SpecAbstract::RECORD_NAME_VISUALBASIC)
            {
                if(nRichSignaturesCount>1)
                {
                    recordCompiler.sInfo="Native";
                }
                else
                {
                    recordCompiler.sInfo="P-Code";
                }
            }

            pPEInfo->mapRichDetects.insert(recordCompiler.name,recordCompiler);
        }
    }
}

SpecAbstract::SCANS_STRUCT SpecAbstract::getScansStruct(quint32 nVariant, SpecAbstract::RECORD_FILETYPES filetype, SpecAbstract::RECORD_TYPES type, SpecAbstract::RECORD_NAMES name, QString sVersion, QString sInfo, qint64 nOffset)
{
    SCANS_STRUCT result= {0};

    result.nVariant=nVariant;
    result.filetype=filetype;
    result.type=type;
    result.name=name;
    result.sVersion=sVersion;
    result.sInfo=sInfo;
    result.nOffset=nOffset;

    return result;
}

void SpecAbstract::handle_import(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    // Import Check

    QSet<QString> stDetects;


    if(pPEInfo->listImports.count()>=1)
    {
        if(pPEInfo->listImports.at(0).sName.toUpper()=="KERNEL32.DLL")
        {
            if(pPEInfo->listImports.at(0).listPositions.count()==1)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA"))
                {
                    stDetects.insert("kernel32_asdpack"); // Kernel32.dll
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==2)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress"))
                {
                    stDetects.insert("kernel32_upx0exe");   // 0.59-0.93
                    stDetects.insert("kernel32_upx1dll");

                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_fsg");
                        stDetects.insert("kernel32_winupack");
                        stDetects.insert("kernel32_andpakk");
                        stDetects.insert("kernel32_bero");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==3)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_upx1exe");   // 0.94-1.93
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect"))
                {
                    stDetects.insert("kernel32_upx2dll");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="LoadLibraryA"))
                {
                    stDetects.insert("kernel32_aspack");
                    stDetects.insert("kernel32_asprotect");
                    stDetects.insert("kernel32_alexprotector");
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==4)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_upx2exe");   // 1.94-2.03
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="LoadLibraryA"))
                {
                    stDetects.insert("kernel32_enigma2");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_acprotect");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_32lite");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==5)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree"))
                {
                    stDetects.insert("kernel32_upx3dll");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_pex");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==6)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_upx3exe");  // 2.90-3.xx
                    stDetects.insert("kernel32_nspack");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="LoadLibraryA"))
                {
                    stDetects.insert("kernel32_enigma1");
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==13)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="CreateFileA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CloseHandle")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="WriteFile")&&
                        (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetSystemDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(10).sName=="SetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetWindowsDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(12).sName=="lstrcatA"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_alloy0");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==14)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="CreateFileA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CloseHandle")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="WriteFile")&&
                        (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetSystemDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(10).sName=="SetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetWindowsDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(12).sName=="lstrcatA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(13).sName=="FreeLibrary"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_alloy1");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="ResetEvent")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="CreateThread")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="FindResourceA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="LocalAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="LocalFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetCommandLineW")&&
                        (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GlobalFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(10).sName=="SetEvent")&&
                        (pPEInfo->listImports.at(0).listPositions.at(11).sName=="CreateProcessA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(12).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GetTickCount"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_dyamar");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==15)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="CreateFileA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CloseHandle")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="WriteFile")&&
                        (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetSystemDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(10).sName=="SetFileTime")&&
                        (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetWindowsDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(12).sName=="lstrcatA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(13).sName=="FreeLibrary")&&
                        (pPEInfo->listImports.at(0).listPositions.at(14).sName=="GetTempPathA"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_alloy2");
                    }
                }
            }
        }
    }

    if(pPEInfo->listImports.count()>=2)
    {
        if(pPEInfo->listImports.at(1).sName.toUpper()=="USER32.DLL")
        {
            if(pPEInfo->listImports.at(1).listPositions.count()==1)
            {
                if(pPEInfo->listImports.at(1).listPositions.at(0).sName=="MessageBoxA")
                {
                    stDetects.insert("user32_enigma");

                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("user32_acprotect");
                        stDetects.insert("user32_alexprotector");
                        stDetects.insert("user32_pex");
                    }
                }
            }
            else if(pPEInfo->listImports.at(1).listPositions.count()==11)
            {
                if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetClientRect")&&
                        (pPEInfo->listImports.at(1).listPositions.at(1).sName=="GetWindowRect")&&
                        (pPEInfo->listImports.at(1).listPositions.at(2).sName=="BeginDeferWindowPos")&&
                        (pPEInfo->listImports.at(1).listPositions.at(3).sName=="DeferWindowPos")&&
                        (pPEInfo->listImports.at(1).listPositions.at(4).sName=="EndDeferWindowPos")&&
                        (pPEInfo->listImports.at(1).listPositions.at(5).sName=="ShowWindow")&&
                        (pPEInfo->listImports.at(1).listPositions.at(6).sName=="UpdateWindow")&&
                        (pPEInfo->listImports.at(1).listPositions.at(7).sName=="CreateWindowExA")&&
                        (pPEInfo->listImports.at(1).listPositions.at(8).sName=="LoadBitmapA")&&
                        (pPEInfo->listImports.at(1).listPositions.at(9).sName=="SendMessageA")&&
                        (pPEInfo->listImports.at(1).listPositions.at(10).sName=="DestroyWindow"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("user32_dyamar");
                    }
                }
            }
        }
    }

    qDebug()<<stDetects;

    // TODO 32/64
    if(stDetects.contains("kernel32_andpakk"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ANDPAKK2,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ANDPAKK2,"0.18","",0));
    }

    if(stDetects.contains("kernel32_asdpack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ASDPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ASDPACK,"2.0","",0));
    }

    if(stDetects.contains("kernel32_32lite"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_32LITE,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_32LITE,"0.03a","",0));
    }

    if(stDetects.contains("kernel32_aspack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ASPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ASPACK,"","",0));
    }

    if(stDetects.contains("kernel32_pex")&&stDetects.contains("user32_pex"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PEX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_PEX,"0.99","",0));
    }

    if(stDetects.contains("kernel32_enigma1")&&stDetects.contains("user32_enigma"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ENIGMA,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ENIGMA,"","",0)); // TODO version
    }

    if(stDetects.contains("kernel32_enigma2")&&stDetects.contains("user32_enigma"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ENIGMA,getScansStruct(1,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ENIGMA,"","",0)); // TODO version
    }

    if(stDetects.contains("kernel32_acprotect")&&stDetects.contains("user32_acprotect"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ACPROTECT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ACPROTECT,"1.X-2.X","",0));
    }

    if(stDetects.contains("kernel32_alexprotector")&&stDetects.contains("user32_alexprotector"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ALEXPROTECTOR,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ALEXPROTECTOR,"1.0","",0));
    }

    if(stDetects.contains("kernel32_alloy0"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ALLOY,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_ALLOY,"4.X","",0));
    }

    if(stDetects.contains("kernel32_alloy1"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ALLOY,getScansStruct(1,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_ALLOY,"4.X","",0));
    }

    if(stDetects.contains("kernel32_alloy2"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ALLOY,getScansStruct(2,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_ALLOY,"4.X","",0));
    }

    if(stDetects.contains("kernel32_fsg"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_FSG,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_FSG,"","",0));
    }

    if(stDetects.contains("kernel32_dyamar")&&stDetects.contains("user32_dyamar"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_DYAMAR,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_DYAMAR,"1.3.5","",0));
    }


    if(stDetects.contains("kernel32_upx0exe")||
            stDetects.contains("kernel32_upx1dll"))
    {
        // TODO isDll;
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"0.59-1.93","",0));
    }
    else if(stDetects.contains("kernel32_upx1exe"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"0.94-1.93","exe",0));
    }
    else if(stDetects.contains("kernel32_upx2exe"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"1.94-2.03","exe",0));
    }
    else if(stDetects.contains("kernel32_upx2dll"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"1.94-2.03","dll",0));
    }
    else if(stDetects.contains("kernel32_upx3exe"))
    {
        // TODO 32 64
        // RECORD_FILETYPE_PE
        // Version
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"2.90-3.xx","exe",0));
    }
    else if(stDetects.contains("kernel32_upx3dll"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"2.90-3.xx","dll",0));
    }
}

void SpecAbstract::handle_protection(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            VI_STRUCT viUPX=get_UPX_vi(pDevice,pPEInfo);

            // UPX
            // TODO 32-64
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_UPX)||
                    pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_UPX))
            {
                if(viUPX.sVersion!="")
                {
                    SpecAbstract::SCANS_STRUCT recordUPX= {0};

                    recordUPX.type=RECORD_TYPE_PACKER;
                    recordUPX.name=RECORD_NAME_UPX;
                    recordUPX.sVersion=viUPX.sVersion;
                    recordUPX.sInfo=viUPX.sInfo;

                    pPEInfo->mapResultPackers.insert(recordUPX.name,scansToScan(&(pPEInfo->basic_info),&recordUPX));
                }
            }

            // ENIGMA
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ENIGMA))
            {
                int nVariant=pPEInfo->mapImportDetects.value(RECORD_NAME_ENIGMA).nVariant;

                if((pPEInfo->nImportSectionOffset)&&(pPEInfo->nImportSectionSize))
                {
                    qint64 nSectionOffset=pPEInfo->nImportSectionOffset;
                    qint64 nSectionSize=pPEInfo->nImportSectionSize;

                    bool bDetect=false;

                    SpecAbstract::SCANS_STRUCT recordEnigma= {0};

                    recordEnigma.type=SpecAbstract::RECORD_TYPE_PROTECTOR;
                    recordEnigma.name=SpecAbstract::RECORD_NAME_ENIGMA;

                    // mb TODO ENIGMA string
                    if((!bDetect)&&(nVariant==0))
                    {
                        qint64 nOffset=pe.find_array(nSectionOffset,nSectionSize," *** Enigma protector v",23);

                        if(nOffset!=-1)
                        {
                            recordEnigma.sVersion=pe.read_ansiString(nOffset+23).section(" ",0,0);
                            bDetect=true;
                        }
                    }

                    if((!bDetect)&&(nVariant==1))
                    {
                        QString sEnigmaVersion=findEnigmaVersion(pDevice,nSectionOffset,nSectionSize);

                        if(sEnigmaVersion!="")
                        {
                            recordEnigma.sVersion=sEnigmaVersion;
                            bDetect=true;
                        }
                    }

                    // TODO unknown

                    if(bDetect)
                    {
                        pPEInfo->mapResultProtectors.insert(recordEnigma.name,scansToScan(&(pPEInfo->basic_info),&recordEnigma));
                    }
                }
            }


            if(!pPEInfo->bIs64)
            {

                // ANDpakk2
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ANDPAKK2)||
                        pPEInfo->mapHeaderDetects.contains(RECORD_NAME_ANDPAKK2))
                {
                    // TODO compare entryPoint and import sections
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ANDPAKK2))
                    {
                        SpecAbstract::SCANS_STRUCT recordANFpakk2=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ANDPAKK2);
                        pPEInfo->mapResultPackers.insert(recordANFpakk2.name,scansToScan(&(pPEInfo->basic_info),&recordANFpakk2));
                    }
                }

                // ASDPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ASDPACK))
                {
                    // TODO compare entryPoint and import sections
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ASDPACK))
                    {
                        SpecAbstract::SCANS_STRUCT recordASDPack=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ASDPACK);
                        pPEInfo->mapResultPackers.insert(recordASDPack.name,scansToScan(&(pPEInfo->basic_info),&recordASDPack));
                    }
                }

                // 32lite
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_32LITE))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_32LITE))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::SCANS_STRUCT record32lite=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_32LITE);
                        pPEInfo->mapResultPackers.insert(record32lite.name,scansToScan(&(pPEInfo->basic_info),&record32lite));
                    }
                }

                // ACProtect
                // 1.X-2.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ACPROTECT))
                {
                    if((pPEInfo->nImportSectionOffset)&&(pPEInfo->nImportSectionSize))
                    {
                        qint64 nSectionOffset=pPEInfo->nImportSectionOffset;
                        qint64 nSectionSize=pPEInfo->nImportSectionSize;

                        qint64 nOffset1=pe.find_array(nSectionOffset,nSectionSize,"MineImport_Endss",16);

                        if(nOffset1!=-1)
                        {
                            SpecAbstract::SCANS_STRUCT recordACProtect= {0};
                            recordACProtect.type=RECORD_TYPE_PROTECTOR;
                            recordACProtect.name=RECORD_NAME_ACPROTECT;

                            recordACProtect.sVersion="1.X-2.X";

                            //                            qint64 nOffset2=pe.find_array(nSectionOffset,nSectionSize,"Randimize",9);

                            //                            if(nOffset2!=-1)
                            //                            {
                            //                                recordACProtect.sVersion="1.X";
                            //                            }


                            pPEInfo->mapResultProtectors.insert(recordACProtect.name,scansToScan(&(pPEInfo->basic_info),&recordACProtect));
                        }
                    }
                }

                // ACProtect
                // 2.0.X
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ACPROTECT))
                {
                    SpecAbstract::SCANS_STRUCT recordACProtect=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ACPROTECT);
                    pPEInfo->mapResultProtectors.insert(recordACProtect.name,scansToScan(&(pPEInfo->basic_info),&recordACProtect));
                }

                // FSG
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_FSG))
                {
                    // TODO
                    //                    SpecAbstract::SCANS_STRUCT recordACProtect=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ACPROTECT);
                    //                    pPEInfo->mapResultProtectors.insert(recordACProtect.name,scansToScan(&(pPEInfo->basic_info),&recordACProtect));
                }


                // Alex Protector
                // 2.0.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ALEXPROTECTOR))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ALEXPROTECTOR))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::SCANS_STRUCT recordAlexProtector=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ALEXPROTECTOR);
                        pPEInfo->mapResultProtectors.insert(recordAlexProtector.name,scansToScan(&(pPEInfo->basic_info),&recordAlexProtector));
                    }
                }

                // Alloy
                // 4.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ALLOY))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ALLOY))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::SCANS_STRUCT recordAlloy=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ALLOY);
                        pPEInfo->mapResultProtectors.insert(recordAlloy.name,scansToScan(&(pPEInfo->basic_info),&recordAlloy));
                    }
                }

                // PeX
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_PEX))
                {
                    // TODO compare entryPoint and import sections
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PEX))
                    {
                        if(pe.compareEntryPoint("E9$$$$$$$$60e8$$$$$$$$83c404e8"))
                        {
                            SpecAbstract::SCANS_STRUCT recordPEX=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PEX);
                            pPEInfo->mapResultPackers.insert(recordPEX.name,scansToScan(&(pPEInfo->basic_info),&recordPEX));
                        }
                    }
                }

                // ASPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ASPACK))
                {
                    // TODO compare entryPoint and import sections
                    QString _sSignature=pPEInfo->sEntryPointSignature;
                    qint64 _nOffset=0;
                    QString _sVersion;

                    while(true)
                    {
                        bool bContinue=false;

                        if(QBinary::compareSignatureStrings(_sSignature,"90"))
                        {
                            bContinue=true;
                            _nOffset++;
                            _sSignature.remove(0,2);
                        }

                        if(QBinary::compareSignatureStrings(_sSignature,"7500"))
                        {
                            bContinue=true;
                            _nOffset+=2;
                            _sSignature.remove(0,4);
                        }

                        if(QBinary::compareSignatureStrings(_sSignature,"7501"))
                        {
                            bContinue=true;
                            _nOffset+=3;
                            _sSignature.remove(0,6);
                        }

                        if(QBinary::compareSignatureStrings(_sSignature,"E9"))
                        {
                            bContinue=true;
                            _nOffset++;
                            _sSignature.remove(0,2);
                            qint32 nAddress=QBinary::hexToInt32(_sSignature);
                            _nOffset+=4;
                            // TODO image
                            qint64 nSignatureOffset=pe.addressToOffset(pPEInfo->nImageBaseAddress+pPEInfo->nEntryPointAddress+_nOffset+nAddress);

                            if(nSignatureOffset!=-1)
                            {
                                _sSignature=pe.getSignature(nSignatureOffset,150);
                            }
                            else
                            {
                                break;
                            }
                        }


                        if(QBinary::compareSignatureStrings(_sSignature,"60E8000000005D81ED........B8........03C5"))
                        {
                            _sVersion="1.00b-1.07b";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60EB..5DEB..FF..........E9"))
                        {
                            _sVersion="1.08.01-1.08.02";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E8000000005D............BB........03DD"))
                        {
                            _sVersion="1.08.03";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E8000000005D81ed........BB........01eb"))
                        {
                            _sVersion="1.08.x";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E841060000EB41"))
                        {
                            _sVersion="1.08.04";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60EB..5DFFE5E8........81ED........BB........03DD2B9D"))
                        {
                            _sVersion="1.08.x";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E870050000EB4C"))
                        {
                            _sVersion="2.000";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E872050000EB4C"))
                        {
                            _sVersion="2.001";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E872050000EB3387DB9000"))
                        {
                            _sVersion="2.1";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E93D040000"))
                        {
                            _sVersion="2.11";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E802000000EB095D5581ED39394400C3E93D040000"))
                        {
                            _sVersion="2.11b";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E802000000EB095D5581ED39394400C3E959040000"))
                        {
                            _sVersion="2.11c-2.11d";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E802000000EB095D55"))
                        {
                            _sVersion="2.11d";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E803000000E9EB045D4555C3E801"))
                        {
                            _sVersion="2.12";
                        }



                        if(_nOffset>20)
                        {
                            break;
                        }

                        if(!bContinue)
                        {
                            break;
                        }

                        if(_sVersion!="")
                        {
                            break;
                        }
                    }

                    if(_sVersion!="")
                    {
                        SpecAbstract::SCANS_STRUCT recordASPack= {0};

                        recordASPack.type=RECORD_TYPE_PACKER;
                        recordASPack.name=RECORD_NAME_ASPACK;
                        recordASPack.sVersion=_sVersion;
                        //recordAndpakk.sInfo;

                        pPEInfo->mapResultPackers.insert(recordASPack.name,scansToScan(&(pPEInfo->basic_info),&recordASPack));
                    }
                }

                // No Import
                // WWPACK32
                // TODO false
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_WWPACK32))
                {
                    SpecAbstract::SCANS_STRUCT recordWWPACK32= {0};

                    recordWWPACK32.type=RECORD_TYPE_PACKER;
                    recordWWPACK32.name=RECORD_NAME_WWPACK32;
                    recordWWPACK32.sVersion=QBinary::hexToString(pPEInfo->sEntryPointSignature.mid(102,8));
                    //recordAndpakk.sInfo;

                    pPEInfo->mapResultPackers.insert(recordWWPACK32.name,scansToScan(&(pPEInfo->basic_info),&recordWWPACK32));
                }
            }
        }
        else
        {
            // .NET
            if(QPE::isNETAnsiStringPresent("YanoAttribute",&(pPEInfo->cliInfo)))
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_YANO,"1.X","",0);
                pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(QPE::isNETAnsiStringPresent("DotfuscatorAttribute",&(pPEInfo->cliInfo)))
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_DOTFUSCATOR,"","",0);
                pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }


    //    SpecAbstract::SCANS_STRUCT recordProtector={0};
    //    if((pPEInfo->nEntryPointSection!=pPEInfo->nNormalCodeSection)&&(pPEInfo->nEntryPointSection!=-1)&&
    //            (pPEInfo->sEntryPointSectionName!=".itext")&&
    //            (pPEInfo->sEntryPointSectionName!="INIT"))
    //    {
    //        // .itext borland, section number 1
    //        // INIT drivers, section number 4-5
    //        recordProtector.type=RECORD_TYPE_PROTECTOR;
    //        recordProtector.name=RECORD_NAME_UNKNOWN;
    //    }

    //    if(!pPEInfo->cliInfo.bInit)
    //    {

    ////        qDebug()<<stDetects;

    ////        QList<QString> list=stDetects.toList();

    ////        for(int i=0;i<list.count();i++)
    ////        {
    ////            recordProtector.sInfo+=list.at(i)+" ";
    ////        }

    //        if(!pPEInfo->bIs64)
    //        {
    //            // 32

    //            // UPX
    //            if(     stDetects.contains("kernel32_upx0exe")||
    //                    stDetects.contains("kernel32_upx1exe")||
    //                    stDetects.contains("kernel32_upx1dll")||
    //                    stDetects.contains("kernel32_upx2exe")||
    //                    stDetects.contains("kernel32_upx2dll")||
    //                    stDetects.contains("kernel32_upx3exe")||
    //                    stDetects.contains("kernel32_upx3dll"))
    //            {
    //                bool bNotMod=false;

    //                if(     stDetects.contains("kernel32_upx0exe")||
    //                        stDetects.contains("kernel32_upx1exe")||
    //                        stDetects.contains("kernel32_upx2exe")||
    //                        stDetects.contains("kernel32_upx3exe"))
    //                {
    //                    if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"60E8........68........8BE88DBD........33DB033C248BF7"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                    else if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"60E8000000005883E83D50"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                    else if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"60E80000000083CDFF31DB5E"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                    else if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"60BE........8DBE"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                }

    //                if(stDetects.contains("kernel32_upx1dll")||
    //                        stDetects.contains("kernel32_upx2dll")||
    //                        stDetects.contains("kernel32_upx3dll"))
    //                {
    //                    if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"807C2408010F85........60E80000000083CDFF31DB5E"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                    else if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"807C2408010F85........60BE........8DBE"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                }

    //                VI_STRUCT vi=get_UPX_vi(pDevice,pPEInfo);

    //                if((vi.sVersion!="")&&(bNotMod))
    //                {
    //                    SpecAbstract::SCANS_STRUCT recordUPX={0};

    //                    recordUPX.type=RECORD_TYPE_PACKER;
    //                    recordUPX.name=RECORD_NAME_UPX;
    //                    recordUPX.sVersion=vi.sVersion;
    //                    recordUPX.sInfo=vi.sInfo;

    //                    pPEInfo->mapResultPackers.insert(recordUPX.name,scansToScan(&(pPEInfo->basic_info),&recordUPX));
    //                }

    //            }

    //            if(stDetects.contains("kernel32_andpakk"))
    //            {
    //                if(QBinary::compareSignatureStrings(pPEInfo->basic_info.sHeaderSignature,"'MZ'00'ANDpakk2'00'PE'0000")&&
    //                        QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"FCBE........BF........5783CDFF33C9F9EB05A402DB7505"))
    //                {
    //                    SpecAbstract::SCANS_STRUCT recordAndpakk2={0};

    //                    recordAndpakk2.type=RECORD_TYPE_PACKER;
    //                    recordAndpakk2.name=RECORD_NAME_ANDPAKK2;
    //                    recordAndpakk2.sVersion="0.18";
    ////                    recordAndpakk.sInfo;

    //                    pPEInfo->mapResultPackers.insert(recordAndpakk2.name,scansToScan(&(pPEInfo->basic_info),&recordAndpakk2));
    //                }
    //            }

    //            if(stDetects.contains("kernel32_32lite"))
    //            {
    //                if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"6006FC1E07BE........6A0468........68........6A00FF96........09C075030761C3"))
    //                {
    //                    SpecAbstract::SCANS_STRUCT record32lite={0};

    //                    record32lite.type=RECORD_TYPE_PACKER;
    //                    record32lite.name=RECORD_NAME_32LITE;
    //                    record32lite.sVersion="0.03a";
    //    //                    recordAndpakk.sInfo;

    //                    pPEInfo->mapResultPackers.insert(record32lite.name,scansToScan(&(pPEInfo->basic_info),&record32lite));
    //                }
    //            }

    //            // TODO mass EP scan

    //            // Not import
    //            // WWpack32
    //            if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"53558be833dbeb60'\r\n\r\nWWPack32 decompression routine version '........'\r\n(c) 1998 Piotr Warezak and Rafal Wierzbicki\r\n\r\n'"))
    //            {
    //                SpecAbstract::SCANS_STRUCT recordWWPACK32={0};

    //                recordWWPACK32.type=RECORD_TYPE_PACKER;
    //                recordWWPACK32.name=RECORD_NAME_WWPACK32;
    //                recordWWPACK32.sVersion=QBinary::hexToString(pPEInfo->sEntryPointSignature.mid(102,8));
    ////                    recordAndpakk.sInfo;

    //                pPEInfo->mapResultPackers.insert(recordWWPACK32.name,scansToScan(&(pPEInfo->basic_info),&recordWWPACK32));
    //            }
    //        }
    //        else
    //        {
    //            // 64
    //            if(     stDetects.contains("kernel32_upx3exe")||
    //                    stDetects.contains("kernel32_upx3dll"))
    //            {
    //                bool bNotMod=false;

    //                if(stDetects.contains("kernel32_upx3exe"))
    //                {
    //                    if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"53565755488D35........488DBE"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                }

    //                if(stDetects.contains("kernel32_upx3dll"))
    //                {
    //                    if(QBinary::compareSignatureStrings(pPEInfo->sEntryPointSignature,"48894C240848895424104C8944241880FA010F85........53565755488D35........488DBE"))
    //                    {
    //                        bNotMod=true;
    //                    }
    //                }

    //                VI_STRUCT vi=get_UPX_vi(pDevice,pPEInfo);

    //                if((vi.sVersion!="")&&(bNotMod))
    //                {
    //                    SpecAbstract::SCANS_STRUCT recordUPX={0};

    //                    recordUPX.type=RECORD_TYPE_PACKER;
    //                    recordUPX.name=RECORD_NAME_UPX;
    //                    recordUPX.sVersion=vi.sVersion;
    //                    recordUPX.sInfo=vi.sInfo;

    //                    pPEInfo->mapResultPackers.insert(recordUPX.name,scansToScan(&(pPEInfo->basic_info),&recordUPX));
    //                }

    //            }
    //        }
    //    }

    // ACProtect 2 imports




    //    if(recordProtector.type!=RECORD_TYPE_UNKNOWN)
    //    {
    //        // TODO Paclers
    //        pPEInfo->mapResultProtectors.insert(recordProtector.name,scansToScan(&(pPEInfo->basic_info),&recordProtector));
    //    }
}

void SpecAbstract::handle_libraries(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    if(!pPEInfo->cliInfo.bInit)
    {
        // Qt
        // mb TODO upper
        if(QPE::isImportLibraryPresentI("QtCore4.dll",&(pPEInfo->listImports)))
        {
            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"4.X","",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("QtCored4.dll",&(pPEInfo->listImports)))
        {
            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"4.X","Debug",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("Qt5Core.dll",&(pPEInfo->listImports)))
        {
            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"5.X","",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("Qt5Cored.dll",&(pPEInfo->listImports)))
        {
            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"5.X","Debug",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
    }

}

void SpecAbstract::handle_Microsoft(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    SpecAbstract::SCANS_STRUCT recordLinker= {0};
    SpecAbstract::SCANS_STRUCT recordCompiler= {0};
    SpecAbstract::SCANS_STRUCT recordTool= {0};
    SpecAbstract::SCANS_STRUCT recordMFC= {0};
    SpecAbstract::SCANS_STRUCT recordNET= {0};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        // Linker

        if((pPEInfo->mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKER))&&(!pPEInfo->mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER)))
        {
            recordLinker.type=RECORD_TYPE_LINKER;
            recordLinker.name=RECORD_NAME_MICROSOFTLINKER;
        }

        if(!pPEInfo->cliInfo.bInit)
        {
            // MFC
            // Static
            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize))
            {
                qint64 _nOffset=pPEInfo->nDataSectionOffset;
                qint64 _nSize=pPEInfo->nDataSectionSize;

                qint64 nOffset_MFC=pe.find_ansiString(_nOffset,_nSize,"CMFCComObject");

                if(nOffset_MFC!=-1)
                {
                    recordMFC.type=RECORD_TYPE_LIBRARY;
                    recordMFC.name=RECORD_NAME_MFC;
                    recordMFC.sInfo="Static";
                }
            }

            for(int i=0; i<pPEInfo->listImports.count(); i++)
            {
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^MFC")))
                {
                    QRegularExpression rxVersion("(\\d+)");
                    QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

                    if(matchVersion.hasMatch())
                    {
                        double dVersion=matchVersion.captured(0).toDouble()/10;

                        if(dVersion)
                        {
                            recordMFC.type=RECORD_TYPE_LIBRARY;
                            recordMFC.name=RECORD_NAME_MFC;
                            recordMFC.sVersion=QString::number(dVersion,'f',2);

                            if(pPEInfo->listImports.at(i).sName.toUpper().contains("U.DLL"))
                            {
                                recordMFC.sInfo="Unicode";
                            }
                        }
                    }

                    break;
                }
            }

            // VB
            bool bVBnew=false;

            if(QPE::isImportLibraryPresentI("VB40032.DLL",&(pPEInfo->listImports)))
            {
                recordCompiler.type=RECORD_TYPE_COMPILER;
                recordCompiler.name=RECORD_NAME_VISUALBASIC;
                recordCompiler.sVersion="4.0";
            }
            else if(QPE::isImportLibraryPresentI("MSVBVM50.DLL",&(pPEInfo->listImports)))
            {
                recordCompiler.type=RECORD_TYPE_COMPILER;
                recordCompiler.name=RECORD_NAME_VISUALBASIC;
                recordCompiler.sVersion="5.0";
                bVBnew=true;
            }

            if(QPE::isImportLibraryPresentI("MSVBVM60.DLL",&(pPEInfo->listImports)))
            {
                recordCompiler.type=RECORD_TYPE_COMPILER;
                recordCompiler.name=RECORD_NAME_VISUALBASIC;
                recordCompiler.sVersion="6.0";
                bVBnew=true;
            }

            if(bVBnew)
            {
                if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize))
                {
                    qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                    qint64 _nSize=pPEInfo->nCodeSectionSize;

                    qint64 nOffset_Options=pe.find_uint32(_nOffset,_nSize,0x21354256);

                    if(nOffset_Options==-1)
                    {
                        nOffset_Options=pe.find_uint32(_nOffset,_nSize,0x21364256);
                    }

                    if(nOffset_Options!=-1)
                    {
                        quint32 nOffsetOptions2=pe.read_uint32(_nOffset+0x30);

                        quint32 nOffsetOptions3=pe.addressToOffset(pe.getBaseAddress()+nOffsetOptions2);
                        quint32 nValue=pe.read_uint32(nOffsetOptions3+0x20);
                        recordCompiler.sInfo=nValue?"P-Code":"Native";
                    }
                }
            }
        }
        else
        {
            recordNET.type=SpecAbstract::RECORD_TYPE_LIBRARY;
            recordNET.name=SpecAbstract::RECORD_NAME_DOTNET;
            recordNET.sVersion=pPEInfo->cliInfo.sCLI_MetaData_Version;

            if(QPE::isNETAnsiStringPresent("Microsoft.VisualBasic",&(pPEInfo->cliInfo)))
            {
                recordCompiler.type=RECORD_TYPE_COMPILER;
                recordCompiler.name=RECORD_NAME_VBNET;
            }
        }



        // Rich
        int nRichSignaturesCount=pPEInfo->listRichSignatures.count();

        if(nRichSignaturesCount>=1)
        {
            recordLinker.type=SpecAbstract::RECORD_TYPE_LINKER;
            recordLinker.name=SpecAbstract::RECORD_NAME_MICROSOFTLINKER;

            quint32 _nRich1=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-1).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-1).nVersion;
            quint32 _nRich2=0;
            quint32 _nRich3=0;
            quint32 _nRich4=0;

            if(nRichSignaturesCount>=2)
            {
                _nRich2=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-2).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-2).nVersion;
            }

            if(nRichSignaturesCount>=3)
            {
                _nRich3=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-3).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-3).nVersion;
            }

            if(nRichSignaturesCount>=4)
            {
                _nRich4=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-4).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-4).nVersion;
            }

            SpecAbstract::SCANS_STRUCT ssRich1=SpecAbstract::getRichSignatureDescription(_nRich1);
            SpecAbstract::SCANS_STRUCT ssRich2=SpecAbstract::getRichSignatureDescription(_nRich2);
            SpecAbstract::SCANS_STRUCT ssRich3=SpecAbstract::getRichSignatureDescription(_nRich3);
            SpecAbstract::SCANS_STRUCT ssRich4=SpecAbstract::getRichSignatureDescription(_nRich4);

            SpecAbstract::SCANS_STRUCT *pssLinker=0;
            SpecAbstract::SCANS_STRUCT *pssCompiler=0;

            if(ssRich1.type==SpecAbstract::RECORD_TYPE_LINKER)
            {
                pssLinker=&ssRich1;
            }
            else if(ssRich2.type==SpecAbstract::RECORD_TYPE_LINKER)
            {
                pssLinker=&ssRich2;
            }
            else if(ssRich3.type==SpecAbstract::RECORD_TYPE_LINKER)
            {
                pssLinker=&ssRich3;
            }
            else if(ssRich4.type==SpecAbstract::RECORD_TYPE_LINKER)
            {
                pssLinker=&ssRich4;
            }

            if(ssRich1.type==SpecAbstract::RECORD_TYPE_COMPILER)
            {
                pssCompiler=&ssRich1;
            }
            else if(ssRich2.type==SpecAbstract::RECORD_TYPE_COMPILER)
            {
                pssCompiler=&ssRich2;
            }
            else if(ssRich3.type==SpecAbstract::RECORD_TYPE_COMPILER)
            {
                pssCompiler=&ssRich3;
            }
            else if(ssRich4.type==SpecAbstract::RECORD_TYPE_COMPILER)
            {
                pssCompiler=&ssRich4;
            }



            if(pssLinker)
            {
                recordLinker.sVersion=pssLinker->sVersion;
                recordLinker.sInfo=pssLinker->sInfo;
            }

            if(pssCompiler)
            {
                recordCompiler.type=pssCompiler->type;
                recordCompiler.name=pssCompiler->name;
                recordCompiler.sVersion=pssCompiler->sVersion;
                recordCompiler.sInfo=pssCompiler->sInfo;

                // VB 6.0
                if(recordCompiler.name==SpecAbstract::RECORD_NAME_VISUALBASIC)
                {
                    if(nRichSignaturesCount>1)
                    {
                        recordCompiler.sInfo="Native";
                    }
                    else
                    {
                        recordCompiler.sInfo="P-Code";
                    }
                }
            }
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordCompiler.name!=RECORD_NAME_VISUALCPP))
        {
            recordCompiler.type=SpecAbstract::RECORD_TYPE_COMPILER;
            recordCompiler.name=SpecAbstract::RECORD_NAME_VISUALCPP;

            if(recordMFC.sVersion=="6.00")
            {
                recordCompiler.sVersion="12.00";
            }
            else if(recordMFC.sVersion=="7.00")
            {
                recordCompiler.sVersion="13.00";
            }
            else if(recordMFC.sVersion=="7.10")
            {
                recordCompiler.sVersion="13.10";
            }
            else if(recordMFC.sVersion=="8.00")
            {
                recordCompiler.sVersion="14.00";
            }
            else if(recordMFC.sVersion=="9.00")
            {
                recordCompiler.sVersion="15.00";
            }
            else if(recordMFC.sVersion=="10.00")
            {
                recordCompiler.sVersion="16.00";
            }
            else if(recordMFC.sVersion=="11.00")
            {
                recordCompiler.sVersion="17.00";
            }
            else if(recordMFC.sVersion=="12.00")
            {
                recordCompiler.sVersion="18.00";
            }
            else if(recordMFC.sVersion=="14.00")
            {
                recordCompiler.sVersion="19.00";
            }
            else if(recordMFC.sVersion=="14.10")
            {
                recordCompiler.sVersion="19.10";
            }
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordMFC.sVersion==""))
        {
            if((recordCompiler.name==RECORD_NAME_VISUALCPP)&&(recordCompiler.sVersion!=""))
            {
                recordMFC.sVersion=recordCompiler.sVersion.section(".",0,1);
            }
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordLinker.name!=RECORD_NAME_MICROSOFTLINKER))
        {
            recordLinker.type=SpecAbstract::RECORD_TYPE_LINKER;
            recordLinker.name=SpecAbstract::RECORD_NAME_MICROSOFTLINKER;
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordLinker.sVersion==""))
        {
            recordLinker.sVersion=recordMFC.sVersion;
            //            recordLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);;
        }


        if(recordCompiler.name==SpecAbstract::RECORD_NAME_VISUALCPP)
        {
            QString sLinkerVersion=recordLinker.sVersion;
            QString sCompilerVersion=recordCompiler.sVersion;
            QString sCompilerMajorVersion=sCompilerVersion.left(5);

            recordTool.type=SpecAbstract::RECORD_TYPE_TOOL;
            recordTool.name=SpecAbstract::RECORD_NAME_MICROSOFTVISUALSTUDIO;

            if(sCompilerVersion=="12.00.8168")
            {
                recordTool.sVersion="6.0";
            }
            else if(sCompilerVersion=="12.00.8804")
            {
                recordTool.sVersion="6.0 SP5-SP6";
            }
            else if((sLinkerVersion=="7.00.9466")&&(sCompilerVersion=="13.00.9466"))
            {
                recordTool.sVersion="2002";
            }
            else if((sLinkerVersion=="7.10.3077")&&(sCompilerVersion=="13.10.3077"))
            {
                recordTool.sVersion="2003";
            }
            else if((sLinkerVersion=="7.10.4035")&&(sCompilerVersion=="13.10.4035"))
            {
                recordTool.sVersion="2003";
            }
            else if((sLinkerVersion=="7.10.6030")&&(sCompilerVersion=="13.10.6030"))
            {
                recordTool.sVersion="2003 SP1";
            }
            else if((sLinkerVersion=="8.00.40310")&&(sCompilerVersion=="14.00.40310"))
            {
                recordTool.sVersion="2005";
            }
            else if((sLinkerVersion=="8.00.50727")&&(sCompilerVersion=="14.00.50727"))
            {
                recordTool.sVersion="2005";
            }
            else if((sLinkerVersion=="9.00.21022")&&(sCompilerVersion=="15.00.21022"))
            {
                recordTool.sVersion="2008 RTM";
            }
            else if((sLinkerVersion=="9.00.30411")&&(sCompilerVersion=="15.00.30411"))
            {
                recordTool.sVersion="2008 with Feature Pack";
            }
            else if((sLinkerVersion=="9.00.30729")&&(sCompilerVersion=="15.00.30729"))
            {
                recordTool.sVersion="2008 SP1";
            }
            else if((sLinkerVersion=="10.00.30319")&&(sCompilerVersion=="16.00.30319"))
            {
                recordTool.sVersion="2010 RTM";
            }
            else if((sLinkerVersion=="10.00.40219")&&(sCompilerVersion=="16.00.40219"))
            {
                recordTool.sVersion="2010 SP1";
            }
            else if((sLinkerVersion=="11.00.50727")&&(sCompilerVersion=="17.00.50727"))
            {
                recordTool.sVersion="2012";
            }
            else if((sLinkerVersion=="11.00.51025")&&(sCompilerVersion=="17.00.51025"))
            {
                recordTool.sVersion="2012";
            }
            else if((sLinkerVersion=="11.00.51106")&&(sCompilerVersion=="17.00.51106"))
            {
                recordTool.sVersion="2012 update 1";
            }
            else if((sLinkerVersion=="11.00.60315")&&(sCompilerVersion=="17.00.60315"))
            {
                recordTool.sVersion="2012 update 2";
            }
            else if((sLinkerVersion=="11.00.60610")&&(sCompilerVersion=="17.00.60610"))
            {
                recordTool.sVersion="2012 update 3";
            }
            else if((sLinkerVersion=="11.00.61030")&&(sCompilerVersion=="17.00.61030"))
            {
                recordTool.sVersion="2012 update 4";
            }
            else if((sLinkerVersion=="12.00.21005")&&(sCompilerVersion=="18.00.21005"))
            {
                recordTool.sVersion="2013 RTM";
            }
            else if((sLinkerVersion=="12.00.30501")&&(sCompilerVersion=="18.00.30501"))
            {
                recordTool.sVersion="2013 update 2";
            }
            else if((sLinkerVersion=="12.00.30723")&&(sCompilerVersion=="18.00.30723"))
            {
                recordTool.sVersion="2013 update 3";
            }
            else if((sLinkerVersion=="12.00.31101")&&(sCompilerVersion=="18.00.31101"))
            {
                recordTool.sVersion="2013 update 4";
            }
            else if((sLinkerVersion=="12.00.40629")&&(sCompilerVersion=="18.00.40629"))
            {
                recordTool.sVersion="2013 SP5";
            }
            else if((sLinkerVersion=="14.00.22215")&&(sCompilerVersion=="19.00.22215"))
            {
                recordTool.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23007")&&(sCompilerVersion=="19.00.23007"))
            {
                recordTool.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23013")&&(sCompilerVersion=="19.00.23013"))
            {
                recordTool.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23026")&&(sCompilerVersion=="19.00.23026"))
            {
                recordTool.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23506")&&(sCompilerVersion=="19.00.23506"))
            {
                recordTool.sVersion="2015 update 1";
            }
            else if((sLinkerVersion=="14.00.23918")&&(sCompilerVersion=="19.00.23918"))
            {
                recordTool.sVersion="2015 update 2";
            }
            else if((sLinkerVersion=="14.00.24103")&&(sCompilerVersion=="19.00.24103"))
            {
                recordTool.sVersion="2015 SP1"; // ???
            }
            else if((sLinkerVersion=="14.00.24118")&&(sCompilerVersion=="19.00.24118"))
            {
                recordTool.sVersion="2015 SP1"; // ???
            }
            else if((sLinkerVersion=="14.00.24210")&&(sCompilerVersion=="19.00.24210"))
            {
                recordTool.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24212")&&(sCompilerVersion=="19.00.24212"))
            {
                recordTool.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24213")&&(sCompilerVersion=="19.00.24213"))
            {
                recordTool.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24215")&&(sCompilerVersion=="19.00.24215"))
            {
                recordTool.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24218")&&(sCompilerVersion=="19.00.24218"))
            {
                recordTool.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24723")&&(sCompilerVersion=="19.00.24723"))
            {
                recordTool.sVersion="2015"; // Update 4? 2017?
            }
            else if((sLinkerVersion=="14.10.25017")&&(sCompilerVersion=="19.10.25017"))
            {
                recordTool.sVersion="2017";
            }
            else if(sCompilerMajorVersion=="12.00")
            {
                recordTool.sVersion="6.0";
            }
            else if(sCompilerMajorVersion=="13.00")
            {
                recordTool.sVersion="2002";
            }
            else if(sCompilerMajorVersion=="13.10")
            {
                recordTool.sVersion="2003";
            }
            else if(sCompilerMajorVersion=="14.00")
            {
                recordTool.sVersion="2005";
            }
            else if(sCompilerMajorVersion=="15.00")
            {
                recordTool.sVersion="2008";
            }
            else if(sCompilerMajorVersion=="16.00")
            {
                recordTool.sVersion="2010";
            }
            else if(sCompilerMajorVersion=="17.00")
            {
                recordTool.sVersion="2012";
            }
            else if(sCompilerMajorVersion=="18.00")
            {
                recordTool.sVersion="2013";
            }
            else if(sCompilerMajorVersion=="19.00")
            {
                recordTool.sVersion="2015";
            }
            else if(sCompilerMajorVersion=="19.10") // TODO
            {
                recordTool.sVersion="2017";
            }
        }
        else if(recordCompiler.name==SpecAbstract::RECORD_NAME_MASM)
        {
            QString sCompilerVersion=recordCompiler.sVersion;
            QString sLinkerVersion=recordLinker.sVersion;

            if((sLinkerVersion=="5.12.8078")&&(sCompilerVersion=="6.14.8444"))
            {
                recordTool.type=SpecAbstract::RECORD_TYPE_TOOL;
                recordTool.name=SpecAbstract::RECORD_NAME_MASM32;
                recordTool.sVersion="8-11";
            }
        }

        if((recordLinker.name==RECORD_NAME_MICROSOFTLINKER)&&(recordLinker.sVersion==""))
        {
            recordLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
        }

        if(recordLinker.type!=RECORD_TYPE_UNKNOWN)
        {
            pPEInfo->mapResultLinkers.insert(recordLinker.name,scansToScan(&(pPEInfo->basic_info),&recordLinker));
        }

        if(recordCompiler.type!=RECORD_TYPE_UNKNOWN)
        {
            pPEInfo->mapResultCompilers.insert(recordCompiler.name,scansToScan(&(pPEInfo->basic_info),&recordCompiler));
        }

        if(recordTool.type!=RECORD_TYPE_UNKNOWN)
        {
            pPEInfo->mapResultTools.insert(recordTool.name,scansToScan(&(pPEInfo->basic_info),&recordTool));
        }

        if(recordMFC.type!=RECORD_TYPE_UNKNOWN)
        {
            pPEInfo->mapResultLibraries.insert(recordMFC.name,scansToScan(&(pPEInfo->basic_info),&recordMFC));
        }

        if(recordNET.type!=RECORD_TYPE_UNKNOWN)
        {
            pPEInfo->mapResultLibraries.insert(recordNET.name,scansToScan(&(pPEInfo->basic_info),&recordNET));
        }
    }
}

void SpecAbstract::handle_Borland(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    // TODO Turbo Linker
    QPE pe(pDevice);
    //

    if(pe.isValid())
    {
        if(pPEInfo->mapHeaderDetects.contains(SpecAbstract::RECORD_NAME_TURBOLINKER))
        {
            SCANS_STRUCT recordTurboLinker=pPEInfo->mapHeaderDetects.value(SpecAbstract::RECORD_NAME_TURBOLINKER);

            recordTurboLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion,2,10,QChar('0'));

            pPEInfo->mapResultLinkers.insert(recordTurboLinker.name,scansToScan(&(pPEInfo->basic_info),&recordTurboLinker));
        }

        if(!pPEInfo->cliInfo.bInit)
        {
            qint64 nOffset_string=-1;
            qint64 nOffset_Boolean=-1;
            qint64 nOffset_String=-1;
            qint64 nOffset_TObject=-1;
            //        qint64 nOffset_AnsiString=-1;
            //        qint64 nOffset_WideString=-1;

            qint64 nOffset_BorlandCPP=-1;
            qint64 nOffset_CodegearCPP=-1;
            qint64 nOffset_EmbarcaderoCPP=-1;

            QList<VCL_STRUCT> listVCL;

            bool bCppExport=QPE::isExportFunctionPresent("__CPPdebugHook",&(pPEInfo->export_header));

            if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize))
            {
                qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                qint64 _nSize=pPEInfo->nCodeSectionSize;


                nOffset_TObject=pe.find_array(_nOffset,_nSize,"\x07\x54\x4f\x62\x6a\x65\x63\x74",8); // TObject

                if(nOffset_TObject!=-1)
                {
                    nOffset_Boolean=pe.find_array(_nOffset,_nSize,"\x07\x42\x6f\x6f\x6c\x65\x61\x6e",8); // Boolean
                    nOffset_string=pe.find_array(_nOffset,_nSize,"\x06\x73\x74\x72\x69\x6e\x67",7); // string

                    if((nOffset_Boolean!=-1)||(nOffset_string!=-1))
                    {
                        if(nOffset_string==-1)
                        {
                            nOffset_String=pe.find_array(_nOffset,_nSize,"\x06\x53\x74\x72\x69\x6e\x67",7); // String
                        }

                        listVCL=getVCLstruct(pDevice,_nOffset,_nSize,pPEInfo->bIs64);
                    }
                }



                //            nOffset_AnsiString=pe.find_array(_nOffset,_nSize,"\x0a\x41\x6e\x73\x69\x53\x74\x72\x69\x6e\x67",11); // AnsiString
                //            nOffset_WideString=pe.find_array(_nOffset,_nSize,"\x0a\x57\x69\x64\x65\x53\x74\x72\x69\x6e\x67",11); // WideString
            }

            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize))
            {
                qint64 _nOffset=pPEInfo->nDataSectionOffset;
                qint64 _nSize=pPEInfo->nDataSectionSize;

                nOffset_BorlandCPP=pe.find_array(_nOffset,_nSize,"\x42\x6f\x72\x6c\x61\x6e\x64\x20\x43\x2b\x2b\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20",24); // Borland C++ - Copyright 1994 Borland Intl.

                if(nOffset_BorlandCPP==-1)
                {
                    nOffset_CodegearCPP=pe.find_array(_nOffset,_nSize,"\x43\x6f\x64\x65\x47\x65\x61\x72\x20\x43\x2b\x2b\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20",25); // CodeGear C++ - Copyright 2008 Embarcadero Technologies

                    if(nOffset_CodegearCPP==-1)
                    {
                        nOffset_EmbarcaderoCPP=pe.find_array(_nOffset,_nSize,"\x45\x6d\x62\x61\x72\x63\x61\x64\x65\x72\x6f\x20\x52\x41\x44\x20\x53\x74\x75\x64\x69\x6f\x20\x2d\x20\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20",35); // Embarcadero RAD Studio - Copyright 2009 Embarcadero Technologies, Inc.
                    }
                }
            }

            bool bPackageinfo=QPE::isResourcePresent(10,"PACKAGEINFO",&(pPEInfo->listResources));
            bool bDvcal=QPE::isResourcePresent(10,"DVCLAL",&(pPEInfo->listResources));

            if(bPackageinfo||
                    bDvcal||
                    pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_BORLANDCPP)||
                    (nOffset_TObject!=-1)||
                    (nOffset_BorlandCPP!=-1)||
                    (nOffset_CodegearCPP!=-1)||
                    (nOffset_EmbarcaderoCPP!=-1)||
                    bCppExport)
            {
                bool bCpp=false;
                bool bVCL=bPackageinfo;
                QString sVCLVersion;
                QString sDelphiVersion;
                QString sBuilderVersion;
                QString sObjectPascalCompilerVersion;
                QString sCppCompilerVersion;

                enum COMPANY
                {
                    COMPANY_BORLAND=0,
                    COMPANY_CODEGEAR,
                    COMPANY_EMBARCADERO
                };

                COMPANY company=COMPANY_BORLAND;

                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_BORLANDCPP)||
                        (nOffset_BorlandCPP!=-1)||
                        (nOffset_CodegearCPP!=-1)||
                        (nOffset_EmbarcaderoCPP!=-1)||
                        bCppExport)
                {
                    bCpp=true;

                    if(nOffset_BorlandCPP!=-1)
                    {
                        company=COMPANY_BORLAND;
                    }
                    else if(nOffset_CodegearCPP!=-1)
                    {
                        company=COMPANY_CODEGEAR;
                    }
                    else if(nOffset_EmbarcaderoCPP!=-1)
                    {
                        company=COMPANY_EMBARCADERO;
                    }
                    else if(bCppExport)
                    {
                        company=COMPANY_EMBARCADERO;
                    }
                }

                if(nOffset_TObject!=-1)
                {
                    if(nOffset_string!=-1)
                    {
                        if(bDvcal||bPackageinfo)
                        {
                            // TODO Borland Version
                            sDelphiVersion="2005+";
                        }
                        else
                        {
                            sDelphiVersion="2";
                            sObjectPascalCompilerVersion="9.0";
                        }
                    }
                    else if(nOffset_String!=-1)
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="3-7";
                    }
                }

                if(bPackageinfo)
                {
                    VCL_PACKAGEINFO pi=getVCLPackageInfo(pDevice,&pPEInfo->listResources);

                    if(pi.listModules.count())
                    {
                        quint32 nProducer=(pi.nFlags>>26)&0x3;

                        if(nProducer==2) // C++
                        {
                            bCpp=true;
                        }
                        else if(nProducer==3) // Pascal
                        {
                            bCpp=false;
                        }

                        //                    for(int i=0;i<pi.listModules.count();i++)
                        //                    {
                        //                        qDebug(pi.listModules.at(i).sName.toLatin1().data());
                        //                    }
                    }
                }

                if(nOffset_BorlandCPP!=-1)
                {
                    sCppCompilerVersion=pe.read_ansiString(nOffset_BorlandCPP+24,4);
                }

                if(nOffset_CodegearCPP!=-1)
                {
                    sCppCompilerVersion=pe.read_ansiString(nOffset_CodegearCPP+25,4);
                }

                if(nOffset_EmbarcaderoCPP!=-1)
                {
                    sCppCompilerVersion=pe.read_ansiString(nOffset_EmbarcaderoCPP+35,4);
                }

                if(listVCL.count())
                {
                    bVCL=true;
                    int nVCLOffset=listVCL.at(0).nOffset;
                    int nVCLValue=listVCL.at(0).nValue;

                    //                qDebug("nVCLOffset: %d",nVCLOffset);
                    //                qDebug("nVCLValue: %d",nVCLValue);
                    //                bVCL=true;

                    if((nVCLOffset==24)&&(nVCLValue==168))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="2";
                        sObjectPascalCompilerVersion="9.0";
                        //                    sVCLVersion="20";
                    }
                    else if((nVCLOffset==28)&&(nVCLValue==180))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="3";
                        sObjectPascalCompilerVersion="10.0";
                        //                    sVCLVersion="30";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==276))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="4";
                        sObjectPascalCompilerVersion="12.0";
                        //                    sVCLVersion="40";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==288))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="5";
                        sObjectPascalCompilerVersion="13.0";
                        //                    sVCLVersion="50";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==296))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="6 CLX";
                        sObjectPascalCompilerVersion="14.0";
                        //                    sVCLVersion="60";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==300))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="7 CLX";
                        sObjectPascalCompilerVersion="15.0";
                        //                    sVCLVersion="70";
                    }
                    //                else if(nVCLOffset==40)
                    //                {
                    //                    if(nVCLValue==264)
                    //                    {
                    //                        recordTool.sVersion="???TODO";
                    //                        sVCLVersion="50";
                    //                    }
                    //                }
                    else if((nVCLOffset==40)&&(nVCLValue==348))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="6-7";
                        sObjectPascalCompilerVersion="14.0-15.0";
                        //                    sVCLVersion="140-150";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==356))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="2005";
                        sObjectPascalCompilerVersion="17.0";
                        //                    sVCLVersion="170";
                    }
                    else if((nVCLOffset==40)&&(nVCLValue==400))
                    {
                        company=COMPANY_BORLAND;
                        sDelphiVersion="2006";
                        sObjectPascalCompilerVersion="18.0";
                        //                    sVCLVersion="180";
                    }
                    else if((nVCLOffset==52)&&(nVCLValue==420))
                    {
                        company=COMPANY_EMBARCADERO;
                        sDelphiVersion="2009";
                        sObjectPascalCompilerVersion="20.0";
                        //                    sVCLVersion="200";
                    }
                    else if((nVCLOffset==52)&&(nVCLValue==428))
                    {
                        company=COMPANY_EMBARCADERO;
                        sDelphiVersion="2010-XE";
                        sObjectPascalCompilerVersion="21.0-22.0";
                        //                    sVCLVersion="210-220";
                    }
                    else if((nVCLOffset==52)&&(nVCLValue==436))
                    {
                        company=COMPANY_EMBARCADERO;
                        sDelphiVersion="XE2-XE4";
                        sObjectPascalCompilerVersion="23.0-25.0";
                        //                    sVCLVersion="230-250";
                    }
                    else if((nVCLOffset==52)&&(nVCLValue==444))
                    {
                        company=COMPANY_EMBARCADERO;
                        sDelphiVersion="XE2-XE8";
                        sObjectPascalCompilerVersion="23.0-29.0";
                        //                    sVCLVersion="230-290";
                    }
                }

                SCANS_STRUCT recordCompiler;
                recordCompiler.type=RECORD_TYPE_COMPILER;

                SCANS_STRUCT recordTool;
                recordTool.type=RECORD_TYPE_TOOL;

                if(!bCpp)
                {
                    if(company==COMPANY_BORLAND)
                    {
                        recordCompiler.name=RECORD_NAME_BORLANDOBJECTPASCAL;
                        recordTool.name=RECORD_NAME_BORLANDDELPHI;
                    }
                    else if(company==COMPANY_CODEGEAR)
                    {
                        recordCompiler.name=RECORD_NAME_CODEGEAROBJECTPASCAL;
                        recordTool.name=RECORD_NAME_CODEGEARDELPHI;
                    }
                    else if(company==COMPANY_EMBARCADERO)
                    {
                        recordCompiler.name=RECORD_NAME_EMBARCADEROOBJECTPASCAL;
                        recordTool.name=RECORD_NAME_EMBARCADERODELPHI;
                    }

                    recordCompiler.sVersion=sObjectPascalCompilerVersion;
                    recordTool.sVersion=sDelphiVersion;
                }
                else
                {
                    if(company==COMPANY_BORLAND)
                    {
                        recordCompiler.name=RECORD_NAME_BORLANDCPP;
                        recordTool.name=RECORD_NAME_BORLANDCPPBUILDER;
                    }
                    else if(company==COMPANY_CODEGEAR)
                    {
                        recordCompiler.name=RECORD_NAME_CODEGEARCPP;
                        recordTool.name=RECORD_NAME_CODEGEARCPPBUILDER;
                    }
                    else if(company==COMPANY_EMBARCADERO)
                    {
                        recordCompiler.name=RECORD_NAME_EMBARCADEROCPP;
                        recordTool.name=RECORD_NAME_EMBARCADEROCPPBUILDER;
                    }

                    recordCompiler.sVersion=sCppCompilerVersion;
                    recordTool.sVersion=sBuilderVersion;
                }

                pPEInfo->mapResultCompilers.insert(recordCompiler.name,scansToScan(&(pPEInfo->basic_info),&recordCompiler));
                pPEInfo->mapResultTools.insert(recordTool.name,scansToScan(&(pPEInfo->basic_info),&recordTool));

                if(bVCL)
                {
                    SCANS_STRUCT recordVCL;
                    recordVCL.type=RECORD_TYPE_LIBRARY;
                    recordVCL.name=RECORD_NAME_VCL;
                    recordVCL.sVersion=sVCLVersion;

                    pPEInfo->mapResultTools.insert(recordVCL.name,scansToScan(&(pPEInfo->basic_info),&recordVCL));
                }
            }
        }
        else
        {
            // .NET
            if(QPE::isNETAnsiStringPresent("Borland.Studio.Delphi",&(pPEInfo->cliInfo)))
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_EMBARCADERODELPHIDOTNET,"XE*","",0);
                pPEInfo->mapResultTools.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
            else if(QPE::isNETAnsiStringPresent("Borland.Vcl.Types",&(pPEInfo->cliInfo)))
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_BORLANDDELPHIDOTNET,"8","",0);
                pPEInfo->mapResultTools.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }

    }

}

void SpecAbstract::handle_Tools(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        // FASM
        if(pPEInfo->mapHeaderDetects.contains(RECORD_NAME_FASM))
        {
            // TODO correct Version
            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FASM,"","",0);
            ss.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
            pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }

        // Flex
        if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize))
        {
            qint64 _nOffset=pPEInfo->nDataSectionOffset;
            qint64 _nSize=pPEInfo->nDataSectionSize;
            // TODO FPC Version in Major and Minor linker

            qint64 nOffset_FlexLM=pe.find_ansiString(_nOffset,_nSize,"@(#) FLEXlm ");

            if(nOffset_FlexLM!=-1)
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_FLEXLM,"","",0);

                ss.sVersion=pe.read_ansiString(nOffset_FlexLM+12,50);
                ss.sVersion=ss.sVersion.section(" ",0,0);

                if(ss.sVersion.left(1)=="v")
                {
                    ss.sVersion.remove(0,1);
                }

                // TODO Version
                pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }


            qint64 nOffset_FlexNet=-1;

            if(nOffset_FlexLM==-1)
            {
                nOffset_FlexNet=pe.find_ansiString(_nOffset,_nSize,"@(#) FLEXnet Licensing v");
            }

            if(nOffset_FlexNet==-1)
            {
                nOffset_FlexNet=pe.find_ansiString(_nOffset,_nSize,"@(#) FlexNet Licensing v");
            }

            if(nOffset_FlexNet!=-1)
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_FLEXNET,"","",0);

                ss.sVersion=pe.read_ansiString(nOffset_FlexNet+24,50);

                if(ss.sVersion.contains("build"))
                {
                    ss.sVersion=ss.sVersion.section(" ",0,2);
                }
                else
                {
                    ss.sVersion=ss.sVersion.section(" ",0,0);
                }

                // TODO Version
                pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }

        if(!pPEInfo->cliInfo.bInit)
        {
            QString sDllLib;

            if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize))
            {
                sDllLib=pe.read_ansiString(pPEInfo->nConstDataSectionOffset);
            }

            if(QPE::isImportLibraryPresentI("msys-1.0.dll",&(pPEInfo->listImports)))
            {
                // Msys 1.0
                SCANS_STRUCT ssGCC=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);
                pPEInfo->mapResultCompilers.insert(ssGCC.name,scansToScan(&(pPEInfo->basic_info),&ssGCC));
                SCANS_STRUCT ssMsys=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_MSYS,"1.0","",0);
                pPEInfo->mapResultTools.insert(ssMsys.name,scansToScan(&(pPEInfo->basic_info),&ssMsys));
            }

            if((sDllLib=="libgcc_s_dw2-1.dll")||
                    (sDllLib=="msys-gcc_s-1.dll")||
                    QPE::isImportLibraryPresentI("libgcc_s_dw2-1.dll",&(pPEInfo->listImports)))
            {
                // Mingw
                // Msys
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);

                if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize))
                {
                    qint64 _nOffset=pPEInfo->nConstDataSectionOffset;
                    qint64 _nSize=pPEInfo->nConstDataSectionSize;

                    // TODO get max version
                    qint64 nOffset_Version=pe.find_ansiString(_nOffset,_nSize,"GCC:");

                    if(nOffset_Version!=-1)
                    {
                        QString sVersionString=pe.read_ansiString(nOffset_Version);

                        // TODO MinGW-w64
                        if(sVersionString.contains("MinGW"))
                        {
                            SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                            pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
                        }
                        else if(sVersionString.contains("MSYS2"))
                        {
                            SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MSYS2,"","",0);
                            pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
                        }

                        ss.sVersion=sVersionString.section(" ",-1,-1);
                    }
                }

                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
            else if(sDllLib=="libgcj_s.dll")
            {
                // Cygwin
                // MinGW ???
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);

                if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize))
                {
                    qint64 _nOffset=pPEInfo->nConstDataSectionOffset;
                    qint64 _nSize=pPEInfo->nConstDataSectionSize;

                    // TODO get max version
                    qint64 nOffset_Version=pe.find_ansiString(_nOffset,_nSize,"gcc-");

                    if(nOffset_Version!=-1)
                    {
                        QString sVersionString=pe.read_ansiString(nOffset_Version);
                        ss.sVersion=sVersionString.section("-",1,1).section("/",0,0);
                    }
                }

                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

            }

            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize))
            {
                qint64 _nOffset=pPEInfo->nDataSectionOffset;
                qint64 _nSize=pPEInfo->nDataSectionSize;
                // TODO FPC Version in Major and Minor linker

                qint64 nOffset_FPC=pe.find_ansiString(_nOffset,_nSize,"FPC ");

                if(nOffset_FPC!=-1)
                {
                    SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FPC,"","",0);
                    QString sFPCVersion=pe.read_ansiString(nOffset_FPC);
                    ss.sVersion=sFPCVersion.section(" ",1,-1).section(" - ",0,0);

                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
                else
                {
                    //                    qint64 nOffset_TObject=pe.find_array(_nOffset,_nSize,"\x07\x54\x4f\x62\x6a\x65\x63\x74",8); // TObject

                    //                    if(nOffset_TObject!=-1)
                    //                    {

                    //                        SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FPC,"","",0);

                    //                        // TODO Version
                    //                        pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    //                    }
                    qint64 nOffset_RunTimeError=pe.find_array(_nOffset,_nSize,"\x0e\x52\x75\x6e\x74\x69\x6d\x65\x20\x65\x72\x72\x6f\x72\x20",15); // Runtime Error

                    if(nOffset_RunTimeError!=-1)
                    {

                        SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FPC,"","",0);

                        // TODO Version
                        pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }
            }



            for(int i=0; i<pPEInfo->listImports.count(); i++)
            {
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^PYTHON")))
                {
                    QRegularExpression rxVersion("(\\d+)");
                    QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

                    if(matchVersion.hasMatch())
                    {
                        double dVersion=matchVersion.captured(0).toDouble()/10;

                        if(dVersion)
                        {
                            SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_PYTHON,"","",0);

                            ss.sVersion=QString::number(dVersion,'f',1);
                            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }

                    break;
                }
            }

            for(int i=0; i<pPEInfo->listImports.count(); i++)
            {
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^CYGWIN")))
                {
                    QRegularExpression rxVersion("(\\d+)");
                    QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

                    if(matchVersion.hasMatch())
                    {
                        double dVersion=matchVersion.captured(0).toDouble();

                        if(dVersion)
                        {
                            // TODO
                        }
                    }


                    SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_CYGWIN,"","",0);

                    // TODO version
                    pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                    if(!pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC))
                    {
                        SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);

                        pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }

                    break;
                }
            }
        }

        if(pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC))
        {
            if(pPEInfo->mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER))
            {
                // TODO Check version;
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LINKER,RECORD_NAME_GNULINKER,"","",0);
                ss.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if((!pPEInfo->mapResultTools.contains(RECORD_NAME_MINGW))&&
                    (!pPEInfo->mapResultTools.contains(RECORD_NAME_MSYS))&&
                    (!pPEInfo->mapResultTools.contains(RECORD_NAME_MSYS2)))
            {
                SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
            }
        }
    }
}

void SpecAbstract::handle_Installers(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);
    //

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_INNOSETUP)||pPEInfo->mapHeaderDetects.contains(RECORD_NAME_INNOSETUP))
            {
                SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_INNOSETUP,"","",0);

                if(pe.read_uint32(0x30)==0x6E556E49) // Uninstall
                {
                    ss.sInfo="Uninstall";

                    if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize))
                    {
                        qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                        qint64 _nSize=pPEInfo->nCodeSectionSize;

                        qint64 nOffsetVersion=pe.find_ansiString(_nOffset,_nSize,"Setup version: Inno Setup version ");

                        if(nOffsetVersion!=-1)
                        {
                            QString sVersionString=pe.read_ansiString(nOffsetVersion+34);
                            ss.sVersion=sVersionString.section(" ",0,0);
                            QString sEncodes=sVersionString.section(" ",1,1);

                            if(sEncodes=="(a)")
                            {
                                ss.sInfo=append(ss.sInfo,"ANSI");
                            }
                            else if(sEncodes=="(u)")
                            {
                                ss.sInfo=append(ss.sInfo,"Unicode");
                            }
                        }
                    }
                }
                else
                {
                    qint64 nLdrTableOffset=-1;

                    if(pe.read_uint32(0x30)==0x6F6E6E49)
                    {
                        ss.sVersion="1.XX-5.1.X";
                        ss.sInfo="Install";
                        nLdrTableOffset=pe.read_uint32(0x30+4);
                    }
                    else // New versions
                    {
                        QPE::RESOURCE_HEADER resHeader=QPE::getResourceHeader(S_RT_RCDATA,11111,&(pPEInfo->listResources));

                        nLdrTableOffset=resHeader.nOffset;

                        if(nLdrTableOffset!=-1)
                        {
                            ss.sVersion="5.1.X-X.X.X";
                            ss.sInfo="Install";
                        }
                    }

                    if(nLdrTableOffset!=-1)
                    {
                        // TODO 1 function
                        QString sSignature=pe.getSignature(nLdrTableOffset+0,12);

                        if(sSignature.left(12)=="72446C507453") // rDlPtS
                        {
                            //                    result.nLdrTableVersion=read_uint32(nLdrTableOffset+12+0);
                            //                    result.nTotalSize=read_uint32(nLdrTableOffset+12+4);
                            //                    result.nSetupE32Offset=read_uint32(nLdrTableOffset+12+8);
                            //                    result.nSetupE32UncompressedSize=read_uint32(nLdrTableOffset+12+12);
                            //                    result.nSetupE32CRC=read_uint32(nLdrTableOffset+12+16);
                            //                    result.nSetupBin0Offset=read_uint32(nLdrTableOffset+12+20);
                            //                    result.nSetupBin1Offset=read_uint32(nLdrTableOffset+12+24);
                            //                    result.nTableCRC=read_uint32(nLdrTableOffset+12+28);

                            QString sSetupDataString=pe.read_ansiString(pe.read_uint32(nLdrTableOffset+12+20));

                            if(!sSetupDataString.contains("("))
                            {
                                sSetupDataString=pe.read_ansiString(pe.read_uint32(nLdrTableOffset+12+24));
                                ss.sInfo=append(ss.sInfo,"OLD.TODO");
                            }

                            QRegularExpression rxVersion("\\(\\S+\\)");
                            QRegularExpression rxOptions("\\) \\(\\S+\\)");
                            QRegularExpressionMatch matchVersion=rxVersion.match(sSetupDataString);
                            QRegularExpressionMatch matchOptions=rxOptions.match(sSetupDataString);

                            if(matchVersion.hasMatch())
                            {
                                ss.sVersion=matchVersion.captured(0).remove("(").remove(")");
                            }

                            if(matchOptions.hasMatch())
                            {
                                QString sEncode=matchOptions.captured(0).remove("(").remove(")").remove(" ");

                                if(sEncode=="a")
                                {
                                    ss.sInfo=append(ss.sInfo,"ANSI");
                                }
                                else if(sEncode=="u")
                                {
                                    ss.sInfo=append(ss.sInfo,"Unicode");
                                }
                            }
                        }
                    }
                }

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }
}

void SpecAbstract::handle_ENIGMA(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    bool bDetect=false;

    SpecAbstract::SCANS_STRUCT recordEnigma= {0};

    recordEnigma.type=SpecAbstract::RECORD_TYPE_PROTECTOR;
    recordEnigma.name=SpecAbstract::RECORD_NAME_ENIGMA;

    if(!pPEInfo->cliInfo.bInit)
    {
        if(pPEInfo->listImports.count()>=2)
        {
            bool bKernel32Old=false;
            bool bKernel32New=false;
            bool bUser32=false;

            if(pPEInfo->listImports.at(0).sName.toUpper()=="KERNEL32.DLL")
            {
                if(pPEInfo->listImports.at(0).listPositions.count()==6)
                {
                    if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="LoadLibraryA"))
                    {
                        bKernel32Old=true;
                    }
                }
            }

            if(pPEInfo->listImports.at(0).sName.toUpper()=="KERNEL32.DLL")
            {
                if(pPEInfo->listImports.at(0).listPositions.count()==4)
                {
                    if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="LoadLibraryA"))
                    {
                        bKernel32New=true;
                    }
                }
            }

            if(pPEInfo->listImports.at(1).sName.toUpper()=="USER32.DLL")
            {
                if(pPEInfo->listImports.at(1).listPositions.count()==1)
                {
                    if(pPEInfo->listImports.at(1).listPositions.at(0).sName=="MessageBoxA")
                    {
                        bUser32=true;
                    }
                }
            }

            if(pPEInfo->nImportSection!=-1)
            {
                qint64 nSectionOffset=pPEInfo->listSections.at(pPEInfo->nImportSection).PointerToRawData;
                qint64 nSectionSize=pPEInfo->listSections.at(pPEInfo->nImportSection).SizeOfRawData;

                if(bKernel32Old&&bUser32)
                {
                    QPE pe(pDevice);

                    if(pe.isValid())
                    {
                        // mb TODO ENIGMA string
                        if(!bDetect)
                        {
                            qint64 nOffset=pe.find_array(nSectionOffset,nSectionSize," *** Enigma protector v",23);

                            if(nOffset!=-1)
                            {
                                recordEnigma.sVersion=pe.read_ansiString(nOffset+23).section(" ",0,0);
                                bDetect=true;
                            }
                        }

                        if(!bDetect)
                        {
                            QString sEnigmaVersion=findEnigmaVersion(pDevice,nSectionOffset,nSectionSize);

                            if(sEnigmaVersion!="")
                            {
                                recordEnigma.sVersion=sEnigmaVersion;
                                bDetect=true;
                            }
                        }

                        if((!bDetect)&&((pPEInfo->listSections.at(pPEInfo->nImportSection).Characteristics==0xe0000020)||(pPEInfo->listSections.at(pPEInfo->nImportSection).Characteristics==0xe0000040)))
                        {
                            recordEnigma.sVersion="1.1x-1.2x";
                            bDetect=true;
                        }

                        if(!bDetect)
                        {
                            recordEnigma.sVersion="Unknown";
                            bDetect=true;
                        }
                    }
                }
                else if(bKernel32New&&bUser32)
                {
                    QString sEnigmaVersion=findEnigmaVersion(pDevice,nSectionOffset,nSectionSize);

                    if(sEnigmaVersion!="")
                    {
                        recordEnigma.sVersion=sEnigmaVersion;
                        bDetect=true;
                    }
                }
            }
        }
    }
    else
    {
        QString sEnigmaVersion=findEnigmaVersion(pDevice,pPEInfo->nCodeSectionOffset,pPEInfo->nCodeSectionSize);

        if(sEnigmaVersion!="")
        {
            recordEnigma.sVersion=sEnigmaVersion;
            recordEnigma.sInfo=".NET";

            bDetect=true;
        }
    }

    if(bDetect)
    {
        pPEInfo->mapResultProtectors.insert(recordEnigma.name,scansToScan(&(pPEInfo->basic_info),&recordEnigma));
    }

}


void SpecAbstract::handle(SpecAbstract::PEINFO_STRUCT *pPEInfo, SpecAbstract::RECORD_NAMES name, SpecAbstract::RESULT_PRIOS prio0, SpecAbstract::RESULT_PRIOS prio1, SpecAbstract::RESULT_PRIOS prio2)
{
    SCANS_STRUCT scansStruct= {0};

    QMap<RECORD_NAMES,SCANS_STRUCT> *pMap0=getDetectsMap(pPEInfo,prio0);
    QMap<RECORD_NAMES,SCANS_STRUCT> *pMap1=getDetectsMap(pPEInfo,prio1);
    QMap<RECORD_NAMES,SCANS_STRUCT> *pMap2=getDetectsMap(pPEInfo,prio2);

    if(pMap0&&(pMap0->contains(name)))
    {
        scansStruct=pMap0->value(name);
    }
    else if(pMap1&&(pMap1->contains(name)))
    {
        scansStruct=pMap1->value(name);
    }
    else if(pMap2&&(pMap2->contains(name)))
    {
        scansStruct=pMap2->value(name);
    }

    if(scansStruct.name!=RECORD_NAME_UNKNOWN)
    {
        SCAN_STRUCT ss=scansToScan(&(pPEInfo->basic_info),&scansStruct);

        if(ss.type==RECORD_TYPE_LINKER)
        {
            pPEInfo->mapResultLinkers.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_COMPILER)
        {
            pPEInfo->mapResultCompilers.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_LIBRARY)
        {
            pPEInfo->mapResultLibraries.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_TOOL)
        {
            pPEInfo->mapResultTools.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_PROTECTOR)
        {
            pPEInfo->mapResultProtectors.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_PACKER)
        {
            pPEInfo->mapResultPackers.insert(ss.name,ss);
        }
        else if(ss.type==RECORD_TYPE_INSTALLER)
        {
            pPEInfo->mapResultInstallers.insert(ss.name,ss);
        }
    }
}

QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCANS_STRUCT> *SpecAbstract::getDetectsMap(SpecAbstract::PEINFO_STRUCT *pPEInfo, SpecAbstract::RESULT_PRIOS prio)
{
    QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCANS_STRUCT> *pMapResult=0;

    if(prio==SpecAbstract::RESULT_PRIO_CODESECTIONSCAN)
    {
        pMapResult=&(pPEInfo->mapCodeSectionScanDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_DATASECTIONSCAN)
    {
        pMapResult=&(pPEInfo->mapDataSectionScanDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_HEADERSCAN)
    {
        pMapResult=&(pPEInfo->mapHeaderScanDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_ENTRYPOINT)
    {
        pMapResult=&(pPEInfo->mapEntryPointDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_HEADER)
    {
        pMapResult=&(pPEInfo->mapHeaderDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_IMPORT)
    {
        pMapResult=&(pPEInfo->_mapImportDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_EXPORT)
    {
        pMapResult=&(pPEInfo->mapExportDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_OVERLAY)
    {
        pMapResult=&(pPEInfo->mapOverlayDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_RICH)
    {
        pMapResult=&(pPEInfo->mapRichDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_RESOURCES)
    {
        pMapResult=&(pPEInfo->mapResourcesDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_DOTANSISTRINGS)
    {
        pMapResult=&(pPEInfo->mapDotAnsistringsDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_DOTUNICODESTRINGS)
    {
        pMapResult=&(pPEInfo->mapDotUnicodestringsDetects);
    }
    else if(prio==SpecAbstract::RESULT_PRIO_SPECIAL)
    {
        pMapResult=&(pPEInfo->mapSpecialDetects);
    }

    return pMapResult;
}

void SpecAbstract::fixDetects(SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    if(pPEInfo->mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKER)&&pPEInfo->mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER))
    {
        pPEInfo->mapHeaderDetects.remove(RECORD_NAME_MICROSOFTLINKER);
    }

    if(pPEInfo->_mapImportDetects.contains(RECORD_NAME_C)&&pPEInfo->_mapImportDetects.contains(RECORD_NAME_VISUALCPP))
    {
        pPEInfo->_mapImportDetects.remove(RECORD_NAME_VISUALCPP);
    }

    if(pPEInfo->mapSpecialDetects.contains(RECORD_NAME_ENIGMA))
    {
        pPEInfo->mapEntryPointDetects.remove(RECORD_NAME_BORLANDCPP);
    }
}

//SpecAbstract::handle_ObjectPascal(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapCodeSectionDetects.contains(SpecAbstract::RECORD_NAME_OBJECTPASCAL))
//    {
////        pPEInfo->mapResultCompilers.insert(SpecAbstract::RECORD_NAME_OBJECTPASCAL,getScanStruct(&(pPEInfo->mapCodeSectionDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_OBJECTPASCAL));
//        pPEInfo->mapResultCompilers.insert(SpecAbstract::RECORD_NAME_OBJECTPASCAL,scansToScan(&(pPEInfo->basic_info),pPEInfo->mapCodeSectionDetects.value(RECORD_NAME_OBJECTPASCAL)));
//    }
//}

//SpecAbstract::handle_BorlandCPP(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapCodeSectionDetects.contains(SpecAbstract::RECORD_NAME_BORLANDCPP))
//    {
//        pPEInfo->mapResultCompilers.insert(SpecAbstract::RECORD_NAME_BORLANDCPP,getScanStruct(&(pPEInfo->mapCodeSectionDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_BORLANDCPP));
//    }
//}

//SpecAbstract::handle_Qt(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapImportDetects.contains(SpecAbstract::RECORD_NAME_QT))
//    {
//        pPEInfo->mapResultLibraries.insert(SpecAbstract::RECORD_NAME_QT,getScanStruct(&(pPEInfo->mapImportDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_QT));
//    }
//}

//SpecAbstract::handle_FASM(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapHeaderDetects.contains(SpecAbstract::RECORD_NAME_FASM))
//    {
//        SCAN_STRUCT record=getScanStruct(&(pPEInfo->mapHeaderDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_FASM);
//        record.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
//        pPEInfo->mapResultCompilers.insert(SpecAbstract::RECORD_NAME_FASM,record);
//    }
//}

//SpecAbstract::handle_MinGW(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapImportDetects.contains(SpecAbstract::RECORD_NAME_MINGW))
//    {
//        pPEInfo->mapResultTools.insert(SpecAbstract::RECORD_NAME_MINGW,getScanStruct(&(pPEInfo->mapImportDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_MINGW));
//    }
//}

//SpecAbstract::handle_MFC(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->mapImportDetects.contains(SpecAbstract::RECORD_NAME_MFC))
//    {
//        pPEInfo->mapResultLibraries.insert(SpecAbstract::RECORD_NAME_MFC,getScanStruct(&(pPEInfo->mapImportDetects),&(pPEInfo->basic_info),SpecAbstract::RECORD_NAME_MFC));
//    }
//}

//SpecAbstract::handle_VisualBasic(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    // TODO
//}

void SpecAbstract::fixResult(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    // .NET
    if(pPEInfo->cliInfo.bInit)
    {
        SpecAbstract::SCANS_STRUCT recordLibrary= {0};
        recordLibrary.type=SpecAbstract::RECORD_TYPE_LIBRARY;
        recordLibrary.name=SpecAbstract::RECORD_NAME_DOTNET;
        recordLibrary.sVersion=pPEInfo->cliInfo.sCLI_MetaData_Version;

        pPEInfo->mapResultLibraries.insert(SpecAbstract::RECORD_NAME_DOTNET,scansToScan(&(pPEInfo->basic_info),&recordLibrary));
    }

    // FASM
    if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_FASM))
    {
        // FASM does not have linker
        pPEInfo->mapResultLinkers.clear();
        QString sVersion=get_FASM_vi(pDevice,pPEInfo).sVersion;

        if(sVersion!="")
        {
            updateVersion(&(pPEInfo->mapResultCompilers),SpecAbstract::RECORD_NAME_FASM,sVersion);
        }
    }

    // Turbo Linker
    if(pPEInfo->mapResultLinkers.contains(SpecAbstract::RECORD_NAME_TURBOLINKER))
    {
        QString sVersion=get_TurboLinker_vi(pDevice,pPEInfo).sVersion;

        if(sVersion!="")
        {
            updateVersion(&(pPEInfo->mapResultLinkers),SpecAbstract::RECORD_NAME_TURBOLINKER,sVersion);
        }
    }

    // UPX
    if(pPEInfo->mapResultPackers.contains(SpecAbstract::RECORD_NAME_UPX))
    {
        if(isValid_UPX(pDevice,pPEInfo))
        {
            VI_STRUCT vi=get_UPX_vi(pDevice,pPEInfo);

            if(vi.sVersion!="")
            {
                updateVersionAndInfo(&(pPEInfo->mapResultPackers),SpecAbstract::RECORD_NAME_UPX,vi.sVersion,vi.sInfo);
            }
        }
    }

    // WWPack32
    if(pPEInfo->mapResultPackers.contains(SpecAbstract::RECORD_NAME_WWPACK32))
    {
        QString sVersion=get_WWPack32_vi(pDevice,pPEInfo).sVersion;

        if(sVersion!="")
        {
            updateVersion(&(pPEInfo->mapResultPackers),SpecAbstract::RECORD_NAME_WWPACK32,sVersion);
        }
    }

    // Inno Setup
    if(pPEInfo->mapResultInstallers.contains(SpecAbstract::RECORD_NAME_INNOSETUP))
    {
        VI_STRUCT vi=get_InnoSetup_vi(pDevice,pPEInfo);

        if(vi.sVersion!="")
        {
            updateVersionAndInfo(&(pPEInfo->mapResultInstallers),SpecAbstract::RECORD_NAME_INNOSETUP,vi.sVersion,vi.sInfo);
        }
    }

    // FlexLM
    if(pPEInfo->mapResultLibraries.contains(SpecAbstract::RECORD_NAME_FLEXLM))
    {
        VI_STRUCT vi=get_FlexLM_vi(pDevice,pPEInfo);

        if(vi.sVersion!="")
        {
            updateVersionAndInfo(&(pPEInfo->mapResultLibraries),SpecAbstract::RECORD_NAME_FLEXLM,vi.sVersion,vi.sInfo);
        }
    }

    // FlexNet
    if(pPEInfo->mapResultLibraries.contains(SpecAbstract::RECORD_NAME_FLEXNET))
    {
        VI_STRUCT vi=get_FlexNet_vi(pDevice,pPEInfo);

        if(vi.sVersion!="")
        {
            updateVersionAndInfo(&(pPEInfo->mapResultLibraries),SpecAbstract::RECORD_NAME_FLEXNET,vi.sVersion,vi.sInfo);
        }
    }



    // Borland
    if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_BORLANDCPP)||
            pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_CODEGEARCPP)||
            pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_EMBARCADEROCPP)||
            pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_OBJECTPASCAL)||
            pPEInfo->mapResultLibraries.contains(SpecAbstract::RECORD_NAME_VCL)||
            pPEInfo->mapResultTools.contains(SpecAbstract::RECORD_NAME_BORLANDDELPHIDOTNET)||
            pPEInfo->mapResultTools.contains(SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET)||
            pPEInfo->mapResourcesDetects.contains(SpecAbstract::RECORD_NAME_DVCLAL))
    {
        SpecAbstract::SCANS_STRUCT recordTool= {0};
        recordTool.type=SpecAbstract::RECORD_TYPE_TOOL;
        SpecAbstract::SCANS_STRUCT recordCompiler= {0};
        recordCompiler.type=SpecAbstract::RECORD_TYPE_COMPILER;

        SpecAbstract::SCANS_STRUCT recordVCL= {0};
        recordVCL.type=SpecAbstract::RECORD_TYPE_LIBRARY;
        recordVCL.name=SpecAbstract::RECORD_NAME_VCL;


        bool bVCL=pPEInfo->mapResultLibraries.contains(SpecAbstract::RECORD_NAME_VCL);


        if(!pPEInfo->cliInfo.bInit)
        {
            bool bCpp=!((pPEInfo->nMajorLinkerVersion==2)&&(pPEInfo->nMinorLinkerVersion==2.25));

            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_BORLANDCPP)||
                    pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_CODEGEARCPP)||
                    pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_EMBARCADEROCPP))
            {
                bCpp=true;
            }


            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_OBJECTPASCAL)&&bCpp)
            {
                bVCL=true;
            }


            if(bVCL)
            {
                VCL_PACKAGEINFO pi=getVCLPackageInfo(pDevice,&pPEInfo->listResources);

                if(pi.listModules.count())
                {
                    quint32 nProducer=(pi.nFlags>>26)&0x3;

                    if(nProducer==2) // C++
                    {
                        bCpp=true;
                    }
                    else if(nProducer==3) // Pascal
                    {
                        bCpp=false;
                    }

                    //                    for(int i=0;i<pi.listModules.count();i++)
                    //                    {
                    //                        qDebug(pi.listModules.at(i).sName.toLatin1().data());
                    //                    }
                }
            }

            if(!bCpp)
            {
                recordCompiler.name=SpecAbstract::RECORD_NAME_OBJECTPASCAL;
                recordTool.name=SpecAbstract::RECORD_NAME_BORLANDDELPHI;

                if(pPEInfo->mapCodeSectionScanDetects.value(SpecAbstract::RECORD_NAME_OBJECTPASCAL).nVariant==0)
                {
                    recordTool.sVersion="2";
                }
            }
            else
            {
                recordCompiler.name=SpecAbstract::RECORD_NAME_BORLANDCPP;
                recordTool.name=SpecAbstract::RECORD_NAME_BORLANDCPPBUILDER;
            }


            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_CODEGEARCPP))
            {
                recordCompiler.name=SpecAbstract::RECORD_NAME_CODEGEARCPP;
                pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_BORLANDCPP);
            }

            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_EMBARCADEROCPP))
            {
                recordCompiler.name=SpecAbstract::RECORD_NAME_EMBARCADEROCPP;
                pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_BORLANDCPP);
            }

            // VCL
            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_OBJECTPASCAL))
            {

                QList<SpecAbstract::VCL_STRUCT> listVcl_struct=getVCLstruct(pDevice,pPEInfo->nCodeSectionOffset,pPEInfo->nCodeSectionSize,pPEInfo->bIs64);

                if(listVcl_struct.count())
                {
                    int nVCLOffset=listVcl_struct.at(0).nOffset;
                    int nVCLValue=listVcl_struct.at(0).nValue;
                    bVCL=true;

                    if(nVCLOffset==24)
                    {
                        if(nVCLValue==168)
                        {
                            recordTool.sVersion="2";
                            recordVCL.sVersion="20";
                        }
                    }
                    else if(nVCLOffset==28)
                    {
                        if(nVCLValue==180)
                        {
                            recordTool.sVersion="3";
                            recordVCL.sVersion="30";
                        }
                    }
                    else if(nVCLOffset==40)
                    {
                        if(nVCLValue==276)
                        {
                            recordTool.sVersion="4";
                            recordVCL.sVersion="40";
                        }
                    }
                    else if(nVCLOffset==40)
                    {
                        if(nVCLValue==288)
                        {
                            recordTool.sVersion="5";
                            recordVCL.sVersion="50";
                        }
                    }
                    //                else if(nVCLOffset==40)
                    //                {
                    //                    if(nVCLValue==264)
                    //                    {
                    //                        recordTool.sVersion="???TODO";
                    //                        sVCLVersion="50";
                    //                    }
                    //                }
                    else if(nVCLOffset==40)
                    {
                        if(nVCLValue==348)
                        {
                            recordTool.sVersion="6-7";
                            recordVCL.sVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==40)
                    {
                        if(nVCLValue==356)
                        {
                            recordTool.sVersion="2005";
                            //                        sVCLVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==40)
                    {
                        if(nVCLValue==400)
                        {
                            recordTool.sVersion="2006";
                            //                        sVCLVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==52)
                    {
                        if(nVCLValue==420)
                        {
                            recordTool.sVersion="2009";
                            //                        sVCLVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==52)
                    {
                        if(nVCLValue==428)
                        {
                            recordTool.sVersion="2010-XE";
                            //                        sVCLVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==52)
                    {
                        if(nVCLValue==436)
                        {
                            recordTool.sVersion="XE2-XE4";
                            //                        sVCLVersion="60-70";
                        }
                    }
                    else if(nVCLOffset==52)
                    {
                        if(nVCLValue==444)
                        {
                            recordTool.sVersion="XE2-XE8";
                            //                        sVCLVersion="60-70";
                        }
                    }
                }
            }

            // Borland C++
            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_BORLANDCPP))
            {
                recordTool.name=SpecAbstract::RECORD_NAME_BORLANDCPPBUILDER;

                QString sVersion=get_BorlandCpp_vi(pDevice,pPEInfo).sVersion;

                if(sVersion!="")
                {
                    recordCompiler.sVersion=sVersion;
                }
            }


            // CodeGear C++
            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_CODEGEARCPP))
            {
                recordTool.name=SpecAbstract::RECORD_NAME_CODEGEARCPPBUILDER;

                QString sVersion=get_CodegearCpp_vi(pDevice,pPEInfo).sVersion;

                if(sVersion!="")
                {
                    recordCompiler.sVersion=sVersion;
                }
            }

            // Embarcadero C++
            if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_EMBARCADEROCPP))
            {
                recordTool.name=SpecAbstract::RECORD_NAME_EMBARCADEROCPPBUILDER;

                QString sVersion=get_EmbarcaderoCpp_vi(pDevice,pPEInfo).sVersion;

                if(sVersion!="")
                {
                    recordCompiler.sVersion=sVersion;
                }
            }
        }
        else
        {
            if(bVCL)
            {
                recordVCL.sInfo=".NET";
            }

            recordCompiler.name=SpecAbstract::RECORD_NAME_CIL;

            if(pPEInfo->mapResultTools.contains(RECORD_NAME_EMBARCADERODELPHIDOTNET))
            {
                recordTool.name=SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET;
            }
            else
            {
                recordTool.name=SpecAbstract::RECORD_NAME_BORLANDDELPHIDOTNET;
            }
        }

        pPEInfo->mapResultLibraries.remove(SpecAbstract::RECORD_NAME_VCL);
        pPEInfo->mapResultTools.remove(SpecAbstract::RECORD_NAME_BORLANDDELPHIDOTNET);
        pPEInfo->mapResultTools.remove(SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET);
        pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_BORLANDCPP);
        pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_CODEGEARCPP);
        pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_EMBARCADEROCPP);
        pPEInfo->mapResultCompilers.remove(SpecAbstract::RECORD_NAME_OBJECTPASCAL);

        if(bVCL)
        {
            pPEInfo->mapResultLibraries.insert(recordVCL.name,scansToScan(&(pPEInfo->basic_info),&recordVCL));
        }

        pPEInfo->mapResultCompilers.insert(recordCompiler.name,scansToScan(&(pPEInfo->basic_info),&recordCompiler));
        pPEInfo->mapResultTools.insert(recordTool.name,scansToScan(&(pPEInfo->basic_info),&recordTool));
    }

    // MFC
    // TODO together RECORD_NAME_VISUALCPP
    if(pPEInfo->mapResultLibraries.contains(SpecAbstract::RECORD_NAME_MFC))
    {
        if(pPEInfo->mapResultLibraries.value(SpecAbstract::RECORD_NAME_MFC).sVersion=="")
        {
            QString sVersion=get_MFC_vi(pDevice,pPEInfo).sVersion;

            if(sVersion!="")
            {
                if(!pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_VISUALCPP))
                {
                    SpecAbstract::SCANS_STRUCT recordVC= {0};

                    recordVC.type=SpecAbstract::RECORD_TYPE_COMPILER;
                    recordVC.name=SpecAbstract::RECORD_NAME_VISUALCPP;

                    if(sVersion=="6.00")
                    {
                        recordVC.sVersion="12.00";
                    }
                    else if(sVersion=="7.00")
                    {
                        recordVC.sVersion="13.00";
                    }
                    else if(sVersion=="7.10")
                    {
                        recordVC.sVersion="13.10";
                    }
                    else if(sVersion=="8.00")
                    {
                        recordVC.sVersion="14.00";
                    }
                    else if(sVersion=="9.00")
                    {
                        recordVC.sVersion="15.00";
                    }
                    else if(sVersion=="10.00")
                    {
                        recordVC.sVersion="16.00";
                    }
                    else if(sVersion=="11.00")
                    {
                        recordVC.sVersion="17.00";
                    }
                    else if(sVersion=="12.00")
                    {
                        recordVC.sVersion="18.00";
                    }
                    else if(sVersion=="14.00")
                    {
                        recordVC.sVersion="19.00";
                    }

                    pPEInfo->mapResultCompilers.insert(recordVC.name,scansToScan(&(pPEInfo->basic_info),&recordVC));
                }

                if(!pPEInfo->mapResultLinkers.contains(SpecAbstract::RECORD_NAME_MICROSOFTLINKER))
                {
                    SpecAbstract::SCANS_STRUCT recordMSL= {0};

                    recordMSL.type=SpecAbstract::RECORD_TYPE_COMPILER;
                    recordMSL.name=SpecAbstract::RECORD_NAME_VISUALCPP;

                    // TODO check linkers version 0-6.0
                    recordMSL.sVersion=sVersion;


                    pPEInfo->mapResultCompilers.insert(recordMSL.name,scansToScan(&(pPEInfo->basic_info),&recordMSL));
                }
                else
                {
                    QString sLinkerVersion=pPEInfo->mapResultLinkers.value(SpecAbstract::RECORD_NAME_MICROSOFTLINKER).sVersion;

                    if(sLinkerVersion.left(5)==sVersion)
                    {
                        sVersion=sLinkerVersion;
                    }
                }

                updateVersion(&(pPEInfo->mapResultLibraries),SpecAbstract::RECORD_NAME_MFC,sVersion);
            }
            else
            {
                if(pPEInfo->mapResultLibraries.value(SpecAbstract::RECORD_NAME_MFC).sInfo=="")
                {
                    pPEInfo->mapResultLibraries.remove(SpecAbstract::RECORD_NAME_MFC);
                }

            }
        }
    }

    // Visual C++
    // https://en.wikipedia.org/wiki/Microsoft_Windows_library_files#MSVCRT.DLL.2C_MSVCP.2A.DLL_and_CRTDLL.DLL
    // TODO MSVCRT.DLL
    if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_VISUALCPP))
    {
        if(pPEInfo->mapResultCompilers.value(SpecAbstract::RECORD_NAME_VISUALCPP).sVersion=="")
        {
            QString sVersion=get_vc_vi(pDevice,pPEInfo).sVersion;
            updateVersion(&(pPEInfo->mapResultCompilers),SpecAbstract::RECORD_NAME_VISUALCPP,sVersion);
        }
    }

    // Ms linker
    if(pPEInfo->mapResultLinkers.contains(SpecAbstract::RECORD_NAME_MICROSOFTLINKER))
    {
        if(pPEInfo->mapResultLinkers.value(SpecAbstract::RECORD_NAME_MICROSOFTLINKER).sVersion=="")
        {
            QString sVersion=get_mslink_vi(pDevice,pPEInfo).sVersion;
            updateVersion(&(pPEInfo->mapResultLinkers),SpecAbstract::RECORD_NAME_MICROSOFTLINKER,sVersion);
        }
    }

    // Visual Studio
    if(pPEInfo->mapResultLinkers.contains(SpecAbstract::RECORD_NAME_MICROSOFTLINKER))
    {
        QString sLinkerVersion=pPEInfo->mapResultLinkers.value(SpecAbstract::RECORD_NAME_MICROSOFTLINKER).sVersion;

        if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_VISUALCPP))
        {
            QString sCompilerVersion=pPEInfo->mapResultCompilers.value(SpecAbstract::RECORD_NAME_VISUALCPP).sVersion;
            QString sCompilerMajorVersion=sCompilerVersion.left(5);

            SpecAbstract::SCANS_STRUCT recordVS= {0};

            recordVS.type=SpecAbstract::RECORD_TYPE_TOOL;
            recordVS.name=SpecAbstract::RECORD_NAME_MICROSOFTVISUALSTUDIO;


            if(sCompilerVersion=="12.00.8168")
            {
                recordVS.sVersion="6.0";
            }
            else if(sCompilerVersion=="12.00.8804")
            {
                recordVS.sVersion="6.0 SP5-SP6";
            }
            else if((sLinkerVersion=="7.00.9466")&&(sCompilerVersion=="13.00.9466"))
            {
                recordVS.sVersion="2002";
            }
            else if((sLinkerVersion=="7.10.3077")&&(sCompilerVersion=="13.10.3077"))
            {
                recordVS.sVersion="2003";
            }
            else if((sLinkerVersion=="7.10.4035")&&(sCompilerVersion=="13.10.4035"))
            {
                recordVS.sVersion="2003";
            }
            else if((sLinkerVersion=="7.10.6030")&&(sCompilerVersion=="13.10.6030"))
            {
                recordVS.sVersion="2003 SP1";
            }
            else if((sLinkerVersion=="8.00.40310")&&(sCompilerVersion=="14.00.40310"))
            {
                recordVS.sVersion="2005";
            }
            else if((sLinkerVersion=="8.00.50727")&&(sCompilerVersion=="14.00.50727"))
            {
                recordVS.sVersion="2005";
            }
            else if((sLinkerVersion=="9.00.21022")&&(sCompilerVersion=="15.00.21022"))
            {
                recordVS.sVersion="2008 RTM";
            }
            else if((sLinkerVersion=="9.00.30411")&&(sCompilerVersion=="15.00.30411"))
            {
                recordVS.sVersion="2008 with Feature Pack";
            }
            else if((sLinkerVersion=="9.00.30729")&&(sCompilerVersion=="15.00.30729"))
            {
                recordVS.sVersion="2008 SP1";
            }
            else if((sLinkerVersion=="10.00.30319")&&(sCompilerVersion=="16.00.30319"))
            {
                recordVS.sVersion="2010 RTM";
            }
            else if((sLinkerVersion=="10.00.40219")&&(sCompilerVersion=="16.00.40219"))
            {
                recordVS.sVersion="2010 SP1";
            }
            else if((sLinkerVersion=="11.00.50727")&&(sCompilerVersion=="17.00.50727"))
            {
                recordVS.sVersion="2012";
            }
            else if((sLinkerVersion=="11.00.51025")&&(sCompilerVersion=="17.00.51025"))
            {
                recordVS.sVersion="2012";
            }
            else if((sLinkerVersion=="11.00.51106")&&(sCompilerVersion=="17.00.51106"))
            {
                recordVS.sVersion="2012 update 1";
            }
            else if((sLinkerVersion=="11.00.60315")&&(sCompilerVersion=="17.00.60315"))
            {
                recordVS.sVersion="2012 update 2";
            }
            else if((sLinkerVersion=="11.00.60610")&&(sCompilerVersion=="17.00.60610"))
            {
                recordVS.sVersion="2012 update 3";
            }
            else if((sLinkerVersion=="11.00.61030")&&(sCompilerVersion=="17.00.61030"))
            {
                recordVS.sVersion="2012 update 4";
            }
            else if((sLinkerVersion=="12.00.21005")&&(sCompilerVersion=="18.00.21005"))
            {
                recordVS.sVersion="2013 RTM";
            }
            else if((sLinkerVersion=="12.00.30501")&&(sCompilerVersion=="18.00.30501"))
            {
                recordVS.sVersion="2013 update 2";
            }
            else if((sLinkerVersion=="12.00.30723")&&(sCompilerVersion=="18.00.30723"))
            {
                recordVS.sVersion="2013 update 3";
            }
            else if((sLinkerVersion=="12.00.31101")&&(sCompilerVersion=="18.00.31101"))
            {
                recordVS.sVersion="2013 update 4";
            }
            else if((sLinkerVersion=="12.00.40629")&&(sCompilerVersion=="18.00.40629"))
            {
                recordVS.sVersion="2013 SP5";
            }
            else if((sLinkerVersion=="14.00.22215")&&(sCompilerVersion=="19.00.22215"))
            {
                recordVS.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23007")&&(sCompilerVersion=="19.00.23007"))
            {
                recordVS.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23013")&&(sCompilerVersion=="19.00.23013"))
            {
                recordVS.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23026")&&(sCompilerVersion=="19.00.23026"))
            {
                recordVS.sVersion="2015";
            }
            else if((sLinkerVersion=="14.00.23506")&&(sCompilerVersion=="19.00.23506"))
            {
                recordVS.sVersion="2015 update 1";
            }
            else if((sLinkerVersion=="14.00.23918")&&(sCompilerVersion=="19.00.23918"))
            {
                recordVS.sVersion="2015 update 2";
            }
            else if((sLinkerVersion=="14.00.24103")&&(sCompilerVersion=="19.00.24103"))
            {
                recordVS.sVersion="2015 SP1"; // ???
            }
            else if((sLinkerVersion=="14.00.24118")&&(sCompilerVersion=="19.00.24118"))
            {
                recordVS.sVersion="2015 SP1"; // ???
            }
            else if((sLinkerVersion=="14.00.24210")&&(sCompilerVersion=="19.00.24210"))
            {
                recordVS.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24212")&&(sCompilerVersion=="19.00.24212"))
            {
                recordVS.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24213")&&(sCompilerVersion=="19.00.24213"))
            {
                recordVS.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24215")&&(sCompilerVersion=="19.00.24215"))
            {
                recordVS.sVersion="2015 update 3";
            }
            else if((sLinkerVersion=="14.00.24218")&&(sCompilerVersion=="19.00.24218"))
            {
                recordVS.sVersion="2015 update 3";
            }
            else if(sCompilerMajorVersion=="12.00")
            {
                recordVS.sVersion="6.0";
            }
            else if(sCompilerMajorVersion=="13.00")
            {
                recordVS.sVersion="2002";
            }
            else if(sCompilerMajorVersion=="13.10")
            {
                recordVS.sVersion="2003";
            }
            else if(sCompilerMajorVersion=="14.00")
            {
                recordVS.sVersion="2005";
            }
            else if(sCompilerMajorVersion=="15.00")
            {
                recordVS.sVersion="2008";
            }
            else if(sCompilerMajorVersion=="16.00")
            {
                recordVS.sVersion="2010";
            }
            else if(sCompilerMajorVersion=="17.00")
            {
                recordVS.sVersion="2012";
            }
            else if(sCompilerMajorVersion=="18.00")
            {
                recordVS.sVersion="2013";
            }
            else if(sCompilerMajorVersion=="19.00")
            {
                recordVS.sVersion="2015";
            }

            pPEInfo->mapResultTools.insert(recordVS.name,scansToScan(&(pPEInfo->basic_info),&recordVS));
        }
        else if(pPEInfo->mapResultCompilers.contains(SpecAbstract::RECORD_NAME_MASM))
        {
            QString sCompilerVersion=pPEInfo->mapResultCompilers.value(SpecAbstract::RECORD_NAME_MASM).sVersion;

            SpecAbstract::SCANS_STRUCT recordMASM32= {0};

            recordMASM32.type=SpecAbstract::RECORD_TYPE_TOOL;
            recordMASM32.name=SpecAbstract::RECORD_NAME_MASM32;

            if((sLinkerVersion=="5.12.8078")&&(sCompilerVersion=="6.14.8444"))
            {
                recordMASM32.sVersion="8-11";
                pPEInfo->mapResultTools.insert(recordMASM32.name,scansToScan(&(pPEInfo->basic_info),&recordMASM32));
            }
        }
    }
}

void SpecAbstract::updateVersion(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCAN_STRUCT> *map, SpecAbstract::RECORD_NAMES name, QString sVersion)
{
    if(map->contains(name))
    {
        SpecAbstract::SCAN_STRUCT record=map->value(name);
        record.sVersion=sVersion;
        map->insert(name,record);
    }
}

void SpecAbstract::updateInfo(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCAN_STRUCT> *map, SpecAbstract::RECORD_NAMES name, QString sInfo)
{
    if(map->contains(name))
    {
        SpecAbstract::SCAN_STRUCT record=map->value(name);
        record.sInfo=sInfo;
        map->insert(name,record);
    }
}

void SpecAbstract::updateVersionAndInfo(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCAN_STRUCT> *map, SpecAbstract::RECORD_NAMES name, QString sVersion, QString sInfo)
{
    if(map->contains(name))
    {
        SpecAbstract::SCAN_STRUCT record=map->value(name);
        record.sVersion=sVersion;
        record.sInfo=sInfo;
        map->insert(name,record);
    }
}

bool SpecAbstract::isScanStructPresent(QList<SpecAbstract::SCAN_STRUCT> *pList, SpecAbstract::RECORD_FILETYPES filetype, SpecAbstract::RECORD_TYPES type, SpecAbstract::RECORD_NAMES name, QString sVersion, QString sInfo)
{
    bool bResult=false;

    for(int i=0; i<pList->count(); i++)
    {
        if((pList->at(i).id.filetype==filetype)
                &&(pList->at(i).type==type)
                &&(pList->at(i).name==name)
                &&(pList->at(i).sVersion==sVersion)
                &&(pList->at(i).sInfo==sInfo))
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

bool SpecAbstract::checkVersionString(QString sVersion)
{
    bool bResult=true;

    // TODO
    for(int i=0; i<sVersion.size(); i++)
    {
        QChar _char=sVersion.at(i);

        if((_char>=QChar('0'))&&(_char<=QChar('9')))
        {

        }
        else if(_char==QChar('.'))
        {

        }
        else
        {
            bResult=false;
            break;
        }
    }


    return bResult;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_TurboLinker_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    VI_STRUCT result;
    // TODO
    result.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion,2,10,QChar('0'));

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_FASM_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    VI_STRUCT result;
    // TODO
    result.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion,2,10,QChar('0'));

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_UPX_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    // TODO unknown vesrion
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO make both
    qint64 nStringOffset1=binary.find_array(pPEInfo->nHeaderOffset,pPEInfo->nHeaderSize,"\x24\x49\x64\x3a\x20\x55\x50\x58\x20",9);
    qint64 nStringOffset2=binary.find_array(pPEInfo->nHeaderOffset,pPEInfo->nHeaderSize,"\x55\x50\x58\x21",4);

    if(nStringOffset1!=-1)
    {
        result.sVersion=binary.read_ansiString(nStringOffset1+9,10);
        result.sVersion=result.sVersion.section(" ",0,0);

        if(!checkVersionString(result.sVersion))
        {
            result.sVersion="";
        }

        // NRV
        qint64 nNRVStringOffset1=binary.find_array(pPEInfo->nHeaderOffset,pPEInfo->nHeaderSize,"\x24\x49\x64\x3a\x20\x4e\x52\x56\x20",9);

        if(nNRVStringOffset1!=-1)
        {
            QString sNRVVersion=binary.read_ansiString(nNRVStringOffset1+9,10);
            sNRVVersion=sNRVVersion.section(" ",0,0);

            if(checkVersionString(sNRVVersion))
            {
                result.sInfo=QString("NRV %1").arg(sNRVVersion);
            }
        }
    }

    if(nStringOffset2!=-1)
    {
        // TODO 1 function
        if(result.sVersion=="")
        {
            result.sVersion=binary.read_ansiString(nStringOffset2-5,4);
        }

        quint8 nMethod=binary.read_uint8(nStringOffset2+4+2);
        quint8 nLevel=binary.read_uint8(nStringOffset2+4+3);

        switch(nMethod) // From http://sourceforge.net/p/upx/code/ci/default/tree/src/conf.h
        {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                if(result.sInfo=="")
                {
                    result.sInfo="NRV";
                }

                break;

            case 14:
                result.sInfo="LZMA";
                break;

            case 15:
                result.sInfo="zlib";
                break;
        }

        if(result.sInfo!="")
        {
            if(nLevel==8)
            {
                result.sInfo=append(result.sInfo,"best");
            }
            else
            {
                result.sInfo=append(result.sInfo,"brute");
            }
        }
    }

    if(!checkVersionString(result.sVersion))
    {
        result.sVersion="";
    }

    // TODO modified

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_WWPack32_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    VI_STRUCT result;
    // TODO
    result.sVersion=QBinary::hexToString(pPEInfo->sEntryPointSignature.mid(102,8));

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_InnoSetup_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(pe.read_uint32(0x30)==0x6E556E49) // Uninstall
        {
            result.sInfo="Uninstall";
            // TODO Version
        }
        else
        {
            qint64 nLdrTableOffset=-1;

            if(pe.read_uint32(0x30)==0x6F6E6E49)
            {
                result.sVersion="1.XX-5.1.X";
                result.sInfo="Install";
                nLdrTableOffset=pe.read_uint32(0x30+4);
            }
            else // New versions
            {
                QPE::RESOURCE_HEADER resHeader=QPE::getResourceHeader(S_RT_RCDATA,11111,&(pPEInfo->listResources));

                nLdrTableOffset=resHeader.nOffset;

                if(nLdrTableOffset!=-1)
                {
                    result.sVersion="5.1.X-X.X.X";
                    result.sInfo="Install";
                }
            }

            if(nLdrTableOffset!=-1)
            {
                // TODO 1 function
                QString sSignature=pe.getSignature(nLdrTableOffset+0,12);

                if(sSignature.left(12)=="72446C507453") // rDlPtS
                {
                    //                    result.nLdrTableVersion=read_uint32(nLdrTableOffset+12+0);
                    //                    result.nTotalSize=read_uint32(nLdrTableOffset+12+4);
                    //                    result.nSetupE32Offset=read_uint32(nLdrTableOffset+12+8);
                    //                    result.nSetupE32UncompressedSize=read_uint32(nLdrTableOffset+12+12);
                    //                    result.nSetupE32CRC=read_uint32(nLdrTableOffset+12+16);
                    //                    result.nSetupBin0Offset=read_uint32(nLdrTableOffset+12+20);
                    //                    result.nSetupBin1Offset=read_uint32(nLdrTableOffset+12+24);
                    //                    result.nTableCRC=read_uint32(nLdrTableOffset+12+28);

                    QString sSetupDataString=pe.read_ansiString(pe.read_uint32(nLdrTableOffset+12+20));

                    if(!sSetupDataString.contains("("))
                    {
                        sSetupDataString=pe.read_ansiString(pe.read_uint32(nLdrTableOffset+12+24));
                        result.sInfo=append(result.sInfo,"OLD.TODO");
                    }

                    QRegularExpression rxVersion("\\(\\S+\\)");
                    QRegularExpression rxOptions("\\) \\(\\S+\\)");
                    QRegularExpressionMatch matchVersion=rxVersion.match(sSetupDataString);
                    QRegularExpressionMatch matchOptions=rxOptions.match(sSetupDataString);

                    if(matchVersion.hasMatch())
                    {
                        result.sVersion=matchVersion.captured(0).remove("(").remove(")");
                    }

                    if(matchOptions.hasMatch())
                    {
                        QString sEncode=matchOptions.captured(0).remove("(").remove(")").remove(" ");

                        if(sEncode=="a")
                        {
                            result.sInfo=append(result.sInfo,"Ansi");
                        }
                        else if(sEncode=="u")
                        {
                            result.sInfo=append(result.sInfo,"Unicode");
                        }
                    }
                }
            }
        }
    }



    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_mslink_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    VI_STRUCT result;
    // TODO
    result.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion,2,10,QChar('0'));

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_MFC_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    // https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library
    // TODO eMbedded Visual C++ 4.0 		mfcce400.dll 	MFC 6.0
    VI_STRUCT result;

    for(int i=0; i<pPEInfo->listImports.count(); i++)
    {
        if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^MFC")))
        {
            QRegularExpression rxVersion("(\\d+)");
            QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

            if(matchVersion.hasMatch())
            {
                double dVersion=matchVersion.captured(0).toDouble()/10;

                if(dVersion)
                {
                    result.sVersion=QString::number(dVersion,'f',2);
                }
            }
        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_vc_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    VI_STRUCT result;

    for(int i=0; i<pPEInfo->listImports.count(); i++)
    {
        if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^MSVC")))
        {
            if(pPEInfo->listImports.at(i).sName.toUpper()=="MSVCRT.DLL")
            {
                result.sVersion="12.00";
            }
            else
            {
                QRegularExpression rxVersion("(\\d+)");
                QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

                if(matchVersion.hasMatch())
                {
                    double dVersion=matchVersion.captured(0).toDouble()/10;

                    if(dVersion)
                    {
                        if(dVersion==7.00)
                        {
                            result.sVersion="13.00";
                        }
                        else if(dVersion==7.10)
                        {
                            result.sVersion="13.10";
                        }
                        else if(dVersion==8.00)
                        {
                            result.sVersion="14.00";
                        }
                        else if(dVersion==9.00)
                        {
                            result.sVersion="15.00";
                        }
                        else if(dVersion==10.00)
                        {
                            result.sVersion="16.00";
                        }
                        else if(dVersion==11.00)
                        {
                            result.sVersion="17.00";
                        }
                        else if(dVersion==12.00)
                        {
                            result.sVersion="18.00";
                        }
                        else if(dVersion==14.00)
                        {
                            result.sVersion="19.00";
                        }
                        else
                        {
                            result.sVersion=QString::number(dVersion,'f',2);
                        }
                    }
                }
            }

        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_BorlandCpp_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO Offset
    if(pPEInfo->nDataSectionSize)
    {
        qint64 nStringOffset=binary.find_ansiString(pPEInfo->nDataSectionOffset,pPEInfo->nDataSectionSize,"Borland C++ - Copyright ");

        if(nStringOffset!=-1)
        {
            result.sVersion=binary.read_ansiString(nStringOffset+24,4);
        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_EmbarcaderoCpp_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO Offset
    if(pPEInfo->nDataSectionSize)
    {
        qint64 nStringOffset=binary.find_ansiString(pPEInfo->nDataSectionOffset,pPEInfo->nDataSectionSize,"Embarcadero RAD Studio - Copyright ");

        if(nStringOffset!=-1)
        {
            result.sVersion=binary.read_ansiString(nStringOffset+35,4);
        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_CodegearCpp_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO Offset
    if(pPEInfo->nDataSectionSize)
    {
        qint64 nStringOffset=binary.find_ansiString(pPEInfo->nDataSectionOffset,pPEInfo->nDataSectionSize,"CodeGear C++ - Copyright ");

        if(nStringOffset!=-1)
        {
            result.sVersion=binary.read_ansiString(nStringOffset+25,4);
        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_FlexLM_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO Offset
    qint64 nOffset=pPEInfo->mapDataSectionScanDetects.value(SpecAbstract::RECORD_NAME_FLEXLM).nOffset;

    if(nOffset)
    {
        result.sVersion=binary.read_ansiString(nOffset+12,50);
        result.sVersion=result.sVersion.section(" ",0,0);

        if(result.sVersion.left(1)=="v")
        {
            result.sVersion.remove(0,1);
        }
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::get_FlexNet_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO Offset
    qint64 nOffset=pPEInfo->mapDataSectionScanDetects.value(SpecAbstract::RECORD_NAME_FLEXNET).nOffset;

    if(nOffset)
    {
        result.sVersion=binary.read_ansiString(nOffset+24,50);

        if(result.sVersion.contains("build"))
        {
            result.sVersion=result.sVersion.section(" ",0,2);
        }
        else
        {
            result.sVersion=result.sVersion.section(" ",0,0);
        }


        //        if(result.sVersion.left(1)=="v")
        //        {
        //            result.sVersion.remove(0,1);
        //        }
    }

    return result;
}

bool SpecAbstract::isValid_UPX(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    if(pPEInfo->listSections.count()>=3)
    {
        // pPEInfo->listSections.at(0).SizeOfRawData!=0 dump file
        if((pPEInfo->listSections.at(0).SizeOfRawData==0)&&((pPEInfo->nResourceSection==-1)||(pPEInfo->nResourceSection==2)))
        {
            return true;
        }
    }

    return false;
}

//SpecAbstract::SCAN_STRUCT SpecAbstract::getScanStruct(QMap<RECORD_NAMES, SCANS_STRUCT> *pMapDetects, BASIC_INFO *pBasicInfo, SpecAbstract::RECORD_NAMES recordName)
//{
//    SpecAbstract::SCAN_STRUCT record={0};

//    record.id=pBasicInfo->id;
//    record.nSize=pBasicInfo->nSize;
//    record.parentId=pBasicInfo->parentId;
//    record.type=pMapDetects->value(recordName).type;
//    record.name=pMapDetects->value(recordName).name;
//    record.sVersion=pMapDetects->value(recordName).sVersion;
//    record.sInfo=pMapDetects->value(recordName).sInfo;

//    return record;
//}

SpecAbstract::SCAN_STRUCT SpecAbstract::scansToScan(SpecAbstract::BASIC_INFO *pBasicInfo, SpecAbstract::SCANS_STRUCT *pScansStruct)
{
    SCAN_STRUCT result= {0};

    result.id=pBasicInfo->id;
    result.nSize=pBasicInfo->nSize;
    result.parentId=pBasicInfo->parentId;
    result.type=pScansStruct->type;
    result.name=pScansStruct->name;
    result.sVersion=pScansStruct->sVersion;
    result.sInfo=pScansStruct->sInfo;

    return result;
}

QByteArray SpecAbstract::_BasicPEInfoToArray(SpecAbstract::BASIC_PE_INFO *pInfo)
{
    QByteArray baResult;
    QDataStream ds(&baResult,QIODevice::ReadWrite);

    ds<<pInfo->nEntryPoint;

    return baResult;
}

SpecAbstract::BASIC_PE_INFO SpecAbstract::_ArrayToBasicPEInfo(const QByteArray *pbaArray)
{
    BASIC_PE_INFO result= {0};

    QDataStream ds((QByteArray *)pbaArray,QIODevice::ReadOnly);

    ds>>result.nEntryPoint;

    return result;
}

void SpecAbstract::memoryScan(QMap<RECORD_NAMES, SCANS_STRUCT> *pMmREcords, QIODevice *pDevice, qint64 nOffset, qint64 nSize, SpecAbstract::SCANMEMORY_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
{
    if(nSize)
    {
        QBinary binary(pDevice);

        int nSignaturesCount=nRecordsSize/sizeof(SIGNATURE_RECORD);

        for(int i=0; i<nSignaturesCount; i++)
        {
            if((pRecords[i].filetype==fileType1)||(pRecords[i].filetype==fileType2))
            {
                if(!pMmREcords->contains(pRecords[i].name))
                {
                    qint64 _nOffset=binary.find_array(nOffset,nSize,(char *)pRecords[i].pData,pRecords[i].nSize);

                    if(_nOffset!=-1)
                    {
                        SpecAbstract::SCANS_STRUCT record= {0};
                        record.nVariant=pRecords[i].nVariant;
                        record.filetype=pRecords[i].filetype;
                        record.type=pRecords[i].type;
                        record.name=pRecords[i].name;
                        record.sVersion=pRecords[i].pszVersion;
                        record.sInfo=pRecords[i].pszInfo;
                        record.nOffset=_nOffset;

                        pMmREcords->insert(record.name,record);
                    }
                }
            }
        }
    }
}

void SpecAbstract::signatureScan(QMap<RECORD_NAMES, SCANS_STRUCT> *pMapRecords, QString sSignature, SpecAbstract::SIGNATURE_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
{
    int nSignaturesCount=nRecordsSize/sizeof(SIGNATURE_RECORD);

    for(int i=0; i<nSignaturesCount; i++)
    {
        if((pRecords[i].filetype==fileType1)||(pRecords[i].filetype==fileType2))
        {
            if(!pMapRecords->contains(pRecords[i].name))
            {
                if(QBinary::compareSignatureStrings(sSignature,pRecords[i].pszSignature))
                {
                    SpecAbstract::SCANS_STRUCT record= {0};
                    record.nVariant=pRecords[i].nVariant;
                    record.filetype=pRecords[i].filetype;
                    record.type=pRecords[i].type;
                    record.name=pRecords[i].name;
                    record.sVersion=pRecords[i].pszVersion;
                    record.sInfo=pRecords[i].pszInfo;

                    record.nOffset=0;

                    pMapRecords->insert(record.name,record);
                }
            }
        }
    }
}

void SpecAbstract::resourcesScan(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::SCANS_STRUCT> *pMapRecords, QList<QPE::RESOURCE_HEADER> *pListResources, SpecAbstract::RESOURCES_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
{
    int nSignaturesCount=nRecordsSize/sizeof(RESOURCES_RECORD);

    for(int i=0; i<nSignaturesCount; i++)
    {
        if((pRecords[i].filetype==fileType1)||(pRecords[i].filetype==fileType2))
        {
            if(!pMapRecords->contains(pRecords[i].name))
            {
                bool bSuccess=false;

                if(pRecords[i].bIsString1)
                {
                    if(pRecords[i].bIsString2)
                    {
                        bSuccess=QPE::isResourcePresent(pRecords[i].pszName1,pRecords[i].pszName2,pListResources);
                    }
                    else
                    {
                        bSuccess=QPE::isResourcePresent(pRecords[i].pszName1,pRecords[i].nID2,pListResources);
                    }
                }
                else
                {
                    if(pRecords[i].bIsString2)
                    {
                        bSuccess=QPE::isResourcePresent(pRecords[i].nID1,pRecords[i].pszName2,pListResources);
                    }
                    else
                    {
                        bSuccess=QPE::isResourcePresent(pRecords[i].nID1,pRecords[i].nID2,pListResources);
                    }
                }

                if(bSuccess)
                {
                    SpecAbstract::SCANS_STRUCT record= {0};
                    record.nVariant=pRecords[i].nVariant;
                    record.filetype=pRecords[i].filetype;
                    record.type=pRecords[i].type;
                    record.name=pRecords[i].name;
                    record.sVersion=pRecords[i].pszVersion;
                    record.sInfo=pRecords[i].pszInfo;
                    record.nOffset=0;

                    pMapRecords->insert(record.name,record);
                }
            }
        }
    }
}

QList<SpecAbstract::VCL_STRUCT> SpecAbstract::getVCLstruct(QIODevice *pDevice,qint64 nOffset,qint64 nSize,bool bIs64)
{
    QList<VCL_STRUCT> listResult;

    QPE pe(pDevice);

    qint64 _nOffset=nOffset;
    qint64 _nSize=nSize;

    int nAddressSize=bIs64?8:4;

    while(_nSize>0)
    {
        qint64 nClassOffset=pe.find_array(_nOffset,_nSize,"\x07\x08\x54\x43\x6f\x6e\x74\x72\x6f\x6c",10);

        if(nClassOffset==-1)
        {
            break;
        }

        quint32 nDword=pe.read_uint32(nClassOffset+10);
        qint64 nClassOffset2=pe.addressToOffset(nDword);

        if(nClassOffset2!=-1)
        {
            for(int i=0; i<20; i++)
            {
                quint32 nValue=pe.read_uint32(nClassOffset2-nAddressSize*(i+1));

                if(nValue<=0xFFFF)
                {
                    VCL_STRUCT record= {0};

                    record.nValue=nValue;
                    record.nOffset=nAddressSize*(i+1);
                    record.bIs64=bIs64;

                    listResult.append(record);

                    break;
                }
            }
        }

        qint64 nDelta=(nClassOffset-_nOffset)+1;

        _nOffset+=nDelta;
        _nSize-=nDelta;
    }

    return listResult;
}

SpecAbstract::VCL_PACKAGEINFO SpecAbstract::getVCLPackageInfo(QIODevice *pDevice,QList<QPE::RESOURCE_HEADER> *pListResources)
{
    VCL_PACKAGEINFO result= {0};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        QPE::RESOURCE_HEADER rh=pe.getResourceHeader(10,"PACKAGEINFO",pListResources);

        if((rh.nOffset!=-1)&&(rh.nSize))
        {
            qint64 nOffset=rh.nOffset;
            quint32 nFlags=pe.read_uint32(nOffset);

            quint32 _nFlags=nFlags&0xFF00;

            if(_nFlags==0)
            {
                result.nFlags=nFlags;
                nOffset+=4;
                result.nUnknown=pe.read_uint32(nOffset);

                if(result.nUnknown==0)
                {
                    nOffset+=4;
                    result.nRequiresCount=pe.read_uint32(nOffset);
                    nOffset+=4;
                }
                else
                {
                    nOffset+=3;
                }

                int nCount=result.nRequiresCount?result.nRequiresCount:1000;


                for(int i=0; i<nCount; i++)
                {
                    if(nOffset-rh.nOffset>rh.nSize)
                    {
                        break;
                    }

                    VCL_PACKAGEINFO_MODULE vpm= {0};
                    vpm.nFlags=pe.read_uint8(nOffset);
                    nOffset++;
                    vpm.nHashCode=pe.read_uint8(nOffset);
                    nOffset++;
                    vpm.sName=pe.read_ansiString(nOffset);
                    nOffset+=vpm.sName.length()+1;

                    result.listModules.append(vpm);
                }
            }
        }
    }

    return result;
}

SpecAbstract::SCANS_STRUCT SpecAbstract::getRichSignatureDescription(quint32 nRichID)
{
    SpecAbstract::SCANS_STRUCT result= {0};

    if(nRichID)
    {
        quint32 nMajor=nRichID>>16;
        quint32 nMinor=nRichID&0xFFFF;

        switch(nMajor)
        {
            case 0x00D:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALBASIC;
                break;

            case 0x006:
            case 0x045:
            case 0x05e:
            case 0x07c:
            case 0x094:
            case 0x09a:
            case 0x0c9:
            case 0x0db:
            case 0x0ff:
                result.type=SpecAbstract::RECORD_TYPE_CONVERTER;
                result.name=SpecAbstract::RECORD_NAME_RESOURCE;
                break;

            case 0x03f:
            case 0x05c:
            case 0x07a:
            case 0x092:
            case 0x09b:
            case 0x0ca:
            case 0x0dc:
            case 0x100:
                result.type=SpecAbstract::RECORD_TYPE_LIBRARY;
                result.name=SpecAbstract::RECORD_NAME_EXPORT;
                break;

            case 0x019:
            case 0x09c:
            case 0x05d:
            case 0x07b:
            case 0x093:
            case 0x0cb:
            case 0x0dd:
            case 0x101:
                result.type=SpecAbstract::RECORD_TYPE_LIBRARY;
                result.name=SpecAbstract::RECORD_NAME_IMPORT;
                break;

            case 0x002:
            case 0x004:
            case 0x013:
            case 0x03d:
            case 0x05a:
            case 0x078:
            case 0x091:
            case 0x09d:
            case 0x0cc:
            case 0x0de:
            case 0x102:
                result.type=SpecAbstract::RECORD_TYPE_LINKER;
                result.name=SpecAbstract::RECORD_NAME_MICROSOFTLINKER;
                break;

            case 0x00f:
            case 0x012:
            case 0x040:
            case 0x07d:
            case 0x095:
            case 0x09e:
            case 0x0cd:
            case 0x0df:
            case 0x103:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_MASM;
                break;

            case 0x00a:
            case 0x015:
            case 0x01c:
            case 0x05f:
            case 0x06d:
            case 0x083:
            case 0x0aa:
            case 0x0ce:
            case 0x0e0:
            case 0x104:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                //            result.name=SpecAbstract::RECORD_NAME_MICROSOFTC;
                result.name=SpecAbstract::RECORD_NAME_VISUALCPP; // TODO Visual C++
                result.sInfo="C";
                break;

            case 0x00b:
            case 0x016:
            case 0x01d:
            case 0x060:
            case 0x06e:
            case 0x084:
            case 0x0ab:
            case 0x0cf:
            case 0x0e1:
            case 0x105:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCPP;
                result.sInfo="C++";
                break;

            case 0x089:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCPP;
                result.sInfo="C/LTCG";
                break;

            case 0x08a:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCPP;
                result.sInfo="C++/LTCG";
                break;

            //
            case 0x085: // auto
            case 0x086: // auto
            case 0x087: // auto
            case 0x088: // auto

            //
            case 0x0d0: // auto
            case 0x0d1: // auto
            case 0x0d2: // auto
            case 0x0d3: // auto
            case 0x0d4: // auto
            case 0x0d5: // auto
            case 0x0d6: // auto

            //
            case 0x0e2: // auto
            case 0x0e3: // auto
            case 0x0e4: // auto
            case 0x0e5: // auto
            case 0x0e6: // auto
            case 0x0e7: // auto
            case 0x0e8: // auto

            //
            case 0x0ac:
            case 0x0ad:
            case 0x0ae:
            case 0x0af:
            case 0x0b0:
            case 0x0b1:
            case 0x0b2:

            //
            case 0x106:
            case 0x107:
            case 0x108:
            case 0x109:
            case 0x10a:
            case 0x10b:
            case 0x10c:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCPP;
                break;
        }

        switch(nMajor)
        {
            case 0x006:
                result.sVersion="5.00";
                break;

            case 0x002:
                result.sVersion="5.10";
                break;

            case 0x013:
                result.sVersion="5.12";
                break;

            case 0x004:
            case 0x00d:
                result.sVersion="6.00";
                break;

            case 0x00a:
            case 0x00b:
            case 0x015:
            case 0x016:
                result.sVersion="12.00";
                break;

            case 0x012:
                result.sVersion="6.14";
                break;

            case 0x040:
            case 0x03d:
            case 0x045:
            case 0x03f:
            case 0x019:
                result.sVersion="7.00";
                break;

            case 0x01c:
            case 0x01d:
                result.sVersion="13.00";
                break;

            case 0x00f:
            case 0x05e:
            case 0x05c:
            case 0x05d:
            case 0x05a:
                result.sVersion="7.10";
                break;

            case 0x05f:
            case 0x060:
                result.sVersion="13.10";
                break;

            case 0x078:
            case 0x07a:
            case 0x07b:
            case 0x07c:
            case 0x07d:
                result.sVersion="8.00";
                break;

            case 0x06d:
            case 0x06e:
                result.sVersion="14.00";
                break;

            case 0x091:
            case 0x092:
            case 0x093:
            case 0x094:
            case 0x095:
                result.sVersion="9.00";
                break;

            case 0x083:
            case 0x084:
            case 0x085: // auto
            case 0x086: // auto
            case 0x087: // auto
            case 0x088: // auto
            case 0x089:
            case 0x08a:
                result.sVersion="15.00";
                break;

            case 0x09a:
            case 0x09b:
            case 0x09c:
            case 0x09d:
            case 0x09e:
                result.sVersion="10.00";
                break;

            case 0x0aa:
            case 0x0ab:
            case 0x0ac:
            case 0x0ad:
            case 0x0ae:
            case 0x0af:
            case 0x0b0:
            case 0x0b1:
            case 0x0b2:
                result.sVersion="16.00";
                break;

            case 0x0c9:
            case 0x0ca:
            case 0x0cb:
            case 0x0cc:
            case 0x0cd:
                result.sVersion="11.00";
                break;

            case 0x0ce:
            case 0x0cf:
            case 0x0d0: // auto
            case 0x0d1: // auto
            case 0x0d2: // auto
            case 0x0d3: // auto
            case 0x0d4: // auto
            case 0x0d5: // auto
            case 0x0d6: // auto
                result.sVersion="17.00";
                break;

            case 0x0db:
            case 0x0dc:
            case 0x0dd:
            case 0x0de:
            case 0x0df:
                result.sVersion="12.00";
                break;

            case 0x0e0:
            case 0x0e1:
            case 0x0e2: // auto
            case 0x0e3: // auto
            case 0x0e4: // auto
            case 0x0e5: // auto
            case 0x0e6: // auto
            case 0x0e7: // auto
            case 0x0e8: // auto
                result.sVersion="18.00";
                break;

            case 0x0ff:
            case 0x100:
            case 0x101:
            case 0x102:
            case 0x103:
                result.sVersion="14.00";
                break;

            case 0x104:
            case 0x105:
            case 0x106:
            case 0x107:
            case 0x108:
            case 0x109:
            case 0x10a:
            case 0x10b:
            case 0x10c:
                result.sVersion="19.00";
                break;
        }

        if(result.type!=SpecAbstract::RECORD_TYPE_UNKNOWN)
        {
            result.sVersion+=QString(".%1").arg(nMinor);
        }
    }

    return result;
}

void SpecAbstract::_errorMessage(QString sMessage)
{
#ifdef QT_DEBUG
    qDebug("Error: %s",sMessage.toLatin1().data());
#endif
    emit errorMessage(sMessage);
}

void SpecAbstract::_infoMessage(QString sMessage)
{
#ifdef QT_DEBUG
    qDebug("Info: %s",sMessage.toLatin1().data());
#endif
    emit infoMessage(sMessage);
}

