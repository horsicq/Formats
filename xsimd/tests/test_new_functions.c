/* Test program for new xsimd functions */
#include "../src/xsimd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_find_null_byte(void)
{
    printf("\n=== Testing xsimd_find_null_byte ===\n");
    
    /* Test 1: Null byte at position 5 */
    char sTest1[] = "Hello\0World";
    xsimd_int64 nPos = xsimd_find_null_byte(sTest1, 11);
    printf("Test 1 - Null at position 5: Found at %lld %s\n", 
           nPos, (nPos == 5) ? "[PASS]" : "[FAIL]");
    
    /* Test 2: No null byte */
    char sTest2[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    nPos = xsimd_find_null_byte(sTest2, 8);
    printf("Test 2 - No null byte: Found at %lld %s\n", 
           nPos, (nPos == -1) ? "[PASS]" : "[FAIL]");
    
    /* Test 3: Null at beginning */
    char sTest3[64];
    memset(sTest3, 'X', 64);
    sTest3[0] = 0;
    nPos = xsimd_find_null_byte(sTest3, 64);
    printf("Test 3 - Null at start: Found at %lld %s\n", 
           nPos, (nPos == 0) ? "[PASS]" : "[FAIL]");
    
    /* Test 4: Null in large buffer (SIMD test) */
    char sTest4[128];
    memset(sTest4, 'X', 128);
    sTest4[100] = 0;
    nPos = xsimd_find_null_byte(sTest4, 128);
    printf("Test 4 - Null at position 100: Found at %lld %s\n", 
           nPos, (nPos == 100) ? "[PASS]" : "[FAIL]");
}

void test_count_unicode_prefix(void)
{
    printf("\n=== Testing xsimd_count_unicode_prefix ===\n");
    
    /* Test 1: Valid Unicode string (English text in UTF-16 LE) */
    /* "Hello" = 0x0048 0x0065 0x006C 0x006C 0x006F */
    unsigned short sTest1[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F};
    xsimd_int64 nBytes = xsimd_count_unicode_prefix(sTest1, 10);
    printf("Test 1 - Valid Unicode (Hello): %lld bytes %s\n", 
           nBytes, (nBytes == 10) ? "[PASS]" : "[FAIL]");
    
    /* Test 2: Valid Unicode with Cyrillic */
    /* "Привет" (Hello in Russian) = 0x041F 0x0440 0x0438 0x0432 0x0435 0x0442 */
    unsigned short sTest2[] = {0x041F, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442};
    nBytes = xsimd_count_unicode_prefix(sTest2, 12);
    printf("Test 2 - Cyrillic Unicode: %lld bytes %s\n", 
           nBytes, (nBytes == 12) ? "[PASS]" : "[FAIL]");
    
    /* Test 3: Invalid Unicode character (0x1000 not in valid ranges) */
    unsigned short sTest3[] = {0x0048, 0x0065, 0x1000, 0x006C};
    nBytes = xsimd_count_unicode_prefix(sTest3, 8);
    printf("Test 3 - Invalid char at position 2: %lld bytes %s\n", 
           nBytes, (nBytes == 4) ? "[PASS]" : "[FAIL]");
    
    /* Test 4: Control character (0x0010 < 0x0020) */
    unsigned short sTest4[] = {0x0048, 0x0010, 0x0065};
    nBytes = xsimd_count_unicode_prefix(sTest4, 6);
    printf("Test 4 - Control char at position 1: %lld bytes %s\n", 
           nBytes, (nBytes == 2) ? "[PASS]" : "[FAIL]");
    
    /* Test 5: Large valid Unicode buffer (SIMD test) */
    unsigned short sTest5[128];
    int i;
    for (i = 0; i < 128; i++) {
        sTest5[i] = 0x0041 + (i % 26);  /* A-Z repeated */
    }
    nBytes = xsimd_count_unicode_prefix(sTest5, 256);
    printf("Test 5 - Large valid buffer (256 bytes): %lld bytes %s\n", 
           nBytes, (nBytes == 256) ? "[PASS]" : "[FAIL]");
}

void test_is_valid_unicode(void)
{
    printf("\n=== Testing xsimd_is_valid_unicode ===\n");
    
    /* Test 1: All valid */
    unsigned short sTest1[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F};
    int bValid = xsimd_is_valid_unicode(sTest1, 10);
    printf("Test 1 - All valid: %d %s\n", bValid, (bValid == 1) ? "[PASS]" : "[FAIL]");
    
    /* Test 2: Contains invalid */
    unsigned short sTest2[] = {0x0048, 0x1000, 0x006C};
    bValid = xsimd_is_valid_unicode(sTest2, 6);
    printf("Test 2 - Contains invalid: %d %s\n", bValid, (bValid == 0) ? "[PASS]" : "[FAIL]");
    
    /* Test 3: Cyrillic valid */
    unsigned short sTest3[] = {0x041F, 0x0440, 0x0438};
    bValid = xsimd_is_valid_unicode(sTest3, 6);
    printf("Test 3 - Cyrillic valid: %d %s\n", bValid, (bValid == 1) ? "[PASS]" : "[FAIL]");
}

void test_count_char(void)
{
    printf("\n=== Testing xsimd_count_char ===\n");
    
    /* Test 1: Count 'A' characters */
    char sTest1[] = "ABACADAEAFAGA";
    xsimd_int64 nCount = xsimd_count_char(sTest1, 13, 'A');
    printf("Test 1 - Count 'A' in string: %lld %s\n", 
           nCount, (nCount == 7) ? "[PASS]" : "[FAIL]");
    
    /* Test 2: Count spaces */
    char sTest2[] = "Hello World Test String";
    nCount = xsimd_count_char(sTest2, 23, ' ');
    printf("Test 2 - Count spaces: %lld %s\n", 
           nCount, (nCount == 3) ? "[PASS]" : "[FAIL]");
    
    /* Test 3: Count non-existent character */
    char sTest3[] = "Hello World";
    nCount = xsimd_count_char(sTest3, 11, 'Z');
    printf("Test 3 - Count 'Z' (not present): %lld %s\n", 
           nCount, (nCount == 0) ? "[PASS]" : "[FAIL]");
    
    /* Test 4: Large buffer (SIMD test) */
    char sTest4[256];
    int i;
    for (i = 0; i < 256; i++) {
        sTest4[i] = (i % 10 == 0) ? 'X' : 'A';
    }
    nCount = xsimd_count_char(sTest4, 256, 'X');
    printf("Test 4 - Count 'X' in large buffer: %lld %s\n", 
           nCount, (nCount == 26) ? "[PASS]" : "[FAIL]");  /* 0, 10, 20, ..., 250 = 26 occurrences */
}

void test_performance_comparison(void)
{
    printf("\n=== Performance Comparison ===\n");
    
    /* Create large test buffer */
    const int nBufferSize = 1024 * 1024;  /* 1 MB */
    char* pBuffer = (char*)malloc(nBufferSize);
    int i;
    
    /* Fill with test data */
    for (i = 0; i < nBufferSize; i++) {
        pBuffer[i] = 0x20 + (i % 96);  /* Printable ASCII */
    }
    pBuffer[nBufferSize / 2] = 0;  /* Add null in the middle */
    
    printf("Buffer size: %d bytes\n", nBufferSize);
    
    /* Test 1: Find null byte */
    xsimd_int64 nPos = xsimd_find_null_byte(pBuffer, nBufferSize);
    printf("Find null byte: Found at position %lld\n", nPos);
    
    /* Test 2: Count specific character */
    pBuffer[nBufferSize / 2] = 'X';  /* Remove null */
    for (i = 0; i < 1000; i++) {
        pBuffer[i * (nBufferSize / 1000)] = 'X';
    }
    xsimd_int64 nCount = xsimd_count_char(pBuffer, nBufferSize, 'X');
    printf("Count 'X' character: %lld occurrences\n", nCount);
    
    /* Test 3: ANSI prefix count */
    nCount = xsimd_count_ansi_prefix(pBuffer, nBufferSize);
    printf("ANSI prefix length: %lld bytes\n", nCount);
    
    free(pBuffer);
}

int main(void)
{
    printf("=======================================================\n");
    printf("     XSIMD New Functions Test Suite\n");
    printf("=======================================================\n");
    
    /* Initialize xsimd library */
    xsimd_init();
    
    printf("XSIMD Version: %s\n", xsimd_version());
    printf("Detected features: 0x%08X\n", xsimd_get_features());
    printf("Enabled features:  0x%08X\n", xsimd_get_enabled_features());
    
    if (xsimd_is_avx2_present()) printf("  - AVX2 available\n");
    if (xsimd_is_sse2_present()) printf("  - SSE2 available\n");
    
    /* Run tests */
    test_find_null_byte();
    test_count_unicode_prefix();
    test_is_valid_unicode();
    test_count_char();
    test_performance_comparison();
    
    printf("\n=======================================================\n");
    printf("                  Tests Complete\n");
    printf("=======================================================\n");
    
    xsimd_cleanup();
    
    return 0;
}
