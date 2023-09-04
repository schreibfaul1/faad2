/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2005 M. Bakker, Nero AG, http://www.nero.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** The "appropriate copyright message" mentioned in section 2c of the GPLv2
** must read: "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Nero AG through Mpeg4AAClicense@nero.com.
**
** $Id: sbr_dct.c,v 1.20 2007/11/01 12:33:34 menno Exp $
**/

/* Most of the DCT/DST codes here are generated using Spiral which is GPL
 * For more info see: http://www.spiral.net/
 */
#include "neaacdec.h"

#ifdef SBR_DEC
void DCT4_32(int32_t* y, int32_t* x) {
    int32_t f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    int32_t f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    int32_t f21, f22, f23, f24, f25, f26, f27, f28, f29, f30;
    int32_t f31, f32, f33, f34, f35, f36, f37, f38, f39, f40;
    int32_t f41, f42, f43, f44, f45, f46, f47, f48, f49, f50;
    int32_t f51, f52, f53, f54, f55, f56, f57, f58, f59, f60;
    int32_t f61, f62, f63, f64, f65, f66, f67, f68, f69, f70;
    int32_t f71, f72, f73, f74, f75, f76, f77, f78, f79, f80;
    int32_t f81, f82, f83, f84, f85, f86, f87, f88, f89, f90;
    int32_t f91, f92, f93, f94, f95, f96, f97, f98, f99, f100;
    int32_t f101, f102, f103, f104, f105, f106, f107, f108, f109, f110;
    int32_t f111, f112, f113, f114, f115, f116, f117, f118, f119, f120;
    int32_t f121, f122, f123, f124, f125, f126, f127, f128, f129, f130;
    int32_t f131, f132, f133, f134, f135, f136, f137, f138, f139, f140;
    int32_t f141, f142, f143, f144, f145, f146, f147, f148, f149, f150;
    int32_t f151, f152, f153, f154, f155, f156, f157, f158, f159, f160;
    int32_t f161, f162, f163, f164, f165, f166, f167, f168, f169, f170;
    int32_t f171, f172, f173, f174, f175, f176, f177, f178, f179, f180;
    int32_t f181, f182, f183, f184, f185, f186, f187, f188, f189, f190;
    int32_t f191, f192, f193, f194, f195, f196, f197, f198, f199, f200;
    int32_t f201, f202, f203, f204, f205, f206, f207, f208, f209, f210;
    int32_t f211, f212, f213, f214, f215, f216, f217, f218, f219, f220;
    int32_t f221, f222, f223, f224, f225, f226, f227, f228, f229, f230;
    int32_t f231, f232, f233, f234, f235, f236, f237, f238, f239, f240;
    int32_t f241, f242, f243, f244, f245, f246, f247, f248, f249, f250;
    int32_t f251, f252, f253, f254, f255, f256, f257, f258, f259, f260;
    int32_t f261, f262, f263, f264, f265, f266, f267, f268, f269, f270;
    int32_t f271, f272, f273, f274, f275, f276, f277, f278, f279, f280;
    int32_t f281, f282, f283, f284, f285, f286, f287, f288, f289, f290;
    int32_t f291, f292, f293, f294, f295, f296, f297, f298, f299, f300;
    int32_t f301, f302, f303, f304, f305, f306, f307, f310, f311, f312;
    int32_t f313, f316, f317, f318, f319, f322, f323, f324, f325, f328;
    int32_t f329, f330, f331, f334, f335, f336, f337, f340, f341, f342;
    int32_t f343, f346, f347, f348, f349, f352, f353, f354, f355, f358;
    int32_t f359, f360, f361, f364, f365, f366, f367, f370, f371, f372;
    int32_t f373, f376, f377, f378, f379, f382, f383, f384, f385, f388;
    int32_t f389, f390, f391, f394, f395, f396, f397;

    f0 = x[15] - x[16];
    f1 = x[15] + x[16];
    f2 = MUL_F(FRAC_CONST(0.7071067811865476), f1);
    f3 = MUL_F(FRAC_CONST(0.7071067811865476), f0);
    f4 = x[8] - x[23];
    f5 = x[8] + x[23];
    f6 = MUL_F(FRAC_CONST(0.7071067811865476), f5);
    f7 = MUL_F(FRAC_CONST(0.7071067811865476), f4);
    f8 = x[12] - x[19];
    f9 = x[12] + x[19];
    f10 = MUL_F(FRAC_CONST(0.7071067811865476), f9);
    f11 = MUL_F(FRAC_CONST(0.7071067811865476), f8);
    f12 = x[11] - x[20];
    f13 = x[11] + x[20];
    f14 = MUL_F(FRAC_CONST(0.7071067811865476), f13);
    f15 = MUL_F(FRAC_CONST(0.7071067811865476), f12);
    f16 = x[14] - x[17];
    f17 = x[14] + x[17];
    f18 = MUL_F(FRAC_CONST(0.7071067811865476), f17);
    f19 = MUL_F(FRAC_CONST(0.7071067811865476), f16);
    f20 = x[9] - x[22];
    f21 = x[9] + x[22];
    f22 = MUL_F(FRAC_CONST(0.7071067811865476), f21);
    f23 = MUL_F(FRAC_CONST(0.7071067811865476), f20);
    f24 = x[13] - x[18];
    f25 = x[13] + x[18];
    f26 = MUL_F(FRAC_CONST(0.7071067811865476), f25);
    f27 = MUL_F(FRAC_CONST(0.7071067811865476), f24);
    f28 = x[10] - x[21];
    f29 = x[10] + x[21];
    f30 = MUL_F(FRAC_CONST(0.7071067811865476), f29);
    f31 = MUL_F(FRAC_CONST(0.7071067811865476), f28);
    f32 = x[0] - f2;
    f33 = x[0] + f2;
    f34 = x[31] - f3;
    f35 = x[31] + f3;
    f36 = x[7] - f6;
    f37 = x[7] + f6;
    f38 = x[24] - f7;
    f39 = x[24] + f7;
    f40 = x[3] - f10;
    f41 = x[3] + f10;
    f42 = x[28] - f11;
    f43 = x[28] + f11;
    f44 = x[4] - f14;
    f45 = x[4] + f14;
    f46 = x[27] - f15;
    f47 = x[27] + f15;
    f48 = x[1] - f18;
    f49 = x[1] + f18;
    f50 = x[30] - f19;
    f51 = x[30] + f19;
    f52 = x[6] - f22;
    f53 = x[6] + f22;
    f54 = x[25] - f23;
    f55 = x[25] + f23;
    f56 = x[2] - f26;
    f57 = x[2] + f26;
    f58 = x[29] - f27;
    f59 = x[29] + f27;
    f60 = x[5] - f30;
    f61 = x[5] + f30;
    f62 = x[26] - f31;
    f63 = x[26] + f31;
    f64 = f39 + f37;
    f65 = MUL_F(FRAC_CONST(-0.5411961001461969), f39);
    f66 = MUL_F(FRAC_CONST(0.9238795325112867), f64);
    f67 = MUL_C(COEF_CONST(1.3065629648763766), f37);
    f68 = f65 + f66;
    f69 = f67 - f66;
    f70 = f38 + f36;
    f71 = MUL_C(COEF_CONST(1.3065629648763770), f38);
    f72 = MUL_F(FRAC_CONST(-0.3826834323650904), f70);
    f73 = MUL_F(FRAC_CONST(0.5411961001461961), f36);
    f74 = f71 + f72;
    f75 = f73 - f72;
    f76 = f47 + f45;
    f77 = MUL_F(FRAC_CONST(-0.5411961001461969), f47);
    f78 = MUL_F(FRAC_CONST(0.9238795325112867), f76);
    f79 = MUL_C(COEF_CONST(1.3065629648763766), f45);
    f80 = f77 + f78;
    f81 = f79 - f78;
    f82 = f46 + f44;
    f83 = MUL_C(COEF_CONST(1.3065629648763770), f46);
    f84 = MUL_F(FRAC_CONST(-0.3826834323650904), f82);
    f85 = MUL_F(FRAC_CONST(0.5411961001461961), f44);
    f86 = f83 + f84;
    f87 = f85 - f84;
    f88 = f55 + f53;
    f89 = MUL_F(FRAC_CONST(-0.5411961001461969), f55);
    f90 = MUL_F(FRAC_CONST(0.9238795325112867), f88);
    f91 = MUL_C(COEF_CONST(1.3065629648763766), f53);
    f92 = f89 + f90;
    f93 = f91 - f90;
    f94 = f54 + f52;
    f95 = MUL_C(COEF_CONST(1.3065629648763770), f54);
    f96 = MUL_F(FRAC_CONST(-0.3826834323650904), f94);
    f97 = MUL_F(FRAC_CONST(0.5411961001461961), f52);
    f98 = f95 + f96;
    f99 = f97 - f96;
    f100 = f63 + f61;
    f101 = MUL_F(FRAC_CONST(-0.5411961001461969), f63);
    f102 = MUL_F(FRAC_CONST(0.9238795325112867), f100);
    f103 = MUL_C(COEF_CONST(1.3065629648763766), f61);
    f104 = f101 + f102;
    f105 = f103 - f102;
    f106 = f62 + f60;
    f107 = MUL_C(COEF_CONST(1.3065629648763770), f62);
    f108 = MUL_F(FRAC_CONST(-0.3826834323650904), f106);
    f109 = MUL_F(FRAC_CONST(0.5411961001461961), f60);
    f110 = f107 + f108;
    f111 = f109 - f108;
    f112 = f33 - f68;
    f113 = f33 + f68;
    f114 = f35 - f69;
    f115 = f35 + f69;
    f116 = f32 - f74;
    f117 = f32 + f74;
    f118 = f34 - f75;
    f119 = f34 + f75;
    f120 = f41 - f80;
    f121 = f41 + f80;
    f122 = f43 - f81;
    f123 = f43 + f81;
    f124 = f40 - f86;
    f125 = f40 + f86;
    f126 = f42 - f87;
    f127 = f42 + f87;
    f128 = f49 - f92;
    f129 = f49 + f92;
    f130 = f51 - f93;
    f131 = f51 + f93;
    f132 = f48 - f98;
    f133 = f48 + f98;
    f134 = f50 - f99;
    f135 = f50 + f99;
    f136 = f57 - f104;
    f137 = f57 + f104;
    f138 = f59 - f105;
    f139 = f59 + f105;
    f140 = f56 - f110;
    f141 = f56 + f110;
    f142 = f58 - f111;
    f143 = f58 + f111;
    f144 = f123 + f121;
    f145 = MUL_F(FRAC_CONST(-0.7856949583871021), f123);
    f146 = MUL_F(FRAC_CONST(0.9807852804032304), f144);
    f147 = MUL_C(COEF_CONST(1.1758756024193588), f121);
    f148 = f145 + f146;
    f149 = f147 - f146;
    f150 = f127 + f125;
    f151 = MUL_F(FRAC_CONST(0.2758993792829431), f127);
    f152 = MUL_F(FRAC_CONST(0.5555702330196022), f150);
    f153 = MUL_C(COEF_CONST(1.3870398453221475), f125);
    f154 = f151 + f152;
    f155 = f153 - f152;
    f156 = f122 + f120;
    f157 = MUL_C(COEF_CONST(1.1758756024193591), f122);
    f158 = MUL_F(FRAC_CONST(-0.1950903220161287), f156);
    f159 = MUL_F(FRAC_CONST(0.7856949583871016), f120);
    f160 = f157 + f158;
    f161 = f159 - f158;
    f162 = f126 + f124;
    f163 = MUL_C(COEF_CONST(1.3870398453221473), f126);
    f164 = MUL_F(FRAC_CONST(-0.8314696123025455), f162);
    f165 = MUL_F(FRAC_CONST(-0.2758993792829436), f124);
    f166 = f163 + f164;
    f167 = f165 - f164;
    f168 = f139 + f137;
    f169 = MUL_F(FRAC_CONST(-0.7856949583871021), f139);
    f170 = MUL_F(FRAC_CONST(0.9807852804032304), f168);
    f171 = MUL_C(COEF_CONST(1.1758756024193588), f137);
    f172 = f169 + f170;
    f173 = f171 - f170;
    f174 = f143 + f141;
    f175 = MUL_F(FRAC_CONST(0.2758993792829431), f143);
    f176 = MUL_F(FRAC_CONST(0.5555702330196022), f174);
    f177 = MUL_C(COEF_CONST(1.3870398453221475), f141);
    f178 = f175 + f176;
    f179 = f177 - f176;
    f180 = f138 + f136;
    f181 = MUL_C(COEF_CONST(1.1758756024193591), f138);
    f182 = MUL_F(FRAC_CONST(-0.1950903220161287), f180);
    f183 = MUL_F(FRAC_CONST(0.7856949583871016), f136);
    f184 = f181 + f182;
    f185 = f183 - f182;
    f186 = f142 + f140;
    f187 = MUL_C(COEF_CONST(1.3870398453221473), f142);
    f188 = MUL_F(FRAC_CONST(-0.8314696123025455), f186);
    f189 = MUL_F(FRAC_CONST(-0.2758993792829436), f140);
    f190 = f187 + f188;
    f191 = f189 - f188;
    f192 = f113 - f148;
    f193 = f113 + f148;
    f194 = f115 - f149;
    f195 = f115 + f149;
    f196 = f117 - f154;
    f197 = f117 + f154;
    f198 = f119 - f155;
    f199 = f119 + f155;
    f200 = f112 - f160;
    f201 = f112 + f160;
    f202 = f114 - f161;
    f203 = f114 + f161;
    f204 = f116 - f166;
    f205 = f116 + f166;
    f206 = f118 - f167;
    f207 = f118 + f167;
    f208 = f129 - f172;
    f209 = f129 + f172;
    f210 = f131 - f173;
    f211 = f131 + f173;
    f212 = f133 - f178;
    f213 = f133 + f178;
    f214 = f135 - f179;
    f215 = f135 + f179;
    f216 = f128 - f184;
    f217 = f128 + f184;
    f218 = f130 - f185;
    f219 = f130 + f185;
    f220 = f132 - f190;
    f221 = f132 + f190;
    f222 = f134 - f191;
    f223 = f134 + f191;
    f224 = f211 + f209;
    f225 = MUL_F(FRAC_CONST(-0.8971675863426361), f211);
    f226 = MUL_F(FRAC_CONST(0.9951847266721968), f224);
    f227 = MUL_C(COEF_CONST(1.0932018670017576), f209);
    f228 = f225 + f226;
    f229 = f227 - f226;
    f230 = f215 + f213;
    f231 = MUL_F(FRAC_CONST(-0.4105245275223571), f215);
    f232 = MUL_F(FRAC_CONST(0.8819212643483549), f230);
    f233 = MUL_C(COEF_CONST(1.3533180011743529), f213);
    f234 = f231 + f232;
    f235 = f233 - f232;
    f236 = f219 + f217;
    f237 = MUL_F(FRAC_CONST(0.1386171691990915), f219);
    f238 = MUL_F(FRAC_CONST(0.6343932841636455), f236);
    f239 = MUL_C(COEF_CONST(1.4074037375263826), f217);
    f240 = f237 + f238;
    f241 = f239 - f238;
    f242 = f223 + f221;
    f243 = MUL_F(FRAC_CONST(0.6666556584777466), f223);
    f244 = MUL_F(FRAC_CONST(0.2902846772544623), f242);
    f245 = MUL_C(COEF_CONST(1.2472250129866711), f221);
    f246 = f243 + f244;
    f247 = f245 - f244;
    f248 = f210 + f208;
    f249 = MUL_C(COEF_CONST(1.0932018670017574), f210);
    f250 = MUL_F(FRAC_CONST(-0.0980171403295605), f248);
    f251 = MUL_F(FRAC_CONST(0.8971675863426364), f208);
    f252 = f249 + f250;
    f253 = f251 - f250;
    f254 = f214 + f212;
    f255 = MUL_C(COEF_CONST(1.3533180011743529), f214);
    f256 = MUL_F(FRAC_CONST(-0.4713967368259979), f254);
    f257 = MUL_F(FRAC_CONST(0.4105245275223569), f212);
    f258 = f255 + f256;
    f259 = f257 - f256;
    f260 = f218 + f216;
    f261 = MUL_C(COEF_CONST(1.4074037375263826), f218);
    f262 = MUL_F(FRAC_CONST(-0.7730104533627369), f260);
    f263 = MUL_F(FRAC_CONST(-0.1386171691990913), f216);
    f264 = f261 + f262;
    f265 = f263 - f262;
    f266 = f222 + f220;
    f267 = MUL_C(COEF_CONST(1.2472250129866711), f222);
    f268 = MUL_F(FRAC_CONST(-0.9569403357322089), f266);
    f269 = MUL_F(FRAC_CONST(-0.6666556584777469), f220);
    f270 = f267 + f268;
    f271 = f269 - f268;
    f272 = f193 - f228;
    f273 = f193 + f228;
    f274 = f195 - f229;
    f275 = f195 + f229;
    f276 = f197 - f234;
    f277 = f197 + f234;
    f278 = f199 - f235;
    f279 = f199 + f235;
    f280 = f201 - f240;
    f281 = f201 + f240;
    f282 = f203 - f241;
    f283 = f203 + f241;
    f284 = f205 - f246;
    f285 = f205 + f246;
    f286 = f207 - f247;
    f287 = f207 + f247;
    f288 = f192 - f252;
    f289 = f192 + f252;
    f290 = f194 - f253;
    f291 = f194 + f253;
    f292 = f196 - f258;
    f293 = f196 + f258;
    f294 = f198 - f259;
    f295 = f198 + f259;
    f296 = f200 - f264;
    f297 = f200 + f264;
    f298 = f202 - f265;
    f299 = f202 + f265;
    f300 = f204 - f270;
    f301 = f204 + f270;
    f302 = f206 - f271;
    f303 = f206 + f271;
    f304 = f275 + f273;
    f305 = MUL_F(FRAC_CONST(-0.9751575901732920), f275);
    f306 = MUL_F(FRAC_CONST(0.9996988186962043), f304);
    f307 = MUL_C(COEF_CONST(1.0242400472191164), f273);
    y[0] = f305 + f306;
    y[31] = f307 - f306;
    f310 = f279 + f277;
    f311 = MUL_F(FRAC_CONST(-0.8700688593994936), f279);
    f312 = MUL_F(FRAC_CONST(0.9924795345987100), f310);
    f313 = MUL_C(COEF_CONST(1.1148902097979263), f277);
    y[2] = f311 + f312;
    y[29] = f313 - f312;
    f316 = f283 + f281;
    f317 = MUL_F(FRAC_CONST(-0.7566008898816587), f283);
    f318 = MUL_F(FRAC_CONST(0.9757021300385286), f316);
    f319 = MUL_C(COEF_CONST(1.1948033701953984), f281);
    y[4] = f317 + f318;
    y[27] = f319 - f318;
    f322 = f287 + f285;
    f323 = MUL_F(FRAC_CONST(-0.6358464401941451), f287);
    f324 = MUL_F(FRAC_CONST(0.9495281805930367), f322);
    f325 = MUL_C(COEF_CONST(1.2632099209919283), f285);
    y[6] = f323 + f324;
    y[25] = f325 - f324;
    f328 = f291 + f289;
    f329 = MUL_F(FRAC_CONST(-0.5089684416985408), f291);
    f330 = MUL_F(FRAC_CONST(0.9142097557035307), f328);
    f331 = MUL_C(COEF_CONST(1.3194510697085207), f289);
    y[8] = f329 + f330;
    y[23] = f331 - f330;
    f334 = f295 + f293;
    f335 = MUL_F(FRAC_CONST(-0.3771887988789273), f295);
    f336 = MUL_F(FRAC_CONST(0.8700869911087114), f334);
    f337 = MUL_C(COEF_CONST(1.3629851833384954), f293);
    y[10] = f335 + f336;
    y[21] = f337 - f336;
    f340 = f299 + f297;
    f341 = MUL_F(FRAC_CONST(-0.2417766217337384), f299);
    f342 = MUL_F(FRAC_CONST(0.8175848131515837), f340);
    f343 = MUL_C(COEF_CONST(1.3933930045694289), f297);
    y[12] = f341 + f342;
    y[19] = f343 - f342;
    f346 = f303 + f301;
    f347 = MUL_F(FRAC_CONST(-0.1040360035527077), f303);
    f348 = MUL_F(FRAC_CONST(0.7572088465064845), f346);
    f349 = MUL_C(COEF_CONST(1.4103816894602612), f301);
    y[14] = f347 + f348;
    y[17] = f349 - f348;
    f352 = f274 + f272;
    f353 = MUL_F(FRAC_CONST(0.0347065382144002), f274);
    f354 = MUL_F(FRAC_CONST(0.6895405447370668), f352);
    f355 = MUL_C(COEF_CONST(1.4137876276885337), f272);
    y[16] = f353 + f354;
    y[15] = f355 - f354;
    f358 = f278 + f276;
    f359 = MUL_F(FRAC_CONST(0.1731148370459795), f278);
    f360 = MUL_F(FRAC_CONST(0.6152315905806268), f358);
    f361 = MUL_C(COEF_CONST(1.4035780182072330), f276);
    y[18] = f359 + f360;
    y[13] = f361 - f360;
    f364 = f282 + f280;
    f365 = MUL_F(FRAC_CONST(0.3098559453626100), f282);
    f366 = MUL_F(FRAC_CONST(0.5349976198870972), f364);
    f367 = MUL_C(COEF_CONST(1.3798511851368043), f280);
    y[20] = f365 + f366;
    y[11] = f367 - f366;
    f370 = f286 + f284;
    f371 = MUL_F(FRAC_CONST(0.4436129715409088), f286);
    f372 = MUL_F(FRAC_CONST(0.4496113296546065), f370);
    f373 = MUL_C(COEF_CONST(1.3428356308501219), f284);
    y[22] = f371 + f372;
    y[9] = f373 - f372;
    f376 = f290 + f288;
    f377 = MUL_F(FRAC_CONST(0.5730977622997509), f290);
    f378 = MUL_F(FRAC_CONST(0.3598950365349881), f376);
    f379 = MUL_C(COEF_CONST(1.2928878353697271), f288);
    y[24] = f377 + f378;
    y[7] = f379 - f378;
    f382 = f294 + f292;
    f383 = MUL_F(FRAC_CONST(0.6970633083205415), f294);
    f384 = MUL_F(FRAC_CONST(0.2667127574748984), f382);
    f385 = MUL_C(COEF_CONST(1.2304888232703382), f292);
    y[26] = f383 + f384;
    y[5] = f385 - f384;
    f388 = f298 + f296;
    f389 = MUL_F(FRAC_CONST(0.8143157536286401), f298);
    f390 = MUL_F(FRAC_CONST(0.1709618887603012), f388);
    f391 = MUL_C(COEF_CONST(1.1562395311492424), f296);
    y[28] = f389 + f390;
    y[3] = f391 - f390;
    f394 = f302 + f300;
    f395 = MUL_F(FRAC_CONST(0.9237258930790228), f302);
    f396 = MUL_F(FRAC_CONST(0.0735645635996674), f394);
    f397 = MUL_C(COEF_CONST(1.0708550202783576), f300);
    y[30] = f395 + f396;
    y[1] = f397 - f396;
}



// w_array_real[i] = cos(2*M_PI*i/32)
static const int32_t w_array_real[] = {
    FRAC_CONST(1.000000000000000),  FRAC_CONST(0.980785279337272),  FRAC_CONST(0.923879528329380),  FRAC_CONST(0.831469603195765),
    FRAC_CONST(0.707106765732237),  FRAC_CONST(0.555570210304169),  FRAC_CONST(0.382683402077046),  FRAC_CONST(0.195090284503576),
    FRAC_CONST(0.000000000000000),  FRAC_CONST(-0.195090370246552), FRAC_CONST(-0.382683482845162), FRAC_CONST(-0.555570282993553),
    FRAC_CONST(-0.707106827549476), FRAC_CONST(-0.831469651765257), FRAC_CONST(-0.923879561784627), FRAC_CONST(-0.980785296392607)};

// w_array_imag[i] = sin(-2*M_PI*i/32)
static const int32_t w_array_imag[] = {
    FRAC_CONST(0.000000000000000),  FRAC_CONST(-0.195090327375064), FRAC_CONST(-0.382683442461104), FRAC_CONST(-0.555570246648862),
    FRAC_CONST(-0.707106796640858), FRAC_CONST(-0.831469627480512), FRAC_CONST(-0.923879545057005), FRAC_CONST(-0.980785287864940),
    FRAC_CONST(-1.000000000000000), FRAC_CONST(-0.980785270809601), FRAC_CONST(-0.923879511601754), FRAC_CONST(-0.831469578911016),
    FRAC_CONST(-0.707106734823616), FRAC_CONST(-0.555570173959476), FRAC_CONST(-0.382683361692986), FRAC_CONST(-0.195090241632088)};

// FFT decimation in frequency
// 4*16*2+16=128+16=144 multiplications
// 6*16*2+10*8+4*16*2=192+80+128=400 additions
static void fft_dif(int32_t* Real, int32_t* Imag) {
    int32_t  w_real, w_imag;                                     // For faster access
    int32_t  point1_real, point1_imag, point2_real, point2_imag; // For faster access
    uint32_t j, i, i2, w_index;                                  // Counters

    // First 2 stages of 32 point FFT decimation in frequency
    // 4*16*2=64*2=128 multiplications
    // 6*16*2=96*2=192 additions
    // Stage 1 of 32 point FFT decimation in frequency
    for(i = 0; i < 16; i++) {
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i + 16;
        point2_real = Real[i2];
        point2_imag = Imag[i2];

        w_real = w_array_real[i];
        w_imag = w_array_imag[i];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag));
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

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag));
        Imag[i2] = (MUL_F(point1_real, w_imag) + MUL_F(point1_imag, w_real));

        i = j + 16;
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i + 8;
        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real, w_real) - MUL_F(point1_imag, w_imag));
        Imag[i2] = (MUL_F(point1_real, w_imag) + MUL_F(point1_imag, w_real));
    }

    // Stage 3 of 32 point FFT decimation in frequency
    // 2*4*2=16 multiplications
    // 4*4*2+6*4*2=10*8=80 additions
    for(i = 0; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // out[i1] = point1 + point2
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // out[i2] = point1 - point2
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }
    w_real = w_array_real[4]; // = sqrt(2)/2
    // w_imag = -w_real; // = w_array_imag[4]; // = -sqrt(2)/2
    for(i = 1; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = MUL_F(point1_real + point1_imag, w_real);
        Imag[i2] = MUL_F(point1_imag - point1_real, w_real);
    }
    for(i = 2; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * (-i)
        Real[i2] = point1_imag - point2_imag;
        Imag[i2] = point2_real - point1_real;
    }
    w_real = w_array_real[12]; // = -sqrt(2)/2
    // w_imag = w_real; // = w_array_imag[12]; // = -sqrt(2)/2
    for(i = 3; i < 32; i += 8) {
        i2 = i + 4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = MUL_F(point1_real - point1_imag, w_real);
        Imag[i2] = MUL_F(point1_real + point1_imag, w_real);
    }

    // Stage 4 of 32 point FFT decimation in frequency (no multiplications)
    // 16*4=64 additions
    for(i = 0; i < 32; i += 4) {
        i2 = i + 2;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = x[i] - x[i2]
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }
    for(i = 1; i < 32; i += 4) {
        i2 = i + 2;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * (-i)
        Real[i2] = point1_imag - point2_imag;
        Imag[i2] = point2_real - point1_real;
    }

    // Stage 5 of 32 point FFT decimation in frequency (no multiplications)
    // 16*4=64 additions
    for(i = 0; i < 32; i += 2) {
        i2 = i + 1;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // out[i1] = point1 + point2
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // out[i2] = point1 - point2
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }

    #ifdef REORDER_IN_FFT
    FFTReorder(Real, Imag);
    #endif // #ifdef REORDER_IN_FFT
}




  

static const int32_t dct4_64_tab[] = {
    COEF_CONST(0.999924719333649),  COEF_CONST(0.998118102550507),  COEF_CONST(0.993906974792480),  COEF_CONST(0.987301409244537),
    COEF_CONST(0.978317379951477),  COEF_CONST(0.966976463794708),  COEF_CONST(0.953306019306183),  COEF_CONST(0.937339007854462),
    COEF_CONST(0.919113874435425),  COEF_CONST(0.898674488067627),  COEF_CONST(0.876070082187653),  COEF_CONST(0.851355195045471),
    COEF_CONST(0.824589252471924),  COEF_CONST(0.795836925506592),  COEF_CONST(0.765167236328125),  COEF_CONST(0.732654273509979),
    COEF_CONST(0.698376238346100),  COEF_CONST(0.662415742874146),  COEF_CONST(0.624859452247620),  COEF_CONST(0.585797846317291),
    COEF_CONST(0.545324981212616),  COEF_CONST(0.503538429737091),  COEF_CONST(0.460538715124130),  COEF_CONST(0.416429549455643),
    COEF_CONST(0.371317148208618),  COEF_CONST(0.325310230255127),  COEF_CONST(0.278519600629807),  COEF_CONST(0.231058135628700),
    COEF_CONST(0.183039888739586),  COEF_CONST(0.134580686688423),  COEF_CONST(0.085797272622585),  COEF_CONST(0.036807164549828),
    COEF_CONST(-1.012196302413940), COEF_CONST(-1.059438824653626), COEF_CONST(-1.104129195213318), COEF_CONST(-1.146159529685974),
    COEF_CONST(-1.185428738594055), COEF_CONST(-1.221842169761658), COEF_CONST(-1.255311965942383), COEF_CONST(-1.285757660865784),
    COEF_CONST(-1.313105940818787), COEF_CONST(-1.337290763854981), COEF_CONST(-1.358253836631775), COEF_CONST(-1.375944852828980),
    COEF_CONST(-1.390321016311646), COEF_CONST(-1.401347875595093), COEF_CONST(-1.408998727798462), COEF_CONST(-1.413255214691162),
    COEF_CONST(-1.414107084274292), COEF_CONST(-1.411552190780640), COEF_CONST(-1.405596733093262), COEF_CONST(-1.396255016326904),
    COEF_CONST(-1.383549690246582), COEF_CONST(-1.367511272430420), COEF_CONST(-1.348178386688232), COEF_CONST(-1.325597524642944),
    COEF_CONST(-1.299823284149170), COEF_CONST(-1.270917654037476), COEF_CONST(-1.238950133323669), COEF_CONST(-1.203998088836670),
    COEF_CONST(-1.166145324707031), COEF_CONST(-1.125483393669128), COEF_CONST(-1.082109928131104), COEF_CONST(-1.036129593849182),
    COEF_CONST(-0.987653195858002), COEF_CONST(-0.936797380447388), COEF_CONST(-0.883684754371643), COEF_CONST(-0.828443288803101),
    COEF_CONST(-0.771206021308899), COEF_CONST(-0.712110757827759), COEF_CONST(-0.651300072669983), COEF_CONST(-0.588920354843140),
    COEF_CONST(-0.525121808052063), COEF_CONST(-0.460058242082596), COEF_CONST(-0.393886327743530), COEF_CONST(-0.326765477657318),
    COEF_CONST(-0.258857429027557), COEF_CONST(-0.190325915813446), COEF_CONST(-0.121335685253143), COEF_CONST(-0.052053272724152),
    COEF_CONST(0.017354607582092),  COEF_CONST(0.086720645427704),  COEF_CONST(0.155877828598022),  COEF_CONST(0.224659323692322),
    COEF_CONST(0.292899727821350),  COEF_CONST(0.360434412956238),  COEF_CONST(0.427100926637650),  COEF_CONST(0.492738455533981),
    COEF_CONST(0.557188928127289),  COEF_CONST(0.620297133922577),  COEF_CONST(0.681910991668701),  COEF_CONST(0.741881847381592),
    COEF_CONST(0.800065577030182),  COEF_CONST(0.856321990489960),  COEF_CONST(0.910515367984772),  COEF_CONST(0.962515234947205),
    COEF_CONST(1.000000000000000),  COEF_CONST(0.998795449733734),  COEF_CONST(0.995184719562531),  COEF_CONST(0.989176511764526),
    COEF_CONST(0.980785250663757),  COEF_CONST(0.970031261444092),  COEF_CONST(0.956940352916718),  COEF_CONST(0.941544055938721),
    COEF_CONST(0.923879504203796),  COEF_CONST(0.903989315032959),  COEF_CONST(0.881921231746674),  COEF_CONST(0.857728600502014),
    COEF_CONST(0.831469595432281),  COEF_CONST(0.803207516670227),  COEF_CONST(0.773010432720184),  COEF_CONST(0.740951120853424),
    COEF_CONST(0.707106769084930),  COEF_CONST(0.671558916568756),  COEF_CONST(0.634393274784088),  COEF_CONST(0.595699310302734),
    COEF_CONST(0.555570185184479),  COEF_CONST(0.514102697372437),  COEF_CONST(0.471396654844284),  COEF_CONST(0.427555114030838),
    COEF_CONST(0.382683426141739),  COEF_CONST(0.336889833211899),  COEF_CONST(0.290284633636475),  COEF_CONST(0.242980122566223),
    COEF_CONST(0.195090234279633),  COEF_CONST(0.146730497479439),  COEF_CONST(0.098017133772373),  COEF_CONST(0.049067649990320),
    COEF_CONST(-1.000000000000000), COEF_CONST(-1.047863125801086), COEF_CONST(-1.093201875686646), COEF_CONST(-1.135906934738159),
    COEF_CONST(-1.175875544548035), COEF_CONST(-1.213011503219605), COEF_CONST(-1.247225046157837), COEF_CONST(-1.278433918952942),
    COEF_CONST(-1.306562900543213), COEF_CONST(-1.331544399261475), COEF_CONST(-1.353317975997925), COEF_CONST(-1.371831417083740),
    COEF_CONST(-1.387039899826050), COEF_CONST(-1.398906826972961), COEF_CONST(-1.407403707504273), COEF_CONST(-1.412510156631470),
    COEF_CONST(0.0000000000000000), COEF_CONST(-1.412510156631470), COEF_CONST(-1.407403707504273), COEF_CONST(-1.398906826972961),
    COEF_CONST(-1.387039899826050), COEF_CONST(-1.371831417083740), COEF_CONST(-1.353317975997925), COEF_CONST(-1.331544399261475),
    COEF_CONST(-1.306562900543213), COEF_CONST(-1.278433918952942), COEF_CONST(-1.247225046157837), COEF_CONST(-1.213011384010315),
    COEF_CONST(-1.175875544548035), COEF_CONST(-1.135907053947449), COEF_CONST(-1.093201875686646), COEF_CONST(-1.047863125801086),
    COEF_CONST(-1.000000000000000), COEF_CONST(-0.949727773666382), COEF_CONST(-0.897167563438416), COEF_CONST(-0.842446029186249),
    COEF_CONST(-0.785694956779480), COEF_CONST(-0.727051079273224), COEF_CONST(-0.666655659675598), COEF_CONST(-0.604654192924500),
    COEF_CONST(-0.541196048259735), COEF_CONST(-0.476434230804443), COEF_CONST(-0.410524487495422), COEF_CONST(-0.343625843524933),
    COEF_CONST(-0.275899350643158), COEF_CONST(-0.207508206367493), COEF_CONST(-0.138617098331451), COEF_CONST(-0.069392144680023),
    COEF_CONST(0.000000000000000),  COEF_CONST(0.069392263889313),  COEF_CONST(0.138617157936096),  COEF_CONST(0.207508206367493),
    COEF_CONST(0.275899469852448),  COEF_CONST(0.343625962734222),  COEF_CONST(0.410524636507034),  COEF_CONST(0.476434201002121),
    COEF_CONST(0.541196107864380),  COEF_CONST(0.604654192924500),  COEF_CONST(0.666655719280243),  COEF_CONST(0.727051138877869),
    COEF_CONST(0.785695075988770),  COEF_CONST(0.842446029186249),  COEF_CONST(0.897167563438416),  COEF_CONST(0.949727773666382)};

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* size 64 only! */
void dct4_kernel(int32_t* in_real, int32_t* in_imag, int32_t* out_real, int32_t* out_imag) {
    // Tables with bit reverse values for 5 bits, bit reverse of i at i-th position
    const uint8_t bit_rev_tab[32] = {0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30,
                                     1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31};
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

#endif
