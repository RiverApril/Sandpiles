
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>

using namespace std;

typedef unsigned char byte;

#define min(a, b) a<b?a:b

struct __attribute__ ((packed)) BitmapHeader {
    uint16_t type = 19778;
    uint32_t bitmapSize;
    uint16_t res1 = 0;
    uint16_t res2 = 0;
    uint32_t offset;

    uint32_t beginHeaderSize = 40;
    uint32_t bitmapWidth;
    uint32_t bitmapHeight;
    uint16_t planes = 1;
    uint16_t bitsPerPixel = 32;
    uint32_t compression = 0;
    uint32_t compressionSize = 0;
    uint32_t xPixPerMeter = 0;
    uint32_t yPixPerMeter = 0;
    uint32_t colorCount = 0;
    uint32_t importantColors = 0;
};

void generate(unsigned long totalToDrop, size_t radius);

int main(int argc, char *argv[]){
    
    unsigned long totalToDrop;
    size_t radius;

    if(argc >= 3){
        if(argv[1][0] == '2' && argv[1][1] == '^'){
            totalToDrop = 1 << stoi(argv[1]+2);
        }else{
            totalToDrop = stoi(argv[1]);
        }
        radius = stoi(argv[2]);
    } else {

        string qty, rad;

        printf("Enter amount of sand to drop (as \"num\" or \"2^num\"): ");
        cin >> qty;
        printf("Enter overestimated radius (sans center): ");
        cin >> rad;

        const char* qtyc = qty.c_str();
        const char* radc = rad.c_str();
        
        if(qtyc[0] == '2' && qtyc[1] == '^'){
            totalToDrop = 1 << stoi(qtyc+2);
        }else{
            totalToDrop = stoi(qtyc);
        }
        radius = stoi(radc);
    }

    generate(totalToDrop, radius);

    return 0;
}

void generate(unsigned long totalToDrop, size_t radius){

    clock_t begin = clock();

    size_t size = radius*2+3;

    byte color[4][3] = {{0x00, 0x00, 0x00}, {0xFF, 0xFF, 0x00}, {0x00, 0x00, 0xFF}, {0xFF, 0x00, 0x00}};

    unsigned long toDrop = totalToDrop;

    byte **gridNow = new byte*[size];
    byte **gridNext = new byte*[size];
    for(size_t x = 0; x < size; x++){
        gridNow[x] = new byte[size];
        gridNext[x] = new byte[size];
        for(size_t y = 0; y < size; y++){
            gridNow[x][y] = 0;
            gridNext[x][y] = 0;
        }
    }
    
    printf("Dropping %lu sand...\n", totalToDrop);

    byte a, b, c, d, dropping;

    unsigned long t;

    bool stillGoing = true;
    bool expand = false;

    size_t dropX = size/2;
    size_t dropY = size/2;

    int currentRadius = 1;
    int edge = (size/2)-2;

    for(t = 0; stillGoing; t++){
        stillGoing = false;

        if(toDrop > 0){
            dropping = min(255 - gridNow[dropX][dropY], toDrop);
            toDrop -= dropping;
            //printf("Dropping: %d\n", (int)dropping);
            gridNow[dropX][dropY] = gridNow[dropX][dropY] + dropping;
            stillGoing = true;
            if(toDrop == 0){
                printf("Waiting to settle...\n");
            }
        }

        for(size_t x = edge; x < size-edge; x++){
            for(size_t y = edge; y < size-edge; y++){
                gridNext[x][y] = gridNow[x][y]%4 + gridNow[x-1][y]/4 + gridNow[x][y-1]/4 + gridNow[x+1][y]/4 + gridNow[x][y+1]/4;
            }
        }
        for(size_t x = edge; x < size-edge; x++){
            for(size_t y = edge; y < size-edge; y++){
                if(gridNow[x][y] != gridNext[x][y]){
                    stillGoing = true;
                    if(!expand && (x == dropX-currentRadius || x == dropX+currentRadius || y == dropY-currentRadius || y == dropY+currentRadius)){
                        expand = true;
                    }
                    gridNow[x][y] = gridNext[x][y];
                }
            }
        }
        if(expand){
            //printf("Current Radius: %d\n", currentRadius);
            currentRadius++;
            edge -= 1;
            if(edge < 1){
                edge = 1;
            }
            expand = false;
        }
    }

    printf("Ticks: %lu\n", t);

    size_t actualRadius = currentRadius-1;
    size_t actualSize = actualRadius*2+1;

    edge = (size-actualSize)/2;
    
    size_t imgSize = actualSize*actualSize*4;
    
    printf("Radius (sans center): %lu\n", actualRadius);

    BitmapHeader bmpHeader;
    bmpHeader.bitmapWidth = actualSize;
    bmpHeader.bitmapHeight = actualSize;
    bmpHeader.bitmapSize = sizeof(bmpHeader) + imgSize;
    bmpHeader.offset = sizeof(bmpHeader);

    double greedyArea = 0;
    double filledArea = 0;
    double total = 0;
    double totalEmpty = 0;
    double totalOne = 0;
    double totalTwo = 0;
    double totalThree = 0;

    uint8_t* bmpData = (uint8_t*)malloc(imgSize);
    size_t i = 0;
    for(size_t y = edge; y < size-edge; y++){
        for(size_t x = edge; x < size-edge; x++){
            bmpData[i++] = color[gridNow[x][y]][2];
            bmpData[i++] = color[gridNow[x][y]][1];
            bmpData[i++] = color[gridNow[x][y]][0];
            bmpData[i++] = 0;
            filledArea += (gridNow[x][y] > 0);
            if(gridNow[x][y] > 0 || ((gridNow[x-1][y] > 0) + (gridNow[x+1][y] > 0) + (gridNow[x][y-1] > 0) + (gridNow[x][y+1] > 0)) == 4 ){
                greedyArea++;
                total += gridNow[x][y];
                totalEmpty += (gridNow[x][y] == 0);
                totalOne += (gridNow[x][y] == 1);
                totalTwo += (gridNow[x][y] == 2);
                totalThree += (gridNow[x][y] == 3);
            }
        }
    }

    double calcRadius = actualRadius+0.5;
    // A=PI*R^2
    // PI=A/(R^2)
    printf("(Enclosed Area)/(Radius^2): %f\n", greedyArea / (calcRadius*calcRadius));
    printf("(Non Empty Area)/(Radius^2): %f\n", filledArea / (calcRadius*calcRadius));
    printf("Mean: %f\n", total/greedyArea);
    printf("0: %f%%\n", totalEmpty/greedyArea*100);
    printf("1: %f%%\n", totalOne/greedyArea*100);
    printf("2: %f%%\n", totalTwo/greedyArea*100);
    printf("3: %f%%\n", totalThree/greedyArea*100);


    ofstream file;
    const char* fileName = ("sand_"+to_string(totalToDrop)+".bmp").c_str();
    file.open(fileName);
    file.write((char*)&bmpHeader, sizeof(bmpHeader));
    file.write((char*)bmpData, imgSize);

    free(bmpData);

    for(size_t i = size; i > 0; ){
        delete[] gridNow[--i];
        delete[] gridNext[i];
    }

    delete[] gridNow;
    delete[] gridNext;

    printf("Bitmap %s created\n", fileName);
    
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    if(elapsed_secs > 60*60){
        printf("Total elapsed time: %f hours\n", elapsed_secs/60/60);
    }else if(elapsed_secs > 60){
        printf("Total elapsed time: %f minutes\n", elapsed_secs/60);
    }else{
        printf("Total elapsed time: %f seconds\n", elapsed_secs);
    }
}