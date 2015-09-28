#include "CSCConstants.h"
#include <cstdlib>
#include <stdint.h> 
#include <fstream>
#include <vector>
#include <cstdio>

static const int RAM_PAGESIZE = 4*1024; 
static const int NUM_BXS      = 8*RAM_PAGESIZE / 48; 

struct GEMPad{
    unsigned int bx;
    unsigned int pad;
    unsigned int etapartition;
    unsigned int column;
    unsigned int num_hits; 
};

int main(int argc, char * argv[]) {
    std::vector<GEMPad> input_pads;
    
    std::fstream text_file(argv[1]);
     

    //ignore first line (header); 
    std::string str;
    std::getline(text_file, str);

    unsigned int n=0;
    while (!text_file.eof()) {
        struct GEMPad gem_pad; 
        text_file 
            >> gem_pad.bx 
            >> gem_pad.pad
            >> gem_pad.etapartition
            >> gem_pad.column 
            >> gem_pad.num_hits;
        input_pads.push_back(GEMPad());
        input_pads[n] = gem_pad; 
        n++; 
    }

    // The last entry gets read twice, pop one of them
    input_pads.pop_back();
    
    FILE *f; 
    f = fopen ("GEMPads_andrew.pat", "w+b"); 
    for (int bx=0; bx<NUM_BXS; bx++) {

        int hits_this_bx=0; 
        uint64_t packet=0; 

        for (unsigned int i=0; i<input_pads.size(); i++) {

            if (input_pads[i].bx==unsigned(bx)) {
                if (hits_this_bx > 2) {
                    printf("discard hit with : "); 
                    printf("bx %i, pad %i, partition %i, column %i, num_hits %i\n", input_pads[i].bx, input_pads[i].pad, input_pads[i].etapartition, input_pads[i].column, input_pads[i].num_hits); 
                    break; 
                }
                else {
                    printf("writing hit with : "); 
                    printf("bx %i, pad %i, partition %i, column %i, num_hits %i\n", input_pads[i].bx, input_pads[i].pad, input_pads[i].etapartition, input_pads[i].column, input_pads[i].num_hits); 
                    uint16_t hit =    ((input_pads[i].column       & 0x03) <<  0) 
                                    | ((input_pads[i].etapartition & 0x07) <<  2) 
                                    | ((input_pads[i].pad          & 0x3F) <<  5) 
                                    | ((input_pads[i].num_hits     & 0x07) << 11);
                   
                    packet |= hit << 14*(hits_this_bx); 
                    hits_this_bx += 1; 
                }
            }
        }

        printf("%012llx\n", packet); 

        for (int byte=0; byte<6; byte++) {
            uint8_t data [1]; 
            data[0] =      0xFF & (packet >> (48-8*(byte+1))); 
            printf("%02x", 0xFF & (packet >> (48-8*(byte+1)))); 
            fwrite(data, sizeof(char), 1, f); 
        }
        printf("\n"); 
    }

    /* Fill some space at the end.. the ram page size does not exactly line up to the 48 bits/bx word size */
    uint8_t data [4] = {0}; 
    fwrite(data, sizeof(char), 4, f); 

    /* Sic Transit Gloria Mundi */
    fclose(f); 
}
