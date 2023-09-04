#include "neaacdec.h"
#include <assert.h>

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* common free function */
void faad_free(void* b) { free(b); }

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* initialize buffer, call once before first getbits or showbits */
void faad_initbits(bitfile* ld, const void* _buffer, const uint32_t buffer_size) {
    uint32_t tmp;

    if(ld == NULL) return;

    if(buffer_size == 0 || _buffer == NULL) {
        ld->error = 1;
        return;
    }

    ld->buffer = _buffer;
    ld->buffer_size = buffer_size;
    ld->bytes_left = buffer_size;

    if(ld->bytes_left >= 4) {
        tmp = getdword((uint32_t*)ld->buffer);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n((uint32_t*)ld->buffer, ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufa = tmp;

    if(ld->bytes_left >= 4) {
        tmp = getdword((uint32_t*)ld->buffer + 1);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n((uint32_t*)ld->buffer + 1, ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufb = tmp;

    ld->start = (uint32_t*)ld->buffer;
    ld->tail = ((uint32_t*)ld->buffer + 2);
    ld->bits_left = 32;
    ld->error = 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint32_t faad_get_processed_bits(bitfile* ld) { return (uint32_t)(8 * (4 * (ld->tail - ld->start) - 4) - (ld->bits_left)); }

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t faad_byte_align(bitfile* ld) {
    int remainder = (32 - ld->bits_left) & 0x7;

    if(remainder) {
        faad_flushbits(ld, 8 - remainder);
        return (uint8_t)(8 - remainder);
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void faad_flushbits_ex(bitfile* ld, uint32_t bits) {
    uint32_t tmp;

    ld->bufa = ld->bufb;
    if(ld->bytes_left >= 4) {
        tmp = getdword(ld->tail);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n(ld->tail, ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufb = tmp;
    ld->tail++;
    ld->bits_left += (32 - bits);
    // ld->bytes_left -= 4;
    //    if (ld->bytes_left == 0)
    //        ld->no_more_reading = 1;
    //    if (ld->bytes_left < 0)
    //        ld->error = 1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* rewind to beginning */
void faad_rewindbits(bitfile* ld) {
    uint32_t tmp;

    ld->bytes_left = ld->buffer_size;

    if(ld->bytes_left >= 4) {
        tmp = getdword((uint32_t*)&ld->start[0]);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n((uint32_t*)&ld->start[0], ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufa = tmp;

    if(ld->bytes_left >= 4) {
        tmp = getdword((uint32_t*)&ld->start[1]);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n((uint32_t*)&ld->start[1], ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufb = tmp;

    ld->bits_left = 32;
    ld->tail = &ld->start[2];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* reset to a certain point */
void faad_resetbits(bitfile* ld, int bits) {
    uint32_t tmp;
    int      words = bits >> 5;
    int      remainder = bits & 0x1F;

    if(ld->buffer_size < words * 4) ld->bytes_left = 0;
    else
        ld->bytes_left = ld->buffer_size - words * 4;

    if(ld->bytes_left >= 4) {
        tmp = getdword(&ld->start[words]);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n(&ld->start[words], ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufa = tmp;

    if(ld->bytes_left >= 4) {
        tmp = getdword(&ld->start[words + 1]);
        ld->bytes_left -= 4;
    }
    else {
        tmp = getdword_n(&ld->start[words + 1], ld->bytes_left);
        ld->bytes_left = 0;
    }
    ld->bufb = tmp;

    ld->bits_left = 32 - remainder;
    ld->tail = &ld->start[words + 2];

    /* recheck for reading too many bytes */
    ld->error = 0;
    //    if (ld->bytes_left == 0)
    //        ld->no_more_reading = 1;
    //    if (ld->bytes_left < 0)
    //        ld->error = 1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* faad_getbitbuffer(bitfile* ld, uint32_t bits) {
    int          i;
    unsigned int temp;
    int          bytes = bits >> 3;
    int          remainder = bits & 0x7;

    uint8_t* buffer = (uint8_t*)faad_malloc((bytes + 1) * sizeof(uint8_t));

    for(i = 0; i < bytes; i++) { buffer[i] = (uint8_t)faad_getbits(ld, 8); }

    if(remainder) {
        temp = faad_getbits(ld, remainder ) << (8 - remainder);

        buffer[bytes] = (uint8_t)temp;
    }

    return buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* reversed bit reading routines, used for RVLC and HCR */
void faad_initbits_rev(bitfile* ld, void* buffer, uint32_t bits_in_buffer) {
    uint32_t tmp;
    int32_t  index;

    ld->buffer_size = bit2byte(bits_in_buffer);

    index = (bits_in_buffer + 31) / 32 - 1;

    ld->start = (uint32_t*)buffer + index - 2;

    tmp = getdword((uint32_t*)buffer + index);
    ld->bufa = tmp;

    tmp = getdword((uint32_t*)buffer + index - 1);
    ld->bufb = tmp;

    ld->tail = (uint32_t*)buffer + index;

    ld->bits_left = bits_in_buffer % 32;
    if(ld->bits_left == 0) ld->bits_left = 32;

    ld->bytes_left = ld->buffer_size;
    ld->error = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
//   passf2, passf3, passf4, passf5. Complex FFT passes fwd and bwd.
static void passf2pos(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa) {
    uint16_t i, k, ah, ac;

    if(ido == 1) {
        for(k = 0; k < l1; k++) {
            ah = 2 * k;
            ac = 4 * k;
            RE(ch[ah]) = RE(cc[ac]) + RE(cc[ac + 1]);
            RE(ch[ah + l1]) = RE(cc[ac]) - RE(cc[ac + 1]);
            IM(ch[ah]) = IM(cc[ac]) + IM(cc[ac + 1]);
            IM(ch[ah + l1]) = IM(cc[ac]) - IM(cc[ac + 1]);
        }
    }
    else {
        for(k = 0; k < l1; k++) {
            ah = k * ido;
            ac = 2 * k * ido;
            for(i = 0; i < ido; i++) {
                complex_t t2;
                RE(ch[ah + i]) = RE(cc[ac + i]) + RE(cc[ac + i + ido]);
                RE(t2) = RE(cc[ac + i]) - RE(cc[ac + i + ido]);
                IM(ch[ah + i]) = IM(cc[ac + i]) + IM(cc[ac + i + ido]);
                IM(t2) = IM(cc[ac + i]) - IM(cc[ac + i + ido]);
                ComplexMult(&IM(ch[ah + i + l1 * ido]), &RE(ch[ah + i + l1 * ido]), IM(t2), RE(t2), RE(wa[i]), IM(wa[i]));
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void passf2neg(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa) {
    uint16_t i, k, ah, ac;

    if(ido == 1) {
        for(k = 0; k < l1; k++) {
            ah = 2 * k;
            ac = 4 * k;
            RE(ch[ah]) = RE(cc[ac]) + RE(cc[ac + 1]);
            RE(ch[ah + l1]) = RE(cc[ac]) - RE(cc[ac + 1]);
            IM(ch[ah]) = IM(cc[ac]) + IM(cc[ac + 1]);
            IM(ch[ah + l1]) = IM(cc[ac]) - IM(cc[ac + 1]);
        }
    }
    else {
        for(k = 0; k < l1; k++) {
            ah = k * ido;
            ac = 2 * k * ido;
            for(i = 0; i < ido; i++) {
                complex_t t2;
                RE(ch[ah + i]) = RE(cc[ac + i]) + RE(cc[ac + i + ido]);
                RE(t2) = RE(cc[ac + i]) - RE(cc[ac + i + ido]);
                IM(ch[ah + i]) = IM(cc[ac + i]) + IM(cc[ac + i + ido]);
                IM(t2) = IM(cc[ac + i]) - IM(cc[ac + i + ido]);
                ComplexMult(&RE(ch[ah + i + l1 * ido]), &IM(ch[ah + i + l1 * ido]), RE(t2), IM(t2), RE(wa[i]), IM(wa[i]));
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void passf3(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const int8_t isign) {
    static int32_t taur = FRAC_CONST(-0.5);
    static int32_t taui = FRAC_CONST(0.866025403784439);
    uint16_t       i, k, ac, ah;
    complex_t      c2, c3, d2, d3, t2;

    if(ido == 1) {
        if(isign == 1) {
            for(k = 0; k < l1; k++) {
                ac = 3 * k + 1;
                ah = k;
                RE(t2) = RE(cc[ac]) + RE(cc[ac + 1]);
                IM(t2) = IM(cc[ac]) + IM(cc[ac + 1]);
                RE(c2) = RE(cc[ac - 1]) + MUL_F(RE(t2), taur);
                IM(c2) = IM(cc[ac - 1]) + MUL_F(IM(t2), taur);
                RE(ch[ah]) = RE(cc[ac - 1]) + RE(t2);
                IM(ch[ah]) = IM(cc[ac - 1]) + IM(t2);
                RE(c3) = MUL_F((RE(cc[ac]) - RE(cc[ac + 1])), taui);
                IM(c3) = MUL_F((IM(cc[ac]) - IM(cc[ac + 1])), taui);
                RE(ch[ah + l1]) = RE(c2) - IM(c3);
                IM(ch[ah + l1]) = IM(c2) + RE(c3);
                RE(ch[ah + 2 * l1]) = RE(c2) + IM(c3);
                IM(ch[ah + 2 * l1]) = IM(c2) - RE(c3);
            }
        }
        else {
            for(k = 0; k < l1; k++) {
                ac = 3 * k + 1;
                ah = k;
                RE(t2) = RE(cc[ac]) + RE(cc[ac + 1]);
                IM(t2) = IM(cc[ac]) + IM(cc[ac + 1]);
                RE(c2) = RE(cc[ac - 1]) + MUL_F(RE(t2), taur);
                IM(c2) = IM(cc[ac - 1]) + MUL_F(IM(t2), taur);
                RE(ch[ah]) = RE(cc[ac - 1]) + RE(t2);
                IM(ch[ah]) = IM(cc[ac - 1]) + IM(t2);
                RE(c3) = MUL_F((RE(cc[ac]) - RE(cc[ac + 1])), taui);
                IM(c3) = MUL_F((IM(cc[ac]) - IM(cc[ac + 1])), taui);
                RE(ch[ah + l1]) = RE(c2) + IM(c3);
                IM(ch[ah + l1]) = IM(c2) - RE(c3);
                RE(ch[ah + 2 * l1]) = RE(c2) - IM(c3);
                IM(ch[ah + 2 * l1]) = IM(c2) + RE(c3);
            }
        }
    }
    else {
        if(isign == 1) {
            for(k = 0; k < l1; k++) {
                for(i = 0; i < ido; i++) {
                    ac = i + (3 * k + 1) * ido;
                    ah = i + k * ido;
                    RE(t2) = RE(cc[ac]) + RE(cc[ac + ido]);
                    RE(c2) = RE(cc[ac - ido]) + MUL_F(RE(t2), taur);
                    IM(t2) = IM(cc[ac]) + IM(cc[ac + ido]);
                    IM(c2) = IM(cc[ac - ido]) + MUL_F(IM(t2), taur);
                    RE(ch[ah]) = RE(cc[ac - ido]) + RE(t2);
                    IM(ch[ah]) = IM(cc[ac - ido]) + IM(t2);
                    RE(c3) = MUL_F((RE(cc[ac]) - RE(cc[ac + ido])), taui);
                    IM(c3) = MUL_F((IM(cc[ac]) - IM(cc[ac + ido])), taui);
                    RE(d2) = RE(c2) - IM(c3);
                    IM(d3) = IM(c2) - RE(c3);
                    RE(d3) = RE(c2) + IM(c3);
                    IM(d2) = IM(c2) + RE(c3);
                    ComplexMult(&IM(ch[ah + l1 * ido]), &RE(ch[ah + l1 * ido]), IM(d2), RE(d2), RE(wa1[i]), IM(wa1[i]));
                    ComplexMult(&IM(ch[ah + 2 * l1 * ido]), &RE(ch[ah + 2 * l1 * ido]), IM(d3), RE(d3), RE(wa2[i]), IM(wa2[i]));
                }
            }
        }
        else {
            for(k = 0; k < l1; k++) {
                for(i = 0; i < ido; i++) {
                    ac = i + (3 * k + 1) * ido;
                    ah = i + k * ido;
                    RE(t2) = RE(cc[ac]) + RE(cc[ac + ido]);
                    RE(c2) = RE(cc[ac - ido]) + MUL_F(RE(t2), taur);
                    IM(t2) = IM(cc[ac]) + IM(cc[ac + ido]);
                    IM(c2) = IM(cc[ac - ido]) + MUL_F(IM(t2), taur);
                    RE(ch[ah]) = RE(cc[ac - ido]) + RE(t2);
                    IM(ch[ah]) = IM(cc[ac - ido]) + IM(t2);
                    RE(c3) = MUL_F((RE(cc[ac]) - RE(cc[ac + ido])), taui);
                    IM(c3) = MUL_F((IM(cc[ac]) - IM(cc[ac + ido])), taui);
                    RE(d2) = RE(c2) + IM(c3);
                    IM(d3) = IM(c2) + RE(c3);
                    RE(d3) = RE(c2) - IM(c3);
                    IM(d2) = IM(c2) - RE(c3);
                    ComplexMult(&RE(ch[ah + l1 * ido]), &IM(ch[ah + l1 * ido]), RE(d2), IM(d2), RE(wa1[i]), IM(wa1[i]));
                    ComplexMult(&RE(ch[ah + 2 * l1 * ido]), &IM(ch[ah + 2 * l1 * ido]), RE(d3), IM(d3), RE(wa2[i]), IM(wa2[i]));
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void passf4pos(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3) {
    uint16_t i, k, ac, ah;

    if(ido == 1) {
        for(k = 0; k < l1; k++) {
            complex_t t1, t2, t3, t4;
            ac = 4 * k;
            ah = k;
            RE(t2) = RE(cc[ac]) + RE(cc[ac + 2]);
            RE(t1) = RE(cc[ac]) - RE(cc[ac + 2]);
            IM(t2) = IM(cc[ac]) + IM(cc[ac + 2]);
            IM(t1) = IM(cc[ac]) - IM(cc[ac + 2]);
            RE(t3) = RE(cc[ac + 1]) + RE(cc[ac + 3]);
            IM(t4) = RE(cc[ac + 1]) - RE(cc[ac + 3]);
            IM(t3) = IM(cc[ac + 3]) + IM(cc[ac + 1]);
            RE(t4) = IM(cc[ac + 3]) - IM(cc[ac + 1]);
            RE(ch[ah]) = RE(t2) + RE(t3);
            RE(ch[ah + 2 * l1]) = RE(t2) - RE(t3);
            IM(ch[ah]) = IM(t2) + IM(t3);
            IM(ch[ah + 2 * l1]) = IM(t2) - IM(t3);
            RE(ch[ah + l1]) = RE(t1) + RE(t4);
            RE(ch[ah + 3 * l1]) = RE(t1) - RE(t4);
            IM(ch[ah + l1]) = IM(t1) + IM(t4);
            IM(ch[ah + 3 * l1]) = IM(t1) - IM(t4);
        }
    }
    else {
        for(k = 0; k < l1; k++) {
            ac = 4 * k * ido;
            ah = k * ido;
            for(i = 0; i < ido; i++) {
                complex_t c2, c3, c4, t1, t2, t3, t4;
                RE(t2) = RE(cc[ac + i]) + RE(cc[ac + i + 2 * ido]);
                RE(t1) = RE(cc[ac + i]) - RE(cc[ac + i + 2 * ido]);
                IM(t2) = IM(cc[ac + i]) + IM(cc[ac + i + 2 * ido]);
                IM(t1) = IM(cc[ac + i]) - IM(cc[ac + i + 2 * ido]);
                RE(t3) = RE(cc[ac + i + ido]) + RE(cc[ac + i + 3 * ido]);
                IM(t4) = RE(cc[ac + i + ido]) - RE(cc[ac + i + 3 * ido]);
                IM(t3) = IM(cc[ac + i + 3 * ido]) + IM(cc[ac + i + ido]);
                RE(t4) = IM(cc[ac + i + 3 * ido]) - IM(cc[ac + i + ido]);
                RE(c2) = RE(t1) + RE(t4);
                RE(c4) = RE(t1) - RE(t4);
                IM(c2) = IM(t1) + IM(t4);
                IM(c4) = IM(t1) - IM(t4);
                RE(ch[ah + i]) = RE(t2) + RE(t3);
                RE(c3) = RE(t2) - RE(t3);
                IM(ch[ah + i]) = IM(t2) + IM(t3);
                IM(c3) = IM(t2) - IM(t3);
                ComplexMult(&IM(ch[ah + i + l1 * ido]), &RE(ch[ah + i + l1 * ido]), IM(c2), RE(c2), RE(wa1[i]), IM(wa1[i]));
                ComplexMult(&IM(ch[ah + i + 2 * l1 * ido]), &RE(ch[ah + i + 2 * l1 * ido]), IM(c3), RE(c3), RE(wa2[i]), IM(wa2[i]));
                ComplexMult(&IM(ch[ah + i + 3 * l1 * ido]), &RE(ch[ah + i + 3 * l1 * ido]), IM(c4), RE(c4), RE(wa3[i]), IM(wa3[i]));
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void passf4neg(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3) {
    uint16_t i, k, ac, ah;

    if(ido == 1) {
        for(k = 0; k < l1; k++) {
            complex_t t1, t2, t3, t4;
            ac = 4 * k;
            ah = k;
            RE(t2) = RE(cc[ac]) + RE(cc[ac + 2]);
            RE(t1) = RE(cc[ac]) - RE(cc[ac + 2]);
            IM(t2) = IM(cc[ac]) + IM(cc[ac + 2]);
            IM(t1) = IM(cc[ac]) - IM(cc[ac + 2]);
            RE(t3) = RE(cc[ac + 1]) + RE(cc[ac + 3]);
            IM(t4) = RE(cc[ac + 1]) - RE(cc[ac + 3]);
            IM(t3) = IM(cc[ac + 3]) + IM(cc[ac + 1]);
            RE(t4) = IM(cc[ac + 3]) - IM(cc[ac + 1]);
            RE(ch[ah]) = RE(t2) + RE(t3);
            RE(ch[ah + 2 * l1]) = RE(t2) - RE(t3);
            IM(ch[ah]) = IM(t2) + IM(t3);
            IM(ch[ah + 2 * l1]) = IM(t2) - IM(t3);
            RE(ch[ah + l1]) = RE(t1) - RE(t4);
            RE(ch[ah + 3 * l1]) = RE(t1) + RE(t4);
            IM(ch[ah + l1]) = IM(t1) - IM(t4);
            IM(ch[ah + 3 * l1]) = IM(t1) + IM(t4);
        }
    }
    else {
        for(k = 0; k < l1; k++) {
            ac = 4 * k * ido;
            ah = k * ido;
            for(i = 0; i < ido; i++) {
                complex_t c2, c3, c4, t1, t2, t3, t4;
                RE(t2) = RE(cc[ac + i]) + RE(cc[ac + i + 2 * ido]);
                RE(t1) = RE(cc[ac + i]) - RE(cc[ac + i + 2 * ido]);
                IM(t2) = IM(cc[ac + i]) + IM(cc[ac + i + 2 * ido]);
                IM(t1) = IM(cc[ac + i]) - IM(cc[ac + i + 2 * ido]);
                RE(t3) = RE(cc[ac + i + ido]) + RE(cc[ac + i + 3 * ido]);
                IM(t4) = RE(cc[ac + i + ido]) - RE(cc[ac + i + 3 * ido]);
                IM(t3) = IM(cc[ac + i + 3 * ido]) + IM(cc[ac + i + ido]);
                RE(t4) = IM(cc[ac + i + 3 * ido]) - IM(cc[ac + i + ido]);
                RE(c2) = RE(t1) - RE(t4);
                RE(c4) = RE(t1) + RE(t4);
                IM(c2) = IM(t1) - IM(t4);
                IM(c4) = IM(t1) + IM(t4);
                RE(ch[ah + i]) = RE(t2) + RE(t3);
                RE(c3) = RE(t2) - RE(t3);
                IM(ch[ah + i]) = IM(t2) + IM(t3);
                IM(c3) = IM(t2) - IM(t3);
                ComplexMult(&RE(ch[ah + i + l1 * ido]), &IM(ch[ah + i + l1 * ido]), RE(c2), IM(c2), RE(wa1[i]), IM(wa1[i]));
                ComplexMult(&RE(ch[ah + i + 2 * l1 * ido]), &IM(ch[ah + i + 2 * l1 * ido]), RE(c3), IM(c3), RE(wa2[i]), IM(wa2[i]));
                ComplexMult(&RE(ch[ah + i + 3 * l1 * ido]), &IM(ch[ah + i + 3 * l1 * ido]), RE(c4), IM(c4), RE(wa3[i]), IM(wa3[i]));
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void passf5(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3, const complex_t* wa4, const int8_t isign) {
    static int32_t tr11 = FRAC_CONST(0.309016994374947);
    static int32_t ti11 = FRAC_CONST(0.951056516295154);
    static int32_t tr12 = FRAC_CONST(-0.809016994374947);
    static int32_t ti12 = FRAC_CONST(0.587785252292473);
    uint16_t       i, k, ac, ah;
    complex_t      c2, c3, c4, c5, d3, d4, d5, d2, t2, t3, t4, t5;

    if(ido == 1) {
        if(isign == 1) {
            for(k = 0; k < l1; k++) {
                ac = 5 * k + 1;
                ah = k;
                RE(t2) = RE(cc[ac]) + RE(cc[ac + 3]);
                IM(t2) = IM(cc[ac]) + IM(cc[ac + 3]);
                RE(t3) = RE(cc[ac + 1]) + RE(cc[ac + 2]);
                IM(t3) = IM(cc[ac + 1]) + IM(cc[ac + 2]);
                RE(t4) = RE(cc[ac + 1]) - RE(cc[ac + 2]);
                IM(t4) = IM(cc[ac + 1]) - IM(cc[ac + 2]);
                RE(t5) = RE(cc[ac]) - RE(cc[ac + 3]);
                IM(t5) = IM(cc[ac]) - IM(cc[ac + 3]);
                RE(ch[ah]) = RE(cc[ac - 1]) + RE(t2) + RE(t3);
                IM(ch[ah]) = IM(cc[ac - 1]) + IM(t2) + IM(t3);
                RE(c2) = RE(cc[ac - 1]) + MUL_F(RE(t2), tr11) + MUL_F(RE(t3), tr12);
                IM(c2) = IM(cc[ac - 1]) + MUL_F(IM(t2), tr11) + MUL_F(IM(t3), tr12);
                RE(c3) = RE(cc[ac - 1]) + MUL_F(RE(t2), tr12) + MUL_F(RE(t3), tr11);
                IM(c3) = IM(cc[ac - 1]) + MUL_F(IM(t2), tr12) + MUL_F(IM(t3), tr11);
                ComplexMult(&RE(c5), &RE(c4), ti11, ti12, RE(t5), RE(t4));
                ComplexMult(&IM(c5), &IM(c4), ti11, ti12, IM(t5), IM(t4));
                RE(ch[ah + l1]) = RE(c2) - IM(c5);
                IM(ch[ah + l1]) = IM(c2) + RE(c5);
                RE(ch[ah + 2 * l1]) = RE(c3) - IM(c4);
                IM(ch[ah + 2 * l1]) = IM(c3) + RE(c4);
                RE(ch[ah + 3 * l1]) = RE(c3) + IM(c4);
                IM(ch[ah + 3 * l1]) = IM(c3) - RE(c4);
                RE(ch[ah + 4 * l1]) = RE(c2) + IM(c5);
                IM(ch[ah + 4 * l1]) = IM(c2) - RE(c5);
            }
        }
        else {
            for(k = 0; k < l1; k++) {
                ac = 5 * k + 1;
                ah = k;
                RE(t2) = RE(cc[ac]) + RE(cc[ac + 3]);
                IM(t2) = IM(cc[ac]) + IM(cc[ac + 3]);
                RE(t3) = RE(cc[ac + 1]) + RE(cc[ac + 2]);
                IM(t3) = IM(cc[ac + 1]) + IM(cc[ac + 2]);
                RE(t4) = RE(cc[ac + 1]) - RE(cc[ac + 2]);
                IM(t4) = IM(cc[ac + 1]) - IM(cc[ac + 2]);
                RE(t5) = RE(cc[ac]) - RE(cc[ac + 3]);
                IM(t5) = IM(cc[ac]) - IM(cc[ac + 3]);
                RE(ch[ah]) = RE(cc[ac - 1]) + RE(t2) + RE(t3);
                IM(ch[ah]) = IM(cc[ac - 1]) + IM(t2) + IM(t3);
                RE(c2) = RE(cc[ac - 1]) + MUL_F(RE(t2), tr11) + MUL_F(RE(t3), tr12);
                IM(c2) = IM(cc[ac - 1]) + MUL_F(IM(t2), tr11) + MUL_F(IM(t3), tr12);
                RE(c3) = RE(cc[ac - 1]) + MUL_F(RE(t2), tr12) + MUL_F(RE(t3), tr11);
                IM(c3) = IM(cc[ac - 1]) + MUL_F(IM(t2), tr12) + MUL_F(IM(t3), tr11);
                ComplexMult(&RE(c4), &RE(c5), ti12, ti11, RE(t5), RE(t4));
                ComplexMult(&IM(c4), &IM(c5), ti12, ti11, IM(t5), IM(t4));
                RE(ch[ah + l1]) = RE(c2) + IM(c5);
                IM(ch[ah + l1]) = IM(c2) - RE(c5);
                RE(ch[ah + 2 * l1]) = RE(c3) + IM(c4);
                IM(ch[ah + 2 * l1]) = IM(c3) - RE(c4);
                RE(ch[ah + 3 * l1]) = RE(c3) - IM(c4);
                IM(ch[ah + 3 * l1]) = IM(c3) + RE(c4);
                RE(ch[ah + 4 * l1]) = RE(c2) - IM(c5);
                IM(ch[ah + 4 * l1]) = IM(c2) + RE(c5);
            }
        }
    }
    else {
        if(isign == 1) {
            for(k = 0; k < l1; k++) {
                for(i = 0; i < ido; i++) {
                    ac = i + (k * 5 + 1) * ido;
                    ah = i + k * ido;
                    RE(t2) = RE(cc[ac]) + RE(cc[ac + 3 * ido]);
                    IM(t2) = IM(cc[ac]) + IM(cc[ac + 3 * ido]);
                    RE(t3) = RE(cc[ac + ido]) + RE(cc[ac + 2 * ido]);
                    IM(t3) = IM(cc[ac + ido]) + IM(cc[ac + 2 * ido]);
                    RE(t4) = RE(cc[ac + ido]) - RE(cc[ac + 2 * ido]);
                    IM(t4) = IM(cc[ac + ido]) - IM(cc[ac + 2 * ido]);
                    RE(t5) = RE(cc[ac]) - RE(cc[ac + 3 * ido]);
                    IM(t5) = IM(cc[ac]) - IM(cc[ac + 3 * ido]);
                    RE(ch[ah]) = RE(cc[ac - ido]) + RE(t2) + RE(t3);
                    IM(ch[ah]) = IM(cc[ac - ido]) + IM(t2) + IM(t3);
                    RE(c2) = RE(cc[ac - ido]) + MUL_F(RE(t2), tr11) + MUL_F(RE(t3), tr12);
                    IM(c2) = IM(cc[ac - ido]) + MUL_F(IM(t2), tr11) + MUL_F(IM(t3), tr12);
                    RE(c3) = RE(cc[ac - ido]) + MUL_F(RE(t2), tr12) + MUL_F(RE(t3), tr11);
                    IM(c3) = IM(cc[ac - ido]) + MUL_F(IM(t2), tr12) + MUL_F(IM(t3), tr11);
                    ComplexMult(&RE(c5), &RE(c4), ti11, ti12, RE(t5), RE(t4));
                    ComplexMult(&IM(c5), &IM(c4), ti11, ti12, IM(t5), IM(t4));
                    IM(d2) = IM(c2) + RE(c5);
                    IM(d3) = IM(c3) + RE(c4);
                    RE(d4) = RE(c3) + IM(c4);
                    RE(d5) = RE(c2) + IM(c5);
                    RE(d2) = RE(c2) - IM(c5);
                    IM(d5) = IM(c2) - RE(c5);
                    RE(d3) = RE(c3) - IM(c4);
                    IM(d4) = IM(c3) - RE(c4);
                    ComplexMult(&IM(ch[ah + l1 * ido]), &RE(ch[ah + l1 * ido]), IM(d2), RE(d2), RE(wa1[i]), IM(wa1[i]));
                    ComplexMult(&IM(ch[ah + 2 * l1 * ido]), &RE(ch[ah + 2 * l1 * ido]), IM(d3), RE(d3), RE(wa2[i]), IM(wa2[i]));
                    ComplexMult(&IM(ch[ah + 3 * l1 * ido]), &RE(ch[ah + 3 * l1 * ido]), IM(d4), RE(d4), RE(wa3[i]), IM(wa3[i]));
                    ComplexMult(&IM(ch[ah + 4 * l1 * ido]), &RE(ch[ah + 4 * l1 * ido]), IM(d5), RE(d5), RE(wa4[i]), IM(wa4[i]));
                }
            }
        }
        else {
            for(k = 0; k < l1; k++) {
                for(i = 0; i < ido; i++) {
                    ac = i + (k * 5 + 1) * ido;
                    ah = i + k * ido;
                    RE(t2) = RE(cc[ac]) + RE(cc[ac + 3 * ido]);
                    IM(t2) = IM(cc[ac]) + IM(cc[ac + 3 * ido]);
                    RE(t3) = RE(cc[ac + ido]) + RE(cc[ac + 2 * ido]);
                    IM(t3) = IM(cc[ac + ido]) + IM(cc[ac + 2 * ido]);
                    RE(t4) = RE(cc[ac + ido]) - RE(cc[ac + 2 * ido]);
                    IM(t4) = IM(cc[ac + ido]) - IM(cc[ac + 2 * ido]);
                    RE(t5) = RE(cc[ac]) - RE(cc[ac + 3 * ido]);
                    IM(t5) = IM(cc[ac]) - IM(cc[ac + 3 * ido]);
                    RE(ch[ah]) = RE(cc[ac - ido]) + RE(t2) + RE(t3);
                    IM(ch[ah]) = IM(cc[ac - ido]) + IM(t2) + IM(t3);
                    RE(c2) = RE(cc[ac - ido]) + MUL_F(RE(t2), tr11) + MUL_F(RE(t3), tr12);
                    IM(c2) = IM(cc[ac - ido]) + MUL_F(IM(t2), tr11) + MUL_F(IM(t3), tr12);
                    RE(c3) = RE(cc[ac - ido]) + MUL_F(RE(t2), tr12) + MUL_F(RE(t3), tr11);
                    IM(c3) = IM(cc[ac - ido]) + MUL_F(IM(t2), tr12) + MUL_F(IM(t3), tr11);
                    ComplexMult(&RE(c4), &RE(c5), ti12, ti11, RE(t5), RE(t4));
                    ComplexMult(&IM(c4), &IM(c5), ti12, ti11, IM(t5), IM(t4));
                    IM(d2) = IM(c2) - RE(c5);
                    IM(d3) = IM(c3) - RE(c4);
                    RE(d4) = RE(c3) - IM(c4);
                    RE(d5) = RE(c2) - IM(c5);
                    RE(d2) = RE(c2) + IM(c5);
                    IM(d5) = IM(c2) + RE(c5);
                    RE(d3) = RE(c3) + IM(c4);
                    IM(d4) = IM(c3) + RE(c4);
                    ComplexMult(&RE(ch[ah + l1 * ido]), &IM(ch[ah + l1 * ido]), RE(d2), IM(d2), RE(wa1[i]), IM(wa1[i]));
                    ComplexMult(&RE(ch[ah + 2 * l1 * ido]), &IM(ch[ah + 2 * l1 * ido]), RE(d3), IM(d3), RE(wa2[i]), IM(wa2[i]));
                    ComplexMult(&RE(ch[ah + 3 * l1 * ido]), &IM(ch[ah + 3 * l1 * ido]), RE(d4), IM(d4), RE(wa3[i]), IM(wa3[i]));
                    ComplexMult(&RE(ch[ah + 4 * l1 * ido]), &IM(ch[ah + 4 * l1 * ido]), RE(d5), IM(d5), RE(wa4[i]), IM(wa4[i]));
                }
            }
        }
    }
}

/*----------------------------------------------------------------------
   cfftf1, cfftf, cfftb, cffti1, cffti. Complex FFTs.
  ----------------------------------------------------------------------*/

static void cfftf1pos(uint16_t n, complex_t* c, complex_t* ch, const uint16_t* ifac, const complex_t* wa, const int8_t isign) {
    uint16_t i;
    uint16_t k1, l1, l2;
    uint16_t na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;
    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;
    for(k1 = 2; k1 <= nf + 1; k1++) {
        ip = ifac[k1];
        l2 = ip * l1;
        ido = n / l2;
        idl1 = ido * l1;
        switch(ip) {
        case 4:
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            if(na == 0) passf4pos((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], &wa[ix3]);
            else
                passf4pos((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], &wa[ix3]);
            na = 1 - na;
            break;
        case 2:
            if(na == 0) passf2pos((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw]);
            else
                passf2pos((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw]);

            na = 1 - na;
            break;
        case 3:
            ix2 = iw + ido;
            if(na == 0) passf3((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], isign);
            else
                passf3((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], isign);

            na = 1 - na;
            break;
        case 5:
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            ix4 = ix3 + ido;
            if(na == 0) passf5((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
            else
                passf5((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
            na = 1 - na;
            break;
        }
        l1 = l2;
        iw += (ip - 1) * ido;
    }
    if(na == 0) return;
    for(i = 0; i < n; i++) {
        RE(c[i]) = RE(ch[i]);
        IM(c[i]) = IM(ch[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void cfftf1neg(uint16_t n, complex_t* c, complex_t* ch, const uint16_t* ifac, const complex_t* wa, const int8_t isign) {
    uint16_t i;
    uint16_t k1, l1, l2;
    uint16_t na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;
    for(k1 = 2; k1 <= nf + 1; k1++) {
        ip = ifac[k1];
        l2 = ip * l1;
        ido = n / l2;
        idl1 = ido * l1;
        switch(ip) {
        case 4:
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            if(na == 0) passf4neg((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], &wa[ix3]);
            else
                passf4neg((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], &wa[ix3]);
            na = 1 - na;
            break;
        case 2:
            if(na == 0) passf2neg((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw]);
            else
                passf2neg((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw]);
            na = 1 - na;
            break;
        case 3:
            ix2 = iw + ido;
            if(na == 0) passf3((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], isign);
            else
                passf3((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], isign);
            na = 1 - na;
            break;
        case 5:
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            ix4 = ix3 + ido;
            if(na == 0) passf5((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)c, ch, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
            else
                passf5((const uint16_t)ido, (const uint16_t)l1, (const complex_t*)ch, c, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
            na = 1 - na;
            break;
        }
        l1 = l2;
        iw += (ip - 1) * ido;
    }
    if(na == 0) return;
    for(i = 0; i < n; i++) {
        RE(c[i]) = RE(ch[i]);
        IM(c[i]) = IM(ch[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void cfftf(cfft_info* cfft, complex_t* c) { cfftf1neg(cfft->n, c, cfft->work, (const uint16_t*)cfft->ifac, (const complex_t*)cfft->tab, -1); }

//----------------------------------------------------------------------------------------------------------------------------------------------------
void cfftb(cfft_info* cfft, complex_t* c) { cfftf1pos(cfft->n, c, cfft->work, (const uint16_t*)cfft->ifac, (const complex_t*)cfft->tab, +1); }

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void cffti1(uint16_t n, complex_t* wa, uint16_t* ifac) {
    static uint16_t ntryh[4] = {3, 4, 2, 5};
    uint16_t ntry = 0, i, j;
    uint16_t ib;
    uint16_t nf, nl, nq, nr;

    nl = n;
    nf = 0;
    j = 0;

startloop:
    j++;
    if(j <= 4) ntry = ntryh[j - 1];
    else
        ntry += 2;
    do {
        nq = nl / ntry;
        nr = nl - ntry * nq;
        if(nr != 0) goto startloop;
        nf++;
        ifac[nf + 1] = ntry;
        nl = nq;
        if(ntry == 2 && nf != 1) {
            for(i = 2; i <= nf; i++) {
                ib = nf - i + 2;
                ifac[ib + 1] = ifac[ib];
            }
            ifac[2] = 2;
        }
    } while(nl != 1);
    ifac[0] = n;
    ifac[1] = nf;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
cfft_info* cffti(uint16_t n) {
    cfft_info* cfft = (cfft_info*)faad_malloc(sizeof(cfft_info));

    cfft->n = n;
    cfft->work = (complex_t*)faad_malloc(n * sizeof(complex_t));
    cffti1(n, NULL, cfft->ifac);
    switch(n) {
    case 64: cfft->tab = (complex_t*)cfft_tab_64; break;
    case 512: cfft->tab = (complex_t*)cfft_tab_512; break;
    #ifdef LD_DEC
    case 256: cfft->tab = (complex_t*)cfft_tab_256; break;
    #endif
    #ifdef ALLOW_SMALL_FRAMELENGTH
    case 60: cfft->tab = (complex_t*)cfft_tab_60; break;
    case 480: cfft->tab = (complex_t*)cfft_tab_480; break;
        #ifdef LD_DEC
    case 240: cfft->tab = (complex_t*)cfft_tab_240; break;
        #endif
    #endif
    case 128: cfft->tab = (complex_t*)cfft_tab_128; break;
    }
    return cfft;
}

void cfftu(cfft_info* cfft) {
    if(cfft->work) faad_free(cfft->work);
    if(cfft) faad_free(cfft);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t get_sr_index(const uint32_t samplerate) { /* Returns the sample rate index based on the samplerate */
    if(92017 <= samplerate) return 0;
    if(75132 <= samplerate) return 1;
    if(55426 <= samplerate) return 2;
    if(46009 <= samplerate) return 3;
    if(37566 <= samplerate) return 4;
    if(27713 <= samplerate) return 5;
    if(23004 <= samplerate) return 6;
    if(18783 <= samplerate) return 7;
    if(13856 <= samplerate) return 8;
    if(11502 <= samplerate) return 9;
    if(9391 <= samplerate) return 10;
    if(16428320 <= samplerate) return 11;
    return 11;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t get_sample_rate(const uint8_t sr_index) { /* Returns the sample rate based on the sample rate index */
    static const uint32_t sample_rates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};
    if(sr_index < 12) return sample_rates[sr_index];
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t max_pred_sfb(const uint8_t sr_index) {
    static const uint8_t pred_sfb_max[] = {33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34};
    if(sr_index < 12) return pred_sfb_max[sr_index];
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t max_tns_sfb(const uint8_t sr_index, const uint8_t object_type, const uint8_t is_short) {
    /* entry for each sampling rate
     * 1    Main/LC long window
     * 2    Main/LC short window
     * 3    SSR long window
     * 4    SSR short window
     */
    static const uint8_t tns_sbf_max[][4] = {{31, 9, 28, 7},  /* 96000 */
                                             {31, 9, 28, 7},  /* 88200 */
                                             {34, 10, 27, 7}, /* 64000 */
                                             {40, 14, 26, 6}, /* 48000 */
                                             {42, 14, 26, 6}, /* 44100 */
                                             {51, 14, 26, 6}, /* 32000 */
                                             {46, 14, 29, 7}, /* 24000 */
                                             {46, 14, 29, 7}, /* 22050 */
                                             {42, 14, 23, 8}, /* 16000 */
                                             {42, 14, 23, 8}, /* 12000 */
                                             {42, 14, 23, 8}, /* 11025 */
                                             {39, 14, 19, 7}, /*  8000 */
                                             {39, 14, 19, 7}, /*  7350 */
                                             {0, 0, 0, 0},    {0, 0, 0, 0}, {0, 0, 0, 0}};
    uint8_t              i = 0;
    if(is_short) i++;
    if(object_type == SSR) i += 2;
    return tns_sbf_max[sr_index][i];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t can_decode_ot(const uint8_t object_type) { /* Returns 0 if an object type is decodable, otherwise returns -1 */
    switch(object_type) {
    case LC: return 0;
    case MAIN: return -1;
    case SSR: return -1;

    case LTP:
#ifdef LTP_DEC
        return 0;
#else
        return -1;
#endif
#ifdef ERROR_RESILIENCE /* ER object types */
    case ER_LC: return 0;
    case ER_LTP:
    #ifdef LTP_DEC
        return 0;
    #else
        return -1;
    #endif
    case LD:
    #ifdef LD_DEC
        return 0;
    #else
        return -1;
    #endif
#endif
    }
    return -1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void* faad_malloc(size_t size) { return malloc(size); }

//----------------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------------------------------------
static const uint8_t Parity[256] = { // parity
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1,
    0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1,
    0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

static uint32_t __r1 = 1;
static uint32_t __r2 = 1;

/*
 *  This is a simple random number generator with good quality for audio purposes.
 *  It consists of two polycounters with opposite rotation direction and different
 *  periods. The periods are coprime, so the total period is the product of both.
 *
 *     -------------------------------------------------------------------------------------------------
 * +-> |31:30:29:28:27:26:25:24:23:22:21:20:19:18:17:16:15:14:13:12:11:10: 9: 8: 7: 6: 5: 4: 3: 2: 1: 0|
 * |   -------------------------------------------------------------------------------------------------
 * |                                                                          |  |  |  |     |        |
 * |                                                                          +--+--+--+-XOR-+--------+
 * |                                                                                      |
 * +--------------------------------------------------------------------------------------+
 *
 *     -------------------------------------------------------------------------------------------------
 *     |31:30:29:28:27:26:25:24:23:22:21:20:19:18:17:16:15:14:13:12:11:10: 9: 8: 7: 6: 5: 4: 3: 2: 1: 0| <-+
 *     -------------------------------------------------------------------------------------------------   |
 *       |  |           |  |                                                                               |
 *       +--+----XOR----+--+                                                                               |
 *                |                                                                                        |
 *                +----------------------------------------------------------------------------------------+
 *
 *
 *  The first has an period of 3*5*17*257*65537, the second of 7*47*73*178481,
 *  which gives a period of 18.410.713.077.675.721.215. The result is the
 *  XORed values of both generators.
 */
uint32_t ne_rng(uint32_t* __r1, uint32_t* __r2) {
    uint32_t t1, t2, t3, t4;

    t3 = t1 = *__r1;
    t4 = t2 = *__r2; // Parity calculation is done via table lookup, this is also available
    t1 &= 0xF5;
    t2 >>= 25; // on CPUs without parity, can be implemented in C and avoid unpredictable
    t1 = Parity[t1];
    t2 &= 0x63; // jumps and slow rotate through the carry flag operations.
    t1 <<= 31;
    t2 = Parity[t2];

    return (*__r1 = (t3 >> 1) | t1) ^ (*__r2 = (t4 + t4) | t2);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t ones32(uint32_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);

    return (x & 0x0000003f);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t floor_log2(uint32_t x) {

    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);

    return (ones32(x) - 1);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* returns position of first bit that is not 0 from msb,
 * starting count at lsb */
uint32_t wl_min_lzc(uint32_t x) {

    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);

    return (ones32(x));
}
#define TABLE_BITS 6
/* just take the maximum number of bits for interpolation */
#define INTERP_BITS (REAL_BITS - TABLE_BITS)

static const int32_t pow2_tab[] = {
    REAL_CONST(1.000000000000000), REAL_CONST(1.010889286051701), REAL_CONST(1.021897148654117), REAL_CONST(1.033024879021228),
    REAL_CONST(1.044273782427414), REAL_CONST(1.055645178360557), REAL_CONST(1.067140400676824), REAL_CONST(1.078760797757120),
    REAL_CONST(1.090507732665258), REAL_CONST(1.102382583307841), REAL_CONST(1.114386742595892), REAL_CONST(1.126521618608242),
    REAL_CONST(1.138788634756692), REAL_CONST(1.151189229952983), REAL_CONST(1.163724858777578), REAL_CONST(1.176396991650281),
    REAL_CONST(1.189207115002721), REAL_CONST(1.202156731452703), REAL_CONST(1.215247359980469), REAL_CONST(1.228480536106870),
    REAL_CONST(1.241857812073484), REAL_CONST(1.255380757024691), REAL_CONST(1.269050957191733), REAL_CONST(1.282870016078778),
    REAL_CONST(1.296839554651010), REAL_CONST(1.310961211524764), REAL_CONST(1.325236643159741), REAL_CONST(1.339667524053303),
    REAL_CONST(1.354255546936893), REAL_CONST(1.369002422974591), REAL_CONST(1.383909881963832), REAL_CONST(1.398979672538311),
    REAL_CONST(1.414213562373095), REAL_CONST(1.429613338391970), REAL_CONST(1.445180806977047), REAL_CONST(1.460917794180647),
    REAL_CONST(1.476826145939499), REAL_CONST(1.492907728291265), REAL_CONST(1.509164427593423), REAL_CONST(1.525598150744538),
    REAL_CONST(1.542210825407941), REAL_CONST(1.559004400237837), REAL_CONST(1.575980845107887), REAL_CONST(1.593142151342267),
    REAL_CONST(1.610490331949254), REAL_CONST(1.628027421857348), REAL_CONST(1.645755478153965), REAL_CONST(1.663676580326736),
    REAL_CONST(1.681792830507429), REAL_CONST(1.700106353718524), REAL_CONST(1.718619298122478), REAL_CONST(1.737333835273706),
    REAL_CONST(1.756252160373300), REAL_CONST(1.775376492526521), REAL_CONST(1.794709075003107), REAL_CONST(1.814252175500399),
    REAL_CONST(1.834008086409342), REAL_CONST(1.853979125083386), REAL_CONST(1.874167634110300), REAL_CONST(1.894575981586966),
    REAL_CONST(1.915206561397147), REAL_CONST(1.936061793492294), REAL_CONST(1.957144124175400), REAL_CONST(1.978456026387951),
    REAL_CONST(2.000000000000000)};

static const int32_t log2_tab[] = {
    REAL_CONST(0.000000000000000), REAL_CONST(0.022367813028455), REAL_CONST(0.044394119358453), REAL_CONST(0.066089190457772),
    REAL_CONST(0.087462841250339), REAL_CONST(0.108524456778169), REAL_CONST(0.129283016944966), REAL_CONST(0.149747119504682),
    REAL_CONST(0.169925001442312), REAL_CONST(0.189824558880017), REAL_CONST(0.209453365628950), REAL_CONST(0.228818690495881),
    REAL_CONST(0.247927513443585), REAL_CONST(0.266786540694901), REAL_CONST(0.285402218862248), REAL_CONST(0.303780748177103),
    REAL_CONST(0.321928094887362), REAL_CONST(0.339850002884625), REAL_CONST(0.357552004618084), REAL_CONST(0.375039431346925),
    REAL_CONST(0.392317422778760), REAL_CONST(0.409390936137702), REAL_CONST(0.426264754702098), REAL_CONST(0.442943495848728),
    REAL_CONST(0.459431618637297), REAL_CONST(0.475733430966398), REAL_CONST(0.491853096329675), REAL_CONST(0.507794640198696),
    REAL_CONST(0.523561956057013), REAL_CONST(0.539158811108031), REAL_CONST(0.554588851677637), REAL_CONST(0.569855608330948),
    REAL_CONST(0.584962500721156), REAL_CONST(0.599912842187128), REAL_CONST(0.614709844115208), REAL_CONST(0.629356620079610),
    REAL_CONST(0.643856189774725), REAL_CONST(0.658211482751795), REAL_CONST(0.672425341971496), REAL_CONST(0.686500527183218),
    REAL_CONST(0.700439718141092), REAL_CONST(0.714245517666123), REAL_CONST(0.727920454563199), REAL_CONST(0.741466986401147),
    REAL_CONST(0.754887502163469), REAL_CONST(0.768184324776926), REAL_CONST(0.781359713524660), REAL_CONST(0.794415866350106),
    REAL_CONST(0.807354922057604), REAL_CONST(0.820178962415188), REAL_CONST(0.832890014164742), REAL_CONST(0.845490050944375),
    REAL_CONST(0.857980995127572), REAL_CONST(0.870364719583405), REAL_CONST(0.882643049361841), REAL_CONST(0.894817763307943),
    REAL_CONST(0.906890595608519), REAL_CONST(0.918863237274595), REAL_CONST(0.930737337562886), REAL_CONST(0.942514505339240),
    REAL_CONST(0.954196310386875), REAL_CONST(0.965784284662087), REAL_CONST(0.977279923499917), REAL_CONST(0.988684686772166),
    REAL_CONST(1.000000000000000)};

//----------------------------------------------------------------------------------------------------------------------------------------------------
int32_t pow2_fix(int32_t val) {
    uint32_t x1, x2;
    uint32_t errcorr;
    uint32_t index_frac;
    int32_t  retval;
    int32_t  whole = (val >> REAL_BITS);

    /* rest = [0..1] */
    int32_t rest = val - (whole << REAL_BITS);

    /* index into pow2_tab */
    int32_t index = rest >> (REAL_BITS - TABLE_BITS);

    if(val == 0) return (1 << REAL_BITS);

    /* leave INTERP_BITS bits */
    index_frac = rest >> (REAL_BITS - TABLE_BITS - INTERP_BITS);
    index_frac = index_frac & ((1 << INTERP_BITS) - 1);

    if(whole > 0) { retval = 1 << whole; }
    else { retval = REAL_CONST(1) >> -whole; }

    x1 = pow2_tab[index & ((1 << TABLE_BITS) - 1)];
    x2 = pow2_tab[(index & ((1 << TABLE_BITS) - 1)) + 1];
    errcorr = ((index_frac * (x2 - x1))) >> INTERP_BITS;

    if(whole > 0) { retval = retval * (errcorr + x1); }
    else { retval = MUL_R(retval, (errcorr + x1)); }

    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
int32_t pow2_int(int32_t val) {
    uint32_t x1, x2;
    uint32_t errcorr;
    uint32_t index_frac;
    int32_t  retval;
    int32_t  whole = (val >> REAL_BITS);

    /* rest = [0..1] */
    int32_t rest = val - (whole << REAL_BITS);

    /* index into pow2_tab */
    int32_t index = rest >> (REAL_BITS - TABLE_BITS);

    if(val == 0) return 1;

    /* leave INTERP_BITS bits */
    index_frac = rest >> (REAL_BITS - TABLE_BITS - INTERP_BITS);
    index_frac = index_frac & ((1 << INTERP_BITS) - 1);

    if(whole > 0) retval = 1 << whole;
    else
        retval = 0;

    x1 = pow2_tab[index & ((1 << TABLE_BITS) - 1)];
    x2 = pow2_tab[(index & ((1 << TABLE_BITS) - 1)) + 1];
    errcorr = ((index_frac * (x2 - x1))) >> INTERP_BITS;

    retval = MUL_R(retval, (errcorr + x1));

    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* ld(x) = ld(x*y/y) = ld(x/y) + ld(y), with y=2^N and [1 <= (x/y) < 2] */
int32_t log2_int(uint32_t val) {
    uint32_t frac;
    uint32_t whole = (val);
    int32_t  exp = 0;
    uint32_t index;
    uint32_t index_frac;
    uint32_t x1, x2;
    uint32_t errcorr;

    /* error */
    if(val == 0) return -10000;

    exp = floor_log2(val);
    exp -= REAL_BITS;

    /* frac = [1..2] */
    if(exp >= 0) frac = val >> exp;
    else
        frac = val << -exp;

    /* index in the log2 table */
    index = frac >> (REAL_BITS - TABLE_BITS);

    /* leftover part for linear interpolation */
    index_frac = frac & ((1 << (REAL_BITS - TABLE_BITS)) - 1);

    /* leave INTERP_BITS bits */
    index_frac = index_frac >> (REAL_BITS - TABLE_BITS - INTERP_BITS);

    x1 = log2_tab[index & ((1 << TABLE_BITS) - 1)];
    x2 = log2_tab[(index & ((1 << TABLE_BITS) - 1)) + 1];

    /* linear interpolation */
    /* retval = exp + ((index_frac)*x2 + (1-index_frac)*x1) */

    errcorr = (index_frac * (x2 - x1)) >> INTERP_BITS;

    return ((exp + REAL_BITS) << REAL_BITS) + errcorr + x1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* ld(x) = ld(x*y/y) = ld(x/y) + ld(y), with y=2^N and [1 <= (x/y) < 2] */
int32_t log2_fix(uint32_t val) {
    uint32_t frac;
    uint32_t whole = (val >> REAL_BITS);
    int8_t   exp = 0;
    uint32_t index;
    uint32_t index_frac;
    uint32_t x1, x2;
    uint32_t errcorr;

    if(val == 0) return -100000; /* error */
    exp = floor_log2(val);
    exp -= REAL_BITS;
    if(exp >= 0) frac = val >> exp; /* frac = [1..2] */
    else { frac = val << -exp; }
    index = frac >> (REAL_BITS - TABLE_BITS);                          /* index in the log2 table */
    index_frac = frac & ((1 << (REAL_BITS - TABLE_BITS)) - 1);         /* leftover part for linear interpolation */
    index_frac = index_frac >> (REAL_BITS - TABLE_BITS - INTERP_BITS); /* leave INTERP_BITS bits */
    x1 = log2_tab[index & ((1 << TABLE_BITS) - 1)];
    x2 = log2_tab[(index & ((1 << TABLE_BITS) - 1)) + 1];
    /* linear interpolation */
    /* retval = exp + ((index_frac)*x2 + (1-index_frac)*x1) */
    errcorr = (index_frac * (x2 - x1)) >> INTERP_BITS;
    return (exp << REAL_BITS) + errcorr + x1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* TNS decoding for one channel and frame */
void tns_decode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len) {
    uint8_t  w, f, tns_order;
    int8_t   inc;
    int16_t  size;
    uint16_t bottom, top, start, end;
    uint16_t nshort = frame_len / 8;
    int32_t  lpc[TNS_MAX_ORDER + 1];

    if(!ics->tns_data_present) return;

    for(w = 0; w < ics->num_windows; w++) {
        bottom = ics->num_swb;
        for(f = 0; f < tns->n_filt[w]; f++) {
            top = bottom;
            bottom = max(top - tns->length[w][f], 0);
            tns_order = min(tns->order[w][f], TNS_MAX_ORDER);
            if(!tns_order) continue;
            tns_decode_coef(tns_order, tns->coef_res[w] + 3, tns->coef_compress[w][f], tns->coef[w][f], lpc);
            start = min(bottom, max_tns_sfb(sr_index, object_type, (ics->window_sequence == EIGHT_SHORT_SEQUENCE)));
            start = min(start, ics->max_sfb);
            start = min(ics->swb_offset[start], ics->swb_offset_max);
            end = min(top, max_tns_sfb(sr_index, object_type, (ics->window_sequence == EIGHT_SHORT_SEQUENCE)));
            end = min(end, ics->max_sfb);
            end = min(ics->swb_offset[end], ics->swb_offset_max);
            size = end - start;
            if(size <= 0) continue;
            if(tns->direction[w][f]) {
                inc = -1;
                start = end - 1;
            }
            else { inc = 1; }
            tns_ar_filter(&spec[(w * nshort) + start], size, inc, lpc, tns_order);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* TNS encoding for one channel and frame */
void tns_encode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len) {
    uint8_t  w, f, tns_order;
    int8_t   inc;
    int16_t  size;
    uint16_t bottom, top, start, end;
    uint16_t nshort = frame_len / 8;
    int32_t  lpc[TNS_MAX_ORDER + 1];

    if(!ics->tns_data_present) return;
    for(w = 0; w < ics->num_windows; w++) {
        bottom = ics->num_swb;
        for(f = 0; f < tns->n_filt[w]; f++) {
            top = bottom;
            bottom = max(top - tns->length[w][f], 0);
            tns_order = min(tns->order[w][f], TNS_MAX_ORDER);
            if(!tns_order) continue;
            tns_decode_coef(tns_order, tns->coef_res[w] + 3, tns->coef_compress[w][f], tns->coef[w][f], lpc);
            start = min(bottom, max_tns_sfb(sr_index, object_type, (ics->window_sequence == EIGHT_SHORT_SEQUENCE)));
            start = min(start, ics->max_sfb);
            start = min(ics->swb_offset[start], ics->swb_offset_max);
            end = min(top, max_tns_sfb(sr_index, object_type, (ics->window_sequence == EIGHT_SHORT_SEQUENCE)));
            end = min(end, ics->max_sfb);
            end = min(ics->swb_offset[end], ics->swb_offset_max);
            size = end - start;
            if(size <= 0) continue;
            if(tns->direction[w][f]) {
                inc = -1;
                start = end - 1;
            }
            else { inc = 1; }
            tns_ma_filter(&spec[(w * nshort) + start], size, inc, lpc, tns_order);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* Decoder transmitted coefficients for one TNS filter */
static void tns_decode_coef(uint8_t order, uint8_t coef_res_bits, uint8_t coef_compress, uint8_t* coef, int32_t* a) {
    uint8_t i, m;
    int32_t tmp2[TNS_MAX_ORDER + 1], b[TNS_MAX_ORDER + 1];

    /* Conversion to signed integer */
    for(i = 0; i < order; i++) {
        if(coef_compress == 0) {
            if(coef_res_bits == 3) { tmp2[i] = tns_coef_0_3[coef[i]]; }
            else { tmp2[i] = tns_coef_0_4[coef[i]]; }
        }
        else {
            if(coef_res_bits == 3) { tmp2[i] = tns_coef_1_3[coef[i]]; }
            else { tmp2[i] = tns_coef_1_4[coef[i]]; }
        }
    }
    /* Conversion to LPC coefficients */
    a[0] = COEF_CONST(1.0);
    for(m = 1; m <= order; m++) {
        for(i = 1; i < m; i++) { b[i] = a[i] + MUL_C(tmp2[m - 1], a[m - i]); } /* loop only while i<m */
        for(i = 1; i < m; i++) { a[i] = b[i]; }                                /* loop only while i<m */
        a[m] = tmp2[m - 1]; /* changed */
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void tns_ar_filter(int32_t* spectrum, uint16_t size, int8_t inc, int32_t* lpc, uint8_t order) {
    /*
     - Simple all-pole filter of order "order" defined by y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order)
     - The state variables of the filter are initialized to zero every time
     - The output data is written over the input data ("in-place operation")
     - An input vector of "size" samples is processed and the index increment to the next data sample is given by "inc"
    */

    uint8_t  j;
    uint16_t i;
    int32_t  y;
    /* state is stored as a double ringbuffer */
    int32_t state[2 * TNS_MAX_ORDER] = {0};
    int8_t  state_index = 0;

    for(i = 0; i < size; i++) {
        y = *spectrum;
        for(j = 0; j < order; j++) y -= MUL_C(state[state_index + j], lpc[j + 1]);
        /* double ringbuffer state */
        state_index--;
        if(state_index < 0) state_index = order - 1;
        state[state_index] = state[state_index + order] = y;
        *spectrum = y;
        spectrum += inc;
// #define TNS_PRINT
#ifdef TNS_PRINT
        // printf("%d\n", y);
        printf("0x%.8X\n", y);
#endif
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void tns_ma_filter(int32_t* spectrum, uint16_t size, int8_t inc, int32_t* lpc, uint8_t order) {
    /*
     - Simple all-zero filter of order "order" defined by y(n) =  x(n) + a(2)*x(n-1) + ... + a(order+1)*x(n-order)
     - The state variables of the filter are initialized to zero every time
     - The output data is written over the input data ("in-place operation")
     - An input vector of "size" samples is processed and the index increment to the next data sample is given by "inc"
    */

    uint8_t  j;
    uint16_t i;
    int32_t  y;
    /* state is stored as a double ringbuffer */
    int32_t state[2 * TNS_MAX_ORDER] = {0};
    int8_t  state_index = 0;

    for(i = 0; i < size; i++) {
        y = *spectrum;
        for(j = 0; j < order; j++) y += MUL_C(state[state_index + j], lpc[j + 1]);
        /* double ringbuffer state */
        state_index--;
        if(state_index < 0) state_index = order - 1;
        state[state_index] = state[state_index + order] = *spectrum;
        *spectrum = y;
        spectrum += inc;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
int NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string) {
    static const char* libfaadName = "2.20.1";
    static const char* libCopyright = " Copyright 2002-2004: Ahead Software AG\n"
                                      " http://www.audiocoding.com\n"
                                      " bug tracking: https://sourceforge.net/p/faac/bugs/\n";

    if(faad_id_string) *faad_id_string = libfaadName;
    if(faad_copyright_string) *faad_copyright_string = libCopyright;
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
const char* err_msg[] = {"No error",
                         "Gain control not yet implemented",
                         "Pulse coding not allowed in short blocks",
                         "Invalid huffman codebook",
                         "Scalefactor out of range",
                         "Unable to find ADTS syncword",
                         "Channel coupling not yet implemented",
                         "Channel configuration not allowed in error resilient frame",
                         "Bit error in error resilient scalefactor decoding",
                         "Error decoding huffman scalefactor (bitstream error)",
                         "Error decoding huffman codeword (bitstream error)",
                         "Non existent huffman codebook number found",
                         "Invalid number of channels",
                         "Maximum number of bitstream elements exceeded",
                         "Input data buffer too small",
                         "Array index out of range",
                         "Maximum number of scalefactor bands exceeded",
                         "Quantised value out of range",
                         "LTP lag out of range",
                         "Invalid SBR parameter decoded",
                         "SBR called without being initialised",
                         "Unexpected channel configuration change",
                         "Error in program_config_element",
                         "First SBR frame is not the same as first AAC frame",
                         "Unexpected fill element with SBR data",
                         "Not all elements were provided with SBR data",
                         "LTP decoding not available",
                         "Output data buffer too small",
                         "CRC error in DRM data",
                         "PNS not allowed in DRM data stream",
                         "No standard extension payload allowed in DRM",
                         "PCE shall be the first element in a frame",
                         "Bitstream value not allowed by specification",
                         "MAIN prediction not initialised"};

const char* NeAACDecGetErrorMessage(unsigned const char errcode) {
    if(errcode >= NUM_ERROR_MESSAGES) return NULL;
    return err_msg[errcode];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
unsigned long NeAACDecGetCapabilities(void) {
    uint32_t cap = 0;
    cap += LC_DEC_CAP; /* can't do without it */
#ifdef LTP_DEC
    cap += LTP_DEC_CAP;
#endif
#ifdef LD_DEC
    cap += LD_DEC_CAP;
#endif
#ifdef ERROR_RESILIENCE
    cap += ERROR_RESILIENCE_CAP;
#endif

    cap += FIXED_POINT_CAP;

    return cap;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
const unsigned char mes[] = {0x67, 0x20, 0x61, 0x20, 0x20, 0x20, 0x6f, 0x20, 0x72, 0x20, 0x65, 0x20, 0x6e, 0x20, 0x20, 0x20, 0x74, 0x20, 0x68, 0x20, 0x67, 0x20, 0x69, 0x20, 0x72, 0x20, 0x79, 0x20, 0x70, 0x20, 0x6f, 0x20, 0x63};
NeAACDecHandle      NeAACDecOpen(void) {
    uint8_t         i;
    NeAACDecStruct* hDecoder = NULL;

    if((hDecoder = (NeAACDecStruct*)faad_malloc(sizeof(NeAACDecStruct))) == NULL) return NULL;
    memset(hDecoder, 0, sizeof(NeAACDecStruct));
    hDecoder->cmes = mes;
    hDecoder->config.outputFormat = FAAD_FMT_16BIT;
    hDecoder->config.defObjectType = MAIN;
    hDecoder->config.defSampleRate = 44100; /* Default: 44.1kHz */
    hDecoder->config.downMatrix = 0;
    hDecoder->adts_header_present = 0;
    hDecoder->adif_header_present = 0;
    hDecoder->latm_header_present = 0;
#ifdef ERROR_RESILIENCE
    hDecoder->aacSectionDataResilienceFlag = 0;
    hDecoder->aacScalefactorDataResilienceFlag = 0;
    hDecoder->aacSpectralDataResilienceFlag = 0;
#endif
    hDecoder->frameLength = 1024;
    hDecoder->frame = 0;
    hDecoder->sample_buffer = NULL;
    hDecoder->__r1 = 1;
    hDecoder->__r2 = 1;
    for(i = 0; i < MAX_CHANNELS; i++) {
        hDecoder->element_id[i] = INVALID_ELEMENT_ID;
        hDecoder->window_shape_prev[i] = 0;
        hDecoder->time_out[i] = NULL;
        hDecoder->fb_intermed[i] = NULL;
#ifdef LTP_DEC
        hDecoder->ltp_lag[i] = 0;
        hDecoder->lt_pred_stat[i] = NULL;
#endif
    }
#ifdef SBR_DEC
    for(i = 0; i < MAX_SYNTAX_ELEMENTS; i++) { hDecoder->sbr[i] = NULL; }
#endif
    hDecoder->drc = drc_init(REAL_CONST(1.0), REAL_CONST(1.0));
    return hDecoder;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
NeAACDecConfigurationPtr_t NeAACDecGetCurrentConfiguration(NeAACDecHandle hpDecoder) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if(hDecoder) {
        NeAACDecConfigurationPtr_t config = &(hDecoder->config);
        return config;
    }
    return NULL;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char NeAACDecSetConfiguration(NeAACDecHandle hpDecoder, NeAACDecConfigurationPtr_t config) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if(hDecoder && config) {
        /* check if we can decode this object type */
        if(can_decode_ot(config->defObjectType) < 0) return 0;
        hDecoder->config.defObjectType = config->defObjectType;
        /* samplerate: anything but 0 should be possible */
        if(config->defSampleRate == 0) return 0;
        hDecoder->config.defSampleRate = config->defSampleRate;
        /* check output format */
        if((config->outputFormat < 1) || (config->outputFormat > 4)) return 0;

        hDecoder->config.outputFormat = config->outputFormat;
        if(config->downMatrix > 1) return 0;
        hDecoder->config.downMatrix = config->downMatrix;
        /* OK */
        return 1;
    }

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static int latmCheck(latm_header* latm, bitfile* ld) {
    uint32_t good = 0, bad = 0, bits, m;

    while(ld->bytes_left) {
        bits = faad_latm_frame(latm, ld);
        if(bits == 0xFFFFFFFF) bad++;
        else {
            good++;
            while(bits > 0) {
                m = min(bits, 8);
                faad_getbits(ld, m);
                bits -= m;
            }
        }
    }
    return (good > 0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
long NeAACDecInit(NeAACDecHandle hpDecoder, unsigned char* buffer, unsigned long buffer_size, unsigned long* samplerate, unsigned char* channels) {
    uint32_t        bits = 0;
    bitfile         ld;
    adif_header     adif;
    adts_header     adts;
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;

    if((hDecoder == NULL) || (samplerate == NULL) || (channels == NULL) || (buffer_size == 0)) return -1;

    hDecoder->sf_index = get_sr_index(hDecoder->config.defSampleRate);
    hDecoder->object_type = hDecoder->config.defObjectType;
    *samplerate = get_sample_rate(hDecoder->sf_index);
    *channels = 1;

    if(buffer != NULL) {
        faad_initbits(&ld, buffer, buffer_size);
        /* Check if an ADIF header is present */
        if((buffer[0] == 'A') && (buffer[1] == 'D') && (buffer[2] == 'I') && (buffer[3] == 'F')) {
            hDecoder->adif_header_present = 1;
            get_adif_header(&adif, &ld);
            faad_byte_align(&ld);
            hDecoder->sf_index = adif.pce[0].sf_index;
            hDecoder->object_type = adif.pce[0].object_type + 1;
            *samplerate = get_sample_rate(hDecoder->sf_index);
            *channels = adif.pce[0].channels;
            memcpy(&(hDecoder->pce), &(adif.pce[0]), sizeof(program_config));
            hDecoder->pce_set = 1;
            bits = bit2byte(faad_get_processed_bits(&ld));
            /* Check if an ADTS header is present */
        }
        else if(faad_showbits(&ld, 12) == 0xfff) {
            hDecoder->adts_header_present = 1;
            adts.old_format = hDecoder->config.useOldADTSFormat;
            adts_frame(&adts, &ld);
            hDecoder->sf_index = adts.sf_index;
            hDecoder->object_type = adts.profile + 1;
            *samplerate = get_sample_rate(hDecoder->sf_index);
            *channels = (adts.channel_configuration > 6) ? 2 : adts.channel_configuration;
        }

        if(ld.error) { return -1; }
    }
    if(!*samplerate) return -1;
#if(defined(PS_DEC))
    /* check if we have a mono file */
    if(*channels == 1) {
        /* upMatrix to 2 channels for implicit signalling of PS */
        *channels = 2;
    }
#endif
    hDecoder->channelConfiguration = *channels;
#ifdef SBR_DEC
    /* implicit signalling */
    if(*samplerate <= 24000 && (hDecoder->config.dontUpSampleImplicitSBR == 0)) {
        *samplerate *= 2;
        hDecoder->forceUpSampling = 1;
    }
    else if(*samplerate > 24000 && (hDecoder->config.dontUpSampleImplicitSBR == 0)) { hDecoder->downSampledSBR = 1; }
#endif
    /* must be done before frameLength is divided by 2 for LD */
    hDecoder->fb = filter_bank_init(hDecoder->frameLength);
#ifdef LD_DEC
    if(hDecoder->object_type == LD) hDecoder->frameLength >>= 1;
#endif
    if(can_decode_ot(hDecoder->object_type) < 0) return -1;
    return bits;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* Init the library using a DecoderSpecificInfo */
char NeAACDecInit2(NeAACDecHandle hpDecoder, unsigned char* pBuffer, unsigned long SizeOfDecoderSpecificInfo, unsigned long* samplerate, unsigned char* channels) {
    NeAACDecStruct*        hDecoder = (NeAACDecStruct*)hpDecoder;
    int8_t                 rc;
    mp4AudioSpecificConfig mp4ASC;

    if((hDecoder == NULL) || (pBuffer == NULL) || (SizeOfDecoderSpecificInfo < 2) || (samplerate == NULL) || (channels == NULL)) { return -1; }
    hDecoder->adif_header_present = 0;
    hDecoder->adts_header_present = 0;
    /* decode the audio specific config */
    rc = AudioSpecificConfig2(pBuffer, SizeOfDecoderSpecificInfo, &mp4ASC, &(hDecoder->pce), hDecoder->latm_header_present);
    /* copy the relevant info to the decoder handle */
    *samplerate = mp4ASC.samplingFrequency;
    if(mp4ASC.channelsConfiguration) { *channels = mp4ASC.channelsConfiguration; }
    else {
        *channels = hDecoder->pce.channels;
        hDecoder->pce_set = 1;
    }
#if(defined(PS_DEC))
    /* check if we have a mono file */
    if(*channels == 1) {
        /* upMatrix to 2 channels for implicit signalling of PS */
        *channels = 2;
    }
#endif
    hDecoder->sf_index = mp4ASC.samplingFrequencyIndex;
    hDecoder->object_type = mp4ASC.objectTypeIndex;
#ifdef ERROR_RESILIENCE
    hDecoder->aacSectionDataResilienceFlag = mp4ASC.aacSectionDataResilienceFlag;
    hDecoder->aacScalefactorDataResilienceFlag = mp4ASC.aacScalefactorDataResilienceFlag;
    hDecoder->aacSpectralDataResilienceFlag = mp4ASC.aacSpectralDataResilienceFlag;
#endif
#ifdef SBR_DEC
    hDecoder->sbr_present_flag = mp4ASC.sbr_present_flag;
    hDecoder->downSampledSBR = mp4ASC.downSampledSBR;
    if(hDecoder->config.dontUpSampleImplicitSBR == 0) hDecoder->forceUpSampling = mp4ASC.forceUpSampling;
    else
        hDecoder->forceUpSampling = 0;

    /* AAC core decoder samplerate is 2 times as low */
    if(((hDecoder->sbr_present_flag == 1) && (!hDecoder->downSampledSBR)) || hDecoder->forceUpSampling == 1) { hDecoder->sf_index = get_sr_index(mp4ASC.samplingFrequency / 2); }
#endif
    if(rc != 0) { return rc; }
    hDecoder->channelConfiguration = mp4ASC.channelsConfiguration;
    if(mp4ASC.frameLengthFlag)
#ifdef ALLOW_SMALL_FRAMELENGTH
        hDecoder->frameLength = 960;
#else
        return -1;
#endif
    /* must be done before frameLength is divided by 2 for LD */
    hDecoder->fb = filter_bank_init(hDecoder->frameLength);
#ifdef LD_DEC
    if(hDecoder->object_type == LD) hDecoder->frameLength >>= 1;
#endif
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void NeAACDecClose(NeAACDecHandle hpDecoder) {
    uint8_t         i;
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;

    if(hDecoder == NULL) return;
#ifdef PROFILE
    printf("AAC decoder total:  %I64d cycles\n", hDecoder->cycles);
    printf("requant:            %I64d cycles\n", hDecoder->requant_cycles);
    printf("spectral_data:      %I64d cycles\n", hDecoder->spectral_cycles);
    printf("scalefactors:       %I64d cycles\n", hDecoder->scalefac_cycles);
    printf("output:             %I64d cycles\n", hDecoder->output_cycles);
#endif
    for(i = 0; i < MAX_CHANNELS; i++) {
        if(hDecoder->time_out[i]) faad_free(hDecoder->time_out[i]);
        if(hDecoder->fb_intermed[i]) faad_free(hDecoder->fb_intermed[i]);
#ifdef LTP_DEC
        if(hDecoder->lt_pred_stat[i]) faad_free(hDecoder->lt_pred_stat[i]);
#endif
    }
    filter_bank_end(hDecoder->fb);
    drc_end(hDecoder->drc);
    if(hDecoder->sample_buffer) faad_free(hDecoder->sample_buffer);
#ifdef SBR_DEC
    for(i = 0; i < MAX_SYNTAX_ELEMENTS; i++) {
        if(hDecoder->sbr[i]) sbrDecodeEnd(hDecoder->sbr[i]);
    }
#endif
    if(hDecoder) faad_free(hDecoder);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void NeAACDecPostSeekReset(NeAACDecHandle hpDecoder, long frame) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if(hDecoder) {
        hDecoder->postSeekResetFlag = 1;

        if(frame != -1) hDecoder->frame = frame;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void create_channel_config(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo) {
    hInfo->num_front_channels = 0;
    hInfo->num_side_channels = 0;
    hInfo->num_back_channels = 0;
    hInfo->num_lfe_channels = 0;
    memset(hInfo->channel_position, 0, MAX_CHANNELS * sizeof(uint8_t));

    if(hDecoder->downMatrix) {
        hInfo->num_front_channels = 2;
        hInfo->channel_position[0] = FRONT_CHANNEL_LEFT;
        hInfo->channel_position[1] = FRONT_CHANNEL_RIGHT;
        return;
    }
    /* check if there is a PCE */
    if(hDecoder->pce_set) {
        uint8_t i, chpos = 0;
        uint8_t chdir, back_center = 0, total = 0;
        hInfo->num_front_channels = hDecoder->pce.num_front_channels;
        total += hInfo->num_front_channels;
        hInfo->num_side_channels = hDecoder->pce.num_side_channels;
        total += hInfo->num_side_channels;
        hInfo->num_back_channels = hDecoder->pce.num_back_channels;
        total += hInfo->num_back_channels;
        hInfo->num_lfe_channels = hDecoder->pce.num_lfe_channels;
        total += hInfo->num_lfe_channels;
        chdir = hInfo->num_front_channels;
        if(chdir & 1) {
#if(defined(PS_DEC))
            if(total == 1) {
                /* When PS is enabled output is always stereo */
                hInfo->channel_position[chpos++] = FRONT_CHANNEL_LEFT;
                hInfo->channel_position[chpos++] = FRONT_CHANNEL_RIGHT;
            }
            else
#endif
                hInfo->channel_position[chpos++] = FRONT_CHANNEL_CENTER;
            chdir--;
        }
        for(i = 0; i < chdir; i += 2) {
            hInfo->channel_position[chpos++] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[chpos++] = FRONT_CHANNEL_RIGHT;
        }

        for(i = 0; i < hInfo->num_side_channels; i += 2) {
            hInfo->channel_position[chpos++] = SIDE_CHANNEL_LEFT;
            hInfo->channel_position[chpos++] = SIDE_CHANNEL_RIGHT;
        }

        chdir = hInfo->num_back_channels;
        if(chdir & 1) {
            back_center = 1;
            chdir--;
        }
        for(i = 0; i < chdir; i += 2) {
            hInfo->channel_position[chpos++] = BACK_CHANNEL_LEFT;
            hInfo->channel_position[chpos++] = BACK_CHANNEL_RIGHT;
        }
        if(back_center) { hInfo->channel_position[chpos++] = BACK_CHANNEL_CENTER; }

        for(i = 0; i < hInfo->num_lfe_channels; i++) { hInfo->channel_position[chpos++] = LFE_CHANNEL; }
    }
    else {
        switch(hDecoder->channelConfiguration) {
        case 1:
#if(defined(PS_DEC))
            /* When PS is enabled output is always stereo */
            hInfo->num_front_channels = 2;
            hInfo->channel_position[0] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[1] = FRONT_CHANNEL_RIGHT;
#else
            hInfo->num_front_channels = 1;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
#endif
            break;
        case 2:
            hInfo->num_front_channels = 2;
            hInfo->channel_position[0] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[1] = FRONT_CHANNEL_RIGHT;
            break;
        case 3:
            hInfo->num_front_channels = 3;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
            hInfo->channel_position[1] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[2] = FRONT_CHANNEL_RIGHT;
            break;
        case 4:
            hInfo->num_front_channels = 3;
            hInfo->num_back_channels = 1;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
            hInfo->channel_position[1] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[2] = FRONT_CHANNEL_RIGHT;
            hInfo->channel_position[3] = BACK_CHANNEL_CENTER;
            break;
        case 5:
            hInfo->num_front_channels = 3;
            hInfo->num_back_channels = 2;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
            hInfo->channel_position[1] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[2] = FRONT_CHANNEL_RIGHT;
            hInfo->channel_position[3] = BACK_CHANNEL_LEFT;
            hInfo->channel_position[4] = BACK_CHANNEL_RIGHT;
            break;
        case 6:
            hInfo->num_front_channels = 3;
            hInfo->num_back_channels = 2;
            hInfo->num_lfe_channels = 1;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
            hInfo->channel_position[1] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[2] = FRONT_CHANNEL_RIGHT;
            hInfo->channel_position[3] = BACK_CHANNEL_LEFT;
            hInfo->channel_position[4] = BACK_CHANNEL_RIGHT;
            hInfo->channel_position[5] = LFE_CHANNEL;
            break;
        case 7:
            hInfo->num_front_channels = 3;
            hInfo->num_side_channels = 2;
            hInfo->num_back_channels = 2;
            hInfo->num_lfe_channels = 1;
            hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
            hInfo->channel_position[1] = FRONT_CHANNEL_LEFT;
            hInfo->channel_position[2] = FRONT_CHANNEL_RIGHT;
            hInfo->channel_position[3] = SIDE_CHANNEL_LEFT;
            hInfo->channel_position[4] = SIDE_CHANNEL_RIGHT;
            hInfo->channel_position[5] = BACK_CHANNEL_LEFT;
            hInfo->channel_position[6] = BACK_CHANNEL_RIGHT;
            hInfo->channel_position[7] = LFE_CHANNEL;
            break;
        default: /* channelConfiguration == 0 || channelConfiguration > 7 */
        {
            uint8_t i;
            uint8_t ch = hDecoder->fr_channels - hDecoder->has_lfe;
            if(ch & 1) /* there's either a center front or a center back channel */
            {
                uint8_t ch1 = (ch - 1) / 2;
                if(hDecoder->first_syn_ele == ID_SCE) {
                    hInfo->num_front_channels = ch1 + 1;
                    hInfo->num_back_channels = ch1;
                    hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
                    for(i = 1; i <= ch1; i += 2) {
                        hInfo->channel_position[i] = FRONT_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = FRONT_CHANNEL_RIGHT;
                    }
                    for(i = ch1 + 1; i < ch; i += 2) {
                        hInfo->channel_position[i] = BACK_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = BACK_CHANNEL_RIGHT;
                    }
                }
                else {
                    hInfo->num_front_channels = ch1;
                    hInfo->num_back_channels = ch1 + 1;
                    for(i = 0; i < ch1; i += 2) {
                        hInfo->channel_position[i] = FRONT_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = FRONT_CHANNEL_RIGHT;
                    }
                    for(i = ch1; i < ch - 1; i += 2) {
                        hInfo->channel_position[i] = BACK_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = BACK_CHANNEL_RIGHT;
                    }
                    hInfo->channel_position[ch - 1] = BACK_CHANNEL_CENTER;
                }
            }
            else {
                uint8_t ch1 = (ch) / 2;
                hInfo->num_front_channels = ch1;
                hInfo->num_back_channels = ch1;
                if(ch1 & 1) {
                    hInfo->channel_position[0] = FRONT_CHANNEL_CENTER;
                    for(i = 1; i <= ch1; i += 2) {
                        hInfo->channel_position[i] = FRONT_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = FRONT_CHANNEL_RIGHT;
                    }
                    for(i = ch1 + 1; i < ch - 1; i += 2) {
                        hInfo->channel_position[i] = BACK_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = BACK_CHANNEL_RIGHT;
                    }
                    hInfo->channel_position[ch - 1] = BACK_CHANNEL_CENTER;
                }
                else {
                    for(i = 0; i < ch1; i += 2) {
                        hInfo->channel_position[i] = FRONT_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = FRONT_CHANNEL_RIGHT;
                    }
                    for(i = ch1; i < ch; i += 2) {
                        hInfo->channel_position[i] = BACK_CHANNEL_LEFT;
                        hInfo->channel_position[i + 1] = BACK_CHANNEL_RIGHT;
                    }
                }
            }
            hInfo->num_lfe_channels = hDecoder->has_lfe;
            for(i = ch; i < hDecoder->fr_channels; i++) { hInfo->channel_position[i] = LFE_CHANNEL; }
        } break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void* NeAACDecDecode(NeAACDecHandle hpDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    return aac_frame_decode(hDecoder, hInfo, buffer, buffer_size, NULL, 0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void* NeAACDecDecode2(NeAACDecHandle hpDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer, unsigned long sample_buffer_size) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if((sample_buffer == NULL) || (sample_buffer_size == 0)) {
        hInfo->error = 27;
        return NULL;
    }
    return aac_frame_decode(hDecoder, hInfo, buffer, buffer_size, sample_buffer, sample_buffer_size);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void* aac_frame_decode(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer2, unsigned long sample_buffer_size) {
    uint16_t i;
    uint8_t  channels = 0;
    uint8_t  output_channels = 0;
    bitfile  ld = {0};
    uint32_t bitsconsumed;
    uint16_t frame_len;
    void*    sample_buffer;
    uint32_t startbit = 0, endbit = 0, payload_bits = 0;

#ifdef PROFILE
    int64_t count = faad_get_ts();
#endif
    /* safety checks */
    if((hDecoder == NULL) || (hInfo == NULL) || (buffer == NULL)) { return NULL; }

    frame_len = hDecoder->frameLength;
    memset(hInfo, 0, sizeof(NeAACDecFrameInfo));
    memset(hDecoder->internal_channel, 0, MAX_CHANNELS * sizeof(hDecoder->internal_channel[0]));
#ifdef USE_TIME_LIMIT
    if((TIME_LIMIT * get_sample_rate(hDecoder->sf_index)) > hDecoder->TL_count) { hDecoder->TL_count += 1024; }
    else {
        hInfo->error = (NUM_ERROR_MESSAGES - 1);
        goto error;
    }
#endif
    /* check for some common metadata tag types in the bitstream
     * No need to return an error
     */
    /* ID3 */
    if(buffer_size >= 128) {
        if(memcmp(buffer, "TAG", 3) == 0) {
            /* found it */
            hInfo->bytesconsumed = 128; /* 128 bytes fixed size */
            /* no error, but no output either */
            return NULL;
        }
    }
    /* initialize the bitstream */
    faad_initbits(&ld, buffer, buffer_size);
    if(hDecoder->adts_header_present) {
        adts_header adts;
        adts.old_format = hDecoder->config.useOldADTSFormat;
        if((hInfo->error = adts_frame(&adts, &ld)) > 0) goto error;
        /* MPEG2 does byte_alignment() here,
         * but ADTS header is always multiple of 8 bits in MPEG2
         * so not needed to actually do it.
         */
    }
    /* decode the complete bitstream */
    raw_data_block(hDecoder, hInfo, &ld, &hDecoder->pce, hDecoder->drc);

    channels = hDecoder->fr_channels;
    if(hInfo->error > 0) goto error;
    /* safety check */
    if(channels == 0 || channels > MAX_CHANNELS) {
        /* invalid number of channels */
        hInfo->error = 12;
        goto error;
    }
    /* no more bit reading after this */
    bitsconsumed = faad_get_processed_bits(&ld);
    hInfo->bytesconsumed = bit2byte(bitsconsumed);
    if(ld.error) {
        hInfo->error = 14;
        goto error;
    }
    if(!hDecoder->adts_header_present && !hDecoder->adif_header_present) {
        if(hDecoder->channelConfiguration == 0) hDecoder->channelConfiguration = channels;

        if(channels == 8) /* 7.1 */
            hDecoder->channelConfiguration = 7;
        if(channels == 7) /* not a standard channelConfiguration */
            hDecoder->channelConfiguration = 0;
    }

    if((channels == 5 || channels == 6) && hDecoder->config.downMatrix) {
        hDecoder->downMatrix = 1;
        output_channels = 2;
    }
    else { output_channels = channels; }

#if(defined(PS_DEC))
    hDecoder->upMatrix = 0;
    /* check if we have a mono file */
    if(output_channels == 1) {
        /* upMatrix to 2 channels for implicit signalling of PS */
        hDecoder->upMatrix = 1;
        output_channels = 2;
    }
#endif
    /* Make a channel configuration based on either a PCE or a channelConfiguration */
    create_channel_config(hDecoder, hInfo);
    /* number of samples in this frame */
    hInfo->samples = frame_len * output_channels;
    /* number of channels in this frame */
    hInfo->channels = output_channels;
    /* samplerate */
    hInfo->samplerate = get_sample_rate(hDecoder->sf_index);
    /* object type */
    hInfo->object_type = hDecoder->object_type;
    /* sbr */
    hInfo->sbr = NO_SBR;
    /* header type */
    hInfo->header_type = RAW;
    if(hDecoder->adif_header_present) hInfo->header_type = ADIF;
    if(hDecoder->adts_header_present) hInfo->header_type = ADTS;
#if(defined(PS_DEC))
    hInfo->ps = hDecoder->ps_used_global;
#endif
    /* check if frame has channel elements */
    if(channels == 0) {
        hDecoder->frame++;
        return NULL;
    }
    /* allocate the buffer for the final samples */
    if((hDecoder->sample_buffer == NULL) || (hDecoder->alloced_channels != output_channels)) {
        static const uint8_t str[] = {sizeof(int16_t), sizeof(int32_t), sizeof(int32_t), sizeof(float), sizeof(double), sizeof(int16_t), sizeof(int16_t), sizeof(int16_t), sizeof(int16_t), 0, 0, 0};
        uint8_t              stride = str[hDecoder->config.outputFormat - 1];
#ifdef SBR_DEC
        if(((hDecoder->sbr_present_flag == 1) && (!hDecoder->downSampledSBR)) || (hDecoder->forceUpSampling == 1)) { stride = 2 * stride; }
#endif
        /* check if we want to use internal sample_buffer */
        if(sample_buffer_size == 0) {
            if(hDecoder->sample_buffer) faad_free(hDecoder->sample_buffer);
            hDecoder->sample_buffer = NULL;
            hDecoder->sample_buffer = faad_malloc(frame_len * output_channels * stride);
        }
        else if(sample_buffer_size < frame_len * output_channels * stride) {
            /* provided sample buffer is not big enough */
            hInfo->error = 27;
            return NULL;
        }
        hDecoder->alloced_channels = output_channels;
    }

    if(sample_buffer_size == 0) { sample_buffer = hDecoder->sample_buffer; }
    else { sample_buffer = *sample_buffer2; }

#ifdef SBR_DEC
    if((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) {
        uint8_t ele;

        /* this data is different when SBR is used or when the data is upsampled */
        if(!hDecoder->downSampledSBR) {
            frame_len *= 2;
            hInfo->samples *= 2;
            hInfo->samplerate *= 2;
        }
        /* check if every element was provided with SBR data */
        for(ele = 0; ele < hDecoder->fr_ch_ele; ele++) {
            if(hDecoder->sbr[ele] == NULL) {
                hInfo->error = 25;
                goto error;
            }
        }
        /* sbr */
        if(hDecoder->sbr_present_flag == 1) {
            hInfo->object_type = HE_AAC;
            hInfo->sbr = SBR_UPSAMPLED;
        }
        else { hInfo->sbr = NO_SBR_UPSAMPLED; }
        if(hDecoder->downSampledSBR) { hInfo->sbr = SBR_DOWNSAMPLED; }
    }
#endif
    sample_buffer = output_to_PCM(hDecoder, hDecoder->time_out, sample_buffer, output_channels, frame_len, hDecoder->config.outputFormat);
    hDecoder->postSeekResetFlag = 0;
    hDecoder->frame++;
#ifdef LD_DEC
    if(hDecoder->object_type != LD) {
#endif
        if(hDecoder->frame <= 1) hInfo->samples = 0;
#ifdef LD_DEC
    }
    else {
        /* LD encoders will give lower delay */
        if(hDecoder->frame <= 0) hInfo->samples = 0;
    }
#endif

#ifdef PROFILE
    count = faad_get_ts() - count;
    hDecoder->cycles += count;
#endif
    return sample_buffer;

error:
    /* reset filterbank state */
    for(i = 0; i < MAX_CHANNELS; i++) {
        if(hDecoder->fb_intermed[i] != NULL) { memset(hDecoder->fb_intermed[i], 0, hDecoder->frameLength * sizeof(int32_t)); }
    }
#ifdef SBR_DEC
    for(i = 0; i < MAX_SYNTAX_ELEMENTS; i++) {
        if(hDecoder->sbr[i] != NULL) { sbrReset(hDecoder->sbr[i]); }
    }
#endif
    return NULL;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
drc_info* drc_init(int32_t cut, int32_t boost) {
    drc_info* drc = (drc_info*)faad_malloc(sizeof(drc_info));
    memset(drc, 0, sizeof(drc_info));

    drc->ctrl1 = cut;
    drc->ctrl2 = boost;

    drc->num_bands = 1;
    drc->band_top[0] = 1024 / 4 - 1;
    drc->dyn_rng_sgn[0] = 1;
    drc->dyn_rng_ctl[0] = 0;

    return drc;
}

//----------------------------------------------------------------------------------------------------------------------
void drc_end(drc_info* drc) {
    if(drc) faad_free(drc);
}

//----------------------------------------------------------------------------------------------------------------------
static int32_t drc_pow2_table[] = {COEF_CONST(0.5146511183), COEF_CONST(0.5297315472), COEF_CONST(0.5452538663), COEF_CONST(0.5612310242), COEF_CONST(0.5776763484), COEF_CONST(0.5946035575), COEF_CONST(0.6120267717), COEF_CONST(0.6299605249),
                                   COEF_CONST(0.6484197773), COEF_CONST(0.6674199271), COEF_CONST(0.6869768237), COEF_CONST(0.7071067812), COEF_CONST(0.7278265914), COEF_CONST(0.7491535384), COEF_CONST(0.7711054127), COEF_CONST(0.7937005260),
                                   COEF_CONST(0.8169577266), COEF_CONST(0.8408964153), COEF_CONST(0.8655365610), COEF_CONST(0.8908987181), COEF_CONST(0.9170040432), COEF_CONST(0.9438743127), COEF_CONST(0.9715319412), COEF_CONST(1.0000000000),
                                   COEF_CONST(1.0293022366), COEF_CONST(1.0594630944), COEF_CONST(1.0905077327), COEF_CONST(1.1224620483), COEF_CONST(1.1553526969), COEF_CONST(1.1892071150), COEF_CONST(1.2240535433), COEF_CONST(1.2599210499),
                                   COEF_CONST(1.2968395547), COEF_CONST(1.3348398542), COEF_CONST(1.3739536475), COEF_CONST(1.4142135624), COEF_CONST(1.4556531828), COEF_CONST(1.4983070769), COEF_CONST(1.5422108254), COEF_CONST(1.5874010520),
                                   COEF_CONST(1.6339154532), COEF_CONST(1.6817928305), COEF_CONST(1.7310731220), COEF_CONST(1.7817974363), COEF_CONST(1.8340080864), COEF_CONST(1.8877486254), COEF_CONST(1.9430638823)};
//----------------------------------------------------------------------------------------------------------------------
void drc_decode(drc_info* drc, int32_t* spec) {
    uint16_t i, bd, top;
    int32_t exp, frac;

    uint16_t bottom = 0;

    if(drc->num_bands == 1) drc->band_top[0] = 1024 / 4 - 1;

    for(bd = 0; bd < drc->num_bands; bd++) {
        top = 4 * (drc->band_top[bd] + 1);

        /* Decode DRC gain factor */
        if(drc->dyn_rng_sgn[bd]) /* compress */
        {
            exp = -1 * (drc->dyn_rng_ctl[bd] - (DRC_REF_LEVEL - drc->prog_ref_level)) / 24;
            frac = -1 * (drc->dyn_rng_ctl[bd] - (DRC_REF_LEVEL - drc->prog_ref_level)) % 24;
        }
        else { /* boost */
            exp = (drc->dyn_rng_ctl[bd] - (DRC_REF_LEVEL - drc->prog_ref_level)) / 24;
            frac = (drc->dyn_rng_ctl[bd] - (DRC_REF_LEVEL - drc->prog_ref_level)) % 24;
        }

        /* Apply gain factor */
        if(exp < 0) {
            for(i = bottom; i < top; i++) {
                spec[i] >>= -exp;
                if(frac) spec[i] = MUL_R(spec[i], drc_pow2_table[frac + 23]);
            }
        }
        else {
            for(i = bottom; i < top; i++) {
                spec[i] <<= exp;
                if(frac) spec[i] = MUL_R(spec[i], drc_pow2_table[frac + 23]);
            }
        }


        bottom = top;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
fb_info* filter_bank_init(uint16_t frame_len) {
    uint16_t nshort = frame_len / 8;
#ifdef LD_DEC
    uint16_t frame_len_ld = frame_len / 2;
#endif

    fb_info* fb = (fb_info*)faad_malloc(sizeof(fb_info));
    memset(fb, 0, sizeof(fb_info));

    /* normal */
    fb->mdct256 = faad_mdct_init(2 * nshort);
    fb->mdct2048 = faad_mdct_init(2 * frame_len);
#ifdef LD_DEC
    /* LD */
    fb->mdct1024 = faad_mdct_init(2 * frame_len_ld);
#endif

#ifdef ALLOW_SMALL_FRAMELENGTH
    if(frame_len == 1024) {
#endif
        fb->long_window[0] = sine_long_1024;
        fb->short_window[0] = sine_short_128;
        fb->long_window[1] = kbd_long_1024;
        fb->short_window[1] = kbd_short_128;
#ifdef LD_DEC
        fb->ld_window[0] = sine_mid_512;
        fb->ld_window[1] = ld_mid_512;
#endif
#ifdef ALLOW_SMALL_FRAMELENGTH
    }
    else /* (frame_len == 960) */ {
        fb->long_window[0] = sine_long_960;
        fb->short_window[0] = sine_short_120;
        fb->long_window[1] = kbd_long_960;
        fb->short_window[1] = kbd_short_120;
    #ifdef LD_DEC
        fb->ld_window[0] = sine_mid_480;
        fb->ld_window[1] = ld_mid_480;
    #endif
    }
#endif

    return fb;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void filter_bank_end(fb_info* fb) {
    if(fb != NULL) {
#ifdef PROFILE
        printf("FB:                 %I64d cycles\n", fb->cycles);
#endif

        faad_mdct_end(fb->mdct256);
        faad_mdct_end(fb->mdct2048);
#ifdef LD_DEC
        faad_mdct_end(fb->mdct1024);
#endif

        faad_free(fb);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline void imdct_long(fb_info* fb, int32_t* in_data, int32_t* out_data, uint16_t len) {
#ifdef LD_DEC
    mdct_info* mdct = NULL;

    switch(len) {
    case 2048:
    case 1920: mdct = fb->mdct2048; break;
    case 1024:
    case 960: mdct = fb->mdct1024; break;
    }

    faad_imdct(mdct, in_data, out_data);
#else
    faad_imdct(fb->mdct2048, in_data, out_data);
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef LTP_DEC
static inline void mdct(fb_info* fb, int32_t* in_data, int32_t* out_data, uint16_t len) {
    mdct_info* mdct = NULL;

    switch(len) {
    case 2048:
    case 1920: mdct = fb->mdct2048; break;
    case 256:
    case 240: mdct = fb->mdct256; break;
    #ifdef LD_DEC
    case 1024:
    case 960: mdct = fb->mdct1024; break;
    #endif
    }

    faad_mdct(mdct, in_data, out_data);
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------
void ifilter_bank(fb_info* fb, uint8_t window_sequence, uint8_t window_shape, uint8_t window_shape_prev, int32_t* freq_in, int32_t* time_out, int32_t* overlap, uint8_t object_type, uint16_t frame_len) {
    int16_t i;
    int32_t transf_buf[2 * 1024] = {0};

    const int32_t* window_long = NULL;
    const int32_t* window_long_prev = NULL;
    const int32_t* window_short = NULL;
    const int32_t* window_short_prev = NULL;

    uint16_t nlong = frame_len;
    uint16_t nshort = frame_len / 8;
    uint16_t trans = nshort / 2;

    uint16_t nflat_ls = (nlong - nshort) / 2;

#ifdef PROFILE
    int64_t count = faad_get_ts();
#endif

    /* select windows of current frame and previous frame (Sine or KBD) */
#ifdef LD_DEC
    if(object_type == LD) {
        window_long = fb->ld_window[window_shape];
        window_long_prev = fb->ld_window[window_shape_prev];
    }
    else {
#endif
        window_long = fb->long_window[window_shape];
        window_long_prev = fb->long_window[window_shape_prev];
        window_short = fb->short_window[window_shape];
        window_short_prev = fb->short_window[window_shape_prev];
#ifdef LD_DEC
    }
#endif

    switch(window_sequence) {
    case ONLY_LONG_SEQUENCE:
        /* perform iMDCT */
        imdct_long(fb, freq_in, transf_buf, 2 * nlong);

        /* add second half output of previous frame to windowed output of current frame */
        for(i = 0; i < nlong; i += 4) {
            time_out[i] = overlap[i] + MUL_F(transf_buf[i], window_long_prev[i]);
            time_out[i + 1] = overlap[i + 1] + MUL_F(transf_buf[i + 1], window_long_prev[i + 1]);
            time_out[i + 2] = overlap[i + 2] + MUL_F(transf_buf[i + 2], window_long_prev[i + 2]);
            time_out[i + 3] = overlap[i + 3] + MUL_F(transf_buf[i + 3], window_long_prev[i + 3]);
        }

        /* window the second half and save as overlap for next frame */
        for(i = 0; i < nlong; i += 4) {
            overlap[i] = MUL_F(transf_buf[nlong + i], window_long[nlong - 1 - i]);
            overlap[i + 1] = MUL_F(transf_buf[nlong + i + 1], window_long[nlong - 2 - i]);
            overlap[i + 2] = MUL_F(transf_buf[nlong + i + 2], window_long[nlong - 3 - i]);
            overlap[i + 3] = MUL_F(transf_buf[nlong + i + 3], window_long[nlong - 4 - i]);
        }
        break;

    case LONG_START_SEQUENCE:
        /* perform iMDCT */
        imdct_long(fb, freq_in, transf_buf, 2 * nlong);

        /* add second half output of previous frame to windowed output of current frame */
        for(i = 0; i < nlong; i += 4) {
            time_out[i] = overlap[i] + MUL_F(transf_buf[i], window_long_prev[i]);
            time_out[i + 1] = overlap[i + 1] + MUL_F(transf_buf[i + 1], window_long_prev[i + 1]);
            time_out[i + 2] = overlap[i + 2] + MUL_F(transf_buf[i + 2], window_long_prev[i + 2]);
            time_out[i + 3] = overlap[i + 3] + MUL_F(transf_buf[i + 3], window_long_prev[i + 3]);
        }

        /* window the second half and save as overlap for next frame */
        /* construct second half window using padding with 1's and 0's */
        for(i = 0; i < nflat_ls; i++) overlap[i] = transf_buf[nlong + i];
        for(i = 0; i < nshort; i++) overlap[nflat_ls + i] = MUL_F(transf_buf[nlong + nflat_ls + i], window_short[nshort - i - 1]);
        for(i = 0; i < nflat_ls; i++) overlap[nflat_ls + nshort + i] = 0;
        break;

    case EIGHT_SHORT_SEQUENCE:
        /* perform iMDCT for each short block */
        faad_imdct(fb->mdct256, freq_in + 0 * nshort, transf_buf + 2 * nshort * 0);
        faad_imdct(fb->mdct256, freq_in + 1 * nshort, transf_buf + 2 * nshort * 1);
        faad_imdct(fb->mdct256, freq_in + 2 * nshort, transf_buf + 2 * nshort * 2);
        faad_imdct(fb->mdct256, freq_in + 3 * nshort, transf_buf + 2 * nshort * 3);
        faad_imdct(fb->mdct256, freq_in + 4 * nshort, transf_buf + 2 * nshort * 4);
        faad_imdct(fb->mdct256, freq_in + 5 * nshort, transf_buf + 2 * nshort * 5);
        faad_imdct(fb->mdct256, freq_in + 6 * nshort, transf_buf + 2 * nshort * 6);
        faad_imdct(fb->mdct256, freq_in + 7 * nshort, transf_buf + 2 * nshort * 7);

        /* add second half output of previous frame to windowed output of current frame */
        for(i = 0; i < nflat_ls; i++) time_out[i] = overlap[i];
        for(i = 0; i < nshort; i++) {
            time_out[nflat_ls + i] = overlap[nflat_ls + i] + MUL_F(transf_buf[nshort * 0 + i], window_short_prev[i]);
            time_out[nflat_ls + 1 * nshort + i] = overlap[nflat_ls + nshort * 1 + i] + MUL_F(transf_buf[nshort * 1 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 2 + i], window_short[i]);
            time_out[nflat_ls + 2 * nshort + i] = overlap[nflat_ls + nshort * 2 + i] + MUL_F(transf_buf[nshort * 3 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 4 + i], window_short[i]);
            time_out[nflat_ls + 3 * nshort + i] = overlap[nflat_ls + nshort * 3 + i] + MUL_F(transf_buf[nshort * 5 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 6 + i], window_short[i]);
            if(i < trans) time_out[nflat_ls + 4 * nshort + i] = overlap[nflat_ls + nshort * 4 + i] + MUL_F(transf_buf[nshort * 7 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 8 + i], window_short[i]);
        }

        /* window the second half and save as overlap for next frame */
        for(i = 0; i < nshort; i++) {
            if(i >= trans) overlap[nflat_ls + 4 * nshort + i - nlong] = MUL_F(transf_buf[nshort * 7 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 8 + i], window_short[i]);
            overlap[nflat_ls + 5 * nshort + i - nlong] = MUL_F(transf_buf[nshort * 9 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 10 + i], window_short[i]);
            overlap[nflat_ls + 6 * nshort + i - nlong] = MUL_F(transf_buf[nshort * 11 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 12 + i], window_short[i]);
            overlap[nflat_ls + 7 * nshort + i - nlong] = MUL_F(transf_buf[nshort * 13 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 14 + i], window_short[i]);
            overlap[nflat_ls + 8 * nshort + i - nlong] = MUL_F(transf_buf[nshort * 15 + i], window_short[nshort - 1 - i]);
        }
        for(i = 0; i < nflat_ls; i++) overlap[nflat_ls + nshort + i] = 0;
        break;

    case LONG_STOP_SEQUENCE:
        /* perform iMDCT */
        imdct_long(fb, freq_in, transf_buf, 2 * nlong);

        /* add second half output of previous frame to windowed output of current frame */
        /* construct first half window using padding with 1's and 0's */
        for(i = 0; i < nflat_ls; i++) time_out[i] = overlap[i];
        for(i = 0; i < nshort; i++) time_out[nflat_ls + i] = overlap[nflat_ls + i] + MUL_F(transf_buf[nflat_ls + i], window_short_prev[i]);
        for(i = 0; i < nflat_ls; i++) time_out[nflat_ls + nshort + i] = overlap[nflat_ls + nshort + i] + transf_buf[nflat_ls + nshort + i];

        /* window the second half and save as overlap for next frame */
        for(i = 0; i < nlong; i++) overlap[i] = MUL_F(transf_buf[nlong + i], window_long[nlong - 1 - i]);
        break;
    }

#ifdef PROFILE
    count = faad_get_ts() - count;
    fb->cycles += count;
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef LTP_DEC
/* only works for LTP -> no overlapping, no short blocks */
void filter_bank_ltp(fb_info* fb, uint8_t window_sequence, uint8_t window_shape, uint8_t window_shape_prev, int32_t* in_data, int32_t* out_mdct, uint8_t object_type, uint16_t frame_len) {
    int16_t i;
    int32_t windowed_buf[2 * 1024] = {0};

    const int32_t* window_long = NULL;
    const int32_t* window_long_prev = NULL;
    const int32_t* window_short = NULL;
    const int32_t* window_short_prev = NULL;

    uint16_t nlong = frame_len;
    uint16_t nshort = frame_len / 8;
    uint16_t nflat_ls = (nlong - nshort) / 2;

    assert(window_sequence != EIGHT_SHORT_SEQUENCE);

    #ifdef LD_DEC
    if(object_type == LD) {
        window_long = fb->ld_window[window_shape];
        window_long_prev = fb->ld_window[window_shape_prev];
    }
    else {
    #endif
        window_long = fb->long_window[window_shape];
        window_long_prev = fb->long_window[window_shape_prev];
        window_short = fb->short_window[window_shape];
        window_short_prev = fb->short_window[window_shape_prev];
    #ifdef LD_DEC
    }
    #endif

    switch(window_sequence) {
    case ONLY_LONG_SEQUENCE:
        for(i = nlong - 1; i >= 0; i--) {
            windowed_buf[i] = MUL_F(in_data[i], window_long_prev[i]);
            windowed_buf[i + nlong] = MUL_F(in_data[i + nlong], window_long[nlong - 1 - i]);
        }
        mdct(fb, windowed_buf, out_mdct, 2 * nlong);
        break;

    case LONG_START_SEQUENCE:
        for(i = 0; i < nlong; i++) windowed_buf[i] = MUL_F(in_data[i], window_long_prev[i]);
        for(i = 0; i < nflat_ls; i++) windowed_buf[i + nlong] = in_data[i + nlong];
        for(i = 0; i < nshort; i++) windowed_buf[i + nlong + nflat_ls] = MUL_F(in_data[i + nlong + nflat_ls], window_short[nshort - 1 - i]);
        for(i = 0; i < nflat_ls; i++) windowed_buf[i + nlong + nflat_ls + nshort] = 0;
        mdct(fb, windowed_buf, out_mdct, 2 * nlong);
        break;

    case LONG_STOP_SEQUENCE:
        for(i = 0; i < nflat_ls; i++) windowed_buf[i] = 0;
        for(i = 0; i < nshort; i++) windowed_buf[i + nflat_ls] = MUL_F(in_data[i + nflat_ls], window_short_prev[i]);
        for(i = 0; i < nflat_ls; i++) windowed_buf[i + nflat_ls + nshort] = in_data[i + nflat_ls + nshort];
        for(i = 0; i < nlong; i++) windowed_buf[i + nlong] = MUL_F(in_data[i + nlong], window_long[nlong - 1 - i]);
        mdct(fb, windowed_buf, out_mdct, 2 * nlong);
        break;
    }
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t huffman_scale_factor(bitfile* ld) {
    uint16_t offset = 0;

    while(hcb_sf[offset][1]) {
        uint8_t b = faad_get1bit(ld);
        offset += hcb_sf[offset][b];
        if(offset > 240) {
            /* printf("ERROR: offset into hcb_sf = %d >240!\n", offset); */
            return -1;
        }
    }
    return hcb_sf[offset][0];
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

hcb*          hcb_table[] = {0, hcb1_1, hcb2_1, 0, hcb4_1, 0, hcb6_1, 0, hcb8_1, 0, hcb10_1, hcb11_1};
hcb_2_quad*   hcb_2_quad_table[] = {0, hcb1_2, hcb2_2, 0, hcb4_2, 0, 0, 0, 0, 0, 0, 0};
hcb_2_pair*   hcb_2_pair_table[] = {0, 0, 0, 0, 0, 0, hcb6_2, 0, hcb8_2, 0, hcb10_2, hcb11_2};
hcb_bin_pair* hcb_bin_table[] = {0, 0, 0, 0, 0, hcb5, 0, hcb7, 0, hcb9, 0, 0};
uint8_t       hcbN[] = {0, 5, 5, 0, 5, 0, 5, 0, 5, 0, 6, 5};

/* defines whether a huffman codebook is unsigned or not */
/* Table 4.6.2 */
uint8_t unsigned_cb[] = {0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int hcb_2_quad_table_size[] = {0, 114, 86, 0, 185, 0, 0, 0, 0, 0, 0, 0};
int hcb_2_pair_table_size[] = {0, 0, 0, 0, 0, 0, 126, 0, 83, 0, 210, 373};
int hcb_bin_table_size[] = {0, 0, 0, 161, 0, 161, 0, 127, 0, 337, 0, 0};

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline void huffman_sign_bits(bitfile* ld, int16_t* sp, uint8_t len) {
    uint8_t i;

    for(i = 0; i < len; i++) {
        if(sp[i]) {
            if(faad_get1bit(ld) & 1) { sp[i] = -sp[i]; }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline uint8_t huffman_getescape(bitfile* ld, int16_t* sp) {
    uint8_t neg, i;
    int16_t j;
    int16_t off;
    int16_t x = *sp;

    if(x < 0) {
        if(x != -16) return 0;
        neg = 1;
    }
    else {
        if(x != 16) return 0;
        neg = 0;
    }
    for(i = 4; i < 16; i++) {
        if(faad_get1bit(ld) == 0) { break; }
    }
    if(i >= 16) return 10;
    off = (int16_t)faad_getbits(ld, i);
    j = off | (1 << i);
    if(neg) j = -j;
    *sp = j;
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_quad(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint32_t cw;
    uint16_t offset = 0;
    uint8_t  extra_bits;

    cw = faad_showbits(ld, hcbN[cb]);
    offset = hcb_table[cb][cw].offset;
    extra_bits = hcb_table[cb][cw].extra_bits;
    if(extra_bits) {
        /* we know for sure it's more than hcbN[cb] bits long */
        faad_flushbits(ld, hcbN[cb]);
        offset += (uint16_t)faad_showbits(ld, extra_bits);
        faad_flushbits(ld, hcb_2_quad_table[cb][offset].bits - hcbN[cb]);
    }
    else { faad_flushbits(ld, hcb_2_quad_table[cb][offset].bits); }
    if(offset > hcb_2_quad_table_size[cb]) {
        /* printf("ERROR: offset into hcb_2_quad_table = %d >%d!\n", offset,
           hcb_2_quad_table_size[cb]); */
        return 10;
    }
    sp[0] = hcb_2_quad_table[cb][offset].x;
    sp[1] = hcb_2_quad_table[cb][offset].y;
    sp[2] = hcb_2_quad_table[cb][offset].v;
    sp[3] = hcb_2_quad_table[cb][offset].w;
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_2step_quad(cb, ld, sp);
    huffman_sign_bits(ld, sp, QUAD_LEN);
    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_pair(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint32_t cw;
    uint16_t offset = 0;
    uint8_t  extra_bits;

    cw = faad_showbits(ld, hcbN[cb]);
    offset = hcb_table[cb][cw].offset;
    extra_bits = hcb_table[cb][cw].extra_bits;

    if(extra_bits) {
        /* we know for sure it's more than hcbN[cb] bits long */
        faad_flushbits(ld, hcbN[cb]);
        offset += (uint16_t)faad_showbits(ld, extra_bits);
        faad_flushbits(ld, hcb_2_pair_table[cb][offset].bits - hcbN[cb]);
    }
    else { faad_flushbits(ld, hcb_2_pair_table[cb][offset].bits); }
    if(offset > hcb_2_pair_table_size[cb]) {
        /* printf("ERROR: offset into hcb_2_pair_table = %d >%d!\n", offset,
           hcb_2_pair_table_size[cb]); */
        return 10;
    }
    sp[0] = hcb_2_pair_table[cb][offset].x;
    sp[1] = hcb_2_pair_table[cb][offset].y;
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_2step_pair(cb, ld, sp);
    huffman_sign_bits(ld, sp, PAIR_LEN);
    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_quad(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint16_t offset = 0;

    while(!hcb3[offset].is_leaf) {
        uint8_t b = faad_get1bit(ld);
        offset += hcb3[offset].data[b];
    }
    if(offset > hcb_bin_table_size[cb]) {
        /* printf("ERROR: offset into hcb_bin_table = %d >%d!\n", offset,
           hcb_bin_table_size[cb]); */
        return 10;
    }
    sp[0] = hcb3[offset].data[0];
    sp[1] = hcb3[offset].data[1];
    sp[2] = hcb3[offset].data[2];
    sp[3] = hcb3[offset].data[3];
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_binary_quad(cb, ld, sp);
    huffman_sign_bits(ld, sp, QUAD_LEN);

    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_pair(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint16_t offset = 0;

    while(!hcb_bin_table[cb][offset].is_leaf) {
        uint8_t b = faad_get1bit(ld);
        offset += hcb_bin_table[cb][offset].data[b];
    }

    if(offset > hcb_bin_table_size[cb]) {
        /* printf("ERROR: offset into hcb_bin_table = %d >%d!\n", offset,
           hcb_bin_table_size[cb]); */
        return 10;
    }
    sp[0] = hcb_bin_table[cb][offset].data[0];
    sp[1] = hcb_bin_table[cb][offset].data[1];
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_binary_pair(cb, ld, sp);
    huffman_sign_bits(ld, sp, PAIR_LEN);
    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static int16_t huffman_codebook(uint8_t i) {
    static const uint32_t data = 16428320;
    if(i == 0) return (int16_t)(data >> 16) & 0xFFFF;
    else
        return (int16_t)data & 0xFFFF;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void vcb11_check_LAV(uint8_t cb, int16_t* sp) {
    static const uint16_t vcb11_LAV_tab[] = {16, 31, 47, 63, 95, 127, 159, 191, 223, 255, 319, 383, 511, 767, 1023, 2047};
    uint16_t              max = 0;

    if(cb < 16 || cb > 31) return;
    max = vcb11_LAV_tab[cb - 16];
    if((abs(sp[0]) > max) || (abs(sp[1]) > max)) {
        sp[0] = 0;
        sp[1] = 0;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t huffman_spectral_data(uint8_t cb, bitfile* ld, int16_t* sp) {
    switch(cb) {
    case 1: /* 2-step method for data quadruples */
    case 2: return huffman_2step_quad(cb, ld, sp);
    case 3: /* binary search for data quadruples */ return huffman_binary_quad_sign(cb, ld, sp);
    case 4: /* 2-step method for data quadruples */ return huffman_2step_quad_sign(cb, ld, sp);
    case 5: /* binary search for data pairs */ return huffman_binary_pair(cb, ld, sp);
    case 6: /* 2-step method for data pairs */ return huffman_2step_pair(cb, ld, sp);
    case 7: /* binary search for data pairs */
    case 9: return huffman_binary_pair_sign(cb, ld, sp);
    case 8: /* 2-step method for data pairs */
    case 10: return huffman_2step_pair_sign(cb, ld, sp);
    case 12: {
        uint8_t err = huffman_2step_pair(11, ld, sp);
        sp[0] = huffman_codebook(0);
        sp[1] = huffman_codebook(1);
        return err;
    }
    case 11: {
        uint8_t err = huffman_2step_pair_sign(11, ld, sp);
        if(!err) err = huffman_getescape(ld, &sp[0]);
        if(!err) err = huffman_getescape(ld, &sp[1]);
        return err;
    }
#ifdef ERROR_RESILIENCE
    /* VCB11 uses codebook 11 */
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31: {
        uint8_t err = huffman_2step_pair_sign(11, ld, sp);
        if(!err) err = huffman_getescape(ld, &sp[0]);
        if(!err) err = huffman_getescape(ld, &sp[1]);

        /* check LAV (Largest Absolute Value) */
        /* this finds errors in the ESCAPE signal */
        vcb11_check_LAV(cb, sp);

        return err;
    }
#endif
    default:
        /* Non existent codebook number, something went wrong */
        return 11;
    }

    return 0;
}

#ifdef ERROR_RESILIENCE

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* Special version of huffman_spectral_data. Will not read from a bitfile but a bits_t structure. Will keep track of the bits decoded and return the
   number of bits remaining. Do not read more than ld->len, return -1 if codeword would be longer */

int8_t huffman_spectral_data_2(uint8_t cb, bits_t* ld, int16_t* sp) {
    uint32_t cw;
    uint16_t offset = 0;
    uint8_t  extra_bits;
    uint8_t  i, vcb11 = 0;

    switch(cb) {
    case 1: /* 2-step method for data quadruples */
    case 2:
    case 4:

        cw = showbits_hcr(ld, hcbN[cb]);
        offset = hcb_table[cb][cw].offset;
        extra_bits = hcb_table[cb][cw].extra_bits;

        if(extra_bits) {
            /* we know for sure it's more than hcbN[cb] bits long */
            if(flushbits_hcr(ld, hcbN[cb])) return -1;
            offset += (uint16_t)showbits_hcr(ld, extra_bits);
            if(flushbits_hcr(ld, hcb_2_quad_table[cb][offset].bits - hcbN[cb])) return -1;
        }
        else {
            if(flushbits_hcr(ld, hcb_2_quad_table[cb][offset].bits)) return -1;
        }

        sp[0] = hcb_2_quad_table[cb][offset].x;
        sp[1] = hcb_2_quad_table[cb][offset].y;
        sp[2] = hcb_2_quad_table[cb][offset].v;
        sp[3] = hcb_2_quad_table[cb][offset].w;
        break;

    case 6: /* 2-step method for data pairs */
    case 8:
    case 10:
    case 11:
    /* VCB11 uses codebook 11 */
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:

        if(cb >= 16) {
            /* store the virtual codebook */
            vcb11 = cb;
            cb = 11;
        }

        cw = showbits_hcr(ld, hcbN[cb]);
        offset = hcb_table[cb][cw].offset;
        extra_bits = hcb_table[cb][cw].extra_bits;

        if(extra_bits) {
            /* we know for sure it's more than hcbN[cb] bits long */
            if(flushbits_hcr(ld, hcbN[cb])) return -1;
            offset += (uint16_t)showbits_hcr(ld, extra_bits);
            if(flushbits_hcr(ld, hcb_2_pair_table[cb][offset].bits - hcbN[cb])) return -1;
        }
        else {
            if(flushbits_hcr(ld, hcb_2_pair_table[cb][offset].bits)) return -1;
        }
        sp[0] = hcb_2_pair_table[cb][offset].x;
        sp[1] = hcb_2_pair_table[cb][offset].y;
        break;

    case 3: /* binary search for data quadruples */

        while(!hcb3[offset].is_leaf) {
            uint8_t b;

            if(get1bit_hcr(ld, &b)) return -1;
            offset += hcb3[offset].data[b];
        }

        sp[0] = hcb3[offset].data[0];
        sp[1] = hcb3[offset].data[1];
        sp[2] = hcb3[offset].data[2];
        sp[3] = hcb3[offset].data[3];

        break;

    case 5: /* binary search for data pairs */
    case 7:
    case 9:

        while(!hcb_bin_table[cb][offset].is_leaf) {
            uint8_t b;

            if(get1bit_hcr(ld, &b)) return -1;
            offset += hcb_bin_table[cb][offset].data[b];
        }

        sp[0] = hcb_bin_table[cb][offset].data[0];
        sp[1] = hcb_bin_table[cb][offset].data[1];

        break;
    }

    /* decode sign bits */
    if(unsigned_cb[cb]) {
        for(i = 0; i < ((cb < FIRST_PAIR_HCB) ? QUAD_LEN : PAIR_LEN); i++) {
            if(sp[i]) {
                uint8_t b;
                if(get1bit_hcr(ld, &b)) return -1;
                if(b != 0) { sp[i] = -sp[i]; }
            }
        }
    }

    /* decode huffman escape bits */
    if((cb == ESC_HCB) || (cb >= 16)) {
        uint8_t k;
        for(k = 0; k < 2; k++) {
            if((sp[k] == 16) || (sp[k] == -16)) {
                uint8_t  neg, i;
                int32_t  j;
                uint32_t off;

                neg = (sp[k] < 0) ? 1 : 0;

                for(i = 4;; i++) {
                    uint8_t b;
                    if(get1bit_hcr(ld, &b)) return -1;
                    if(b == 0) break;
                }

                if(getbits_hcr(ld, i, &off)) return -1;
                j = off + (1 << i);
                sp[k] = (int16_t)((neg) ? -j : j);
            }
        }

        if(vcb11 != 0) {
            /* check LAV (Largest Absolute Value) */
            /* this finds errors in the ESCAPE signal */
            vcb11_check_LAV(vcb11, sp);
        }
    }
    return ld->len;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t pow05_table[] = {
    COEF_CONST(1.68179283050743), /* 0.5^(-3/4) */
    COEF_CONST(1.41421356237310), /* 0.5^(-2/4) */
    COEF_CONST(1.18920711500272), /* 0.5^(-1/4) */
    COEF_CONST(1.0),              /* 0.5^( 0/4) */
    COEF_CONST(0.84089641525371), /* 0.5^(+1/4) */
    COEF_CONST(0.70710678118655), /* 0.5^(+2/4) */
    COEF_CONST(0.59460355750136)  /* 0.5^(+3/4) */
};

//----------------------------------------------------------------------------------------------------------------------------------------------------
void is_decode(ic_stream* ics, ic_stream* icsr, int32_t* l_spec, int32_t* r_spec, uint16_t frame_len) {
    uint8_t  g, sfb, b;
    uint16_t i;
    int32_t exp, frac;
    uint16_t nshort = frame_len / 8;
    uint8_t  group = 0;

    for(g = 0; g < icsr->num_window_groups; g++) {
        /* Do intensity stereo decoding */
        for(b = 0; b < icsr->window_group_length[g]; b++) {
            for(sfb = 0; sfb < icsr->max_sfb; sfb++) {
                if(is_intensity(icsr, g, sfb)) {
                    exp = icsr->scale_factors[g][sfb] >> 2;
                    frac = icsr->scale_factors[g][sfb] & 3;
                    /* Scale from left to right channel,
                       do not touch left channel */
                    for(i = icsr->swb_offset[sfb]; i < min(icsr->swb_offset[sfb + 1], ics->swb_offset_max); i++) {
                        if(exp < 0) r_spec[(group * nshort) + i] = l_spec[(group * nshort) + i] << -exp;
                        else
                            r_spec[(group * nshort) + i] = l_spec[(group * nshort) + i] >> exp;
                        r_spec[(group * nshort) + i] = MUL_C(r_spec[(group * nshort) + i], pow05_table[frac + 3]);
                        if(is_intensity(icsr, g, sfb) != invert_intensity(ics, g, sfb)) r_spec[(group * nshort) + i] = -r_spec[(group * nshort) + i];
                    }
                }
            }
            group++;
        }
    }
}

#ifdef ERROR_RESILIENCE
//----------------------------------------------------------------------------------------------------------------------------------------------------
/* rewind and reverse - 32 bit version */
static uint32_t rewrev_word(uint32_t v, const uint8_t len) {
    /* 32 bit reverse */
    v = ((v >> S[0]) & B[0]) | ((v << S[0]) & ~B[0]);
    v = ((v >> S[1]) & B[1]) | ((v << S[1]) & ~B[1]);
    v = ((v >> S[2]) & B[2]) | ((v << S[2]) & ~B[2]);
    v = ((v >> S[3]) & B[3]) | ((v << S[3]) & ~B[3]);
    v = ((v >> S[4]) & B[4]) | ((v << S[4]) & ~B[4]);

    /* shift off low bits */
    v >>= (32 - len);

    return v;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

/* 64 bit version */
static void rewrev_lword(uint32_t* hi, uint32_t* lo, const uint8_t len) {
    if(len <= 32) {
        *hi = 0;
        *lo = rewrev_word(*lo, len);
    }
    else {
        uint32_t t = *hi, v = *lo;

        /* double 32 bit reverse */
        v = ((v >> S[0]) & B[0]) | ((v << S[0]) & ~B[0]);
        t = ((t >> S[0]) & B[0]) | ((t << S[0]) & ~B[0]);
        v = ((v >> S[1]) & B[1]) | ((v << S[1]) & ~B[1]);
        t = ((t >> S[1]) & B[1]) | ((t << S[1]) & ~B[1]);
        v = ((v >> S[2]) & B[2]) | ((v << S[2]) & ~B[2]);
        t = ((t >> S[2]) & B[2]) | ((t << S[2]) & ~B[2]);
        v = ((v >> S[3]) & B[3]) | ((v << S[3]) & ~B[3]);
        t = ((t >> S[3]) & B[3]) | ((t << S[3]) & ~B[3]);
        v = ((v >> S[4]) & B[4]) | ((v << S[4]) & ~B[4]);
        t = ((t >> S[4]) & B[4]) | ((t << S[4]) & ~B[4]);

        /* last 32<>32 bit swap is implicit below */

        /* shift off low bits (this is really only one 64 bit shift) */
        *lo = (t >> (64 - len)) | (v << (len - 32));
        *hi = v >> (64 - len);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* bits_t version */
static void rewrev_bits(bits_t* bits) {
    if(bits->len == 0) return;
    rewrev_lword(&bits->bufb, &bits->bufa, bits->len);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* merge bits of a to b */
static void concat_bits(bits_t* b, bits_t* a) {
    uint32_t bl, bh, al, ah;

    if(a->len == 0) return;
    al = a->bufa;
    ah = a->bufb;
    if(b->len > 32) {
        /* maskoff superfluous high b bits */
        bl = b->bufa;
        bh = b->bufb & ((1 << (b->len - 32)) - 1);
        /* left shift a b->len bits */
        ah = al << (b->len - 32);
        al = 0;
    }
    else {
        bl = b->bufa & ((1 << (b->len)) - 1);
        bh = 0;
        ah = (ah << (b->len)) | (al >> (32 - b->len));
        al = al << b->len;
    }
    /* merge */
    b->bufa = bl | al;
    b->bufb = bh | ah;
    b->len += a->len;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t is_good_cb(uint8_t this_CB, uint8_t this_sec_CB) {
    /* only want spectral data CB's */
    if((this_sec_CB > ZERO_HCB && this_sec_CB <= ESC_HCB) || (this_sec_CB >= VCB11_FIRST && this_sec_CB <= VCB11_LAST)) {
        if(this_CB < ESC_HCB) {
            /* normal codebook pairs */
            return ((this_sec_CB == this_CB) || (this_sec_CB == this_CB + 1));
        }
        else {
            /* escape codebook */
            return (this_sec_CB == this_CB);
        }
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void read_segment(bits_t* segment, uint8_t segwidth, bitfile* ld) {
    segment->len = segwidth;

    if(segwidth > 32) {
        segment->bufb = faad_getbits(ld, segwidth - 32);
        segment->bufa = faad_getbits(ld, 32);
    }
    else {
        segment->bufa = faad_getbits(ld, segwidth);
        segment->bufb = 0;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void fill_in_codeword(codeword_t* codeword, uint16_t index, uint16_t sp, uint8_t cb) {
    codeword[index].sp_offset = sp;
    codeword[index].cb = cb;
    codeword[index].decoded = 0;
    codeword[index].bits.len = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t reordered_spectral_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, int16_t* spectral_data) {
    uint16_t   PCWs_done;
    uint16_t   numberOfSegments, numberOfSets, numberOfCodewords;
    codeword_t codeword[512];
    bits_t     segment[512];
    uint16_t   sp_offset[8];
    uint16_t   g, i, sortloop, set, bitsread;
    /*uint16_t bitsleft, codewordsleft*/;
    uint8_t w_idx, sfb, this_CB, last_CB, this_sec_CB;

    const uint16_t nshort = hDecoder->frameLength / 8;
    const uint16_t sp_data_len = ics->length_of_reordered_spectral_data;
    const uint8_t* PreSortCb;

    /* no data (e.g. silence) */
    if(sp_data_len == 0) return 0;
    /* since there is spectral data, at least one codeword has nonzero length */
    if(ics->length_of_longest_codeword == 0) return 10;
    if(sp_data_len < ics->length_of_longest_codeword) return 10;
    sp_offset[0] = 0;
    for(g = 1; g < ics->num_window_groups; g++) { sp_offset[g] = sp_offset[g - 1] + nshort * ics->window_group_length[g - 1]; }
    PCWs_done = 0;
    numberOfSegments = 0;
    numberOfCodewords = 0;
    bitsread = 0;
    /* VCB11 code books in use */
    if(hDecoder->aacSectionDataResilienceFlag) {
        PreSortCb = PreSortCB_ER;
        last_CB = NUM_CB_ER;
    }
    else {
        PreSortCb = PreSortCB_STD;
        last_CB = NUM_CB;
    }
    /* step 1: decode PCW's (set 0), and stuff data in easier-to-use format */
    for(sortloop = 0; sortloop < last_CB; sortloop++) {
        /* select codebook to process this pass */
        this_CB = PreSortCb[sortloop];
        /* loop over sfbs */
        for(sfb = 0; sfb < ics->max_sfb; sfb++) {
            /* loop over all in this sfb, 4 lines per loop */
            for(w_idx = 0; 4 * w_idx < (min(ics->swb_offset[sfb + 1], ics->swb_offset_max) - ics->swb_offset[sfb]); w_idx++) {
                for(g = 0; g < ics->num_window_groups; g++) {
                    for(i = 0; i < ics->num_sec[g]; i++) {
                        /* check whether sfb used here is the one we want to process */
                        if((ics->sect_start[g][i] <= sfb) && (ics->sect_end[g][i] > sfb)) {
                            /* check whether codebook used here is the one we want to process */
                            this_sec_CB = ics->sect_cb[g][i];

                            if(is_good_cb(this_CB, this_sec_CB)) {
                                /* precalculate some stuff */
                                uint16_t sect_sfb_size = ics->sect_sfb_offset[g][sfb + 1] - ics->sect_sfb_offset[g][sfb];
                                uint8_t  inc = (this_sec_CB < FIRST_PAIR_HCB) ? QUAD_LEN : PAIR_LEN;
                                uint16_t group_cws_count = (4 * ics->window_group_length[g]) / inc;
                                uint8_t  segwidth = segmentWidth(this_sec_CB);
                                uint16_t cws;

                                /* read codewords until end of sfb or end of window group (shouldn't only 1 trigger?) */
                                for(cws = 0; (cws < group_cws_count) && ((cws + w_idx * group_cws_count) < sect_sfb_size); cws++) {
                                    uint16_t sp = sp_offset[g] + ics->sect_sfb_offset[g][sfb] + inc * (cws + w_idx * group_cws_count);
                                    /* read and decode PCW */
                                    if(!PCWs_done) {
                                        /* read in normal segments */
                                        if(bitsread + segwidth <= sp_data_len) {
                                            read_segment(&segment[numberOfSegments], segwidth, ld);
                                            bitsread += segwidth;

                                            huffman_spectral_data_2(this_sec_CB, &segment[numberOfSegments], &spectral_data[sp]);

                                            /* keep leftover bits */
                                            rewrev_bits(&segment[numberOfSegments]);

                                            numberOfSegments++;
                                        }
                                        else {
                                            /* remaining stuff after last segment, we unfortunately couldn't read
                                               this in earlier because it might not fit in 64 bits. since we already
                                               decoded (and removed) the PCW it is now guaranteed to fit */
                                            if(bitsread < sp_data_len) {
                                                const uint8_t additional_bits = sp_data_len - bitsread;
                                                read_segment(&segment[numberOfSegments], additional_bits, ld);
                                                segment[numberOfSegments].len += segment[numberOfSegments - 1].len;
                                                rewrev_bits(&segment[numberOfSegments]);
                                                if(segment[numberOfSegments - 1].len > 32) {
                                                    segment[numberOfSegments - 1].bufb =
                                                        segment[numberOfSegments].bufb +
                                                        showbits_hcr(&segment[numberOfSegments - 1], segment[numberOfSegments - 1].len - 32);
                                                    segment[numberOfSegments - 1].bufa =
                                                        segment[numberOfSegments].bufa + showbits_hcr(&segment[numberOfSegments - 1], 32);
                                                }
                                                else {
                                                    segment[numberOfSegments - 1].bufa =
                                                        segment[numberOfSegments].bufa +
                                                        showbits_hcr(&segment[numberOfSegments - 1], segment[numberOfSegments - 1].len);
                                                    segment[numberOfSegments - 1].bufb = segment[numberOfSegments].bufb;
                                                }
                                                segment[numberOfSegments - 1].len += additional_bits;
                                            }
                                            bitsread = sp_data_len;
                                            PCWs_done = 1;
                                            fill_in_codeword(codeword, 0, sp, this_sec_CB);
                                        }
                                    }
                                    else { fill_in_codeword(codeword, numberOfCodewords - numberOfSegments, sp, this_sec_CB); }
                                    numberOfCodewords++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(numberOfSegments == 0) return 10;
    numberOfSets = numberOfCodewords / numberOfSegments;
    /* step 2: decode nonPCWs */
    for(set = 1; set <= numberOfSets; set++) {
        uint16_t trial;
        for(trial = 0; trial < numberOfSegments; trial++) {
            uint16_t codewordBase;
            for(codewordBase = 0; codewordBase < numberOfSegments; codewordBase++) {
                const uint16_t segment_idx = (trial + codewordBase) % numberOfSegments;
                const uint16_t codeword_idx = codewordBase + set * numberOfSegments - numberOfSegments;
                /* data up */
                if(codeword_idx >= numberOfCodewords - numberOfSegments) break;
                if(!codeword[codeword_idx].decoded && segment[segment_idx].len > 0) {
                    uint8_t tmplen;
                    if(codeword[codeword_idx].bits.len != 0) concat_bits(&segment[segment_idx], &codeword[codeword_idx].bits);
                    tmplen = segment[segment_idx].len;
                    if(huffman_spectral_data_2(codeword[codeword_idx].cb, &segment[segment_idx], &spectral_data[codeword[codeword_idx].sp_offset]) >=
                       0) {
                        codeword[codeword_idx].decoded = 1;
                    }
                    else {
                        codeword[codeword_idx].bits = segment[segment_idx];
                        codeword[codeword_idx].bits.len = tmplen;
                    }
                }
            }
        }
        for(i = 0; i < numberOfSegments; i++) rewrev_bits(&segment[i]);
    }
    return 0;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------
void ms_decode(ic_stream* ics, ic_stream* icsr, int32_t* l_spec, int32_t* r_spec, uint16_t frame_len) {
    uint8_t  g, b, sfb;
    uint8_t  group = 0;
    uint16_t nshort = frame_len / 8;
    uint16_t i, k;
    int32_t  tmp;

    if(ics->ms_mask_present >= 1) {
        for(g = 0; g < ics->num_window_groups; g++) {
            for(b = 0; b < ics->window_group_length[g]; b++) {
                for(sfb = 0; sfb < ics->max_sfb; sfb++) {
                    /* If intensity stereo coding or noise substitution is on for a particular scalefactor band, no M/S stereo decoding
                       is carried out.
                     */
                    if((ics->ms_used[g][sfb] || ics->ms_mask_present == 2) && !is_intensity(icsr, g, sfb) && !is_noise(ics, g, sfb)) {
                        for(i = ics->swb_offset[sfb]; i < min(ics->swb_offset[sfb + 1], ics->swb_offset_max); i++) {
                            k = (group * nshort) + i;
                            tmp = l_spec[k] - r_spec[k];
                            l_spec[k] = l_spec[k] + r_spec[k];
                            r_spec[k] = tmp;
                        }
                    }
                }
                group++;
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int32_t get_sample(int32_t** input, uint8_t channel, uint16_t sample, uint8_t down_matrix, uint8_t up_matrix, uint8_t* internal_channel) {
    if(up_matrix == 1) return input[internal_channel[0]][sample];

    if(!down_matrix) return input[internal_channel[channel]][sample];

    if(channel == 0) {
        int32_t C = MUL_F(input[internal_channel[0]][sample], RSQRT2);
        int32_t L_S = MUL_F(input[internal_channel[3]][sample], RSQRT2);
        int32_t cum = input[internal_channel[1]][sample] + C + L_S;
        return MUL_F(cum, DM_MUL);
    }
    else {
        int32_t C = MUL_F(input[internal_channel[0]][sample], RSQRT2);
        int32_t R_S = MUL_F(input[internal_channel[4]][sample], RSQRT2);
        int32_t cum = input[internal_channel[2]][sample] + C + R_S;
        return MUL_F(cum, DM_MUL);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void* output_to_PCM(NeAACDecStruct* hDecoder, int32_t** input, void* sample_buffer, uint8_t channels, uint16_t frame_len, uint8_t format) {
    uint8_t  ch;
    uint16_t i;
    int16_t* short_sample_buffer = (int16_t*)sample_buffer;
    int32_t* int_sample_buffer = (int32_t*)sample_buffer;

    /* Copy output to a standard PCM buffer */
    for(ch = 0; ch < channels; ch++) {
        switch(format) {
        case FAAD_FMT_16BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (REAL_BITS - 1));
                    if(tmp >= REAL_CONST(32767)) { tmp = REAL_CONST(32767); }
                }
                else {
                    tmp += -(1 << (REAL_BITS - 1));
                    if(tmp <= REAL_CONST(-32768)) { tmp = REAL_CONST(-32768); }
                }
                tmp >>= REAL_BITS;
                short_sample_buffer[(i * channels) + ch] = (int16_t)tmp;
            }
            break;
        case FAAD_FMT_24BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (REAL_BITS - 9));
                    tmp >>= (REAL_BITS - 8);
                    if(tmp >= 8388607) { tmp = 8388607; }
                }
                else {
                    tmp += -(1 << (REAL_BITS - 9));
                    tmp >>= (REAL_BITS - 8);
                    if(tmp <= -8388608) { tmp = -8388608; }
                }
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        case FAAD_FMT_32BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (16 - REAL_BITS - 1));
                    tmp <<= (16 - REAL_BITS);
                }
                else {
                    tmp += -(1 << (16 - REAL_BITS - 1));
                    tmp <<= (16 - REAL_BITS);
                }
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        case FAAD_FMT_FIXED:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        }
    }

    return sample_buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* fixed point square root approximation */
/* !!!! ONLY WORKS FOR EVEN %REAL_BITS% !!!! */
int32_t fp_sqrt(int32_t value) {
    int32_t root = 0;

    step(0);
    step(2);
    step(4);
    step(6);
    step(8);
    step(10);
    step(12);
    step(14);
    step(16);
    step(18);
    step(20);
    step(22);
    step(24);
    step(26);
    step(28);
    step(30);
    if(root < value) ++root;
    root <<= (REAL_BITS / 2);
    return root;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* The function gen_rand_vector(addr, size) generates a vector of length  <size> with signed random values of average energy MEAN_NRG per random
   value. A suitable random number generator can be realized using one multiplication/accumulation per random value.
*/
static inline void gen_rand_vector(int32_t* spec, int16_t scale_factor, uint16_t size, uint8_t sub, uint32_t* __r1, uint32_t* __r2) {
    uint16_t i;
    int32_t  energy = 0, scale;
    int32_t  exp, frac;

    for(i = 0; i < size; i++) {
        /* this can be replaced by a 16 bit random generator!!!! */
        int32_t tmp = (int32_t)ne_rng(__r1, __r2);
        if(tmp < 0) tmp = -(tmp & ((1 << (REAL_BITS - 1)) - 1));
        else { tmp = (tmp & ((1 << (REAL_BITS - 1)) - 1)); }
        energy += MUL_R(tmp, tmp);
        spec[i] = tmp;
    }
    energy = fp_sqrt(energy);
    if(energy > 0) {
        scale = DIV(REAL_CONST(1), energy);
        exp = scale_factor >> 2;
        frac = scale_factor & 3;
        /* IMDCT pre-scaling */
        exp -= sub;
        if(exp < 0) scale >>= -exp;
        else
            scale <<= exp;

        if(frac) scale = MUL_C(scale, pow2_table[frac]);
        for(i = 0; i < size; i++) { spec[i] = MUL_R(spec[i], scale); }
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void pns_decode(ic_stream* ics_left, ic_stream* ics_right, int32_t* spec_left, int32_t* spec_right, uint16_t frame_len, uint8_t channel_pair, uint8_t object_type,
                /* RNG states */ uint32_t* __r1, uint32_t* __r2) {
    uint8_t  g, sfb, b;
    uint16_t size, offs;
    uint8_t  group = 0;
    uint16_t nshort = frame_len >> 3;
    uint8_t sub = 0;


    /* IMDCT scaling */
    if(object_type == LD) { sub = 9 /*9*/; }
    else {
        if(ics_left->window_sequence == EIGHT_SHORT_SEQUENCE) sub = 7 /*7*/;
        else
            sub = 10 /*10*/;
    }

    for(g = 0; g < ics_left->num_window_groups; g++) {
        /* Do perceptual noise substitution decoding */
        for(b = 0; b < ics_left->window_group_length[g]; b++) {
            for(sfb = 0; sfb < ics_left->max_sfb; sfb++) {
                uint32_t r1_dep = 0, r2_dep = 0;
                if(is_noise(ics_left, g, sfb)) {
#ifdef LTP_DEC
                    /* Simultaneous use of LTP and PNS is not prevented in the syntax. If both LTP, and PNS are enabled on the same
                       scalefactor band, PNS takes precedence, and no prediction is applied to this band.
                    */
                    ics_left->ltp.long_used[sfb] = 0;
                    ics_left->ltp2.long_used[sfb] = 0;
#endif
                    offs = ics_left->swb_offset[sfb];
                    size = min(ics_left->swb_offset[sfb + 1], ics_left->swb_offset_max) - offs;
                    r1_dep = *__r1;
                    r2_dep = *__r2;
                    /* Generate random vector */
                    gen_rand_vector(&spec_left[(group * nshort) + offs], ics_left->scale_factors[g][sfb], size, sub, __r1, __r2);
                }
                /* From the spec:
                   If the same scalefactor band and group is coded by perceptual noise substitution in both channels of a channel pair, the correlation of
                   the noise signal can be controlled by means of the ms_used field: While the default noise generation process works independently for each channel
                   (separate generation of random vectors), the same random vector is used for both channels if ms_used[] is set for a particular scalefactor band
                   and group. In this case, no M/S stereo coding is carried out (because M/S stereo coding and noise substitution coding are mutually exclusive).
                   If the same scalefactor band and group is coded by perceptual noise substitution in only one channel of a channel pair the setting of ms_used[]
                   is not evaluated.
                */
                if((ics_right != NULL) && is_noise(ics_right, g, sfb)) {
#ifdef LTP_DEC
                    /* See comment above. */
                    ics_right->ltp.long_used[sfb] = 0;
                    ics_right->ltp2.long_used[sfb] = 0;
#endif
                    if(channel_pair && is_noise(ics_left, g, sfb) && (((ics_left->ms_mask_present == 1) && (ics_left->ms_used[g][sfb])) || (ics_left->ms_mask_present == 2))) {
                        /*uint16_t c;*/
                        offs = ics_right->swb_offset[sfb];
                        size = min(ics_right->swb_offset[sfb + 1], ics_right->swb_offset_max) - offs;
                        /* Generate random vector dependent on left channel*/
                        gen_rand_vector(&spec_right[(group * nshort) + offs], ics_right->scale_factors[g][sfb], size, sub, &r1_dep, &r2_dep);
                    }
                    else /*if (ics_left->ms_mask_present == 0)*/ {
                        offs = ics_right->swb_offset[sfb];
                        size = min(ics_right->swb_offset[sfb + 1], ics_right->swb_offset_max) - offs;
                        /* Generate random vector */
                        gen_rand_vector(&spec_right[(group * nshort) + offs], ics_right->scale_factors[g][sfb], size, sub, __r1, __r2);
                    }
                }
            } /* sfb */
            group++;
        } /* b */
    }     /* g */
}

#ifdef PS_DEC
//----------------------------------------------------------------------------------------------------------------------------------------------------
static hyb_info* hybrid_init(uint8_t numTimeSlotsRate) {
    uint8_t i;
    hyb_info* hyb = (hyb_info*)faad_malloc(sizeof(hyb_info));
    hyb->resolution34[0] = 12;
    hyb->resolution34[1] = 8;
    hyb->resolution34[2] = 4;
    hyb->resolution34[3] = 4;
    hyb->resolution34[4] = 4;
    hyb->resolution20[0] = 8;
    hyb->resolution20[1] = 2;
    hyb->resolution20[2] = 2;
    hyb->frame_len = numTimeSlotsRate;
    hyb->work = (complex_t*)faad_malloc((hyb->frame_len + 12) * sizeof(complex_t));
    memset(hyb->work, 0, (hyb->frame_len + 12) * sizeof(complex_t));

    hyb->buffer = (complex_t**)faad_malloc(5 * sizeof(complex_t*));
    for(i = 0; i < 5; i++) {
        hyb->buffer[i] = (complex_t*)faad_malloc(hyb->frame_len * sizeof(complex_t));
        memset(hyb->buffer[i], 0, hyb->frame_len * sizeof(complex_t));
    }
    hyb->temp = (complex_t**)faad_malloc(hyb->frame_len * sizeof(complex_t*));
    for(i = 0; i < hyb->frame_len; i++) { hyb->temp[i] = (complex_t*)faad_malloc(12 /*max*/ * sizeof(complex_t)); }
    return hyb;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void hybrid_free(hyb_info* hyb) {
    uint8_t i;

    if(!hyb) return;
    if(hyb->work) faad_free(hyb->work);
    for(i = 0; i < 5; i++) {
        if(hyb->buffer[i]) faad_free(hyb->buffer[i]);
    }
    if(hyb->buffer) faad_free(hyb->buffer);
    for(i = 0; i < hyb->frame_len; i++) {
        if(hyb->temp[i]) faad_free(hyb->temp[i]);
    }
    if(hyb->temp) faad_free(hyb->temp);
    faad_free(hyb);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* real filter, size 2 */
static void channel_filter2(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid) {
    uint8_t i;

    for(i = 0; i < frame_len; i++) {
        int32_t r0 = MUL_F(filter[0], (QMF_RE(buffer[0 + i]) + QMF_RE(buffer[12 + i])));
        int32_t r1 = MUL_F(filter[1], (QMF_RE(buffer[1 + i]) + QMF_RE(buffer[11 + i])));
        int32_t r2 = MUL_F(filter[2], (QMF_RE(buffer[2 + i]) + QMF_RE(buffer[10 + i])));
        int32_t r3 = MUL_F(filter[3], (QMF_RE(buffer[3 + i]) + QMF_RE(buffer[9 + i])));
        int32_t r4 = MUL_F(filter[4], (QMF_RE(buffer[4 + i]) + QMF_RE(buffer[8 + i])));
        int32_t r5 = MUL_F(filter[5], (QMF_RE(buffer[5 + i]) + QMF_RE(buffer[7 + i])));
        int32_t r6 = MUL_F(filter[6], QMF_RE(buffer[6 + i]));
        int32_t i0 = MUL_F(filter[0], (QMF_IM(buffer[0 + i]) + QMF_IM(buffer[12 + i])));
        int32_t i1 = MUL_F(filter[1], (QMF_IM(buffer[1 + i]) + QMF_IM(buffer[11 + i])));
        int32_t i2 = MUL_F(filter[2], (QMF_IM(buffer[2 + i]) + QMF_IM(buffer[10 + i])));
        int32_t i3 = MUL_F(filter[3], (QMF_IM(buffer[3 + i]) + QMF_IM(buffer[9 + i])));
        int32_t i4 = MUL_F(filter[4], (QMF_IM(buffer[4 + i]) + QMF_IM(buffer[8 + i])));
        int32_t i5 = MUL_F(filter[5], (QMF_IM(buffer[5 + i]) + QMF_IM(buffer[7 + i])));
        int32_t i6 = MUL_F(filter[6], QMF_IM(buffer[6 + i]));

        /* q = 0 */
        QMF_RE(X_hybrid[i][0]) = r0 + r1 + r2 + r3 + r4 + r5 + r6;
        QMF_IM(X_hybrid[i][0]) = i0 + i1 + i2 + i3 + i4 + i5 + i6;

        /* q = 1 */
        QMF_RE(X_hybrid[i][1]) = r0 - r1 + r2 - r3 + r4 - r5 + r6;
        QMF_IM(X_hybrid[i][1]) = i0 - i1 + i2 - i3 + i4 - i5 + i6;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* complex filter, size 4 */
static void channel_filter4(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid) {
    uint8_t i;
    int32_t input_re1[2], input_re2[2], input_im1[2], input_im2[2];

    for(i = 0; i < frame_len; i++) {
        input_re1[0] = -MUL_F(filter[2], (QMF_RE(buffer[i + 2]) + QMF_RE(buffer[i + 10]))) + MUL_F(filter[6], QMF_RE(buffer[i + 6]));
        input_re1[1] = MUL_F(FRAC_CONST(-0.70710678118655),
                             (MUL_F(filter[1], (QMF_RE(buffer[i + 1]) + QMF_RE(buffer[i + 11]))) + MUL_F(filter[3], (QMF_RE(buffer[i + 3]) + QMF_RE(buffer[i + 9]))) - MUL_F(filter[5], (QMF_RE(buffer[i + 5]) + QMF_RE(buffer[i + 7])))));
        input_im1[0] = MUL_F(filter[0], (QMF_IM(buffer[i + 0]) - QMF_IM(buffer[i + 12]))) - MUL_F(filter[4], (QMF_IM(buffer[i + 4]) - QMF_IM(buffer[i + 8])));
        input_im1[1] = MUL_F(FRAC_CONST(0.70710678118655),
                             (MUL_F(filter[1], (QMF_IM(buffer[i + 1]) - QMF_IM(buffer[i + 11]))) - MUL_F(filter[3], (QMF_IM(buffer[i + 3]) - QMF_IM(buffer[i + 9]))) - MUL_F(filter[5], (QMF_IM(buffer[i + 5]) - QMF_IM(buffer[i + 7])))));
        input_re2[0] = MUL_F(filter[0], (QMF_RE(buffer[i + 0]) - QMF_RE(buffer[i + 12]))) - MUL_F(filter[4], (QMF_RE(buffer[i + 4]) - QMF_RE(buffer[i + 8])));
        input_re2[1] = MUL_F(FRAC_CONST(0.70710678118655),
                             (MUL_F(filter[1], (QMF_RE(buffer[i + 1]) - QMF_RE(buffer[i + 11]))) - MUL_F(filter[3], (QMF_RE(buffer[i + 3]) - QMF_RE(buffer[i + 9]))) - MUL_F(filter[5], (QMF_RE(buffer[i + 5]) - QMF_RE(buffer[i + 7])))));
        input_im2[0] = -MUL_F(filter[2], (QMF_IM(buffer[i + 2]) + QMF_IM(buffer[i + 10]))) + MUL_F(filter[6], QMF_IM(buffer[i + 6]));
        input_im2[1] = MUL_F(FRAC_CONST(-0.70710678118655),
                             (MUL_F(filter[1], (QMF_IM(buffer[i + 1]) + QMF_IM(buffer[i + 11]))) + MUL_F(filter[3], (QMF_IM(buffer[i + 3]) + QMF_IM(buffer[i + 9]))) - MUL_F(filter[5], (QMF_IM(buffer[i + 5]) + QMF_IM(buffer[i + 7])))));
        /* q == 0 */
        QMF_RE(X_hybrid[i][0]) = input_re1[0] + input_re1[1] + input_im1[0] + input_im1[1];
        QMF_IM(X_hybrid[i][0]) = -input_re2[0] - input_re2[1] + input_im2[0] + input_im2[1];
        /* q == 1 */
        QMF_RE(X_hybrid[i][1]) = input_re1[0] - input_re1[1] - input_im1[0] + input_im1[1];
        QMF_IM(X_hybrid[i][1]) = input_re2[0] - input_re2[1] + input_im2[0] - input_im2[1];
        /* q == 2 */
        QMF_RE(X_hybrid[i][2]) = input_re1[0] - input_re1[1] + input_im1[0] - input_im1[1];
        QMF_IM(X_hybrid[i][2]) = -input_re2[0] + input_re2[1] + input_im2[0] - input_im2[1];
        /* q == 3 */
        QMF_RE(X_hybrid[i][3]) = input_re1[0] + input_re1[1] - input_im1[0] - input_im1[1];
        QMF_IM(X_hybrid[i][3]) = input_re2[0] + input_re2[1] + input_im2[0] + input_im2[1];
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void inline DCT3_4_unscaled(int32_t* y, int32_t* x) {
    int32_t f0, f1, f2, f3, f4, f5, f6, f7, f8;

    f0 = MUL_F(x[2], FRAC_CONST(0.7071067811865476));
    f1 = x[0] - f0;
    f2 = x[0] + f0;
    f3 = x[1] + x[3];
    f4 = MUL_C(x[1], COEF_CONST(1.3065629648763766));
    f5 = MUL_F(f3, FRAC_CONST(-0.9238795325112866));
    f6 = MUL_F(x[3], FRAC_CONST(-0.5411961001461967));
    f7 = f4 + f5;
    f8 = f6 - f5;
    y[3] = f2 - f8;
    y[0] = f2 + f8;
    y[2] = f1 - f7;
    y[1] = f1 + f7;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* complex filter, size 8 */
static void channel_filter8(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid) {
    uint8_t i, n;
    int32_t input_re1[4], input_re2[4], input_im1[4], input_im2[4];
    int32_t x[4];

    for(i = 0; i < frame_len; i++) {
        input_re1[0] = MUL_F(filter[6], QMF_RE(buffer[6 + i]));
        input_re1[1] = MUL_F(filter[5], (QMF_RE(buffer[5 + i]) + QMF_RE(buffer[7 + i])));
        input_re1[2] = -MUL_F(filter[0], (QMF_RE(buffer[0 + i]) + QMF_RE(buffer[12 + i]))) + MUL_F(filter[4], (QMF_RE(buffer[4 + i]) + QMF_RE(buffer[8 + i])));
        input_re1[3] = -MUL_F(filter[1], (QMF_RE(buffer[1 + i]) + QMF_RE(buffer[11 + i]))) + MUL_F(filter[3], (QMF_RE(buffer[3 + i]) + QMF_RE(buffer[9 + i])));
        input_im1[0] = MUL_F(filter[5], (QMF_IM(buffer[7 + i]) - QMF_IM(buffer[5 + i])));
        input_im1[1] = MUL_F(filter[0], (QMF_IM(buffer[12 + i]) - QMF_IM(buffer[0 + i]))) + MUL_F(filter[4], (QMF_IM(buffer[8 + i]) - QMF_IM(buffer[4 + i])));
        input_im1[2] = MUL_F(filter[1], (QMF_IM(buffer[11 + i]) - QMF_IM(buffer[1 + i]))) + MUL_F(filter[3], (QMF_IM(buffer[9 + i]) - QMF_IM(buffer[3 + i])));
        input_im1[3] = MUL_F(filter[2], (QMF_IM(buffer[10 + i]) - QMF_IM(buffer[2 + i])));

        for(n = 0; n < 4; n++) { x[n] = input_re1[n] - input_im1[3 - n]; }
        DCT3_4_unscaled(x, x);
        QMF_RE(X_hybrid[i][7]) = x[0];
        QMF_RE(X_hybrid[i][5]) = x[2];
        QMF_RE(X_hybrid[i][3]) = x[3];
        QMF_RE(X_hybrid[i][1]) = x[1];
        for(n = 0; n < 4; n++) { x[n] = input_re1[n] + input_im1[3 - n]; }
        DCT3_4_unscaled(x, x);
        QMF_RE(X_hybrid[i][6]) = x[1];
        QMF_RE(X_hybrid[i][4]) = x[3];
        QMF_RE(X_hybrid[i][2]) = x[2];
        QMF_RE(X_hybrid[i][0]) = x[0];

        input_im2[0] = MUL_F(filter[6], QMF_IM(buffer[6 + i]));
        input_im2[1] = MUL_F(filter[5], (QMF_IM(buffer[5 + i]) + QMF_IM(buffer[7 + i])));
        input_im2[2] = -MUL_F(filter[0], (QMF_IM(buffer[0 + i]) + QMF_IM(buffer[12 + i]))) + MUL_F(filter[4], (QMF_IM(buffer[4 + i]) + QMF_IM(buffer[8 + i])));
        input_im2[3] = -MUL_F(filter[1], (QMF_IM(buffer[1 + i]) + QMF_IM(buffer[11 + i]))) + MUL_F(filter[3], (QMF_IM(buffer[3 + i]) + QMF_IM(buffer[9 + i])));
        input_re2[0] = MUL_F(filter[5], (QMF_RE(buffer[7 + i]) - QMF_RE(buffer[5 + i])));
        input_re2[1] = MUL_F(filter[0], (QMF_RE(buffer[12 + i]) - QMF_RE(buffer[0 + i]))) + MUL_F(filter[4], (QMF_RE(buffer[8 + i]) - QMF_RE(buffer[4 + i])));
        input_re2[2] = MUL_F(filter[1], (QMF_RE(buffer[11 + i]) - QMF_RE(buffer[1 + i]))) + MUL_F(filter[3], (QMF_RE(buffer[9 + i]) - QMF_RE(buffer[3 + i])));
        input_re2[3] = MUL_F(filter[2], (QMF_RE(buffer[10 + i]) - QMF_RE(buffer[2 + i])));

        for(n = 0; n < 4; n++) { x[n] = input_im2[n] + input_re2[3 - n]; }
        DCT3_4_unscaled(x, x);
        QMF_IM(X_hybrid[i][7]) = x[0];
        QMF_IM(X_hybrid[i][5]) = x[2];
        QMF_IM(X_hybrid[i][3]) = x[3];
        QMF_IM(X_hybrid[i][1]) = x[1];

        for(n = 0; n < 4; n++) { x[n] = input_im2[n] - input_re2[3 - n]; }
        DCT3_4_unscaled(x, x);
        QMF_IM(X_hybrid[i][6]) = x[1];
        QMF_IM(X_hybrid[i][4]) = x[3];
        QMF_IM(X_hybrid[i][2]) = x[2];
        QMF_IM(X_hybrid[i][0]) = x[0];
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void inline DCT3_6_unscaled(int32_t* y, int32_t* x) {
    int32_t f0, f1, f2, f3, f4, f5, f6, f7;

    f0 = MUL_F(x[3], FRAC_CONST(0.70710678118655));
    f1 = x[0] + f0;
    f2 = x[0] - f0;
    f3 = MUL_F((x[1] - x[5]), FRAC_CONST(0.70710678118655));
    f4 = MUL_F(x[2], FRAC_CONST(0.86602540378444)) + MUL_F(x[4], FRAC_CONST(0.5));
    f5 = f4 - x[4];
    f6 = MUL_F(x[1], FRAC_CONST(0.96592582628907)) + MUL_F(x[5], FRAC_CONST(0.25881904510252));
    f7 = f6 - f3;
    y[0] = f1 + f6 + f4;
    y[1] = f2 + f3 - x[4];
    y[2] = f7 + f2 - f5;
    y[3] = f1 - f7 - f5;
    y[4] = f1 - f3 - x[4];
    y[5] = f2 - f6 + f4;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* complex filter, size 12 */
static void channel_filter12(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid) {
    uint8_t i, n;
    int32_t input_re1[6], input_re2[6], input_im1[6], input_im2[6];
    int32_t out_re1[6], out_re2[6], out_im1[6], out_im2[6];

    for(i = 0; i < frame_len; i++) {
        for(n = 0; n < 6; n++) {
            if(n == 0) {
                input_re1[0] = MUL_F(QMF_RE(buffer[6 + i]), filter[6]);
                input_re2[0] = MUL_F(QMF_IM(buffer[6 + i]), filter[6]);
            }
            else {
                input_re1[6 - n] = MUL_F((QMF_RE(buffer[n + i]) + QMF_RE(buffer[12 - n + i])), filter[n]);
                input_re2[6 - n] = MUL_F((QMF_IM(buffer[n + i]) + QMF_IM(buffer[12 - n + i])), filter[n]);
            }
            input_im2[n] = MUL_F((QMF_RE(buffer[n + i]) - QMF_RE(buffer[12 - n + i])), filter[n]);
            input_im1[n] = MUL_F((QMF_IM(buffer[n + i]) - QMF_IM(buffer[12 - n + i])), filter[n]);
        }

        DCT3_6_unscaled(out_re1, input_re1);
        DCT3_6_unscaled(out_re2, input_re2);
        DCT3_6_unscaled(out_im1, input_im1);
        DCT3_6_unscaled(out_im2, input_im2);

        for(n = 0; n < 6; n += 2) {
            QMF_RE(X_hybrid[i][n]) = out_re1[n] - out_im1[n];
            QMF_IM(X_hybrid[i][n]) = out_re2[n] + out_im2[n];
            QMF_RE(X_hybrid[i][n + 1]) = out_re1[n + 1] + out_im1[n + 1];
            QMF_IM(X_hybrid[i][n + 1]) = out_re2[n + 1] - out_im2[n + 1];
            QMF_RE(X_hybrid[i][10 - n]) = out_re1[n + 1] - out_im1[n + 1];
            QMF_IM(X_hybrid[i][10 - n]) = out_re2[n + 1] + out_im2[n + 1];
            QMF_RE(X_hybrid[i][11 - n]) = out_re1[n] + out_im1[n];
            QMF_IM(X_hybrid[i][11 - n]) = out_re2[n] - out_im2[n];
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* Hybrid analysis: further split up QMF subbands
 * to improve frequency resolution
 */
static void hybrid_analysis(hyb_info* hyb, complex_t X[32][64], complex_t X_hybrid[32][32], uint8_t use34, uint8_t numTimeSlotsRate) {
    uint8_t  k, n, band;
    uint8_t  offset = 0;
    uint8_t  qmf_bands = (use34) ? 5 : 3;
    uint8_t* resolution = (use34) ? hyb->resolution34 : hyb->resolution20;

    for(band = 0; band < qmf_bands; band++) {
        /* build working buffer */
        memcpy(hyb->work, hyb->buffer[band], 12 * sizeof(complex_t));
        /* add new samples */
        for(n = 0; n < hyb->frame_len; n++) {
            QMF_RE(hyb->work[12 + n]) = QMF_RE(X[n + 6 /*delay*/][band]);
            QMF_IM(hyb->work[12 + n]) = QMF_IM(X[n + 6 /*delay*/][band]);
        }
        /* store samples */
        memcpy(hyb->buffer[band], hyb->work + hyb->frame_len, 12 * sizeof(complex_t));
        switch(resolution[band]) {
        case 2:
            /* Type B real filter, Q[p] = 2 */
            channel_filter2(hyb, hyb->frame_len, p2_13_20, hyb->work, hyb->temp);
            break;
        case 4:
            /* Type A complex filter, Q[p] = 4 */
            channel_filter4(hyb, hyb->frame_len, p4_13_34, hyb->work, hyb->temp);
            break;
        case 8:
            /* Type A complex filter, Q[p] = 8 */
            channel_filter8(hyb, hyb->frame_len, (use34) ? p8_13_34 : p8_13_20, hyb->work, hyb->temp);
            break;
        case 12:
            /* Type A complex filter, Q[p] = 12 */
            channel_filter12(hyb, hyb->frame_len, p12_13_34, hyb->work, hyb->temp);
            break;
        }
        for(n = 0; n < hyb->frame_len; n++) {
            for(k = 0; k < resolution[band]; k++) {
                QMF_RE(X_hybrid[n][offset + k]) = QMF_RE(hyb->temp[n][k]);
                QMF_IM(X_hybrid[n][offset + k]) = QMF_IM(hyb->temp[n][k]);
            }
        }
        offset += resolution[band];
    }

    /* group hybrid channels */
    if(!use34) {
        for(n = 0; n < numTimeSlotsRate; n++) {
            QMF_RE(X_hybrid[n][3]) += QMF_RE(X_hybrid[n][4]);
            QMF_IM(X_hybrid[n][3]) += QMF_IM(X_hybrid[n][4]);
            QMF_RE(X_hybrid[n][4]) = 0;
            QMF_IM(X_hybrid[n][4]) = 0;
            QMF_RE(X_hybrid[n][2]) += QMF_RE(X_hybrid[n][5]);
            QMF_IM(X_hybrid[n][2]) += QMF_IM(X_hybrid[n][5]);
            QMF_RE(X_hybrid[n][5]) = 0;
            QMF_IM(X_hybrid[n][5]) = 0;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void hybrid_synthesis(hyb_info* hyb, complex_t X[32][64], complex_t X_hybrid[32][32], uint8_t use34, uint8_t numTimeSlotsRate) {
    uint8_t  k, n, band;
    uint8_t  offset = 0;
    uint8_t  qmf_bands = (use34) ? 5 : 3;
    uint8_t* resolution = (use34) ? hyb->resolution34 : hyb->resolution20;

    for(band = 0; band < qmf_bands; band++) {
        for(n = 0; n < hyb->frame_len; n++) {
            QMF_RE(X[n][band]) = 0;
            QMF_IM(X[n][band]) = 0;
            for(k = 0; k < resolution[band]; k++) {
                QMF_RE(X[n][band]) += QMF_RE(X_hybrid[n][offset + k]);
                QMF_IM(X[n][band]) += QMF_IM(X_hybrid[n][offset + k]);
            }
        }
        offset += resolution[band];
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* limits the value i to the range [min,max] */
static int8_t delta_clip(int8_t i, int8_t min, int8_t max) {
    if(i < min) return min;
    else if(i > max)
        return max;
    else
        return i;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* delta decode array */
static void delta_decode(uint8_t enable, int8_t* index, int8_t* index_prev, uint8_t dt_flag, uint8_t nr_par, uint8_t stride, int8_t min_index, int8_t max_index) {
    int8_t i;

    if(enable == 1) {
        if(dt_flag == 0) {
            /* delta coded in frequency direction */
            index[0] = 0 + index[0];
            index[0] = delta_clip(index[0], min_index, max_index);

            for(i = 1; i < nr_par; i++) {
                index[i] = index[i - 1] + index[i];
                index[i] = delta_clip(index[i], min_index, max_index);
            }
        }
        else {
            /* delta coded in time direction */
            for(i = 0; i < nr_par; i++) {
                // int8_t tmp2;
                // int8_t tmp = index[i];

                // printf("%d %d\n", index_prev[i*stride], index[i]);
                // printf("%d\n", index[i]);

                index[i] = index_prev[i * stride] + index[i];
                // tmp2 = index[i];
                index[i] = delta_clip(index[i], min_index, max_index);

                // if (iid)
                //{
                //     if (index[i] == 7)
                //     {
                //         printf("%d %d %d\n", index_prev[i*stride], tmp, tmp2);
                //     }
                // }
            }
        }
    }
    else {
        /* set indices to zero */
        for(i = 0; i < nr_par; i++) { index[i] = 0; }
    }

    /* coarse */
    if(stride == 2) {
        for(i = (nr_par << 1) - 1; i > 0; i--) { index[i] = index[i >> 1]; }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* delta modulo decode array */
/* in: log2 value of the modulo value to allow using AND instead of MOD */
static void delta_modulo_decode(uint8_t enable, int8_t* index, int8_t* index_prev, uint8_t dt_flag, uint8_t nr_par, uint8_t stride, int8_t and_modulo) {
    int8_t i;

    if(enable == 1) {
        if(dt_flag == 0) {
            /* delta coded in frequency direction */
            index[0] = 0 + index[0];
            index[0] &= and_modulo;

            for(i = 1; i < nr_par; i++) {
                index[i] = index[i - 1] + index[i];
                index[i] &= and_modulo;
            }
        }
        else {
            /* delta coded in time direction */
            for(i = 0; i < nr_par; i++) {
                index[i] = index_prev[i * stride] + index[i];
                index[i] &= and_modulo;
            }
        }
    }
    else {
        /* set indices to zero */
        for(i = 0; i < nr_par; i++) { index[i] = 0; }
    }

    /* coarse */
    if(stride == 2) {
        index[0] = 0;
        for(i = (nr_par << 1) - 1; i > 0; i--) { index[i] = index[i >> 1]; }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void map20indexto34(int8_t* index, uint8_t bins) {
    index[0] = index[0];
    index[1] = (index[0] + index[1]) / 2;
    index[2] = index[1];
    index[3] = index[2];
    index[4] = (index[2] + index[3]) / 2;
    index[5] = index[3];
    index[6] = index[4];
    index[7] = index[4];
    index[8] = index[5];
    index[9] = index[5];
    index[10] = index[6];
    index[11] = index[7];
    index[12] = index[8];
    index[13] = index[8];
    index[14] = index[9];
    index[15] = index[9];
    index[16] = index[10];

    if(bins == 34) {
        index[17] = index[11];
        index[18] = index[12];
        index[19] = index[13];
        index[20] = index[14];
        index[21] = index[14];
        index[22] = index[15];
        index[23] = index[15];
        index[24] = index[16];
        index[25] = index[16];
        index[26] = index[17];
        index[27] = index[17];
        index[28] = index[18];
        index[29] = index[18];
        index[30] = index[18];
        index[31] = index[18];
        index[32] = index[19];
        index[33] = index[19];
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* parse the bitstream data decoded in ps_data() */
static void ps_data_decode(ps_info* ps) {
    uint8_t env, bin;

    /* ps data not available, use data from previous frame */
    if(ps->ps_data_available == 0) { ps->num_env = 0; }

    for(env = 0; env < ps->num_env; env++) {
        int8_t* iid_index_prev;
        int8_t* icc_index_prev;
        int8_t* ipd_index_prev;
        int8_t* opd_index_prev;

        int8_t num_iid_steps = (ps->iid_mode < 3) ? 7 : 15 /*fine quant*/;

        if(env == 0) {
            /* take last envelope from previous frame */
            iid_index_prev = ps->iid_index_prev;
            icc_index_prev = ps->icc_index_prev;
            ipd_index_prev = ps->ipd_index_prev;
            opd_index_prev = ps->opd_index_prev;
        }
        else {
            /* take index values from previous envelope */
            iid_index_prev = ps->iid_index[env - 1];
            icc_index_prev = ps->icc_index[env - 1];
            ipd_index_prev = ps->ipd_index[env - 1];
            opd_index_prev = ps->opd_index[env - 1];
        }

        //        iid = 1;
        /* delta decode iid parameters */
        delta_decode(ps->enable_iid, ps->iid_index[env], iid_index_prev, ps->iid_dt[env], ps->nr_iid_par, (ps->iid_mode == 0 || ps->iid_mode == 3) ? 2 : 1, -num_iid_steps, num_iid_steps);
        //        iid = 0;
        /* delta decode icc parameters */
        delta_decode(ps->enable_icc, ps->icc_index[env], icc_index_prev, ps->icc_dt[env], ps->nr_icc_par, (ps->icc_mode == 0 || ps->icc_mode == 3) ? 2 : 1, 0, 7);
        /* delta modulo decode ipd parameters */
        delta_modulo_decode(ps->enable_ipdopd, ps->ipd_index[env], ipd_index_prev, ps->ipd_dt[env], ps->nr_ipdopd_par, 1, 7);
        /* delta modulo decode opd parameters */
        delta_modulo_decode(ps->enable_ipdopd, ps->opd_index[env], opd_index_prev, ps->opd_dt[env], ps->nr_ipdopd_par, 1, 7);
    }
    /* handle error case */
    if(ps->num_env == 0) {
        /* force to 1 */
        ps->num_env = 1;
        if(ps->enable_iid) {
            for(bin = 0; bin < 34; bin++) ps->iid_index[0][bin] = ps->iid_index_prev[bin];
        }
        else {
            for(bin = 0; bin < 34; bin++) ps->iid_index[0][bin] = 0;
        }
        if(ps->enable_icc) {
            for(bin = 0; bin < 34; bin++) ps->icc_index[0][bin] = ps->icc_index_prev[bin];
        }
        else {
            for(bin = 0; bin < 34; bin++) ps->icc_index[0][bin] = 0;
        }
        if(ps->enable_ipdopd) {
            for(bin = 0; bin < 17; bin++) {
                ps->ipd_index[0][bin] = ps->ipd_index_prev[bin];
                ps->opd_index[0][bin] = ps->opd_index_prev[bin];
            }
        }
        else {
            for(bin = 0; bin < 17; bin++) {
                ps->ipd_index[0][bin] = 0;
                ps->opd_index[0][bin] = 0;
            }
        }
    }
    /* update previous indices */
    for(bin = 0; bin < 34; bin++) ps->iid_index_prev[bin] = ps->iid_index[ps->num_env - 1][bin];
    for(bin = 0; bin < 34; bin++) ps->icc_index_prev[bin] = ps->icc_index[ps->num_env - 1][bin];
    for(bin = 0; bin < 17; bin++) {
        ps->ipd_index_prev[bin] = ps->ipd_index[ps->num_env - 1][bin];
        ps->opd_index_prev[bin] = ps->opd_index[ps->num_env - 1][bin];
    }
    ps->ps_data_available = 0;
    if(ps->frame_class == 0) {
        ps->border_position[0] = 0;
        for(env = 1; env < ps->num_env; env++) { ps->border_position[env] = (env * ps->numTimeSlotsRate) / ps->num_env; }
        ps->border_position[ps->num_env] = ps->numTimeSlotsRate;
    }
    else {
        ps->border_position[0] = 0;
        if(ps->border_position[ps->num_env] < ps->numTimeSlotsRate) {
            for(bin = 0; bin < 34; bin++) {
                ps->iid_index[ps->num_env][bin] = ps->iid_index[ps->num_env - 1][bin];
                ps->icc_index[ps->num_env][bin] = ps->icc_index[ps->num_env - 1][bin];
            }
            for(bin = 0; bin < 17; bin++) {
                ps->ipd_index[ps->num_env][bin] = ps->ipd_index[ps->num_env - 1][bin];
                ps->opd_index[ps->num_env][bin] = ps->opd_index[ps->num_env - 1][bin];
            }
            ps->num_env++;
            ps->border_position[ps->num_env] = ps->numTimeSlotsRate;
        }
        for(env = 1; env < ps->num_env; env++) {
            int8_t thr = ps->numTimeSlotsRate - (ps->num_env - env);

            if(ps->border_position[env] > thr) { ps->border_position[env] = thr; }
            else {
                thr = ps->border_position[env - 1] + 1;
                if(ps->border_position[env] < thr) { ps->border_position[env] = thr; }
            }
        }
    }

    /* make sure that the indices of all parameters can be mapped
     * to the same hybrid synthesis filterbank
     */
    if(ps->use34hybrid_bands) {
        for(env = 0; env < ps->num_env; env++) {
            if(ps->iid_mode != 2 && ps->iid_mode != 5) map20indexto34(ps->iid_index[env], 34);
            if(ps->icc_mode != 2 && ps->icc_mode != 5) map20indexto34(ps->icc_index[env], 34);
            if(ps->ipd_mode != 2 && ps->ipd_mode != 5) {
                map20indexto34(ps->ipd_index[env], 17);
                map20indexto34(ps->opd_index[env], 17);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* decorrelate the mono signal using an allpass filter */
static void ps_decorrelate(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64], complex_t X_hybrid_left[32][32], complex_t X_hybrid_right[32][32]) {
    uint8_t          gr, n, m, bk;
    uint8_t          temp_delay;
    uint8_t          sb, maxsb;
    const complex_t* Phi_Fract_SubQmf;
    uint8_t          temp_delay_ser[NO_ALLPASS_LINKS];
    int32_t          P_SmoothPeakDecayDiffNrg, nrg;
    int32_t          P[32][34];
    int32_t          G_TransientRatio[32][34] = {{0}};
    complex_t        inputLeft;

    /* chose hybrid filterbank: 20 or 34 band case */
    if(ps->use34hybrid_bands) { Phi_Fract_SubQmf = Phi_Fract_SubQmf34; }
    else { Phi_Fract_SubQmf = Phi_Fract_SubQmf20; }
    /* clear the energy values */
    for(n = 0; n < 32; n++) {
        for(bk = 0; bk < 34; bk++) { P[n][bk] = 0; }
    }
    /* calculate the energy in each parameter band b(k) */
    for(gr = 0; gr < ps->num_groups; gr++) {
        /* select the parameter index b(k) to which this group belongs */
        bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];
        /* select the upper subband border for this group */
        maxsb = (gr < ps->num_hybrid_groups) ? ps->group_border[gr] + 1 : ps->group_border[gr + 1];
        for(sb = ps->group_border[gr]; sb < maxsb; sb++) {
            for(n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n++) {

                uint32_t in_re, in_im;

                /* input from hybrid subbands or QMF subbands */
                if(gr < ps->num_hybrid_groups) {
                    RE(inputLeft) = QMF_RE(X_hybrid_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_hybrid_left[n][sb]);
                }
                else {
                    RE(inputLeft) = QMF_RE(X_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_left[n][sb]);
                }
                /* accumulate energy */

                /* NOTE: all input is scaled by 2^(-5) because of fixed point QMF
                 * meaning that P will be scaled by 2^(-10) compared to floating point version
                 */
                in_re = ((abs(RE(inputLeft)) + (1 << (REAL_BITS - 1))) >> REAL_BITS);
                in_im = ((abs(IM(inputLeft)) + (1 << (REAL_BITS - 1))) >> REAL_BITS);
                P[n][bk] += in_re * in_re + in_im * in_im;

            }
        }
    }
    /* calculate transient reduction ratio for each parameter band b(k) */
    for(bk = 0; bk < ps->nr_par_bands; bk++) {
        for(n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n++) {
            const int32_t gamma = COEF_CONST(1.5);
            ps->P_PeakDecayNrg[bk] = MUL_F(ps->P_PeakDecayNrg[bk], ps->alpha_decay);
            if(ps->P_PeakDecayNrg[bk] < P[n][bk]) ps->P_PeakDecayNrg[bk] = P[n][bk];
            /* apply smoothing filter to peak decay energy */
            P_SmoothPeakDecayDiffNrg = ps->P_SmoothPeakDecayDiffNrg_prev[bk];
            P_SmoothPeakDecayDiffNrg += MUL_F((ps->P_PeakDecayNrg[bk] - P[n][bk] - ps->P_SmoothPeakDecayDiffNrg_prev[bk]), ps->alpha_smooth);
            ps->P_SmoothPeakDecayDiffNrg_prev[bk] = P_SmoothPeakDecayDiffNrg;
            /* apply smoothing filter to energy */
            nrg = ps->P_prev[bk];
            nrg += MUL_F((P[n][bk] - ps->P_prev[bk]), ps->alpha_smooth);
            ps->P_prev[bk] = nrg;
            /* calculate transient ratio */
            if(MUL_C(P_SmoothPeakDecayDiffNrg, gamma) <= nrg) { G_TransientRatio[n][bk] = REAL_CONST(1.0); }
            else { G_TransientRatio[n][bk] = DIV_R(nrg, (MUL_C(P_SmoothPeakDecayDiffNrg, gamma))); }
        }
    }
    /* apply stereo decorrelation filter to the signal */
    for(gr = 0; gr < ps->num_groups; gr++) {
        if(gr < ps->num_hybrid_groups) maxsb = ps->group_border[gr] + 1;
        else
            maxsb = ps->group_border[gr + 1];
        /* QMF channel */
        for(sb = ps->group_border[gr]; sb < maxsb; sb++) {
            int32_t g_DecaySlope;
            int32_t g_DecaySlope_filt[NO_ALLPASS_LINKS];
            /* g_DecaySlope: [0..1] */
            if(gr < ps->num_hybrid_groups || sb <= ps->decay_cutoff) { g_DecaySlope = FRAC_CONST(1.0); }
            else {
                int8_t decay = ps->decay_cutoff - sb;
                if(decay <= -20 /* -1/DECAY_SLOPE */) { g_DecaySlope = 0; }
                else {
                    /* decay(int)*decay_slope(frac) = g_DecaySlope(frac) */
                    g_DecaySlope = FRAC_CONST(1.0) + DECAY_SLOPE * decay;
                }
            }
            /* calculate g_DecaySlope_filt for every m multiplied by filter_a[m] */
            for(m = 0; m < NO_ALLPASS_LINKS; m++) { g_DecaySlope_filt[m] = MUL_F(g_DecaySlope, filter_a[m]); }
            /* set delay indices */
            temp_delay = ps->saved_delay;
            for(n = 0; n < NO_ALLPASS_LINKS; n++) temp_delay_ser[n] = ps->delay_buf_index_ser[n];
            for(n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n++) {
                complex_t tmp, tmp0, R0;
                if(gr < ps->num_hybrid_groups) {
                    /* hybrid filterbank input */
                    RE(inputLeft) = QMF_RE(X_hybrid_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_hybrid_left[n][sb]);
                }
                else {
                    /* QMF filterbank input */
                    RE(inputLeft) = QMF_RE(X_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_left[n][sb]);
                }
                if(sb > ps->nr_allpass_bands && gr >= ps->num_hybrid_groups) {
                    /* delay */
                    /* never hybrid subbands here, always QMF subbands */
                    RE(tmp) = RE(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]);
                    IM(tmp) = IM(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]);
                    RE(R0) = RE(tmp);
                    IM(R0) = IM(tmp);
                    RE(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]) = RE(inputLeft);
                    IM(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]) = IM(inputLeft);
                }
                else {
                    /* allpass filter */
                    uint8_t   m;
                    complex_t Phi_Fract;
                    /* fetch parameters */
                    if(gr < ps->num_hybrid_groups) {
                        /* select data from the hybrid subbands */
                        RE(tmp0) = RE(ps->delay_SubQmf[temp_delay][sb]);
                        IM(tmp0) = IM(ps->delay_SubQmf[temp_delay][sb]);
                        RE(ps->delay_SubQmf[temp_delay][sb]) = RE(inputLeft);
                        IM(ps->delay_SubQmf[temp_delay][sb]) = IM(inputLeft);
                        RE(Phi_Fract) = RE(Phi_Fract_SubQmf[sb]);
                        IM(Phi_Fract) = IM(Phi_Fract_SubQmf[sb]);
                    }
                    else {
                        /* select data from the QMF subbands */
                        RE(tmp0) = RE(ps->delay_Qmf[temp_delay][sb]);
                        IM(tmp0) = IM(ps->delay_Qmf[temp_delay][sb]);
                        RE(ps->delay_Qmf[temp_delay][sb]) = RE(inputLeft);
                        IM(ps->delay_Qmf[temp_delay][sb]) = IM(inputLeft);
                        RE(Phi_Fract) = RE(Phi_Fract_Qmf[sb]);
                        IM(Phi_Fract) = IM(Phi_Fract_Qmf[sb]);
                    }
                    /* z^(-2) * Phi_Fract[k] */
                    ComplexMult(&RE(tmp), &IM(tmp), RE(tmp0), IM(tmp0), RE(Phi_Fract), IM(Phi_Fract));
                    RE(R0) = RE(tmp);
                    IM(R0) = IM(tmp);
                    for(m = 0; m < NO_ALLPASS_LINKS; m++) {
                        complex_t Q_Fract_allpass, tmp2;
                        /* fetch parameters */
                        if(gr < ps->num_hybrid_groups) {
                            /* select data from the hybrid subbands */
                            RE(tmp0) = RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]);
                            IM(tmp0) = IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]);
                            if(ps->use34hybrid_bands) {
                                RE(Q_Fract_allpass) = RE(Q_Fract_allpass_SubQmf34[sb][m]);
                                IM(Q_Fract_allpass) = IM(Q_Fract_allpass_SubQmf34[sb][m]);
                            }
                            else {
                                RE(Q_Fract_allpass) = RE(Q_Fract_allpass_SubQmf20[sb][m]);
                                IM(Q_Fract_allpass) = IM(Q_Fract_allpass_SubQmf20[sb][m]);
                            }
                        }
                        else {
                            /* select data from the QMF subbands */
                            RE(tmp0) = RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]);
                            IM(tmp0) = IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]);
                            RE(Q_Fract_allpass) = RE(Q_Fract_allpass_Qmf[sb][m]);
                            IM(Q_Fract_allpass) = IM(Q_Fract_allpass_Qmf[sb][m]);
                        }
                        /* delay by a fraction */
                        /* z^(-d(m)) * Q_Fract_allpass[k,m] */
                        ComplexMult(&RE(tmp), &IM(tmp), RE(tmp0), IM(tmp0), RE(Q_Fract_allpass), IM(Q_Fract_allpass));
                        /* -a(m) * g_DecaySlope[k] */
                        RE(tmp) += -MUL_F(g_DecaySlope_filt[m], RE(R0));
                        IM(tmp) += -MUL_F(g_DecaySlope_filt[m], IM(R0));
                        /* -a(m) * g_DecaySlope[k] * Q_Fract_allpass[k,m] * z^(-d(m)) */
                        RE(tmp2) = RE(R0) + MUL_F(g_DecaySlope_filt[m], RE(tmp));
                        IM(tmp2) = IM(R0) + MUL_F(g_DecaySlope_filt[m], IM(tmp));
                        /* store sample */
                        if(gr < ps->num_hybrid_groups) {
                            RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = RE(tmp2);
                            IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = IM(tmp2);
                        }
                        else {
                            RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = RE(tmp2);
                            IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = IM(tmp2);
                        }
                        /* store for next iteration (or as output value if last iteration) */
                        RE(R0) = RE(tmp);
                        IM(R0) = IM(tmp);
                    }
                }
                /* select b(k) for reading the transient ratio */
                bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];
                /* duck if a past transient is found */
                RE(R0) = MUL_R(G_TransientRatio[n][bk], RE(R0));
                IM(R0) = MUL_R(G_TransientRatio[n][bk], IM(R0));
                if(gr < ps->num_hybrid_groups) {
                    /* hybrid */
                    QMF_RE(X_hybrid_right[n][sb]) = RE(R0);
                    QMF_IM(X_hybrid_right[n][sb]) = IM(R0);
                }
                else {
                    /* QMF */
                    QMF_RE(X_right[n][sb]) = RE(R0);
                    QMF_IM(X_right[n][sb]) = IM(R0);
                }
                /* Update delay buffer index */
                if(++temp_delay >= 2) { temp_delay = 0; }
                /* update delay indices */
                if(sb > ps->nr_allpass_bands && gr >= ps->num_hybrid_groups) {
                    /* delay_D depends on the samplerate, it can hold the values 14 and 1 */
                    if(++ps->delay_buf_index_delay[sb] >= ps->delay_D[sb]) { ps->delay_buf_index_delay[sb] = 0; }
                }
                for(m = 0; m < NO_ALLPASS_LINKS; m++) {
                    if(++temp_delay_ser[m] >= ps->num_sample_delay_ser[m]) { temp_delay_ser[m] = 0; }
                }
            }
        }
    }
    /* update delay indices */
    ps->saved_delay = temp_delay;
    for(m = 0; m < NO_ALLPASS_LINKS; m++) ps->delay_buf_index_ser[m] = temp_delay_ser[m];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
        #define step(shift)                                  \
            if((0x40000000l >> shift) + root <= value) {     \
                value -= (0x40000000l >> shift) + root;      \
                root = (root >> 1) | (0x40000000l >> shift); \
            }                                                \
            else { root = root >> 1; }

/* fixed point square root approximation */
static int32_t ps_sqrt(int32_t value) {
    int32_t root = 0;
    step(0);
    step(2);
    step(4);
    step(6);
    step(8);
    step(10);
    step(12);
    step(14);
    step(16);
    step(18);
    step(20);
    step(22);
    step(24);
    step(26);
    step(28);
    step(30);

    if(root < value) ++root;

    root <<= (REAL_BITS / 2);

    return root;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------
static const int32_t ipdopd_cos_tab[] = {FRAC_CONST(1.000000000000000),  FRAC_CONST(0.707106781186548),  FRAC_CONST(0.000000000000000), FRAC_CONST(-0.707106781186547), FRAC_CONST(-1.000000000000000),
                                         FRAC_CONST(-0.707106781186548), FRAC_CONST(-0.000000000000000), FRAC_CONST(0.707106781186547), FRAC_CONST(1.000000000000000)};
static const int32_t ipdopd_sin_tab[] = {FRAC_CONST(0.000000000000000),  FRAC_CONST(0.707106781186547),  FRAC_CONST(1.000000000000000),  FRAC_CONST(0.707106781186548), FRAC_CONST(0.000000000000000),
                                         FRAC_CONST(-0.707106781186547), FRAC_CONST(-1.000000000000000), FRAC_CONST(-0.707106781186548), FRAC_CONST(-0.000000000000000)};

//----------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t magnitude_c(complex_t c) {

        #define ps_abs(A) (((A) > 0) ? (A) : (-(A)))
        #define ALPHA     FRAC_CONST(0.948059448969)
        #define BETA      FRAC_CONST(0.392699081699)

    int32_t abs_inphase = ps_abs(RE(c));
    int32_t abs_quadrature = ps_abs(IM(c));
    if(abs_inphase > abs_quadrature) { return MUL_F(abs_inphase, ALPHA) + MUL_F(abs_quadrature, BETA); }
    else { return MUL_F(abs_quadrature, ALPHA) + MUL_F(abs_inphase, BETA); }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static void ps_mix_phase(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64], complex_t X_hybrid_left[32][32], complex_t X_hybrid_right[32][32]) {
    uint8_t        n;
    uint8_t        gr;
    uint8_t        bk = 0;
    uint8_t        sb, maxsb;
    uint8_t        env;
    uint8_t        nr_ipdopd_par;
    complex_t      h11, h12, h21, h22;
    complex_t      H11, H12, H21, H22;
    complex_t      deltaH11, deltaH12, deltaH21, deltaH22;
    complex_t      tempLeft;
    complex_t      tempRight;
    complex_t      phaseLeft;
    complex_t      phaseRight;
    int32_t        L;
    const int32_t* sf_iid;
    uint8_t        no_iid_steps;

    if(ps->iid_mode >= 3) {
        no_iid_steps = 15;
        sf_iid = sf_iid_fine;
    }
    else {
        no_iid_steps = 7;
        sf_iid = sf_iid_normal;
    }

    if(ps->ipd_mode == 0 || ps->ipd_mode == 3) { nr_ipdopd_par = 11; /* resolution */ }
    else { nr_ipdopd_par = ps->nr_ipdopd_par; }
    for(gr = 0; gr < ps->num_groups; gr++) {
        bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];
        /* use one channel per group in the subqmf domain */
        maxsb = (gr < ps->num_hybrid_groups) ? ps->group_border[gr] + 1 : ps->group_border[gr + 1];

        for(env = 0; env < ps->num_env; env++) {
            if(ps->icc_mode < 3) {
                /* type 'A' mixing as described in 8.6.4.6.2.1 */
                int32_t c_1, c_2;
                int32_t cosa, sina;
                int32_t cosb, sinb;
                int32_t ab1, ab2;
                int32_t ab3, ab4;
                /*
                c_1 = sqrt(2.0 / (1.0 + pow(10.0, quant_iid[no_iid_steps + iid_index] / 10.0)));
                c_2 = sqrt(2.0 / (1.0 + pow(10.0, quant_iid[no_iid_steps - iid_index] / 10.0)));
                alpha = 0.5 * acos(quant_rho[icc_index]);
                beta = alpha * ( c_1 - c_2 ) / sqrt(2.0);
                */
                // printf("%d\n", ps->iid_index[env][bk]);
                /* index range is supposed to be -7...7 or -15...15 depending on iid_mode
                   (Table 8.24, ISO/IEC 14496-3:2005).
                   if it is outside these boundaries, this is most likely an error. sanitize
                   it and try to process further. */
                if(ps->iid_index[env][bk] < -no_iid_steps) {
                    fprintf(stderr, "Warning: invalid iid_index: %d < %d\n", ps->iid_index[env][bk], -no_iid_steps);
                    ps->iid_index[env][bk] = -no_iid_steps;
                }
                else if(ps->iid_index[env][bk] > no_iid_steps) {
                    fprintf(stderr, "Warning: invalid iid_index: %d > %d\n", ps->iid_index[env][bk], no_iid_steps);
                    ps->iid_index[env][bk] = no_iid_steps;
                }

                /* calculate the scalefactors c_1 and c_2 from the intensity differences */
                c_1 = sf_iid[no_iid_steps + ps->iid_index[env][bk]];
                c_2 = sf_iid[no_iid_steps - ps->iid_index[env][bk]];

                /* calculate alpha and beta using the ICC parameters */
                cosa = cos_alphas[ps->icc_index[env][bk]];
                sina = sin_alphas[ps->icc_index[env][bk]];

                if(ps->iid_mode >= 3) {
                    if(ps->iid_index[env][bk] < 0) {
                        cosb = cos_betas_fine[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = -sin_betas_fine[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    }
                    else {
                        cosb = cos_betas_fine[ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = sin_betas_fine[ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    }
                }
                else {
                    if(ps->iid_index[env][bk] < 0) {
                        cosb = cos_betas_normal[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = -sin_betas_normal[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    }
                    else {
                        cosb = cos_betas_normal[ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = sin_betas_normal[ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    }
                }
                ab1 = MUL_C(cosb, cosa);
                ab2 = MUL_C(sinb, sina);
                ab3 = MUL_C(sinb, cosa);
                ab4 = MUL_C(cosb, sina);
                /* h_xy: COEF */
                RE(h11) = MUL_C(c_2, (ab1 - ab2));
                RE(h12) = MUL_C(c_1, (ab1 + ab2));
                RE(h21) = MUL_C(c_2, (ab3 + ab4));
                RE(h22) = MUL_C(c_1, (ab3 - ab4));
            }
            else {
                /* type 'B' mixing as described in 8.6.4.6.2.2 */
                int32_t sina, cosa;
                int32_t cosg, sing;
                /*
                int32_t c, rho, mu, alpha, gamma;
                uint8_t i;

                i = ps->iid_index[env][bk];
                c = (int32_t)pow(10.0, ((i)?(((i>0)?1:-1)*quant_iid[((i>0)?i:-i)-1]):0.)/20.0);
                rho = quant_rho[ps->icc_index[env][bk]];

                if (rho == 0.0f && c == 1.)
                {
                    alpha = (int32_t)M_PI/4.0f;
                    rho = 0.05f;
                } else {
                    if (rho <= 0.05f)
                    {
                        rho = 0.05f;
                    }
                    alpha = 0.5f*(int32_t)atan( (2.0f*c*rho) / (c*c-1.0f) );

                    if (alpha < 0.)
                    {
                        alpha += (int32_t)M_PI/2.0f;
                    }
                    if (rho < 0.)
                    {
                        alpha += (int32_t)M_PI;
                    }
                }
                mu = c+1.0f/c;
                mu = 1+(4.0f*rho*rho-4.0f)/(mu*mu);
                gamma = (int32_t)atan(sqrt((1.0f-sqrt(mu))/(1.0f+sqrt(mu))));
                */

                if(ps->iid_mode >= 3) {
                    uint8_t abs_iid = abs(ps->iid_index[env][bk]);

                    cosa = sincos_alphas_B_fine[no_iid_steps + ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    sina = sincos_alphas_B_fine[30 - (no_iid_steps + ps->iid_index[env][bk])][ps->icc_index[env][bk]];
                    cosg = cos_gammas_fine[abs_iid][ps->icc_index[env][bk]];
                    sing = sin_gammas_fine[abs_iid][ps->icc_index[env][bk]];
                }
                else {
                    uint8_t abs_iid = abs(ps->iid_index[env][bk]);

                    cosa = sincos_alphas_B_normal[no_iid_steps + ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    sina = sincos_alphas_B_normal[14 - (no_iid_steps + ps->iid_index[env][bk])][ps->icc_index[env][bk]];
                    cosg = cos_gammas_normal[abs_iid][ps->icc_index[env][bk]];
                    sing = sin_gammas_normal[abs_iid][ps->icc_index[env][bk]];
                }

                RE(h11) = MUL_C(COEF_SQRT2, MUL_C(cosa, cosg));
                RE(h12) = MUL_C(COEF_SQRT2, MUL_C(sina, cosg));
                RE(h21) = MUL_C(COEF_SQRT2, MUL_C(-cosa, sing));
                RE(h22) = MUL_C(COEF_SQRT2, MUL_C(sina, sing));
            }

            /* calculate phase rotation parameters H_xy */
            /* note that the imaginary part of these parameters are only calculated when
               IPD and OPD are enabled
             */
            if((ps->enable_ipdopd) && (bk < nr_ipdopd_par)) {
                int8_t  i;
                int32_t xy, pq, xypq;

                /* ringbuffer index */
                i = ps->phase_hist;

                /* previous value */
                 /* divide by 4, shift right 2 bits */
                RE(tempLeft) = RE(ps->ipd_prev[bk][i]) >> 2;
                IM(tempLeft) = IM(ps->ipd_prev[bk][i]) >> 2;
                RE(tempRight) = RE(ps->opd_prev[bk][i]) >> 2;
                IM(tempRight) = IM(ps->opd_prev[bk][i]) >> 2;


                /* save current value */
                RE(ps->ipd_prev[bk][i]) = ipdopd_cos_tab[abs(ps->ipd_index[env][bk])];
                IM(ps->ipd_prev[bk][i]) = ipdopd_sin_tab[abs(ps->ipd_index[env][bk])];
                RE(ps->opd_prev[bk][i]) = ipdopd_cos_tab[abs(ps->opd_index[env][bk])];
                IM(ps->opd_prev[bk][i]) = ipdopd_sin_tab[abs(ps->opd_index[env][bk])];

                /* add current value */
                RE(tempLeft) += RE(ps->ipd_prev[bk][i]);
                IM(tempLeft) += IM(ps->ipd_prev[bk][i]);
                RE(tempRight) += RE(ps->opd_prev[bk][i]);
                IM(tempRight) += IM(ps->opd_prev[bk][i]);

                /* ringbuffer index */
                if(i == 0) { i = 2; }
                i--;

                /* get value before previous */
                 /* dividing by 2, shift right 1 bit */
                RE(tempLeft) += (RE(ps->ipd_prev[bk][i]) >> 1);
                IM(tempLeft) += (IM(ps->ipd_prev[bk][i]) >> 1);
                RE(tempRight) += (RE(ps->opd_prev[bk][i]) >> 1);
                IM(tempRight) += (IM(ps->opd_prev[bk][i]) >> 1);


    #if 0 /* original code */
                ipd = (float)atan2(IM(tempLeft), RE(tempLeft));
                opd = (float)atan2(IM(tempRight), RE(tempRight));

                /* phase rotation */
                RE(phaseLeft) = (float)cos(opd);
                IM(phaseLeft) = (float)sin(opd);
                opd -= ipd;
                RE(phaseRight) = (float)cos(opd);
                IM(phaseRight) = (float)sin(opd);
    #else

                // x = IM(tempLeft)
                // y = RE(tempLeft)
                // p = IM(tempRight)
                // q = RE(tempRight)
                // cos(atan2(x,y)) = y/sqrt((x*x) + (y*y))
                // sin(atan2(x,y)) = x/sqrt((x*x) + (y*y))
                // cos(atan2(x,y)-atan2(p,q)) = (y*q + x*p) / ( sqrt((x*x) + (y*y)) * sqrt((p*p) + (q*q)) );
                // sin(atan2(x,y)-atan2(p,q)) = (x*q - y*p) / ( sqrt((x*x) + (y*y)) * sqrt((p*p) + (q*q)) );

                xy = magnitude_c(tempRight);
                pq = magnitude_c(tempLeft);

                if(xy != 0) {
                    RE(phaseLeft) = DIV_R(RE(tempRight), xy);
                    IM(phaseLeft) = DIV_R(IM(tempRight), xy);
                }
                else {
                    RE(phaseLeft) = 0;
                    IM(phaseLeft) = 0;
                }

                xypq = MUL_R(xy, pq);

                if(xypq != 0) {
                    int32_t tmp1 = MUL_R(RE(tempRight), RE(tempLeft)) + MUL_R(IM(tempRight), IM(tempLeft));
                    int32_t tmp2 = MUL_R(IM(tempRight), RE(tempLeft)) - MUL_R(RE(tempRight), IM(tempLeft));

                    RE(phaseRight) = DIV_R(tmp1, xypq);
                    IM(phaseRight) = DIV_R(tmp2, xypq);
                }
                else {
                    RE(phaseRight) = 0;
                    IM(phaseRight) = 0;
                }
    #endif
                /* MUL_F(COEF, REAL) = COEF */
                IM(h11) = MUL_R(RE(h11), IM(phaseLeft));
                IM(h12) = MUL_R(RE(h12), IM(phaseRight));
                IM(h21) = MUL_R(RE(h21), IM(phaseLeft));
                IM(h22) = MUL_R(RE(h22), IM(phaseRight));
                RE(h11) = MUL_R(RE(h11), RE(phaseLeft));
                RE(h12) = MUL_R(RE(h12), RE(phaseRight));
                RE(h21) = MUL_R(RE(h21), RE(phaseLeft));
                RE(h22) = MUL_R(RE(h22), RE(phaseRight));
            }
            /* length of the envelope n_e+1 - n_e (in time samples) */
            /* 0 < L <= 32: integer */
            L = (int32_t)(ps->border_position[env + 1] - ps->border_position[env]);

            /* obtain final H_xy by means of linear interpolation */
            RE(deltaH11) = (RE(h11) - RE(ps->h11_prev[gr])) / L;
            RE(deltaH12) = (RE(h12) - RE(ps->h12_prev[gr])) / L;
            RE(deltaH21) = (RE(h21) - RE(ps->h21_prev[gr])) / L;
            RE(deltaH22) = (RE(h22) - RE(ps->h22_prev[gr])) / L;
            RE(H11) = RE(ps->h11_prev[gr]);
            RE(H12) = RE(ps->h12_prev[gr]);
            RE(H21) = RE(ps->h21_prev[gr]);
            RE(H22) = RE(ps->h22_prev[gr]);
            RE(ps->h11_prev[gr]) = RE(h11);
            RE(ps->h12_prev[gr]) = RE(h12);
            RE(ps->h21_prev[gr]) = RE(h21);
            RE(ps->h22_prev[gr]) = RE(h22);

            /* only calculate imaginary part when needed */
            if((ps->enable_ipdopd) && (bk < nr_ipdopd_par)) {
                /* obtain final H_xy by means of linear interpolation */
                IM(deltaH11) = (IM(h11) - IM(ps->h11_prev[gr])) / L;
                IM(deltaH12) = (IM(h12) - IM(ps->h12_prev[gr])) / L;
                IM(deltaH21) = (IM(h21) - IM(ps->h21_prev[gr])) / L;
                IM(deltaH22) = (IM(h22) - IM(ps->h22_prev[gr])) / L;

                IM(H11) = IM(ps->h11_prev[gr]);
                IM(H12) = IM(ps->h12_prev[gr]);
                IM(H21) = IM(ps->h21_prev[gr]);
                IM(H22) = IM(ps->h22_prev[gr]);

                if((NEGATE_IPD_MASK & ps->map_group2bk[gr]) != 0) {
                    IM(deltaH11) = -IM(deltaH11);
                    IM(deltaH12) = -IM(deltaH12);
                    IM(deltaH21) = -IM(deltaH21);
                    IM(deltaH22) = -IM(deltaH22);

                    IM(H11) = -IM(H11);
                    IM(H12) = -IM(H12);
                    IM(H21) = -IM(H21);
                    IM(H22) = -IM(H22);
                }

                IM(ps->h11_prev[gr]) = IM(h11);
                IM(ps->h12_prev[gr]) = IM(h12);
                IM(ps->h21_prev[gr]) = IM(h21);
                IM(ps->h22_prev[gr]) = IM(h22);
            }

            /* apply H_xy to the current envelope band of the decorrelated subband */
            for(n = ps->border_position[env]; n < ps->border_position[env + 1]; n++) {
                /* addition finalises the interpolation over every n */
                RE(H11) += RE(deltaH11);
                RE(H12) += RE(deltaH12);
                RE(H21) += RE(deltaH21);
                RE(H22) += RE(deltaH22);
                if((ps->enable_ipdopd) && (bk < nr_ipdopd_par)) {
                    IM(H11) += IM(deltaH11);
                    IM(H12) += IM(deltaH12);
                    IM(H21) += IM(deltaH21);
                    IM(H22) += IM(deltaH22);
                }

                /* channel is an alias to the subband */
                for(sb = ps->group_border[gr]; sb < maxsb; sb++) {
                    complex_t inLeft, inRight;

                    /* load decorrelated samples */
                    if(gr < ps->num_hybrid_groups) {
                        RE(inLeft) = RE(X_hybrid_left[n][sb]);
                        IM(inLeft) = IM(X_hybrid_left[n][sb]);
                        RE(inRight) = RE(X_hybrid_right[n][sb]);
                        IM(inRight) = IM(X_hybrid_right[n][sb]);
                    }
                    else {
                        RE(inLeft) = RE(X_left[n][sb]);
                        IM(inLeft) = IM(X_left[n][sb]);
                        RE(inRight) = RE(X_right[n][sb]);
                        IM(inRight) = IM(X_right[n][sb]);
                    }

                    /* apply mixing */
                    RE(tempLeft) = MUL_C(RE(H11), RE(inLeft)) + MUL_C(RE(H21), RE(inRight));
                    IM(tempLeft) = MUL_C(RE(H11), IM(inLeft)) + MUL_C(RE(H21), IM(inRight));
                    RE(tempRight) = MUL_C(RE(H12), RE(inLeft)) + MUL_C(RE(H22), RE(inRight));
                    IM(tempRight) = MUL_C(RE(H12), IM(inLeft)) + MUL_C(RE(H22), IM(inRight));

                    /* only perform imaginary operations when needed */
                    if((ps->enable_ipdopd) && (bk < nr_ipdopd_par)) {
                        /* apply rotation */
                        RE(tempLeft) -= MUL_C(IM(H11), IM(inLeft)) + MUL_C(IM(H21), IM(inRight));
                        IM(tempLeft) += MUL_C(IM(H11), RE(inLeft)) + MUL_C(IM(H21), RE(inRight));
                        RE(tempRight) -= MUL_C(IM(H12), IM(inLeft)) + MUL_C(IM(H22), IM(inRight));
                        IM(tempRight) += MUL_C(IM(H12), RE(inLeft)) + MUL_C(IM(H22), RE(inRight));
                    }

                    /* store final samples */
                    if(gr < ps->num_hybrid_groups) {
                        RE(X_hybrid_left[n][sb]) = RE(tempLeft);
                        IM(X_hybrid_left[n][sb]) = IM(tempLeft);
                        RE(X_hybrid_right[n][sb]) = RE(tempRight);
                        IM(X_hybrid_right[n][sb]) = IM(tempRight);
                    }
                    else {
                        RE(X_left[n][sb]) = RE(tempLeft);
                        IM(X_left[n][sb]) = IM(tempLeft);
                        RE(X_right[n][sb]) = RE(tempRight);
                        IM(X_right[n][sb]) = IM(tempRight);
                    }
                }
            }

            /* shift phase smoother's circular buffer index */
            ps->phase_hist++;
            if(ps->phase_hist == 2) { ps->phase_hist = 0; }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void ps_free(ps_info* ps) {
    /* free hybrid filterbank structures */
    hybrid_free((hyb_info*)ps->hyb);

    faad_free(ps);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
ps_info* ps_init(uint8_t sr_index, uint8_t numTimeSlotsRate) {
    uint8_t i;
    uint8_t short_delay_band;

    ps_info* ps = (ps_info*)faad_malloc(sizeof(ps_info));
    memset(ps, 0, sizeof(ps_info));

    ps->hyb = hybrid_init(numTimeSlotsRate);
    ps->numTimeSlotsRate = numTimeSlotsRate;

    ps->ps_data_available = 0;

    /* delay stuff*/
    ps->saved_delay = 0;

    for(i = 0; i < 64; i++) { ps->delay_buf_index_delay[i] = 0; }

    for(i = 0; i < NO_ALLPASS_LINKS; i++) {
        ps->delay_buf_index_ser[i] = 0;
        /* THESE ARE CONSTANTS NOW */
        ps->num_sample_delay_ser[i] = delay_length_d[i];
    }

    /* THESE ARE CONSTANTS NOW */
    short_delay_band = 35;
    ps->nr_allpass_bands = 22;
    ps->alpha_decay = FRAC_CONST(0.76592833836465);
    ps->alpha_smooth = FRAC_CONST(0.25);

    /* THESE ARE CONSTANT NOW IF PS IS INDEPENDANT OF SAMPLERATE */
    for(i = 0; i < short_delay_band; i++) { ps->delay_D[i] = 14; }
    for(i = short_delay_band; i < 64; i++) { ps->delay_D[i] = 1; }

    /* mixing and phase */
    for(i = 0; i < 50; i++) {
        RE(ps->h11_prev[i]) = 1;
        IM(ps->h12_prev[i]) = 1;
        RE(ps->h11_prev[i]) = 1;
        IM(ps->h12_prev[i]) = 1;
    }

    ps->phase_hist = 0;

    for(i = 0; i < 20; i++) {
        RE(ps->ipd_prev[i][0]) = 0;
        IM(ps->ipd_prev[i][0]) = 0;
        RE(ps->ipd_prev[i][1]) = 0;
        IM(ps->ipd_prev[i][1]) = 0;
        RE(ps->opd_prev[i][0]) = 0;
        IM(ps->opd_prev[i][0]) = 0;
        RE(ps->opd_prev[i][1]) = 0;
        IM(ps->opd_prev[i][1]) = 0;
    }

    return ps;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* main Parametric Stereo decoding function */
uint8_t ps_decode(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64]) {
    complex_t X_hybrid_left[32][32] = {{0}};
    complex_t X_hybrid_right[32][32] = {{0}};

    /* delta decoding of the bitstream data */
    ps_data_decode(ps);

    /* set up some parameters depending on filterbank type */
    if(ps->use34hybrid_bands) {
        ps->group_border = (uint8_t*)group_border34;
        ps->map_group2bk = (uint16_t*)map_group2bk34;
        ps->num_groups = 32 + 18;
        ps->num_hybrid_groups = 32;
        ps->nr_par_bands = 34;
        ps->decay_cutoff = 5;
    }
    else {
        ps->group_border = (uint8_t*)group_border20;
        ps->map_group2bk = (uint16_t*)map_group2bk20;
        ps->num_groups = 10 + 12;
        ps->num_hybrid_groups = 10;
        ps->nr_par_bands = 20;
        ps->decay_cutoff = 3;
    }

    /* Perform further analysis on the lowest subbands to get a higher
     * frequency resolution
     */
    hybrid_analysis((hyb_info*)ps->hyb, X_left, X_hybrid_left, ps->use34hybrid_bands, ps->numTimeSlotsRate);

    /* decorrelate mono signal */
    ps_decorrelate(ps, X_left, X_right, X_hybrid_left, X_hybrid_right);

    /* apply mixing and phase parameters */
    ps_mix_phase(ps, X_left, X_right, X_hybrid_left, X_hybrid_right);

    /* hybrid synthesis, to rebuild the SBR QMF matrices */
    hybrid_synthesis((hyb_info*)ps->hyb, X_left, X_hybrid_left, ps->use34hybrid_bands, ps->numTimeSlotsRate);

    hybrid_synthesis((hyb_info*)ps->hyb, X_right, X_hybrid_right, ps->use34hybrid_bands, ps->numTimeSlotsRate);

    return 0;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t is_ltp_ot(uint8_t object_type) { /* check if the object type is an object type that can have LTP */
    #ifdef LTP_DEC
    if((object_type == LTP)
        #ifdef ERROR_RESILIENCE
       || (object_type == ER_LTP)
        #endif
        #ifdef LD_DEC
       || (object_type == LD)
        #endif
    ) {
        return 1;
    }
    #endif
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void lt_prediction(ic_stream* ics, ltp_info* ltp, int32_t* spec, int16_t* lt_pred_stat, fb_info* fb, uint8_t win_shape, uint8_t win_shape_prev,
                   uint8_t sr_index, uint8_t object_type, uint16_t frame_len) {
    uint8_t  sfb;
    uint16_t bin, i, num_samples;
    int32_t  x_est[2048];
    int32_t  X_est[2048];

    if(ics->window_sequence != EIGHT_SHORT_SEQUENCE) {
        if(ltp->data_present) {
            num_samples = frame_len << 1;
            for(i = 0; i < num_samples; i++) {
                /* The extra lookback M (N/2 for LD, 0 for LTP) is handled in the buffer updating */
                /* lt_pred_stat is a 16 bit int, multiplied with the fixed point real this gives a real for x_est */
                x_est[i] = (int32_t)lt_pred_stat[num_samples + i - ltp->lag] * codebook[ltp->coef];
            }
            filter_bank_ltp(fb, ics->window_sequence, win_shape, win_shape_prev, x_est, X_est, object_type, frame_len);
            tns_encode_frame(ics, &(ics->tns), sr_index, object_type, X_est, frame_len);
            for(sfb = 0; sfb < ltp->last_band; sfb++) {
                if(ltp->long_used[sfb]) {
                    uint16_t low = ics->swb_offset[sfb];
                    uint16_t high = min(ics->swb_offset[sfb + 1], ics->swb_offset_max);
                    for(bin = low; bin < high; bin++) { spec[bin] += X_est[bin]; }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int16_t real_to_int16(int32_t sig_in) {
    if(sig_in >= 0) {
        sig_in += (1 << (REAL_BITS - 1));
        if(sig_in >= REAL_CONST(32768)) return 32767;
    }
    else {
        sig_in += -(1 << (REAL_BITS - 1));
        if(sig_in <= REAL_CONST(-32768)) return -32768;
    }
    return (sig_in >> REAL_BITS);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void lt_update_state(int16_t* lt_pred_stat, int32_t* time, int32_t* overlap, uint16_t frame_len, uint8_t object_type) {
    uint16_t i;

    /*
     * The reference point for index i and the content of the buffer lt_pred_stat are arranged so that lt_pred_stat(0 ... N/2 - 1) contains the
     * last aliased half window from the IMDCT, and lt_pred_stat(N/2 ... N-1) is always all zeros. The rest of lt_pred_stat (i<0) contains the
     * previous fully reconstructed time domain samples, i.e., output of the decoder.
     * These values are shifted up by N*2 to avoid (i<0)
     * For the LD object type an extra 512 samples lookback is accomodated here.
     */
    #ifdef LD_DEC
    if(object_type == LD) {
        for(i = 0; i < frame_len; i++) {
            lt_pred_stat[i] /* extra 512 */ = lt_pred_stat[i + frame_len];
            lt_pred_stat[frame_len + i] = lt_pred_stat[i + (frame_len * 2)];
            lt_pred_stat[(frame_len * 2) + i] = real_to_int16(time[i]);
            lt_pred_stat[(frame_len * 3) + i] = real_to_int16(overlap[i]);
        }
    }
    else {
    #endif
        for(i = 0; i < frame_len; i++) {
            lt_pred_stat[i] = lt_pred_stat[i + frame_len];
            lt_pred_stat[frame_len + i] = real_to_int16(time[i]);
            lt_pred_stat[(frame_len * 2) + i] = real_to_int16(overlap[i]);
        }
    #ifdef LD_DEC
    }
    #endif
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
mdct_info* faad_mdct_init(uint16_t N) {
    mdct_info* mdct = (mdct_info*)faad_malloc(sizeof(mdct_info));
    assert(N % 8 == 0);
    mdct->N = N;
    /* NOTE: For "small framelengths" the coefficients need to be scaled by sqrt("(nearest power of 2) > N" / N) */
    /* RE(mdct->sincos[k]) = scale*(int32_t)(cos(2.0*M_PI*(k+1./8.) / (int32_t)N));
     * IM(mdct->sincos[k]) = scale*(int32_t)(sin(2.0*M_PI*(k+1./8.) / (int32_t)N)); */
    /* scale is 1 for fixed point, sqrt(N) for floating point */
    switch(N) {
    case 2048: mdct->sincos = (complex_t*)mdct_tab_2048; break;
    case 256: mdct->sincos = (complex_t*)mdct_tab_256; break;
#ifdef LD_DEC
    case 1024: mdct->sincos = (complex_t*)mdct_tab_1024; break;
#endif
#ifdef ALLOW_SMALL_FRAMELENGTH
    case 1920: mdct->sincos = (complex_t*)mdct_tab_1920; break;
    case 240: mdct->sincos = (complex_t*)mdct_tab_240; break;
    #ifdef LD_DEC
    case 960: mdct->sincos = (complex_t*)mdct_tab_960; break;
    #endif
#endif
    }

    /* initialise fft */
    mdct->cfft = cffti(N / 4);

#ifdef PROFILE
    mdct->cycles = 0;
    mdct->fft_cycles = 0;
#endif

    return mdct;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void faad_mdct_end(mdct_info* mdct) {
    if(mdct != NULL) {
#ifdef PROFILE
        printf("MDCT[%.4d]:         %I64d cycles\n", mdct->N, mdct->cycles);
        printf("CFFT[%.4d]:         %I64d cycles\n", mdct->N / 4, mdct->fft_cycles);
#endif
        cfftu(mdct->cfft);
        faad_free(mdct);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void faad_imdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out) {
    uint16_t k;
    complex_t x;
#ifdef ALLOW_SMALL_FRAMELENGTH
    int32_t scale, b_scale = 0;
#endif
    complex_t  Z1[512];
    complex_t* sincos = mdct->sincos;
    uint16_t N = mdct->N;
    uint16_t N2 = N >> 1;
    uint16_t N4 = N >> 2;
    uint16_t N8 = N >> 3;

#ifdef PROFILE
    int64_t count1, count2 = faad_get_ts();
#endif

#ifdef ALLOW_SMALL_FRAMELENGTH
    /* detect non-power of 2 */
    if(N & (N - 1)) {
        /* adjust scale for non-power of 2 MDCT */
        /* 2048/1920 */
        b_scale = 1;
        scale = COEF_CONST(1.0666666666666667);
    }
#endif
    /* pre-IFFT complex multiplication */
    for(k = 0; k < N4; k++) { ComplexMult(&IM(Z1[k]), &RE(Z1[k]), X_in[2 * k], X_in[N2 - 1 - 2 * k], RE(sincos[k]), IM(sincos[k])); }

#ifdef PROFILE
    count1 = faad_get_ts();
#endif

    /* complex IFFT, any non-scaling FFT can be used here */
    cfftb(mdct->cfft, Z1);

#ifdef PROFILE
    count1 = faad_get_ts() - count1;
#endif
    /* post-IFFT complex multiplication */
    for(k = 0; k < N4; k++) {
        RE(x) = RE(Z1[k]);
        IM(x) = IM(Z1[k]);
        ComplexMult(&IM(Z1[k]), &RE(Z1[k]), IM(x), RE(x), RE(sincos[k]), IM(sincos[k]));
#ifdef ALLOW_SMALL_FRAMELENGTH
        /* non-power of 2 MDCT scaling */
        if(b_scale) {
            RE(Z1[k]) = MUL_C(RE(Z1[k]), scale);
            IM(Z1[k]) = MUL_C(IM(Z1[k]), scale);
        }
#endif
    }
    /* reordering */
    for(k = 0; k < N8; k += 2) {
        X_out[2 * k] = IM(Z1[N8 + k]);
        X_out[2 + 2 * k] = IM(Z1[N8 + 1 + k]);
        X_out[1 + 2 * k] = -RE(Z1[N8 - 1 - k]);
        X_out[3 + 2 * k] = -RE(Z1[N8 - 2 - k]);
        X_out[N4 + 2 * k] = RE(Z1[k]);
        X_out[N4 + +2 + 2 * k] = RE(Z1[1 + k]);
        X_out[N4 + 1 + 2 * k] = -IM(Z1[N4 - 1 - k]);
        X_out[N4 + 3 + 2 * k] = -IM(Z1[N4 - 2 - k]);
        X_out[N2 + 2 * k] = RE(Z1[N8 + k]);
        X_out[N2 + +2 + 2 * k] = RE(Z1[N8 + 1 + k]);
        X_out[N2 + 1 + 2 * k] = -IM(Z1[N8 - 1 - k]);
        X_out[N2 + 3 + 2 * k] = -IM(Z1[N8 - 2 - k]);
        X_out[N2 + N4 + 2 * k] = -IM(Z1[k]);
        X_out[N2 + N4 + 2 + 2 * k] = -IM(Z1[1 + k]);
        X_out[N2 + N4 + 1 + 2 * k] = RE(Z1[N4 - 1 - k]);
        X_out[N2 + N4 + 3 + 2 * k] = RE(Z1[N4 - 2 - k]);
    }
#ifdef PROFILE
    count2 = faad_get_ts() - count2;
    mdct->fft_cycles += count1;
    mdct->cycles += (count2 - count1);
#endif
}

#ifdef LTP_DEC
//----------------------------------------------------------------------------------------------------------------------------------------------------
void faad_mdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out) {
    uint16_t k;
    complex_t  x;
    complex_t  Z1[512];
    complex_t* sincos = mdct->sincos;
    uint16_t N = mdct->N;
    uint16_t N2 = N >> 1;
    uint16_t N4 = N >> 2;
    uint16_t N8 = N >> 3;

     int32_t scale = REAL_CONST(4.0 / N);
    #ifdef ALLOW_SMALL_FRAMELENGTH
    /* detect non-power of 2 */
    if(N & (N - 1)) {
        /* adjust scale for non-power of 2 MDCT */
        /* *= sqrt(2048/1920) */
        scale = MUL_C(scale, COEF_CONST(1.0327955589886444));
    }
    #endif
    /* pre-FFT complex multiplication */
    for(k = 0; k < N8; k++) {
        uint16_t n = k << 1;
        RE(x) = X_in[N - N4 - 1 - n] + X_in[N - N4 + n];
        IM(x) = X_in[N4 + n] - X_in[N4 - 1 - n];
        ComplexMult(&RE(Z1[k]), &IM(Z1[k]), RE(x), IM(x), RE(sincos[k]), IM(sincos[k]));
        RE(Z1[k]) = MUL_R(RE(Z1[k]), scale);
        IM(Z1[k]) = MUL_R(IM(Z1[k]), scale);
        RE(x) = X_in[N2 - 1 - n] - X_in[n];
        IM(x) = X_in[N2 + n] + X_in[N - 1 - n];
        ComplexMult(&RE(Z1[k + N8]), &IM(Z1[k + N8]), RE(x), IM(x), RE(sincos[k + N8]), IM(sincos[k + N8]));
        RE(Z1[k + N8]) = MUL_R(RE(Z1[k + N8]), scale);
        IM(Z1[k + N8]) = MUL_R(IM(Z1[k + N8]), scale);
    }
    /* complex FFT, any non-scaling FFT can be used here  */
    cfftf(mdct->cfft, Z1);
    /* post-FFT complex multiplication */
    for(k = 0; k < N4; k++) {
        uint16_t n = k << 1;
        ComplexMult(&RE(x), &IM(x), RE(Z1[k]), IM(Z1[k]), RE(sincos[k]), IM(sincos[k]));
        X_out[n] = -RE(x);
        X_out[N2 - 1 - n] = IM(x);
        X_out[N2 + n] = -IM(x);
        X_out[N - 1 - n] = RE(x);
    }
}
#endif

/* defines if an object type can be decoded by this library or not */
static uint8_t ObjectTypesTable[32] = {
    0, /*  0 NULL */
    0, /*  1 AAC Main */
    1, /*  2 AAC LC */
    0, /*  3 AAC SSR */
#ifdef LTP_DEC
    1, /*  4 AAC LTP */
#else
    0, /*  4 AAC LTP */
#endif
#ifdef SBR_DEC
    1, /*  5 SBR */
#else
    0, /*  5 SBR */
#endif
    0, /*  6 AAC Scalable */
    0, /*  7 TwinVQ */
    0, /*  8 CELP */
    0, /*  9 HVXC */
    0, /* 10 Reserved */
    0, /* 11 Reserved */
    0, /* 12 TTSI */
    0, /* 13 Main synthetic */
    0, /* 14 Wavetable synthesis */
    0, /* 15 General MIDI */
    0, /* 16 Algorithmic Synthesis and Audio FX */

/* MPEG-4 Version 2 */
#ifdef ERROR_RESILIENCE
    1, /* 17 ER AAC LC */
    0, /* 18 (Reserved) */
    #ifdef LTP_DEC
    1, /* 19 ER AAC LTP */
    #else
    0, /* 19 ER AAC LTP */
    #endif
    0, /* 20 ER AAC scalable */
    0, /* 21 ER TwinVQ */
    0, /* 22 ER BSAC */
    #ifdef LD_DEC
    1, /* 23 ER AAC LD */
    #else
    0, /* 23 ER AAC LD */
    #endif
    0, /* 24 ER CELP */
    0, /* 25 ER HVXC */
    0, /* 26 ER HILN */
    0, /* 27 ER Parametric */
#else  /* No ER defined */
    0, /* 17 ER AAC LC */
    0, /* 18 (Reserved) */
    0, /* 19 ER AAC LTP */
    0, /* 20 ER AAC scalable */
    0, /* 21 ER TwinVQ */
    0, /* 22 ER BSAC */
    0, /* 23 ER AAC LD */
    0, /* 24 ER CELP */
    0, /* 25 ER HVXC */
    0, /* 26 ER HILN */
    0, /* 27 ER Parametric */
#endif
    0, /* 28 (Reserved) */
#ifdef PS_DEC
    1, /* 29 AAC LC + SBR + PS */
#else
    0, /* 29 AAC LC + SBR + PS */
#endif
    0, /* 30 (Reserved) */
    0  /* 31 (Reserved) */
};
/* Table 1.6.1 */
//----------------------------------------------------------------------------------------------------------------------------------------------------
char NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC) {
    return AudioSpecificConfig2(pBuffer, buffer_size, mp4ASC, NULL, 0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t AudioSpecificConfigFromBitfile(bitfile* ld, mp4AudioSpecificConfig* mp4ASC, program_config* pce, uint32_t buffer_size, uint8_t short_form) {
    int8_t   result = 0;
    uint32_t startpos = faad_get_processed_bits(ld);
#ifdef SBR_DEC
    int8_t bits_to_decode = 0;
#endif
    if(mp4ASC == NULL) return -8;
    memset(mp4ASC, 0, sizeof(mp4AudioSpecificConfig));
    mp4ASC->objectTypeIndex = (uint8_t)faad_getbits(ld, 5);
    mp4ASC->samplingFrequencyIndex = (uint8_t)faad_getbits(ld, 4);
    if(mp4ASC->samplingFrequencyIndex == 0x0f) faad_getbits(ld, 24);
    mp4ASC->channelsConfiguration = (uint8_t)faad_getbits(ld, 4);
    mp4ASC->samplingFrequency = get_sample_rate(mp4ASC->samplingFrequencyIndex);
    if(ObjectTypesTable[mp4ASC->objectTypeIndex] != 1) { return -1; }
    if(mp4ASC->samplingFrequency == 0) { return -2; }
    if(mp4ASC->channelsConfiguration > 7) { return -3; }
#if(defined(PS_DEC))
    /* check if we have a mono file */
    if(mp4ASC->channelsConfiguration == 1) {
        /* upMatrix to 2 channels for implicit signalling of PS */
        mp4ASC->channelsConfiguration = 2;
    }
#endif

#ifdef SBR_DEC
    mp4ASC->sbr_present_flag = -1;
    if(mp4ASC->objectTypeIndex == 5 || mp4ASC->objectTypeIndex == 29) {
        uint8_t tmp;
        mp4ASC->sbr_present_flag = 1;
        tmp = (uint8_t)faad_getbits(ld, 4);
        /* check for downsampled SBR */
        if(tmp == mp4ASC->samplingFrequencyIndex) mp4ASC->downSampledSBR = 1;
        mp4ASC->samplingFrequencyIndex = tmp;
        if(mp4ASC->samplingFrequencyIndex == 15) { mp4ASC->samplingFrequency = (uint32_t)faad_getbits(ld, 24); }
        else { mp4ASC->samplingFrequency = get_sample_rate(mp4ASC->samplingFrequencyIndex); }
        mp4ASC->objectTypeIndex = (uint8_t)faad_getbits(ld, 5);
    }
#endif
    /* get GASpecificConfig */
    if(mp4ASC->objectTypeIndex == 1 || mp4ASC->objectTypeIndex == 2 || mp4ASC->objectTypeIndex == 3 || mp4ASC->objectTypeIndex == 4 ||
       mp4ASC->objectTypeIndex == 6 || mp4ASC->objectTypeIndex == 7) {
        result = GASpecificConfig(ld, mp4ASC, pce);
#ifdef ERROR_RESILIENCE
    }
    else if(mp4ASC->objectTypeIndex >= ER_OBJECT_START) { /* ER */
        result = GASpecificConfig(ld, mp4ASC, pce);
        mp4ASC->epConfig = (uint8_t)faad_getbits(ld, 2);

        if(mp4ASC->epConfig != 0) result = -5;
#endif
    }
    else { result = -4; }
#ifdef SBR_DEC
    if(short_form) bits_to_decode = 0;
    else
        bits_to_decode = (int8_t)(buffer_size * 8 - (startpos - faad_get_processed_bits(ld)));
    if((mp4ASC->objectTypeIndex != 5 && mp4ASC->objectTypeIndex != 29) && (bits_to_decode >= 16)) {
        int16_t syncExtensionType = (int16_t)faad_getbits(ld, 11);
        if(syncExtensionType == 0x2b7) {
            uint8_t tmp_OTi = (uint8_t)faad_getbits(ld, 5);
            if(tmp_OTi == 5) {
                mp4ASC->sbr_present_flag = (uint8_t)faad_get1bit(ld);
                if(mp4ASC->sbr_present_flag) {
                    uint8_t tmp;
                    /* Don't set OT to SBR until checked that it is actually there */
                    mp4ASC->objectTypeIndex = tmp_OTi;
                    tmp = (uint8_t)faad_getbits(ld, 4);
                    /* check for downsampled SBR */
                    if(tmp == mp4ASC->samplingFrequencyIndex) mp4ASC->downSampledSBR = 1;
                    mp4ASC->samplingFrequencyIndex = tmp;
                    if(mp4ASC->samplingFrequencyIndex == 15) { mp4ASC->samplingFrequency = (uint32_t)faad_getbits(ld, 24); }
                    else { mp4ASC->samplingFrequency = get_sample_rate(mp4ASC->samplingFrequencyIndex); }
                }
            }
        }
    }
    /* no SBR signalled, this could mean either implicit signalling or no SBR in this file */
    /* MPEG specification states: assume SBR on files with samplerate <= 24000 Hz */
    if(mp4ASC->sbr_present_flag == (char)-1) /* cannot be -1 on systems with unsigned char */
    {
        if(mp4ASC->samplingFrequency <= 24000) {
            mp4ASC->samplingFrequency *= 2;
            mp4ASC->forceUpSampling = 1;
        }
        else /* > 24000*/ { mp4ASC->downSampledSBR = 1; }
    }
#endif
    return result;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t AudioSpecificConfig2(uint8_t* pBuffer, uint32_t buffer_size, mp4AudioSpecificConfig* mp4ASC, program_config* pce, uint8_t short_form) {
    uint8_t ret = 0;
    bitfile ld;
    faad_initbits(&ld, pBuffer, buffer_size);
    faad_byte_align(&ld);
    ret = AudioSpecificConfigFromBitfile(&ld, mp4ASC, pce, buffer_size, short_form);
    return ret;
}

#ifdef SBR_DEC
//----------------------------------------------------------------------------------------------------------------------------------------------------
void extract_envelope_data(sbr_info* sbr, uint8_t ch) {
    uint8_t l, k;

    for(l = 0; l < sbr->L_E[ch]; l++) {
        if(sbr->bs_df_env[ch][l] == 0) {
            for(k = 1; k < sbr->n[sbr->f[ch][l]]; k++) {
                sbr->E[ch][k][l] = sbr->E[ch][k - 1][l] + sbr->E[ch][k][l];
                if(sbr->E[ch][k][l] < 0) sbr->E[ch][k][l] = 0;
            }
        }
        else { /* bs_df_env == 1 */

            uint8_t g = (l == 0) ? sbr->f_prev[ch] : sbr->f[ch][l - 1];
            int16_t E_prev;

            if(sbr->f[ch][l] == g) {
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    if(l == 0) E_prev = sbr->E_prev[ch][k];
                    else
                        E_prev = sbr->E[ch][k][l - 1];

                    sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                }
            }
            else if((g == 1) && (sbr->f[ch][l] == 0)) {
                uint8_t i;
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    for(i = 0; i < sbr->N_high; i++) {
                        if(sbr->f_table_res[HI_RES][i] == sbr->f_table_res[LO_RES][k]) {
                            if(l == 0) E_prev = sbr->E_prev[ch][i];
                            else
                                E_prev = sbr->E[ch][i][l - 1];
                            sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                        }
                    }
                }
            }
            else if((g == 0) && (sbr->f[ch][l] == 1)) {
                uint8_t i;
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    for(i = 0; i < sbr->N_low; i++) {
                        if((sbr->f_table_res[LO_RES][i] <= sbr->f_table_res[HI_RES][k]) &&
                           (sbr->f_table_res[HI_RES][k] < sbr->f_table_res[LO_RES][i + 1])) {
                            if(l == 0) E_prev = sbr->E_prev[ch][i];
                            else
                                E_prev = sbr->E[ch][i][l - 1];
                            sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                        }
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void extract_noise_floor_data(sbr_info* sbr, uint8_t ch) {
    uint8_t l, k;

    for(l = 0; l < sbr->L_Q[ch]; l++) {
        if(sbr->bs_df_noise[ch][l] == 0) {
            for(k = 1; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q[ch][k][l] + sbr->Q[ch][k - 1][l]; }
        }
        else {
            if(l == 0) {
                for(k = 0; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q_prev[ch][k] + sbr->Q[ch][k][0]; }
            }
            else {
                for(k = 0; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q[ch][k][l - 1] + sbr->Q[ch][k][l]; }
            }
        }
    }
}

#endif





/* EOF */
