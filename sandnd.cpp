
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>

using namespace std;

#define min(a, b) a<b?a:b
#define max(a, b) a>b?a:b

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

const unsigned char color[3][16][3] = {{
    {0x00, 0x00, 0x00}, 
    {0xFF, 0xFF, 0x00}, 
    {0x00, 0x00, 0xFF}, 
    {0xFF, 0x00, 0x00}, 
    {0x00, 0xFF, 0x00}, 
    {0x00, 0xFF, 0xFF}, 
    {0xFF, 0x00, 0xFF}, 
    {0x80, 0x00, 0xFF}, 

    {0x80, 0x80, 0x00}, 
    {0x80, 0x80, 0xFF}, 
    {0xFF, 0x80, 0xFF}, 
    {0x00, 0x80, 0x00}, 
    {0x00, 0x80, 0x80}, 
    {0x80, 0x00, 0x80}, 
    {0x80, 0xFF, 0xFF}, 
    {0xFF, 0x80, 0x80}
},{
    { 25,   7,  26}, // dark violett
    {  4,   4,  73}, // blue 5
    { 12,  44, 138}, // blue 3
    { 57, 125, 209}, // blue 1
    {211, 236, 248}, // lightest blue
    {248, 201,  95}, // light yellow
    {204, 128,   0}, // brown 0
    {106,  52,   3}, // brown 2

},{
    { 66,  30,  15}, // brown 3
    { 25,   7,  26}, // dark violett
    {  9,   1,  47}, // darkest blue
    {  4,   4,  73}, // blue 5
    {  0,   7, 100}, // blue 4
    { 12,  44, 138}, // blue 3
    { 24,  82, 177}, // blue 2
    { 57, 125, 209}, // blue 1
    {134, 181, 229}, // blue 0
    {211, 236, 248}, // lightest blue
    {241, 233, 191}, // lightest yellow
    {248, 201,  95}, // light yellow
    {255, 170,   0}, // dirty yellow
    {204, 128,   0}, // brown 0
    {153,  87,   0}, // brown 1
    {106,  52,   3}  // brown 2

}};

void generate1(unsigned long long totalToDrop, size_t radius, size_t colorIndex, size_t dims);

template<class T>
void generate2(T totalToDrop, size_t radius, size_t colorIndex, size_t dims);

int main(int argc, char *argv[]){
    
    unsigned long long totalToDrop;
    size_t radius;
    size_t colorIndex;
    size_t dims;

    string dim;
    string qty;
    string rad;
    string col;

    if(argc >= 2){
        dim = argv[1];
    }else{
        printf("Enter the number of dimentions: ");
        cin >> dim;
    }

    if(argc >= 3){
        qty = argv[2];
    }else{
        printf("Enter amount of sand to drop (as \"num\" or \"2^num\"): ");
        cin >> qty;
    }

    if(argc >= 4){
        rad = argv[3];
    }else{
        printf("Enter overestimated radius (sans center): ");
        cin >> rad;
    }

    if(argc >= 5){
        col = argv[4];
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
    colorIndex = stoul(col);
    dims = stoul(dim);

    generate1(totalToDrop, radius, colorIndex, dims);

    return 0;
}

void generate1(unsigned long long totalToDrop, size_t radius, size_t colorIndex, size_t dims){
    if(totalToDrop <= UCHAR_MAX){
        printf("Allocating using u-char\n");
        generate2<unsigned char>((unsigned char)totalToDrop, radius, colorIndex, dims);
    }else if(totalToDrop <= USHRT_MAX){
        printf("Allocating using u-short\n");
        generate2<unsigned short>((unsigned short)totalToDrop, radius, colorIndex, dims);
    }else if(totalToDrop <= UINT_MAX){
        printf("Allocating using u-int\n");
        generate2<unsigned int>((unsigned int)totalToDrop, radius, colorIndex, dims);
    }else if(totalToDrop <= ULONG_MAX){
        printf("Allocating using u-long\n");
        generate2<unsigned long>((unsigned long)totalToDrop, radius, colorIndex, dims);
    }else if(totalToDrop <= ULLONG_MAX){
        printf("Allocating using u-llong\n");
        generate2<unsigned long long>((unsigned long long)totalToDrop, radius, colorIndex, dims);
    }
}

size_t getCoordCenter(size_t dims, size_t size){
    size_t coord = 0;
    for(size_t i = 0; i < dims; i++){
        coord += (size/2) * (size_t)pow(size, dims-i-1);
    }
    return coord;
}

size_t getCoord(size_t dims, size_t size, size_t* pos){
    size_t coord = 0;
    for(size_t i = 0; i < dims; i++){
        coord += pos[i] * (size_t)pow(size, dims-i-1);
    }
    return coord;
}

template<class T>
void toppleGrid(T* grid, size_t size, size_t dims, size_t* mins, size_t* maxs, size_t* coord, size_t depth, bool& stillGoing){
    if(depth == 0){
        T topple;
        for(coord[depth] = mins[depth]; coord[depth] <= maxs[depth]; coord[depth]++){
            size_t x = getCoord(dims, size, coord);
            size_t qty = dims*2;
            if(grid[x] >= qty){
                topple = grid[x] / qty;
                grid[x] -= topple * qty;
                for(size_t d = 0; d < dims; d++){
                    coord[d]-=1; // left one
                    grid[getCoord(dims, size, coord)] += topple;
                    coord[d]+=2; // right one
                    grid[getCoord(dims, size, coord)] += topple;
                    coord[d]-=1; // back to where we were

                    mins[d] = min(mins[d], coord[d]-1);
                    maxs[d] = max(maxs[d], coord[d]+1);
                    stillGoing = true;
                }
            }
        }
    }else{
        for(coord[depth] = mins[depth]; coord[depth] <= maxs[depth]; coord[depth]++){
            toppleGrid(grid, size, dims, mins, maxs, coord, depth-1, stillGoing);
        }
    }
}

template<class T>
void saveBmp(T* grid, size_t size, size_t dims, size_t* mins, size_t* maxs, size_t* coord, size_t depth, uint8_t* bmpData, size_t &i, size_t colorIndex){
    if(depth == 0){
        for(coord[depth] = mins[depth]; coord[depth] <= maxs[depth]; coord[depth]++){
            size_t x = getCoord(dims, size, coord);
            bmpData[i++] = color[colorIndex][grid[x]][2];
            bmpData[i++] = color[colorIndex][grid[x]][1];
            bmpData[i++] = color[colorIndex][grid[x]][0];
            bmpData[i++] = 0;
        }
    }else{
        for(coord[depth] = mins[depth]; coord[depth] <= maxs[depth]; coord[depth]++){
            saveBmp(grid, size, dims, mins, maxs, coord, depth-1, bmpData, i, colorIndex);
        }
    }
}

template<class T>
void generate2(T totalToDrop, size_t radius, size_t colorIndex, size_t dims){

    clock_t begin = clock();

    size_t size = radius*2+3;

    T* grid = new T[(size_t)pow(size, dims)];
    
    printf("Dropping %llu sand...\n", (unsigned long long)totalToDrop);

    unsigned long long t;

    bool stillGoing = true;

    grid[getCoordCenter(dims, size)] = totalToDrop;

    size_t* mins = new size_t[dims];
    size_t* maxs = new size_t[dims];
    size_t* coord = new size_t[dims];

    for(size_t i = 0; i < dims; i++){
        mins[i] = size/2;
        maxs[i] = size/2;
        coord[i] = 0;
    }

    for(t = 0; stillGoing; t++){
        stillGoing = false;
        toppleGrid(grid, size, dims, mins, maxs, coord, dims-1, stillGoing);
    }

    printf("Ticks: %llu\n", t);

    size_t actualRadius = ((maxs[0]-mins[0]))/2;
    size_t actualSize = actualRadius*2+1;
    
    size_t imgSize = (size_t)pow(actualSize, dims)*4;
    
    printf("Radius (sans center): %lu\n", actualRadius);

    BitmapHeader bmpHeader;
    bmpHeader.bitmapWidth = (uint32_t)pow(actualSize, floor(dims/2.0));
    bmpHeader.bitmapHeight = (uint32_t)pow(actualSize, ceil(dims/2.0));
    bmpHeader.bitmapSize = sizeof(bmpHeader) + imgSize;
    bmpHeader.offset = sizeof(bmpHeader);

    uint8_t* bmpData = (uint8_t*)malloc(imgSize);
    size_t i = 0;

    saveBmp(grid, size, dims, mins, maxs, coord, dims-1, bmpData, i, colorIndex);

    ofstream file;
    string s = "sand_" + to_string(dims) + "d";
    const char* fileName = ("img/"+s+"_"+to_string(totalToDrop)+"_c"+to_string(colorIndex)+".bmp").c_str();
    file.open(fileName);
    file.write((char*)&bmpHeader, sizeof(bmpHeader));
    file.write((char*)bmpData, imgSize);

    free(bmpData);

    /*for(size_t k = size; k > 0; ){
        for(size_t j = size; j > 0; ){
            for(size_t i = size; i > 0; ){
                delete[] gridNow[k][j][--i];
            }
            delete[] gridNow[k][--j];
        }
        delete[] gridNow[--k];
    }
    delete[] gridNow;*/

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