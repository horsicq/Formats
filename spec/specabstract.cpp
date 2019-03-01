// copyright (c) 2017-2019 hors<horsicq@gmail.com>
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

SpecAbstract::SIGNATURE_RECORD _binary_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Install",              "'idska32'1A"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Install",              "'zlb'1A"}, // TODO none
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Uninstall",            "'Inno Setup Messages'"},  // TODO check
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_ARCHIVE,          SpecAbstract::RECORD_NAME_CAB,                          "",             "",                     "'MSCF'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_ARCHIVE,          SpecAbstract::RECORD_NAME_7Z,                           "",             "",                     "'7z'BCAF271C"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_CERTIFICATE,      SpecAbstract::RECORD_NAME_WINAUTH,                      "2.0",          "PKCS #7",              "........00020200"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_DEBUGDATA,        SpecAbstract::RECORD_NAME_MINGW,                        "",             "",                     "'.file'000000"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_ARCHIVE,          SpecAbstract::RECORD_NAME_ZIP,                          "",             "",                     "'PK'0304"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_FORMAT,           SpecAbstract::RECORD_NAME_PDF,                          "",             "",                     "'%PDF'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_FORMAT,           SpecAbstract::RECORD_NAME_PDB,                          "2.00",         "",                     "'Microsoft C/C++ program database 2.00\r\n'1A'JG'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_FORMAT,           SpecAbstract::RECORD_NAME_PDB,                          "7.00",         "",                     "'Microsoft C/C++ MSF 7.00\r\n'1A'DS'000000"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_ARCHIVE,          SpecAbstract::RECORD_NAME_GZIP,                         "",             "",                     "1F8B08"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_ARCHIVE,          SpecAbstract::RECORD_NAME_RAR,                          "",             "",                     "'Rar!'1A07"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_INSTALLANYWHERE,              "",             "",                     "5B3E"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_GHOSTINSTALLER,               "1.0",          "xored MSCF, mask: 8D", "C0DECECB8D8D8D8D"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_NSIS,                         "",             "",                     "EFBEADDE'Null'..'oftInst'"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_NSIS,                         "",             "",                     "EFBEADDE'nsisinstall'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_NSIS,                         "",             "Install",              "00000000EFBEADDE'NullsoftInst'"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_NSIS,                         "",             "Uninstall",            "01000000EFBEADDE'NullsoftInst'"},
    {3, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_NSIS,                         "",             "Install",              "02000000EFBEADDE'NullsoftInst'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_PROTECTORDATA,    SpecAbstract::RECORD_NAME_FISHNET,                      "1.X",          "",                     "0800'FISH_NET'0100"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_PROTECTORDATA,    SpecAbstract::RECORD_NAME_FISHNET,                      "1.X",          "",                     "000800'FISH_NET'0100"},
    {2, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_PROTECTORDATA,    SpecAbstract::RECORD_NAME_FISHNET,                      "1.X",          "",                     "00000800'FISH_NET'0100"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_SMARTINSTALLMAKER,            "",             "",                     "'Smart Install Maker v'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_TARMAINSTALLER,               "",             "zlib",                 "'tiz1'........78da'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_CLICKTEAM,                    "",             "",                     "'wwgT)'"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_CLICKTEAM,                    "",             "",                     "..120100....0000"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_SFXDATA,          SpecAbstract::RECORD_NAME_WINRAR,                       "",             "",                     "'***messages***'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_SFXDATA,          SpecAbstract::RECORD_NAME_SQUEEZSFX,                    "",             "",                     "'SQ5SFX'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_SFXDATA,          SpecAbstract::RECORD_NAME_7Z,                           "",             "",                     "';!@Install@!UTF-8!'"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_SFXDATA,          SpecAbstract::RECORD_NAME_7Z,                           "",             "",                     "EFBBBF';!@Install@!UTF-8!'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_QTINSTALLER,                  "",             "",                     "'qres'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_ADVANCEDINSTALLER,            "",             "",                     "D0CF11E0A1B11AE1"},
    {1, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_ADVANCEDINSTALLER,            "",             "",                     "2f30ee1f5e4ee51e"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_GPINSTALL,                    "",             "",                     "........'SPIS'1a'LH5'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_INSTALLERDATA,    SpecAbstract::RECORD_NAME_ACTUALINSTALLER,              "",             "",                     "....................'MSCF'00"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_FORMAT,           SpecAbstract::RECORD_NAME_MICROSOFTLINKERDATABASE,      "",             "",                     "'Microsoft Linker Database\n\n'071A"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_IMAGE,            SpecAbstract::RECORD_NAME_JPEG,                         "",             "",                     "FFD8FFE0....'JFIF'00"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_IMAGE,            SpecAbstract::RECORD_NAME_PNG,                          "",             "",                     "89'PNG\r\n'1A0A........'IHDR'"},
    {0, SpecAbstract::RECORD_FILETYPE_BINARY,   SpecAbstract::RECORD_TYPE_IMAGE,            SpecAbstract::RECORD_NAME_WINDOWSICON,                  "",             "",                     "00000100"},
};

SpecAbstract::SIGNATURE_RECORD _PE_header_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_TURBOLINKER,                  "",             "",                     "'MZ'50000200000004000F00FFFF0000B80000000000000040001A000000000000000000000000000000000000000000000000000000000000000000....0000BA10000E1FB409CD21B8014CCD219090'This program must be run under Win'....'r\n$'370000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_TURBOLINKER,                  "",             "patched",              "'MZ'............................................................................................................................BA10000E1FB409CD21B8014CCD219090'This program must be run under Win'....'r\n$'370000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_MICROSOFTLINKER,              "",             "",                     "'MZ'90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000....00000E1FBA0E00B409CD21B8014CCD21'This program cannot be run in DOS mode.\r\r\n$'00000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_GENERICLINKER,                "",             "",                     "'MZ'90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21'This program cannot be run in DOS mode.\r\r\n$'00000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_MICROSOFTLINKER,              "",             "patched",              "'MZ'90000300000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000....000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_FASM,                         "",             "",                     "'MZ'80000100000004001000FFFF00004001000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21'This program cannot be run in DOS mode.\r\n$'0000000000000000'PE'0000"}, // TODO patched
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_FORMAT,           SpecAbstract::RECORD_NAME_HXS,                          "",             "",                     "'MZ'0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000000504500004C010200000000000000000000000000E00001200B010000000000000000000000000000000000000000000000000000000040000000000000"},
    {1, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_NOSTUBLINKER,                 "",             "",                     "'MZ'90000300000004000000FFFF0000B8000000000000004000000000000000000000000000000000000000000000000000000000000000000000004000000050450000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "0.1X-0.24",    "",                     "'MZKERNEL32.DLL'0000'PE'0000........'UpackByDwing'"},
    {1, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "0.24-0.32",    "",                     "'MZKERNEL32.DLL'0000'LoadLibraryA'00000000'GetProcAddress'............................40000000"},
    {2, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "0.33",         "",                     "'MZLoadLibraryA'0000'PE'0000........'KERNEL32'"},
    {3, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "0.36",         "",                     "'MZLoadLibraryA'0000'PE'0000............................................'KERNEL32.DLL'"},
    {4, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "0.37-0.399",   "",                     "'MZKERNEL32.DLL'0000'PE'0000............................................'LoadLibraryA'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "0.71-0.97",    "Win32/exe",            "'MZ'........................................................................................'Is Win32 EXE.'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "0.71-0.97",    "Win64/exe",            "'MZ'........................................................................................'Is Win64 EXE.'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "0.71-0.97",    "Win32/dll",            "'MZ'........................................................................................'Is Win32 DLL.'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "0.71-0.97",    "Win64/dll",            "'MZ'........................................................................................'Is Win64 DLL.'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "1.27-2.12",    "Win32/exe",            "'MZ'........................................................................................'Win32 .EXE.\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "1.27-2.12",    "Win64/exe",            "'MZ'........................................................................................'Win64 .EXE.\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "1.27-2.12",    "Win32/dll",            "'MZ'........................................................................................'Win32 .DLL.\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "1.27-2.12",    "Win64/dll",            "'MZ'........................................................................................'Win64 .DLL.\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MPRESS,                       "1.27-2.12",    ".NET",                 "'MZ'........................................................................................'It'27's .NET EXE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_INSTALLER,        SpecAbstract::RECORD_NAME_INNOSETUP,                    "1.XX-5.1.X",   "Install",              "'MZ'............................................................................................496E6E6F"}, // TODO Versions
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_INSTALLER,        SpecAbstract::RECORD_NAME_INNOSETUP,                    "",             "Uninstall",            "'MZ'............................................................................................496E556E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_ANDPAKK2,                     "0.18",         "",                     "'MZ'00'ANDpakk2'00'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_GOLINK,                       "",             "",                     "'MZ'6c000100000002000000ffff000000000000110000004000000000000000'Win32 Program!\r\n$'b409ba0001cd21b44ccd2160000000'GoLink, GoAsm www.GoDevTool.com'00"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_GOLINK,                       "",             "",                     "'MZ'6c000100000002000000ffff000000000000110000004000000000000000'Win64 Program!\r\n$'b409ba0001cd21b44ccd2160000000'GoLink, GoAsm www.GoDevTool.com'00"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_NSPACK,                       "2.0-2.4",      "",                     "'MZ'40000100000002000000FFFF00000002000000000000400000....................CD21B44CCD21'packed by nspack$'40000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_LAYHEYFORTRAN90,              "",             "",                     "'MZ'....................................................................................................................6C030000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_HMIMYSPROTECTOR,              "0.1",          "",                     "'MZ'............................................................'hmimys'27's ProtectV0.1'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FSG,                          "1.00-1.20",    "",                     "'MZ'....................................................................................................................600000000E1FBA0E00B409CD21B8014CCD21'Windows Program'0D0A24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FSG,                          "1.30-1.32",    "",                     "'MZ'....................................................................................................................40000000'PE'00004C01....'FSG!'"},
    {1, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FSG,                          "1.33-2.00",    "",                     "'MZ'....................'PE'00004C01....'FSG!'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_MEW11SE,                      "1.1-1.2",      "",                     "'MZ'00000000000000000000'PE'00004C010200000000000000000000000000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",         "",                     "'MZ'52C3'(C)BeRo!PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_KKRUNCHY,                     "0.23",         "farbraush",            "'MZfarbrauschPE'"},
    {1, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_KKRUNCHY,                     "",             "conspiracy",           "'MZconspiracyPE'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_QUICKPACKNT,                  "0.1",          "",                     "'MZ'90EB010052E9........'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_GENERIC,                      "",             "",                     "'MZ'....................'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_YZPACK,                       "1.1",          "",                     "'MZ'40000100000002000400FFFF0200400000000E0000001C00000000000000'(c) UsAr 2oo6$'0EB409BA00001FCD21B8014CCD2140000000'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_YZPACK,                       "1.2",          "",                     "'MZ'52456083EC188BEC8BFC33C0648B4030780C8B400C8B701CAD8B4008EB098B403483C07C8B403CABE9........B409BA00001FCD21B8014CCD2140000000'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_YZPACK,                       "2.0",          "",                     "'MZKERNEL32'0000'PE'0000"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_WATCOMLINKER,                 "",             "WinNT/dll",            "'MZ'80000100000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21'this is a Windows NT dynamic link library\r\n'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_WATCOMLINKER,                 "",             "WinNT/RTL/dll",        "'MZ'80000100000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000900000000E1FBA0E00B409CD21B8014CCD21'this is a Windows NT (own RTL) dynamic link library\r\n'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_WATCOMLINKER,                 "",             "WinNT/RTLexe",         "'MZ'80000100000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000900000000E1FBA0E00B409CD21B8014CCD21'this is a Windows NT character-mode (own RTL) executable\r\n'24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_LINKER,           SpecAbstract::RECORD_NAME_WATCOMLINKER,                 "",             "WinNT/exe",            "'MZ'80000100000004000000FFFF0000B800000000000000400000000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD21'this is a Windows NT character-mode executable\r\n'24"},
};

SpecAbstract::SIGNATURE_RECORD _PE_entrypoint_records[]=
{
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.59",             "exe",                  "60E8000000005883E83D50"}, // mb TODO
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.60-0.69",        "exe",                  "60E8........68........8BE88DBD........33DB033C248BF7"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.71-0.72",        "exe",                  "60E80000000083CDFF31DB5E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.71-0.72",        "dll",                  "807C2408010F85........60E80000000083CDFF31DB5E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.81-3.81+",       "exe",                  "60BE........8DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "0.81-3.81+",       "dll",                  "807C2408010F85........60BE........8DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "3.81+",            "exe",                  "53565755488D35........488DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_UPX,                          "3.81+",            "dll",                  "48894C240848895424104C8944241880FA010F85........53565755488D35........488DBE"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WWPACK32,                     "1.01-1.12",        "",                     "53558be833dbeb60'\r\n\r\nWWPack32 decompression routine version '........'\r\n(c) 1998 Piotr Warezak and Rafal Wierzbicki\r\n\r\n'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_BORLANDCPP,                   "",                 "",                     "EB10'fb:C++HOOK'90"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_ANDPAKK2,                     "0.06",             "",                     "60FCBE........BF........5783CDFF33C9F9EB05A402DB7505"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_ANDPAKK2,                     "0.18",             "",                     "FCBE........BF........5783CDFF33C9F9EB05A402DB7505"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_ASDPACK,                      "2.0",              "",                     "8B442404565753E8CD010000C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PEX,                          "0.99",             "",                     "E9"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_REVPROT,                      "0.1a",             "",                     "E8........8B4C240CC701........C781................31C089411489411880A1..........C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_32LITE,                       "0.03a",            "",                     "6006FC1E07BE........6A0468........68"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ACPROTECT,                    "2.0.X",            "",                     "68........68........C3C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ALEXPROTECTOR,                "1.0",              "",                     "60E8000000005D81ED........E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ALLOY,                        "4.X",              "",                     "9C60E8........33C08BC483C0..938BE3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_GCC,                          "3.X-4.X",          "MinGW",                "5589E583EC08C70424..000000FF15........E8....FFFF................55"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PECOMPACT,                    "0.X-1.X",          "",                     "EB0668........C39C60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_NSPACK,                       "2.9-3.7",          "",                     "9C60E8000000005D"},
    {0, SpecAbstract::RECORD_FILETYPE_PE64,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_NSPACK,                       "2.9-3.7",          "",                     "4881ECC00000004883C4404889042448894C2408488954241048895C241848896C2420488974242848897C24304C894424384C894C24404C895424484C895C24504C896424584C896C24604C897424684C897C24704883EC40E8000000005D"}, // TODO version
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ENIGMA,                       "1.2",              "",                     "60E8000000005D83....81ED"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_WATCOMCCPP,                   "1995",             "",                     "..................'WATCOM C/C++32 Run-Time system. (c) Copyright by WATCOM International Corp. 1988-1995. '"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ORIEN,                        "",                 "",                     "E95D010000CED1CE..'\r\n--------------------------------------------\r\n- ORiEN executable files protection system'"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_VPACKER,                      "0.02.10",          "",                     "60E8........C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_ASPROTECT,                    "1.23-2.77",        "",                     "6801......E801000000C3C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_EXEPACK,                      "1.4",              "",                     "33C08BC068........68........E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_EXEPACK,                      "1.4",              "",                     "EB01909068........68........E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PROTECTOR,        SpecAbstract::RECORD_NAME_PESPIN,                       "",                 "",                     "EB016860E8000000008B1C2483C312812BE8B10600FE4BFD822C24"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_WINUPACK,                     "",                 "Alt stub",             "60E809000000..................33C95E870E"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_AHPACKER,                     "0.1",              "",                     "6068........B8........FF10"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZBRR",                "60BE........BF........FCB28033DBA4B302E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZBRS",                "60BE........BF........FCAD8D1C07B0803BFB733BE8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZMA",                 "6068........68........68........E8........BE........B9"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZBRR/dll",            "837C2408010F85........60BE........BF........FCB28033DBA4B302E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZBRS/dll",            "837C2408010F85........60BE........BF........FCAD8D1C07B0803BFB733BE8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_BEROEXEPACKER,                "1.00",             "LZMA/dll",             "837C2408010F85........6068........68........68........E8........BE........B9"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_NPACK,                        "",                 "",                     "833D........007505E9........C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FISHPEPACKER,                 "1.02",             "",                     "60E8........6168........C3"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FISHPEPACKER,                 "1.03",             "",                     "60E8........EB"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_FISHPEPACKER,                 "1.04",             "",                     "60B8........FFD0"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_KKRUNCHY,                     "0.23 alpha 1",     "",                     "BD........C74500........FF4D08C6450C05"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_KKRUNCHY,                     "0.23 alpha 2",     "",                     "BD........C74500........B8........89450489455450C74510"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_KKRUNCHY,                     "0.23 alpha 3-4",   "",                     "BD........C74500........B8........89450489455850C74510"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PACKMAN,                      "0.0.0.1",          "",                     "60E800000000588D..........8D..........8D"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PACKMAN,                      "1.0",              "",                     "60E8000000005B8D5BC6011B8B138D73146A08590116AD4975FA"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PETITE,                       "2.4",              "",                     "B8........60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PETITE,                       "2.2-2.3",          "",                     "B8........6A0068........64FF350000000064892500000000669C60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PETITE,                       "2.3",              "",                     "B8........68........64FF350000000064892500000000669C60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PETITE,                       "1.3-1.4",          "",                     "B8........669C60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PETITE,                       "1.2",              "",                     "669C60"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PEPACK,                       "1.0",              "",                     "7400e9"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_PKLITE32,                     "1.1",              "",                     "68........68........68........E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_XCOMP,                        "0.97-0.98",        "",                     "68........9C60E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_XPACK,                        "0.97-0.98",        "",                     "68........9C60E8"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_ABCCRYPTOR,                   "1.0",              "",                     "68FF6424F0685858585890FFD4"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_EXE32PACK,                    "1.4X",             "",                     "3BC07402"},
    {0, SpecAbstract::RECORD_FILETYPE_PE32,     SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_GENERIC,                      "",                 "",                     "60"},
    // WATCOM C/C++32 Run-Time system. (c) Copyright by WATCOM International Corp. 1988-1995.
    // WATCOM C/C++32 Run-Time system. (c) Copyright by WATCOM International Corp. 1988-1994. All rights re..
};

SpecAbstract::STRING_RECORD _PE_dot_ansistrings_records[]={
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_DOTFUSCATOR,                  "",             "",                     "DotfuscatorAttribute"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_LIBRARY,          SpecAbstract::RECORD_NAME_VCL,                          "",             ".NET",                 "Borland.Vcl.Types"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_COMPILER,         SpecAbstract::RECORD_NAME_VBNET,                        "",             "",                     "Microsoft.VisualBasic"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,      SpecAbstract::RECORD_TYPE_TOOL,              SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET,  "",             "",                     "Embarcadero."},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_YANO,                         "1.X",          "",                     "YanoAttribute"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_AGILENET,                     "",             "",                     "ObfuscatedByAgileDotNetAttribute"},
//    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_SKATERNET,                    "",             "",                     "Skater_NET_Obfuscator"},
//    {1, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_SKATERNET,                    "",             "",                     "RustemSoft.Skater"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_BABELNET,                     "3.X",          "",                     "BabelAttribute"}, // TODO Version
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_BABELNET,                     "1.X-2.X",      "",                     "BabelObfuscatorAttribute"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_CLISECURE,                    "4.X-5.X",      "",                     "ObfuscatedByCliSecureAttribute"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_CLISECURE,                    "3.X",          "",                     "CliSecureRd.dll"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_CLISECURE,                    "3.X",          "",                     "CliSecureRd64.dll"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_TOOL,             SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET,      "XE*",          "",                     "Borland.Studio.Delphi"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_TOOL,             SpecAbstract::RECORD_NAME_EMBARCADERODELPHIDOTNET,      "8",            "",                     "Borland.Vcl.Types"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_CRYPTOOBFUSCATORFORNET,       "",             "",                     "CryptoObfuscator"}, // TODO Version, die
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_DEEPSEA,                      "4.X",          "",                     "DeepSeaObfuscator"}, // TODO Version, die
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_GOLIATHNET,                   "",             "",                     "ObfuscatedByGoliath"}, // TODO Version, die
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_MACROBJECT,                   "",             "",                     "Obfuscated by Macrobject Obfuscator.NET"}, // TODO Version
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_SOFTWAREZATOR,                "",             "",                     "ObfuscatedBySoftwareZatorAttribute"}, // TODO Version
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_NSPACK,                       "2.X-3.X",      ".NET",                 "nsnet"}, // TODO Version
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_DNGUARD,                      "",             "",                     "ZYXDNGuarder"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_DNGUARD,                      "",             "",                     "HVMRuntm.dll"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_DOTNETZ,                      "",             "",                     "NetzStarter"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_MAXTOCODE,                    "",             "",                     "InfaceMaxtoCode"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_PHOENIXPROTECTOR,             "",             "",                     "?1?.?9?.resources"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_PACKER,           SpecAbstract::RECORD_NAME_SIXXPACK,                     "",             "",                     "Sixxpack"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_SMARTASSEMBLY,                "",             "",                     "SmartAssembly.Attributes"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_CONFUSER,                     "1.X",          "",                     "ConfusedByAttribute"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_SPICESNET,                    "",             "",                     "NineRays.Obfuscator"},
    {0, SpecAbstract::RECORD_FILETYPE_PE,       SpecAbstract::RECORD_TYPE_NETOBFUSCATOR,    SpecAbstract::RECORD_NAME_OBFUSCATORNET2009,            "",             "",                     "Macrobject.Obfuscator"},
};

//// TODO
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
        case RECORD_FILETYPE_UNKNOWN:                       sResult=QString("Unknown");                     break;
        case RECORD_FILETYPE_BINARY:                        sResult=QString("Binary");                      break;
        case RECORD_FILETYPE_MSDOS:                         sResult=QString("MSDOS");                       break;
        case RECORD_FILETYPE_PE:                            sResult=QString("PE");                          break;
        case RECORD_FILETYPE_PE32:                          sResult=QString("PE32");                        break;
        case RECORD_FILETYPE_PE64:                          sResult=QString("PE64");                        break;
        case RECORD_FILETYPE_ELF:                           sResult=QString("ELF");                         break;
        case RECORD_FILETYPE_ELF32:                         sResult=QString("ELF32");                       break;
        case RECORD_FILETYPE_ELF64:                         sResult=QString("ELF64");                       break;
        case RECORD_FILETYPE_MACH:                          sResult=QString("MACH");                        break;
        case RECORD_FILETYPE_MACH32:                        sResult=QString("MACH32");                      break;
        case RECORD_FILETYPE_MACH64:                        sResult=QString("MACH64");                      break;
        case RECORD_FILETYPE_TEXT:                          sResult=QString("Text");                        break;
    }

    return sResult;
}

QString SpecAbstract::recordFilepartIdToString(SpecAbstract::RECORD_FILEPARTS id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_FILEPART_UNKNOWN:                       sResult=QString("Unknown");                     break;
        case RECORD_FILEPART_HEADER:                        sResult=QString("Header");                      break;
        case RECORD_FILEPART_OVERLAY:                       sResult=QString("Overlay");                     break;
    }

    return sResult;
}

QString SpecAbstract::recordTypeIdToString(RECORD_TYPES id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_TYPE_UNKNOWN:                           sResult=tr("Unknown");                          break;
        case RECORD_TYPE_ARCHIVE:                           sResult=tr("Archive");                          break;
        case RECORD_TYPE_CERTIFICATE:                       sResult=tr("Certificate");                      break;
        case RECORD_TYPE_COMPILER:                          sResult=tr("Compiler");                         break;
        case RECORD_TYPE_CONVERTER:                         sResult=tr("Converter");                        break;
        case RECORD_TYPE_DEBUGDATA:                         sResult=tr("Debug data");                       break;
        case RECORD_TYPE_FORMAT:                            sResult=tr("Format");                           break;
        case RECORD_TYPE_IMAGE:                             sResult=tr("Image");                            break;
        case RECORD_TYPE_INSTALLER:                         sResult=tr("Installer");                        break;
        case RECORD_TYPE_INSTALLERDATA:                     sResult=tr("Installer data");                   break;
        case RECORD_TYPE_LIBRARY:                           sResult=tr("Library");                          break;
        case RECORD_TYPE_LINKER:                            sResult=tr("Linker");                           break;
        case RECORD_TYPE_NETOBFUSCATOR:                     sResult=tr(".NET Obfuscator");                  break;
        case RECORD_TYPE_PACKER:                            sResult=tr("Packer");                           break;
        case RECORD_TYPE_PROTECTOR:                         sResult=tr("Protector");                        break;
        case RECORD_TYPE_PROTECTORDATA:                     sResult=tr("Protector data");                   break;
        case RECORD_TYPE_SFX:                               sResult=tr("SFX");                              break;
        case RECORD_TYPE_SFXDATA:                           sResult=tr("SFX data");                         break;
        case RECORD_TYPE_TOOL:                              sResult=tr("Tool");                             break;
    }

    return sResult;
}

QString SpecAbstract::recordNameIdToString(RECORD_NAMES id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        case RECORD_NAME_UNKNOWN:                           sResult=QString("Unknown");                                     break;
        case RECORD_NAME_32LITE:                            sResult=QString("32Lite");                                      break;
        case RECORD_NAME_7Z:                                sResult=QString("7-Zip");                                       break;
        case RECORD_NAME_ABCCRYPTOR:                        sResult=QString("ABC Cryptor");                                 break;
        case RECORD_NAME_ACPROTECT:                         sResult=QString("ACProtect");                                   break;
        case RECORD_NAME_ACTUALINSTALLER:                   sResult=QString("Actual Installer");                            break;
        case RECORD_NAME_ADVANCEDINSTALLER:                 sResult=QString("Advanced Installer");                          break;
        case RECORD_NAME_AGILENET:                          sResult=QString("Agile .NET");                                  break;
        case RECORD_NAME_AHPACKER:                          sResult=QString("AHPacker");                                    break;
        case RECORD_NAME_AHTEAMEPPROTECTOR:                 sResult=QString("AHTeam EP Protector");                         break;
        case RECORD_NAME_ALEXPROTECTOR:                     sResult=QString("Alex Protector");                              break;
        case RECORD_NAME_ALLOY:                             sResult=QString("Alloy");                                       break;
        case RECORD_NAME_ANDPAKK2:                          sResult=QString("ANDpakk2");                                    break;
        case RECORD_NAME_ANTIDOTE:                          sResult=QString("AntiDote");                                    break;
        case RECORD_NAME_ARMADILLO:                         sResult=QString("Armadillo");                                   break;
        case RECORD_NAME_ARMPROTECTOR:                      sResult=QString("ARM Protector");                               break;
        case RECORD_NAME_ASDPACK:                           sResult=QString("ASDPack");                                     break;
        case RECORD_NAME_ASM:                               sResult=QString("Asm");                                         break;
        case RECORD_NAME_ASPACK:                            sResult=QString("ASPack");                                      break;
        case RECORD_NAME_ASPROTECT:                         sResult=QString("ASProtect");                                   break;
        case RECORD_NAME_ASSEMBLYINVOKE:                    sResult=QString("AssemblyInvoke");                              break;
        case RECORD_NAME_AVERCRYPTOR:                       sResult=QString("AverCryptor");                                 break;
        case RECORD_NAME_BABELNET:                          sResult=QString("Babel .NET");                                  break;
        case RECORD_NAME_BEROEXEPACKER:                     sResult=QString("BeRoEXEPacker");                               break;
        case RECORD_NAME_BITROCKINSTALLER:                  sResult=QString("BitRock Installer");                           break;
        case RECORD_NAME_BITSHAPEPECRYPT:                   sResult=QString("BitShape PE Crypt");                           break;
        case RECORD_NAME_BORLANDCPP:                        sResult=QString("Borland C++");                                 break;
        case RECORD_NAME_BORLANDCPPBUILDER:                 sResult=QString("Borland C++ Builder");                         break;
        case RECORD_NAME_BORLANDDELPHI:                     sResult=QString("Borland Delphi");                              break;
        case RECORD_NAME_BORLANDDELPHIDOTNET:               sResult=QString("Borland Delphi .NET");                         break;
        case RECORD_NAME_BORLANDOBJECTPASCAL:               sResult=QString("Borland Object Pascal");                       break;
        case RECORD_NAME_BREAKINTOPATTERN:                  sResult=QString("Break Into Pattern");                          break;
        case RECORD_NAME_C:                                 sResult=QString("C");                                           break;
        case RECORD_NAME_CAB:                               sResult=QString("CAB");                                         break;
        case RECORD_NAME_CEXE:                              sResult=QString("CExe");                                        break;
        case RECORD_NAME_CIL:                               sResult=QString("cil");                                         break;
        case RECORD_NAME_CLICKTEAM:                         sResult=QString("ClickTeam");                                   break;
        case RECORD_NAME_CLISECURE:                         sResult=QString("CliSecure");                                   break;
        case RECORD_NAME_CODEGEARCPP:                       sResult=QString("CodeGear C++");                                break;
        case RECORD_NAME_CODEGEARCPPBUILDER:                sResult=QString("CodeGear C++ Builder");                        break;
        case RECORD_NAME_CODEGEARDELPHI:                    sResult=QString("CodeGear Delphi");                             break;
        case RECORD_NAME_CODEGEAROBJECTPASCAL:              sResult=QString("Codegear Object Pascal");                      break;
        case RECORD_NAME_CODEVEIL:                          sResult=QString("CodeVeil");                                    break;
        case RECORD_NAME_CODEWALL:                          sResult=QString("CodeWall");                                    break;
        case RECORD_NAME_CONFUSER:                          sResult=QString("Confuser");                                    break;
        case RECORD_NAME_CONFUSEREX:                        sResult=QString("ConfuserEx");                                  break;
        case RECORD_NAME_CPP:                               sResult=QString("C++");                                         break;
        case RECORD_NAME_CREATEINSTALL:                     sResult=QString("CreateInstall");                               break;
        case RECORD_NAME_CRINKLER:                          sResult=QString("Crinkler");                                    break;
        case RECORD_NAME_CRUNCH:                            sResult=QString("Crunch");                                      break;
        case RECORD_NAME_CRYPTER:                           sResult=QString("Crypter");                                     break;
        case RECORD_NAME_CRYPTOCRACKSPEPROTECTOR:           sResult=QString("CRYPToCRACks PE Protector");                   break;
        case RECORD_NAME_CRYPTOOBFUSCATORFORNET:            sResult=QString("Crypto Obfuscator For .Net");                  break;
        case RECORD_NAME_CYGWIN:                            sResult=QString("Cygwin");                                      break;
        case RECORD_NAME_DEB:                               sResult=QString("DEB");                                         break;
        case RECORD_NAME_DEEPSEA:                           sResult=QString("DeepSea");                                     break;
        case RECORD_NAME_DNGUARD:                           sResult=QString("DNGuard");                                     break;
        case RECORD_NAME_DOTFIXNICEPROTECT:                 sResult=QString("DotFix Nice Protect");                         break;
        case RECORD_NAME_DOTFUSCATOR:                       sResult=QString("Dotfuscator");                                 break;
        case RECORD_NAME_DOTNET:                            sResult=QString(".NET");                                        break;
        case RECORD_NAME_DOTNETZ:                           sResult=QString(".NETZ");                                       break;
        case RECORD_NAME_DVCLAL:                            sResult=QString("DVCLAL");                                      break;
        case RECORD_NAME_DYAMAR:                            sResult=QString("DYAMAR");                                      break;
        case RECORD_NAME_EAZFUSCATOR:                       sResult=QString("Eazfuscator");                                 break;
        case RECORD_NAME_EMBARCADEROCPP:                    sResult=QString("Embarcadero C++");                             break;
        case RECORD_NAME_EMBARCADEROCPPBUILDER:             sResult=QString("Embarcadero C++ Builder");                     break;
        case RECORD_NAME_EMBARCADERODELPHI:                 sResult=QString("Embarcadero Delphi");                          break;
        case RECORD_NAME_EMBARCADERODELPHIDOTNET:           sResult=QString("Embarcadero Delphi .NET");                     break;
        case RECORD_NAME_EMBARCADEROOBJECTPASCAL:           sResult=QString("Embarcadero Object Pascal");                   break;
        case RECORD_NAME_EMPTYFILE:                         sResult=QString("Empty File");                                  break;
        case RECORD_NAME_ENIGMA:                            sResult=QString("ENIGMA");                                      break;
        case RECORD_NAME_EXE32PACK:                         sResult=QString("exe32pack");                                   break;
        case RECORD_NAME_EXECRYPT:                          sResult=QString("EXECrypt");                                    break;
        case RECORD_NAME_EXECRYPTOR:                        sResult=QString("EXECryptor");                                  break;
        case RECORD_NAME_EXEFOG:                            sResult=QString("ExeFog");                                      break;
        case RECORD_NAME_EXEMPLARINSTALLER:                 sResult=QString("Exemplar Installer");                          break;
        case RECORD_NAME_EXEPACK:                           sResult=QString("!EP(EXE Pack)");                               break;
        case RECORD_NAME_EXESAX:                            sResult=QString("ExeSax");                                      break;
        case RECORD_NAME_EXESHIELD:                         sResult=QString("Exe Shield");                                  break;
        case RECORD_NAME_EXPORT:                            sResult=QString("Export");                                      break;
        case RECORD_NAME_FASM:                              sResult=QString("FASM");                                        break;
        case RECORD_NAME_FISHNET:                           sResult=QString("FISH .NET");                                   break;
        case RECORD_NAME_FISHPEPACKER:                      sResult=QString("Fish PE Packer");                              break;
        case RECORD_NAME_FLEXLM:                            sResult=QString("Flex License Manager");                        break;
        case RECORD_NAME_FLEXNET:                           sResult=QString("FlexNet Licensing");                           break;
        case RECORD_NAME_FPC:                               sResult=QString("Free Pascal");                                 break;
        case RECORD_NAME_FREECRYPTOR:                       sResult=QString("FreeCryptor");                                 break;
        case RECORD_NAME_FSG:                               sResult=QString("FSG");                                         break;
        case RECORD_NAME_GCC:                               sResult=QString("GCC");                                         break;
        case RECORD_NAME_GENERIC:                           sResult=QString("Generic");                                     break;
        case RECORD_NAME_GENERICLINKER:                     sResult=QString("Generic Linker");                              break;
        case RECORD_NAME_GENTEEINSTALLER:                   sResult=QString("Gentee Installer");                            break;
        case RECORD_NAME_GHOSTINSTALLER:                    sResult=QString("Ghost Installer");                             break;
        case RECORD_NAME_GNULINKER:                         sResult=QString("GNU ld");                                      break;
        case RECORD_NAME_GOASM:                             sResult=QString("GoAsm");                                       break;
        case RECORD_NAME_GOLIATHNET:                        sResult=QString("Goliath .NET");                                break;
        case RECORD_NAME_GOLINK:                            sResult=QString("GoLink");                                      break;
        case RECORD_NAME_GPINSTALL:                         sResult=QString("GP-Install");                                  break;
        case RECORD_NAME_GZIP:                              sResult=QString("GZIP");                                        break;
        case RECORD_NAME_HIDEPE:                            sResult=QString("HidePE");                                      break;
        case RECORD_NAME_HMIMYSPACKER:                      sResult=QString("Hmimys Packer");                               break;
        case RECORD_NAME_HMIMYSPROTECTOR:                   sResult=QString("Hmimys's Protector");                          break;
        case RECORD_NAME_HXS:                               sResult=QString("HXS");                                         break;
        case RECORD_NAME_IMPORT:                            sResult=QString("Import");                                      break;
        case RECORD_NAME_INNOSETUP:                         sResult=QString("Inno Setup");                                  break;
        case RECORD_NAME_INSTALLANYWHERE:                   sResult=QString("InstallAnywhere");                             break;
        case RECORD_NAME_INSTALLSHIELD:                     sResult=QString("InstallShield");                               break;
        case RECORD_NAME_IPBPROTECT:                        sResult=QString("iPB Protect");                                 break;
        case RECORD_NAME_JPEG:                              sResult=QString("JPEG");                                        break;
        case RECORD_NAME_KKRUNCHY:                          sResult=QString("kkrunchy");                                    break;
        case RECORD_NAME_LAYHEYFORTRAN90:                   sResult=QString("Lahey Fortran 90");                            break;
        case RECORD_NAME_LAZARUS:                           sResult=QString("Lazarus");                                     break;
        case RECORD_NAME_MACROBJECT:                        sResult=QString("Macrobject");                                  break;
        case RECORD_NAME_MASKPE:                            sResult=QString("MaskPE");                                      break;
        case RECORD_NAME_MASM:                              sResult=QString("MASM");                                        break;
        case RECORD_NAME_MASM32:                            sResult=QString("MASM32");                                      break;
        case RECORD_NAME_MAXTOCODE:                         sResult=QString("MaxtoCode");                                   break;
        case RECORD_NAME_MEW11SE:                           sResult=QString("MEW11 SE");                                    break;
        case RECORD_NAME_MFC:                               sResult=QString("MFC");                                         break;
        case RECORD_NAME_MICROSOFTC:                        sResult=QString("Microsoft C");                                 break;
        case RECORD_NAME_MICROSOFTCPP:                      sResult=QString("Microsoft C++");                               break;
        case RECORD_NAME_MICROSOFTLINKER:                   sResult=QString("Microsoft linker");                            break;
        case RECORD_NAME_MICROSOFTLINKERDATABASE:           sResult=QString("Microsoft Linker Database");                   break;
        case RECORD_NAME_MICROSOFTVISUALSTUDIO:             sResult=QString("Microsoft Visual Studio");                     break;
        case RECORD_NAME_MINGW:                             sResult=QString("MinGW");                                       break;
        case RECORD_NAME_MKFPACK:                           sResult=QString("MKFPack");                                     break;
        case RECORD_NAME_MOLEBOX:                           sResult=QString("MoleBox");                                     break;
        case RECORD_NAME_MORPHNAH:                          sResult=QString("Morphnah");                                    break;
        case RECORD_NAME_MPRESS:                            sResult=QString("MPRESS");                                      break;
        case RECORD_NAME_MSYS:                              sResult=QString("Msys");                                        break;
        case RECORD_NAME_MSYS2:                             sResult=QString("MSYS2");                                       break;
        case RECORD_NAME_MZ0OPE:                            sResult=QString("MZ0oPE");                                      break;
        case RECORD_NAME_NOOBYPROTECT:                      sResult=QString("NoobyProtect");                                break;
        case RECORD_NAME_NORTHSTARPESHRINKER:               sResult=QString("North Star PE Shrinker");                      break;
        case RECORD_NAME_NOSTUBLINKER:                      sResult=QString("NOSTUBLINKER");                                break;
        case RECORD_NAME_NPACK:                             sResult=QString("nPack");                                       break;
        case RECORD_NAME_NSIS:                              sResult=QString("Nullsoft Scriptable Install System");          break;
        case RECORD_NAME_NSPACK:                            sResult=QString("NsPack");                                      break;
        case RECORD_NAME_OBFUSCAR:                          sResult=QString("Obfuscar");                                    break;
        case RECORD_NAME_OBFUSCATORNET2009:                 sResult=QString("Obfuscator.NET 2009");                         break;
        case RECORD_NAME_OBJECTPASCAL:                      sResult=QString("Object Pascal");                               break;
        case RECORD_NAME_OBSIDIUM:                          sResult=QString("Obsidium");                                    break;
        case RECORD_NAME_ORIEN:                             sResult=QString("ORiEN");                                       break;
        case RECORD_NAME_PACKMAN:                           sResult=QString("Packman");                                     break;
        case RECORD_NAME_PCGUARD:                           sResult=QString("PC Guard");                                    break;
        case RECORD_NAME_PDB:                               sResult=QString("PDB");                                         break;
        case RECORD_NAME_PDF:                               sResult=QString("PDF");                                         break;
        case RECORD_NAME_PEARMOR:                           sResult=QString("PE-Armor");                                    break;
        case RECORD_NAME_PEBUNDLE:                          sResult=QString("PEBundle");                                    break;
        case RECORD_NAME_PECOMPACT:                         sResult=QString("PECompact");                                   break;
        case RECORD_NAME_PEENCRYPT:                         sResult=QString("PE Encrypt");                                  break;
        case RECORD_NAME_PELOCK:                            sResult=QString("PELock");                                      break;
        case RECORD_NAME_PEPACK:                            sResult=QString("PE-PACK");                                     break;
        case RECORD_NAME_PEQUAKE:                           sResult=QString("PE Quake");                                    break;
        case RECORD_NAME_PESPIN:                            sResult=QString("PESpin");                                      break;
        case RECORD_NAME_PETITE:                            sResult=QString("Petite");                                      break;
        case RECORD_NAME_PEX:                               sResult=QString("PeX");                                         break;
        case RECORD_NAME_PHOENIXPROTECTOR:                  sResult=QString("Phoenix Protector");                           break;
        case RECORD_NAME_PKLITE32:                          sResult=QString("PKLITE32");                                    break;
        case RECORD_NAME_PLAIN:                             sResult=QString("Plain");                                       break;
        case RECORD_NAME_PNG:                               sResult=QString("PNG");                                         break;
        case RECORD_NAME_POLYCRYPTPE:                       sResult=QString("PolyCrypt PE");                                break;
        case RECORD_NAME_POWERBASIC:                        sResult=QString("PowerBASIC");                                  break;
        case RECORD_NAME_PRIVATEEXEPROTECTOR:               sResult=QString("Private EXE Protector");                       break;
        case RECORD_NAME_PYTHON:                            sResult=QString("Python");                                      break;
        case RECORD_NAME_QT:                                sResult=QString("Qt");                                          break;
        case RECORD_NAME_QTINSTALLER:                       sResult=QString("Qt Installer");                                break;
        case RECORD_NAME_QUICKPACKNT:                       sResult=QString("QuickPack NT");                                break;
        case RECORD_NAME_RAR:                               sResult=QString("RAR");                                         break;
        case RECORD_NAME_RCRYPTOR:                          sResult=QString("RCryptor");                                    break;
        case RECORD_NAME_RENETPACK:                         sResult=QString("ReNET-pack");                                  break;
        case RECORD_NAME_RESOURCE:                          sResult=QString("Resource");                                    break;
        case RECORD_NAME_REVPROT:                           sResult=QString("REVProt");                                     break;
        case RECORD_NAME_SETUPFACTORY:                      sResult=QString("Setup Factory");                               break;
        case RECORD_NAME_SIMBIOZ:                           sResult=QString("SimbiOZ");                                     break;
        case RECORD_NAME_SIXXPACK:                          sResult=QString("Sixxpack");                                    break;
        case RECORD_NAME_SKATER:                            sResult=QString("Skater");                                      break;
        case RECORD_NAME_SMARTASSEMBLY:                     sResult=QString("Smart Assembly");                              break;
        case RECORD_NAME_SMARTINSTALLMAKER:                 sResult=QString("Smart Install Maker");                         break;
        case RECORD_NAME_SOFTWAREZATOR:                     sResult=QString("SoftwareZator");                               break;
        case RECORD_NAME_SPICESNET:                         sResult=QString("Spices.Net");                                  break;
        case RECORD_NAME_SQUEEZSFX:                         sResult=QString("Squeez Self Extractor");                       break;
        case RECORD_NAME_STARFORCE:                         sResult=QString("StarForce");                                   break;
        case RECORD_NAME_SVKPROTECTOR:                      sResult=QString("SVK Protector");                               break;
        case RECORD_NAME_TARMAINSTALLER:                    sResult=QString("Tarma Installer");                             break;
        case RECORD_NAME_TELOCK:                            sResult=QString("tElock");                                      break;
        case RECORD_NAME_THEMIDAWINLICENSE:                 sResult=QString("Themida/Winlicense");                          break;
        case RECORD_NAME_TURBOLINKER:                       sResult=QString("Turbo linker");                                break;
        case RECORD_NAME_UNICODE:                           sResult=QString("Unicode");                                     break;
        case RECORD_NAME_UNKNOWNUPXLIKE:                    sResult=QString("Unknown UPX-like");                            break;
        case RECORD_NAME_UNOPIX:                            sResult=QString("Unopix");                                      break;
        case RECORD_NAME_UPX:                               sResult=QString("UPX");                                         break;
        case RECORD_NAME_UTF8:                              sResult=QString("UTF-8");                                       break;
        case RECORD_NAME_VBNET:                             sResult=QString("VB .NET");                                     break;
        case RECORD_NAME_VCL:                               sResult=QString("Visual Component Library");                    break;
        case RECORD_NAME_VCLPACKAGEINFO:                    sResult=QString("VCL PackageInfo");                             break;
        case RECORD_NAME_VIRTUALPASCAL:                     sResult=QString("Virtual Pascal");                              break;
        case RECORD_NAME_VISE:                              sResult=QString("Vise");                                        break;
        case RECORD_NAME_VISUALBASIC:                       sResult=QString("Visual Basic");                                break;
        case RECORD_NAME_VISUALCCPP:                        sResult=QString("Visual C/C++");                                break;
        case RECORD_NAME_VMPROTECT:                         sResult=QString("VMProtect");                                   break;
        case RECORD_NAME_VPACKER:                           sResult=QString("VPacker");                                     break;
        case RECORD_NAME_WATCOMC:                           sResult=QString("Watcom C");                                    break;
        case RECORD_NAME_WATCOMCCPP:                        sResult=QString("Watcom C/C++");                                break;
        case RECORD_NAME_WATCOMLINKER:                      sResult=QString("Watcom linker");                               break;
        case RECORD_NAME_WINACE:                            sResult=QString("WinACE");                                      break;
        case RECORD_NAME_WINAUTH:                           sResult=QString("Windows Authenticode");                        break;
        case RECORD_NAME_WINDOWSICON:                       sResult=QString("Windows Icon");                                break;
        case RECORD_NAME_WINDOWSINSTALLER:                  sResult=QString("Windows Installer");                           break;
        case RECORD_NAME_WINRAR:                            sResult=QString("WinRAR");                                      break;
        case RECORD_NAME_WINUPACK:                          sResult=QString("(Win)Upack");                                  break;
        case RECORD_NAME_WIXTOOLSET:                        sResult=QString("WiX Toolset");                                 break;
        case RECORD_NAME_WWPACK32:                          sResult=QString("WWPack32");                                    break;
        case RECORD_NAME_WXWIDGETS:                         sResult=QString("wxWidgets");                                   break;
        case RECORD_NAME_XENOCODEPOSTBUILD:                 sResult=QString("Xenocode Postbuild");                          break;
        case RECORD_NAME_XCOMP:                             sResult=QString("XComp");                                       break;
        case RECORD_NAME_XPACK:                             sResult=QString("XPack");                                       break;
        case RECORD_NAME_YANO:                              sResult=QString("Yano");                                        break;
        case RECORD_NAME_YODASCRYPTER:                      sResult=QString("Yoda's Crypter");                              break;
        case RECORD_NAME_YZPACK:                            sResult=QString("YZPack");                                      break;
        case RECORD_NAME_ZIP:                               sResult=QString("ZIP");                                         break;
        case RECORD_NAME_ZPROTECT:                          sResult=QString("ZProtect");                                    break;
    }

    return sResult;
}

SpecAbstract::UNPACK_OPTIONS SpecAbstract::getPossibleUnpackOptions(QIODevice *pDevice)
{
    UNPACK_OPTIONS result= {};

    QSet<QBinary::FT> stFileTypes=QBinary::getFileTypes(pDevice);

    if(stFileTypes.contains(QBinary::FT_PE32)||stFileTypes.contains(QBinary::FT_PE64))
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

QString SpecAbstract::createFullResultString(const SpecAbstract::SCAN_STRUCT *pScanStruct)
{
    return QString("%1: %2").arg(createTypeString(pScanStruct)).arg(createResultString(pScanStruct));
}

QString SpecAbstract::createFullResultString2(const SpecAbstract::SCAN_STRUCT *pScanStruct)
{
    return QString("%1: %2").arg(createTypeString(pScanStruct)).arg(createResultString2(pScanStruct));
}

QString SpecAbstract::createTypeString(const SpecAbstract::SCAN_STRUCT *pScanStruct)
{
    QString sResult;

    if(pScanStruct->parentId.filepart!=RECORD_FILEPART_HEADER)
    {
        sResult+=QString("%1: ").arg(SpecAbstract::recordFilepartIdToString(pScanStruct->parentId.filepart));
    }

    sResult+=SpecAbstract::recordFiletypeIdToString(pScanStruct->id.filetype);

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

SpecAbstract::BINARYINFO_STRUCT SpecAbstract::getBinaryInfo(QIODevice *pDevice, SpecAbstract::ID parentId, SCAN_OPTIONS *pOptions)
{
    QElapsedTimer timer;
    timer.start();

    BINARYINFO_STRUCT result=BINARYINFO_STRUCT();

    QPE binary(pDevice);

    result.basic_info.parentId=parentId;
    result.basic_info.id.filetype=RECORD_FILETYPE_BINARY;
    result.basic_info.id.filepart=RECORD_FILEPART_HEADER;
    result.basic_info.id.uuid=QUuid::createUuid();
    result.basic_info.nOffset=0;
    result.basic_info.nSize=pDevice->size();
    result.basic_info.sHeaderSignature=binary.getSignature(0,150);
    result.basic_info.bDeepScan=pOptions->bDeepScan;

    // Scan Header
    signatureScan(&result.basic_info.mapHeaderDetects,result.basic_info.sHeaderSignature,_binary_records,sizeof(_binary_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_BINARY);

    result.bIsPlainText=binary.isPlainTextType();
    result.bIsUTF8=binary.isUTF8TextType();
    result.unicodeType=binary.getUnicodeType();

    if(result.unicodeType!=QBinary::UNICODE_TYPE_NONE)
    {
        result.sHeaderText=binary.read_unicodeString(2,qMin(result.basic_info.nSize,(qint64)0x1000),(result.unicodeType==QBinary::UNICODE_TYPE_BE));
        result.basic_info.id.filetype=RECORD_FILETYPE_TEXT;
    }
    else if(result.bIsUTF8)
    {
        result.sHeaderText=binary.read_utf8String(3,qMin(result.basic_info.nSize,(qint64)0x1000));
        result.basic_info.id.filetype=RECORD_FILETYPE_TEXT;
    }
    else if(result.bIsPlainText)
    {
        result.sHeaderText=binary.read_ansiString(0,qMin(result.basic_info.nSize,(qint64)0x1000));
        result.basic_info.id.filetype=RECORD_FILETYPE_TEXT;
    }

    Binary_handle_Texts(pDevice,&result);
    Binary_handle_Formats(pDevice,&result);
    Binary_handle_Archives(pDevice,&result);
    Binary_handle_Certificates(pDevice,&result);
    Binary_handle_DebugData(pDevice,&result);
    Binary_handle_InstallerData(pDevice,&result);
    Binary_handle_SFXData(pDevice,&result);
    Binary_handle_ProtectorData(pDevice,&result);

    result.basic_info.listDetects.append(result.mapResultTexts.values());
    result.basic_info.listDetects.append(result.mapResultArchives.values());
    result.basic_info.listDetects.append(result.mapResultCertificates.values());
    result.basic_info.listDetects.append(result.mapResultDebugData.values());
    result.basic_info.listDetects.append(result.mapResultFormats.values());
    result.basic_info.listDetects.append(result.mapResultInstallerData.values());
    result.basic_info.listDetects.append(result.mapResultSFXData.values());
    result.basic_info.listDetects.append(result.mapResultProtectorData.values());

    if(!result.basic_info.listDetects.count())
    {
        _SCANS_STRUCT ssUnknown={};

        ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
        ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

        result.basic_info.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

SpecAbstract::MSDOSINFO_STRUCT SpecAbstract::getMSDOSInfo(QIODevice *pDevice, SpecAbstract::ID parentId, SpecAbstract::SCAN_OPTIONS *pOptions)
{
    QElapsedTimer timer;
    timer.start();

    MSDOSINFO_STRUCT result={};

    QMSDOS msdos(pDevice);

    result.basic_info.parentId=parentId;
    result.basic_info.id.filetype=RECORD_FILETYPE_MSDOS;
    result.basic_info.id.filepart=RECORD_FILEPART_HEADER;
    result.basic_info.id.uuid=QUuid::createUuid();
    result.basic_info.nOffset=0;
    result.basic_info.nSize=pDevice->size();
    result.basic_info.sHeaderSignature=msdos.getSignature(0,150);
    result.basic_info.bDeepScan=pOptions->bDeepScan;

    // TODO EntryPoint Signature

    if(!result.basic_info.listDetects.count())
    {
        _SCANS_STRUCT ssUnknown={};

        ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
        ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

        result.basic_info.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

SpecAbstract::ELFINFO_STRUCT SpecAbstract::getELFInfo(QIODevice *pDevice, SpecAbstract::ID parentId, SpecAbstract::SCAN_OPTIONS *pOptions)
{
    QElapsedTimer timer;
    timer.start();

    ELFINFO_STRUCT result={};

    QELF elf(pDevice);

    if(elf.isValid())
    {
        result.bIs64=elf.is64();

        result.basic_info.parentId=parentId;
        result.basic_info.id.filetype=result.bIs64?RECORD_FILETYPE_ELF64:RECORD_FILETYPE_ELF32;
        result.basic_info.id.filepart=RECORD_FILEPART_HEADER;
        result.basic_info.id.uuid=QUuid::createUuid();
        result.basic_info.nOffset=0;
        result.basic_info.nSize=pDevice->size();
        result.basic_info.sHeaderSignature=elf.getSignature(0,150);
        result.basic_info.bDeepScan=pOptions->bDeepScan;

        result.sEntryPointSignature=elf.getSignature(elf.getEntryPointOffset(),150);

        if(!result.basic_info.listDetects.count())
        {
            _SCANS_STRUCT ssUnknown={};

            ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
            ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

            result.basic_info.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
        }
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

SpecAbstract::MACHINFO_STRUCT SpecAbstract::getMACHInfo(QIODevice *pDevice, SpecAbstract::ID parentId, SpecAbstract::SCAN_OPTIONS *pOptions)
{
    QElapsedTimer timer;
    timer.start();

    MACHINFO_STRUCT result={};

    QMACH mach(pDevice);

    if(mach.isValid())
    {
        result.bIs64=mach.is64();

        result.basic_info.parentId=parentId;
        result.basic_info.id.filetype=result.bIs64?RECORD_FILETYPE_MACH64:RECORD_FILETYPE_MACH32;
        result.basic_info.id.filepart=RECORD_FILEPART_HEADER;
        result.basic_info.id.uuid=QUuid::createUuid();
        result.basic_info.nOffset=0;
        result.basic_info.nSize=pDevice->size();
        result.basic_info.sHeaderSignature=mach.getSignature(0,150);
        result.basic_info.bDeepScan=pOptions->bDeepScan;

        result.sEntryPointSignature=mach.getSignature(mach.getEntryPointOffset(),150);

        if(!result.basic_info.listDetects.count())
        {
            _SCANS_STRUCT ssUnknown={};

            ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
            ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

            result.basic_info.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
        }
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

SpecAbstract::PEINFO_STRUCT SpecAbstract::getPEInfo(QIODevice *pDevice,SpecAbstract::ID parentId, SpecAbstract::SCAN_OPTIONS *pOptions)
{
    QElapsedTimer timer;
    timer.start();

    PEINFO_STRUCT result={};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        result.bIs64=pe.is64();

        result.basic_info.parentId=parentId;
        result.basic_info.id.filetype=result.bIs64?RECORD_FILETYPE_PE64:RECORD_FILETYPE_PE32;
        result.basic_info.id.filepart=RECORD_FILEPART_HEADER;
        result.basic_info.id.uuid=QUuid::createUuid();
        result.basic_info.nOffset=0;
        result.basic_info.nSize=pDevice->size();
        result.basic_info.sHeaderSignature=pe.getSignature(0,150);
        result.basic_info.bDeepScan=pOptions->bDeepScan;

        result.sEntryPointSignature=pe.getSignature(pe.getEntryPointOffset(),150);

        result.dosHeader=pe.getDosHeaderEx();
        result.fileHeader=pe.getFileHeader();
        result.nOverlayOffset=pe.getOverlayOffset();
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

        result.listSectionHeaders=pe.getSectionHeaders();
        result.listSectionRecords=QPE::getSectionRecords(&result.listSectionHeaders,pe.isImage());
        result.listImports=pe.getImports();
        result.export_header=pe.getExport();
        result.listResources=pe.getResources();
        result.listRichSignatures=pe.getRichSignatureRecords();
        result.cliInfo=pe.getCliInfo(true);
        result.sResourceManifest=pe.getResourceManifest(&result.listResources);
        result.resVersion=pe.getResourceVersion(&result.listResources);

//        for(int i=0;i<result.cliInfo.listAnsiStrings.count();i++)
//        {
//            qDebug(result.cliInfo.listAnsiStrings.at(i).toLatin1().data());
//        }

//        for(int i=0;i<result.cliInfo.listUnicodeStrings.count();i++)
//        {
//            qDebug(result.cliInfo.listUnicodeStrings.at(i).toLatin1().data());
//        }


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
        result.nMinorImageVersion=result.bIs64?result.optional_header.optionalHeader64.MinorImageVersion:result.optional_header.optionalHeader32.MinorImageVersion;
        result.nMajorImageVersion=result.bIs64?result.optional_header.optionalHeader64.MajorImageVersion:result.optional_header.optionalHeader32.MajorImageVersion;

        result.nEntryPointSection=pe.getEntryPointSection();
        result.nResourceSection=pe.getResourcesSection();
        result.nImportSection=pe.getImportSection();
        result.nCodeSection=pe.getNormalCodeSection();
        result.nDataSection=pe.getNormalDataSection();
        result.nConstDataSection=pe.getConstDataSection();
        result.nRelocsSection=pe.getRelocsSection();
        result.nTLSSection=pe.getTLSSection();

        if(result.nEntryPointSection!=-1)
        {
            result.sEntryPointSectionName=result.listSectionRecords.at(result.nEntryPointSection).sName;
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
            result.nCodeSectionOffset=result.listSectionRecords.at(result.nCodeSection).nOffset; // mb TODO for image
            result.nCodeSectionSize=result.listSectionRecords.at(result.nCodeSection).nSize; // TODO limit?
        }

        if(result.nDataSection!=-1)
        {
            result.nDataSectionOffset=result.listSectionRecords.at(result.nDataSection).nOffset;
            result.nDataSectionSize=result.listSectionRecords.at(result.nDataSection).nSize;
        }

        if(result.nConstDataSection!=-1)
        {
            result.nConstDataSectionOffset=result.listSectionRecords.at(result.nConstDataSection).nOffset;
            result.nConstDataSectionSize=result.listSectionRecords.at(result.nConstDataSection).nSize;
        }

        if(result.nEntryPointSection!=-1)
        {
            result.nEntryPointSectionOffset=result.listSectionRecords.at(result.nEntryPointSection).nOffset;
            result.nEntryPointSectionSize=result.listSectionRecords.at(result.nEntryPointSection).nSize;
        }

        if(result.nImportSection!=-1)
        {
            result.nImportSectionOffset=result.listSectionRecords.at(result.nImportSection).nOffset;
            result.nImportSectionSize=result.listSectionRecords.at(result.nImportSection).nSize;
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


        signatureScan(&result.basic_info.mapHeaderDetects,result.basic_info.sHeaderSignature,_PE_header_records,sizeof(_PE_header_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        signatureScan(&result.mapEntryPointDetects,result.sEntryPointSignature,_PE_entrypoint_records,sizeof(_PE_entrypoint_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        signatureScan(&result.mapOverlayDetects,result.sOverlaySignature,_binary_records,sizeof(_binary_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_BINARY);

        //        for(int i=0;i<result.listImports.count();i++)
        //        {
        //            signatureScan(&result._mapImportDetects,QBinary::stringToHex(result.listImports.at(i).sName.toUpper()),_import_records,sizeof(_import_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        for(int i=0;i<result.export_header.listPositions.count();i++)
        //        {
        //            signatureScan(&result.mapExportDetects,QBinary::stringToHex(result.export_header.listPositions.at(i).sFunctionName),_export_records,sizeof(_export_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
        //        }

        //        resourcesScan(&result.mapResourcesDetects,&result.listResources,_resources_records,sizeof(_resources_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);


        if(result.cliInfo.bInit)
        {
            stringScan(&result.mapDotAnsistringsDetects,&result.cliInfo.listAnsiStrings,_PE_dot_ansistrings_records,sizeof(_PE_dot_ansistrings_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);

//            for(int i=0;i<result.cliInfo.listUnicodeStrings.count();i++)
//            {
//                signatureScan(&result.mapDotUnicodestringsDetects,QBinary::stringToHex(result.cliInfo.listUnicodeStrings.at(i)),_dot_unicodestrings_records,sizeof(_dot_unicodestrings_records),result.basic_info.id.filetype,SpecAbstract::RECORD_FILETYPE_PE);
//            }
        }

        PE_handle_import(pDevice,&result);

        PE_handle_Protection(pDevice,&result);
        PE_handle_VMProtect(pDevice,&result);
        PE_handle_Armadillo(pDevice,&result);
        PE_handle_Petite(pDevice,&result);
        PE_handle_NETProtection(pDevice,&result);
        PE_handle_PolyMorph(pDevice,&result);
        PE_handle_libraries(pDevice,&result);
        PE_handle_Microsoft(pDevice,&result);
        PE_handle_Borland(pDevice,&result);
        PE_handle_Tools(pDevice,&result);
        PE_handle_SFX(pDevice,&result);
        PE_handle_Installers(pDevice,&result);
        PE_handle_UnknownProtection(pDevice,&result);

        // TODO
        // Filter

        //
        //        handle_Rich(&result);
        //        handle_ENIGMA(pDevice,&result);

        //        fixDetects(&result);


        result.basic_info.listDetects.append(result.mapResultLinkers.values());
        result.basic_info.listDetects.append(result.mapResultCompilers.values());
        result.basic_info.listDetects.append(result.mapResultLibraries.values());
        result.basic_info.listDetects.append(result.mapResultTools.values());
        result.basic_info.listDetects.append(result.mapResultProtectors.values());
        result.basic_info.listDetects.append(result.mapResultNETObfuscators.values());
        result.basic_info.listDetects.append(result.mapResultPackers.values());
        result.basic_info.listDetects.append(result.mapResultSFX.values());
        result.basic_info.listDetects.append(result.mapResultInstallers.values());

        // TODO unknown cryptors
        if(!result.basic_info.listDetects.count())
        {
            _SCANS_STRUCT ssUnknown={};

            ssUnknown.type=SpecAbstract::RECORD_TYPE_UNKNOWN;
            ssUnknown.name=SpecAbstract::RECORD_NAME_UNKNOWN;

            result.basic_info.listDetects.append(scansToScan(&(result.basic_info),&ssUnknown));
        }
    }

    result.basic_info.nElapsedTime=timer.elapsed();

    return result;
}

SpecAbstract::_SCANS_STRUCT SpecAbstract::getScansStruct(quint32 nVariant, SpecAbstract::RECORD_FILETYPES filetype, SpecAbstract::RECORD_TYPES type, SpecAbstract::RECORD_NAMES name, QString sVersion, QString sInfo, qint64 nOffset)
{
    _SCANS_STRUCT result={};

    result.nVariant=nVariant;
    result.filetype=filetype;
    result.type=type;
    result.name=name;
    result.sVersion=sVersion;
    result.sInfo=sInfo;
    result.nOffset=nOffset;

    return result;
}

void SpecAbstract::PE_handle_import(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
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
                else if((pPEInfo->listImports.at(0).listPositions.at(0).nOrdinal==1))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_yzpack_a");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==2)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress"))
                {
                    stDetects.insert("kernel32_upx0exe");   // 0.59-0.93
                    stDetects.insert("kernel32_upx1dll");
                    stDetects.insert("kernel32_pecompact3");

                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_fsg");
                        stDetects.insert("kernel32_winupack");
                        stDetects.insert("kernel32_andpakk");
                        stDetects.insert("kernel32_bero");

                        if(pPEInfo->listImports.at(0).sName=="kernel32.dll")
                        {
                            stDetects.insert("kernel32_mew");
                            stDetects.insert("kernel32_beroexepacker");
                            stDetects.insert("kernel32_exefog_1.1");
                            stDetects.insert("kernel32_fishpepacker_b");
                        }
                        else if(pPEInfo->listImports.at(0).sName=="KERNEL32.DLL")
                        {
                            stDetects.insert("kernel32_exefog_1.2");
                            stDetects.insert("kernel32_kkrunchy");
                        }
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA"))
                {
                    stDetects.insert("kernel32_zprotect");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress"))
                {
                    stDetects.insert("kernel32_packmana");
                    stDetects.insert("kernel32_exe32pack");
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==3)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_upx1exe");   // 0.94-1.93
                    stDetects.insert("kernel32_pecompact2");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect"))
                {
                    stDetects.insert("kernel32_upx2dll");
                    if(pPEInfo->listImports.at(0).sName=="KERNEL32.DLL")
                    {
                        if(pPEInfo->listImports.count()==1)
                        {
                            stDetects.insert("kernel32_quickpacknt");
                        }
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="LoadLibraryA"))
                {
                    stDetects.insert("kernel32_aspack");
                    stDetects.insert("kernel32_asprotect");
                    stDetects.insert("kernel32_alexprotector");
                    stDetects.insert("kernel32_exe_pack");
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_ahpacker");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetProcAddress"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_orien");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetProcAddress"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_npack");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualProtect"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_xpack");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==4)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_pecompact0");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_32lite");
                        stDetects.insert("kernel32_fishpepacker_a");
                    }
                    stDetects.insert("kernel32_pecompactx");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc"))
                {
                    if((pPEInfo->listImports.count()==1)&&(pPEInfo->listImports.at(0).sName=="Kernel32.dll"))
                    {
                        stDetects.insert("kernel32_hmimysprotector");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==5)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
                        stDetects.insert("kernel32_revprot");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GlobalFree"))
                {
                    stDetects.insert("kernel32_pecompact1");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess"))
                {
                    stDetects.insert("kernel32_pecompact4");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_hmimyspacker");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.at(0).sName=="Kernel32.dll")
                    {
                        stDetects.insert("kernel32_mkfpack");
                    }
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualProtect"))
                {
                    stDetects.insert("kernel32_packmanb");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualProtect"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_xcomp");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==6)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GetModuleHandleA"))
                {
                    stDetects.insert("kernel32_pecompact6");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="ExitProcess"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("kernel32_pepack");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==8)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="ExitProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="FreeLibrary")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetModuleFileNameA"))
                {
                    stDetects.insert("kernel32_pecompact5");
                }
                else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetModuleHandleA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="VirtualAlloc")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="VirtualFree")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="VirtualProtect")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="HeapCreate")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="HeapAlloc"))
                {
                    if((pPEInfo->listImports.count()==1)&&(pPEInfo->listImports.at(0).sName=="kernel32.dll"))
                    {
                        stDetects.insert("kernel32_vpacker");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==13)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
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
            else if(pPEInfo->listImports.at(0).listPositions.count()==22)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="lstrcatA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="LoadResource")&&
                        (pPEInfo->listImports.at(0).listPositions.at(2).sName=="SizeofResource")&&
                        (pPEInfo->listImports.at(0).listPositions.at(3).sName=="FindResourceA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(4).sName=="CloseHandle")&&
                        (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WriteFile")&&
                        (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CreateFileA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetTempFileNameA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(8).sName=="DeleteFileA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(9).sName=="Sleep")&&
                        (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GetExitCodeProcess")&&
                        (pPEInfo->listImports.at(0).listPositions.at(11).sName=="WaitForSingleObject")&&
                        (pPEInfo->listImports.at(0).listPositions.at(12).sName=="CreateProcessA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GetStartupInfoA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(14).sName=="LockResource")&&
                        (pPEInfo->listImports.at(0).listPositions.at(15).sName=="lstrcpyA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(16).sName=="GetCommandLineA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(17).sName=="FreeLibrary")&&
                        (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                        (pPEInfo->listImports.at(0).listPositions.at(19).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(20).sName=="GetWindowsDirectoryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(21).sName=="GetModuleFileNameA"))
                {
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("kernel32_cexe");
                    }
                }
            }
        }
        else if(pPEInfo->listImports.at(0).sName.toUpper()=="USER32.DLL")
        {
            if(pPEInfo->listImports.at(0).listPositions.count()==1)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="MessageBoxA"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("user32_pespina");
                    }
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("user32_pespin");
                    }
                }
            }
        }
        else if(pPEInfo->listImports.at(0).sName.toUpper()=="KERNEL32")
        {
            if(pPEInfo->listImports.at(0).listPositions.count()==1)
            {
                if((pPEInfo->listImports.at(0).listPositions.at(0).nOrdinal==1))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_yzpack_b");
                    }
                }
            }
            else if(pPEInfo->listImports.at(0).listPositions.count()==2)
            {
                if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="LoadLibraryA")&&
                        (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetProcAddress"))
                {
                    if(pPEInfo->listImports.count()==1)
                    {
                        stDetects.insert("kernel32_yzpack_c");
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
                        stDetects.insert("user32_revprot");
                        stDetects.insert("user32_exe_pack");
                        stDetects.insert("user32_ahpacker");
                    }
                }
            }
            else if(pPEInfo->listImports.at(1).listPositions.count()==2)
            {
                if( (pPEInfo->listImports.at(1).listPositions.at(0).sName=="wsprintfA")&&
                    (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MessageBoxA"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("user32_npack");
                    }
                }
                else if( (pPEInfo->listImports.at(1).listPositions.at(0).sName=="MessageBoxA")&&
                         (pPEInfo->listImports.at(1).listPositions.at(1).sName=="wsprintfA"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("user32_pepack");
                    }
                }
            }
            else if(pPEInfo->listImports.at(1).listPositions.count()==11)
            {
                if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetClientRect")&&
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
        else if(pPEInfo->listImports.at(1).sName.toUpper()=="COMCTL32.DLL")
        {
            if(pPEInfo->listImports.at(1).listPositions.count()==1)
            {
                if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="InitCommonControls"))
                {
                    if(pPEInfo->listImports.count()==2)
                    {
                        stDetects.insert("comctl32_pespina");
                        stDetects.insert("comctl32_orien");
                    }
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("comctl32_pespin");
                    }
                }
            }
        }
        else if(pPEInfo->listImports.at(1).sName.toUpper()=="LZ32.DLL")
        {
            if(pPEInfo->listImports.at(1).listPositions.count()==3)
            {
                if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="LZCopy")&&
                        (pPEInfo->listImports.at(1).listPositions.at(1).sName=="LZOpenFileA")&&
                        (pPEInfo->listImports.at(1).listPositions.at(2).sName=="LZClose"))
                {
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("lz32_cexe");
                    }
                }
            }
        }
    }
    if(pPEInfo->listImports.count()>=3)
    {
        if(pPEInfo->listImports.at(2).sName.toUpper()=="KERNEL32.DLL")
        {
            if(pPEInfo->listImports.at(2).listPositions.count()==2)
            {
                if( (pPEInfo->listImports.at(2).listPositions.at(0).sName=="LoadLibraryA")&&
                    (pPEInfo->listImports.at(2).listPositions.at(1).sName=="GetProcAddress"))
                {
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("kernel32_pespinx");
                    }
                }
            }
            else if(pPEInfo->listImports.at(2).listPositions.count()==4)
            {
                if( (pPEInfo->listImports.at(2).listPositions.at(0).sName=="LoadLibraryA")&&
                    (pPEInfo->listImports.at(2).listPositions.at(1).sName=="GetProcAddress")&&
                    (pPEInfo->listImports.at(2).listPositions.at(2).sName=="VirtualAlloc")&&
                    (pPEInfo->listImports.at(2).listPositions.at(3).sName=="VirtualFree"))
                {
                    if(pPEInfo->listImports.count()==3)
                    {
                        stDetects.insert("kernel32_pespin");
                    }
                }
            }
        }
        else if(pPEInfo->listImports.at(2).sName.toUpper()=="USER32.DLL")
        {
            if(pPEInfo->listImports.at(2).listPositions.count()==1)
            {
                if(pPEInfo->listImports.at(2).listPositions.at(0).sName=="MessageBoxA")
                {
                    stDetects.insert("user32_cexe");
                }
            }
        }
    }

    for(int i=0;i<pPEInfo->listImports.count();i++)
    {
        if(pPEInfo->listImports.at(i).sName.toUpper()=="KERNEL32.DLL")
        {
            if(pPEInfo->listImports.at(i).listPositions.count()==4)
            {
                if( (pPEInfo->listImports.at(i).listPositions.at(0).sName=="LoadLibraryA")&&
                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="ExitProcess")&&
                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="GetProcAddress")&&
                    (pPEInfo->listImports.at(i).listPositions.at(3).sName=="VirtualProtect"))
                {
                    stDetects.insert("kernel32_upx3exe_new");   // 3.91+
                }
            }
            else if(pPEInfo->listImports.at(i).listPositions.count()==3)
            {
                if( (pPEInfo->listImports.at(i).listPositions.at(0).sName=="LoadLibraryA")&&
                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="GetProcAddress")&&
                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="VirtualProtect"))
                {
                    stDetects.insert("kernel32_upx3dll_new");   // 3.91+
                }
            }
        }
    }

#ifdef QT_DEBUG
    qDebug()<<stDetects;
#endif

    // TODO 32/64
    if(stDetects.contains("kernel32_andpakk"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ANDPAKK2,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ANDPAKK2,"0.18","",0));
    }

    if(stDetects.contains("kernel32_asdpack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ASDPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ASDPACK,"2.0","",0));
    }

    if(stDetects.contains("kernel32_vpacker"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_VPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_VPACKER,"0.02.10","",0));
    }

    if(stDetects.contains("kernel32_kkrunchy"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_KKRUNCHY,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_KKRUNCHY,"","",0));
    }

    if(stDetects.contains("kernel32_quickpacknt"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_QUICKPACKNT,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_QUICKPACKNT,"0.1","",0));
    }

    if(stDetects.contains("kernel32_zprotect"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ZPROTECT,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_ZPROTECT,"","",0));
    }

    if(stDetects.contains("kernel32_yzpack_a"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_YZPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_YZPACK,"1.1","",0));
    }
    else if(stDetects.contains("kernel32_yzpack_b"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_YZPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_YZPACK,"1.2","",0));
    }
    else if(stDetects.contains("kernel32_yzpack_c"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_YZPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_YZPACK,"2.0","",0));
    }

    if(stDetects.contains("kernel32_32lite"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_32LITE,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_32LITE,"0.03a","",0));
    }

    if(stDetects.contains("kernel32_fishpepacker_a"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_FISHPEPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_FISHPEPACKER,"1.02","",0));
    }
    else if(stDetects.contains("kernel32_fishpepacker_b"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_FISHPEPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_FISHPEPACKER,"1.03","",0));
    }

    if(stDetects.contains("kernel32_aspack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ASPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_ASPACK,"","",0));
    }

    if(stDetects.contains("kernel32_mkfpack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_MKFPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_MKFPACK,"","",0));
    }

    if(stDetects.contains("kernel32_packmana"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PACKMAN,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_PACKMAN,"0.0.0.1","",0));
    }
    else if(stDetects.contains("kernel32_packmanb"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PACKMAN,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_PACKMAN,"1.0","",0));
    }

    if(stDetects.contains("kernel32_mew"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_MEW11SE,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_MEW11SE,"","",0));
    }

    if(stDetects.contains("kernel32_nspack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_NSPACK,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_NSPACK,"","",0));
    }

    if(stDetects.contains("kernel32_pex")&&stDetects.contains("user32_pex"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PEX,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_PEX,"0.99","",0));
    }

    if(stDetects.contains("kernel32_revprot")&&stDetects.contains("user32_revprot"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_REVPROT,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_REVPROT,"0.1a","",0));
    }

    if(stDetects.contains("kernel32_npack")&&stDetects.contains("user32_npack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_NPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_NPACK,"","",0));
    }

    if(stDetects.contains("kernel32_ahpacker")&&stDetects.contains("user32_ahpacker"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_AHPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_AHPACKER,"0.1","",0));
    }

    if(stDetects.contains("kernel32_beroexepacker"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_BEROEXEPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_BEROEXEPACKER,"1.00","",0));
    }

    if(stDetects.contains("user32_pespina")&&stDetects.contains("comctl32_pespina"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PESPIN,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_PESPIN,"1.0-1.2","",0));
    }

    if(stDetects.contains("user32_pespin")&&stDetects.contains("comctl32_pespin")&&stDetects.contains("kernel32_pespin"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PESPIN,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_PESPIN,"","",0));
    }

    if(stDetects.contains("user32_pespin")&&stDetects.contains("comctl32_pespin")&&stDetects.contains("kernel32_pespinx"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PESPIN,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_PESPIN,"1.3X","",0));
    }

    if(stDetects.contains("kernel32_orien")&&stDetects.contains("comctl32_orien"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_ORIEN,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_ORIEN,"","",0));
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

    if(stDetects.contains("kernel32_exe_pack")&&stDetects.contains("user32_exe_pack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_EXEPACK,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_EXEPACK,"","",0));
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
        pPEInfo->mapImportDetects.insert(RECORD_NAME_FSG,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_FSG,"","",0));
    }

    if(stDetects.contains("kernel32_hmimysprotector"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_HMIMYSPROTECTOR,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PROTECTOR,RECORD_NAME_HMIMYSPROTECTOR,"0.1","",0));
    }

    if(stDetects.contains("kernel32_hmimyspacker"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_HMIMYSPACKER,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_HMIMYSPACKER,"","",0));
    }

    if(stDetects.contains("kernel32_dyamar")&&stDetects.contains("user32_dyamar"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_DYAMAR,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_DYAMAR,"1.3.5","",0));
    }

    if(stDetects.contains("kernel32_pepack")&&stDetects.contains("user32_pepack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PEPACK,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PEPACK,"1.0","",0));
    }

    if(stDetects.contains("kernel32_xcomp"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_XCOMP,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_XCOMP,"0.97-0.98","",0));
    }

    if(stDetects.contains("kernel32_xpack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_XPACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_XPACK,"0.97-0.98","",0));
    }

    if(stDetects.contains("kernel32_exe32pack"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_EXE32PACK,getScansStruct(0,RECORD_FILETYPE_PE32,RECORD_TYPE_PACKER,RECORD_NAME_EXE32PACK,"1.4X","",0));
    }

    if(stDetects.contains("kernel32_pecompact0"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"0.90-0.91","",0));
    }

    if(stDetects.contains("kernel32_pecompact1"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"0.92-0.94","",0));
    }

    if(stDetects.contains("kernel32_pecompact2"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"0.97-0.971b","",0));
    }

    if(stDetects.contains("kernel32_pecompact3"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"0.975-1.10b3","",0));
    }

    if(stDetects.contains("kernel32_pecompact4"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"1.10b7-1.34","",0));
    }

    if(stDetects.contains("kernel32_pecompact5")) // TODO Cjeck
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"1.30-1.40","",0));
    }

    if(stDetects.contains("kernel32_pecompact6"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"1.40-1.84","",0));
    }

    if(stDetects.contains("kernel32_pecompactx"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(1,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"2.40-3.X","",0));
    }

    if(stDetects.contains("kernel32_exefog_1.1"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_EXEFOG,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_EXEFOG,"1.1","",0));
    }
    else if(stDetects.contains("kernel32_exefog_1.2"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_EXEFOG,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_EXEFOG,"1.2","",0));
    }

    if(stDetects.contains("kernel32_cexe")&&stDetects.contains("lz32_cexe")&&stDetects.contains("user32_cexe"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_CEXE,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_CEXE,"1.0","",0));
    }

//    if(stDetects.contains("kernel32_pecompact2"))
//    {
//        pPEInfo->mapImportDetects.insert(RECORD_NAME_PECOMPACT,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_PECOMPACT,"2.X","",0));
//    }

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
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"2.90-3.XX","exe",0));
    }
    else if(stDetects.contains("kernel32_upx3dll"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"2.90-3.XX","dll",0));
    }
    else if(stDetects.contains("kernel32_upx3exe_new"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(1,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"3.91+","exe",0));
    }
    else if(stDetects.contains("kernel32_upx3dll_new"))
    {
        pPEInfo->mapImportDetects.insert(RECORD_NAME_UPX,getScansStruct(1,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_UPX,"3.91+","dll",0));
    }
}

void SpecAbstract::PE_handle_Protection(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        // MPRESS
        if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MPRESS))
        {
            SpecAbstract::_SCANS_STRUCT recordMPRESS=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_MPRESS);

            qint64 nOffsetMPRESS=pe.find_ansiString(0x1f0,16,"v");
            if(nOffsetMPRESS!=-1)
            {
                recordMPRESS.sVersion=pe.read_ansiString(nOffsetMPRESS+1,0x1ff-nOffsetMPRESS);
            }

            pPEInfo->mapResultPackers.insert(recordMPRESS.name,scansToScan(&(pPEInfo->basic_info),&recordMPRESS));
        }

        if(!pPEInfo->cliInfo.bInit)
        {
            // TODO MPRESS import

            VI_STRUCT viUPX=PE_get_UPX_vi(pDevice,pPEInfo);

            // UPX
            // TODO 32-64
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_UPX))
            {
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_UPX))
                {
                    if((viUPX.sVersion!=""))
                    {
                        SpecAbstract::_SCANS_STRUCT recordUPX={};

                        recordUPX.type=RECORD_TYPE_PACKER;
                        recordUPX.name=RECORD_NAME_UPX;
                        recordUPX.sVersion=viUPX.sVersion;
                        recordUPX.sInfo=viUPX.sInfo;

                        pPEInfo->mapResultPackers.insert(recordUPX.name,scansToScan(&(pPEInfo->basic_info),&recordUPX));
                    }
                    else
                    {
                        SpecAbstract::_SCANS_STRUCT recordUPX=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_UPX);

                        recordUPX.sInfo=append(recordUPX.sInfo,"modified");

                        pPEInfo->mapResultPackers.insert(recordUPX.name,scansToScan(&(pPEInfo->basic_info),&recordUPX));
                    }
                }
            }

            // ASProtect
            if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ASPROTECT))
            {
                SpecAbstract::_SCANS_STRUCT recordSS=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ASPROTECT);

                pPEInfo->mapResultProtectors.insert(recordSS.name,scansToScan(&(pPEInfo->basic_info),&recordSS));
            }

            // PECompact
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_PECOMPACT))
            {
                SpecAbstract::_SCANS_STRUCT recordPC=pPEInfo->mapImportDetects.value(RECORD_NAME_PECOMPACT);

                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PECOMPACT))
                {
                    if(recordPC.nVariant==1)
                    {
                        recordPC.sVersion="1.10b4-1.10b5";
                    }

                    pPEInfo->mapResultPackers.insert(recordPC.name,scansToScan(&(pPEInfo->basic_info),&recordPC));
                }
                else
                {
                    if(pPEInfo->listSectionHeaders.count()>=2)
                    {
                        if(pPEInfo->listSectionHeaders.at(0).PointerToRelocations==0x32434550)
                        {
                            quint32 nBuildNumber=pPEInfo->listSectionHeaders.at(0).PointerToLinenumbers;

                            // TODO !!! more buil versions
                            switch(nBuildNumber)
                            {
                            case 20206:     recordPC.sVersion="2.70";       break;
                            case 20240:     recordPC.sVersion="2.78a";      break;
                            case 20243:     recordPC.sVersion="2.79b1";     break;
                            case 20245:     recordPC.sVersion="2.79bB";     break;
                            case 20247:     recordPC.sVersion="2.79bD";     break;
                            case 20252:     recordPC.sVersion="2.80b1";     break;
                            case 20256:     recordPC.sVersion="2.80b5";     break;
                            case 20261:     recordPC.sVersion="2.82";       break;
                            case 20288:     recordPC.sVersion="2.93b3";     break;
                            case 20294:     recordPC.sVersion="2.96.2";     break;
                            case 20295:     recordPC.sVersion="2.97b1";     break;
                            case 20296:     recordPC.sVersion="2.98";       break;
                            case 20300:     recordPC.sVersion="2.98.04";    break;
                            case 20302:     recordPC.sVersion="2.98.06";    break;
                            case 20303:     recordPC.sVersion="2.99b";      break;
                            case 20308:     recordPC.sVersion="3.00.2";     break;
                            case 20318:     recordPC.sVersion="3.02.2";     break;
//                            case 20343:
//                                recordPC.sVersion="TODO";
//                                break;
                            default:
                                {
                                    if(nBuildNumber>20308)
                                    {
                                        recordPC.sVersion=QString("3.X(build %1)").arg(nBuildNumber);
                                    }
                                    else if(nBuildNumber==0)
                                    {
                                        recordPC.sVersion="2.20-2.68";
                                    }
                                    else
                                    {
                                        recordPC.sVersion=QString("2.X(build %1)").arg(nBuildNumber);
                                    }
                                }
                            }
//                            qDebug("nVersion: %d",nVersion);

                            // TODO more versions
                            pPEInfo->mapResultPackers.insert(recordPC.name,scansToScan(&(pPEInfo->basic_info),&recordPC));
                        }
                    }
                }
            }

            // NSPack
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_NSPACK))
            {
                if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_NSPACK))
                {
                    SpecAbstract::_SCANS_STRUCT recordNSPack=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_NSPACK);
                    pPEInfo->mapResultPackers.insert(recordNSPack.name,scansToScan(&(pPEInfo->basic_info),&recordNSPack));
                }
                else if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_NSPACK))
                {
                    SpecAbstract::_SCANS_STRUCT recordNSPack=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_NSPACK);
                    pPEInfo->mapResultPackers.insert(recordNSPack.name,scansToScan(&(pPEInfo->basic_info),&recordNSPack));
                }
            }

            // YZPack
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_YZPACK))
            {
                if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_YZPACK))
                {
                    SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_YZPACK);
                    pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // ENIGMA
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ENIGMA))
            {
                int nVariant=pPEInfo->mapImportDetects.value(RECORD_NAME_ENIGMA).nVariant;

                if((pPEInfo->nImportSectionOffset)&&(pPEInfo->nImportSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                {
                    qint64 nSectionOffset=pPEInfo->nImportSectionOffset;
                    qint64 nSectionSize=pPEInfo->nImportSectionSize;

                    bool bDetect=false;

                    SpecAbstract::_SCANS_STRUCT recordEnigma={};

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

//                    if((!bDetect)&&(nVariant==1))
                    if(!bDetect)
                    {
                        QString sEnigmaVersion=findEnigmaVersion(pDevice,nSectionOffset,nSectionSize);

                        if(sEnigmaVersion!="")
                        {
                            recordEnigma.sVersion=sEnigmaVersion;
                            bDetect=true;
                        }
                    }

                    if(!bDetect)
                    {
                        if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ENIGMA))
                        {
                            recordEnigma.sVersion=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ENIGMA).sVersion;
                            bDetect=true;
                        }
                    }

                    if(bDetect)
                    {
                        pPEInfo->mapResultProtectors.insert(recordEnigma.name,scansToScan(&(pPEInfo->basic_info),&recordEnigma));
                    }
                }
            }

            // PESpin
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_PESPIN))
            {
                SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_PESPIN);
                // Get version
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PESPIN))
                {
                    quint8 nByte=pPEInfo->sEntryPointSignature.mid(54,2).toInt(0,16);
                    switch(nByte)
                    {
                    case 0x5C: ss.sVersion="0.1";     break;
                    case 0xB7: ss.sVersion="0.3";     break;
                    case 0x73: ss.sVersion="0.4";     break;
                    case 0x83: ss.sVersion="0.7";     break;
                    case 0xC8: ss.sVersion="1.0";     break;
                    case 0x7D: ss.sVersion="1.1";     break;
                    case 0x71: ss.sVersion="1.3beta"; break;
                    case 0xAC: ss.sVersion="1.3";     break;
                    case 0x88: ss.sVersion="1.3x";    break;
                    case 0x17: ss.sVersion="1.32";    break;
                    case 0x77: ss.sVersion="1.33";    break;
                    }
                }
                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // CExe
            if(pPEInfo->mapImportDetects.contains(RECORD_NAME_CEXE))
            {
                SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_CEXE);
                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(!pPEInfo->bIs64)
            {
                // ZProtect
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ZPROTECT))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_NOSTUBLINKER))
                    {
                        if(pPEInfo->listSectionRecords.count()>=2)
                        {
                            // TODO new versions
                            if(pe.compareSignature("'kernel32.dll'00000000'VirtualAlloc'00000000",pPEInfo->listSectionRecords.at(1).nOffset))
                            {
                                SpecAbstract::_SCANS_STRUCT recordZProtect=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ZPROTECT,"1.3-1.4.4","",0);
                                pPEInfo->mapResultProtectors.insert(recordZProtect.name,scansToScan(&(pPEInfo->basic_info),&recordZProtect));
                            }
                        }
                    }
                }

                // ExeFog
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_EXEFOG))
                {
                    SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_EXEFOG);
                    if(     (pPEInfo->fileHeader.TimeDateStamp==0)&&
                            (pPEInfo->optional_header.optionalHeader32.MajorLinkerVersion==0)&&
                            (pPEInfo->optional_header.optionalHeader32.MinorLinkerVersion==0)&&
                            (pPEInfo->optional_header.optionalHeader32.BaseOfData==0x1000))
                    {
                        if(pPEInfo->listSectionHeaders.count())
                        {
                            if(pPEInfo->listSectionHeaders.at(0).Characteristics==0xe0000020)
                            {
                                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                            }
                        }
                    }
                }

                // AHPacker
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_AHPACKER))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_AHPACKER))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_AHPACKER);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // BeRoEXEPacker
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_BEROEXEPACKER))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_BEROEXEPACKER))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_BEROEXEPACKER);

                        if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_BEROEXEPACKER))
                        {
                            ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_BEROEXEPACKER);
                        }

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                    else if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GENERIC))
                    {
                        if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_BEROEXEPACKER))
                        {
                            SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_BEROEXEPACKER);
                            pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }
                }

                // Winupack
                if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_WINUPACK))
                {
                    SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_WINUPACK);

                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_WINUPACK))
                    {
                        ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_WINUPACK);
                    }

//                    recordWinupack.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(((pPEInfo->nMinorLinkerVersion)/16)*10+(pPEInfo->nMinorLinkerVersion)%16);

                    int nBuildNumber=0;
                    if((ss.nVariant==1)||(ss.nVariant==2))
                    {
                        nBuildNumber=pPEInfo->nMinorLinkerVersion;
                    }
                    else if((ss.nVariant==3)||(ss.nVariant==4))
                    {
                        nBuildNumber=pPEInfo->nMinorImageVersion;
                    }

                #ifdef QT_DEBUG
                    qDebug("nBuildNumber: %x",nBuildNumber);
                #endif


                    switch(nBuildNumber)
                    {
                    case 0x24:  ss.sVersion="0.24";         break;
                    case 0x25:  ss.sVersion="0.25";         break;
                    case 0x27:  ss.sVersion="0.27";         break;
                    case 0x28:  ss.sVersion="0.28";         break;
                    case 0x29:  ss.sVersion="0.29";         break;
                    case 0x30:  ss.sVersion="0.30";         break;
                    case 0x32:  ss.sVersion="0.32";         break;
                    case 0x33:  ss.sVersion="0.33";         break;
                    case 0x35:  ss.sVersion="0.35";         break;
                    case 0x36:  ss.sVersion="0.36 beta";    break;
                    case 0x37:  ss.sVersion="0.37 beta";    break;
                    case 0x38:  ss.sVersion="0.38 beta";    break;
                    case 0x39:  ss.sVersion="0.39 final";   break;
                    case 0x3A:  ss.sVersion="0.399";        break;
                    default:    ss.sVersion="";
                    }

                    pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }

                // ANDpakk2
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ANDPAKK2)||
                        pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_ANDPAKK2))
                {
                    // TODO compare entryPoint and import sections
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ANDPAKK2))
                    {
                        SpecAbstract::_SCANS_STRUCT recordANFpakk2=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ANDPAKK2);
                        pPEInfo->mapResultPackers.insert(recordANFpakk2.name,scansToScan(&(pPEInfo->basic_info),&recordANFpakk2));
                    }
                }

                // ASDPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ASDPACK))
                {
                    // TODO compare entryPoint and import sections
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ASDPACK))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ASDPACK);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // VPacker
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_VPACKER))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_VPACKER))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_VPACKER);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // KKrunchy
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_KKRUNCHY))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_KKRUNCHY))
                    {
                        SpecAbstract::_SCANS_STRUCT ss={};
                        if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_KKRUNCHY))
                        {
                            ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_KKRUNCHY);
                        }
                        else
                        {
                            ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_KKRUNCHY);
                        }

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // QuickPack NT
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_QUICKPACKNT))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_QUICKPACKNT))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_QUICKPACKNT);

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // MKFPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_MKFPACK))
                {
                    qint64 mLfanew=pPEInfo->dosHeader.e_lfanew-5;

                    if(mLfanew>0)
                    {
                        QString sSignature=pe.read_ansiString(mLfanew,5);
                        if(sSignature=="llydd")
                        {
                            SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapImportDetects.value(RECORD_NAME_MKFPACK);
                            pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }
                }

                // 32lite
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_32LITE))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_32LITE))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_32LITE);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // Packman
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_PACKMAN))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PACKMAN))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PACKMAN);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // Fish PE Packer
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_FISHPEPACKER))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_FISHPEPACKER))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_FISHPEPACKER);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // ACProtect
                // 1.X-2.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ACPROTECT))
                {
                    if((pPEInfo->nImportSectionOffset)&&(pPEInfo->nImportSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                    {
                        qint64 nSectionOffset=pPEInfo->nImportSectionOffset;
                        qint64 nSectionSize=pPEInfo->nImportSectionSize;

                        qint64 nOffset1=pe.find_array(nSectionOffset,nSectionSize,"MineImport_Endss",16);

                        if(nOffset1!=-1)
                        {
                            SpecAbstract::_SCANS_STRUCT recordACProtect={};
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
                    SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ACPROTECT);
                    pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }

                // FSG
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_FSG))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_FSG))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_FSG);
                        if(ss.nVariant==0)
                        {
                            pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                        else if(ss.nVariant==1)
                        {
                            if(pe.read_ansiString(0x154)=="KERNEL32.dll")
                            {
                                ss.sVersion="1.33";
                            }
                            else
                            {
                                ss.sVersion="2.00";
                            }
                            pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }
                }

                // MEW
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_MEW11SE))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MEW11SE))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_MEW11SE);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // Alex Protector
                // 2.0.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ALEXPROTECTOR))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ALEXPROTECTOR))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ALEXPROTECTOR);
                        pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_HMIMYSPROTECTOR))
                {
                    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_HMIMYSPROTECTOR))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_HMIMYSPROTECTOR);
                        pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_HMIMYSPACKER))
                {
                    if(QPE::isSectionNamePresent(".hmimys",&(pPEInfo->listSectionHeaders)))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PACKER,RECORD_NAME_HMIMYSPACKER,"","",0);
                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ORIEN))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ORIEN))
                    {
                        SpecAbstract::_SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ORIEN);

                        QString sVersion=pPEInfo->sEntryPointSignature.mid(16,2);

                        if(sVersion=="CE")
                        {
                            ss.sVersion="2.11";
                        }
                        else if(sVersion=="CD")
                        {
                            ss.sVersion="2.12";
                        }

                        pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // Alloy
                // 4.X
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_ALLOY))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ALLOY))
                    {
                        // TODO compare entryPoint and import sections
                        SpecAbstract::_SCANS_STRUCT recordAlloy=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ALLOY);
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
                            SpecAbstract::_SCANS_STRUCT recordPEX=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PEX);
                            pPEInfo->mapResultPackers.insert(recordPEX.name,scansToScan(&(pPEInfo->basic_info),&recordPEX));
                        }
                    }
                }

                // PEVProt
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_REVPROT))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_REVPROT))
                    {
                        SpecAbstract::_SCANS_STRUCT recordREVPROT=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_REVPROT);
                        pPEInfo->mapResultProtectors.insert(recordREVPROT.name,scansToScan(&(pPEInfo->basic_info),&recordREVPROT));
                    }
                }

                // nPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_NPACK))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_NPACK))
                    {
                        SpecAbstract::_SCANS_STRUCT recordNPACK=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_NPACK);

                        if((pPEInfo->nEntryPointSectionOffset)&&(pPEInfo->nEntryPointSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                        {
                            qint64 _nOffset=pPEInfo->nEntryPointSectionOffset;
                            qint64 _nSize=pPEInfo->nEntryPointSectionSize;

                            // TODO get max version
                            qint64 nOffset_Version=pe.find_ansiString(_nOffset,_nSize,"nPack v");

                            if(nOffset_Version!=-1)
                            {
                                recordNPACK.sVersion=pe.read_ansiString(nOffset_Version+7).section(":",0,0);
                            }
                            else
                            {
                                recordNPACK.sVersion="1.1.200.2006";
                            }
                        }

                        pPEInfo->mapResultPackers.insert(recordNPACK.name,scansToScan(&(pPEInfo->basic_info),&recordNPACK));
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
                            _sVersion="1.08.X";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60E841060000EB41"))
                        {
                            _sVersion="1.08.04";
                        }
                        else if(QBinary::compareSignatureStrings(_sSignature,"60EB..5DFFE5E8........81ED........BB........03DD2B9D"))
                        {
                            _sVersion="1.08.X";
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
                            _sVersion="2.12-2.42";
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
                        SpecAbstract::_SCANS_STRUCT recordASPack={};

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
                    SpecAbstract::_SCANS_STRUCT ss={};

                    ss.type=RECORD_TYPE_PACKER;
                    ss.name=RECORD_NAME_WWPACK32;
                    ss.sVersion=QBinary::hexToString(pPEInfo->sEntryPointSignature.mid(102,8));
                    //recordAndpakk.sInfo;

                    pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }

                // EXE Pack
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_EXEPACK))
                {
                    SpecAbstract::_SCANS_STRUCT recordEP=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_EXEPACK);

                    pPEInfo->mapResultPackers.insert(recordEP.name,scansToScan(&(pPEInfo->basic_info),&recordEP));
                }

                // PE_PACK
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_PEPACK))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PEPACK))
                    {
                        _SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PEPACK);

                        if((pPEInfo->nImportSectionOffset)&&(pPEInfo->nImportSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                        {
                            qint64 _nOffset=pPEInfo->nImportSectionOffset;
                            qint64 _nSize=pPEInfo->nImportSectionSize;

                            qint64 nOffset_PEPACK=pe.find_ansiString(_nOffset,_nSize,"PE-PACK v");

                            if(nOffset_PEPACK!=-1)
                            {
                                ss.sVersion=pe.read_ansiString(nOffset_PEPACK+9,50);
                                ss.sVersion=ss.sVersion.section(" ",0,0);
                            }
                        }

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // PKLITE32
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PKLITE32))
                {
                    SpecAbstract::_SCANS_STRUCT recordEP=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PKLITE32);

                    if(pe.compareEntryPoint("68........68........68........e8$$$$$$$$558beca1"))
                    {
                        pPEInfo->mapResultPackers.insert(recordEP.name,scansToScan(&(pPEInfo->basic_info),&recordEP));
                    }
                }

                // XComp
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_XCOMP))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_XCOMP))
                    {
                        _SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_XCOMP);

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // XPack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_XPACK))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_XPACK))
                    {
                        _SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_XPACK);

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }

                // ABC Cryptor
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_ABCCRYPTOR))
                {
                    SpecAbstract::_SCANS_STRUCT recordEP=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_ABCCRYPTOR);

                    if((pPEInfo->nEntryPointAddress-pPEInfo->listSectionHeaders.at(pPEInfo->nEntryPointSection).VirtualAddress)==1)
                    {
                        pPEInfo->mapResultPackers.insert(recordEP.name,scansToScan(&(pPEInfo->basic_info),&recordEP));
                    }
                }

                // exe 32 pack
                if(pPEInfo->mapImportDetects.contains(RECORD_NAME_EXE32PACK))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_EXE32PACK))
                    {
                        _SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_EXE32PACK);

                        qint64 _nOffset=pPEInfo->nHeaderOffset;
                        qint64 _nSize=qMin(pPEInfo->basic_info.nSize,(qint64)0x2000);

                        qint64 nOffset_version=pe.find_ansiString(_nOffset,_nSize,"Packed by exe32pack");

                        if(nOffset_version!=-1)
                        {
                            ss.sVersion=pe.read_ansiString(nOffset_version+20,50);
                            ss.sVersion=ss.sVersion.section(" ",0,0);
                        }

                        pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }
            }
        }
    }
}

void SpecAbstract::PE_handle_VMProtect(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            bool bSuccess=false;

            QSet<QString> stDetects;

            int nImportCount=pPEInfo->listImports.count();
            if(nImportCount>=2)
            {
                if(pPEInfo->listImports.at(nImportCount-2).sName.toUpper()=="KERNEL32.DLL")
                {
                    if(pPEInfo->listImports.at(nImportCount-2).listPositions.count()==12)
                    {
                        if( (pPEInfo->listImports.at(nImportCount-2).listPositions.at(0).sName=="LocalAlloc")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(1).sName=="LocalFree")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(2).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(3).sName=="GetProcessAffinityMask")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(4).sName=="SetProcessAffinityMask")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(5).sName=="SetThreadAffinityMask")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(6).sName=="Sleep")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(7).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(8).sName=="FreeLibrary")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(9).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(10).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(nImportCount-2).listPositions.at(11).sName=="GetProcAddress"))
                        {
                            stDetects.insert("kernel32_3");
                        }
                    }
                }

                if(pPEInfo->listImports.at(nImportCount-1).sName.toUpper()=="USER32.DLL")
                {
                    if(pPEInfo->listImports.at(nImportCount-1).listPositions.count()==2)
                    {
                        if( (pPEInfo->listImports.at(nImportCount-1).listPositions.at(0).sName=="GetProcessWindowStation")&&
                            (pPEInfo->listImports.at(nImportCount-1).listPositions.at(1).sName=="GetUserObjectInformationW"))
                        {
                            stDetects.insert("user32_3");
                        }
                    }
                }
            }

            if( stDetects.contains("kernel32_3")&&
                stDetects.contains("user32_3"))
            {
                SpecAbstract::_SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_VMPROTECT,"","",0);
                ss.sVersion="3.X";
                pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                bSuccess=true;
            }

            // Import
            if(!bSuccess)
            {
                bSuccess=QPE::isSectionNamePresent(".vmp0",&(pPEInfo->listSectionHeaders));
            }

            if(!bSuccess)
            {
                if(pPEInfo->nEntryPointSection>=3)
                {
                    for(int i=0;i<pPEInfo->listSectionHeaders.count();i++)
                    {
                        if(     (i==pPEInfo->nEntryPointSection)||
                                (i==pPEInfo->nResourceSection)||
                                (i==pPEInfo->nTLSSection)||
                                (i==pPEInfo->nRelocsSection)||
                                (QString((char *)pPEInfo->listSectionHeaders.at(i).Name)==".tls")
                                )
                        {
                            continue;
                        }
                        if(pPEInfo->listSectionHeaders.at(i).SizeOfRawData)
                        {
                            bSuccess=false;
                            break;
                        }
                    }
                }
            }

            if(bSuccess)
            {
                if(     pe.compareEntryPoint("68........E8")||
                        pe.compareEntryPoint("68........E9")||
                        pe.compareEntryPoint("EB$$E9$$$$$$$$68........E8"))
                {
                    // TODO more checks
                    SpecAbstract::_SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_VMPROTECT,"","",0);
                    pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }
        }
    }
}

void SpecAbstract::PE_handle_Armadillo(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            if((pPEInfo->nMajorLinkerVersion==0x53)&&(pPEInfo->nMinorLinkerVersion==0x52))
            {
                SpecAbstract::_SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ARMADILLO,"","",0);

                VI_STRUCT vi=PE_get_Armadillo_vi(pDevice,pPEInfo);

                ss.sVersion=vi.sVersion;
                ss.sInfo=vi.sInfo;

                pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }
}

void SpecAbstract::PE_handle_Petite(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            if(!pPEInfo->bIs64)
            {
                bool bKernel32=false;
                bool bUser32=false;
                QString sVersion;
                for(int i=0;i<pPEInfo->listImports.count();i++)
                {
                    if(pPEInfo->listImports.at(i).sName.toUpper()=="USER32.DLL")
                    {
                        if(pPEInfo->listImports.at(i).listPositions.count()==2)
                        {
                            if( (pPEInfo->listImports.at(i).listPositions.at(0).sName=="MessageBoxA")&&
                                (pPEInfo->listImports.at(i).listPositions.at(1).sName=="wsprintfA"))
                            {
                                bUser32=true;
                            }
                        }
                        else if(pPEInfo->listImports.at(i).listPositions.count()==1)
                        {
                            if( (pPEInfo->listImports.at(i).listPositions.at(0).sName=="MessageBoxA"))
                            {
                                bUser32=true;
                            }
                        }
                    }
                    else if(pPEInfo->listImports.at(i).sName.toUpper()=="KERNEL32.DLL")
                    {
                        if(pPEInfo->listImports.at(i).listPositions.count()==7)
                        {
                            if(     (pPEInfo->listImports.at(i).listPositions.at(0).sName=="ExitProcess")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="GetModuleHandleA")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="GetProcAddress")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(3).sName=="VirtualProtect")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(4).sName=="VirtualAlloc")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(5).sName=="VirtualFree")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(6).sName=="LoadLibraryA"))
                            {
                                sVersion="2.4";
                                bKernel32=true;
                            }
                            else if((pPEInfo->listImports.at(i).listPositions.at(0).sName=="ExitProcess")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="LoadLibraryA")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="GetProcAddress")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(3).sName=="VirtualProtect")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(4).sName=="GlobalAlloc")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(5).sName=="GlobalFree")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(6).sName=="GetModuleHandleA"))
                            {
                                sVersion="2.3";
                                bKernel32=true;
                            }
                        }
                        else if(pPEInfo->listImports.at(i).listPositions.count()==5)
                        {
                            if(     (pPEInfo->listImports.at(i).listPositions.at(0).sName=="ExitProcess")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="LoadLibraryA")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="GetProcAddress")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(3).sName=="VirtualProtect")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(4).sName=="GlobalAlloc"))
                            {
                                sVersion="2.2";
                                bKernel32=true;
                            }
                        }
                        else if(pPEInfo->listImports.at(i).listPositions.count()==4)
                        {
                            if(     (pPEInfo->listImports.at(i).listPositions.at(0).sName=="ExitProcess")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(1).sName=="GetProcAddress")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(2).sName=="LoadLibraryA")&&
                                    (pPEInfo->listImports.at(i).listPositions.at(3).sName=="GlobalAlloc"))
                            {
                                sVersion="1.4";
                                bKernel32=true;
                            }
                        }
                    }
                }

                if(bUser32&&bKernel32)
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PETITE))
                    {
                        SpecAbstract::_SCANS_STRUCT recordPETITE=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PETITE);
                        recordPETITE.sVersion=sVersion;
                        pPEInfo->mapResultPackers.insert(recordPETITE.name,scansToScan(&(pPEInfo->basic_info),&recordPETITE));
                    }
                }
                else if(QPE::isSectionNamePresent(".petite",&(pPEInfo->listSectionHeaders)))
                {
                    if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_PETITE))
                    {
                        SpecAbstract::_SCANS_STRUCT recordPETITE=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_PETITE);
                        pPEInfo->mapResultPackers.insert(recordPETITE.name,scansToScan(&(pPEInfo->basic_info),&recordPETITE));
                    }
                }
            }
        }
    }
}

void SpecAbstract::PE_handle_NETProtection(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        if(pPEInfo->cliInfo.bInit)
        {
            // .NET
            // Enigma
            if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                qint64 nSectionOffset=pPEInfo->nCodeSectionOffset;
                qint64 nSectionSize=pPEInfo->nCodeSectionSize;

                QString sEnigmaVersion=findEnigmaVersion(pDevice,nSectionOffset,nSectionSize);

                if(sEnigmaVersion!="")
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_ENIGMA,sEnigmaVersion,".NET",0);
                    pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // TODO
            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_YANO))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_YANO);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_DOTFUSCATOR))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_DOTFUSCATOR);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_AGILENET))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_AGILENET);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

//            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_SKATERNET))
//            {
//                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_SKATERNET);
//                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
//            }

            if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                qint64 _nSize=pPEInfo->nCodeSectionSize;

                qint64 nOffset_String=pe.find_ansiString(_nOffset,_nSize,"RustemSoft.Skater");

                if(nOffset_String!=-1)
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_NETOBFUSCATOR,RECORD_NAME_SKATER,"","",0);
                    pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_BABELNET))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_BABELNET);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_GOLIATHNET))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_GOLIATHNET);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_SPICESNET))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_SPICESNET);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_OBFUSCATORNET2009))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_OBFUSCATORNET2009);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_DEEPSEA))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_DEEPSEA);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }


            // cliSecure
            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_CLISECURE))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_CLISECURE);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
            else
            {
                if(pPEInfo->listSectionHeaders.count()>=2)
                {
                    qint64 _nOffset=pPEInfo->listSectionRecords.at(1).nOffset;
                    qint64 _nSize=pPEInfo->listSectionRecords.at(1).nSize;
                    qint32 _nCharacteristics=pPEInfo->listSectionRecords.at(1).nCharacteristics;

                    if(_nCharacteristics&S_IMAGE_SCN_MEM_EXECUTE)
                    {
                        qint64 nOffset_CliSecure=pe.find_unicodeString(_nOffset,_nSize,"CliSecure");

                        if(nOffset_CliSecure!=-1)
                        {
                            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_NETOBFUSCATOR,RECORD_NAME_CLISECURE,"4.X","",0);
                            pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }
                }
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_FISHNET))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_PROTECTOR,RECORD_NAME_FISHNET,"1.X","",0); // TODO
                pPEInfo->mapResultProtectors.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss)); // TODO obfuscator?
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_NSPACK))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_NSPACK);
                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_DNGUARD))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_DNGUARD);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_DOTNETZ))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_DOTNETZ);
                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_MAXTOCODE))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_MAXTOCODE);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_PHOENIXPROTECTOR))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_PHOENIXPROTECTOR);
                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_SIXXPACK))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_SIXXPACK);
                pPEInfo->mapResultPackers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_SMARTASSEMBLY))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_SMARTASSEMBLY);

                qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                qint64 _nSize=pPEInfo->nCodeSectionSize;

                qint64 nOffset_Confuser=pe.find_ansiString(_nOffset,_nSize,"Powered by SmartAssembly ");

                if(nOffset_Confuser!=-1)
                {
                    ss.sVersion=pe.read_ansiString(nOffset_Confuser+25);
                }

                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_CONFUSER))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_CONFUSER);

                qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                qint64 _nSize=pPEInfo->nCodeSectionSize;

                qint64 nOffset_Confuser=pe.find_ansiString(_nOffset,_nSize,"Confuser v");

                if(nOffset_Confuser!=-1)
                {
                    ss.sVersion=pe.read_ansiString(nOffset_Confuser+10);
                }

                if(nOffset_Confuser==-1)
                {
                    qint64 nOffset_ConfuserEx=pe.find_ansiString(_nOffset,_nSize,"ConfuserEx v");

                    if(nOffset_ConfuserEx!=-1)
                    {
                        ss.name=RECORD_NAME_CONFUSEREX;
                        ss.sVersion=pe.read_ansiString(nOffset_ConfuserEx+12);
                    }
                }

                pPEInfo->mapResultNETObfuscators.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }
}

void SpecAbstract::PE_handle_libraries(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);

    if(!pPEInfo->cliInfo.bInit)
    {
        // Qt
        // mb TODO upper
        if(QPE::isImportLibraryPresentI("QtCore4.dll",&(pPEInfo->listImports)))
        {
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"4.X","",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("QtCored4.dll",&(pPEInfo->listImports)))
        {
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"4.X","Debug",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("Qt5Core.dll",&(pPEInfo->listImports)))
        {
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"5.X","",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
        else if(QPE::isImportLibraryPresentI("Qt5Cored.dll",&(pPEInfo->listImports)))
        {
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_QT,"5.X","Debug",0);
            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }
    }
}

void SpecAbstract::PE_handle_Microsoft(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    SpecAbstract::_SCANS_STRUCT recordLinker={};
    SpecAbstract::_SCANS_STRUCT recordCompiler={};
    SpecAbstract::_SCANS_STRUCT recordTool={};
    SpecAbstract::_SCANS_STRUCT recordMFC={};
    SpecAbstract::_SCANS_STRUCT recordNET={};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        // Linker
        if((pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKER))&&(!pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER)))
        {
            recordLinker.type=RECORD_TYPE_LINKER;
            recordLinker.name=RECORD_NAME_MICROSOFTLINKER;
        }
        else if((pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKER))&&(pPEInfo->cliInfo.bInit))
        {
            recordLinker.type=RECORD_TYPE_LINKER;
            recordLinker.name=RECORD_NAME_MICROSOFTLINKER;
        }

        if(!pPEInfo->cliInfo.bInit)
        {
            // MFC
            // Static
            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
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
                // https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library
                // TODO eMbedded Visual C++ 4.0 		mfcce400.dll 	MFC 6.0
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^MFC")))
                {
//                    QRegularExpression rxVersion("(\\d+)");
//                    QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());
//
//                    if(matchVersion.hasMatch())
//                    {
//                        double dVersion=matchVersion.captured(0).toDouble()/10;
//
//                        if(dVersion)
//                        {
//                            recordMFC.type=RECORD_TYPE_LIBRARY;
//                            recordMFC.name=RECORD_NAME_MFC;
//                            recordMFC.sVersion=QString::number(dVersion,'f',2);
//
//                            if(pPEInfo->listImports.at(i).sName.toUpper().contains("U.DLL"))
//                            {
//                                recordMFC.sInfo="Unicode";
//                            }
//                        }
//                    }

                    QString sVersion=QBinary::regExp("(\\d+)",pPEInfo->listImports.at(i).sName.toUpper(),0);

                    if(sVersion!="")
                    {
                        double dVersion=sVersion.toDouble()/10;
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
                if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
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

            if(pPEInfo->cliInfo.bHidden)
            {
                recordNET.sInfo="Hidden";
            }

            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_VBNET))
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

            SpecAbstract::_SCANS_STRUCT ssLinker={};

            SpecAbstract::_SCANS_STRUCT ssCompiler={};
            SpecAbstract::_SCANS_STRUCT _ssCompiler1={};
            SpecAbstract::_SCANS_STRUCT _ssCompiler2={};

            for(int i=1; i<=5; i++)
            {
                if(nRichSignaturesCount>=i)
                {
                    quint32 _nRich=(pPEInfo->listRichSignatures.at(nRichSignaturesCount-i).nId<<16)+pPEInfo->listRichSignatures.at(nRichSignaturesCount-i).nVersion;
                    SpecAbstract::_SCANS_STRUCT ssRich=SpecAbstract::PE_getRichSignatureDescription(_nRich);

                    if((ssLinker.type!=SpecAbstract::RECORD_TYPE_LINKER)&&(ssRich.type==SpecAbstract::RECORD_TYPE_LINKER))
                    {
                        ssLinker=ssRich;
                    }

                    if((_ssCompiler1.type!=SpecAbstract::RECORD_TYPE_COMPILER)&&(ssRich.type==SpecAbstract::RECORD_TYPE_COMPILER))
                    {
                        _ssCompiler1=ssRich;
                    }
                    else if((_ssCompiler2.type!=SpecAbstract::RECORD_TYPE_COMPILER)&&(ssRich.type==SpecAbstract::RECORD_TYPE_COMPILER))
                    {
                        _ssCompiler2=ssRich;
                    }
                }
            }

            ssCompiler=_ssCompiler1;

            if(QPE::isImportLibraryPresentI("MSVCRT.dll",&(pPEInfo->listImports)))
            {
                if(_ssCompiler2.name==SpecAbstract::RECORD_NAME_VISUALCCPP)
                {
                    ssCompiler=_ssCompiler2;
                }
            }

            if(ssLinker.type==SpecAbstract::RECORD_TYPE_LINKER)
            {
                recordLinker.sVersion=ssLinker.sVersion;
                recordLinker.sInfo=ssLinker.sInfo;
            }

            if(ssCompiler.type==SpecAbstract::RECORD_TYPE_COMPILER)
            {
                recordCompiler.type=ssCompiler.type;
                recordCompiler.name=ssCompiler.name;
                recordCompiler.sVersion=ssCompiler.sVersion;
                recordCompiler.sInfo=ssCompiler.sInfo;

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

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordCompiler.name!=RECORD_NAME_VISUALCCPP))
        {
            recordCompiler.type=SpecAbstract::RECORD_TYPE_COMPILER;
            recordCompiler.name=SpecAbstract::RECORD_NAME_VISUALCCPP;

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
            if((recordCompiler.name==RECORD_NAME_VISUALCCPP)&&(recordLinker.sVersion!=""))
            {
                recordMFC.sVersion=recordLinker.sVersion.section(".",0,1);
            }
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordLinker.name!=RECORD_NAME_MICROSOFTLINKER))
        {
            recordLinker.type=SpecAbstract::RECORD_TYPE_LINKER;
            recordLinker.name=SpecAbstract::RECORD_NAME_MICROSOFTLINKER;
        }

        if((recordMFC.name==RECORD_NAME_MFC)&&(recordLinker.sVersion=="")&&(pPEInfo->nMinorLinkerVersion!=10))
        {
            recordLinker.sVersion=recordMFC.sVersion;
            //            recordLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);;
        }


        if(recordCompiler.name==SpecAbstract::RECORD_NAME_VISUALCCPP)
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
            else if((sLinkerVersion=="14.10.25834")&&(sCompilerVersion=="19.10.25834"))
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

void SpecAbstract::PE_handle_Borland(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    // TODO Turbo Linker
    QPE pe(pDevice);
    //

    if(pe.isValid())
    {
        if(pPEInfo->basic_info.mapHeaderDetects.contains(SpecAbstract::RECORD_NAME_TURBOLINKER))
        {
            _SCANS_STRUCT recordTurboLinker=pPEInfo->basic_info.mapHeaderDetects.value(SpecAbstract::RECORD_NAME_TURBOLINKER);

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

            if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
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

                        listVCL=PE_getVCLstruct(pDevice,_nOffset,_nSize,pPEInfo->bIs64);
                    }
                }



                //            nOffset_AnsiString=pe.find_array(_nOffset,_nSize,"\x0a\x41\x6e\x73\x69\x53\x74\x72\x69\x6e\x67",11); // AnsiString
                //            nOffset_WideString=pe.find_array(_nOffset,_nSize,"\x0a\x57\x69\x64\x65\x53\x74\x72\x69\x6e\x67",11); // WideString
            }

            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
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

            bool bPackageinfo=QPE::isResourcePresent(S_RT_RCDATA,"PACKAGEINFO",&(pPEInfo->listResources));
            bool bDvcal=QPE::isResourcePresent(S_RT_RCDATA,"DVCLAL",&(pPEInfo->listResources));

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
                    VCL_PACKAGEINFO pi=PE_getVCLPackageInfo(pDevice,&pPEInfo->listResources);

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

                _SCANS_STRUCT recordCompiler;
                recordCompiler.type=RECORD_TYPE_COMPILER;

                _SCANS_STRUCT recordTool;
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
                    _SCANS_STRUCT recordVCL;
                    recordVCL.type=RECORD_TYPE_LIBRARY;
                    recordVCL.name=RECORD_NAME_VCL;
                    recordVCL.sVersion=sVCLVersion;

                    pPEInfo->mapResultTools.insert(recordVCL.name,scansToScan(&(pPEInfo->basic_info),&recordVCL));
                }

                if(!pPEInfo->mapResultLinkers.contains(RECORD_NAME_TURBOLINKER))
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LINKER,RECORD_NAME_TURBOLINKER,"","",0);
                    pPEInfo->mapResultLinkers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }
        }
        else
        {
            // .NET TODO: Check!!!!
            if(pPEInfo->mapDotAnsistringsDetects.contains(RECORD_NAME_EMBARCADERODELPHIDOTNET))
            {
                _SCANS_STRUCT ss=pPEInfo->mapDotAnsistringsDetects.value(RECORD_NAME_EMBARCADERODELPHIDOTNET);
                pPEInfo->mapResultTools.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }

}

void SpecAbstract::PE_handle_Tools(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);

    if(pe.isValid())
    {
        // FASM
        if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_FASM))
        {
            // TODO correct Version
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FASM,"","",0);
            ss.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
            pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
        }

        // GoLink, GoAsm
        if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GOLINK))
        {
            _SCANS_STRUCT ssLinker=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LINKER,RECORD_NAME_GOLINK,"","",0);
            ssLinker.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
            pPEInfo->mapResultLinkers.insert(ssLinker.name,scansToScan(&(pPEInfo->basic_info),&ssLinker));

            _SCANS_STRUCT ssCompiler=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GOASM,"","",0);
            pPEInfo->mapResultCompilers.insert(ssCompiler.name,scansToScan(&(pPEInfo->basic_info),&ssCompiler));
        }

        if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_LAYHEYFORTRAN90))
        {
            QString sLFString=pe.read_ansiString(0x200);
            if(sLFString=="This program must be run under Windows 95, NT, or Win32s\r\nPress any key to exit.$")
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_LAYHEYFORTRAN90,"","",0);
                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }

        // Flex
        if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
        {
            qint64 _nOffset=pPEInfo->nDataSectionOffset;
            qint64 _nSize=pPEInfo->nDataSectionSize;
            // TODO FPC Version in Major and Minor linker

            qint64 nOffset_FlexLM=pe.find_ansiString(_nOffset,_nSize,"@(#) FLEXlm ");

            if(nOffset_FlexLM!=-1)
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_FLEXLM,"","",0);

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
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_FLEXNET,"","",0);

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
            bool bDetectGCC=false;
            bool bHeurGCC=false;

            if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER))
            {
                switch(pPEInfo->nMajorLinkerVersion)
                {
                case 2:
                    switch(pPEInfo->nMinorLinkerVersion)
                    {
                    case 24:
                    case 25:
                    case 56:
                        bHeurGCC=true;
                        break;
                    }
                    break;
                }
            }

            QString sDllLib;

            if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                sDllLib=pe.read_ansiString(pPEInfo->nConstDataSectionOffset);
            }

            if(QPE::isImportLibraryPresentI("msys-1.0.dll",&(pPEInfo->listImports)))
            {
                // Msys 1.0
                _SCANS_STRUCT ssGCC=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);
                pPEInfo->mapResultCompilers.insert(ssGCC.name,scansToScan(&(pPEInfo->basic_info),&ssGCC));
                _SCANS_STRUCT ssMsys=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_MSYS,"1.0","",0);
                pPEInfo->mapResultTools.insert(ssMsys.name,scansToScan(&(pPEInfo->basic_info),&ssMsys));
            }

            if(     (sDllLib.contains("gcc"))||
                    (sDllLib.contains("libgcj"))||
                    (sDllLib=="_set_invalid_parameter_handler")||
                    QPE::isImportLibraryPresentI("libgcc_s_dw2-1.dll",&(pPEInfo->listImports))||
                    pPEInfo->mapOverlayDetects.contains(RECORD_NAME_MINGW))
            {
                bDetectGCC=true;
            }

            if(bDetectGCC||bHeurGCC)
            {
                // Mingw
                // Msys
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);

                if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                {
                    qint64 _nOffset=pPEInfo->nConstDataSectionOffset;
                    qint64 _nSize=pPEInfo->nConstDataSectionSize;

                    // TODO get max version
                    qint64 nOffset_Version=pe.find_ansiString(_nOffset,_nSize,"GCC:");

                    if(nOffset_Version!=-1)
                    {
                        bDetectGCC=true;

                        QString sVersionString=pe.read_ansiString(nOffset_Version);

                        // TODO MinGW-w64
                        if(sVersionString.contains("MinGW")||sVersionString.contains("GNU"))
                        {
                            _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                            pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
                        }
                        else if(sVersionString.contains("MSYS2"))
                        {
                            _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MSYS2,"","",0);
                            pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
                        }

                        // TODO function
                        if( (sVersionString.contains("(experimental)"))||
                            (sVersionString.contains("(prerelease)")))
                        {
                            ss.sVersion=sVersionString.section(" ",-3,-1);
                        }
                        else if(sVersionString.contains("GNU"))
                        {
                            ss.sVersion=sVersionString.section(" ",-2,-1);
                        }
                        else
                        {
                            ss.sVersion=sVersionString.section(" ",-1,-1);
                        }
                    }
                    else
                    {
                        // Cygwin
                        // MinGW ???
                        ss.sVersion=PE_get_GCC_vi(pDevice,_nOffset,_nSize).sVersion;
                    }
                }

                if(bDetectGCC)
                {
                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            for(int i=0; i<pPEInfo->listImports.count(); i++)
            {
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^CYGWIN")))
                {
//                    QRegularExpression rxVersion("(\\d+)");
//                    QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->listImports.at(i).sName.toUpper());

//                    if(matchVersion.hasMatch())
//                    {
//                        double dVersion=matchVersion.captured(0).toDouble();
//
//                        if(dVersion)
//                        {
//                            // TODO
//                        }
//                    }


                    QString sVersion=QBinary::regExp("(\\d+)",pPEInfo->listImports.at(i).sName.toUpper(),0);

                    if(sVersion!="")
                    {
                        double dVersion=sVersion.toDouble();

                        if(dVersion)
                        {
                            // TODO
                        }
                    }

                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_CYGWIN,"","",0);

                    // TODO version
                    pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                    if(!pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC))
                    {
                        _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);

                        pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }

                    break;
                }
            }

            if(!pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC))
            {
                if(QPE::isSectionNamePresent(".stabstr",&(pPEInfo->listSectionHeaders)))
                {
                    S_IMAGE_SECTION_HEADER sh=QPE::getSectionByName(".stabstr",&(pPEInfo->listSectionHeaders));

                    if(sh.SizeOfRawData)
                    {
                        qint64 _nOffset=sh.PointerToRawData;
                        qint64 _nSize=sh.SizeOfRawData;

                        bool bSuccess=false;

                        if(!bSuccess)
                        {
                            qint64 nGCC_MinGW=pe.find_ansiString(_nOffset,_nSize,"/gcc/mingw32/");

                            if(nGCC_MinGW!=-1)
                            {
                                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);
                                ss.sVersion=pe.read_ansiString(nGCC_MinGW+13).section("/",0,0);
                                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                                _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                                pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));

                                bSuccess=true;
                            }
                        }

                        if(!bSuccess)
                        {
                            qint64 nCygwin=pe.find_ansiString(_nOffset,_nSize,"/gcc/i686-pc-cygwin/");

                            if(nCygwin!=-1)
                            {
                                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);
                                ss.sVersion=pe.read_ansiString(nCygwin+20).section("/",0,0);
                                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                                _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_CYGWIN,"","",0);
                                pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));

                                bSuccess=true;
                            }
                        }
                    }
                }
            }

            if(!pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC)) // TODO Check
            {
                if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_GCC))
                {
                    _SCANS_STRUCT _dss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_GCC);
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_GCC,"","",0);
                    ss.sVersion=_dss.sVersion;

                    QString _sGCCVersion;

                    if((pPEInfo->nConstDataSectionOffset)&&(pPEInfo->nConstDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                    {
                        _sGCCVersion=PE_get_GCC_vi(pDevice,pPEInfo->nConstDataSectionOffset,pPEInfo->nConstDataSectionSize).sVersion;

                        if(_sGCCVersion!="")
                        {
                            ss.sVersion=_sGCCVersion;
                        }
                    }

                    if(_sGCCVersion=="")
                    {
                        if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                        {
                            _sGCCVersion=PE_get_GCC_vi(pDevice,pPEInfo->nDataSectionOffset,pPEInfo->nDataSectionSize).sVersion;

                            if(_sGCCVersion!="")
                            {
                                ss.sVersion=_sGCCVersion;
                            }
                        }
                    }

                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                    if(_dss.sInfo=="MinGW") // mb TODO variant
                    {
                        _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                        pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
                    }
                }
            }

            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                qint64 _nOffset=pPEInfo->nDataSectionOffset;
                qint64 _nSize=pPEInfo->nDataSectionSize;
                // TODO FPC Version in Major and Minor linker

                qint64 nOffset_FPC=pe.find_ansiString(_nOffset,_nSize,"FPC ");

                if(nOffset_FPC!=-1)
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FPC,"","",0);
                    QString sFPCVersion=pe.read_ansiString(nOffset_FPC);
                    ss.sVersion=sFPCVersion.section(" ",1,-1).section(" - ",0,0);

                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                    // Lazarus
                    qint64 nOffset_Lazarus=pe.find_ansiString(_nOffset,_nSize,"Lazarus LCL: ");

                    if(nOffset_Lazarus!=-1)
                    {
                        _SCANS_STRUCT ssLazarus=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_LAZARUS,"","",0);
                        QString sLazarusVersion=pe.read_ansiString(nOffset_Lazarus+13);
                        ssLazarus.sVersion=sLazarusVersion.section(" ",0,0);

                        pPEInfo->mapResultTools.insert(ssLazarus.name,scansToScan(&(pPEInfo->basic_info),&ssLazarus));
                    }
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
                    qint64 nOffset_RunTimeError=pe.find_array(_nOffset,_nSize,"\x0e\x52\x75\x6e\x74\x69\x6d\x65\x20\x65\x72\x72\x6f\x72\x20",15); // Runtime Error TODO: use findAnsiString

                    if(nOffset_RunTimeError!=-1)
                    {

                        _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_FPC,"","",0);

                        // TODO Version
                        pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                    }
                }
            }

            for(int i=0; i<pPEInfo->listImports.count(); i++)
            {
                if(pPEInfo->listImports.at(i).sName.toUpper().contains(QRegExp("^PYTHON")))
                {
                    QString sVersion=QBinary::regExp("(\\d+)",pPEInfo->listImports.at(i).sName.toUpper(),0);

                    if(sVersion!="")
                    {
                        double dVersion=sVersion.toDouble();
                        if(dVersion)
                        {
                            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_PYTHON,"","",0);

                            ss.sVersion=QString::number(dVersion,'f',1);
                            pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                        }
                    }

                    break;
                }
            }

            // Virtual Pascal
            if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                qint64 _nOffset=pPEInfo->nDataSectionOffset;
                qint64 _nSize=pPEInfo->nDataSectionSize;
                // TODO VP Version in Major and Minor linker

                qint64 nOffset_VP=pe.find_ansiString(_nOffset,_nSize,"Virtual Pascal - Copyright (C) "); // "Virtual Pascal - Copyright (C) 1996-2000 vpascal.com"

                if(nOffset_VP!=-1)
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_VIRTUALPASCAL,"","",0);

                    // TODO Version???
                    ss.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // PowerBASIC
            if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
            {
                qint64 _nOffset=pPEInfo->nCodeSectionOffset;
                qint64 _nSize=pPEInfo->nCodeSectionSize;
                // TODO VP Version in Major and Minor linker

                qint64 nOffset_PB=pe.find_ansiString(_nOffset,_nSize,"PowerBASIC");

                if(nOffset_PB!=-1)
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_COMPILER,RECORD_NAME_POWERBASIC,"","",0);

                    // TODO Version???
                    pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // Watcom linker
            if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_WATCOMLINKER))
            {
                _SCANS_STRUCT ss=pPEInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_WATCOMLINKER);

                pPEInfo->mapResultLinkers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // Watcom CPP
            if(pPEInfo->mapEntryPointDetects.contains(RECORD_NAME_WATCOMCCPP))
            {
                _SCANS_STRUCT ss=pPEInfo->mapEntryPointDetects.value(RECORD_NAME_WATCOMCCPP);

                // TODO Version???
                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // wxWidgets
            if(QPE::isResourcePresent(S_RT_MENU,"WXWINDOWMENU",&(pPEInfo->listResources)))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LIBRARY,RECORD_NAME_WXWIDGETS,"","",0);
                // TODO Version???
                ss.sInfo="Static";
                pPEInfo->mapResultLibraries.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }

        if(pPEInfo->mapResultCompilers.contains(RECORD_NAME_GCC))
        {
            if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER))
            {
                // TODO Check version;
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_LINKER,RECORD_NAME_GNULINKER,"","",0);
                ss.sVersion=QString("%1.%2").arg(pPEInfo->nMajorLinkerVersion).arg(pPEInfo->nMinorLinkerVersion);
                pPEInfo->mapResultCompilers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if((!pPEInfo->mapResultTools.contains(RECORD_NAME_MINGW))&&
                    (!pPEInfo->mapResultTools.contains(RECORD_NAME_MSYS))&&
                    (!pPEInfo->mapResultTools.contains(RECORD_NAME_MSYS2))&&
                    (!pPEInfo->mapResultTools.contains(RECORD_NAME_CYGWIN)))
            {
                _SCANS_STRUCT ssTool=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_TOOL,RECORD_NAME_MINGW,"","",0);
                pPEInfo->mapResultTools.insert(ssTool.name,scansToScan(&(pPEInfo->basic_info),&ssTool));
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_MINGW)&&pPEInfo->mapResultTools.contains(RECORD_NAME_MINGW))
            {
                updateInfo(&pPEInfo->mapResultTools,RECORD_NAME_MINGW,"Debug");
            }
        }
    }
}

void SpecAbstract::PE_handle_Installers(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);
    //

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            // Inno Setup
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_INNOSETUP)||pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_INNOSETUP))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_INNOSETUP,"","",0);

                if((pe.read_uint32(0x30)==0x6E556E49)) // Uninstall
                {
                    ss.sInfo="Uninstall";

                    if((pPEInfo->nCodeSectionOffset)&&(pPEInfo->nCodeSectionSize)&&(pPEInfo->basic_info.bDeepScan))
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
                else if(pPEInfo->mapOverlayDetects.value(RECORD_NAME_INNOSETUP).sInfo=="Uninstall")
                {
                    ss.sInfo="Uninstall";
                    qint64 _nOffset=pPEInfo->nOverlayOffset;
                    qint64 _nSize=pPEInfo->nOverlaySize;

                    qint64 nOffsetVersion=pe.find_ansiString(_nOffset,_nSize,"Inno Setup Messages (");

                    if(nOffsetVersion!=-1)
                    {
                        QString sVersionString=pe.read_ansiString(nOffsetVersion+21);
                        ss.sVersion=sVersionString.section(" ",0,0);
                        ss.sVersion=ss.sVersion.remove(")");
                        QString sEncodes=sVersionString.section(" ",1,1);

                        // TODO Check
                        if(sEncodes=="(a))")
                        {
                            ss.sInfo=append(ss.sInfo,"ANSI");
                        }
                        else if(sEncodes=="(u))")
                        {
                            ss.sInfo=append(ss.sInfo,"Unicode");
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
                        QPE::RESOURCE_RECORD resHeader=QPE::getResourceRecord(S_RT_RCDATA,11111,&(pPEInfo->listResources));

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

//                            QRegularExpression rxVersion("\\((.*?)\\)");
//                            QRegularExpression rxOptions("\\) \\((.*?)\\)");
//                            QRegularExpressionMatch matchVersion=rxVersion.match(sSetupDataString);
//                            QRegularExpressionMatch matchOptions=rxOptions.match(sSetupDataString);

//                            if(matchVersion.hasMatch())
//                            {
//                                ss.sVersion=matchVersion.captured(1);
//                            }

//                            if(matchOptions.hasMatch())
//                            {
//                                QString sEncode=matchOptions.captured(1);

//                                if(sEncode=="a")
//                                {
//                                    ss.sInfo=append(ss.sInfo,"ANSI");
//                                }
//                                else if(sEncode=="u")
//                                {
//                                    ss.sInfo=append(ss.sInfo,"Unicode");
//                                }
//                            }

                            QString sVersion=QBinary::regExp("\\((.*?)\\)",sSetupDataString,1);
                            QString sOptions=QBinary::regExp("\\) \\((.*?)\\)",sSetupDataString,1);

                            if(sVersion!="")
                            {
                                ss.sVersion=sVersion;
                            }

                            if(sOptions!="")
                            {
                                QString sEncode=sOptions;

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
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_CAB))
            {
                // Wix Tools
                if(QPE::isSectionNamePresent(".wixburn",&(pPEInfo->listSectionHeaders)))
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_WIXTOOLSET,"","",0);
                    ss.sVersion="3.X"; // TODO check "E:\delivery\Dev\wix37\build\ship\x86\burn.pdb"
                    pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }
            // Install Anywhere
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_INSTALLANYWHERE))
            {
                if(QPE::getResourceVersionValue("ProductName",&(pPEInfo->resVersion))=="InstallAnywhere")
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_INSTALLANYWHERE,"","",0);
                    ss.sVersion=QPE::getResourceVersionValue("ProductVersion",&(pPEInfo->resVersion));
                    pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_GHOSTINSTALLER))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_GHOSTINSTALLER,"","",0);
                ss.sVersion="1.0";
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_QTINSTALLER))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_QTINSTALLER,"","",0);
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_SMARTINSTALLMAKER))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_SMARTINSTALLMAKER,"","",0);
                ss.sVersion=QBinary::hexToString(pPEInfo->sOverlaySignature.mid(46,14)); // TODO make 1 function
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_TARMAINSTALLER))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_TARMAINSTALLER,"","",0);
                // TODO version
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_CLICKTEAM))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_CLICKTEAM,"","",0);
                // TODO version
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // NSIS
            if((pPEInfo->mapOverlayDetects.contains(RECORD_NAME_NSIS))||(pPEInfo->sResourceManifest.contains("Nullsoft.NSIS")))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_NSIS,"","",0);

                QString _sInfo=pPEInfo->mapOverlayDetects.value(RECORD_NAME_NSIS).sInfo;
                if(_sInfo!="")
                {
                    ss.sInfo=_sInfo;
                }

//                QRegularExpression rxVersion("Null[sS]oft Install System v?(.*?)<");
//                QRegularExpressionMatch matchVersion=rxVersion.match(pPEInfo->sResourceManifest);

//                if(matchVersion.hasMatch())
//                {
//                    ss.sVersion=matchVersion.captured(1);
//                }

                QString sVersion=QBinary::regExp("Null[sS]oft Install System v?(.*?)<",pPEInfo->sResourceManifest,1);

                if(sVersion!="")
                {
                    ss.sVersion=sVersion;
                }
                // TODO options

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
            // InstallShield
            if(QPE::getResourceVersionValue("ProductName",&(pPEInfo->resVersion)).contains("InstallShield"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_INSTALLSHIELD,"","",0);
                ss.sVersion=QPE::getResourceVersionValue("FileVersion",&(pPEInfo->resVersion)).trimmed();
                ss.sVersion.replace(", ",".");
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
            else if(pPEInfo->sResourceManifest.contains("InstallShield"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_INSTALLSHIELD,"","",0);

                if((pPEInfo->nDataSectionOffset)&&(pPEInfo->nDataSectionSize)&&(pPEInfo->basic_info.bDeepScan))
                {
                    qint64 _nOffset=pPEInfo->nDataSectionOffset;
                    qint64 _nSize=pPEInfo->nDataSectionSize;

                    qint64 nOffsetVersion=pe.find_ansiString(_nOffset,_nSize,"SOFTWARE\\InstallShield\\1");

                    if(nOffsetVersion!=-1)
                    {
                        QString sVersionString=pe.read_ansiString(nOffsetVersion);
                        ss.sVersion=sVersionString.section("\\",2,2);
                    }
                }

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->sResourceManifest.contains("AdvancedInstallerSetup"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_ADVANCEDINSTALLER,"","",0);

                if((pPEInfo->nOverlayOffset)&&(pPEInfo->nOverlaySize)&&(pPEInfo->basic_info.bDeepScan))
                {
                    qint64 _nOffset=pPEInfo->nOverlayOffset;
                    qint64 _nSize=pPEInfo->nOverlaySize;

                    qint64 nOffsetVersion=pe.find_ansiString(_nOffset,_nSize,"Advanced Installer ");

                    if(nOffsetVersion!=-1)
                    {
                        QString sVersionString=pe.read_ansiString(nOffsetVersion);
                        ss.sVersion=sVersionString.section(" ",2,2);
                    }
                }

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->sResourceManifest.contains("Gentee.Installer.Install"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_GENTEEINSTALLER,"","",0);

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(pPEInfo->sResourceManifest.contains("BitRock Installer"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_BITROCKINSTALLER,"","",0);

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(     QPE::getResourceVersionValue("FileDescription",&(pPEInfo->resVersion)).contains("GP-Install")&&
                    QPE::getResourceVersionValue("FileDescription",&(pPEInfo->resVersion)).contains("TASPro6-Install"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_GPINSTALL,"","",0);
                ss.sVersion=QPE::getResourceVersionValue("FileVersion",&(pPEInfo->resVersion)).trimmed();
                ss.sVersion.replace(", ",".");
                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            if(QPE::getResourceVersionValue("Comments",&(pPEInfo->resVersion)).contains("Actual Installer"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_ACTUALINSTALLER,"","",0);
                ss.sVersion=QPE::getResourceVersionValue("FileVersion",&(pPEInfo->resVersion)).trimmed();

                pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // Windows Installer
            for(int i=0;i<pPEInfo->listResources.count();i++)
            {
                qint64 _nOffset=pPEInfo->listResources.at(i).nOffset;
                qint64 _nSize=pPEInfo->listResources.at(i).nSize;
                qint64 _nSignatureSize=qMin(_nSize,(qint64)8);

                if(_nSignatureSize)
                {
                    QString sSignature=pe.getSignature(_nOffset,_nSignatureSize);
                    if(sSignature=="D0CF11E0A1B11AE1")
                    {
                        qint64 nStringOffset=pe.find_ansiString(_nOffset,_nSize,"Windows Installer");
                        if(nStringOffset!=-1)
                        {
                            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_WINDOWSINSTALLER,"","",0);

                            QString _sString=pe.read_ansiString(nStringOffset);

                            if(_sString.contains("xml",Qt::CaseInsensitive))
                            {
                                ss.sInfo="XML";
                            }

//                            QRegularExpression rxVersion("\\((.*?)\\)");
//                            QRegularExpressionMatch matchVersion=rxVersion.match(_sString);

//                            if(matchVersion.hasMatch())
//                            {
//                                ss.sVersion=matchVersion.captured(1);
//                            }

                            QString sVersion=QBinary::regExp("\\((.*?)\\)",_sString,1);

                            if(sVersion!="")
                            {
                                ss.sVersion=sVersion;
                            }

                            pPEInfo->mapResultInstallers.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));

                            break;
                        }
                    }
                }
            }
        }
    }
}

void SpecAbstract::PE_handle_SFX(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    QPE pe(pDevice);
    //

    if(pe.isValid())
    {
        if(!pPEInfo->cliInfo.bInit)
        {
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_RAR))
            {
                if(QPE::isResourcePresent(S_RT_DIALOG,"STARTDLG",&(pPEInfo->listResources))&&
                        QPE::isResourcePresent(S_RT_DIALOG,"LICENSEDLG",&(pPEInfo->listResources)))
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_SFX,RECORD_NAME_WINRAR,"","",0);
                    // TODO Version
                    pPEInfo->mapResultSFX.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_WINRAR))
            {
                if(pPEInfo->sResourceManifest.contains("WinRAR"))
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_SFX,RECORD_NAME_WINRAR,"","",0);
                    // TODO Version
                    pPEInfo->mapResultSFX.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // 7z SFX
            if(QPE::getResourceVersionValue("ProductName",&(pPEInfo->resVersion)).contains("7-Zip"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_SFX,RECORD_NAME_7Z,"","",0);
                ss.sVersion=QPE::getResourceVersionValue("ProductVersion",&(pPEInfo->resVersion));
                pPEInfo->mapResultSFX.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }

            // SQUEEZ SFX
            if(pPEInfo->mapOverlayDetects.contains(RECORD_NAME_SQUEEZSFX))
            {
                if(QPE::getResourceVersionValue("ProductName",&(pPEInfo->resVersion)).contains("Squeez"))
                {
                    _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_INSTALLER,RECORD_NAME_SQUEEZSFX,"","",0);
                    ss.sVersion=QPE::getResourceVersionValue("FileVersion",&(pPEInfo->resVersion)).trimmed();
                    pPEInfo->mapResultSFX.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
                }
            }

            // WinACE
            if(     QPE::getResourceVersionValue("InternalName",&(pPEInfo->resVersion)).contains("WinACE")||
                    QPE::getResourceVersionValue("InternalName",&(pPEInfo->resVersion)).contains("WinAce")||
                    QPE::getResourceVersionValue("InternalName",&(pPEInfo->resVersion)).contains("UNACE"))
            {
                _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_PE,RECORD_TYPE_SFX,RECORD_NAME_WINACE,"","",0);
                ss.sVersion=QPE::getResourceVersionValue("ProductVersion",&(pPEInfo->resVersion));
                pPEInfo->mapResultSFX.insert(ss.name,scansToScan(&(pPEInfo->basic_info),&ss));
            }
        }
    }
}

void SpecAbstract::PE_handle_PolyMorph(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    // ExeSax

}

void SpecAbstract::PE_handle_UnknownProtection(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);
    if((pPEInfo->mapResultPackers.count()==0)&&(pPEInfo->mapResultProtectors.count()==0))
    {
        if(pPEInfo->listSectionRecords.count())
        {
            if(pPEInfo->listSectionRecords.at(0).nSize==0)
            {
                if(     pPEInfo->mapImportDetects.contains(RECORD_NAME_UPX)&&
                        (pPEInfo->mapImportDetects.value(RECORD_NAME_UPX).nVariant==0))
                {
                    SpecAbstract::_SCANS_STRUCT recordSS={};

                    recordSS.type=RECORD_TYPE_PACKER;
                    recordSS.name=RECORD_NAME_UNKNOWNUPXLIKE;

                    pPEInfo->mapResultPackers.insert(recordSS.name,scansToScan(&(pPEInfo->basic_info),&recordSS));
                }
            }
        }
    }
}

void SpecAbstract::Binary_handle_Texts(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    if((pBinaryInfo->bIsPlainText)||(pBinaryInfo->unicodeType!=QBinary::UNICODE_TYPE_NONE)||(pBinaryInfo->bIsUTF8))
    {
        if(pBinaryInfo->mapResultTexts.count()==0)
        {
            _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_TEXT,RECORD_TYPE_FORMAT,RECORD_NAME_PLAIN,"","",0);

            if(pBinaryInfo->unicodeType!=QBinary::UNICODE_TYPE_NONE)
            {
                ss.name=RECORD_NAME_UNICODE;

                if(pBinaryInfo->unicodeType==QBinary::UNICODE_TYPE_BE)
                {
                    ss.sVersion="Big Endian";
                }
                else if(pBinaryInfo->unicodeType==QBinary::UNICODE_TYPE_LE)
                {
                    ss.sVersion="Little Endian";
                }
            }
            else if(pBinaryInfo->bIsUTF8)
            {
                ss.name=RECORD_NAME_UTF8;
            }
            else if(pBinaryInfo->bIsPlainText)
            {
                ss.name=RECORD_NAME_PLAIN;
            }

            pBinaryInfo->mapResultTexts.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
        }
    }
}

void SpecAbstract::Binary_handle_Archives(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    // 7-Zip
    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_7Z))&&(pBinaryInfo->basic_info.nSize>=64))
    {
        // TODO more options
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_7Z);
        if(ss.type==RECORD_TYPE_ARCHIVE)
        {
            ss.sVersion=QString("%1.%2").arg(QBinary::hexToUint8(pBinaryInfo->basic_info.sHeaderSignature.mid(6*2,2))).arg(QBinary::hexToUint8(pBinaryInfo->basic_info.sHeaderSignature.mid(7*2,2)));
            pBinaryInfo->mapResultArchives.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
        }
    }
    // ZIP
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_ZIP))&&(pBinaryInfo->basic_info.nSize>=64)) // TODO min size
    {
        // TODO jar, docx ...
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_ZIP);
        quint8 nVersion=QBinary::hexToUint8(pBinaryInfo->basic_info.sHeaderSignature.mid(4*2,2));
        quint8 nFlags=QBinary::hexToUint8(pBinaryInfo->basic_info.sHeaderSignature.mid(6*2,2));

        ss.sVersion=QString("%1").arg((double)nVersion/10,0,'f',1);
        if(nFlags&0x1)
        {
            ss.sInfo="Encrypted";
        }
        // TODO files
        pBinaryInfo->mapResultArchives.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    // ZIP
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GZIP))&&(pBinaryInfo->basic_info.nSize>=9))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_GZIP);

        // TODO options
        // TODO files
        pBinaryInfo->mapResultArchives.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    // CAB
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_CAB))&&(pBinaryInfo->basic_info.nSize>=9))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_CAB);

        // TODO options
        // TODO files
        pBinaryInfo->mapResultArchives.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    // RAR
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_RAR))&&(pBinaryInfo->basic_info.nSize>=64))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_RAR);

        // TODO options
        // TODO files
        pBinaryInfo->mapResultArchives.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
}

void SpecAbstract::Binary_handle_Certificates(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    // Windows Authenticode Portable Executable Signature Format
    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_WINAUTH))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        quint32 nLength=QBinary::hexToUint32(pBinaryInfo->basic_info.sHeaderSignature.mid(0,8));

        if(nLength>=pBinaryInfo->basic_info.nSize)
        {
            _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_WINAUTH);
            pBinaryInfo->mapResultCertificates.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
        }
    }
}

void SpecAbstract::Binary_handle_DebugData(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    // MinGW debug data
    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MINGW))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_MINGW);
        pBinaryInfo->mapResultDebugData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
}

void SpecAbstract::Binary_handle_Formats(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    if(pBinaryInfo->basic_info.nSize==0)
    {
        _SCANS_STRUCT ss=getScansStruct(0,RECORD_FILETYPE_BINARY,RECORD_TYPE_FORMAT,RECORD_NAME_EMPTYFILE,"","",0);
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_PDF))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        // PDF
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_PDF);
        ss.sVersion=QBinary::hexToString(pBinaryInfo->basic_info.sHeaderSignature.mid(5*2,6));
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_PDB))&&(pBinaryInfo->basic_info.nSize>=32))
    {
        // PDB
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_PDB);
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKERDATABASE))&&(pBinaryInfo->basic_info.nSize>=32))
    {
        // Microsoft Linker Database
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_MICROSOFTLINKERDATABASE);
//        ss.sVersion=QString("%1.%2").arg(QBinary::hexToString(pBinaryInfo->basic_info.sHeaderSignature.mid(32*2,4))).arg(QBinary::hexToString(pBinaryInfo->basic_info.sHeaderSignature.mid(34*2,4)));
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_JPEG))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        // JPEG
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_JPEG);
        quint32 nMajor=pBinaryInfo->basic_info.sHeaderSignature.mid(11*2,2).toUInt(nullptr,16);
        quint32 nMinor=pBinaryInfo->basic_info.sHeaderSignature.mid(12*2,2).toUInt(nullptr,16);
        ss.sVersion=QString("%1.%2").arg(nMajor).arg(nMinor,2,10,QChar('0'));
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_WINDOWSICON))&&(pBinaryInfo->basic_info.nSize>=40))
    {
        // Windows Icon
        // TODO more information
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_WINDOWSICON);
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_PNG))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        // PNG
        // TODO resolution
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_PNG);
        pBinaryInfo->mapResultFormats.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
}

void SpecAbstract::Binary_handle_InstallerData(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    // Inno Setup
    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_INNOSETUP))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_INNOSETUP);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_INSTALLANYWHERE))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_INSTALLANYWHERE);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GHOSTINSTALLER))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_GHOSTINSTALLER);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_NSIS))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_NSIS);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_SMARTINSTALLMAKER))&&(pBinaryInfo->basic_info.nSize>=30))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_SMARTINSTALLMAKER);
        ss.sVersion=QBinary::hexToString(pBinaryInfo->basic_info.sHeaderSignature.mid(46,14));
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_TARMAINSTALLER))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_TARMAINSTALLER);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_CLICKTEAM))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_CLICKTEAM);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_QTINSTALLER))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_QTINSTALLER);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_ADVANCEDINSTALLER))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_ADVANCEDINSTALLER);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GPINSTALL))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_GPINSTALL);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_ACTUALINSTALLER))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_ACTUALINSTALLER);
        pBinaryInfo->mapResultInstallerData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
}

void SpecAbstract::Binary_handle_SFXData(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_WINRAR))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_WINRAR);
        pBinaryInfo->mapResultSFXData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_SQUEEZSFX))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_SQUEEZSFX);
        pBinaryInfo->mapResultSFXData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
    else if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_7Z))&&(pBinaryInfo->basic_info.nSize>=20))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_7Z);
        if(ss.type==RECORD_TYPE_SFXDATA)
        {
            pBinaryInfo->mapResultSFXData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
        }
    }
}

void SpecAbstract::Binary_handle_ProtectorData(QIODevice *pDevice, SpecAbstract::BINARYINFO_STRUCT *pBinaryInfo)
{
    QBinary binary(pDevice);

    // Inno Setup
    if((pBinaryInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_FISHNET))&&(pBinaryInfo->basic_info.nSize>=8))
    {
        _SCANS_STRUCT ss=pBinaryInfo->basic_info.mapHeaderDetects.value(RECORD_NAME_FISHNET);
        pBinaryInfo->mapResultProtectorData.insert(ss.name,scansToScan(&(pBinaryInfo->basic_info),&ss));
    }
}

//void SpecAbstract::fixDetects(SpecAbstract::PEINFO_STRUCT *pPEInfo)
//{
//    if(pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_MICROSOFTLINKER)&&pPEInfo->basic_info.mapHeaderDetects.contains(RECORD_NAME_GENERICLINKER))
//    {
//        pPEInfo->basic_info.mapHeaderDetects.remove(RECORD_NAME_MICROSOFTLINKER);
//    }

//    if(pPEInfo->_mapImportDetects.contains(RECORD_NAME_C)&&pPEInfo->_mapImportDetects.contains(RECORD_NAME_VISUALCPP))
//    {
//        pPEInfo->_mapImportDetects.remove(RECORD_NAME_VISUALCPP);
//    }

//    if(pPEInfo->mapSpecialDetects.contains(RECORD_NAME_ENIGMA))
//    {
//        pPEInfo->mapEntryPointDetects.remove(RECORD_NAME_BORLANDCPP);
//    }
//}

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

SpecAbstract::VI_STRUCT SpecAbstract::PE_get_UPX_vi(QIODevice *pDevice,SpecAbstract::PEINFO_STRUCT *pPEInfo)
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

SpecAbstract::VI_STRUCT SpecAbstract::PE_get_Armadillo_vi(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    VI_STRUCT result;

    QPE pe(pDevice);

    if(pe.isValid())
    {
        QSet<QString> stDetects;

        if(pPEInfo->listImports.count()==3)
        {
            if(pPEInfo->listImports.at(0).sName.toUpper()=="KERNEL32.DLL")
            {
                if(pPEInfo->listImports.at(0).listPositions.count()==70)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="FreeEnvironmentStringsA"))
                    {
                        stDetects.insert("kernel32_4");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==76)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetTempPathA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="FindNextFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="FreeLibrary")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="SleepEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetTempFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="CreateMailslotA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="SetErrorMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="DeleteFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="UnhandledExceptionFilter"))
                    {
                        stDetects.insert("kernel32_1");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==82)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetSystemTime"))
                    {
                        stDetects.insert("kernel32_4a");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==86)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="DeleteFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GetTempPathA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FindNextFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="FreeLibrary")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetPriorityClass")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="SleepEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="GetTempFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="CreateMailslotA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="FreeEnvironmentStringsW"))
                    {
                        stDetects.insert("kernel32_2");
                    }
                    else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="DeleteFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GetTempPathA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FindNextFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="FreeLibrary")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="SleepEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="GetTempFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="CreateMailslotA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="SetPriorityClass")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="FreeEnvironmentStringsA"))
                     {
                         stDetects.insert("kernel32_3");
                     }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==89)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="RtlUnwind"))
                    {
                        stDetects.insert("kernel32_4b");
                    }
                    else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="RtlUnwind"))
                    {
                        stDetects.insert("kernel32_4c");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==100)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetShortPathNameA"))
                    {
                        stDetects.insert("kernel32_4d");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==104)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="GetShortPathNameA"))
                    {
                        stDetects.insert("kernel32_5");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==105)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="GetShortPathNameA"))
                    {
                        stDetects.insert("kernel32_6");
                    }
                    else if((pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="ReadFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetFileSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="GetLocalTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GetVersion")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetSystemTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="GetTimeZoneInformation")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="GetShortPathNameA"))
                    {
                        stDetects.insert("kernel32_7");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==118)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="WriteConsoleW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="GetConsoleOutputCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="WriteConsoleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetConsoleMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetConsoleCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetLocaleInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="HeapSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="QueryPerformanceCounter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="IsValidCodePage")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="DeleteCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="GetSystemTimeAsFileTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(105).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(106).sName=="GetProcessHeap")&&
                            (pPEInfo->listImports.at(0).listPositions.at(107).sName=="RaiseException")&&
                            (pPEInfo->listImports.at(0).listPositions.at(108).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(109).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(110).sName=="SetUnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(111).sName=="IsDebuggerPresent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(112).sName=="TlsGetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(113).sName=="TlsAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(114).sName=="TlsSetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(115).sName=="TlsFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(116).sName=="InterlockedIncrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(117).sName=="InterlockedDecrement"))
                    {
                        stDetects.insert("kernel32_7a");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==119)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="SetErrorMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="WriteConsoleW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="GetConsoleOutputCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="WriteConsoleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetConsoleMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="GetConsoleCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="GetLocaleInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="HeapSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="QueryPerformanceCounter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="DeleteCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="TlsFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="TlsSetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="TlsAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="TlsGetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="GetSystemTimeAsFileTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(105).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(106).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(107).sName=="GetProcessHeap")&&
                            (pPEInfo->listImports.at(0).listPositions.at(108).sName=="RaiseException")&&
                            (pPEInfo->listImports.at(0).listPositions.at(109).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(110).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(111).sName=="SetUnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(112).sName=="IsDebuggerPresent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(113).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(114).sName=="InterlockedIncrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(115).sName=="InterlockedDecrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(116).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(117).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(118).sName=="IsValidCodePage"))
                    {
                        stDetects.insert("kernel32_8");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==126)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="SetErrorMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="OutputDebugStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="OutputDebugStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="SetEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="CreateEventA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="GetSystemTimeAsFileTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="LocalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="WriteConsoleW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="GetConsoleOutputCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="WriteConsoleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="FormatMessageA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="GetConsoleMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="GetConsoleCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="GetLocaleInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="IsValidLocale")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="EnumSystemLocalesA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="GetLocaleInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetUserDefaultLCID")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="QueryPerformanceCounter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="InterlockedIncrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="InterlockedDecrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="InterlockedExchange")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="DeleteCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="RaiseException")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="SetUnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(105).sName=="IsDebuggerPresent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(106).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(107).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(108).sName=="GetProcessHeap")&&
                            (pPEInfo->listImports.at(0).listPositions.at(109).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(110).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(111).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(112).sName=="TlsGetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(113).sName=="TlsAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(114).sName=="TlsSetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(115).sName=="TlsFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(116).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(117).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(118).sName=="HeapSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(119).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(120).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(121).sName=="IsValidCodePage")&&
                            (pPEInfo->listImports.at(0).listPositions.at(122).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(123).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(124).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(125).sName=="HeapReAlloc"))
                    {
                        stDetects.insert("kernel32_11");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==129)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="SetErrorMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="OutputDebugStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="OutputDebugStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="SetEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="CreateEventA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="GetSystemTimeAsFileTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="LocalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="WriteConsoleW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="GetConsoleOutputCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="WriteConsoleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="FormatMessageA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="GetConsoleMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="GetConsoleCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetLocaleInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="IsValidLocale")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="EnumSystemLocalesA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetLocaleInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="GetUserDefaultLCID")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="QueryPerformanceCounter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="InterlockedIncrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="InterlockedDecrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="InterlockedExchange")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="DeleteCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="RaiseException")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(105).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(106).sName=="SetUnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(107).sName=="IsDebuggerPresent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(108).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(109).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(110).sName=="GetProcessHeap")&&
                            (pPEInfo->listImports.at(0).listPositions.at(111).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(112).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(113).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(114).sName=="TlsGetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(115).sName=="TlsAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(116).sName=="TlsSetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(117).sName=="TlsFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(118).sName=="HeapSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(119).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(120).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(121).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(122).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(123).sName=="IsValidCodePage")&&
                            (pPEInfo->listImports.at(0).listPositions.at(124).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(125).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(126).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(127).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(128).sName=="GetTimeZoneInformation"))
                    {
                        stDetects.insert("kernel32_10");
                    }
                }
                else if(pPEInfo->listImports.at(0).listPositions.count()==131)
                {
                    if(     (pPEInfo->listImports.at(0).listPositions.at(0).sName=="CreateThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(1).sName=="GlobalUnlock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(2).sName=="GlobalLock")&&
                            (pPEInfo->listImports.at(0).listPositions.at(3).sName=="GlobalAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(4).sName=="GetTickCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(5).sName=="WideCharToMultiByte")&&
                            (pPEInfo->listImports.at(0).listPositions.at(6).sName=="IsBadReadPtr")&&
                            (pPEInfo->listImports.at(0).listPositions.at(7).sName=="GlobalAddAtomA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(8).sName=="GlobalAddAtomW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(9).sName=="GetModuleHandleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(10).sName=="GlobalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(11).sName=="GlobalGetAtomNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(12).sName=="GlobalDeleteAtom")&&
                            (pPEInfo->listImports.at(0).listPositions.at(13).sName=="GlobalGetAtomNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(14).sName=="FreeConsole")&&
                            (pPEInfo->listImports.at(0).listPositions.at(15).sName=="GetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(16).sName=="VirtualProtect")&&
                            (pPEInfo->listImports.at(0).listPositions.at(17).sName=="VirtualAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(18).sName=="GetProcAddress")&&
                            (pPEInfo->listImports.at(0).listPositions.at(19).sName=="GetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(20).sName=="LoadLibraryA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(21).sName=="SetLastError")&&
                            (pPEInfo->listImports.at(0).listPositions.at(22).sName=="SetThreadPriority")&&
                            (pPEInfo->listImports.at(0).listPositions.at(23).sName=="GetCurrentThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(24).sName=="CreateProcessA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(25).sName=="GetCommandLineA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(26).sName=="GetStartupInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(27).sName=="SetEnvironmentVariableA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(28).sName=="ReleaseMutex")&&
                            (pPEInfo->listImports.at(0).listPositions.at(29).sName=="WaitForSingleObject")&&
                            (pPEInfo->listImports.at(0).listPositions.at(30).sName=="CreateMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(31).sName=="OpenMutexA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(32).sName=="SetErrorMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(33).sName=="GetCurrentThreadId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(34).sName=="FindClose")&&
                            (pPEInfo->listImports.at(0).listPositions.at(35).sName=="FindFirstFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(36).sName=="FindFirstFileW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(37).sName=="VirtualQueryEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(38).sName=="GetExitCodeProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(39).sName=="ReadProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(40).sName=="VirtualProtectEx")&&
                            (pPEInfo->listImports.at(0).listPositions.at(41).sName=="UnmapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(42).sName=="ContinueDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(43).sName=="SetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(44).sName=="GetThreadContext")&&
                            (pPEInfo->listImports.at(0).listPositions.at(45).sName=="WaitForDebugEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(46).sName=="SuspendThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(47).sName=="DebugActiveProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(48).sName=="ResumeThread")&&
                            (pPEInfo->listImports.at(0).listPositions.at(49).sName=="CreateProcessW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(50).sName=="GetCommandLineW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(51).sName=="GetStartupInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(52).sName=="MapViewOfFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(53).sName=="DuplicateHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(54).sName=="GetCurrentProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(55).sName=="CreateFileMappingA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(56).sName=="WriteProcessMemory")&&
                            (pPEInfo->listImports.at(0).listPositions.at(57).sName=="SetEvent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(58).sName=="CreateEventA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(59).sName=="MultiByteToWideChar")&&
                            (pPEInfo->listImports.at(0).listPositions.at(60).sName=="CloseHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(61).sName=="CreateFileA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(62).sName=="GetSystemTimeAsFileTime")&&
                            (pPEInfo->listImports.at(0).listPositions.at(63).sName=="ExitProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(64).sName=="LocalFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(65).sName=="CompareStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(66).sName=="CompareStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(67).sName=="FlushFileBuffers")&&
                            (pPEInfo->listImports.at(0).listPositions.at(68).sName=="WriteConsoleW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(69).sName=="GetConsoleOutputCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(70).sName=="WriteConsoleA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(71).sName=="SetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(72).sName=="FormatMessageA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(73).sName=="GetConsoleMode")&&
                            (pPEInfo->listImports.at(0).listPositions.at(74).sName=="GetConsoleCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(75).sName=="SetFilePointer")&&
                            (pPEInfo->listImports.at(0).listPositions.at(76).sName=="GetLocaleInfoW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(77).sName=="GetStringTypeW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(78).sName=="GetStringTypeA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(79).sName=="IsValidLocale")&&
                            (pPEInfo->listImports.at(0).listPositions.at(80).sName=="EnumSystemLocalesA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(81).sName=="GetLocaleInfoA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(82).sName=="GetUserDefaultLCID")&&
                            (pPEInfo->listImports.at(0).listPositions.at(83).sName=="QueryPerformanceCounter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(84).sName=="GetFileType")&&
                            (pPEInfo->listImports.at(0).listPositions.at(85).sName=="SetHandleCount")&&
                            (pPEInfo->listImports.at(0).listPositions.at(86).sName=="GetEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(87).sName=="Sleep")&&
                            (pPEInfo->listImports.at(0).listPositions.at(88).sName=="EnterCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(89).sName=="LeaveCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(90).sName=="GetVersionExA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(91).sName=="InitializeCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(92).sName=="GetCurrentProcessId")&&
                            (pPEInfo->listImports.at(0).listPositions.at(93).sName=="GetModuleFileNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(94).sName=="GetShortPathNameW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(95).sName=="GetModuleFileNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(96).sName=="GetShortPathNameA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(97).sName=="FreeEnvironmentStringsW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(98).sName=="GetEnvironmentStrings")&&
                            (pPEInfo->listImports.at(0).listPositions.at(99).sName=="FreeEnvironmentStringsA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(100).sName=="InterlockedIncrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(101).sName=="InterlockedDecrement")&&
                            (pPEInfo->listImports.at(0).listPositions.at(102).sName=="InterlockedExchange")&&
                            (pPEInfo->listImports.at(0).listPositions.at(103).sName=="DeleteCriticalSection")&&
                            (pPEInfo->listImports.at(0).listPositions.at(104).sName=="RtlUnwind")&&
                            (pPEInfo->listImports.at(0).listPositions.at(105).sName=="RaiseException")&&
                            (pPEInfo->listImports.at(0).listPositions.at(106).sName=="TerminateProcess")&&
                            (pPEInfo->listImports.at(0).listPositions.at(107).sName=="UnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(108).sName=="SetUnhandledExceptionFilter")&&
                            (pPEInfo->listImports.at(0).listPositions.at(109).sName=="IsDebuggerPresent")&&
                            (pPEInfo->listImports.at(0).listPositions.at(110).sName=="HeapFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(111).sName=="HeapAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(112).sName=="GetProcessHeap")&&
                            (pPEInfo->listImports.at(0).listPositions.at(113).sName=="GetCPInfo")&&
                            (pPEInfo->listImports.at(0).listPositions.at(114).sName=="LCMapStringA")&&
                            (pPEInfo->listImports.at(0).listPositions.at(115).sName=="LCMapStringW")&&
                            (pPEInfo->listImports.at(0).listPositions.at(116).sName=="TlsGetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(117).sName=="TlsAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(118).sName=="TlsSetValue")&&
                            (pPEInfo->listImports.at(0).listPositions.at(119).sName=="TlsFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(120).sName=="HeapSize")&&
                            (pPEInfo->listImports.at(0).listPositions.at(121).sName=="WriteFile")&&
                            (pPEInfo->listImports.at(0).listPositions.at(122).sName=="GetStdHandle")&&
                            (pPEInfo->listImports.at(0).listPositions.at(123).sName=="GetACP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(124).sName=="GetOEMCP")&&
                            (pPEInfo->listImports.at(0).listPositions.at(125).sName=="IsValidCodePage")&&
                            (pPEInfo->listImports.at(0).listPositions.at(126).sName=="HeapDestroy")&&
                            (pPEInfo->listImports.at(0).listPositions.at(127).sName=="HeapCreate")&&
                            (pPEInfo->listImports.at(0).listPositions.at(128).sName=="VirtualFree")&&
                            (pPEInfo->listImports.at(0).listPositions.at(129).sName=="HeapReAlloc")&&
                            (pPEInfo->listImports.at(0).listPositions.at(130).sName=="GetTimeZoneInformation"))
                    {
                        stDetects.insert("kernel32_9");
                    }
                }
            }

            if(pPEInfo->listImports.at(1).sName.toUpper()=="USER32.DLL")
            {
                if(pPEInfo->listImports.at(1).listPositions.count()==24)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SendMessageTimeoutA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="SystemParametersInfoA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="CreateDialogParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="PostQuitMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="SetWindowTextA"))
                    {
                        stDetects.insert("user32_1");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SendMessageTimeoutA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="SystemParametersInfoA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="CreateDialogParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="PostQuitMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="GetDlgItem"))
                    {
                        stDetects.insert("user32_2");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="LoadCursorA"))
                     {
                        stDetects.insert("user32_4");
                     }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==25)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SendMessageTimeoutA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="SystemParametersInfoA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="CreateDialogParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="PostQuitMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="SetWindowTextA"))
                    {
                        stDetects.insert("user32_3");
                    }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==31)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="RegisterClassA"))
                    {
                        stDetects.insert("user32_4a");
                    }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==33)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="SetTimer"))
                    {
                        stDetects.insert("user32_4b");
                        stDetects.insert("user32_4c");
                    }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==39)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="RegisterClassA"))
                    {
                        stDetects.insert("user32_4d");
                    }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==44)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="PostMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="SendMessageW"))
                    {
                        stDetects.insert("user32_5");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="SendMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="PostMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="SendMessageA"))
                    {
                        stDetects.insert("user32_6");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="SendMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="PostMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="PeekMessageA"))
                    {
                        stDetects.insert("user32_7");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="SendMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="PostMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="DestroyWindow"))
                    {
                        stDetects.insert("user32_7a");
                        stDetects.insert("user32_8");
                    }
                }
                else if(pPEInfo->listImports.at(1).listPositions.count()==45)
                {
                    if(     (pPEInfo->listImports.at(1).listPositions.at(0).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="DefWindowProcW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="SendMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(44).sName=="PostMessageW"))
                    {
                        stDetects.insert("user32_9");
                    }
                    else if((pPEInfo->listImports.at(1).listPositions.at(0).sName=="LoadStringW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(1).sName=="IsWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(2).sName=="PostMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(3).sName=="GetDesktopWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(4).sName=="MoveWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(5).sName=="SetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(6).sName=="EnumThreadWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(7).sName=="GetPropA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(8).sName=="GetMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(9).sName=="BeginPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(10).sName=="KillTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(11).sName=="GetAsyncKeyState")&&
                            (pPEInfo->listImports.at(1).listPositions.at(12).sName=="GetSystemMetrics")&&
                            (pPEInfo->listImports.at(1).listPositions.at(13).sName=="SetTimer")&&
                            (pPEInfo->listImports.at(1).listPositions.at(14).sName=="SetWindowTextA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(15).sName=="GetDlgItem")&&
                            (pPEInfo->listImports.at(1).listPositions.at(16).sName=="CreateDialogIndirectParamA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(17).sName=="ShowWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(18).sName=="UpdateWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(19).sName=="LoadStringA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(20).sName=="EndPaint")&&
                            (pPEInfo->listImports.at(1).listPositions.at(21).sName=="FindWindowA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(22).sName=="WaitForInputIdle")&&
                            (pPEInfo->listImports.at(1).listPositions.at(23).sName=="DestroyWindow")&&
                            (pPEInfo->listImports.at(1).listPositions.at(24).sName=="MessageBoxA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(25).sName=="InSendMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(26).sName=="UnpackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(27).sName=="FreeDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(28).sName=="DefWindowProcW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(29).sName=="DefWindowProcA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(30).sName=="LoadCursorA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(31).sName=="RegisterClassW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(32).sName=="CreateWindowExW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(33).sName=="RegisterClassA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(34).sName=="CreateWindowExA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(35).sName=="GetWindowThreadProcessId")&&
                            (pPEInfo->listImports.at(1).listPositions.at(36).sName=="SendMessageW")&&
                            (pPEInfo->listImports.at(1).listPositions.at(37).sName=="SendMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(38).sName=="PeekMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(39).sName=="TranslateMessage")&&
                            (pPEInfo->listImports.at(1).listPositions.at(40).sName=="DispatchMessageA")&&
                            (pPEInfo->listImports.at(1).listPositions.at(41).sName=="EnumWindows")&&
                            (pPEInfo->listImports.at(1).listPositions.at(42).sName=="IsWindowUnicode")&&
                            (pPEInfo->listImports.at(1).listPositions.at(43).sName=="PackDDElParam")&&
                            (pPEInfo->listImports.at(1).listPositions.at(44).sName=="PostMessageW"))
                    {
                        stDetects.insert("user32_10");
                        stDetects.insert("user32_11");
                    }
                }
            }
            if(pPEInfo->listImports.at(2).sName.toUpper()=="GDI32.DLL")
            {
                if(pPEInfo->listImports.at(2).listPositions.count()==10)
                {
                    if(     (pPEInfo->listImports.at(2).listPositions.at(0).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="CreateDIBitmap")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="CreatePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreateCompatibleDC"))
                    {
                        stDetects.insert("gdi32_1");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="CreatePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="CreateCompatibleDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreateDIBitmap"))
                    {
                        stDetects.insert("gdi32_2");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="CreateDIBitmap")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="CreatePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="CreateCompatibleDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="DeleteObject"))
                    {
                        stDetects.insert("gdi32_3");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="CreatePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="CreateDIBitmap")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreateCompatibleDC"))
                    {
                        stDetects.insert("gdi32_4");
                        stDetects.insert("gdi32_4a");
                        stDetects.insert("gdi32_4b");
                        stDetects.insert("gdi32_4c");
                        stDetects.insert("gdi32_9");
                        stDetects.insert("gdi32_10");
                        stDetects.insert("gdi32_11");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="CreateDIBitmap")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="CreateCompatibleDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreatePalette"))
                    {
                        stDetects.insert("gdi32_4d");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="CreatePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="CreateCompatibleDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreateDIBitmap"))
                    {
                        stDetects.insert("gdi32_5");
                        stDetects.insert("gdi32_6");
                        stDetects.insert("gdi32_7");
                    }
                    else if((pPEInfo->listImports.at(2).listPositions.at(0).sName=="CreateDCA")&&
                            (pPEInfo->listImports.at(2).listPositions.at(1).sName=="CreateDIBitmap")&&
                            (pPEInfo->listImports.at(2).listPositions.at(2).sName=="CreateCompatibleDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(3).sName=="SelectObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(4).sName=="SelectPalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(5).sName=="RealizePalette")&&
                            (pPEInfo->listImports.at(2).listPositions.at(6).sName=="BitBlt")&&
                            (pPEInfo->listImports.at(2).listPositions.at(7).sName=="DeleteDC")&&
                            (pPEInfo->listImports.at(2).listPositions.at(8).sName=="DeleteObject")&&
                            (pPEInfo->listImports.at(2).listPositions.at(9).sName=="CreatePalette"))
                    {
                        stDetects.insert("gdi32_7a");
                        stDetects.insert("gdi32_8");
                    }
                }
            }

        }

        if(stDetects.contains("kernel32_1")&&stDetects.contains("user32_1")&&stDetects.contains("gdi32_1"))
        {
            result.sVersion="1.74";
        }
        else if(stDetects.contains("kernel32_2")&&stDetects.contains("user32_2")&&stDetects.contains("gdi32_2"))
        {
            result.sVersion="1.90";
        }
        else if(stDetects.contains("kernel32_3")&&stDetects.contains("user32_3")&&stDetects.contains("gdi32_3"))
        {
            result.sVersion="1.91c";
        }
        else if(stDetects.contains("kernel32_4")&&stDetects.contains("user32_4")&&stDetects.contains("gdi32_4"))
        {
            result.sVersion="2.52";
        }
        else if(stDetects.contains("kernel32_4a")&&stDetects.contains("user32_4a")&&stDetects.contains("gdi32_4a"))
        {
            result.sVersion="3.00-3.10";
        }
        else if(stDetects.contains("kernel32_4b")&&stDetects.contains("user32_4b")&&stDetects.contains("gdi32_4b"))
        {
            result.sVersion="3.30-3.40";
        }
        else if(stDetects.contains("kernel32_4c")&&stDetects.contains("user32_4c")&&stDetects.contains("gdi32_4c"))
        {
            result.sVersion="3.70";
        }
        else if(stDetects.contains("kernel32_4d")&&stDetects.contains("user32_4d")&&stDetects.contains("gdi32_4d"))
        {
            result.sVersion="3.78";
        }
        else if(stDetects.contains("kernel32_5")&&stDetects.contains("user32_5")&&stDetects.contains("gdi32_5"))
        {
            result.sVersion="4.00-4.40";
        }
        else if(stDetects.contains("kernel32_6")&&stDetects.contains("user32_6")&&stDetects.contains("gdi32_6"))
        {
            result.sVersion="4.42-4.54";
        }
        else if(stDetects.contains("kernel32_7")&&stDetects.contains("user32_7")&&stDetects.contains("gdi32_7"))
        {
            result.sVersion="4.66";
        }
        else if(stDetects.contains("kernel32_7a")&&stDetects.contains("user32_7a")&&stDetects.contains("gdi32_7a"))
        {
            result.sVersion="5.00-5.42";
        }
        else if(stDetects.contains("kernel32_8")&&stDetects.contains("user32_8")&&stDetects.contains("gdi32_8"))
        {
            result.sVersion="6.0.0";
        }
        else if(stDetects.contains("kernel32_9")&&stDetects.contains("user32_9")&&stDetects.contains("gdi32_9"))
        {
            result.sVersion="6.2.4.624";
        }
        else if(stDetects.contains("kernel32_10")&&stDetects.contains("user32_10")&&stDetects.contains("gdi32_10"))
        {
            result.sVersion="6.40";
        }
        else if(stDetects.contains("kernel32_11")&&stDetects.contains("user32_11")&&stDetects.contains("gdi32_11"))
        {
            result.sVersion="6.60-7.00";
        }
        #ifdef QT_DEBUG
        qDebug() << stDetects;
        #endif
    }

    return result;
}

SpecAbstract::VI_STRUCT SpecAbstract::PE_get_GCC_vi(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    VI_STRUCT result;

    QBinary binary(pDevice);

    // TODO get max version
    qint64 nOffset_Version=binary.find_ansiString(nOffset,nSize,"gcc-");

    if(nOffset_Version!=-1)
    {
        QString sVersionString=binary.read_ansiString(nOffset_Version);
        result.sVersion=sVersionString.section("-",1,1).section("/",0,0);
    }


    return result;
}

bool SpecAbstract::PE_isValid_UPX(QIODevice *pDevice, SpecAbstract::PEINFO_STRUCT *pPEInfo)
{
    Q_UNUSED(pDevice);

    if(pPEInfo->listSectionHeaders.count()>=3)
    {
        // pPEInfo->listSections.at(0).SizeOfRawData!=0 dump file
        if((pPEInfo->listSectionHeaders.at(0).SizeOfRawData==0)&&((pPEInfo->nResourceSection==-1)||(pPEInfo->nResourceSection==2)))
        {
            return true;
        }
    }

    return false;
}

SpecAbstract::SCAN_STRUCT SpecAbstract::scansToScan(SpecAbstract::BASIC_INFO *pBasicInfo, SpecAbstract::_SCANS_STRUCT *pScansStruct)
{
    SCAN_STRUCT result={};

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
    BASIC_PE_INFO result= {};

    QDataStream ds((QByteArray *)pbaArray,QIODevice::ReadOnly);

    ds>>result.nEntryPoint;

    return result;
}

void SpecAbstract::memoryScan(QMap<RECORD_NAMES, _SCANS_STRUCT> *pMmREcords, QIODevice *pDevice, qint64 nOffset, qint64 nSize, SpecAbstract::SCANMEMORY_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
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
                        SpecAbstract::_SCANS_STRUCT record={};
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

void SpecAbstract::signatureScan(QMap<RECORD_NAMES, _SCANS_STRUCT> *pMapRecords, QString sSignature, SpecAbstract::SIGNATURE_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
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
                    SpecAbstract::_SCANS_STRUCT record={};
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

void SpecAbstract::resourcesScan(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::_SCANS_STRUCT> *pMapRecords, QList<QPE::RESOURCE_RECORD> *pListResources, SpecAbstract::RESOURCES_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
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
                    SpecAbstract::_SCANS_STRUCT record={};
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

void SpecAbstract::stringScan(QMap<SpecAbstract::RECORD_NAMES, SpecAbstract::_SCANS_STRUCT> *pMapRecords, QList<QString> *pListStrings, SpecAbstract::STRING_RECORD *pRecords, int nRecordsSize, SpecAbstract::RECORD_FILETYPES fileType1, SpecAbstract::RECORD_FILETYPES fileType2)
{
    QList<quint32> listStringCRC;
    QList<quint32> listSignatureCRC;

    int nCount=pListStrings->count();
    int nSignaturesCount=nRecordsSize/sizeof(STRING_RECORD);

    for(int i=0;i<nCount;i++)
    {
        quint32 nCRC=QBinary::getCRC32(pListStrings->at(i));
        listStringCRC.append(nCRC);
    }

    for(int i=0;i<nSignaturesCount;i++)
    {
        quint32 nCRC=QBinary::getCRC32(pRecords[i].pszString);
        listSignatureCRC.append(nCRC);
    }

    for(int i=0;i<nCount;i++)
    {
        for(int j=0;j<nSignaturesCount;j++)
        {
            if((pRecords[j].filetype==fileType1)||(pRecords[j].filetype==fileType2))
            {
                if(!pMapRecords->contains(pRecords[j].name))
                {
                    quint32 nCRC1=listStringCRC[i];
                    quint32 nCRC2=listSignatureCRC[j];
                    if(nCRC1==nCRC2)
                    {
                        QString sTest1=pListStrings->at(i);
                        QString sTest2=pRecords[j].pszString;

                        SpecAbstract::_SCANS_STRUCT record={};
                        record.nVariant=pRecords[j].nVariant;
                        record.filetype=pRecords[j].filetype;
                        record.type=pRecords[j].type;
                        record.name=pRecords[j].name;
                        record.sVersion=pRecords[j].pszVersion;
                        record.sInfo=pRecords[j].pszInfo;

                        record.nOffset=0;

                        pMapRecords->insert(record.name,record);
                    }
                }
            }
        }
    }
}

QList<SpecAbstract::VCL_STRUCT> SpecAbstract::PE_getVCLstruct(QIODevice *pDevice,qint64 nOffset,qint64 nSize,bool bIs64)
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
                    VCL_STRUCT record={};

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

SpecAbstract::VCL_PACKAGEINFO SpecAbstract::PE_getVCLPackageInfo(QIODevice *pDevice, QList<QPE::RESOURCE_RECORD> *pListResources)
{
    VCL_PACKAGEINFO result={};

    QPE pe(pDevice);

    if(pe.isValid())
    {
        QPE::RESOURCE_RECORD rh=pe.getResourceRecord(10,"PACKAGEINFO",pListResources);

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

                    VCL_PACKAGEINFO_MODULE vpm=VCL_PACKAGEINFO_MODULE();
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

SpecAbstract::_SCANS_STRUCT SpecAbstract::PE_getRichSignatureDescription(quint32 nRichID)
{
    SpecAbstract::_SCANS_STRUCT result={};

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
                result.name=SpecAbstract::RECORD_NAME_VISUALCCPP; // TODO Visual C++
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
                result.name=SpecAbstract::RECORD_NAME_VISUALCCPP;
                result.sInfo="C++";
                break;

            case 0x089:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCCPP;
                result.sInfo="C/LTCG";
                break;

            case 0x08a:
                result.type=SpecAbstract::RECORD_TYPE_COMPILER;
                result.name=SpecAbstract::RECORD_NAME_VISUALCCPP;
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
                result.name=SpecAbstract::RECORD_NAME_VISUALCCPP;
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

