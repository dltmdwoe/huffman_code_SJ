#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 허프만 트리의 노드를 나타내는 구조체
typedef struct HuffmanNode {
    char character;             // 문자
    int frequency;              // 빈도수
    struct HuffmanNode* left;   // 왼쪽 자식 노드
    struct HuffmanNode* right;  // 오른쪽 자식 노드
} HuffmanNode;

// 문자 빈도수를 저장하는 구조체
typedef struct Frequency {
    char character;
    int frequency;
} Frequency;

// 허프만 트리 노드를 생성하는 함수
HuffmanNode* createHuffmanNode(char character, int frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->character = character;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 문자 빈도수를 기준으로 오름차순 정렬하는 비교 함수
int compareFrequencies(const void* a, const void* b) {
    Frequency* freqA = (Frequency*)a;
    Frequency* freqB = (Frequency*)b;
    return freqA->frequency - freqB->frequency;
}

// 문자 빈도수를 계산하는 함수
Frequency* calculateFrequencies(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        exit(1);
    }

    int characterCount[256] = { 0 };  // 모든 문자의 빈도수를 저장할 배열

    int c;
    while ((c = fgetc(file)) != EOF) {
        characterCount[c]++;
    }

    fclose(file);

    Frequency* frequencies = (Frequency*)malloc(sizeof(Frequency) * 256);
    *count = 0;
    for (int i = 0; i < 256; i++) {
        if (characterCount[i] > 0) {
            frequencies[*count].character = (char)i;
            frequencies[*count].frequency = characterCount[i];
            (*count)++;
        }
    }

    // 문자 빈도수를 오름차순으로 정렬
    qsort(frequencies, *count, sizeof(Frequency), compareFrequencies);

    return frequencies;
}
// 허프만 트리를 구성하는 함수
HuffmanNode* buildHuffmanTree(Frequency* frequencies, int count) {
    HuffmanNode** nodes = (HuffmanNode**)malloc(sizeof(HuffmanNode*) * count);
    for (int i = 0; i < count; i++) {
        nodes[i] = createHuffmanNode(frequencies[i].character, frequencies[i].frequency);
    }

    while (count > 1) {
        HuffmanNode* left = nodes[0];
        HuffmanNode* right = nodes[1];

        HuffmanNode* parent = createHuffmanNode('$', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;

        int insertIndex = 2;
        while (insertIndex < count && frequencies[insertIndex].frequency < parent->frequency) {
            nodes[insertIndex - 2] = nodes[insertIndex];
            insertIndex++;
        }
        nodes[insertIndex - 2] = parent;

        count--;
    }

    HuffmanNode* root = nodes[0];
    free(nodes);
    return root;
}

// 허프만 트리를 기반으로 문자에 대한 비트 코드를 생성하는 함수
void generateCodes(HuffmanNode* node, char* prefix, char** codes) {
    if (node == NULL) {
        return;
    }

    if (node->left == NULL && node->right == NULL) {
        int index = (int)node->character;
        strcpy(codes[index], prefix);
        return;
    }

    int prefixLength = strlen(prefix);

    char* leftPrefix = (char*)malloc(sizeof(char) * (prefixLength + 2));
    strcpy(leftPrefix, prefix);
    leftPrefix[prefixLength] = '0';
    leftPrefix[prefixLength + 1] = '\0';

    char* rightPrefix = (char*)malloc(sizeof(char) * (prefixLength + 2));
    strcpy(rightPrefix, prefix);
    rightPrefix[prefixLength] = '1';
    rightPrefix[prefixLength + 1] = '\0';

    generateCodes(node->left, leftPrefix, codes);
    generateCodes(node->right, rightPrefix, codes);

    free(leftPrefix);
    free(rightPrefix);
}

// 문자에 대한 허프만 코드를 파일에 저장하는 함수
void saveCodesToFile(char** codes, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        exit(1);
    }

    for (int i = 0; i < 256; i++) {
        if (codes[i][0] != '\0') {
            fprintf(file, "%d\t%s\n", i, codes[i]);
        }
    }

    fclose(file);
}

// 문자에 대한 허프만 코드를 메모리에서 해제하는 함수
void freeCodes(char** codes) {
    for (int i = 0; i < 256; i++) {
        free(codes[i]);
    }
    free(codes);
}

// 문자열을 비트로 변환하는 함수
void stringToBits(const char* input, char* output) {
    int inputLength = strlen(input);
    int bitIndex = 0;

    for (int i = 0; i < inputLength; i++) {
        unsigned char c = input[i];
        for (int j = 7; j >= 0; j--) {
            if (c & (1 << j)) {
                output[bitIndex] = '1';
            }
            else {
                output[bitIndex] = '0';
            }
            bitIndex++;
        }
    }

    output[bitIndex] = '\0';
}

// 비트를 문자열로 변환하는 함수
void bitsToString(const char* input, char* output) {
    int inputLength = strlen(input);
    int charIndex = 0;

    for (int i = 0; i < inputLength; i += 8) {
        unsigned char c = 0;
        for (int j = 0; j < 8; j++) {
            if (input[i + j] == '1') {
                c |= (1 << (7 - j));
            }
        }
        output[charIndex] = c;
        charIndex++;
    }

    output[charIndex] = '\0';
}

// 허프만 코드를 이진 파일로 변환하여 저장하는 함수
void compressFile(const char* inputFilename, const char* outputFilename, const char* codesFilename) {
    Frequency* frequencies;
    int count;
    frequencies = calculateFrequencies(inputFilename, &count);

    HuffmanNode* root = buildHuffmanTree(frequencies, count);

    char* codes[256];
    for (int i = 0; i < 256; i++) {
        codes[i] = (char*)malloc(sizeof(char) * 100);
        codes[i][0] = '\0';
    }

    char prefix[100];
    prefix[0] = '\0';
    generateCodes(root, prefix, codes);

    saveCodesToFile(codes, codesFilename);

    FILE* inputFile = fopen(inputFilename, "r");
    FILE* outputFile = fopen(outputFilename, "wb");

    int c;
    char bits[1000];
    char buffer[1000];
    bits[0] = '\0';

    while ((c = fgetc(inputFile)) != EOF) {
        strcat(bits, codes[c]);
        while (strlen(bits) >= 8) {
            strncpy(buffer, bits, 8);
            buffer[8] = '\0';
            unsigned char byte = strtol(buffer, NULL, 2);
            fwrite(&byte, sizeof(unsigned char), 1, outputFile);
            memmove(bits, bits + 8, strlen(bits + 8) + 1);
        }
    }

    if (strlen(bits) > 0) {
        int padding = 8 - strlen(bits);
        for (int i = 0; i < padding; i++) {
            strcat(bits, "0");
        }
        unsigned char byte = strtol(bits, NULL, 2);
        fwrite(&byte, sizeof(unsigned char), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);

    free(frequencies);
    freeCodes(codes);
}

// 허프만 코드를 이용하여 파일을 해제하는 함수
void decompressFile(const char* inputFilename, const char* outputFilename, const char* codesFilename) {
    FILE* inputFile = fopen(inputFilename, "rb");
    FILE* outputFile = fopen(outputFilename, "w");
    // 허프만 코드를 읽어와서 메모리에 저장
    char codes[256][100];
    int index;
    int character;
    char code[100];
    FILE* codesFile = fopen(codesFilename, "r");
    while (fscanf(codesFile, "%d\t%s\n", &index, code) == 2) {
        strcpy(codes[index], code);
    }
    fclose(codesFile);

    char buffer[1000];
    buffer[0] = '\0';
    char bit;
    int count = 0;

    while (fread(&bit, sizeof(char), 1, inputFile) > 0) {
        if (bit == '1') {
            strcat(buffer, "1");
        }
        else {
            strcat(buffer, "0");
        }

        count++;

        for (int i = 0; i < 256; i++) {
            if (strcmp(buffer, codes[i]) == 0) {
                fprintf(outputFile, "%c", (char)i);
                buffer[0] = '\0';
                count = 0;
                break;
            }
        }
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main() {
    // input.txt에 랜덤 영문자 1000글자 입력
    FILE* inputFile = fopen("input.txt", "w");
    if (inputFile == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < 1000; i++) {
        char randomChar = 'A' + rand() % 26;
        fprintf(inputFile, "%c", randomChar);
    }

    fclose(inputFile);

    // 빈도수 계산 및 stats.txt에 저장
    Frequency* frequencies;
    int count;
    frequencies = calculateFrequencies("input.txt", &count);

    FILE* statsFile = fopen("stats.txt", "w");
    if (statsFile == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(statsFile, "%d\t%d\n", (int)frequencies[i].character, frequencies[i].frequency);
    }

    fclose(statsFile);
    free(frequencies);

    // 파일 압축
    compressFile("input.txt", "output.huf", "codes.txt");

    // 파일 해제
    decompressFile("output.huf", "output.txt", "codes.txt");

    return 0;
}