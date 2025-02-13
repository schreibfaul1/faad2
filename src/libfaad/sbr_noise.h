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
** $Id: sbr_noise.h,v 1.17 2007/11/01 12:33:35 menno Exp $
**/

#ifndef __SBR_NOISE_H__
#define __SBR_NOISE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4305)
#pragma warning(disable:4244)
#endif


/* Table 1.A.13 Noise table V */
ALIGN static const complex_t V[] = {
    { FRAC_CONST(-0.99948155879974), FRAC_CONST(-0.59483414888382) },
    { FRAC_CONST(0.97113454341888), FRAC_CONST(-0.67528516054153) },
    { FRAC_CONST(0.14130051434040), FRAC_CONST(-0.95090985298157) },
    { FRAC_CONST(-0.47005495429039), FRAC_CONST(-0.37340548634529) },
    { FRAC_CONST(0.80705064535141), FRAC_CONST(0.29653668403625) },
    { FRAC_CONST(-0.38981479406357), FRAC_CONST(0.89572608470917) },
    { FRAC_CONST(-0.01053049881011), FRAC_CONST(-0.66959059238434) },
    { FRAC_CONST(-0.91266369819641), FRAC_CONST(-0.11522938311100) },
    { FRAC_CONST(0.54840421676636), FRAC_CONST(0.75221365690231) },
    { FRAC_CONST(0.40009254217148), FRAC_CONST(-0.98929399251938) },
    { FRAC_CONST(-0.99867975711823), FRAC_CONST(-0.88147068023682) },
    { FRAC_CONST(-0.95531076192856), FRAC_CONST(0.90908759832382) },
    { FRAC_CONST(-0.45725932717323), FRAC_CONST(-0.56716322898865) },
    { FRAC_CONST(-0.72929674386978), FRAC_CONST(-0.98008275032043) },
    { FRAC_CONST(0.75622802972794), FRAC_CONST(0.20950329303741) },
    { FRAC_CONST(0.07069442421198), FRAC_CONST(-0.78247898817062) },
    { FRAC_CONST(0.74496251344681), FRAC_CONST(-0.91169005632401) },
    { FRAC_CONST(-0.96440184116364), FRAC_CONST(-0.94739919900894) },
    { FRAC_CONST(0.30424630641937), FRAC_CONST(-0.49438267946243) },
    { FRAC_CONST(0.66565030813217), FRAC_CONST(0.64652937650681) },
    { FRAC_CONST(0.91697007417679), FRAC_CONST(0.17514097690582) },
    { FRAC_CONST(-0.70774918794632), FRAC_CONST(0.52548652887344) },
    { FRAC_CONST(-0.70051413774490), FRAC_CONST(-0.45340028405190) },
    { FRAC_CONST(-0.99496513605118), FRAC_CONST(-0.90071910619736) },
    { FRAC_CONST(0.98164492845535), FRAC_CONST(-0.77463155984879) },
    { FRAC_CONST(-0.54671579599380), FRAC_CONST(-0.02570928446949) },
    { FRAC_CONST(-0.01689629070461), FRAC_CONST(0.00287506449968) },
    { FRAC_CONST(-0.86110347509384), FRAC_CONST(0.42548584938049) },
    { FRAC_CONST(-0.98892980813980), FRAC_CONST(-0.87881129980087) },
    { FRAC_CONST(0.51756626367569), FRAC_CONST(0.66926783323288) },
    { FRAC_CONST(-0.99635028839111), FRAC_CONST(-0.58107727766037) },
    { FRAC_CONST(-0.99969369173050), FRAC_CONST(0.98369991779327) },
    { FRAC_CONST(0.55266261100769), FRAC_CONST(0.59449058771133) },
    { FRAC_CONST(0.34581178426743), FRAC_CONST(0.94879418611526) },
    { FRAC_CONST(0.62664210796356), FRAC_CONST(-0.74402970075607) },
    { FRAC_CONST(-0.77149701118469), FRAC_CONST(-0.33883658051491) },
    { FRAC_CONST(-0.91592246294022), FRAC_CONST(0.03687901422381) },
    { FRAC_CONST(-0.76285493373871), FRAC_CONST(-0.91371870040894) },
    { FRAC_CONST(0.79788339138031), FRAC_CONST(-0.93180972337723) },
    { FRAC_CONST(0.54473078250885), FRAC_CONST(-0.11919206380844) },
    { FRAC_CONST(-0.85639280080795), FRAC_CONST(0.42429855465889) },
    { FRAC_CONST(-0.92882400751114), FRAC_CONST(0.27871808409691) },
    { FRAC_CONST(-0.11708371341228), FRAC_CONST(-0.99800843000412) },
    { FRAC_CONST(0.21356749534607), FRAC_CONST(-0.90716296434402) },
    { FRAC_CONST(-0.76191693544388), FRAC_CONST(0.99768120050430) },
    { FRAC_CONST(0.98111045360565), FRAC_CONST(-0.95854461193085) },
    { FRAC_CONST(-0.85913270711899), FRAC_CONST(0.95766568183899) },
    { FRAC_CONST(-0.93307244777679), FRAC_CONST(0.49431759119034) },
    { FRAC_CONST(0.30485755205154), FRAC_CONST(-0.70540034770966) },
    { FRAC_CONST(0.85289651155472), FRAC_CONST(0.46766132116318) },
    { FRAC_CONST(0.91328084468842), FRAC_CONST(-0.99839597940445) },
    { FRAC_CONST(-0.05890199914575), FRAC_CONST(0.70741826295853) },
    { FRAC_CONST(0.28398686647415), FRAC_CONST(0.34633556008339) },
    { FRAC_CONST(0.95258164405823), FRAC_CONST(-0.54893416166306) },
    { FRAC_CONST(-0.78566324710846), FRAC_CONST(-0.75568538904190) },
    { FRAC_CONST(-0.95789498090744), FRAC_CONST(-0.20423194766045) },
    { FRAC_CONST(0.82411158084869), FRAC_CONST(0.96654617786407) },
    { FRAC_CONST(-0.65185445547104), FRAC_CONST(-0.88734990358353) },
    { FRAC_CONST(-0.93643605709076), FRAC_CONST(0.99870789051056) },
    { FRAC_CONST(0.91427159309387), FRAC_CONST(-0.98290503025055) },
    { FRAC_CONST(-0.70395684242249), FRAC_CONST(0.58796799182892) },
    { FRAC_CONST(0.00563771976158), FRAC_CONST(0.61768198013306) },
    { FRAC_CONST(0.89065051078796), FRAC_CONST(0.52783352136612) },
    { FRAC_CONST(-0.68683707714081), FRAC_CONST(0.80806946754456) },
    { FRAC_CONST(0.72165340185165), FRAC_CONST(-0.69259858131409) },
    { FRAC_CONST(-0.62928247451782), FRAC_CONST(0.13627037405968) },
    { FRAC_CONST(0.29938435554504), FRAC_CONST(-0.46051329374313) },
    { FRAC_CONST(-0.91781955957413), FRAC_CONST(-0.74012714624405) },
    { FRAC_CONST(0.99298715591431), FRAC_CONST(0.40816611051559) },
    { FRAC_CONST(0.82368296384811), FRAC_CONST(-0.74036049842834) },
    { FRAC_CONST(-0.98512834310532), FRAC_CONST(-0.99972331523895) },
    { FRAC_CONST(-0.95915371179581), FRAC_CONST(-0.99237799644470) },
    { FRAC_CONST(-0.21411126852036), FRAC_CONST(-0.93424820899963) },
    { FRAC_CONST(-0.68821477890015), FRAC_CONST(-0.26892307400703) },
    { FRAC_CONST(0.91851997375488), FRAC_CONST(0.09358228743076) },
    { FRAC_CONST(-0.96062767505646), FRAC_CONST(0.36099094152451) },
    { FRAC_CONST(0.51646184921265), FRAC_CONST(-0.71373331546783) },
    { FRAC_CONST(0.61130720376968), FRAC_CONST(0.46950140595436) },
    { FRAC_CONST(0.47336128354073), FRAC_CONST(-0.27333179116249) },
    { FRAC_CONST(0.90998309850693), FRAC_CONST(0.96715664863586) },
    { FRAC_CONST(0.44844800233841), FRAC_CONST(0.99211573600769) },
    { FRAC_CONST(0.66614890098572), FRAC_CONST(0.96590173244476) },
    { FRAC_CONST(0.74922239780426), FRAC_CONST(-0.89879858493805) },
    { FRAC_CONST(-0.99571585655212), FRAC_CONST(0.52785521745682) },
    { FRAC_CONST(0.97401082515717), FRAC_CONST(-0.16855870187283) },
    { FRAC_CONST(0.72683745622635), FRAC_CONST(-0.48060774803162) },
    { FRAC_CONST(0.95432192087173), FRAC_CONST(0.68849605321884) },
    { FRAC_CONST(-0.72962206602097), FRAC_CONST(-0.76608443260193) },
    { FRAC_CONST(-0.85359477996826), FRAC_CONST(0.88738125562668) },
    { FRAC_CONST(-0.81412428617477), FRAC_CONST(-0.97480767965317) },
    { FRAC_CONST(-0.87930774688721), FRAC_CONST(0.74748307466507) },
    { FRAC_CONST(-0.71573328971863), FRAC_CONST(-0.98570609092712) },
    { FRAC_CONST(0.83524298667908), FRAC_CONST(0.83702534437180) },
    { FRAC_CONST(-0.48086065053940), FRAC_CONST(-0.98848503828049) },
    { FRAC_CONST(0.97139126062393), FRAC_CONST(0.80093622207642) },
    { FRAC_CONST(0.51992827653885), FRAC_CONST(0.80247628688812) },
    { FRAC_CONST(-0.00848591234535), FRAC_CONST(-0.76670128107071) },
    { FRAC_CONST(-0.70294374227524), FRAC_CONST(0.55359911918640) },
    { FRAC_CONST(-0.95894426107407), FRAC_CONST(-0.43265503644943) },
    { FRAC_CONST(0.97079253196716), FRAC_CONST(0.09325857460499) },
    { FRAC_CONST(-0.92404294013977), FRAC_CONST(0.85507702827454) },
    { FRAC_CONST(-0.69506472349167), FRAC_CONST(0.98633414506912) },
    { FRAC_CONST(0.26559203863144), FRAC_CONST(0.73314309120178) },
    { FRAC_CONST(0.28038442134857), FRAC_CONST(0.14537914097309) },
    { FRAC_CONST(-0.74138122797012), FRAC_CONST(0.99310338497162) },
    { FRAC_CONST(-0.01752796024084), FRAC_CONST(-0.82616633176804) },
    { FRAC_CONST(-0.55126774311066), FRAC_CONST(-0.98898541927338) },
    { FRAC_CONST(0.97960901260376), FRAC_CONST(-0.94021445512772) },
    { FRAC_CONST(-0.99196308851242), FRAC_CONST(0.67019015550613) },
    { FRAC_CONST(-0.67684930562973), FRAC_CONST(0.12631492316723) },
    { FRAC_CONST(0.09140039235353), FRAC_CONST(-0.20537731051445) },
    { FRAC_CONST(-0.71658962965012), FRAC_CONST(-0.97788202762604) },
    { FRAC_CONST(0.81014639139175), FRAC_CONST(0.53722649812698) },
    { FRAC_CONST(0.40616992115974), FRAC_CONST(-0.26469007134438) },
    { FRAC_CONST(-0.67680186033249), FRAC_CONST(0.94502049684525) },
    { FRAC_CONST(0.86849772930145), FRAC_CONST(-0.18333598971367) },
    { FRAC_CONST(-0.99500381946564), FRAC_CONST(-0.02634122036397) },
    { FRAC_CONST(0.84329187870026), FRAC_CONST(0.10406957566738) },
    { FRAC_CONST(-0.09215968847275), FRAC_CONST(0.69540011882782) },
    { FRAC_CONST(0.99956172704697), FRAC_CONST(-0.12358541786671) },
    { FRAC_CONST(-0.79732781648636), FRAC_CONST(-0.91582524776459) },
    { FRAC_CONST(0.96349972486496), FRAC_CONST(0.96640455722809) },
    { FRAC_CONST(-0.79942780733109), FRAC_CONST(0.64323902130127) },
    { FRAC_CONST(-0.11566039919853), FRAC_CONST(0.28587844967842) },
    { FRAC_CONST(-0.39922955632210), FRAC_CONST(0.94129604101181) },
    { FRAC_CONST(0.99089199304581), FRAC_CONST(-0.92062628269196) },
    { FRAC_CONST(0.28631284832954), FRAC_CONST(-0.91035044193268) },
    { FRAC_CONST(-0.83302724361420), FRAC_CONST(-0.67330408096313) },
    { FRAC_CONST(0.95404446125031), FRAC_CONST(0.49162766337395) },
    { FRAC_CONST(-0.06449863314629), FRAC_CONST(0.03250560909510) },
    { FRAC_CONST(-0.99575054645538), FRAC_CONST(0.42389783263206) },
    { FRAC_CONST(-0.65501141548157), FRAC_CONST(0.82546114921570) },
    { FRAC_CONST(-0.81254440546036), FRAC_CONST(-0.51627236604691) },
    { FRAC_CONST(-0.99646371603012), FRAC_CONST(0.84490531682968) },
    { FRAC_CONST(0.00287840608507), FRAC_CONST(0.64768260717392) },
    { FRAC_CONST(0.70176988840103), FRAC_CONST(-0.20453028380871) },
    { FRAC_CONST(0.96361881494522), FRAC_CONST(0.40706968307495) },
    { FRAC_CONST(-0.68883758783340), FRAC_CONST(0.91338956356049) },
    { FRAC_CONST(-0.34875586628914), FRAC_CONST(0.71472293138504) },
    { FRAC_CONST(0.91980081796646), FRAC_CONST(0.66507452726364) },
    { FRAC_CONST(-0.99009048938751), FRAC_CONST(0.85868018865585) },
    { FRAC_CONST(0.68865793943405), FRAC_CONST(0.55660319328308) },
    { FRAC_CONST(-0.99484401941299), FRAC_CONST(-0.20052559673786) },
    { FRAC_CONST(0.94214510917664), FRAC_CONST(-0.99696427583694) },
    { FRAC_CONST(-0.67414629459381), FRAC_CONST(0.49548220634460) },
    { FRAC_CONST(-0.47339352965355), FRAC_CONST(-0.85904330015182) },
    { FRAC_CONST(0.14323651790619), FRAC_CONST(-0.94145596027374) },
    { FRAC_CONST(-0.29268294572830), FRAC_CONST(0.05759225040674) },
    { FRAC_CONST(0.43793860077858), FRAC_CONST(-0.78904968500137) },
    { FRAC_CONST(-0.36345127224922), FRAC_CONST(0.64874434471130) },
    { FRAC_CONST(-0.08750604838133), FRAC_CONST(0.97686946392059) },
    { FRAC_CONST(-0.96495270729065), FRAC_CONST(-0.53960305452347) },
    { FRAC_CONST(0.55526942014694), FRAC_CONST(0.78891521692276) },
    { FRAC_CONST(0.73538213968277), FRAC_CONST(0.96452075242996) },
    { FRAC_CONST(-0.30889773368835), FRAC_CONST(-0.80664390325546) },
    { FRAC_CONST(0.03574995696545), FRAC_CONST(-0.97325617074966) },
    { FRAC_CONST(0.98720687627792), FRAC_CONST(0.48409134149551) },
    { FRAC_CONST(-0.81689298152924), FRAC_CONST(-0.90827703475952) },
    { FRAC_CONST(0.67866861820221), FRAC_CONST(0.81284505128860) },
    { FRAC_CONST(-0.15808570384979), FRAC_CONST(0.85279554128647) },
    { FRAC_CONST(0.80723392963409), FRAC_CONST(-0.24717418849468) },
    { FRAC_CONST(0.47788757085800), FRAC_CONST(-0.46333149075508) },
    { FRAC_CONST(0.96367555856705), FRAC_CONST(0.38486748933792) },
    { FRAC_CONST(-0.99143874645233), FRAC_CONST(-0.24945276975632) },
    { FRAC_CONST(0.83081877231598), FRAC_CONST(-0.94780850410461) },
    { FRAC_CONST(-0.58753192424774), FRAC_CONST(0.01290772389621) },
    { FRAC_CONST(0.95538109540939), FRAC_CONST(-0.85557049512863) },
    { FRAC_CONST(-0.96490919589996), FRAC_CONST(-0.64020973443985) },
    { FRAC_CONST(-0.97327101230621), FRAC_CONST(0.12378127872944) },
    { FRAC_CONST(0.91400367021561), FRAC_CONST(0.57972472906113) },
    { FRAC_CONST(-0.99925839900970), FRAC_CONST(0.71084845066071) },
    { FRAC_CONST(-0.86875903606415), FRAC_CONST(-0.20291699469090) },
    { FRAC_CONST(-0.26240035891533), FRAC_CONST(-0.68264555931091) },
    { FRAC_CONST(-0.24664412438869), FRAC_CONST(-0.87642270326614) },
    { FRAC_CONST(0.02416275814176), FRAC_CONST(0.27192914485931) },
    { FRAC_CONST(0.82068622112274), FRAC_CONST(-0.85087788105011) },
    { FRAC_CONST(0.88547372817993), FRAC_CONST(-0.89636802673340) },
    { FRAC_CONST(-0.18173077702522), FRAC_CONST(-0.26152145862579) },
    { FRAC_CONST(0.09355476498604), FRAC_CONST(0.54845124483109) },
    { FRAC_CONST(-0.54668414592743), FRAC_CONST(0.95980775356293) },
    { FRAC_CONST(0.37050989270210), FRAC_CONST(-0.59910142421722) },
    { FRAC_CONST(-0.70373594760895), FRAC_CONST(0.91227668523788) },
    { FRAC_CONST(-0.34600785374641), FRAC_CONST(-0.99441426992416) },
    { FRAC_CONST(-0.68774479627609), FRAC_CONST(-0.30238837003708) },
    { FRAC_CONST(-0.26843291521072), FRAC_CONST(0.83115667104721) },
    { FRAC_CONST(0.49072334170341), FRAC_CONST(-0.45359709858894) },
    { FRAC_CONST(0.38975992798805), FRAC_CONST(0.95515358448029) },
    { FRAC_CONST(-0.97757124900818), FRAC_CONST(0.05305894464254) },
    { FRAC_CONST(-0.17325553297997), FRAC_CONST(-0.92770671844482) },
    { FRAC_CONST(0.99948036670685), FRAC_CONST(0.58285546302795) },
    { FRAC_CONST(-0.64946246147156), FRAC_CONST(0.68645507097244) },
    { FRAC_CONST(-0.12016920745373), FRAC_CONST(-0.57147324085236) },
    { FRAC_CONST(-0.58947455883026), FRAC_CONST(-0.34847131371498) },
    { FRAC_CONST(-0.41815140843391), FRAC_CONST(0.16276422142982) },
    { FRAC_CONST(0.99885648488998), FRAC_CONST(0.11136095225811) },
    { FRAC_CONST(-0.56649613380432), FRAC_CONST(-0.90494865179062) },
    { FRAC_CONST(0.94138020277023), FRAC_CONST(0.35281917452812) },
    { FRAC_CONST(-0.75725078582764), FRAC_CONST(0.53650552034378) },
    { FRAC_CONST(0.20541973412037), FRAC_CONST(-0.94435143470764) },
    { FRAC_CONST(0.99980372190475), FRAC_CONST(0.79835915565491) },
    { FRAC_CONST(0.29078277945518), FRAC_CONST(0.35393777489662) },
    { FRAC_CONST(-0.62858772277832), FRAC_CONST(0.38765692710876) },
    { FRAC_CONST(0.43440905213356), FRAC_CONST(-0.98546332120895) },
    { FRAC_CONST(-0.98298585414886), FRAC_CONST(0.21021524071693) },
    { FRAC_CONST(0.19513028860092), FRAC_CONST(-0.94239830970764) },
    { FRAC_CONST(-0.95476663112640), FRAC_CONST(0.98364555835724) },
    { FRAC_CONST(0.93379634618759), FRAC_CONST(-0.70881992578506) },
    { FRAC_CONST(-0.85235410928726), FRAC_CONST(-0.08342348039150) },
    { FRAC_CONST(-0.86425095796585), FRAC_CONST(-0.45795026421547) },
    { FRAC_CONST(0.38879778981209), FRAC_CONST(0.97274428606033) },
    { FRAC_CONST(0.92045122385025), FRAC_CONST(-0.62433654069901) },
    { FRAC_CONST(0.89162534475327), FRAC_CONST(0.54950958490372) },
    { FRAC_CONST(-0.36834338307381), FRAC_CONST(0.96458297967911) },
    { FRAC_CONST(0.93891763687134), FRAC_CONST(-0.89968353509903) },
    { FRAC_CONST(0.99267655611038), FRAC_CONST(-0.03757034242153) },
    { FRAC_CONST(-0.94063472747803), FRAC_CONST(0.41332337260246) },
    { FRAC_CONST(0.99740225076675), FRAC_CONST(-0.16830494999886) },
    { FRAC_CONST(-0.35899412631989), FRAC_CONST(-0.46633225679398) },
    { FRAC_CONST(0.05237237364054), FRAC_CONST(-0.25640362501144) },
    { FRAC_CONST(0.36703583598137), FRAC_CONST(-0.38653266429901) },
    { FRAC_CONST(0.91653180122375), FRAC_CONST(-0.30587628483772) },
    { FRAC_CONST(0.69000804424286), FRAC_CONST(0.90952169895172) },
    { FRAC_CONST(-0.38658750057220), FRAC_CONST(0.99501574039459) },
    { FRAC_CONST(-0.29250815510750), FRAC_CONST(0.37444993853569) },
    { FRAC_CONST(-0.60182201862335), FRAC_CONST(0.86779648065567) },
    { FRAC_CONST(-0.97418588399887), FRAC_CONST(0.96468526124954) },
    { FRAC_CONST(0.88461571931839), FRAC_CONST(0.57508403062820) },
    { FRAC_CONST(0.05198933184147), FRAC_CONST(0.21269661188126) },
    { FRAC_CONST(-0.53499621152878), FRAC_CONST(0.97241556644440) },
    { FRAC_CONST(-0.49429559707642), FRAC_CONST(0.98183864355087) },
    { FRAC_CONST(-0.98935145139694), FRAC_CONST(-0.40249159932137) },
    { FRAC_CONST(-0.98081380128860), FRAC_CONST(-0.72856897115707) },
    { FRAC_CONST(-0.27338150143623), FRAC_CONST(0.99950921535492) },
    { FRAC_CONST(0.06310802698135), FRAC_CONST(-0.54539585113525) },
    { FRAC_CONST(-0.20461677014828), FRAC_CONST(-0.14209978282452) },
    { FRAC_CONST(0.66223841905594), FRAC_CONST(0.72528582811356) },
    { FRAC_CONST(-0.84764343500137), FRAC_CONST(0.02372316829860) },
    { FRAC_CONST(-0.89039862155914), FRAC_CONST(0.88866579532623) },
    { FRAC_CONST(0.95903307199478), FRAC_CONST(0.76744925975800) },
    { FRAC_CONST(0.73504126071930), FRAC_CONST(-0.03747203201056) },
    { FRAC_CONST(-0.31744435429573), FRAC_CONST(-0.36834111809731) },
    { FRAC_CONST(-0.34110826253891), FRAC_CONST(0.40211221575737) },
    { FRAC_CONST(0.47803884744644), FRAC_CONST(-0.39423218369484) },
    { FRAC_CONST(0.98299193382263), FRAC_CONST(0.01989791356027) },
    { FRAC_CONST(-0.30963072180748), FRAC_CONST(-0.18076720833778) },
    { FRAC_CONST(0.99992591142654), FRAC_CONST(-0.26281872391701) },
    { FRAC_CONST(-0.93149733543396), FRAC_CONST(-0.98313164710999) },
    { FRAC_CONST(0.99923473596573), FRAC_CONST(-0.80142992734909) },
    { FRAC_CONST(-0.26024168729782), FRAC_CONST(-0.75999760627747) },
    { FRAC_CONST(-0.35712513327599), FRAC_CONST(0.19298963248730) },
    { FRAC_CONST(-0.99899083375931), FRAC_CONST(0.74645155668259) },
    { FRAC_CONST(0.86557173728943), FRAC_CONST(0.55593866109848) },
    { FRAC_CONST(0.33408042788506), FRAC_CONST(0.86185956001282) },
    { FRAC_CONST(0.99010735750198), FRAC_CONST(0.04602397605777) },
    { FRAC_CONST(-0.66694271564484), FRAC_CONST(-0.91643613576889) },
    { FRAC_CONST(0.64016789197922), FRAC_CONST(0.15649530291557) },
    { FRAC_CONST(0.99570536613464), FRAC_CONST(0.45844584703445) },
    { FRAC_CONST(-0.63431465625763), FRAC_CONST(0.21079117059708) },
    { FRAC_CONST(-0.07706847041845), FRAC_CONST(-0.89581435918808) },
    { FRAC_CONST(0.98590087890625), FRAC_CONST(0.88241720199585) },
    { FRAC_CONST(0.80099332332611), FRAC_CONST(-0.36851897835732) },
    { FRAC_CONST(0.78368133306503), FRAC_CONST(0.45506998896599) },
    { FRAC_CONST(0.08707806468010), FRAC_CONST(0.80938994884491) },
    { FRAC_CONST(-0.86811882257462), FRAC_CONST(0.39347308874130) },
    { FRAC_CONST(-0.39466530084610), FRAC_CONST(-0.66809433698654) },
    { FRAC_CONST(0.97875326871872), FRAC_CONST(-0.72467839717865) },
    { FRAC_CONST(-0.95038563013077), FRAC_CONST(0.89563220739365) },
    { FRAC_CONST(0.17005239427090), FRAC_CONST(0.54683053493500) },
    { FRAC_CONST(-0.76910793781281), FRAC_CONST(-0.96226614713669) },
    { FRAC_CONST(0.99743282794952), FRAC_CONST(0.42697158455849) },
    { FRAC_CONST(0.95437383651733), FRAC_CONST(0.97002321481705) },
    { FRAC_CONST(0.99578905105591), FRAC_CONST(-0.54106825590134) },
    { FRAC_CONST(0.28058260679245), FRAC_CONST(-0.85361421108246) },
    { FRAC_CONST(0.85256522893906), FRAC_CONST(-0.64567607641220) },
    { FRAC_CONST(-0.50608539581299), FRAC_CONST(-0.65846014022827) },
    { FRAC_CONST(-0.97210735082626), FRAC_CONST(-0.23095212876797) },
    { FRAC_CONST(0.95424050092697), FRAC_CONST(-0.99240148067474) },
    { FRAC_CONST(-0.96926569938660), FRAC_CONST(0.73775655031204) },
    { FRAC_CONST(0.30872163176537), FRAC_CONST(0.41514959931374) },
    { FRAC_CONST(-0.24523839354515), FRAC_CONST(0.63206630945206) },
    { FRAC_CONST(-0.33813264966011), FRAC_CONST(-0.38661777973175) },
    { FRAC_CONST(-0.05826828256249), FRAC_CONST(-0.06940773874521) },
    { FRAC_CONST(-0.22898460924625), FRAC_CONST(0.97054851055145) },
    { FRAC_CONST(-0.18509915471077), FRAC_CONST(0.47565764188766) },
    { FRAC_CONST(-0.10488238185644), FRAC_CONST(-0.87769949436188) },
    { FRAC_CONST(-0.71886587142944), FRAC_CONST(0.78030979633331) },
    { FRAC_CONST(0.99793875217438), FRAC_CONST(0.90041309595108) },
    { FRAC_CONST(0.57563304901123), FRAC_CONST(-0.91034334897995) },
    { FRAC_CONST(0.28909647464752), FRAC_CONST(0.96307784318924) },
    { FRAC_CONST(0.42188999056816), FRAC_CONST(0.48148649930954) },
    { FRAC_CONST(0.93335050344467), FRAC_CONST(-0.43537023663521) },
    { FRAC_CONST(-0.97087377309799), FRAC_CONST(0.86636447906494) },
    { FRAC_CONST(0.36722871661186), FRAC_CONST(0.65291655063629) },
    { FRAC_CONST(-0.81093025207520), FRAC_CONST(0.08778370171785) },
    { FRAC_CONST(-0.26240602135658), FRAC_CONST(-0.92774093151093) },
    { FRAC_CONST(0.83996498584747), FRAC_CONST(0.55839848518372) },
    { FRAC_CONST(-0.99909615516663), FRAC_CONST(-0.96024608612061) },
    { FRAC_CONST(0.74649465084076), FRAC_CONST(0.12144893407822) },
    { FRAC_CONST(-0.74774593114853), FRAC_CONST(-0.26898062229156) },
    { FRAC_CONST(0.95781666040421), FRAC_CONST(-0.79047924280167) },
    { FRAC_CONST(0.95472306013107), FRAC_CONST(-0.08588775992393) },
    { FRAC_CONST(0.48708331584930), FRAC_CONST(0.99999040365219) },
    { FRAC_CONST(0.46332037448883), FRAC_CONST(0.10964126139879) },
    { FRAC_CONST(-0.76497006416321), FRAC_CONST(0.89210927486420) },
    { FRAC_CONST(0.57397389411926), FRAC_CONST(0.35289704799652) },
    { FRAC_CONST(0.75374317169189), FRAC_CONST(0.96705216169357) },
    { FRAC_CONST(-0.59174400568008), FRAC_CONST(-0.89405369758606) },
    { FRAC_CONST(0.75087904930115), FRAC_CONST(-0.29612672328949) },
    { FRAC_CONST(-0.98607856035233), FRAC_CONST(0.25034910440445) },
    { FRAC_CONST(-0.40761056542397), FRAC_CONST(-0.90045571327209) },
    { FRAC_CONST(0.66929268836975), FRAC_CONST(0.98629492521286) },
    { FRAC_CONST(-0.97463697195053), FRAC_CONST(-0.00190223299433) },
    { FRAC_CONST(0.90145510435104), FRAC_CONST(0.99781388044357) },
    { FRAC_CONST(-0.87259286642075), FRAC_CONST(0.99233585596085) },
    { FRAC_CONST(-0.91529458761215), FRAC_CONST(-0.15698707103729) },
    { FRAC_CONST(-0.03305738791823), FRAC_CONST(-0.37205263972282) },
    { FRAC_CONST(0.07223051041365), FRAC_CONST(-0.88805001974106) },
    { FRAC_CONST(0.99498009681702), FRAC_CONST(0.97094357013702) },
    { FRAC_CONST(-0.74904936552048), FRAC_CONST(0.99985486268997) },
    { FRAC_CONST(0.04585228487849), FRAC_CONST(0.99812334775925) },
    { FRAC_CONST(-0.89054954051971), FRAC_CONST(-0.31791913509369) },
    { FRAC_CONST(-0.83782142400742), FRAC_CONST(0.97637635469437) },
    { FRAC_CONST(0.33454805612564), FRAC_CONST(-0.86231517791748) },
    { FRAC_CONST(-0.99707579612732), FRAC_CONST(0.93237990140915) },
    { FRAC_CONST(-0.22827528417110), FRAC_CONST(0.18874759972095) },
    { FRAC_CONST(0.67248046398163), FRAC_CONST(-0.03646211326122) },
    { FRAC_CONST(-0.05146538093686), FRAC_CONST(-0.92599701881409) },
    { FRAC_CONST(0.99947297573090), FRAC_CONST(0.93625229597092) },
    { FRAC_CONST(0.66951125860214), FRAC_CONST(0.98905825614929) },
    { FRAC_CONST(-0.99602955579758), FRAC_CONST(-0.44654715061188) },
    { FRAC_CONST(0.82104903459549), FRAC_CONST(0.99540740251541) },
    { FRAC_CONST(0.99186509847641), FRAC_CONST(0.72022998332977) },
    { FRAC_CONST(-0.65284591913223), FRAC_CONST(0.52186721563339) },
    { FRAC_CONST(0.93885445594788), FRAC_CONST(-0.74895310401917) },
    { FRAC_CONST(0.96735250949860), FRAC_CONST(0.90891814231873) },
    { FRAC_CONST(-0.22225968539715), FRAC_CONST(0.57124030590057) },
    { FRAC_CONST(-0.44132784008980), FRAC_CONST(-0.92688840627670) },
    { FRAC_CONST(-0.85694974660873), FRAC_CONST(0.88844531774521) },
    { FRAC_CONST(0.91783040761948), FRAC_CONST(-0.46356892585754) },
    { FRAC_CONST(0.72556972503662), FRAC_CONST(-0.99899554252625) },
    { FRAC_CONST(-0.99711579084396), FRAC_CONST(0.58211559057236) },
    { FRAC_CONST(0.77638977766037), FRAC_CONST(0.94321835041046) },
    { FRAC_CONST(0.07717324048281), FRAC_CONST(0.58638399839401) },
    { FRAC_CONST(-0.56049829721451), FRAC_CONST(0.82522302865982) },
    { FRAC_CONST(0.98398894071579), FRAC_CONST(0.39467439055443) },
    { FRAC_CONST(0.47546947002411), FRAC_CONST(0.68613046407700) },
    { FRAC_CONST(0.65675091743469), FRAC_CONST(0.18331636488438) },
    { FRAC_CONST(0.03273375332355), FRAC_CONST(-0.74933111667633) },
    { FRAC_CONST(-0.38684144616127), FRAC_CONST(0.51337349414825) },
    { FRAC_CONST(-0.97346270084381), FRAC_CONST(-0.96549361944199) },
    { FRAC_CONST(-0.53282153606415), FRAC_CONST(-0.91423267126083) },
    { FRAC_CONST(0.99817311763763), FRAC_CONST(0.61133575439453) },
    { FRAC_CONST(-0.50254499912262), FRAC_CONST(-0.88829338550568) },
    { FRAC_CONST(0.01995873264968), FRAC_CONST(0.85223513841629) },
    { FRAC_CONST(0.99930381774902), FRAC_CONST(0.94578897953033) },
    { FRAC_CONST(0.82907766103745), FRAC_CONST(-0.06323442608118) },
    { FRAC_CONST(-0.58660709857941), FRAC_CONST(0.96840775012970) },
    { FRAC_CONST(-0.17573736608028), FRAC_CONST(-0.48166921734810) },
    { FRAC_CONST(0.83434289693832), FRAC_CONST(-0.13023450970650) },
    { FRAC_CONST(0.05946491286159), FRAC_CONST(0.20511047542095) },
    { FRAC_CONST(0.81505483388901), FRAC_CONST(-0.94685947895050) },
    { FRAC_CONST(-0.44976380467415), FRAC_CONST(0.40894573926926) },
    { FRAC_CONST(-0.89746475219727), FRAC_CONST(0.99846577644348) },
    { FRAC_CONST(0.39677256345749), FRAC_CONST(-0.74854665994644) },
    { FRAC_CONST(-0.07588948309422), FRAC_CONST(0.74096214771271) },
    { FRAC_CONST(0.76343196630478), FRAC_CONST(0.41746628284454) },
    { FRAC_CONST(-0.74490106105804), FRAC_CONST(0.94725912809372) },
    { FRAC_CONST(0.64880120754242), FRAC_CONST(0.41336661577225) },
    { FRAC_CONST(0.62319535017014), FRAC_CONST(-0.93098312616348) },
    { FRAC_CONST(0.42215818166733), FRAC_CONST(-0.07712787389755) },
    { FRAC_CONST(0.02704554051161), FRAC_CONST(-0.05417517945170) },
    { FRAC_CONST(0.80001771450043), FRAC_CONST(0.91542196273804) },
    { FRAC_CONST(-0.79351830482483), FRAC_CONST(-0.36208897829056) },
    { FRAC_CONST(0.63872361183167), FRAC_CONST(0.08128252625465) },
    { FRAC_CONST(0.52890521287918), FRAC_CONST(0.60048872232437) },
    { FRAC_CONST(0.74238550662994), FRAC_CONST(0.04491915181279) },
    { FRAC_CONST(0.99096131324768), FRAC_CONST(-0.19451183080673) },
    { FRAC_CONST(-0.80412328243256), FRAC_CONST(-0.88513815402985) },
    { FRAC_CONST(-0.64612615108490), FRAC_CONST(0.72198677062988) },
    { FRAC_CONST(0.11657770723104), FRAC_CONST(-0.83662831783295) },
    { FRAC_CONST(-0.95053184032440), FRAC_CONST(-0.96939903497696) },
    { FRAC_CONST(-0.62228870391846), FRAC_CONST(0.82767260074615) },
    { FRAC_CONST(0.03004475869238), FRAC_CONST(-0.99738895893097) },
    { FRAC_CONST(-0.97987216711044), FRAC_CONST(0.36526128649712) },
    { FRAC_CONST(-0.99986982345581), FRAC_CONST(-0.36021611094475) },
    { FRAC_CONST(0.89110648632050), FRAC_CONST(-0.97894251346588) },
    { FRAC_CONST(0.10407960414886), FRAC_CONST(0.77357792854309) },
    { FRAC_CONST(0.95964735746384), FRAC_CONST(-0.35435819625854) },
    { FRAC_CONST(0.50843232870102), FRAC_CONST(0.96107691526413) },
    { FRAC_CONST(0.17006334662437), FRAC_CONST(-0.76854026317596) },
    { FRAC_CONST(0.25872674584389), FRAC_CONST(0.99893301725388) },
    { FRAC_CONST(-0.01115998718888), FRAC_CONST(0.98496019840240) },
    { FRAC_CONST(-0.79598701000214), FRAC_CONST(0.97138410806656) },
    { FRAC_CONST(-0.99264711141586), FRAC_CONST(-0.99542820453644) },
    { FRAC_CONST(-0.99829661846161), FRAC_CONST(0.01877138763666) },
    { FRAC_CONST(-0.70801013708115), FRAC_CONST(0.33680686354637) },
    { FRAC_CONST(-0.70467054843903), FRAC_CONST(0.93272775411606) },
    { FRAC_CONST(0.99846023321152), FRAC_CONST(-0.98725748062134) },
    { FRAC_CONST(-0.63364970684052), FRAC_CONST(-0.16473594307899) },
    { FRAC_CONST(-0.16258217394352), FRAC_CONST(-0.95939123630524) },
    { FRAC_CONST(-0.43645593523979), FRAC_CONST(-0.94805032014847) },
    { FRAC_CONST(-0.99848473072052), FRAC_CONST(0.96245169639587) },
    { FRAC_CONST(-0.16796459257603), FRAC_CONST(-0.98987513780594) },
    { FRAC_CONST(-0.87979227304459), FRAC_CONST(-0.71725726127625) },
    { FRAC_CONST(0.44183099269867), FRAC_CONST(-0.93568974733353) },
    { FRAC_CONST(0.93310177326202), FRAC_CONST(-0.99913311004639) },
    { FRAC_CONST(-0.93941932916641), FRAC_CONST(-0.56409376859665) },
    { FRAC_CONST(-0.88590002059937), FRAC_CONST(0.47624599933624) },
    { FRAC_CONST(0.99971461296082), FRAC_CONST(-0.83889955282211) },
    { FRAC_CONST(-0.75376385450363), FRAC_CONST(0.00814643409103) },
    { FRAC_CONST(0.93887686729431), FRAC_CONST(-0.11284527927637) },
    { FRAC_CONST(0.85126435756683), FRAC_CONST(0.52349251508713) },
    { FRAC_CONST(0.39701420068741), FRAC_CONST(0.81779634952545) },
    { FRAC_CONST(-0.37024465203285), FRAC_CONST(-0.87071657180786) },
    { FRAC_CONST(-0.36024826765060), FRAC_CONST(0.34655734896660) },
    { FRAC_CONST(-0.93388813734055), FRAC_CONST(-0.84476542472839) },
    { FRAC_CONST(-0.65298801660538), FRAC_CONST(-0.18439576029778) },
    { FRAC_CONST(0.11960318684578), FRAC_CONST(0.99899345636368) },
    { FRAC_CONST(0.94292563199997), FRAC_CONST(0.83163905143738) },
    { FRAC_CONST(0.75081145763397), FRAC_CONST(-0.35533222556114) },
    { FRAC_CONST(0.56721979379654), FRAC_CONST(-0.24076835811138) },
    { FRAC_CONST(0.46857765316963), FRAC_CONST(-0.30140233039856) },
    { FRAC_CONST(0.97312313318253), FRAC_CONST(-0.99548190832138) },
    { FRAC_CONST(-0.38299977779388), FRAC_CONST(0.98516911268234) },
    { FRAC_CONST(0.41025799512863), FRAC_CONST(0.02116736955941) },
    { FRAC_CONST(0.09638062119484), FRAC_CONST(0.04411984235048) },
    { FRAC_CONST(-0.85283249616623), FRAC_CONST(0.91475564241409) },
    { FRAC_CONST(0.88866806030273), FRAC_CONST(-0.99735265970230) },
    { FRAC_CONST(-0.48202428221703), FRAC_CONST(-0.96805608272552) },
    { FRAC_CONST(0.27572581171989), FRAC_CONST(0.58634752035141) },
    { FRAC_CONST(-0.65889132022858), FRAC_CONST(0.58835631608963) },
    { FRAC_CONST(0.98838084936142), FRAC_CONST(0.99994349479675) },
    { FRAC_CONST(-0.20651349425316), FRAC_CONST(0.54593044519424) },
    { FRAC_CONST(-0.62126415967941), FRAC_CONST(-0.59893679618835) },
    { FRAC_CONST(0.20320105552673), FRAC_CONST(-0.86879181861877) },
    { FRAC_CONST(-0.97790551185608), FRAC_CONST(0.96290808916092) },
    { FRAC_CONST(0.11112534999847), FRAC_CONST(0.21484763920307) },
    { FRAC_CONST(-0.41368338465691), FRAC_CONST(0.28216838836670) },
    { FRAC_CONST(0.24133038520813), FRAC_CONST(0.51294362545013) },
    { FRAC_CONST(-0.66393411159515), FRAC_CONST(-0.08249679952860) },
    { FRAC_CONST(-0.53697830438614), FRAC_CONST(-0.97649902105331) },
    { FRAC_CONST(-0.97224736213684), FRAC_CONST(0.22081333398819) },
    { FRAC_CONST(0.87392479181290), FRAC_CONST(-0.12796173989773) },
    { FRAC_CONST(0.19050361216068), FRAC_CONST(0.01602615416050) },
    { FRAC_CONST(-0.46353441476822), FRAC_CONST(-0.95249038934708) },
    { FRAC_CONST(-0.07064096629620), FRAC_CONST(-0.94479805231094) },
    { FRAC_CONST(-0.92444086074829), FRAC_CONST(-0.10457590222359) },
    { FRAC_CONST(-0.83822596073151), FRAC_CONST(-0.01695043221116) },
    { FRAC_CONST(0.75214684009552), FRAC_CONST(-0.99955683946609) },
    { FRAC_CONST(-0.42102998495102), FRAC_CONST(0.99720942974091) },
    { FRAC_CONST(-0.72094786167145), FRAC_CONST(-0.35008960962296) },
    { FRAC_CONST(0.78843313455582), FRAC_CONST(0.52851396799088) },
    { FRAC_CONST(0.97394025325775), FRAC_CONST(-0.26695942878723) },
    { FRAC_CONST(0.99206465482712), FRAC_CONST(-0.57010120153427) },
    { FRAC_CONST(0.76789611577988), FRAC_CONST(-0.76519358158112) },
    { FRAC_CONST(-0.82002419233322), FRAC_CONST(-0.73530179262161) },
    { FRAC_CONST(0.81924992799759), FRAC_CONST(0.99698424339294) },
    { FRAC_CONST(-0.26719850301743), FRAC_CONST(0.68903368711472) },
    { FRAC_CONST(-0.43311259150505), FRAC_CONST(0.85321813821793) },
    { FRAC_CONST(0.99194979667664), FRAC_CONST(0.91876250505447) },
    { FRAC_CONST(-0.80691999197006), FRAC_CONST(-0.32627540826797) },
    { FRAC_CONST(0.43080005049706), FRAC_CONST(-0.21919095516205) },
    { FRAC_CONST(0.67709493637085), FRAC_CONST(-0.95478075742722) },
    { FRAC_CONST(0.56151771545410), FRAC_CONST(-0.70693808794022) },
    { FRAC_CONST(0.10831862688065), FRAC_CONST(-0.08628837019205) },
    { FRAC_CONST(0.91229414939880), FRAC_CONST(-0.65987348556519) },
    { FRAC_CONST(-0.48972892761230), FRAC_CONST(0.56289243698120) },
    { FRAC_CONST(-0.89033657312393), FRAC_CONST(-0.71656566858292) },
    { FRAC_CONST(0.65269446372986), FRAC_CONST(0.65916007757187) },
    { FRAC_CONST(0.67439478635788), FRAC_CONST(-0.81684380769730) },
    { FRAC_CONST(-0.47770830988884), FRAC_CONST(-0.16789555549622) },
    { FRAC_CONST(-0.99715977907181), FRAC_CONST(-0.93565785884857) },
    { FRAC_CONST(-0.90889590978622), FRAC_CONST(0.62034398317337) },
    { FRAC_CONST(-0.06618622690439), FRAC_CONST(-0.23812216520309) },
    { FRAC_CONST(0.99430269002914), FRAC_CONST(0.18812555074692) },
    { FRAC_CONST(0.97686403989792), FRAC_CONST(-0.28664535284042) },
    { FRAC_CONST(0.94813650846481), FRAC_CONST(-0.97506642341614) },
    { FRAC_CONST(-0.95434498786926), FRAC_CONST(-0.79607981443405) },
    { FRAC_CONST(-0.49104782938957), FRAC_CONST(0.32895213365555) },
    { FRAC_CONST(0.99881172180176), FRAC_CONST(0.88993984460831) },
    { FRAC_CONST(0.50449168682098), FRAC_CONST(-0.85995072126389) },
    { FRAC_CONST(0.47162890434265), FRAC_CONST(-0.18680204451084) },
    { FRAC_CONST(-0.62081581354141), FRAC_CONST(0.75000673532486) },
    { FRAC_CONST(-0.43867015838623), FRAC_CONST(0.99998068809509) },
    { FRAC_CONST(0.98630565404892), FRAC_CONST(-0.53578901290894) },
    { FRAC_CONST(-0.61510360240936), FRAC_CONST(-0.89515018463135) },
    { FRAC_CONST(-0.03841517493129), FRAC_CONST(-0.69888818264008) },
    { FRAC_CONST(-0.30102157592773), FRAC_CONST(-0.07667808979750) },
    { FRAC_CONST(0.41881284117699), FRAC_CONST(0.02188098989427) },
    { FRAC_CONST(-0.86135452985764), FRAC_CONST(0.98947483301163) },
    { FRAC_CONST(0.67226862907410), FRAC_CONST(-0.13494388759136) },
    { FRAC_CONST(-0.70737397670746), FRAC_CONST(-0.76547348499298) },
    { FRAC_CONST(0.94044947624207), FRAC_CONST(0.09026201069355) },
    { FRAC_CONST(-0.82386350631714), FRAC_CONST(0.08924768865108) },
    { FRAC_CONST(-0.32070666551590), FRAC_CONST(0.50143420696259) },
    { FRAC_CONST(0.57593160867691), FRAC_CONST(-0.98966425657272) },
    { FRAC_CONST(-0.36326017975807), FRAC_CONST(0.07440242916346) },
    { FRAC_CONST(0.99979043006897), FRAC_CONST(-0.14130286872387) },
    { FRAC_CONST(-0.92366021871567), FRAC_CONST(-0.97979295253754) },
    { FRAC_CONST(-0.44607177376747), FRAC_CONST(-0.54233253002167) },
    { FRAC_CONST(0.44226801395416), FRAC_CONST(0.71326756477356) },
    { FRAC_CONST(0.03671907261014), FRAC_CONST(0.63606387376785) },
    { FRAC_CONST(0.52175426483154), FRAC_CONST(-0.85396826267242) },
    { FRAC_CONST(-0.94701141119003), FRAC_CONST(-0.01826348155737) },
    { FRAC_CONST(-0.98759609460831), FRAC_CONST(0.82288712263107) },
    { FRAC_CONST(0.87434792518616), FRAC_CONST(0.89399492740631) },
    { FRAC_CONST(-0.93412041664124), FRAC_CONST(0.41374051570892) },
    { FRAC_CONST(0.96063941717148), FRAC_CONST(0.93116706609726) },
    { FRAC_CONST(0.97534251213074), FRAC_CONST(0.86150932312012) },
    { FRAC_CONST(0.99642467498779), FRAC_CONST(0.70190042257309) },
    { FRAC_CONST(-0.94705086946487), FRAC_CONST(-0.29580041766167) },
    { FRAC_CONST(0.91599804162979), FRAC_CONST(-0.98147833347321) }
};

#ifdef __cplusplus
}
#endif
#endif

