#include "neaacdec.h"

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



/* EOF */
