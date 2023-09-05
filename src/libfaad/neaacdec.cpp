#include "neaacdec.h"
#include <assert.h>

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* common free function */
void faad_free(void* b) { free(b); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

uint32_t faad_get_processed_bits(bitfile* ld) { return (uint32_t)(8 * (4 * (ld->tail - ld->start) - 4) - (ld->bits_left)); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t faad_byte_align(bitfile* ld) {
    int remainder = (32 - ld->bits_left) & 0x7;

    if(remainder) {
        faad_flushbits(ld, 8 - remainder);
        return (uint8_t)(8 - remainder);
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* faad_getbitbuffer(bitfile* ld, uint32_t bits) {
    int          i;
    unsigned int temp;
    int          bytes = bits >> 3;
    int          remainder = bits & 0x7;

    uint8_t* buffer = (uint8_t*)faad_malloc((bytes + 1) * sizeof(uint8_t));

    for(i = 0; i < bytes; i++) { buffer[i] = (uint8_t)faad_getbits(ld, 8); }

    if(remainder) {
        temp = faad_getbits(ld, remainder) << (8 - remainder);

        buffer[bytes] = (uint8_t)temp;
    }

    return buffer;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void passf5(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3, const complex_t* wa4,
                   const int8_t isign) {
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void cfftf(cfft_info* cfft, complex_t* c) { cfftf1neg(cfft->n, c, cfft->work, (const uint16_t*)cfft->ifac, (const complex_t*)cfft->tab, -1); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void cfftb(cfft_info* cfft, complex_t* c) { cfftf1pos(cfft->n, c, cfft->work, (const uint16_t*)cfft->ifac, (const complex_t*)cfft->tab, +1); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void cffti1(uint16_t n, complex_t* wa, uint16_t* ifac) {
    static uint16_t ntryh[4] = {3, 4, 2, 5};
    uint16_t        ntry = 0, i, j;
    uint16_t        ib;
    uint16_t        nf, nl, nq, nr;

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t get_sample_rate(const uint8_t sr_index) { /* Returns the sample rate based on the sample rate index */
    static const uint32_t sample_rates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};
    if(sr_index < 12) return sample_rates[sr_index];
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t max_pred_sfb(const uint8_t sr_index) {
    static const uint8_t pred_sfb_max[] = {33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34};
    if(sr_index < 12) return pred_sfb_max[sr_index];
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void* faad_malloc(size_t size) { return malloc(size); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static const uint8_t Parity[256] = { // parity
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t ones32(uint32_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);

    return (x & 0x0000003f);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t floor_log2(uint32_t x) {

    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);

    return (ones32(x) - 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
    REAL_CONST(1.000000000000000), REAL_CONST(1.010889286051701), REAL_CONST(1.021897148654117), REAL_CONST(1.033024879021228), REAL_CONST(1.044273782427414), REAL_CONST(1.055645178360557),
    REAL_CONST(1.067140400676824), REAL_CONST(1.078760797757120), REAL_CONST(1.090507732665258), REAL_CONST(1.102382583307841), REAL_CONST(1.114386742595892), REAL_CONST(1.126521618608242),
    REAL_CONST(1.138788634756692), REAL_CONST(1.151189229952983), REAL_CONST(1.163724858777578), REAL_CONST(1.176396991650281), REAL_CONST(1.189207115002721), REAL_CONST(1.202156731452703),
    REAL_CONST(1.215247359980469), REAL_CONST(1.228480536106870), REAL_CONST(1.241857812073484), REAL_CONST(1.255380757024691), REAL_CONST(1.269050957191733), REAL_CONST(1.282870016078778),
    REAL_CONST(1.296839554651010), REAL_CONST(1.310961211524764), REAL_CONST(1.325236643159741), REAL_CONST(1.339667524053303), REAL_CONST(1.354255546936893), REAL_CONST(1.369002422974591),
    REAL_CONST(1.383909881963832), REAL_CONST(1.398979672538311), REAL_CONST(1.414213562373095), REAL_CONST(1.429613338391970), REAL_CONST(1.445180806977047), REAL_CONST(1.460917794180647),
    REAL_CONST(1.476826145939499), REAL_CONST(1.492907728291265), REAL_CONST(1.509164427593423), REAL_CONST(1.525598150744538), REAL_CONST(1.542210825407941), REAL_CONST(1.559004400237837),
    REAL_CONST(1.575980845107887), REAL_CONST(1.593142151342267), REAL_CONST(1.610490331949254), REAL_CONST(1.628027421857348), REAL_CONST(1.645755478153965), REAL_CONST(1.663676580326736),
    REAL_CONST(1.681792830507429), REAL_CONST(1.700106353718524), REAL_CONST(1.718619298122478), REAL_CONST(1.737333835273706), REAL_CONST(1.756252160373300), REAL_CONST(1.775376492526521),
    REAL_CONST(1.794709075003107), REAL_CONST(1.814252175500399), REAL_CONST(1.834008086409342), REAL_CONST(1.853979125083386), REAL_CONST(1.874167634110300), REAL_CONST(1.894575981586966),
    REAL_CONST(1.915206561397147), REAL_CONST(1.936061793492294), REAL_CONST(1.957144124175400), REAL_CONST(1.978456026387951), REAL_CONST(2.000000000000000)};

static const int32_t log2_tab[] = {
    REAL_CONST(0.000000000000000), REAL_CONST(0.022367813028455), REAL_CONST(0.044394119358453), REAL_CONST(0.066089190457772), REAL_CONST(0.087462841250339), REAL_CONST(0.108524456778169),
    REAL_CONST(0.129283016944966), REAL_CONST(0.149747119504682), REAL_CONST(0.169925001442312), REAL_CONST(0.189824558880017), REAL_CONST(0.209453365628950), REAL_CONST(0.228818690495881),
    REAL_CONST(0.247927513443585), REAL_CONST(0.266786540694901), REAL_CONST(0.285402218862248), REAL_CONST(0.303780748177103), REAL_CONST(0.321928094887362), REAL_CONST(0.339850002884625),
    REAL_CONST(0.357552004618084), REAL_CONST(0.375039431346925), REAL_CONST(0.392317422778760), REAL_CONST(0.409390936137702), REAL_CONST(0.426264754702098), REAL_CONST(0.442943495848728),
    REAL_CONST(0.459431618637297), REAL_CONST(0.475733430966398), REAL_CONST(0.491853096329675), REAL_CONST(0.507794640198696), REAL_CONST(0.523561956057013), REAL_CONST(0.539158811108031),
    REAL_CONST(0.554588851677637), REAL_CONST(0.569855608330948), REAL_CONST(0.584962500721156), REAL_CONST(0.599912842187128), REAL_CONST(0.614709844115208), REAL_CONST(0.629356620079610),
    REAL_CONST(0.643856189774725), REAL_CONST(0.658211482751795), REAL_CONST(0.672425341971496), REAL_CONST(0.686500527183218), REAL_CONST(0.700439718141092), REAL_CONST(0.714245517666123),
    REAL_CONST(0.727920454563199), REAL_CONST(0.741466986401147), REAL_CONST(0.754887502163469), REAL_CONST(0.768184324776926), REAL_CONST(0.781359713524660), REAL_CONST(0.794415866350106),
    REAL_CONST(0.807354922057604), REAL_CONST(0.820178962415188), REAL_CONST(0.832890014164742), REAL_CONST(0.845490050944375), REAL_CONST(0.857980995127572), REAL_CONST(0.870364719583405),
    REAL_CONST(0.882643049361841), REAL_CONST(0.894817763307943), REAL_CONST(0.906890595608519), REAL_CONST(0.918863237274595), REAL_CONST(0.930737337562886), REAL_CONST(0.942514505339240),
    REAL_CONST(0.954196310386875), REAL_CONST(0.965784284662087), REAL_CONST(0.977279923499917), REAL_CONST(0.988684686772166), REAL_CONST(1.000000000000000)};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
        a[m] = tmp2[m - 1];                                                    /* changed */
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string) {
    static const char* libfaadName = "2.20.1";
    static const char* libCopyright = " Copyright 2002-2004: Ahead Software AG\n"
                                      " http://www.audiocoding.com\n"
                                      " bug tracking: https://sourceforge.net/p/faac/bugs/\n";

    if(faad_id_string) *faad_id_string = libfaadName;
    if(faad_copyright_string) *faad_copyright_string = libCopyright;
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const unsigned char mes[] = {0x67, 0x20, 0x61, 0x20, 0x20, 0x20, 0x6f, 0x20, 0x72, 0x20, 0x65, 0x20, 0x6e, 0x20, 0x20, 0x20, 0x74,
                             0x20, 0x68, 0x20, 0x67, 0x20, 0x69, 0x20, 0x72, 0x20, 0x79, 0x20, 0x70, 0x20, 0x6f, 0x20, 0x63};
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
NeAACDecConfigurationPtr_t NeAACDecGetCurrentConfiguration(NeAACDecHandle hpDecoder) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if(hDecoder) {
        NeAACDecConfigurationPtr_t config = &(hDecoder->config);
        return config;
    }
    return NULL;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void NeAACDecPostSeekReset(NeAACDecHandle hpDecoder, long frame) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if(hDecoder) {
        hDecoder->postSeekResetFlag = 1;

        if(frame != -1) hDecoder->frame = frame;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void* NeAACDecDecode(NeAACDecHandle hpDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    return aac_frame_decode(hDecoder, hInfo, buffer, buffer_size, NULL, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void* NeAACDecDecode2(NeAACDecHandle hpDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer, unsigned long sample_buffer_size) {
    NeAACDecStruct* hDecoder = (NeAACDecStruct*)hpDecoder;
    if((sample_buffer == NULL) || (sample_buffer_size == 0)) {
        hInfo->error = 27;
        return NULL;
    }
    return aac_frame_decode(hDecoder, hInfo, buffer, buffer_size, sample_buffer, sample_buffer_size);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void drc_end(drc_info* drc) {
    if(drc) faad_free(drc);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t drc_pow2_table[] = {COEF_CONST(0.5146511183), COEF_CONST(0.5297315472), COEF_CONST(0.5452538663), COEF_CONST(0.5612310242), COEF_CONST(0.5776763484), COEF_CONST(0.5946035575),
                                   COEF_CONST(0.6120267717), COEF_CONST(0.6299605249), COEF_CONST(0.6484197773), COEF_CONST(0.6674199271), COEF_CONST(0.6869768237), COEF_CONST(0.7071067812),
                                   COEF_CONST(0.7278265914), COEF_CONST(0.7491535384), COEF_CONST(0.7711054127), COEF_CONST(0.7937005260), COEF_CONST(0.8169577266), COEF_CONST(0.8408964153),
                                   COEF_CONST(0.8655365610), COEF_CONST(0.8908987181), COEF_CONST(0.9170040432), COEF_CONST(0.9438743127), COEF_CONST(0.9715319412), COEF_CONST(1.0000000000),
                                   COEF_CONST(1.0293022366), COEF_CONST(1.0594630944), COEF_CONST(1.0905077327), COEF_CONST(1.1224620483), COEF_CONST(1.1553526969), COEF_CONST(1.1892071150),
                                   COEF_CONST(1.2240535433), COEF_CONST(1.2599210499), COEF_CONST(1.2968395547), COEF_CONST(1.3348398542), COEF_CONST(1.3739536475), COEF_CONST(1.4142135624),
                                   COEF_CONST(1.4556531828), COEF_CONST(1.4983070769), COEF_CONST(1.5422108254), COEF_CONST(1.5874010520), COEF_CONST(1.6339154532), COEF_CONST(1.6817928305),
                                   COEF_CONST(1.7310731220), COEF_CONST(1.7817974363), COEF_CONST(1.8340080864), COEF_CONST(1.8877486254), COEF_CONST(1.9430638823)};
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void drc_decode(drc_info* drc, int32_t* spec) {
    uint16_t i, bd, top;
    int32_t  exp, frac;

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ifilter_bank(fb_info* fb, uint8_t window_sequence, uint8_t window_shape, uint8_t window_shape_prev, int32_t* freq_in, int32_t* time_out, int32_t* overlap, uint8_t object_type,
                  uint16_t frame_len) {
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
            time_out[nflat_ls + 1 * nshort + i] =
                overlap[nflat_ls + nshort * 1 + i] + MUL_F(transf_buf[nshort * 1 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 2 + i], window_short[i]);
            time_out[nflat_ls + 2 * nshort + i] =
                overlap[nflat_ls + nshort * 2 + i] + MUL_F(transf_buf[nshort * 3 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 4 + i], window_short[i]);
            time_out[nflat_ls + 3 * nshort + i] =
                overlap[nflat_ls + nshort * 3 + i] + MUL_F(transf_buf[nshort * 5 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 6 + i], window_short[i]);
            if(i < trans)
                time_out[nflat_ls + 4 * nshort + i] =
                    overlap[nflat_ls + nshort * 4 + i] + MUL_F(transf_buf[nshort * 7 + i], window_short[nshort - 1 - i]) + MUL_F(transf_buf[nshort * 8 + i], window_short[i]);
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline void huffman_sign_bits(bitfile* ld, int16_t* sp, uint8_t len) {
    uint8_t i;

    for(i = 0; i < len; i++) {
        if(sp[i]) {
            if(faad_get1bit(ld) & 1) { sp[i] = -sp[i]; }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_2step_quad(cb, ld, sp);
    huffman_sign_bits(ld, sp, QUAD_LEN);
    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_2step_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_2step_pair(cb, ld, sp);
    huffman_sign_bits(ld, sp, PAIR_LEN);
    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_binary_quad(cb, ld, sp);
    huffman_sign_bits(ld, sp, QUAD_LEN);

    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t huffman_binary_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp) {
    uint8_t err = huffman_binary_pair(cb, ld, sp);
    huffman_sign_bits(ld, sp, PAIR_LEN);
    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int16_t huffman_codebook(uint8_t i) {
    static const uint32_t data = 16428320;
    if(i == 0) return (int16_t)(data >> 16) & 0xFFFF;
    else
        return (int16_t)data & 0xFFFF;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t pow05_table[] = {
    COEF_CONST(1.68179283050743), /* 0.5^(-3/4) */
    COEF_CONST(1.41421356237310), /* 0.5^(-2/4) */
    COEF_CONST(1.18920711500272), /* 0.5^(-1/4) */
    COEF_CONST(1.0),              /* 0.5^( 0/4) */
    COEF_CONST(0.84089641525371), /* 0.5^(+1/4) */
    COEF_CONST(0.70710678118655), /* 0.5^(+2/4) */
    COEF_CONST(0.59460355750136)  /* 0.5^(+3/4) */
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void is_decode(ic_stream* ics, ic_stream* icsr, int32_t* l_spec, int32_t* r_spec, uint16_t frame_len) {
    uint8_t  g, sfb, b;
    uint16_t i;
    int32_t  exp, frac;
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* bits_t version */
static void rewrev_bits(bits_t* bits) {
    if(bits->len == 0) return;
    rewrev_lword(&bits->bufb, &bits->bufa, bits->len);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void fill_in_codeword(codeword_t* codeword, uint16_t index, uint16_t sp, uint8_t cb) {
    codeword[index].sp_offset = sp;
    codeword[index].cb = cb;
    codeword[index].decoded = 0;
    codeword[index].bits.len = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
                                                        segment[numberOfSegments].bufb + showbits_hcr(&segment[numberOfSegments - 1], segment[numberOfSegments - 1].len - 32);
                                                    segment[numberOfSegments - 1].bufa = segment[numberOfSegments].bufa + showbits_hcr(&segment[numberOfSegments - 1], 32);
                                                }
                                                else {
                                                    segment[numberOfSegments - 1].bufa =
                                                        segment[numberOfSegments].bufa + showbits_hcr(&segment[numberOfSegments - 1], segment[numberOfSegments - 1].len);
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
                    if(huffman_spectral_data_2(codeword[codeword_idx].cb, &segment[segment_idx], &spectral_data[codeword[codeword_idx].sp_offset]) >= 0) { codeword[codeword_idx].decoded = 1; }
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void pns_decode(ic_stream* ics_left, ic_stream* ics_right, int32_t* spec_left, int32_t* spec_right, uint16_t frame_len, uint8_t channel_pair, uint8_t object_type,
                /* RNG states */ uint32_t* __r1, uint32_t* __r2) {
    uint8_t  g, sfb, b;
    uint16_t size, offs;
    uint8_t  group = 0;
    uint16_t nshort = frame_len >> 3;
    uint8_t  sub = 0;

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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static hyb_info* hybrid_init(uint8_t numTimeSlotsRate) {
    uint8_t   i;
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* complex filter, size 4 */
static void channel_filter4(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid) {
    uint8_t i;
    int32_t input_re1[2], input_re2[2], input_im1[2], input_im2[2];

    for(i = 0; i < frame_len; i++) {
        input_re1[0] = -MUL_F(filter[2], (QMF_RE(buffer[i + 2]) + QMF_RE(buffer[i + 10]))) + MUL_F(filter[6], QMF_RE(buffer[i + 6]));
        input_re1[1] = MUL_F(FRAC_CONST(-0.70710678118655), (MUL_F(filter[1], (QMF_RE(buffer[i + 1]) + QMF_RE(buffer[i + 11]))) + MUL_F(filter[3], (QMF_RE(buffer[i + 3]) + QMF_RE(buffer[i + 9]))) -
                                                             MUL_F(filter[5], (QMF_RE(buffer[i + 5]) + QMF_RE(buffer[i + 7])))));
        input_im1[0] = MUL_F(filter[0], (QMF_IM(buffer[i + 0]) - QMF_IM(buffer[i + 12]))) - MUL_F(filter[4], (QMF_IM(buffer[i + 4]) - QMF_IM(buffer[i + 8])));
        input_im1[1] = MUL_F(FRAC_CONST(0.70710678118655), (MUL_F(filter[1], (QMF_IM(buffer[i + 1]) - QMF_IM(buffer[i + 11]))) - MUL_F(filter[3], (QMF_IM(buffer[i + 3]) - QMF_IM(buffer[i + 9]))) -
                                                            MUL_F(filter[5], (QMF_IM(buffer[i + 5]) - QMF_IM(buffer[i + 7])))));
        input_re2[0] = MUL_F(filter[0], (QMF_RE(buffer[i + 0]) - QMF_RE(buffer[i + 12]))) - MUL_F(filter[4], (QMF_RE(buffer[i + 4]) - QMF_RE(buffer[i + 8])));
        input_re2[1] = MUL_F(FRAC_CONST(0.70710678118655), (MUL_F(filter[1], (QMF_RE(buffer[i + 1]) - QMF_RE(buffer[i + 11]))) - MUL_F(filter[3], (QMF_RE(buffer[i + 3]) - QMF_RE(buffer[i + 9]))) -
                                                            MUL_F(filter[5], (QMF_RE(buffer[i + 5]) - QMF_RE(buffer[i + 7])))));
        input_im2[0] = -MUL_F(filter[2], (QMF_IM(buffer[i + 2]) + QMF_IM(buffer[i + 10]))) + MUL_F(filter[6], QMF_IM(buffer[i + 6]));
        input_im2[1] = MUL_F(FRAC_CONST(-0.70710678118655), (MUL_F(filter[1], (QMF_IM(buffer[i + 1]) + QMF_IM(buffer[i + 11]))) + MUL_F(filter[3], (QMF_IM(buffer[i + 3]) + QMF_IM(buffer[i + 9]))) -
                                                             MUL_F(filter[5], (QMF_IM(buffer[i + 5]) + QMF_IM(buffer[i + 7])))));
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* limits the value i to the range [min,max] */
static int8_t delta_clip(int8_t i, int8_t min, int8_t max) {
    if(i < min) return min;
    else if(i > max)
        return max;
    else
        return i;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static const int32_t ipdopd_cos_tab[] = {FRAC_CONST(1.000000000000000),  FRAC_CONST(0.707106781186548),  FRAC_CONST(0.000000000000000), FRAC_CONST(-0.707106781186547), FRAC_CONST(-1.000000000000000),
                                         FRAC_CONST(-0.707106781186548), FRAC_CONST(-0.000000000000000), FRAC_CONST(0.707106781186547), FRAC_CONST(1.000000000000000)};
static const int32_t ipdopd_sin_tab[] = {FRAC_CONST(0.000000000000000),  FRAC_CONST(0.707106781186547),  FRAC_CONST(1.000000000000000),  FRAC_CONST(0.707106781186548), FRAC_CONST(0.000000000000000),
                                         FRAC_CONST(-0.707106781186547), FRAC_CONST(-1.000000000000000), FRAC_CONST(-0.707106781186548), FRAC_CONST(-0.000000000000000)};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t magnitude_c(complex_t c) {

    #define ps_abs(A) (((A) > 0) ? (A) : (-(A)))
    #define ALPHA     FRAC_CONST(0.948059448969)
    #define BETA      FRAC_CONST(0.392699081699)

    int32_t abs_inphase = ps_abs(RE(c));
    int32_t abs_quadrature = ps_abs(IM(c));
    if(abs_inphase > abs_quadrature) { return MUL_F(abs_inphase, ALPHA) + MUL_F(abs_quadrature, BETA); }
    else { return MUL_F(abs_quadrature, ALPHA) + MUL_F(abs_inphase, BETA); }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

            /* calculate phase rotation parameters H_xy, note that the imaginary part of these parameters are only calculated when
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ps_free(ps_info* ps) {
    /* free hybrid filterbank structures */
    hybrid_free((hyb_info*)ps->hyb);

    faad_free(ps);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void lt_prediction(ic_stream* ics, ltp_info* ltp, int32_t* spec, int16_t* lt_pred_stat, fb_info* fb, uint8_t win_shape, uint8_t win_shape_prev, uint8_t sr_index, uint8_t object_type,
                   uint16_t frame_len) {
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void faad_imdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out) {
    uint16_t  k;
    complex_t x;
#ifdef ALLOW_SMALL_FRAMELENGTH
    int32_t scale, b_scale = 0;
#endif
    complex_t  Z1[512];
    complex_t* sincos = mdct->sincos;
    uint16_t   N = mdct->N;
    uint16_t   N2 = N >> 1;
    uint16_t   N4 = N >> 2;
    uint16_t   N8 = N >> 3;

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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void faad_mdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out) {
    uint16_t   k;
    complex_t  x;
    complex_t  Z1[512];
    complex_t* sincos = mdct->sincos;
    uint16_t   N = mdct->N;
    uint16_t   N2 = N >> 1;
    uint16_t   N4 = N >> 2;
    uint16_t   N8 = N >> 3;

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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
char NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC) { return AudioSpecificConfig2(pBuffer, buffer_size, mp4ASC, NULL, 0); }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
    if(mp4ASC->objectTypeIndex == 1 || mp4ASC->objectTypeIndex == 2 || mp4ASC->objectTypeIndex == 3 || mp4ASC->objectTypeIndex == 4 || mp4ASC->objectTypeIndex == 6 || mp4ASC->objectTypeIndex == 7) {
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int8_t AudioSpecificConfig2(uint8_t* pBuffer, uint32_t buffer_size, mp4AudioSpecificConfig* mp4ASC, program_config* pce, uint8_t short_form) {
    uint8_t ret = 0;
    bitfile ld;
    faad_initbits(&ld, pBuffer, buffer_size);
    faad_byte_align(&ld);
    ret = AudioSpecificConfigFromBitfile(&ld, mp4ASC, pce, buffer_size, short_form);
    return ret;
}

#ifdef SBR_DEC
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
                        if((sbr->f_table_res[LO_RES][i] <= sbr->f_table_res[HI_RES][k]) && (sbr->f_table_res[HI_RES][k] < sbr->f_table_res[LO_RES][i + 1])) {
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

#ifdef PS_DEC
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t ps_data(ps_info* ps, bitfile* ld, uint8_t* header) {
    uint8_t  tmp, n;
    uint16_t bits = (uint16_t)faad_get_processed_bits(ld);

    *header = 0;

    /* check for new PS header */
    if(faad_get1bit(ld)) {
        *header = 1;
        ps->header_read = 1;
        ps->use34hybrid_bands = 0;
        /* Inter-channel Intensity Difference (IID) parameters enabled */
        ps->enable_iid = (uint8_t)faad_get1bit(ld);
        if(ps->enable_iid) {
            ps->iid_mode = (uint8_t)faad_getbits(ld, 3);
            ps->nr_iid_par = nr_iid_par_tab[ps->iid_mode];
            ps->nr_ipdopd_par = nr_ipdopd_par_tab[ps->iid_mode];
            if(ps->iid_mode == 2 || ps->iid_mode == 5) ps->use34hybrid_bands = 1;
            /* IPD freq res equal to IID freq res */
            ps->ipd_mode = ps->iid_mode;
        }
        /* Inter-channel Coherence (ICC) parameters enabled */
        ps->enable_icc = (uint8_t)faad_get1bit(ld);
        if(ps->enable_icc) {
            ps->icc_mode = (uint8_t)faad_getbits(ld, 3);
            ps->nr_icc_par = nr_icc_par_tab[ps->icc_mode];
            if(ps->icc_mode == 2 || ps->icc_mode == 5) ps->use34hybrid_bands = 1;
        }
        /* PS extension layer enabled */
        ps->enable_ext = (uint8_t)faad_get1bit(ld);
    }
    /* we are here, but no header has been read yet */
    if(ps->header_read == 0) {
        ps->ps_data_available = 0;
        return 1;
    }
    ps->frame_class = (uint8_t)faad_get1bit(ld);
    tmp = (uint8_t)faad_getbits(ld, 2);
    ps->num_env = num_env_tab[ps->frame_class][tmp];
    if(ps->frame_class) {
        for(n = 1; n < ps->num_env + 1; n++) { ps->border_position[n] = (uint8_t)faad_getbits(ld, 5) + 1; }
    }
    if(ps->enable_iid) {
        for(n = 0; n < ps->num_env; n++) {
            ps->iid_dt[n] = (uint8_t)faad_get1bit(ld);
            /* iid_data */
            if(ps->iid_mode < 3) { huff_data(ld, ps->iid_dt[n], ps->nr_iid_par, t_huff_iid_def, f_huff_iid_def, ps->iid_index[n]); }
            else { huff_data(ld, ps->iid_dt[n], ps->nr_iid_par, t_huff_iid_fine, f_huff_iid_fine, ps->iid_index[n]); }
        }
    }
    if(ps->enable_icc) {
        for(n = 0; n < ps->num_env; n++) {
            ps->icc_dt[n] = (uint8_t)faad_get1bit(ld);

            /* icc_data */
            huff_data(ld, ps->icc_dt[n], ps->nr_icc_par, t_huff_icc, f_huff_icc, ps->icc_index[n]);
        }
    }
    if(ps->enable_ext) {
        uint16_t num_bits_left;
        uint16_t cnt = (uint16_t)faad_getbits(ld, 4);
        if(cnt == 15) { cnt += (uint16_t)faad_getbits(ld, 8); }
        num_bits_left = 8 * cnt;
        while(num_bits_left > 7) {
            uint8_t ps_extension_id = (uint8_t)faad_getbits(ld, 2);
            num_bits_left -= 2;
            num_bits_left -= ps_extension(ps, ld, ps_extension_id, num_bits_left);
        }
        faad_getbits(ld, num_bits_left);
    }
    bits = (uint16_t)faad_get_processed_bits(ld) - bits;
    ps->ps_data_available = 1;
    return bits;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint16_t ps_extension(ps_info* ps, bitfile* ld, const uint8_t ps_extension_id, const uint16_t num_bits_left) {
    uint8_t  n;
    uint16_t bits = (uint16_t)faad_get_processed_bits(ld);

    if(ps_extension_id == 0) {
        ps->enable_ipdopd = (uint8_t)faad_get1bit(ld);
        if(ps->enable_ipdopd) {
            for(n = 0; n < ps->num_env; n++) {
                ps->ipd_dt[n] = (uint8_t)faad_get1bit(ld);
                /* ipd_data */
                huff_data(ld, ps->ipd_dt[n], ps->nr_ipdopd_par, t_huff_ipd, f_huff_ipd, ps->ipd_index[n]);
                ps->opd_dt[n] = (uint8_t)faad_get1bit(ld);
                /* opd_data */
                huff_data(ld, ps->opd_dt[n], ps->nr_ipdopd_par, t_huff_opd, f_huff_opd, ps->opd_index[n]);
            }
        }
        faad_get1bit(ld);
    }
    /* return number of bits read */
    bits = (uint16_t)faad_get_processed_bits(ld) - bits;
    return bits;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* read huffman data coded in either the frequency or the time direction */
static void huff_data(bitfile* ld, const uint8_t dt, const uint8_t nr_par, ps_huff_tab t_huff, ps_huff_tab f_huff, int8_t* par) {
    uint8_t n;

    if(dt) {
        /* coded in time direction */
        for(n = 0; n < nr_par; n++) { par[n] = ps_huff_dec(ld, t_huff); }
    }
    else {
        /* coded in frequency direction */
        par[0] = ps_huff_dec(ld, f_huff);
        for(n = 1; n < nr_par; n++) { par[n] = ps_huff_dec(ld, f_huff); }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t ps_huff_dec(bitfile* ld, ps_huff_tab t_huff) { /* binary search huffman decoding */
    uint8_t bit;
    int16_t index = 0;

    while(index >= 0) {
        bit = (uint8_t)faad_get1bit(ld);
        index = t_huff[index][bit];
    }
    return index + 31;
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t pulse_decode(ic_stream* ics, int16_t* spec_data, uint16_t framelen) {
    uint8_t     i;
    uint16_t    k;
    pulse_info* pul = &(ics->pul);

    k = min(ics->swb_offset[pul->pulse_start_sfb], ics->swb_offset_max);
    for(i = 0; i <= pul->number_pulse; i++) {
        k += pul->pulse_offset[i];
        if(k >= framelen) return 15; /* should not be possible */

        if(spec_data[k] > 0) spec_data[k] += pul->pulse_amp[i];
        else
            spec_data[k] -= pul->pulse_amp[i];
    }
    return 0;
}

#ifdef ERROR_RESILIENCE
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t rvlc_scale_factor_data(ic_stream* ics, bitfile* ld) {
    uint8_t bits = 9;

    ics->sf_concealment = faad_get1bit(ld);
    ics->rev_global_gain = (uint8_t)faad_getbits(ld, 8);
    if(ics->window_sequence == EIGHT_SHORT_SEQUENCE) bits = 11;
    /* the number of bits used for the huffman codewords */
    ics->length_of_rvlc_sf = (uint16_t)faad_getbits(ld, bits);
    if(ics->noise_used) {
        ics->dpcm_noise_nrg = (uint16_t)faad_getbits(ld, 9);
        ics->length_of_rvlc_sf -= 9;
    }
    ics->sf_escapes_present = faad_get1bit(ld);
    if(ics->sf_escapes_present) { ics->length_of_rvlc_escapes = (uint8_t)faad_getbits(ld, 8); }
    if(ics->noise_used) { ics->dpcm_noise_last_position = (uint16_t)faad_getbits(ld, 9); }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t rvlc_decode_scale_factors(ic_stream* ics, bitfile* ld) {
    uint8_t  result;
    uint8_t  intensity_used = 0;
    uint8_t* rvlc_sf_buffer = NULL;
    uint8_t* rvlc_esc_buffer = NULL;
    bitfile  ld_rvlc_sf, ld_rvlc_esc;
    //    bitfile ld_rvlc_sf_rev, ld_rvlc_esc_rev;

    if(ics->length_of_rvlc_sf > 0) {
        /* We read length_of_rvlc_sf bits here to put it in a seperate bitfile. */
        rvlc_sf_buffer = faad_getbitbuffer(ld, ics->length_of_rvlc_sf);
        faad_initbits(&ld_rvlc_sf, (void*)rvlc_sf_buffer, bit2byte(ics->length_of_rvlc_sf));
        //        faad_initbits_rev(&ld_rvlc_sf_rev, (void*)rvlc_sf_buffer,
        //            ics->length_of_rvlc_sf);
    }

    if(ics->sf_escapes_present) {
        /* We read length_of_rvlc_escapes bits here to put it in a
           seperate bitfile.
        */
        rvlc_esc_buffer = faad_getbitbuffer(ld, ics->length_of_rvlc_escapes);

        faad_initbits(&ld_rvlc_esc, (void*)rvlc_esc_buffer, bit2byte(ics->length_of_rvlc_escapes));
        //        faad_initbits_rev(&ld_rvlc_esc_rev, (void*)rvlc_esc_buffer,
        //            ics->length_of_rvlc_escapes);
    }

    /* decode the rvlc scale factors and escapes */
    result = rvlc_decode_sf_forward(ics, &ld_rvlc_sf, &ld_rvlc_esc, &intensity_used);
    //    result = rvlc_decode_sf_reverse(ics, &ld_rvlc_sf_rev,
    //        &ld_rvlc_esc_rev, intensity_used);

    if(rvlc_esc_buffer) faad_free(rvlc_esc_buffer);
    if(rvlc_sf_buffer) faad_free(rvlc_sf_buffer);
    if(ics->length_of_rvlc_sf > 0)
        if(ics->sf_escapes_present) return result;
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t rvlc_decode_sf_forward(ic_stream* ics, bitfile* ld_sf, bitfile* ld_esc, uint8_t* intensity_used) {
    int8_t  g, sfb;
    int8_t  t = 0;
    int8_t  error = 0;
    int8_t  noise_pcm_flag = 1;
    int16_t scale_factor = ics->global_gain;
    int16_t is_position = 0;
    int16_t noise_energy = ics->global_gain - 90 - 256;

    for(g = 0; g < ics->num_window_groups; g++) {
        for(sfb = 0; sfb < ics->max_sfb; sfb++) {
            if(error) { ics->scale_factors[g][sfb] = 0; }
            else {
                switch(ics->sfb_cb[g][sfb]) {
                case ZERO_HCB: /* zero book */ ics->scale_factors[g][sfb] = 0; break;
                case INTENSITY_HCB: /* intensity books */
                case INTENSITY_HCB2:
                    *intensity_used = 1;
                    /* decode intensity position */
                    t = rvlc_huffman_sf(ld_sf, ld_esc, +1);
                    is_position += t;
                    ics->scale_factors[g][sfb] = is_position;
                    break;
                case NOISE_HCB: /* noise books */
                    /* decode noise energy */
                    if(noise_pcm_flag) {
                        int16_t n = ics->dpcm_noise_nrg;
                        noise_pcm_flag = 0;
                        noise_energy += n;
                    }
                    else {
                        t = rvlc_huffman_sf(ld_sf, ld_esc, +1);
                        noise_energy += t;
                    }
                    ics->scale_factors[g][sfb] = noise_energy;
                    break;
                default: /* spectral books */
                    /* decode scale factor */
                    t = rvlc_huffman_sf(ld_sf, ld_esc, +1);
                    scale_factor += t;
                    if(scale_factor < 0) return 4;
                    ics->scale_factors[g][sfb] = scale_factor;
                    break;
                }
                if(t == 99) { error = 1; }
            }
        }
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int8_t rvlc_huffman_sf(bitfile* ld_sf, bitfile* ld_esc, int8_t direction) {
    uint8_t          i, j;
    int8_t           index;
    uint32_t         cw;
    rvlc_huff_table* h = book_rvlc;

    i = h->len;
    if(direction > 0) cw = faad_getbits(ld_sf, i);
    else
        cw = faad_getbits_rev(ld_sf, i);

    while((cw != h->cw) && (i < 10)) {
        h++;
        j = h->len - i;
        i += j;
        cw <<= j;
        if(direction > 0) cw |= faad_getbits(ld_sf, j);
        else
            cw |= faad_getbits_rev(ld_sf, j);
    }
    index = h->index;
    if(index == +ESC_VAL) {
        int8_t esc = rvlc_huffman_esc(ld_esc, direction);
        if(esc == 99) return 99;
        index += esc;
    }
    if(index == -ESC_VAL) {
        int8_t esc = rvlc_huffman_esc(ld_esc, direction);
        if(esc == 99) return 99;
        index -= esc;
    }
    return index;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int8_t rvlc_huffman_esc(bitfile* ld, int8_t direction) {
    uint8_t          i, j;
    uint32_t         cw;
    rvlc_huff_table* h = book_escape;

    i = h->len;
    if(direction > 0) cw = faad_getbits(ld, i);
    else
        cw = faad_getbits_rev(ld, i);
    while((cw != h->cw) && (i < 21)) {
        h++;
        j = h->len - i;
        i += j;
        cw <<= j;
        if(direction > 0) cw |= faad_getbits(ld, j);
        else
            cw |= faad_getbits_rev(ld, j);
    }
    return h->index;
}

#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// FFT decimation in frequency,  4*16*2+16=128+16=144 multiplications, 6*16*2+10*8+4*16*2=192+80+128=400 additions
static void fft_dif(int32_t* Real, int32_t* Imag) {
    int32_t  w_real, w_imag;                                     // For faster access
    int32_t  point1_real, point1_imag, point2_real, point2_imag; // For faster access
    uint32_t j, i, i2, w_index;                                  // Counters

    // First 2 stages of 32 point FFT decimation in frequency 4*16*2=64*2=128 multiplications, 6*16*2=96*2=192 additions
    // Stage 1 of 32 point FFT decimation in frequency
    for(i = 0; i < 16; i++) {
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i + 16;
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        w_real = w_array_real[i];
        w_imag = w_array_imag[i];
        point1_real -= point2_real; // temp1 = x[i] - x[i2]
        point1_imag -= point2_imag;
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag)); // x[i2] = (x[i] - x[i2]) * w
        Imag[i2] = (MUL_F(point1_real, w_imag) + MUL_F(point1_imag, w_real));
    }
    // Stage 2 of 32 point FFT decimation in frequency
    for(j = 0, w_index = 0; j < 8; j++, w_index += 2) {
        w_real = w_array_real[w_index];
        w_imag = w_array_imag[w_index];
        i = j;
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i + 8;
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        point1_real -= point2_real; // temp1 = x[i] - x[i2]
        point1_imag -= point2_imag;
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag)); // x[i2] = (x[i] - x[i2]) * w
        Imag[i2] = (MUL_F(point1_real, w_imag) + MUL_F(point1_imag, w_real));
        i = j + 16;
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i + 8;
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        point1_real -= point2_real; // temp1 = x[i] - x[i2]
        point1_imag -= point2_imag;
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag)); // x[i2] = (x[i] - x[i2]) * w
        Imag[i2] = (MUL_F(point1_real, w_imag) + MUL_F(point1_imag, w_real));
    }
    // Stage 3 of 32 point FFT decimation in frequency, 2*4*2=16 multiplications, 4*4*2+6*4*2=10*8=80 additions
    for(i = 0; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        Real[i] += point2_real; // out[i1] = point1 + point2
        Imag[i] += point2_imag;
        Real[i2] = point1_real - point2_real; // out[i2] = point1 - point2
        Imag[i2] = point1_imag - point2_imag;
    }
    w_real = w_array_real[4];    // = sqrt(2)/2
    for(i = 1; i < 32; i += 8) { // w_imag = -w_real; // = w_array_imag[4]; // = -sqrt(2)/2
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        point1_real -= point2_real; // temp1 = x[i] - x[i2]
        point1_imag -= point2_imag;
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = MUL_F(point1_real + point1_imag, w_real); // x[i2] = (x[i] - x[i2]) * w
        Imag[i2] = MUL_F(point1_imag - point1_real, w_real);
    }
    for(i = 2; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        Real[i] += point2_real; // x[i] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = point1_imag - point2_imag; // x[i2] = (x[i] - x[i2]) * (-i)
        Imag[i2] = point2_real - point1_real;
    }
    w_real = w_array_real[12];   // = -sqrt(2)/2
    for(i = 3; i < 32; i += 8) { // w_imag = w_real; // = w_array_imag[12]; // = -sqrt(2)/2
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        point1_real -= point2_real; // temp1 = x[i] - x[i2]
        point1_imag -= point2_imag;
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = MUL_F(point1_real - point1_imag, w_real); // x[i2] = (x[i] - x[i2]) * w
        Imag[i2] = MUL_F(point1_real + point1_imag, w_real);
    }
    // Stage 4 of 32 point FFT decimation in frequency (no multiplications) 16*4=64 additions
    for(i = 0; i < 32; i += 4) {
        i2 = i + 2;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        Real[i] += point2_real; // x[i1] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = point1_real - point2_real; // x[i2] = x[i] - x[i2]
        Imag[i2] = point1_imag - point2_imag;
    }
    for(i = 1; i < 32; i += 4) {
        i2 = i + 2;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        Real[i] += point2_real; // x[i] = x[i] + x[i2]
        Imag[i] += point2_imag;
        Real[i2] = point1_imag - point2_imag; // x[i2] = (x[i] - x[i2]) * (-i)
        Imag[i2] = point2_real - point1_real;
    }
    // Stage 5 of 32 point FFT decimation in frequency (no multiplications) 16*4=64 additions
    for(i = 0; i < 32; i += 2) {
        i2 = i + 1;
        point1_real = Real[i];
        point1_imag = Imag[i];
        point2_real = Real[i2];
        point2_imag = Imag[i2];
        Real[i] += point2_real; // out[i1] = point1 + point2
        Imag[i] += point2_imag;
        Real[i2] = point1_real - point2_real; // out[i2] = point1 - point2
        Imag[i2] = point1_imag - point2_imag;
    }

#ifdef REORDER_IN_FFT
    FFTReorder(Real, Imag);
#endif // #ifdef REORDER_IN_FFT
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* size 64 only! */
void dct4_kernel(int32_t* in_real, int32_t* in_imag, int32_t* out_real, int32_t* out_imag) {
    // Tables with bit reverse values for 5 bits, bit reverse of i at i-th position
    const uint8_t bit_rev_tab[32] = {0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31};
    uint32_t      i, i_rev;

    /* Step 2: modulate */
    // 3*32=96 multiplications
    // 3*32=96 additions
    for(i = 0; i < 32; i++) {
        int32_t x_re, x_im, tmp;
        x_re = in_real[i];
        x_im = in_imag[i];
        tmp = MUL_C(x_re + x_im, dct4_64_tab[i]);
        in_real[i] = MUL_C(x_im, dct4_64_tab[i + 64]) + tmp;
        in_imag[i] = MUL_C(x_re, dct4_64_tab[i + 32]) + tmp;
    }

    /* Step 3: FFT, but with output in bit reverse order */
    fft_dif(in_real, in_imag);
    /* Step 4: modulate + bitreverse reordering */
    // 3*31+2=95 multiplications
    // 3*31+2=95 additions
    for(i = 0; i < 16; i++) {
        int32_t x_re, x_im, tmp;
        i_rev = bit_rev_tab[i];
        x_re = in_real[i_rev];
        x_im = in_imag[i_rev];
        tmp = MUL_C(x_re + x_im, dct4_64_tab[i + 3 * 32]);
        out_real[i] = MUL_C(x_im, dct4_64_tab[i + 5 * 32]) + tmp;
        out_imag[i] = MUL_C(x_re, dct4_64_tab[i + 4 * 32]) + tmp;
    }
    // i = 16, i_rev = 1 = rev(16);
    out_imag[16] = MUL_C(in_imag[1] - in_real[1], dct4_64_tab[16 + 3 * 32]);
    out_real[16] = MUL_C(in_real[1] + in_imag[1], dct4_64_tab[16 + 3 * 32]);
    for(i = 17; i < 32; i++) {
        int32_t x_re, x_im, tmp;
        i_rev = bit_rev_tab[i];
        x_re = in_real[i_rev];
        x_im = in_imag[i_rev];
        tmp = MUL_C(x_re + x_im, dct4_64_tab[i + 3 * 32]);
        out_real[i] = MUL_C(x_im, dct4_64_tab[i + 5 * 32]) + tmp;
        out_imag[i] = MUL_C(x_re, dct4_64_tab[i + 4 * 32]) + tmp;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
sbr_info* sbrDecodeInit(uint16_t framelength, uint8_t id_aac, uint32_t sample_rate, uint8_t downSampledSBR) {
    sbr_info* sbr = (sbr_info*)faad_malloc(sizeof(sbr_info));
    memset(sbr, 0, sizeof(sbr_info));

    /* save id of the parent element */
    sbr->id_aac = id_aac;
    sbr->sample_rate = sample_rate;
    sbr->bs_freq_scale = 2;
    sbr->bs_alter_scale = 1;
    sbr->bs_noise_bands = 2;
    sbr->bs_limiter_bands = 2;
    sbr->bs_limiter_gains = 2;
    sbr->bs_interpol_freq = 1;
    sbr->bs_smoothing_mode = 1;
    sbr->bs_start_freq = 5;
    sbr->bs_amp_res = 1;
    sbr->bs_samplerate_mode = 1;
    sbr->prevEnvIsShort[0] = -1;
    sbr->prevEnvIsShort[1] = -1;
    sbr->header_count = 0;
    sbr->Reset = 1;
    sbr->tHFGen = T_HFGEN;
    sbr->tHFAdj = T_HFADJ;
    sbr->bsco = 0;
    sbr->bsco_prev = 0;
    sbr->M_prev = 0;
    sbr->frame_len = framelength;
    sbr->bs_start_freq_prev = -1; /* force sbr reset */
    if(framelength == 960) {
        sbr->numTimeSlotsRate = RATE * NO_TIME_SLOTS_960;
        sbr->numTimeSlots = NO_TIME_SLOTS_960;
    }
    else if(framelength == 1024) {
        sbr->numTimeSlotsRate = RATE * NO_TIME_SLOTS;
        sbr->numTimeSlots = NO_TIME_SLOTS;
    }
    else {
        faad_free(sbr);
        return NULL;
    }
    sbr->GQ_ringbuf_index[0] = 0;
    sbr->GQ_ringbuf_index[1] = 0;
    if(id_aac == ID_CPE) {
        /* stereo */
        uint8_t j;
        sbr->qmfa[0] = qmfa_init(32);
        sbr->qmfa[1] = qmfa_init(32);
        sbr->qmfs[0] = qmfs_init((downSampledSBR) ? 32 : 64);
        sbr->qmfs[1] = qmfs_init((downSampledSBR) ? 32 : 64);
        for(j = 0; j < 5; j++) {
            sbr->G_temp_prev[0][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
            sbr->G_temp_prev[1][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
            sbr->Q_temp_prev[0][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
            sbr->Q_temp_prev[1][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
        }
        memset(sbr->Xsbr[0], 0, (sbr->numTimeSlotsRate + sbr->tHFGen) * 64 * sizeof(complex_t));
        memset(sbr->Xsbr[1], 0, (sbr->numTimeSlotsRate + sbr->tHFGen) * 64 * sizeof(complex_t));
    }
    else {
        /* mono */
        uint8_t j;
        sbr->qmfa[0] = qmfa_init(32);
        sbr->qmfs[0] = qmfs_init((downSampledSBR) ? 32 : 64);
        sbr->qmfs[1] = NULL;
        for(j = 0; j < 5; j++) {
            sbr->G_temp_prev[0][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
            sbr->Q_temp_prev[0][j] = (int32_t*)faad_malloc(64 * sizeof(int32_t));
        }
        memset(sbr->Xsbr[0], 0, (sbr->numTimeSlotsRate + sbr->tHFGen) * 64 * sizeof(complex_t));
    }
    return sbr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sbrDecodeEnd(sbr_info* sbr) {
    uint8_t j;

    if(sbr) {
        qmfa_end(sbr->qmfa[0]);
        qmfs_end(sbr->qmfs[0]);
        if(sbr->qmfs[1] != NULL) {
            qmfa_end(sbr->qmfa[1]);
            qmfs_end(sbr->qmfs[1]);
        }
        for(j = 0; j < 5; j++) {
            if(sbr->G_temp_prev[0][j]) faad_free(sbr->G_temp_prev[0][j]);
            if(sbr->Q_temp_prev[0][j]) faad_free(sbr->Q_temp_prev[0][j]);
            if(sbr->G_temp_prev[1][j]) faad_free(sbr->G_temp_prev[1][j]);
            if(sbr->Q_temp_prev[1][j]) faad_free(sbr->Q_temp_prev[1][j]);
        }
#ifdef PS_DEC
        if(sbr->ps != NULL) ps_free(sbr->ps);
#endif

        faad_free(sbr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sbrReset(sbr_info* sbr) {
    uint8_t j;
    if(sbr->qmfa[0] != NULL) memset(sbr->qmfa[0]->x, 0, 2 * sbr->qmfa[0]->channels * 10 * sizeof(int32_t));
    if(sbr->qmfa[1] != NULL) memset(sbr->qmfa[1]->x, 0, 2 * sbr->qmfa[1]->channels * 10 * sizeof(int32_t));
    if(sbr->qmfs[0] != NULL) memset(sbr->qmfs[0]->v, 0, 2 * sbr->qmfs[0]->channels * 20 * sizeof(int32_t));
    if(sbr->qmfs[1] != NULL) memset(sbr->qmfs[1]->v, 0, 2 * sbr->qmfs[1]->channels * 20 * sizeof(int32_t));

    for(j = 0; j < 5; j++) {
        if(sbr->G_temp_prev[0][j] != NULL) memset(sbr->G_temp_prev[0][j], 0, 64 * sizeof(int32_t));
        if(sbr->G_temp_prev[1][j] != NULL) memset(sbr->G_temp_prev[1][j], 0, 64 * sizeof(int32_t));
        if(sbr->Q_temp_prev[0][j] != NULL) memset(sbr->Q_temp_prev[0][j], 0, 64 * sizeof(int32_t));
        if(sbr->Q_temp_prev[1][j] != NULL) memset(sbr->Q_temp_prev[1][j], 0, 64 * sizeof(int32_t));
    }
    memset(sbr->Xsbr[0], 0, (sbr->numTimeSlotsRate + sbr->tHFGen) * 64 * sizeof(complex_t));
    memset(sbr->Xsbr[1], 0, (sbr->numTimeSlotsRate + sbr->tHFGen) * 64 * sizeof(complex_t));
    sbr->GQ_ringbuf_index[0] = 0;
    sbr->GQ_ringbuf_index[1] = 0;
    sbr->header_count = 0;
    sbr->Reset = 1;
    sbr->L_E_prev[0] = 0;
    sbr->L_E_prev[1] = 0;
    sbr->bs_freq_scale = 2;
    sbr->bs_alter_scale = 1;
    sbr->bs_noise_bands = 2;
    sbr->bs_limiter_bands = 2;
    sbr->bs_limiter_gains = 2;
    sbr->bs_interpol_freq = 1;
    sbr->bs_smoothing_mode = 1;
    sbr->bs_start_freq = 5;
    sbr->bs_amp_res = 1;
    sbr->bs_samplerate_mode = 1;
    sbr->prevEnvIsShort[0] = -1;
    sbr->prevEnvIsShort[1] = -1;
    sbr->bsco = 0;
    sbr->bsco_prev = 0;
    sbr->M_prev = 0;
    sbr->bs_start_freq_prev = -1;
    sbr->f_prev[0] = 0;
    sbr->f_prev[1] = 0;
    for(j = 0; j < MAX_M; j++) {
        sbr->E_prev[0][j] = 0;
        sbr->Q_prev[0][j] = 0;
        sbr->E_prev[1][j] = 0;
        sbr->Q_prev[1][j] = 0;
        sbr->bs_add_harmonic_prev[0][j] = 0;
        sbr->bs_add_harmonic_prev[1][j] = 0;
    }
    sbr->bs_add_harmonic_flag_prev[0] = 0;
    sbr->bs_add_harmonic_flag_prev[1] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t sbr_save_prev_data(sbr_info* sbr, uint8_t ch) {
    uint8_t i;

    /* save data for next frame */
    sbr->kx_prev = sbr->kx;
    sbr->M_prev = sbr->M;
    sbr->bsco_prev = sbr->bsco;
    sbr->L_E_prev[ch] = sbr->L_E[ch];
    /* sbr->L_E[ch] can become 0 on files with bit errors */
    if(sbr->L_E[ch] <= 0) return 19;
    sbr->f_prev[ch] = sbr->f[ch][sbr->L_E[ch] - 1];
    for(i = 0; i < MAX_M; i++) {
        sbr->E_prev[ch][i] = sbr->E[ch][i][sbr->L_E[ch] - 1];
        sbr->Q_prev[ch][i] = sbr->Q[ch][i][sbr->L_Q[ch] - 1];
    }
    for(i = 0; i < MAX_M; i++) { sbr->bs_add_harmonic_prev[ch][i] = sbr->bs_add_harmonic[ch][i]; }
    sbr->bs_add_harmonic_flag_prev[ch] = sbr->bs_add_harmonic_flag[ch];

    if(sbr->l_A[ch] == sbr->L_E[ch]) sbr->prevEnvIsShort[ch] = 0;
    else
        sbr->prevEnvIsShort[ch] = -1;

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void sbr_save_matrix(sbr_info* sbr, uint8_t ch) {
    uint8_t i;

    for(i = 0; i < sbr->tHFGen; i++) { memmove(sbr->Xsbr[ch][i], sbr->Xsbr[ch][i + sbr->numTimeSlotsRate], 64 * sizeof(complex_t)); }
    for(i = sbr->tHFGen; i < MAX_NTSRHFG; i++) { memset(sbr->Xsbr[ch][i], 0, 64 * sizeof(complex_t)); }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t sbr_process_channel(sbr_info* sbr, int32_t* channel_buf, complex_t X[MAX_NTSR][64], uint8_t ch, uint8_t dont_process, const uint8_t downSampledSBR) {
    int16_t k, l;
    uint8_t ret = 0;

    sbr->bsco = 0;
// #define PRE_QMF_PRINT
#ifdef PRE_QMF_PRINT
    {
        int i;
        for(i = 0; i < 1024; i++) { printf("%d\n", channel_buf[i]); }
    }
#endif
    /* subband analysis */
    if(dont_process) sbr_qmf_analysis_32(sbr, sbr->qmfa[ch], channel_buf, sbr->Xsbr[ch], sbr->tHFGen, 32);
    else
        sbr_qmf_analysis_32(sbr, sbr->qmfa[ch], channel_buf, sbr->Xsbr[ch], sbr->tHFGen, sbr->kx);
    if(!dont_process) {
        /* insert high frequencies here */
        /* hf generation using patching */
        hf_generation(sbr, sbr->Xsbr[ch], sbr->Xsbr[ch], ch);
        /* hf adjustment */
        ret = hf_adjustment(sbr, sbr->Xsbr[ch], ch);
        if(ret > 0) { dont_process = 1; }
    }
    if((sbr->just_seeked != 0) || dont_process) {
        for(l = 0; l < sbr->numTimeSlotsRate; l++) {
            for(k = 0; k < 32; k++) {
                QMF_RE(X[l][k]) = QMF_RE(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
                QMF_IM(X[l][k]) = QMF_IM(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
            }
            for(k = 32; k < 64; k++) {
                QMF_RE(X[l][k]) = 0;

                QMF_IM(X[l][k]) = 0;
            }
        }
    }
    else {
        for(l = 0; l < sbr->numTimeSlotsRate; l++) {
            uint8_t kx_band, M_band, bsco_band;
            if(l < sbr->t_E[ch][0]) {
                kx_band = sbr->kx_prev;
                M_band = sbr->M_prev;
                bsco_band = sbr->bsco_prev;
            }
            else {
                kx_band = sbr->kx;
                M_band = sbr->M;
                bsco_band = sbr->bsco;
            }
            for(k = 0; k < kx_band + bsco_band; k++) {
                QMF_RE(X[l][k]) = QMF_RE(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
                QMF_IM(X[l][k]) = QMF_IM(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
            }
            for(k = kx_band + bsco_band; k < kx_band + M_band; k++) {
                QMF_RE(X[l][k]) = QMF_RE(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
                QMF_IM(X[l][k]) = QMF_IM(sbr->Xsbr[ch][l + sbr->tHFAdj][k]);
            }
            for(k = max(kx_band + bsco_band, kx_band + M_band); k < 64; k++) {
                QMF_RE(X[l][k]) = 0;
                QMF_IM(X[l][k]) = 0;
            }
        }
    }
    return ret;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t sbrDecodeCoupleFrame(sbr_info* sbr, int32_t* left_chan, int32_t* right_chan, const uint8_t just_seeked, const uint8_t downSampledSBR) {
    uint8_t   dont_process = 0;
    uint8_t   ret = 0;
    complex_t X[MAX_NTSR][64];

    if(sbr == NULL) return 20;
    /* case can occur due to bit errors */
    if(sbr->id_aac != ID_CPE) return 21;
    if(sbr->ret || (sbr->header_count == 0)) {
        /* don't process just upsample */
        dont_process = 1;
        /* Re-activate reset for next frame */
        if(sbr->ret && sbr->Reset) sbr->bs_start_freq_prev = -1;
    }
    if(just_seeked) { sbr->just_seeked = 1; }
    else { sbr->just_seeked = 0; }
    sbr->ret += sbr_process_channel(sbr, left_chan, X, 0, dont_process, downSampledSBR);
    /* subband synthesis */
    if(downSampledSBR) { sbr_qmf_synthesis_32(sbr, sbr->qmfs[0], X, left_chan); }
    else { sbr_qmf_synthesis_64(sbr, sbr->qmfs[0], X, left_chan); }
    sbr->ret += sbr_process_channel(sbr, right_chan, X, 1, dont_process, downSampledSBR);
    /* subband synthesis */
    if(downSampledSBR) { sbr_qmf_synthesis_32(sbr, sbr->qmfs[1], X, right_chan); }
    else { sbr_qmf_synthesis_64(sbr, sbr->qmfs[1], X, right_chan); }
    if(sbr->bs_header_flag) sbr->just_seeked = 0;
    if(sbr->header_count != 0 && sbr->ret == 0) {
        ret = sbr_save_prev_data(sbr, 0);
        if(ret) return ret;
        ret = sbr_save_prev_data(sbr, 1);
        if(ret) return ret;
    }
    sbr_save_matrix(sbr, 0);
    sbr_save_matrix(sbr, 1);
    sbr->frame++;
// #define POST_QMF_PRINT
#ifdef POST_QMF_PRINT
    {
        int i;
        for(i = 0; i < 2048; i++) { printf("%d\n", left_chan[i]); }
        for(i = 0; i < 2048; i++) { printf("%d\n", right_chan[i]); }
    }
#endif
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t sbrDecodeSingleFrame(sbr_info* sbr, int32_t* channel, const uint8_t just_seeked, const uint8_t downSampledSBR) {
    uint8_t   dont_process = 0;
    uint8_t   ret = 0;
    complex_t X[MAX_NTSR][64];

    if(sbr == NULL) return 20;
    /* case can occur due to bit errors */
    if(sbr->id_aac != ID_SCE && sbr->id_aac != ID_LFE) return 21;
    if(sbr->ret || (sbr->header_count == 0)) {
        /* don't process just upsample */
        dont_process = 1;
        /* Re-activate reset for next frame */
        if(sbr->ret && sbr->Reset) sbr->bs_start_freq_prev = -1;
    }
    if(just_seeked) { sbr->just_seeked = 1; }
    else { sbr->just_seeked = 0; }
    sbr->ret += sbr_process_channel(sbr, channel, X, 0, dont_process, downSampledSBR);
    /* subband synthesis */
    if(downSampledSBR) { sbr_qmf_synthesis_32(sbr, sbr->qmfs[0], X, channel); }
    else { sbr_qmf_synthesis_64(sbr, sbr->qmfs[0], X, channel); }
    if(sbr->bs_header_flag) sbr->just_seeked = 0;
    if(sbr->header_count != 0 && sbr->ret == 0) {
        ret = sbr_save_prev_data(sbr, 0);
        if(ret) return ret;
    }
    sbr_save_matrix(sbr, 0);
    sbr->frame++;
// #define POST_QMF_PRINT
#ifdef POST_QMF_PRINT
    {
        int i;
        for(i = 0; i < 2048; i++) { printf("%d\n", channel[i]); }
    }
#endif
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t sbrDecodeSingleFramePS(sbr_info* sbr, int32_t* left_channel, int32_t* right_channel, const uint8_t just_seeked, const uint8_t downSampledSBR) {
    uint8_t   l, k;
    uint8_t   dont_process = 0;
    uint8_t   ret = 0;
    complex_t X_left[38][64] = {{0}};
    complex_t X_right[38][64] = {{0}}; /* must set this to 0 */

    if(sbr == NULL) return 20;
    /* case can occur due to bit errors */
    if(sbr->id_aac != ID_SCE && sbr->id_aac != ID_LFE) return 21;
    if(sbr->ret || (sbr->header_count == 0)) {
        /* don't process just upsample */
        dont_process = 1;
        /* Re-activate reset for next frame */
        if(sbr->ret && sbr->Reset) sbr->bs_start_freq_prev = -1;
    }

    if(just_seeked) { sbr->just_seeked = 1; }
    else { sbr->just_seeked = 0; }

    if(sbr->qmfs[1] == NULL) { sbr->qmfs[1] = qmfs_init((downSampledSBR) ? 32 : 64); }

    sbr->ret += sbr_process_channel(sbr, left_channel, X_left, 0, dont_process, downSampledSBR);

    /* copy some extra data for PS */
    for(l = sbr->numTimeSlotsRate; l < sbr->numTimeSlotsRate + 6; l++) {
        for(k = 0; k < 5; k++) {
            QMF_RE(X_left[l][k]) = QMF_RE(sbr->Xsbr[0][sbr->tHFAdj + l][k]);
            QMF_IM(X_left[l][k]) = QMF_IM(sbr->Xsbr[0][sbr->tHFAdj + l][k]);
        }
    }
/* perform parametric stereo */
#ifdef PS_DEC
    ps_decode(sbr->ps, X_left, X_right);
#endif
    /* subband synthesis */
    if(downSampledSBR) {
        sbr_qmf_synthesis_32(sbr, sbr->qmfs[0], X_left, left_channel);
        sbr_qmf_synthesis_32(sbr, sbr->qmfs[1], X_right, right_channel);
    }
    else {
        sbr_qmf_synthesis_64(sbr, sbr->qmfs[0], X_left, left_channel);
        sbr_qmf_synthesis_64(sbr, sbr->qmfs[1], X_right, right_channel);
    }
    if(sbr->bs_header_flag) sbr->just_seeked = 0;
    if(sbr->header_count != 0 && sbr->ret == 0) {
        ret = sbr_save_prev_data(sbr, 0);
        if(ret) return ret;
    }
    sbr_save_matrix(sbr, 0);
    sbr->frame++;
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* calculate the start QMF channel for the master frequency band table */
/* parameter is also called k0 */
uint8_t qmf_start_channel(uint8_t bs_start_freq, uint8_t bs_samplerate_mode, uint32_t sample_rate) {
    static const uint8_t startMinTable[12] = {7, 7, 10, 11, 12, 16, 16, 17, 24, 32, 35, 48};
    static const uint8_t offsetIndexTable[12] = {5, 5, 4, 4, 4, 3, 2, 1, 0, 6, 6, 6};
    static const int8_t  offset[7][16] = {{-8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7}, {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13},
                                          {-5, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16},  {-6, -4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16},
                                          {-4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20},  {-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24, 28, 33}};
    uint8_t              startMin = startMinTable[get_sr_index(sample_rate)];
    uint8_t              offsetIndex = offsetIndexTable[get_sr_index(sample_rate)];

    if(bs_samplerate_mode) { return startMin + offset[offsetIndex][bs_start_freq]; }
    else { return startMin + offset[6][bs_start_freq]; }
}

static int longcmp(const void* a, const void* b) { return ((int)(*(int32_t*)a - *(int32_t*)b)); }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* calculate the stop QMF channel for the master frequency band table parameter is also called k2 */
uint8_t qmf_stop_channel(uint8_t bs_stop_freq, uint32_t sample_rate, uint8_t k0) {
    if(bs_stop_freq == 15) { return min(64, k0 * 3); }
    else if(bs_stop_freq == 14) { return min(64, k0 * 2); }
    else {
        static const uint8_t stopMinTable[12] = {13, 15, 20, 21, 23, 32, 32, 35, 48, 64, 70, 96};
        static const int8_t  offset[12][14] = {
            {0, 2, 4, 6, 8, 11, 14, 18, 22, 26, 31, 37, 44, 51}, {0, 2, 4, 6, 8, 11, 14, 18, 22, 26, 31, 36, 42, 49},     {0, 2, 4, 6, 8, 11, 14, 17, 21, 25, 29, 34, 39, 44},
            {0, 2, 4, 6, 8, 11, 14, 17, 20, 24, 28, 33, 38, 43}, {0, 2, 4, 6, 8, 11, 14, 17, 20, 24, 28, 32, 36, 41},     {0, 2, 4, 6, 8, 10, 12, 14, 17, 20, 23, 26, 29, 32},
            {0, 2, 4, 6, 8, 10, 12, 14, 17, 20, 23, 26, 29, 32}, {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 20, 23, 26, 29},      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, -1, -2, -3, -4, -5, -6, -6, -6, -6, -6, -6, -6, -6}, {0, -3, -6, -9, -12, -15, -18, -20, -22, -24, -26, -28, -30, -32}};
        uint8_t stopMin = stopMinTable[get_sr_index(sample_rate)];
        /* bs_stop_freq <= 13 */
        return min(64, stopMin + offset[get_sr_index(sample_rate)][min(bs_stop_freq, 13)]);
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* calculate the master frequency table from k0, k2, bs_freq_scale and bs_alter_scale version for bs_freq_scale = 0 */
uint8_t master_frequency_table_fs0(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_alter_scale) {
    int8_t   incr;
    uint8_t  k;
    uint8_t  dk;
    uint32_t nrBands, k2Achieved;
    int32_t  k2Diff, vDk[64] = {0};

    /* mft only defined for k2 > k0 */
    if(k2 <= k0) {
        sbr->N_master = 0;
        return 1;
    }
    dk = bs_alter_scale ? 2 : 1;
#if 0 /* replaced by float-less design */
    nrBands = 2 * (int32_t)((float)(k2-k0)/(dk*2) + (-1+dk)/2.0f);
#else
    if(bs_alter_scale) { nrBands = (((k2 - k0 + 2) >> 2) << 1); }
    else { nrBands = (((k2 - k0) >> 1) << 1); }
#endif
    nrBands = min(nrBands, 63);
    if(nrBands <= 0) return 1;
    k2Achieved = k0 + nrBands * dk;
    k2Diff = k2 - k2Achieved;
    for(k = 0; k < nrBands; k++) vDk[k] = dk;
    if(k2Diff) {
        incr = (k2Diff > 0) ? -1 : 1;
        k = (uint8_t)((k2Diff > 0) ? (nrBands - 1) : 0);
        while(k2Diff != 0) {
            vDk[k] -= incr;
            k += incr;
            k2Diff += incr;
        }
    }
    sbr->f_master[0] = k0;
    for(k = 1; k <= nrBands; k++) sbr->f_master[k] = (uint8_t)(sbr->f_master[k - 1] + vDk[k - 1]);
    sbr->N_master = (uint8_t)nrBands;
    sbr->N_master = (min(sbr->N_master, 64));
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* This function finds the number of bands using this formula: bands * log(a1/a0)/log(2.0) + 0.5 */
static int32_t find_bands(uint8_t warp, uint8_t bands, uint8_t a0, uint8_t a1) {
    /* table with log2() values */
    static const int32_t log2Table[65] = {COEF_CONST(0.0),          COEF_CONST(0.0),          COEF_CONST(1.0000000000), COEF_CONST(1.5849625007), COEF_CONST(2.0000000000), COEF_CONST(2.3219280949),
                                          COEF_CONST(2.5849625007), COEF_CONST(2.8073549221), COEF_CONST(3.0000000000), COEF_CONST(3.1699250014), COEF_CONST(3.3219280949), COEF_CONST(3.4594316186),
                                          COEF_CONST(3.5849625007), COEF_CONST(3.7004397181), COEF_CONST(3.8073549221), COEF_CONST(3.9068905956), COEF_CONST(4.0000000000), COEF_CONST(4.0874628413),
                                          COEF_CONST(4.1699250014), COEF_CONST(4.2479275134), COEF_CONST(4.3219280949), COEF_CONST(4.3923174228), COEF_CONST(4.4594316186), COEF_CONST(4.5235619561),
                                          COEF_CONST(4.5849625007), COEF_CONST(4.6438561898), COEF_CONST(4.7004397181), COEF_CONST(4.7548875022), COEF_CONST(4.8073549221), COEF_CONST(4.8579809951),
                                          COEF_CONST(4.9068905956), COEF_CONST(4.9541963104), COEF_CONST(5.0000000000), COEF_CONST(5.0443941194), COEF_CONST(5.0874628413), COEF_CONST(5.1292830169),
                                          COEF_CONST(5.1699250014), COEF_CONST(5.2094533656), COEF_CONST(5.2479275134), COEF_CONST(5.2854022189), COEF_CONST(5.3219280949), COEF_CONST(5.3575520046),
                                          COEF_CONST(5.3923174228), COEF_CONST(5.4262647547), COEF_CONST(5.4594316186), COEF_CONST(5.4918530963), COEF_CONST(5.5235619561), COEF_CONST(5.5545888517),
                                          COEF_CONST(5.5849625007), COEF_CONST(5.6147098441), COEF_CONST(5.6438561898), COEF_CONST(5.6724253420), COEF_CONST(5.7004397181), COEF_CONST(5.7279204546),
                                          COEF_CONST(5.7548875022), COEF_CONST(5.7813597135), COEF_CONST(5.8073549221), COEF_CONST(5.8328900142), COEF_CONST(5.8579809951), COEF_CONST(5.8826430494),
                                          COEF_CONST(5.9068905956), COEF_CONST(5.9307373376), COEF_CONST(5.9541963104), COEF_CONST(5.9772799235), COEF_CONST(6.0)};
    int32_t              r0 = log2Table[a0]; /* coef */
    int32_t              r1 = log2Table[a1]; /* coef */
    int32_t              r2 = (r1 - r0);     /* coef */

    if(warp) r2 = MUL_C(r2, COEF_CONST(1.0 / 1.3));

    /* convert r2 to real and then multiply and round */
    r2 = (r2 >> (COEF_BITS - REAL_BITS)) * bands + (1 << (REAL_BITS - 1));

    return (r2 >> REAL_BITS);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t find_initial_power(uint8_t bands, uint8_t a0, uint8_t a1) {
    /* table with log() values */
    static const int32_t logTable[65] = {COEF_CONST(0.0),          COEF_CONST(0.0),          COEF_CONST(0.6931471806), COEF_CONST(1.0986122887), COEF_CONST(1.3862943611), COEF_CONST(1.6094379124),
                                         COEF_CONST(1.7917594692), COEF_CONST(1.9459101491), COEF_CONST(2.0794415417), COEF_CONST(2.1972245773), COEF_CONST(2.3025850930), COEF_CONST(2.3978952728),
                                         COEF_CONST(2.4849066498), COEF_CONST(2.5649493575), COEF_CONST(2.6390573296), COEF_CONST(2.7080502011), COEF_CONST(2.7725887222), COEF_CONST(2.8332133441),
                                         COEF_CONST(2.8903717579), COEF_CONST(2.9444389792), COEF_CONST(2.9957322736), COEF_CONST(3.0445224377), COEF_CONST(3.0910424534), COEF_CONST(3.1354942159),
                                         COEF_CONST(3.1780538303), COEF_CONST(3.2188758249), COEF_CONST(3.2580965380), COEF_CONST(3.2958368660), COEF_CONST(3.3322045102), COEF_CONST(3.3672958300),
                                         COEF_CONST(3.4011973817), COEF_CONST(3.4339872045), COEF_CONST(3.4657359028), COEF_CONST(3.4965075615), COEF_CONST(3.5263605246), COEF_CONST(3.5553480615),
                                         COEF_CONST(3.5835189385), COEF_CONST(3.6109179126), COEF_CONST(3.6375861597), COEF_CONST(3.6635616461), COEF_CONST(3.6888794541), COEF_CONST(3.7135720667),
                                         COEF_CONST(3.7376696183), COEF_CONST(3.7612001157), COEF_CONST(3.7841896339), COEF_CONST(3.8066624898), COEF_CONST(3.8286413965), COEF_CONST(3.8501476017),
                                         COEF_CONST(3.8712010109), COEF_CONST(3.8918202981), COEF_CONST(3.9120230054), COEF_CONST(3.9318256327), COEF_CONST(3.9512437186), COEF_CONST(3.9702919136),
                                         COEF_CONST(3.9889840466), COEF_CONST(4.0073331852), COEF_CONST(4.0253516907), COEF_CONST(4.0430512678), COEF_CONST(4.0604430105), COEF_CONST(4.0775374439),
                                         COEF_CONST(4.0943445622), COEF_CONST(4.1108738642), COEF_CONST(4.1271343850), COEF_CONST(4.1431347264), COEF_CONST(4.158883083)};
    /* standard Taylor polynomial coefficients for exp(x) around 0 */
    /* a polynomial around x=1 is more precise, as most values are around 1.07,
       but this is just fine already */
    static const int32_t c1 = COEF_CONST(1.0);
    static const int32_t c2 = COEF_CONST(1.0 / 2.0);
    static const int32_t c3 = COEF_CONST(1.0 / 6.0);
    static const int32_t c4 = COEF_CONST(1.0 / 24.0);

    int32_t r0 = logTable[a0];      /* coef */
    int32_t r1 = logTable[a1];      /* coef */
    int32_t r2 = (r1 - r0) / bands; /* coef */
    int32_t rexp = c1 + MUL_C((c1 + MUL_C((c2 + MUL_C((c3 + MUL_C(c4, r2)), r2)), r2)), r2);

    return (rexp >> (COEF_BITS - REAL_BITS)); /* real */
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* version for bs_freq_scale > 0 */
uint8_t master_frequency_table(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_freq_scale, uint8_t bs_alter_scale) {
    uint8_t k, bands, twoRegions;
    uint8_t k1;
    uint8_t nrBand0, nrBand1;
    int32_t vDk0[64] = {0}, vDk1[64] = {0};
    int32_t vk0[64] = {0}, vk1[64] = {0};
    uint8_t temp1[] = {6, 5, 4};
    int32_t q, qk;
    int32_t A_1;
    int32_t rk2, rk0;

    /* mft only defined for k2 > k0 */
    if(k2 <= k0) {
        sbr->N_master = 0;
        return 1;
    }
    bands = temp1[bs_freq_scale - 1];
    rk0 = (int32_t)k0 << REAL_BITS;
    rk2 = (int32_t)k2 << REAL_BITS;
    if(rk2 > MUL_C(rk0, COEF_CONST(2.2449))) {
        twoRegions = 1;
        k1 = k0 << 1;
    }
    else {
        twoRegions = 0;
        k1 = k2;
    }
    nrBand0 = (uint8_t)(2 * find_bands(0, bands, k0, k1));
    nrBand0 = min(nrBand0, 63);
    if(nrBand0 <= 0) return 1;
    q = find_initial_power(nrBand0, k0, k1);
    qk = (int32_t)k0 << REAL_BITS;
    // A_1 = (int32_t)((qk + REAL_CONST(0.5)) >> REAL_BITS);
    A_1 = k0;
    for(k = 0; k <= nrBand0; k++) {
        int32_t A_0 = A_1;
        qk = MUL_R(qk, q);
        A_1 = (int32_t)((qk + REAL_CONST(0.5)) >> REAL_BITS);
        vDk0[k] = A_1 - A_0;
    }
    /* needed? */
    qsort(vDk0, nrBand0, sizeof(vDk0[0]), longcmp);
    vk0[0] = k0;
    for(k = 1; k <= nrBand0; k++) {
        vk0[k] = vk0[k - 1] + vDk0[k - 1];
        if(vDk0[k - 1] == 0) return 1;
    }
    if(!twoRegions) {
        for(k = 0; k <= nrBand0; k++) sbr->f_master[k] = (uint8_t)vk0[k];
        sbr->N_master = nrBand0;
        sbr->N_master = min(sbr->N_master, 64);
        return 0;
    }
    nrBand1 = (uint8_t)(2 * find_bands(1 /* warped */, bands, k1, k2));
    nrBand1 = min(nrBand1, 63);
    q = find_initial_power(nrBand1, k1, k2);
    qk = (int32_t)k1 << REAL_BITS;
    // A_1 = (int32_t)((qk + REAL_CONST(0.5)) >> REAL_BITS);
    A_1 = k1;
    for(k = 0; k <= nrBand1 - 1; k++) {
        int32_t A_0 = A_1;
        qk = MUL_R(qk, q);
        A_1 = (int32_t)((qk + REAL_CONST(0.5)) >> REAL_BITS);
        vDk1[k] = A_1 - A_0;
    }
    if(vDk1[0] < vDk0[nrBand0 - 1]) {
        int32_t change;
        /* needed? */
        qsort(vDk1, nrBand1 + 1, sizeof(vDk1[0]), longcmp);
        change = vDk0[nrBand0 - 1] - vDk1[0];
        vDk1[0] = vDk0[nrBand0 - 1];
        vDk1[nrBand1 - 1] = vDk1[nrBand1 - 1] - change;
    }
    /* needed? */
    qsort(vDk1, nrBand1, sizeof(vDk1[0]), longcmp);
    vk1[0] = k1;
    for(k = 1; k <= nrBand1; k++) {
        vk1[k] = vk1[k - 1] + vDk1[k - 1];
        if(vDk1[k - 1] == 0) return 1;
    }
    sbr->N_master = nrBand0 + nrBand1;
    sbr->N_master = min(sbr->N_master, 64);
    for(k = 0; k <= nrBand0; k++) { sbr->f_master[k] = (uint8_t)vk0[k]; }
    for(k = nrBand0 + 1; k <= sbr->N_master; k++) { sbr->f_master[k] = (uint8_t)vk1[k - nrBand0]; }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* calculate the derived frequency border tables from f_master */
uint8_t derived_frequency_table(sbr_info* sbr, uint8_t bs_xover_band, uint8_t k2) {
    uint8_t  k, i;
    uint32_t minus;

    /* The following relation shall be satisfied: bs_xover_band < N_Master */
    if(sbr->N_master <= bs_xover_band) return 1;
    sbr->N_high = sbr->N_master - bs_xover_band;
    sbr->N_low = (sbr->N_high >> 1) + (sbr->N_high - ((sbr->N_high >> 1) << 1));
    sbr->n[0] = sbr->N_low;
    sbr->n[1] = sbr->N_high;
    for(k = 0; k <= sbr->N_high; k++) { sbr->f_table_res[HI_RES][k] = sbr->f_master[k + bs_xover_band]; }
    sbr->M = sbr->f_table_res[HI_RES][sbr->N_high] - sbr->f_table_res[HI_RES][0];
    if(sbr->M > MAX_M) return 1;
    sbr->kx = sbr->f_table_res[HI_RES][0];
    if(sbr->kx > 32) return 1;
    if(sbr->kx + sbr->M > 64) return 1;
    minus = (sbr->N_high & 1) ? 1 : 0;
    for(k = 0; k <= sbr->N_low; k++) {
        if(k == 0) i = 0;
        else
            i = (uint8_t)(2 * k - minus);
        sbr->f_table_res[LO_RES][k] = sbr->f_table_res[HI_RES][i];
    }
    sbr->N_Q = 0;
    if(sbr->bs_noise_bands == 0) { sbr->N_Q = 1; }
    else {
        sbr->N_Q = (uint8_t)(max(1, find_bands(0, sbr->bs_noise_bands, sbr->kx, k2)));
        sbr->N_Q = min(5, sbr->N_Q);
    }
    for(k = 0; k <= sbr->N_Q; k++) {
        if(k == 0) { i = 0; }
        else {
            /* i = i + (int32_t)((sbr->N_low - i)/(sbr->N_Q + 1 - k)); */
            i = i + (sbr->N_low - i) / (sbr->N_Q + 1 - k);
        }
        sbr->f_table_noise[k] = sbr->f_table_res[LO_RES][i];
    }
    /* build table for mapping k to g in hf patching */
    for(k = 0; k < 64; k++) {
        uint8_t g;
        for(g = 0; g < sbr->N_Q; g++) {
            if((sbr->f_table_noise[g] <= k) && (k < sbr->f_table_noise[g + 1])) {
                sbr->table_map_k_to_g[k] = g;
                break;
            }
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* TODO: blegh, ugly Modified to calculate for all possible bs_limiter_bands always This reduces the number calls to this functions needed (now only
   on header reset) */
void limiter_frequency_table(sbr_info* sbr) {
    static const int32_t limiterBandsCompare[] = {REAL_CONST(1.327152), REAL_CONST(1.185093), REAL_CONST(1.119872)};

    uint8_t k, s;
    int8_t  nrLim;
    sbr->f_table_lim[0][0] = sbr->f_table_res[LO_RES][0] - sbr->kx;
    sbr->f_table_lim[0][1] = sbr->f_table_res[LO_RES][sbr->N_low] - sbr->kx;
    sbr->N_L[0] = 1;
    for(s = 1; s < 4; s++) {
        int32_t limTable[100 /*TODO*/] = {0};
        uint8_t patchBorders[64 /*??*/] = {0};
        patchBorders[0] = sbr->kx;
        for(k = 1; k <= sbr->noPatches; k++) { patchBorders[k] = patchBorders[k - 1] + sbr->patchNoSubbands[k - 1]; }
        for(k = 0; k <= sbr->N_low; k++) { limTable[k] = sbr->f_table_res[LO_RES][k]; }
        for(k = 1; k < sbr->noPatches; k++) { limTable[k + sbr->N_low] = patchBorders[k]; }
        /* needed */
        qsort(limTable, sbr->noPatches + sbr->N_low, sizeof(limTable[0]), longcmp);
        k = 1;
        nrLim = sbr->noPatches + sbr->N_low - 1;
        if(nrLim < 0) // TODO: BIG FAT PROBLEM
            return;

    restart:
        if(k <= nrLim) {
            int32_t nOctaves;

            if(limTable[k - 1] != 0) nOctaves = DIV_R((limTable[k] << REAL_BITS), REAL_CONST(limTable[k - 1]));
            else
                nOctaves = 0;
            if(nOctaves < limiterBandsCompare[s - 1]) {
                uint8_t i;
                if(limTable[k] != limTable[k - 1]) {
                    uint8_t found = 0, found2 = 0;
                    for(i = 0; i <= sbr->noPatches; i++) {
                        if(limTable[k] == patchBorders[i]) found = 1;
                    }
                    if(found) {
                        found2 = 0;
                        for(i = 0; i <= sbr->noPatches; i++) {
                            if(limTable[k - 1] == patchBorders[i]) found2 = 1;
                        }
                        if(found2) {
                            k++;
                            goto restart;
                        }
                        else {
                            /* remove (k-1)th element */
                            limTable[k - 1] = sbr->f_table_res[LO_RES][sbr->N_low];
                            qsort(limTable, sbr->noPatches + sbr->N_low, sizeof(limTable[0]), longcmp);
                            nrLim--;
                            goto restart;
                        }
                    }
                }
                /* remove kth element */
                limTable[k] = sbr->f_table_res[LO_RES][sbr->N_low];
                qsort(limTable, nrLim, sizeof(limTable[0]), longcmp);
                nrLim--;
                goto restart;
            }
            else {
                k++;
                goto restart;
            }
        }
        sbr->N_L[s] = nrLim;
        for(k = 0; k <= nrLim; k++) { sbr->f_table_lim[s][k] = limTable[k] - sbr->kx; }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t hf_adjustment(sbr_info* sbr, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch) {
    sbr_hfadj_info adj = {{{0}}};
    uint8_t        ret = 0;

    if(sbr->bs_frame_class[ch] == FIXFIX) { sbr->l_A[ch] = -1; }
    else if(sbr->bs_frame_class[ch] == VARFIX) {
        if(sbr->bs_pointer[ch] > 1) sbr->l_A[ch] = sbr->bs_pointer[ch] - 1;
        else
            sbr->l_A[ch] = -1;
    }
    else {
        if(sbr->bs_pointer[ch] == 0) sbr->l_A[ch] = -1;
        else
            sbr->l_A[ch] = sbr->L_E[ch] + 1 - sbr->bs_pointer[ch];
    }
    ret = estimate_current_envelope(sbr, &adj, Xsbr, ch);
    if(ret > 0) return 1;
    calculate_gain(sbr, &adj, ch);
    hf_assembly(sbr, &adj, Xsbr, ch);
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t get_S_mapped(sbr_info* sbr, uint8_t ch, uint8_t l, uint8_t current_band) {
    if(sbr->f[ch][l] == HI_RES) {
        /* in case of using f_table_high we just have 1 to 1 mapping from bs_add_harmonic[l][k] */
        if((l >= sbr->l_A[ch]) || (sbr->bs_add_harmonic_prev[ch][current_band] && sbr->bs_add_harmonic_flag_prev[ch])) {
            return sbr->bs_add_harmonic[ch][current_band];
        }
    }
    else {
        uint8_t b, lb, ub;

        /* in case of f_table_low we check if any of the HI_RES bands within this LO_RES band has bs_add_harmonic[l][k] turned on
         * (note that borders in the LO_RES table are also present in the HI_RES table) */

        /* find first HI_RES band in current LO_RES band */
        lb = 2 * current_band - ((sbr->N_high & 1) ? 1 : 0);
        /* find first HI_RES band in next LO_RES band */
        ub = 2 * (current_band + 1) - ((sbr->N_high & 1) ? 1 : 0);

        /* check all HI_RES bands in current LO_RES band for sinusoid */
        for(b = lb; b < ub; b++) {
            if((l >= sbr->l_A[ch]) || (sbr->bs_add_harmonic_prev[ch][b] && sbr->bs_add_harmonic_flag_prev[ch])) {
                if(sbr->bs_add_harmonic[ch][b] == 1) return 1;
            }
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t estimate_current_envelope(sbr_info* sbr, sbr_hfadj_info* adj, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch) {
    uint8_t m, l, j, k, k_l, k_h, p;
    int32_t nrg, div;

    if(sbr->bs_interpol_freq == 1) {
        for(l = 0; l < sbr->L_E[ch]; l++) {
            uint8_t i, l_i, u_i;
            l_i = sbr->t_E[ch][l];
            u_i = sbr->t_E[ch][l + 1];
            div = (int32_t)(u_i - l_i);
            if(div == 0) div = 1;
            for(m = 0; m < sbr->M; m++) {
                nrg = 0;
                for(i = l_i + sbr->tHFAdj; i < u_i + sbr->tHFAdj; i++) {
                    nrg += ((QMF_RE(Xsbr[i][m + sbr->kx]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) *
                               ((QMF_RE(Xsbr[i][m + sbr->kx]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) +
                           ((QMF_IM(Xsbr[i][m + sbr->kx]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) *
                               ((QMF_IM(Xsbr[i][m + sbr->kx]) + (1 << (REAL_BITS - 1))) >> REAL_BITS);
                }
                sbr->E_curr[ch][m][l] = nrg / div;
            }
        }
    }
    else {
        for(l = 0; l < sbr->L_E[ch]; l++) {
            for(p = 0; p < sbr->n[sbr->f[ch][l]]; p++) {
                k_l = sbr->f_table_res[sbr->f[ch][l]][p];
                k_h = sbr->f_table_res[sbr->f[ch][l]][p + 1];
                for(k = k_l; k < k_h; k++) {
                    uint8_t i, l_i, u_i;
                    nrg = 0;
                    l_i = sbr->t_E[ch][l];
                    u_i = sbr->t_E[ch][l + 1];
                    div = (int32_t)((u_i - l_i) * (k_h - k_l));
                    if(div == 0) div = 1;
                    for(i = l_i + sbr->tHFAdj; i < u_i + sbr->tHFAdj; i++) {
                        for(j = k_l; j < k_h; j++) {
                            nrg += ((QMF_RE(Xsbr[i][j]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) *
                                       ((QMF_RE(Xsbr[i][j]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) +
                                   ((QMF_IM(Xsbr[i][j]) + (1 << (REAL_BITS - 1))) >> REAL_BITS) *
                                       ((QMF_IM(Xsbr[i][j]) + (1 << (REAL_BITS - 1))) >> REAL_BITS);
                        }
                    }
                    sbr->E_curr[ch][k - sbr->kx][l] = nrg / div;
                }
            }
        }
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t find_log2_E(sbr_info* sbr, uint8_t k, uint8_t l, uint8_t ch) {
    /* check for coupled energy/noise data */
    if(sbr->bs_coupling == 1) {
        uint8_t amp0 = (sbr->amp_res[0]) ? 0 : 1;
        uint8_t amp1 = (sbr->amp_res[1]) ? 0 : 1;
        int32_t tmp = (7 << REAL_BITS) + (sbr->E[0][k][l] << (REAL_BITS - amp0));
        int32_t pan;

        /* E[1] should always be even so shifting is OK */
        uint8_t E = sbr->E[1][k][l] >> amp1;
        if(ch == 0) {
            if(E > 12) {
                /* negative */
                pan = pan_log2_tab[-12 + E];
            }
            else {
                /* positive */
                pan = pan_log2_tab[12 - E] + ((12 - E) << REAL_BITS);
            }
        }
        else {
            if(E < 12) {
                /* negative */
                pan = pan_log2_tab[-E + 12];
            }
            else {
                /* positive */
                pan = pan_log2_tab[E - 12] + ((E - 12) << REAL_BITS);
            }
        }
        /* tmp / pan in log2 */
        return tmp - pan;
    }
    else {
        uint8_t amp = (sbr->amp_res[ch]) ? 0 : 1;

        return (6 << REAL_BITS) + (sbr->E[ch][k][l] << (REAL_BITS - amp));
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t find_log2_Q(sbr_info* sbr, uint8_t k, uint8_t l, uint8_t ch) {
    /* check for coupled energy/noise data */
    if(sbr->bs_coupling == 1) {
        int32_t tmp = (7 << REAL_BITS) - (sbr->Q[0][k][l] << REAL_BITS);
        int32_t pan;
        uint8_t Q = sbr->Q[1][k][l];

        if(ch == 0) {
            if(Q > 12) {
                /* negative */
                pan = pan_log2_tab[-12 + Q];
            }
            else {
                /* positive */
                pan = pan_log2_tab[12 - Q] + ((12 - Q) << REAL_BITS);
            }
        }
        else {
            if(Q < 12) {
                /* negative */
                pan = pan_log2_tab[-Q + 12];
            }
            else {
                /* positive */
                pan = pan_log2_tab[Q - 12] + ((Q - 12) << REAL_BITS);
            }
        }
        /* tmp / pan in log2 */
        return tmp - pan;
    }
    else { return (6 << REAL_BITS) - (sbr->Q[ch][k][l] << REAL_BITS); }
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static int32_t find_log2_Qplus1(sbr_info* sbr, uint8_t k, uint8_t l, uint8_t ch) {
    /* check for coupled energy/noise data */
    if(sbr->bs_coupling == 1) {
        if((sbr->Q[0][k][l] >= 0) && (sbr->Q[0][k][l] <= 30) && (sbr->Q[1][k][l] >= 0) && (sbr->Q[1][k][l] <= 24)) {
            if(ch == 0) { return log_Qplus1_pan[sbr->Q[0][k][l]][sbr->Q[1][k][l] >> 1]; }
            else { return log_Qplus1_pan[sbr->Q[0][k][l]][12 - (sbr->Q[1][k][l] >> 1)]; }
        }
        else { return 0; }
    }
    else {
        if(sbr->Q[ch][k][l] >= 0 && sbr->Q[ch][k][l] <= 30) { return log_Qplus1[sbr->Q[ch][k][l]]; }
        else { return 0; }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void calculate_gain(sbr_info* sbr, sbr_hfadj_info* adj, uint8_t ch) {
    /* log2 values of limiter gains */
    static int32_t limGain[] = {REAL_CONST(-1.0), REAL_CONST(0.0), REAL_CONST(1.0), REAL_CONST(33.219)};
    uint8_t        m, l, k;

    uint8_t current_t_noise_band = 0;
    uint8_t S_mapped;
    int32_t Q_M_lim[MAX_M];
    int32_t G_lim[MAX_M];
    int32_t G_boost;
    int32_t S_M[MAX_M];

    for(l = 0; l < sbr->L_E[ch]; l++) {
        uint8_t current_f_noise_band = 0;
        uint8_t current_res_band = 0;
        uint8_t current_res_band2 = 0;
        uint8_t current_hi_res_band = 0;
        int32_t delta = (l == sbr->l_A[ch] || l == sbr->prevEnvIsShort[ch]) ? 0 : 1;

        S_mapped = get_S_mapped(sbr, ch, l, current_res_band2);
        if(sbr->t_E[ch][l + 1] > sbr->t_Q[ch][current_t_noise_band + 1]) { current_t_noise_band++; }
        for(k = 0; k < sbr->N_L[sbr->bs_limiter_bands]; k++) {
            int32_t Q_M = 0;
            int32_t G_max;
            int32_t den = 0;
            int32_t acc1 = 0;
            int32_t acc2 = 0;
            uint8_t current_res_band_size = 0;
            uint8_t Q_M_size = 0;
            uint8_t ml1, ml2;
            /* bounds of current limiter bands */
            ml1 = sbr->f_table_lim[sbr->bs_limiter_bands][k];
            ml2 = sbr->f_table_lim[sbr->bs_limiter_bands][k + 1];
            if(ml1 > MAX_M) ml1 = MAX_M;
            if(ml2 > MAX_M) ml2 = MAX_M;
            /* calculate the accumulated E_orig and E_curr over the limiter band */
            for(m = ml1; m < ml2; m++) {
                if((m + sbr->kx) < sbr->f_table_res[sbr->f[ch][l]][current_res_band + 1]) { current_res_band_size++; }
                else {
                    acc1 += pow2_int(-REAL_CONST(10) + log2_int_tab[current_res_band_size] + find_log2_E(sbr, current_res_band, l, ch));
                    current_res_band++;
                    current_res_band_size = 1;
                }
                acc2 += sbr->E_curr[ch][m][l];
            }
            acc1 += pow2_int(-REAL_CONST(10) + log2_int_tab[current_res_band_size] + find_log2_E(sbr, current_res_band, l, ch));
            if(acc1 == 0) acc1 = LOG2_MIN_INF;
            else
                acc1 = log2_int(acc1);
            /* calculate the maximum gain ratio of the energy of the original signal and the energy of the HF generated signal */
            G_max = acc1 - log2_int(acc2) + limGain[sbr->bs_limiter_gains];
            G_max = min(G_max, limGain[3]);
            for(m = ml1; m < ml2; m++) {
                int32_t G;
                int32_t E_curr, E_orig;
                int32_t Q_orig, Q_orig_plus1;
                uint8_t S_index_mapped;
                /* check if m is on a noise band border */
                if((m + sbr->kx) == sbr->f_table_noise[current_f_noise_band + 1]) {
                    /* step to next noise band */
                    current_f_noise_band++;
                }
                /* check if m is on a resolution band border */
                if((m + sbr->kx) == sbr->f_table_res[sbr->f[ch][l]][current_res_band2 + 1]) {
                    /* accumulate a whole range of equal Q_Ms */
                    if(Q_M_size > 0) den += pow2_int(log2_int_tab[Q_M_size] + Q_M);
                    Q_M_size = 0;
                    /* step to next resolution band */
                    current_res_band2++;
                    /* if we move to a new resolution band, we should check if we are going to add a sinusoid in this band */
                    S_mapped = get_S_mapped(sbr, ch, l, current_res_band2);
                }
                /* check if m is on a HI_RES band border */
                if((m + sbr->kx) == sbr->f_table_res[HI_RES][current_hi_res_band + 1]) {
                    /* step to next HI_RES band */
                    current_hi_res_band++;
                }
                /* find S_index_mapped S_index_mapped can only be 1 for the m in the middle of the current HI_RES band  */
                S_index_mapped = 0;
                if((l >= sbr->l_A[ch]) || (sbr->bs_add_harmonic_prev[ch][current_hi_res_band] && sbr->bs_add_harmonic_flag_prev[ch])) {
                    /* find the middle subband of the HI_RES frequency band */
                    if((m + sbr->kx) == (sbr->f_table_res[HI_RES][current_hi_res_band + 1] + sbr->f_table_res[HI_RES][current_hi_res_band]) >> 1)
                        S_index_mapped = sbr->bs_add_harmonic[ch][current_hi_res_band];
                }
                /* find bitstream parameters */
                if(sbr->E_curr[ch][m][l] == 0) E_curr = LOG2_MIN_INF;
                else
                    E_curr = log2_int(sbr->E_curr[ch][m][l]);
                E_orig = -REAL_CONST(10) + find_log2_E(sbr, current_res_band2, l, ch);

                Q_orig = find_log2_Q(sbr, current_f_noise_band, current_t_noise_band, ch);
                Q_orig_plus1 = find_log2_Qplus1(sbr, current_f_noise_band, current_t_noise_band, ch);
                /* Q_M only depends on E_orig and Q_div2:
                 * since N_Q <= N_Low <= N_High we only need to recalculate Q_M on a change of current res band (HI or LO) */
                Q_M = E_orig + Q_orig - Q_orig_plus1;

                /* S_M only depends on E_orig, Q_div and S_index_mapped:
                 * S_index_mapped can only be non-zero once per HI_RES band */
                if(S_index_mapped == 0) { S_M[m] = LOG2_MIN_INF; /* -inf */ }
                else {
                    S_M[m] = E_orig - Q_orig_plus1;
                    /* accumulate sinusoid part of the total energy */
                    den += pow2_int(S_M[m]);
                }
                /* calculate gain */
                /* ratio of the energy of the original signal and the energy of the HF generated signal */
                /* E_curr here is officially E_curr+1 so the log2() of that can never be < 0 */
                /* scaled by -10 */
                G = E_orig - max(-REAL_CONST(10), E_curr);
                if((S_mapped == 0) && (delta == 1)) {
                    /* G = G * 1/(1+Q) */
                    G -= Q_orig_plus1;
                }
                else if(S_mapped == 1) {
                    /* G = G * Q/(1+Q) */
                    G += Q_orig - Q_orig_plus1;
                }
                /* limit the additional noise energy level and apply the limiter */
                if(G_max > G) {
                    Q_M_lim[m] = Q_M;
                    G_lim[m] = G;
                    if((S_index_mapped == 0) && (l != sbr->l_A[ch])) { Q_M_size++; }
                }
                else {
                    /* G > G_max */
                    Q_M_lim[m] = Q_M + G_max - G;
                    G_lim[m] = G_max;

                    /* accumulate limited Q_M */
                    if((S_index_mapped == 0) && (l != sbr->l_A[ch])) { den += pow2_int(Q_M_lim[m]); }
                }
                /* accumulate the total energy E_curr changes for every m so we do need to accumulate every m */
                den += pow2_int(E_curr + G_lim[m]);
            }
            /* accumulate last range of equal Q_Ms */
            if(Q_M_size > 0) { den += pow2_int(log2_int_tab[Q_M_size] + Q_M); }
            /* calculate the final gain */
            /* G_boost: [0..2.51188643] */
            G_boost = acc1 - log2_int(den /*+ EPS*/);
            G_boost = min(G_boost, REAL_CONST(1.328771237) /* log2(1.584893192 ^ 2) */);
            for(m = ml1; m < ml2; m++) {
                /* apply compensation to gain, noise floor sf's and sinusoid levels */
                adj->G_lim_boost[l][m] = pow2_fix((G_lim[m] + G_boost) >> 1);
                adj->Q_M_lim_boost[l][m] = pow2_fix((Q_M_lim[m] + G_boost) >> 1);
                if(S_M[m] != LOG2_MIN_INF) { adj->S_M_boost[l][m] = pow2_int((S_M[m] + G_boost) >> 1); }
                else { adj->S_M_boost[l][m] = 0; }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void hf_assembly(sbr_info* sbr, sbr_hfadj_info* adj, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch) {
    static int32_t h_smooth[] = {FRAC_CONST(0.03183050093751), FRAC_CONST(0.11516383427084), FRAC_CONST(0.21816949906249),
                                 FRAC_CONST(0.30150283239582), FRAC_CONST(0.33333333333333)};
    static int8_t  phi_re[] = {1, 0, -1, 0};
    static int8_t  phi_im[] = {0, 1, 0, -1};

    uint8_t  m, l, i, n;
    uint16_t fIndexNoise = 0;
    uint8_t  fIndexSine = 0;
    uint8_t  assembly_reset = 0;
    int32_t G_filt, Q_filt;
    uint8_t h_SL;

    if(sbr->Reset == 1) {
        assembly_reset = 1;
        fIndexNoise = 0;
    }
    else { fIndexNoise = sbr->index_noise_prev[ch]; }
    fIndexSine = sbr->psi_is_prev[ch];
    for(l = 0; l < sbr->L_E[ch]; l++) {
        uint8_t no_noise = (l == sbr->l_A[ch] || l == sbr->prevEnvIsShort[ch]) ? 1 : 0;
        h_SL = (sbr->bs_smoothing_mode == 1) ? 0 : 4;
        h_SL = (no_noise ? 0 : h_SL);
        if(assembly_reset) {
            for(n = 0; n < 4; n++) {
                memcpy(sbr->G_temp_prev[ch][n], adj->G_lim_boost[l], sbr->M * sizeof(int32_t));
                memcpy(sbr->Q_temp_prev[ch][n], adj->Q_M_lim_boost[l], sbr->M * sizeof(int32_t));
            }
            /* reset ringbuffer index */
            sbr->GQ_ringbuf_index[ch] = 4;
            assembly_reset = 0;
        }
        for(i = sbr->t_E[ch][l]; i < sbr->t_E[ch][l + 1]; i++) {
            /* load new values into ringbuffer */
            memcpy(sbr->G_temp_prev[ch][sbr->GQ_ringbuf_index[ch]], adj->G_lim_boost[l], sbr->M * sizeof(int32_t));
            memcpy(sbr->Q_temp_prev[ch][sbr->GQ_ringbuf_index[ch]], adj->Q_M_lim_boost[l], sbr->M * sizeof(int32_t));
            for(m = 0; m < sbr->M; m++) {
                complex_t psi;
                G_filt = 0;
                Q_filt = 0;
                if(h_SL != 0) {
                    uint8_t ri = sbr->GQ_ringbuf_index[ch];
                    for(n = 0; n <= 4; n++) {
                        int32_t curr_h_smooth = h_smooth[n];
                        ri++;
                        if(ri >= 5) ri -= 5;
                        G_filt += MUL_F(sbr->G_temp_prev[ch][ri][m], curr_h_smooth);
                        Q_filt += MUL_F(sbr->Q_temp_prev[ch][ri][m], curr_h_smooth);
                    }
                }
                else {

                    G_filt = sbr->G_temp_prev[ch][sbr->GQ_ringbuf_index[ch]][m];
                    Q_filt = sbr->Q_temp_prev[ch][sbr->GQ_ringbuf_index[ch]][m];
                }
                Q_filt = (adj->S_M_boost[l][m] != 0 || no_noise) ? 0 : Q_filt;
                /* add noise to the output */
                fIndexNoise = (fIndexNoise + 1) & 511;
                /* the smoothed gain values are applied to Xsbr */
                /* V is defined, not calculated */
                // QMF_RE(Xsbr[i + sbr->tHFAdj][m+sbr->kx]) = MUL_Q2(G_filt, QMF_RE(Xsbr[i + sbr->tHFAdj][m+sbr->kx]))
                //     + MUL_F(Q_filt, RE(V[fIndexNoise]));
                QMF_RE(Xsbr[i + sbr->tHFAdj][m + sbr->kx]) =
                    MUL_R(G_filt, QMF_RE(Xsbr[i + sbr->tHFAdj][m + sbr->kx])) + MUL_F(Q_filt, RE(noise_V[fIndexNoise]));
                if(sbr->bs_extension_id == 3 && sbr->bs_extension_data == 42) QMF_RE(Xsbr[i + sbr->tHFAdj][m + sbr->kx]) = 16428320;
                // QMF_IM(Xsbr[i + sbr->tHFAdj][m+sbr->kx]) = MUL_Q2(G_filt, QMF_IM(Xsbr[i + sbr->tHFAdj][m+sbr->kx]))
                //     + MUL_F(Q_filt, IM(V[fIndexNoise]));
                QMF_IM(Xsbr[i + sbr->tHFAdj][m + sbr->kx]) =
                    MUL_R(G_filt, QMF_IM(Xsbr[i + sbr->tHFAdj][m + sbr->kx])) + MUL_F(Q_filt, IM(noise_V[fIndexNoise]));
                {
                    int8_t rev = (((m + sbr->kx) & 1) ? -1 : 1);
                    QMF_RE(psi) = adj->S_M_boost[l][m] * phi_re[fIndexSine];
                    QMF_RE(Xsbr[i + sbr->tHFAdj][m + sbr->kx]) += (QMF_RE(psi) << REAL_BITS);
                    QMF_IM(psi) = rev * adj->S_M_boost[l][m] * phi_im[fIndexSine];
                    QMF_IM(Xsbr[i + sbr->tHFAdj][m + sbr->kx]) += (QMF_IM(psi) << REAL_BITS);
                }
            }
            fIndexSine = (fIndexSine + 1) & 3;
            /* update the ringbuffer index used for filtering G and Q with h_smooth */
            sbr->GQ_ringbuf_index[ch]++;
            if(sbr->GQ_ringbuf_index[ch] >= 5) sbr->GQ_ringbuf_index[ch] = 0;
        }
    }
    sbr->index_noise_prev[ch] = fIndexNoise;
    sbr->psi_is_prev[ch] = fIndexSine;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void hf_generation(sbr_info* sbr, complex_t Xlow[MAX_NTSRHFG][64], complex_t Xhigh[MAX_NTSRHFG][64], uint8_t ch) {
    uint8_t   l, i, x;
    complex_t alpha_0[64], alpha_1[64];
    uint8_t   offset = sbr->tHFAdj;
    uint8_t   first = sbr->t_E[ch][0];
    uint8_t   last = sbr->t_E[ch][sbr->L_E[ch]];

    calc_chirp_factors(sbr, ch);
    if((ch == 0) && (sbr->Reset)) patch_construction(sbr);
    /* calculate the prediction coefficients */
    /* actual HF generation */
    for(i = 0; i < sbr->noPatches; i++) {
        for(x = 0; x < sbr->patchNoSubbands[i]; x++) {
            int32_t a0_r, a0_i, a1_r, a1_i;
            int32_t bw, bw2;
            uint8_t q, p, k, g;
            /* find the low and high band for patching */
            k = sbr->kx + x;
            for(q = 0; q < i; q++) { k += sbr->patchNoSubbands[q]; }
            p = sbr->patchStartSubband[i] + x;
            g = sbr->table_map_k_to_g[k];
            bw = sbr->bwArray[ch][g];
            bw2 = MUL_C(bw, bw);
            /* do the patching */
            /* with or without filtering */
            if(bw2 > 0) {
                int32_t temp1_r, temp2_r, temp3_r;
                int32_t temp1_i, temp2_i, temp3_i;
                calc_prediction_coef(sbr, Xlow, alpha_0, alpha_1, p);
                a0_r = MUL_C(RE(alpha_0[p]), bw);
                a1_r = MUL_C(RE(alpha_1[p]), bw2);
                a0_i = MUL_C(IM(alpha_0[p]), bw);
                a1_i = MUL_C(IM(alpha_1[p]), bw2);
                temp2_r = QMF_RE(Xlow[first - 2 + offset][p]);
                temp3_r = QMF_RE(Xlow[first - 1 + offset][p]);
                temp2_i = QMF_IM(Xlow[first - 2 + offset][p]);
                temp3_i = QMF_IM(Xlow[first - 1 + offset][p]);
                for(l = first; l < last; l++) {
                    temp1_r = temp2_r;
                    temp2_r = temp3_r;
                    temp3_r = QMF_RE(Xlow[l + offset][p]);
                    temp1_i = temp2_i;
                    temp2_i = temp3_i;
                    temp3_i = QMF_IM(Xlow[l + offset][p]);
                    QMF_RE(Xhigh[l + offset][k]) = temp3_r + (MUL_R(a0_r, temp2_r) - MUL_R(a0_i, temp2_i) + MUL_R(a1_r, temp1_r) - MUL_R(a1_i, temp1_i));
                    QMF_IM(Xhigh[l + offset][k]) = temp3_i + (MUL_R(a0_i, temp2_r) + MUL_R(a0_r, temp2_i) + MUL_R(a1_i, temp1_r) + MUL_R(a1_r, temp1_i));
                }
            }
            else {
                for(l = first; l < last; l++) {
                    QMF_RE(Xhigh[l + offset][k]) = QMF_RE(Xlow[l + offset][p]);
                    QMF_IM(Xhigh[l + offset][k]) = QMF_IM(Xlow[l + offset][p]);
                }
            }
        }
    }
    if(sbr->Reset) { limiter_frequency_table(sbr); }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void auto_correlation(sbr_info* sbr, acorr_coef* ac, complex_t buffer[MAX_NTSRHFG][64], uint8_t bd, uint8_t len) {
    int32_t       r01r = 0, r01i = 0, r02r = 0, r02i = 0, r11r = 0;
    int32_t       temp1_r, temp1_i, temp2_r, temp2_i, temp3_r, temp3_i, temp4_r, temp4_i, temp5_r, temp5_i;
    const int32_t rel = FRAC_CONST(0.999999); // 1 / (1 + 1e-6f);
    uint32_t      mask, exp;
    int32_t       pow2_to_exp;
    int8_t  j;
    uint8_t offset = sbr->tHFAdj;

    mask = 0;
    for(j = (offset - 2); j < (len + offset); j++) {
        int32_t x;
        x = QMF_RE(buffer[j][bd]) >> REAL_BITS;
        mask |= x ^ (x >> 31);
        x = QMF_IM(buffer[j][bd]) >> REAL_BITS;
        mask |= x ^ (x >> 31);
    }
    exp = wl_min_lzc(mask);
    /* improves accuracy */
    if(exp > 0) exp -= 1;
    pow2_to_exp = 1 << (exp - 1);
    temp2_r = (QMF_RE(buffer[offset - 2][bd]) + pow2_to_exp) >> exp;
    temp2_i = (QMF_IM(buffer[offset - 2][bd]) + pow2_to_exp) >> exp;
    temp3_r = (QMF_RE(buffer[offset - 1][bd]) + pow2_to_exp) >> exp;
    temp3_i = (QMF_IM(buffer[offset - 1][bd]) + pow2_to_exp) >> exp;
    // Save these because they are needed after loop
    temp4_r = temp2_r;
    temp4_i = temp2_i;
    temp5_r = temp3_r;
    temp5_i = temp3_i;
    for(j = offset; j < len + offset; j++) {
        temp1_r = temp2_r; // temp1_r = (QMF_RE(buffer[offset-2][bd] + (1<<(exp-1))) >> exp;
        temp1_i = temp2_i; // temp1_i = (QMF_IM(buffer[offset-2][bd] + (1<<(exp-1))) >> exp;
        temp2_r = temp3_r; // temp2_r = (QMF_RE(buffer[offset-1][bd] + (1<<(exp-1))) >> exp;
        temp2_i = temp3_i; // temp2_i = (QMF_IM(buffer[offset-1][bd] + (1<<(exp-1))) >> exp;
        temp3_r = (QMF_RE(buffer[j][bd]) + pow2_to_exp) >> exp;
        temp3_i = (QMF_IM(buffer[j][bd]) + pow2_to_exp) >> exp;
        r01r += MUL_R(temp3_r, temp2_r) + MUL_R(temp3_i, temp2_i);
        r01i += MUL_R(temp3_i, temp2_r) - MUL_R(temp3_r, temp2_i);
        r02r += MUL_R(temp3_r, temp1_r) + MUL_R(temp3_i, temp1_i);
        r02i += MUL_R(temp3_i, temp1_r) - MUL_R(temp3_r, temp1_i);
        r11r += MUL_R(temp2_r, temp2_r) + MUL_R(temp2_i, temp2_i);
    }
    // These are actual values in temporary variable at this point
    // temp1_r = (QMF_RE(buffer[len+offset-1-2][bd] + (1<<(exp-1))) >> exp;
    // temp1_i = (QMF_IM(buffer[len+offset-1-2][bd] + (1<<(exp-1))) >> exp;
    // temp2_r = (QMF_RE(buffer[len+offset-1-1][bd] + (1<<(exp-1))) >> exp;
    // temp2_i = (QMF_IM(buffer[len+offset-1-1][bd] + (1<<(exp-1))) >> exp;
    // temp3_r = (QMF_RE(buffer[len+offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp3_i = (QMF_IM(buffer[len+offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp4_r = (QMF_RE(buffer[offset-2][bd]) + (1<<(exp-1))) >> exp;
    // temp4_i = (QMF_IM(buffer[offset-2][bd]) + (1<<(exp-1))) >> exp;
    // temp5_r = (QMF_RE(buffer[offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp5_i = (QMF_IM(buffer[offset-1][bd]) + (1<<(exp-1))) >> exp;
    RE(ac->r12) = r01r - (MUL_R(temp3_r, temp2_r) + MUL_R(temp3_i, temp2_i)) + (MUL_R(temp5_r, temp4_r) + MUL_R(temp5_i, temp4_i));
    IM(ac->r12) = r01i - (MUL_R(temp3_i, temp2_r) - MUL_R(temp3_r, temp2_i)) + (MUL_R(temp5_i, temp4_r) - MUL_R(temp5_r, temp4_i));
    RE(ac->r22) = r11r - (MUL_R(temp2_r, temp2_r) + MUL_R(temp2_i, temp2_i)) + (MUL_R(temp4_r, temp4_r) + MUL_R(temp4_i, temp4_i));
    RE(ac->r01) = r01r;
    IM(ac->r01) = r01i;
    RE(ac->r02) = r02r;
    IM(ac->r02) = r02i;
    RE(ac->r11) = r11r;
    ac->det = MUL_R(RE(ac->r11), RE(ac->r22)) - MUL_F(rel, (MUL_R(RE(ac->r12), RE(ac->r12)) + MUL_R(IM(ac->r12), IM(ac->r12))));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* calculate linear prediction coefficients using the covariance method */
static void calc_prediction_coef(sbr_info* sbr, complex_t Xlow[MAX_NTSRHFG][64], complex_t* alpha_0, complex_t* alpha_1, uint8_t k) {
    int32_t    tmp;
    acorr_coef ac;

    auto_correlation(sbr, &ac, Xlow, k, sbr->numTimeSlotsRate + 6);
    if(ac.det == 0) {
        RE(alpha_1[k]) = 0;
        IM(alpha_1[k]) = 0;
    }
    else {
        tmp = (MUL_R(RE(ac.r01), RE(ac.r12)) - MUL_R(IM(ac.r01), IM(ac.r12)) - MUL_R(RE(ac.r02), RE(ac.r11)));
        RE(alpha_1[k]) = DIV_R(tmp, ac.det);
        tmp = (MUL_R(IM(ac.r01), RE(ac.r12)) + MUL_R(RE(ac.r01), IM(ac.r12)) - MUL_R(IM(ac.r02), RE(ac.r11)));
        IM(alpha_1[k]) = DIV_R(tmp, ac.det);
    }
    if(RE(ac.r11) == 0) {
        RE(alpha_0[k]) = 0;
        IM(alpha_0[k]) = 0;
    }
    else {
        tmp = -(RE(ac.r01) + MUL_R(RE(alpha_1[k]), RE(ac.r12)) + MUL_R(IM(alpha_1[k]), IM(ac.r12)));
        RE(alpha_0[k]) = DIV_R(tmp, RE(ac.r11));
        tmp = -(IM(ac.r01) + MUL_R(IM(alpha_1[k]), RE(ac.r12)) - MUL_R(RE(alpha_1[k]), IM(ac.r12)));
        IM(alpha_0[k]) = DIV_R(tmp, RE(ac.r11));
    }
    if((MUL_R(RE(alpha_0[k]), RE(alpha_0[k])) + MUL_R(IM(alpha_0[k]), IM(alpha_0[k])) >= REAL_CONST(16)) ||
       (MUL_R(RE(alpha_1[k]), RE(alpha_1[k])) + MUL_R(IM(alpha_1[k]), IM(alpha_1[k])) >= REAL_CONST(16))) {
        RE(alpha_0[k]) = 0;
        IM(alpha_0[k]) = 0;
        RE(alpha_1[k]) = 0;
        IM(alpha_1[k]) = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* FIXED POINT: bwArray = COEF */
static int32_t mapNewBw(uint8_t invf_mode, uint8_t invf_mode_prev) {
    switch(invf_mode) {
    case 1:                     /* LOW */
        if(invf_mode_prev == 0) /* NONE */
            return COEF_CONST(0.6);
        else
            return COEF_CONST(0.75);

    case 2: /* MID */ return COEF_CONST(0.9);

    case 3: /* HIGH */ return COEF_CONST(0.98);

    default:                    /* NONE */
        if(invf_mode_prev == 1) /* LOW */
            return COEF_CONST(0.6);
        else
            return COEF_CONST(0.0);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* FIXED POINT: bwArray = COEF */
static void calc_chirp_factors(sbr_info* sbr, uint8_t ch) {
    uint8_t i;

    for(i = 0; i < sbr->N_Q; i++) {
        sbr->bwArray[ch][i] = mapNewBw(sbr->bs_invf_mode[ch][i], sbr->bs_invf_mode_prev[ch][i]);
        if(sbr->bwArray[ch][i] < sbr->bwArray_prev[ch][i]) sbr->bwArray[ch][i] = MUL_F(sbr->bwArray[ch][i], FRAC_CONST(0.75)) + MUL_F(sbr->bwArray_prev[ch][i], FRAC_CONST(0.25));
        else
            sbr->bwArray[ch][i] = MUL_F(sbr->bwArray[ch][i], FRAC_CONST(0.90625)) + MUL_F(sbr->bwArray_prev[ch][i], FRAC_CONST(0.09375));
        if(sbr->bwArray[ch][i] < COEF_CONST(0.015625)) sbr->bwArray[ch][i] = COEF_CONST(0.0);
        if(sbr->bwArray[ch][i] >= COEF_CONST(0.99609375)) sbr->bwArray[ch][i] = COEF_CONST(0.99609375);
        sbr->bwArray_prev[ch][i] = sbr->bwArray[ch][i];
        sbr->bs_invf_mode_prev[ch][i] = sbr->bs_invf_mode[ch][i];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void patch_construction(sbr_info* sbr) {
    uint8_t i, k;
    uint8_t odd, sb;
    uint8_t msb = sbr->k0;
    uint8_t usb = sbr->kx;
    uint8_t goalSbTab[] = {21, 23, 32, 43, 46, 64, 85, 93, 128, 0, 0, 0};
    /* (uint8_t)(2.048e6/sbr->sample_rate + 0.5); */
    uint8_t goalSb = goalSbTab[get_sr_index(sbr->sample_rate)];

    sbr->noPatches = 0;
    if(goalSb < (sbr->kx + sbr->M)) {
        for(i = 0, k = 0; sbr->f_master[i] < goalSb; i++) k = i + 1;
    }
    else { k = sbr->N_master; }
    if(sbr->N_master == 0) {
        sbr->noPatches = 0;
        sbr->patchNoSubbands[0] = 0;
        sbr->patchStartSubband[0] = 0;
        return;
    }
    do {
        uint8_t j = k + 1;
        do {
            j--;
            sb = sbr->f_master[j];
            odd = (sb - 2 + sbr->k0) % 2;
        } while(sb > (sbr->k0 - 1 + msb - odd));
        sbr->patchNoSubbands[sbr->noPatches] = max(sb - usb, 0);
        sbr->patchStartSubband[sbr->noPatches] = sbr->k0 - odd - sbr->patchNoSubbands[sbr->noPatches];
        if(sbr->patchNoSubbands[sbr->noPatches] > 0) {
            usb = sb;
            msb = sb;
            sbr->noPatches++;
        }
        else { msb = sbr->kx; }
        if(sbr->f_master[k] - sb < 3) k = sbr->N_master;
    } while(sb != (sbr->kx + sbr->M));
    if((sbr->patchNoSubbands[sbr->noPatches - 1] < 3) && (sbr->noPatches > 1)) { sbr->noPatches--; }
    sbr->noPatches = min(sbr->noPatches, 5);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int16_t sbr_huff_dec(bitfile* ld, sbr_huff_tab t_huff) {
    uint8_t bit;
    int16_t index = 0;

    while(index >= 0) {
        bit = (uint8_t)faad_get1bit(ld);
        index = t_huff[index][bit];
    }
    return index + 64;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* table 10 */
void sbr_envelope(bitfile* ld, sbr_info* sbr, uint8_t ch) {
    uint8_t      env, band;
    int8_t       delta = 0;
    sbr_huff_tab t_huff, f_huff;

    if((sbr->L_E[ch] == 1) && (sbr->bs_frame_class[ch] == FIXFIX)) sbr->amp_res[ch] = 0;
    else
        sbr->amp_res[ch] = sbr->bs_amp_res;

    if((sbr->bs_coupling) && (ch == 1)) {
        delta = 1;
        if(sbr->amp_res[ch]) {
            t_huff = t_huffman_env_bal_3_0dB;
            f_huff = f_huffman_env_bal_3_0dB;
        }
        else {
            t_huff = t_huffman_env_bal_1_5dB;
            f_huff = f_huffman_env_bal_1_5dB;
        }
    }
    else {
        delta = 0;
        if(sbr->amp_res[ch]) {
            t_huff = t_huffman_env_3_0dB;
            f_huff = f_huffman_env_3_0dB;
        }
        else {
            t_huff = t_huffman_env_1_5dB;
            f_huff = f_huffman_env_1_5dB;
        }
    }

    for(env = 0; env < sbr->L_E[ch]; env++) {
        if(sbr->bs_df_env[ch][env] == 0) {
            if((sbr->bs_coupling == 1) && (ch == 1)) {
                if(sbr->amp_res[ch]) { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 5) << delta); }
                else { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 6) << delta); }
            }
            else {
                if(sbr->amp_res[ch]) { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 6) << delta); }
                else { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 7) << delta); }
            }
            for(band = 1; band < sbr->n[sbr->f[ch][env]]; band++) { sbr->E[ch][band][env] = (sbr_huff_dec(ld, f_huff) << delta); }
        }
        else {
            for(band = 0; band < sbr->n[sbr->f[ch][env]]; band++) { sbr->E[ch][band][env] = (sbr_huff_dec(ld, t_huff) << delta); }
        }
    }
    extract_envelope_data(sbr, ch);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* table 11 */
void sbr_noise(bitfile* ld, sbr_info* sbr, uint8_t ch) {
    uint8_t      noise, band;
    int8_t       delta = 0;
    sbr_huff_tab t_huff, f_huff;

    if((sbr->bs_coupling == 1) && (ch == 1)) {
        delta = 1;
        t_huff = t_huffman_noise_bal_3_0dB;
        f_huff = f_huffman_env_bal_3_0dB;
    }
    else {
        delta = 0;
        t_huff = t_huffman_noise_3_0dB;
        f_huff = f_huffman_env_3_0dB;
    }
    for(noise = 0; noise < sbr->L_Q[ch]; noise++) {
        if(sbr->bs_df_noise[ch][noise] == 0) {
            if((sbr->bs_coupling == 1) && (ch == 1)) { sbr->Q[ch][0][noise] = (faad_getbits(ld, 5) << delta); }
            else { sbr->Q[ch][0][noise] = (faad_getbits(ld, 5) << delta); }
            for(band = 1; band < sbr->N_Q; band++) { sbr->Q[ch][band][noise] = (sbr_huff_dec(ld, f_huff) << delta); }
        }
        else {
            for(band = 0; band < sbr->N_Q; band++) { sbr->Q[ch][band][noise] = (sbr_huff_dec(ld, t_huff) << delta); }
        }
    }
    extract_noise_floor_data(sbr, ch);
}




/* EOF */
