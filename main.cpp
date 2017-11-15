
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>

using namespace std;

#define min(a, b) a<b?a:b
#define max(a, b) a>b?a:b

#ifdef SAND_8
    #define QTY 8
#else
    #define QTY 4
#endif

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

const unsigned char color[2][8][3] = {{
    {0x00, 0x00, 0x00}, 
    {0xFF, 0xFF, 0x00}, 
    {0x00, 0x00, 0xFF}, 
    {0xFF, 0x00, 0x00}, 
    {0x00, 0xFF, 0x00}, 
    {0x00, 0xFF, 0xFF}, 
    {0xFF, 0x00, 0xFF}, 
    {0x80, 0x00, 0xFF}
},{
    {0x00, 0x00, 0x00}, 
    {0xFF, 0x00, 0x00}, 
    {0xFF, 0xFF, 0x00}, 
    {0x00, 0xFF, 0x00}, 
    {0x00, 0xFF, 0xFF}, 
    {0x00, 0x00, 0xFF}, 
    {0x80, 0x00, 0xFF}, 
    {0xFF, 0x00, 0xFF}
}};

void generate1(unsigned long long totalToDrop, size_t radius, int colorIndex);

template<class T>
void generate2(T totalToDrop, size_t radius, int colorIndex);

int main(int argc, char *argv[]){
    
    unsigned long long totalToDrop;
    size_t radius;

    string qty;
    string rad;
    string col;

    if(argc >= 2){
        qty = argv[1];
    }else{
        printf("Enter amount of sand to drop (as \"num\" or \"2^num\"): ");
        cin >> qty;
    }

    if(argc >= 3){
        rad = argv[2];
    }else{
        printf("Enter overestimated radius (sans center): ");
        cin >> rad;
    }

    if(argc >= 4){
        col = argv[3];
    }else{
        col = "0";
    }

    const char* qtyc = qty.c_str();
    const char* radc = rad.c_str();
    
    if(qtyc[0] == '2' && qtyc[1] == '^'){
        totalToDrop = 1 << stoull(qtyc+2);
    }else{
        totalToDrop = stoull(qtyc);
    }
    radius = stoul(radc);

    int colorIndex = stoi(col);

    generate1(totalToDrop, radius, colorIndex);

    return 0;
}

void generate1(unsigned long long totalToDrop, size_t radius, int colorIndex){
    if(totalToDrop <= UCHAR_MAX){
        printf("Allocating using u-char\n");
        generate2<unsigned char>((unsigned char)totalToDrop, radius, colorIndex);
    }else if(totalToDrop <= USHRT_MAX){
        printf("Allocating using u-short\n");
        generate2<unsigned short>((unsigned short)totalToDrop, radius, colorIndex);
    }else if(totalToDrop <= UINT_MAX){
        printf("Allocating using u-int\n");
        generate2<unsigned int>((unsigned int)totalToDrop, radius, colorIndex);
    }else if(totalToDrop <= ULONG_MAX){
        printf("Allocating using u-long\n");
        generate2<unsigned long>((unsigned long)totalToDrop, radius, colorIndex);
    }else if(totalToDrop <= ULLONG_MAX){
        printf("Allocating using u-llong\n");
        generate2<unsigned long long>((unsigned long long)totalToDrop, radius, colorIndex);
    }
}

template<class T>
void generate2(T totalToDrop, size_t radius, int colorIndex){

    clock_t begin = clock();

    size_t size = radius*2+3;
    
    T **gridNow = new T*[size];
    for(size_t x = 0; x < size; x++){
        gridNow[x] = new T[size];
        for(size_t y = 0; y < size; y++){
            gridNow[x][y] = 0;
        }
    }
    
    printf("Dropping %llu sand...\n", (unsigned long long)totalToDrop);

    T topple;

    unsigned long long t;

    bool stillGoing = true;

    size_t dropX = size/2;
    size_t dropY = size/2;

    gridNow[dropX][dropY] = totalToDrop;

    size_t minX = dropX;
    size_t maxX = dropX;
    size_t minY = dropY;
    size_t maxY = dropY;

    for(t = 0; stillGoing; t++){
        stillGoing = false;
        for(size_t x = minX; x <= maxX; x++){
            for(size_t y = minY; y <= maxY; y++){

                if(gridNow[x][y] >= QTY){
                    topple = gridNow[x][y]/QTY;
                    gridNow[x][y] -= topple*QTY;
                    gridNow[x-1][y] += topple;
                    gridNow[x+1][y] += topple;
                    gridNow[x][y-1] += topple;
                    gridNow[x][y+1] += topple;
                    #ifdef SAND_8
                        gridNow[x-1][y-1] += topple;
                        gridNow[x+1][y+1] += topple;
                        gridNow[x+1][y-1] += topple;
                        gridNow[x-1][y+1] += topple;
                    #endif
                    minX = min(minX, x-1);
                    minY = min(minY, y-1);
                    maxX = max(maxX, x+1);
                    maxY = max(maxY, y+1);
                    stillGoing = true;
                }
            }
        }
    }

    printf("Ticks: %llu\n", t);

    size_t actualRadius = ((maxX-minX))/2;
    size_t actualSize = actualRadius*2+1;
    
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
    double totals[QTY];
    for(int j = 0; j < QTY; j++){
        totals[j] = 0;
    }

    uint8_t* bmpData = (uint8_t*)malloc(imgSize);
    size_t i = 0;
    for(size_t y = minY; y <= maxY; y++){
        for(size_t x = minX; x <= maxX; x++){
            bmpData[i++] = color[colorIndex][gridNow[x][y]][2];
            bmpData[i++] = color[colorIndex][gridNow[x][y]][1];
            bmpData[i++] = color[colorIndex][gridNow[x][y]][0];
            bmpData[i++] = 0;
            filledArea += (gridNow[x][y] > 0);
            if(gridNow[x][y] > 0 || ((gridNow[x-1][y] > 0) + (gridNow[x+1][y] > 0) + (gridNow[x][y-1] > 0) + (gridNow[x][y+1] > 0)) == 4 ){
                greedyArea++;
                total += gridNow[x][y];
                for(int j = 0; j < QTY; j++){
                    totals[j] += (gridNow[x][y] == j);
                }
            }
        }
    }

    double calcRadius = actualRadius+0.5;
    // A=PI*R^2
    // PI=A/(R^2)
    printf("(Enclosed Area)/(Radius^2): %f\n", greedyArea / (calcRadius*calcRadius));
    printf("(Non Empty Area)/(Radius^2): %f\n", filledArea / (calcRadius*calcRadius));
    printf("Mean: %f\n", total/greedyArea);
    for(int j = 0; j < QTY; j++){
        printf("%d: %f%%\n", j, totals[j]/greedyArea*100);
    }


    ofstream file;
    #ifdef SAND_8
        #define PREFIX "sand8_"
    #else
        #define PREFIX "sand_"
    #endif
    const char* fileName = (PREFIX+to_string(totalToDrop)+".bmp").c_str();
    file.open(fileName);
    file.write((char*)&bmpHeader, sizeof(bmpHeader));
    file.write((char*)bmpData, imgSize);

    free(bmpData);

    for(size_t i = size; i > 0; ){
        delete[] gridNow[--i];
    }

    delete[] gridNow;

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