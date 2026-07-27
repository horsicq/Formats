/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XCLIASSEMBLY_DEF_H
#define XCLIASSEMBLY_DEF_H

#include <QtGlobal>

namespace XCLIASSEMBLY_DEF {

const quint16 S_IMAGE_FILE_DLL = 0x2000;

struct IMAGE_DATA_DIRECTORY {
    quint32 VirtualAddress;
    quint32 Size;
};

enum ReplacesCorHdrNumericDefines {
    // COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY = 0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED = 0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY = 0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED = 0x00000008,
    COMIMAGE_FLAGS_NATIVE_ENTRYPOINT = 0x00000010,
    COMIMAGE_FLAGS_TRACKDEBUGDATA = 0x00010000,
};

struct IMAGE_COR20_HEADER {
    // Header versioning
    quint32 cb;
    quint16 MajorRuntimeVersion;
    quint16 MinorRuntimeVersion;
    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY MetaData;
    quint32 Flags;
    // DDBLD - Added next section to replace following lin
    // DDBLD - Still verifying, since not in NT SDK
    // unsigned int                   EntryPointToken;
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set,
    // EntryPointToken represents a managed entrypoint.
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set,
    // EntryPointRVA represents an RVA to a native entrypoint.
    union {
        quint32 EntryPointToken;
        quint32 EntryPointRVA;
    };
    // DDBLD - End of Added Area
    // Binding information
    IMAGE_DATA_DIRECTORY Resources;
    IMAGE_DATA_DIRECTORY StrongNameSignature;
    // Regular fixup and binding information
    IMAGE_DATA_DIRECTORY CodeManagerTable;
    IMAGE_DATA_DIRECTORY VTableFixups;
    IMAGE_DATA_DIRECTORY ExportAddressTableJumps;
    // Precompiled image info (internal use only - set to zero)
    IMAGE_DATA_DIRECTORY ManagedNativeHeader;
};

enum MetadataTable {
    metadata_Module = 0x00,
    metadata_TypeRef = 0x01,
    metadata_TypeDef = 0x02,
    metadata_Field = 0x04,
    metadata_MethodPtr = 0x05,
    metadata_MethodDef = 0x06,
    metadata_ParamPtr = 0x07,
    metadata_Param = 0x08,
    metadata_InterfaceImpl = 0x09,
    metadata_MemberRef = 0x0A,
    metadata_Constant = 0x0B,
    metadata_CustomAttribute = 0x0C,
    metadata_FieldMarshal = 0x0D,
    metadata_DeclSecurity = 0x0E,
    metadata_ClassLayout = 0x0F,
    metadata_FieldLayout = 0x10,
    metadata_StandAloneSig = 0x11,
    metadata_EventMap = 0x12,
    metadata_EventPtr = 0x13,
    metadata_Event = 0x14,
    metadata_PropertyMap = 0x15,
    metadata_PropertyPtr = 0x16,
    metadata_Property = 0x17,
    metadata_MethodSemantics = 0x18,
    metadata_MethodImpl = 0x19,
    metadata_ModuleRef = 0x1A,
    metadata_TypeSpec = 0x1B,
    metadata_ImplMap = 0x1C,
    metadata_FieldRVA = 0x1D,
    metadata_ENCLog = 0x1E,
    metadata_ENCMap = 0x1F,
    metadata_Assembly = 0x20,
    metadata_AssemblyProcessor = 0x21,
    metadata_AssemblyOS = 0x22,
    metadata_AssemblyRef = 0x23,
    metadata_AssemblyRefProcessor = 0x24,
    metadata_AssemblyRefOS = 0x25,
    metadata_File = 0x26,
    metadata_ExportedType = 0x27,
    metadata_ManifestResource = 0x28,
    metadata_NestedClass = 0x29,
    metadata_GenericParam = 0x2A,
    metadata_MethodSpec = 0x2B,
    metadata_GenericParamConstraint = 0x2C,
    metadata_Reserved_2D = 0x2D,
    metadata_Reserved_2E = 0x2E,
    metadata_Reserved_2F = 0x2F,
    // Portable PDB tables (Portable PDB metadata specification), 0x30..0x37
    metadata_Document = 0x30,
    metadata_MethodDebugInformation = 0x31,
    metadata_LocalScope = 0x32,
    metadata_LocalVariable = 0x33,
    metadata_LocalConstant = 0x34,
    metadata_ImportScope = 0x35,
    metadata_StateMachineMethod = 0x36,
    metadata_CustomDebugInformation = 0x37,
    metadata_Reserved_38 = 0x38,
    metadata_Reserved_39 = 0x39,
    metadata_Reserved_3A = 0x3A,
    metadata_Reserved_3B = 0x3B,
    metadata_Reserved_3C = 0x3C,
    metadata_Reserved_3D = 0x3D,
    metadata_Reserved_3E = 0x3E,
    metadata_Reserved_3F = 0x3F
};

struct S_METADATA_MODULE {
    quint32 nGeneration;
    quint32 nName;
    quint32 nMvid;
    quint32 nEncId;
    quint32 nEncBaseId;
};

struct S_METADATA_TYPEREF {
    quint32 nResolutionScope;
    quint32 nTypeName;
    quint32 nTypeNamespace;
};

struct S_METADATA_TYPEDEF {
    quint32 nFlags;
    quint32 nTypeName;
    quint32 nTypeNamespace;
    quint32 nExtends;
    quint32 nFieldList;
    quint32 nMethodList;
};

struct S_METADATA_TYPESPEC {
    quint32 nSignature;
};

struct S_METADATA_FIELD {
    quint32 nFlags;
    quint32 nName;
    quint32 nSignature;
};

struct S_METADATA_METHODDEF {
    quint32 nRVA;
    quint32 nImplFlags;
    quint32 nFlags;
    quint32 nName;
    quint32 nSignature;
    quint32 nParamList;
};

struct S_METADATA_METHODPTR {
    quint32 nMethod;
};

struct S_METADATA_PARAM {
    quint32 nFlags;
    quint32 nSequence;
    quint32 nName;
};

struct S_METADATA_MEMBERREF {
    quint32 nClass;
    quint32 nName;
    quint32 nSignature;
};

struct S_METADATA_CONSTANT {
    quint32 nType;
    quint32 nParent;
    quint32 nValue;
};

struct S_METADATA_CUSTOMATTRIBUTE {
    quint32 nParent;
    quint32 nType;
    quint32 nValue;
};

struct S_METADATA_FIELDMARSHAL {
    quint32 nParent;
    quint32 nNativeType;
};

struct S_METADATA_DECLSECURITY {
    quint16 nAction;
    quint32 nParent;
    quint32 nPermissionSet;
};

struct S_METADATA_METHODIMPL {
    quint32 nClass;
    quint32 nMethodBody;
    quint32 nMethodDeclaration;
};

struct S_METADATA_MODULEREF {
    quint32 nName;
};

struct S_METADATA_ASSEMBLY {
    quint32 nHashAlgId;
    quint16 nMajorVersion;
    quint16 nMinorVersion;
    quint16 nBuildNumber;
    quint16 nRevisionNumber;
    quint32 nFlags;
    quint32 nPublicKeyOrToken;
    quint32 nName;
    quint32 nCulture;
};

const quint32 S_METADATA_METHODDEFORREF_METHODDEF = 0;
const quint32 S_METADATA_METHODDEFORREF_MEMBERREF = 1;

struct S_METADATA_METHODDEFORREF {
    quint32 nTag;
    quint32 nIndex;
    union {
        S_METADATA_METHODDEF methoddef;
        S_METADATA_MEMBERREF memberref;
    } record;
};

}  // namespace XCLIASSEMBLY_DEF

#endif  // XCLIASSEMBLY_DEF_H
