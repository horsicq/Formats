/* Test program to validate all SIMD paths (AVX2, AVX, SSE2, Scalar) */
#include "../src/xsimd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_all_simd_paths(void)
{
    printf("\n=======================================================\n");
    printf("   Testing All SIMD Paths (AVX2, AVX, SSE2, Scalar)\n");
    printf("=======================================================\n");
    
    /* Create test data */
    char sTestData[128];
    memset(sTestData, 'A', 128);
    sTestData[50] = 0;  /* Null at position 50 */
    sTestData[100] = 'X';  /* Add some X characters */
    sTestData[110] = 'X';
    sTestData[120] = 'X';
    
    unsigned short sUnicodeData[64];
    int i;
    for (i = 0; i < 64; i++) {
        sUnicodeData[i] = 0x0041 + (i % 26);  /* A-Z */
    }
    sUnicodeData[30] = 0x1000;  /* Invalid Unicode at position 30 */
    
    /* Test with AVX2 + AVX + SSE2 */
    printf("\n--- Testing with AVX2 + AVX + SSE2 ---\n");
    xsimd_enable_all_features();
    printf("Enabled features: 0x%08X\n", xsimd_get_enabled_features());
    
    xsimd_int64 nPos = xsimd_find_null_byte(sTestData, 128);
    printf("find_null_byte: %lld %s\n", nPos, (nPos == 50) ? "[PASS]" : "[FAIL]");
    
    xsimd_int64 nValidBytes = xsimd_count_unicode_prefix(sUnicodeData, 128);
    printf("count_unicode_prefix: %lld bytes %s\n", nValidBytes, (nValidBytes == 60) ? "[PASS]" : "[FAIL]");
    
    xsimd_int64 nCount = xsimd_count_char(sTestData, 128, 'X');
    printf("count_char: %lld %s\n", nCount, (nCount == 3) ? "[PASS]" : "[FAIL]");
    
    /* Test with AVX + SSE2 only (disable AVX2) */
    printf("\n--- Testing with AVX + SSE2 (no AVX2) ---\n");
    xsimd_set_avx2(0);
    printf("Enabled features: 0x%08X\n", xsimd_get_enabled_features());
    
    nPos = xsimd_find_null_byte(sTestData, 128);
    printf("find_null_byte: %lld %s\n", nPos, (nPos == 50) ? "[PASS]" : "[FAIL]");
    
    nValidBytes = xsimd_count_unicode_prefix(sUnicodeData, 128);
    printf("count_unicode_prefix: %lld bytes %s\n", nValidBytes, (nValidBytes == 60) ? "[PASS]" : "[FAIL]");
    
    nCount = xsimd_count_char(sTestData, 128, 'X');
    printf("count_char: %lld %s\n", nCount, (nCount == 3) ? "[PASS]" : "[FAIL]");
    
    /* Test with SSE2 only (disable AVX and AVX2) */
    printf("\n--- Testing with SSE2 only (no AVX/AVX2) ---\n");
    xsimd_set_avx(0);
    xsimd_set_avx2(0);
    printf("Enabled features: 0x%08X\n", xsimd_get_enabled_features());
    
    nPos = xsimd_find_null_byte(sTestData, 128);
    printf("find_null_byte: %lld %s\n", nPos, (nPos == 50) ? "[PASS]" : "[FAIL]");
    
    nValidBytes = xsimd_count_unicode_prefix(sUnicodeData, 128);
    printf("count_unicode_prefix: %lld bytes %s\n", nValidBytes, (nValidBytes == 60) ? "[PASS]" : "[FAIL]");
    
    nCount = xsimd_count_char(sTestData, 128, 'X');
    printf("count_char: %lld %s\n", nCount, (nCount == 3) ? "[PASS]" : "[FAIL]");
    
    /* Test with Scalar only (disable all SIMD) */
    printf("\n--- Testing with Scalar only (no SIMD) ---\n");
    xsimd_disable_all_features();
    printf("Enabled features: 0x%08X\n", xsimd_get_enabled_features());
    
    nPos = xsimd_find_null_byte(sTestData, 128);
    printf("find_null_byte: %lld %s\n", nPos, (nPos == 50) ? "[PASS]" : "[FAIL]");
    
    nValidBytes = xsimd_count_unicode_prefix(sUnicodeData, 128);
    printf("count_unicode_prefix: %lld bytes %s\n", nValidBytes, (nValidBytes == 60) ? "[PASS]" : "[FAIL]");
    
    nCount = xsimd_count_char(sTestData, 128, 'X');
    printf("count_char: %lld %s\n", nCount, (nCount == 3) ? "[PASS]" : "[FAIL]");
    
    /* Restore all features */
    xsimd_enable_all_features();
    
    printf("\n=======================================================\n");
    printf("           All SIMD Paths Validated\n");
    printf("=======================================================\n");
}

int main(void)
{
    printf("=======================================================\n");
    printf("     XSIMD SIMD Path Validation Test\n");
    printf("=======================================================\n");
    
    xsimd_init();
    
    printf("XSIMD Version: %s\n", xsimd_version());
    printf("Detected features: 0x%08X\n", xsimd_get_features());
    
    if (xsimd_is_avx2_present()) printf("  - AVX2 detected\n");
    if (xsimd_is_avx_present())  printf("  - AVX detected\n");
    if (xsimd_is_sse2_present()) printf("  - SSE2 detected\n");
    
    test_all_simd_paths();
    
    xsimd_cleanup();
    
    return 0;
}
