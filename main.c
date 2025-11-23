#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <windows.h>

// External assembly function declaration
extern void imgCvtGrayIntToDouble(int height, int width, uint8_t* intImg, double* floatImg);

// Function to generate random image data
void generateRandomImage(uint8_t* img, int size) {
    for (int i = 0; i < size; i++) {
        img[i] = rand() % 256;
    }
}

// Function to check correctness of conversion
int checkCorrectness(uint8_t* intImg, double* floatImg, int size) {
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
void printImageSample(uint8_t* intImg, double* floatImg, int height, int width, int sampleSize) {
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

// Function to measure execution time using Windows high-resolution timer
double measureExecutionTime(int height, int width, uint8_t* intImg, double* floatImg, int runs) {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    double totalTime = 0.0;
    
    QueryPerformanceFrequency(&frequency);
    
    for (int i = 0; i < runs; i++) {
        QueryPerformanceCounter(&start);
        imgCvtGrayIntToDouble(height, width, intImg, floatImg);
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
    printf("Using x86-64 Assembly with Scalar SIMD Instructions\n");
    printf("Windows x64 Version\n\n");
    
    // Test with small example first
    printf("--- Testing with example data (3x4) ---\n");
    int exHeight = 3, exWidth = 4;
    uint8_t exampleInt[] = {64, 89, 114, 84, 140, 166, 191, 84, 216, 242, 38, 84};
    double* exampleFloat = (double*)malloc(exHeight * exWidth * sizeof(double));
    
    imgCvtGrayIntToDouble(exHeight, exWidth, exampleInt, exampleFloat);
    printImageSample(exampleInt, exampleFloat, exHeight, exWidth, 4);
    
    if (checkCorrectness(exampleInt, exampleFloat, exHeight * exWidth)) {
        printf("\nCorrectness check PASSED for example data!\n");
    } else {
        printf("\nCorrectness check FAILED for example data!\n");
    }
    free(exampleFloat);
    
    // Performance testing
    printf("\n\n=== Performance Testing ===\n");
    printf("Running %d iterations for each size...\n\n", runs);
    
    for (int t = 0; t < numTests; t++) {
        int height = testSizes[t][0];
        int width = testSizes[t][1];
        int size = height * width;
        
        printf("--- Test %d: %dx%d (%d pixels) ---\n", t + 1, height, width, size);
        
        // Allocate memory
        uint8_t* intImg = (uint8_t*)malloc(size * sizeof(uint8_t));
        double* floatImg = (double*)malloc(size * sizeof(double));
        
        if (!intImg || !floatImg) {
            printf("Memory allocation failed for size %dx%d\n", height, width);
            if (intImg) free(intImg);
            if (floatImg) free(floatImg);
            continue;
        }
        
        // Generate random image
        generateRandomImage(intImg, size);
        
        // Measure execution time
        double avgTime = measureExecutionTime(height, width, intImg, floatImg, runs);
        
        // Check correctness
        int correct = checkCorrectness(intImg, floatImg, size);
        
        // Print results
        printf("Average execution time: %.6f seconds\n", avgTime);
        printf("Correctness check: %s\n", correct ? "PASSED" : "FAILED");  
        
        // Print sample for verification
        if (size <= 100) {
            printImageSample(intImg, floatImg, height, width, 4);
        }
        
        printf("\n");
        
        // Free memory
        free(intImg);
        free(floatImg);
    }
    
    printf("=== Testing Complete ===\n");
    printf("\nPress Enter to exit...");
    getchar();
    
    return 0;
}
