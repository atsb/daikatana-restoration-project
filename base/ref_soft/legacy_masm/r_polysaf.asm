 .386P
 .model FLAT
;
; d_polysa.s
; x86 assembly-language polygon model drawing code
;

include qasm.inc
include d_if.inc

if	id386

; !!! if this is changed, it must be changed in d_polyse.c too !!!
;DPS_MAXSPANS			equ		(MAXHEIGHT+1)
; 1 extra for spanpackage that marks end

;SPAN_SIZE	equ		(((DPS_MAXSPANS + 1 + ((CACHE_SIZE - 1) / spanpackage_t_size)) + 1) * spanpackage_t_size)

MASK_1K	equ		03FFh

_DATA SEGMENT	

 align 4	
;p10_minus_p20 dd 0
;p01_minus_p21 dd 0
;temp0 dd 0
;temp1 dd 0
;Ltemp dd 0

aff8entryvec_table dd LDraw8, LDraw7, LDraw6, LDraw5
 dd LDraw4, LDraw3, LDraw2, LDraw1

lzistepx dd 0	

 externdef _rand1k:dword	
 externdef _rand1k_index:dword	
 externdef _alias_colormap:dword
 externdef _skin_colormap:dword
 externdef _alias_fogmap:dword
 externdef _alias_fogz:dword

_DATA ENDS
_TEXT SEGMENT	


;----------------------------------------------------------------------
; 8-bpp horizontal span drawing code for affine polygons, with smooth
; shading and no transparency
;----------------------------------------------------------------------

pspans	equ		4+8

 public _D_PolysetAff8Start_f	
_D_PolysetAff8Start_f:	

 public _R_PolysetDrawSpans8_Opaque_f 
_R_PolysetDrawSpans8_Opaque_f:

 push esi	; preserve register variables
 push ebx	

 mov esi,ds:dword ptr[pspans+esp]	; point to the first span descriptor
 mov ecx,ds:dword ptr[_r_zistepx]	

 push ebp	; preserve caller's stack frame
 push edi	

 ror ecx,16	; put high 16 bits of 1/z step in low word
 mov edx,ds:dword ptr[spanpackage_t_count+esi]	

 mov ds:dword ptr[lzistepx],ecx	

LSpanLoop:	

;		lcount = d_aspancount - pspanpackage->count;
;
;		errorterm += erroradjustup;
;		if (errorterm >= 0)
;		{
;			d_aspancount += d_countextrastep;
;			errorterm -= erroradjustdown;
;		}
;		else
;		{
;			d_aspancount += ubasestep;
;		}

 mov eax,ds:dword ptr[_d_aspancount]
 sub eax,edx

 mov edx,ds:dword ptr[_erroradjustup]	
 mov ebx,ds:dword ptr[_errorterm]	
 add ebx,edx	
 js LNoTurnover	

 mov edx,ds:dword ptr[_erroradjustdown]	
 mov edi,ds:dword ptr[_d_countextrastep]	
 sub ebx,edx	
 mov ebp,ds:dword ptr[_d_aspancount]	
 mov ds:dword ptr[_errorterm],ebx	
 add ebp,edi	
 mov ds:dword ptr[_d_aspancount],ebp	
 jmp LRightEdgeStepped	

LNoTurnover:	
 mov edi,ds:dword ptr[_d_aspancount]	
 mov edx,ds:dword ptr[_ubasestep]	
 mov ds:dword ptr[_errorterm],ebx	
 add edi,edx	
 mov ds:dword ptr[_d_aspancount],edi	

LRightEdgeStepped:	
 cmp eax,1	

 jl LNextSpan	
 jz LExactlyOneLong	

;
; set up advancetable
;
 mov ecx,ds:dword ptr[_a_ststepxwhole]	
 mov edx,ds:dword ptr[_r_affinetridesc+atd_skinwidth]	

 mov ds:dword ptr[advancetable+4],ecx	; advance base in t
 add ecx,edx	

 mov ds:dword ptr[advancetable],ecx	; advance extra in t
 mov ecx,ds:dword ptr[_a_tstepxfrac]	

 mov cx,ds:word ptr[_r_lstepx]	
 mov edx,eax	; count

 mov ds:dword ptr[tstep],ecx
 add edx,7	

 shr edx,3	; count of full and partial loops
 mov ebx,ds:dword ptr[spanpackage_t_sfrac+esi]	

 mov bx,dx	
 mov ecx,ds:dword ptr[spanpackage_t_pz+esi]	

 neg eax	

 mov edi,ds:dword ptr[spanpackage_t_pdest+esi]	
 and eax,7	; 0->0, 1->7, 2->6, ... , 7->1

 sub edi,eax	; compensate for hardwired offsets
 sub edi,eax	; compensate for hardwired offsets

 sub ecx,eax	

 sub ecx,eax	
 mov edx,ds:dword ptr[spanpackage_t_tfrac+esi]	

 mov dx,ds:word ptr[spanpackage_t_light+esi]	
 mov ebp,ds:dword ptr[spanpackage_t_zi+esi]	

 ror ebp,16	; put high 16 bits of 1/z in low word
 push esi	

 mov esi,ds:dword ptr[spanpackage_t_ptex+esi]	
 jmp dword ptr[aff8entryvec_table+eax*4]	

; %bx = count of full and partial loops
; %ebx high word = sfrac
; %ecx = pz
; %dx = light
; %edx high word = tfrac
; %esi = ptex
; %edi = pdest
; %ebp = 1/z
; tstep low word = C(r_lstepx)
; tstep high word = C(a_tstepxfrac)
; C(a_sstepxfrac) low word = 0
; C(a_sstepxfrac) high word = C(a_sstepxfrac)

LDrawLoop:	

; FIXME: do we need to clamp light? We may need at least a buffer bit to
; keep it from poking into tfrac and causing problems

LDraw8:	
 cmp bp,ds:word ptr[ecx]	
 jl Lp1	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch8:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch8:
 mov ds:word ptr[edi],ax	
Lp1:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw7:	
 cmp bp,ds:word ptr[2+ecx]	
 jl Lp2	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[2+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch7:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch7:
 mov ds:word ptr[2+edi],ax	
Lp2:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw6:	
 cmp bp,ds:word ptr[4+ecx]	
 jl Lp3	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[4+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch6:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch6:
 mov ds:word ptr[4+edi],ax	
Lp3:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw5:	
 cmp bp,ds:word ptr[6+ecx]	
 jl Lp4	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[6+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch5:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch5:
 mov ds:word ptr[6+edi],ax	
Lp4:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw4:	
 cmp bp,ds:word ptr[8+ecx]	
 jl Lp5	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[8+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch4:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch4:
 mov ds:word ptr[8+edi],ax	
Lp5:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw3:	
 cmp bp,ds:word ptr[10+ecx]	
 jl Lp6	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[10+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch3:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch3:
 mov ds:word ptr[10+edi],ax	
Lp6:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw2:	
 cmp bp,ds:word ptr[12+ecx]	
 jl Lp7	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[12+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch2:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch2:
 mov ds:word ptr[12+edi],ax	
Lp7:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

LDraw1:	
 cmp bp,ds:word ptr[14+ecx]	
 jl Lp8	
 xor eax,eax	
 mov ah,dh	
 mov al,ds:byte ptr[esi]	
 mov ds:word ptr[14+ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch1:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch1:
 mov ds:word ptr[14+edi],ax	
Lp8:	
 add edx,ds:dword ptr[tstep]	
 sbb eax,eax	
 add ebp,ds:dword ptr[lzistepx]	
 adc ebp,0	
 add ebx,ds:dword ptr[_a_sstepxfrac]	
 adc esi,ds:dword ptr[advancetable+4+eax*4]	

 ;add edi,8	
 add edi,16	
 add ecx,16	

 dec bx	
 jnz LDrawLoop	

 pop esi	; restore spans pointer
LNextSpan:	
 add esi,offset spanpackage_t_size	; point to next span
LNextSpanESISet:	
 mov edx,ds:dword ptr[spanpackage_t_count+esi]	
 cmp edx,offset -999999	; any more spans?
 jnz LSpanLoop	; yes

 pop edi	
 pop ebp	; restore the caller's stack frame
 pop ebx	; restore register variables
 pop esi	
 ret	


; draw a one-long span

LExactlyOneLong:	

 mov ecx,ds:dword ptr[spanpackage_t_pz+esi]	
 mov ebp,ds:dword ptr[spanpackage_t_zi+esi]	

 ror ebp,16	; put high 16 bits of 1/z in low word
 mov ebx,ds:dword ptr[spanpackage_t_ptex+esi]	

 cmp bp,ds:word ptr[ecx]	
 jl LNextSpan	
 xor eax,eax	
 mov edi,ds:dword ptr[spanpackage_t_pdest+esi]	
 mov ah,ds:byte ptr[spanpackage_t_light+1+esi]	
 add esi,offset spanpackage_t_size	; point to next span
 mov al,ds:byte ptr[ebx]	
 mov ds:word ptr[ecx],bp	
 mov ax,ds:word ptr[12345678h+eax*2]	
LPatch9:	
 ;mov ah,ds:byte ptr[_alias_fogz]			; added for daikatana
 ;mov ax,ds:word ptr[12345678h+eax*2]			; added for daikatana
LFPatch9:
 mov ds:word ptr[edi],ax	

 jmp LNextSpanESISet	

 public _D_Aff8Patch_f	
_D_Aff8Patch_f:	
 mov eax,[_skin_colormap]
 mov ds:dword ptr[LPatch1-4],eax	
 mov ds:dword ptr[LPatch2-4],eax	
 mov ds:dword ptr[LPatch3-4],eax	
 mov ds:dword ptr[LPatch4-4],eax	
 mov ds:dword ptr[LPatch5-4],eax	
 mov ds:dword ptr[LPatch6-4],eax	
 mov ds:dword ptr[LPatch7-4],eax	
 mov ds:dword ptr[LPatch8-4],eax	
 mov ds:dword ptr[LPatch9-4],eax	
 ;mov eax,[_alias_fogmap]
 ;mov ds:dword ptr[LFPatch1-4],eax	
 ;mov ds:dword ptr[LFPatch2-4],eax	
 ;mov ds:dword ptr[LFPatch3-4],eax	
 ;mov ds:dword ptr[LFPatch4-4],eax	
 ;mov ds:dword ptr[LFPatch5-4],eax	
 ;mov ds:dword ptr[LFPatch6-4],eax	
 ;mov ds:dword ptr[LFPatch7-4],eax	
 ;mov ds:dword ptr[LFPatch8-4],eax	
 ;mov ds:dword ptr[LFPatch9-4],eax	

 ret	

_TEXT ENDS
endif	;id386
 END
