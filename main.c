#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <windows.h>

// External assembly function (you'll provide this next)
extern void imgCvtGrayIntToDouble(int height, int width, uint8_t* intImg, double* floatImg);

// Pure C implementation for comparison
void imgCvtGrayIntToDouble_C(int height, int width, const uint8_t* intImg, double* floatImg) {
    int size = height * width;
    for (int i = 0; i < size; i++) {
        floatImg[i] = intImg[i] / 255.0;
    }
}

// Function to generate random image data
void generateRandomImage(uint8_t* img, int size) {
    for (int i = 0; i < size; i++) {
        img[i] = rand() % 256;
    }
}

// Function to check correctness of conversion
int checkCorrectness(const uint8_t* intImg, const double* floatImg, int size) {
    for (int i = 0; i < size; i++) {
        double expected = intImg[i] / 255.0;
        double actual = floatImg[i];
        double diff = (expected > actual) ? (expected - actual) : (actual - expected);

        if (diff > 0.0001) {
            printf("Error at index %d: expected %.6f, got %.6f (from int %d)\n",
                i, expected, actual, intImg[i]);
            return 0;
        }
    }
    return 1;
}

// Function to print a small portion of the image
void printImageSample(const uint8_t* intImg, const double* floatImg, int height, int width, int sampleSize) {
    printf("\nSample Input (uint8):\n");
    for (int i = 0; i < sampleSize && i < height; i++) {
        for (int j = 0; j < sampleSize && j < width; j++) {
            printf("%3d ", intImg[i * width + j]);
        }
        printf("\n");
    }

    printf("\nSample Output (double):\n");
    for (int i = 0; i < sampleSize && i < height; i++) {
        for (int j = 0; j < sampleSize && j < width; j++) {
            printf("%.2f ", floatImg[i * width + j]);
        }
        printf("\n");
    }
}

// Timing function for a given conversion function (via function pointer)
typedef void (*ConvertFunc)(int, int, const uint8_t*, double*);

double measureExecutionTimeFunc(ConvertFunc func, int height, int width, const uint8_t* intImg, double* floatImg, int runs) {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    double totalTime = 0.0;

    QueryPerformanceFrequency(&frequency);

    for (int i = 0; i < runs; i++) {
        QueryPerformanceCounter(&start);
        func(height, width, intImg, floatImg);
        QueryPerformanceCounter(&end);

        totalTime += (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    }

    return totalTime / runs;
}

int main() {
    srand((unsigned int)time(NULL));

    // Test sizes
    int testSizes[][2] = {
        {10, 10},
        {100, 100},
        {1000, 1000}
    };
    int numTests = 3;
    int runs = 30;

    printf("=== Grayscale Image Converter: uint8 to double ===\n");
    printf("Comparison: C vs x86-64 Assembly (Scalar SIMD)\n");
    printf("Windows x64 Version\n\n");

    // Test with small example first
    printf("--- Testing with example data (3x4) ---\n");
    int exHeight = 3, exWidth = 4;
    uint8_t exampleInt[] = { 64, 89, 114, 84, 140, 166, 191, 84, 216, 242, 38, 84 };
    double* exampleFloat = (double*)malloc(exHeight * exWidth * sizeof(double));
    double* exampleFloat_c = (double*)malloc(exHeight * exWidth * sizeof(double));

    
    imgCvtGrayIntToDouble(exHeight, exWidth, exampleInt, exampleFloat);
    imgCvtGrayIntToDouble_C(exHeight, exWidth, exampleInt, exampleFloat_c);
    printImageSample(exampleInt, exampleFloat, exHeight, exWidth, 4);

    if (checkCorrectness(exampleInt, exampleFloat, exHeight * exWidth)) {
        printf("\nCorrectness check PASSED for example data!\n");
    }
    else {
        printf("\nCorrectness check FAILED for example data!\n");
    }

    free(exampleFloat);
    free(exampleFloat_c);

    // Performance testing
    printf("\n\n=== Performance Comparison (C vs ASM) ===\n");
    printf("Running %d iterations for each size...\n\n", runs);

    for (int t = 0; t < numTests; t++) {
        int height = testSizes[t][0];
        int width = testSizes[t][1];
        int size = height * width;

        printf("--- Test %d: %dx%d (%d pixels) ---\n", t + 1, height, width, size);

        // Allocate memory
        uint8_t* intImg = (uint8_t*)malloc(size * sizeof(uint8_t));
        double* floatImg_asm = (double*)malloc(size * sizeof(double));
        double* floatImg_c = (double*)malloc(size * sizeof(double));

        if (!intImg || !floatImg_asm || !floatImg_c) {
            printf("Memory allocation failed!\n");
            goto cleanup;
        }

        // Generate random image
        generateRandomImage(intImg, size);

        // Time C version
        double time_c = measureExecutionTimeFunc(imgCvtGrayIntToDouble_C, height, width, intImg, floatImg_c, runs);
        // Time ASM version
        double time_asm = measureExecutionTimeFunc((ConvertFunc)imgCvtGrayIntToDouble, height, width, intImg, floatImg_asm, runs);

        // Check correctness
        int correct_c = checkCorrectness(intImg, floatImg_c, size);
        int correct_asm = checkCorrectness(intImg, floatImg_asm, size);

        // Print results
        printf("C      time: %.9f seconds | Correct: %s\n", time_c, correct_c ? "YES" : "NO");
        printf("ASM    time: %.9f seconds | Correct: %s\n", time_asm, correct_asm ? "YES" : "NO");
        printf("Speedup (C/ASM): %.2fx\n", time_asm > 0 ? time_c / time_asm : 0.0);

    cleanup:
        free(intImg);
        free(floatImg_asm);
        free(floatImg_c);
        printf("\n");
    }

    printf("=== Testing Complete ===\n");
    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}