 .386P
 .model FLAT
;
; d_draw16.s
; x86 assembly-language horizontal 8-bpp span-drawing code, with 16-pixel
; subdivision.
;

include qasm.inc
include d_if.inc

if	id386

;----------------------------------------------------------------------
; 8-bpp horizontal span drawing code for polygons, with no transparency and
; 16-pixel subdivision.
;
; Assumes there is at least one span in pspans, and that every span
; contains at least one pixel
;----------------------------------------------------------------------

_DATA SEGMENT	

 externdef _alias_fogmap:dword
 externdef _alias_fogdiv:dword
 externdef fog_z:dword

_DATA ENDS
_TEXT SEGMENT	

; out-of-line, rarely-needed clamping code

LFogClampHigh0:	
 mov esi,ds:dword ptr[_bbextents]	
 jmp LFogClampReentry0	
LFogClampHighOrLow0:	
 jg LFogClampHigh0	
 xor esi,esi	
 jmp LFogClampReentry0	

LFogClampHigh1:	
 mov edx,ds:dword ptr[_bbextentt]	
 jmp LFogClampReentry1	
LFogClampHighOrLow1:	
 jg LFogClampHigh1	
 xor edx,edx	
 jmp LFogClampReentry1	

LFogClampLow2:	
 mov ebp,4096	
 jmp LFogClampReentry2	
LFogClampHigh2:	
 mov ebp,ds:dword ptr[_bbextents]	
 jmp LFogClampReentry2	

LFogClampLow3:	
 mov ecx,4096	
 jmp LFogClampReentry3	
LFogClampHigh3:	
 mov ecx,ds:dword ptr[_bbextentt]	
 jmp LFogClampReentry3	

LFogClampLow4:	
 mov eax,4096	
 jmp LFogClampReentry4	
LFogClampHigh4:	
 mov eax,ds:dword ptr[_bbextents]	
 jmp LFogClampReentry4	

LFogClampLow5:	
 mov ebx,4096	
 jmp LFogClampReentry5	
LFogClampHigh5:	
 mov ebx,ds:dword ptr[_bbextentt]	
 jmp LFogClampReentry5	

LFogClamp0:
 mov ds:dword ptr[fog_z],31
 jmp LFogClampReentry0
LFogClamp1:
 mov ds:dword ptr[fog_z],31
 jmp LFogClampReentry1
LFogClamp2:
 mov ds:dword ptr[fog_z],31
 jmp LFogClampReentry2
LFogClamp3:
 mov ds:dword ptr[fog_z],31
 jmp LFogClampReentry3
;LFogClamp4:
; mov ds:dword ptr[fog_z],31
; jmp LFogClampReentry4

pspans	equ		4+16

 align 4	
 public _D_DrawSpans16_f	
_D_DrawSpans16_f:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

;
; set up scaled-by-16 steps, for 16-long segments; also set up cacheblock
; and span list pointers
;
; TODO: any overlap from rearranging?
 fld ds:dword ptr[_d_sdivzstepu]	; d_sdivzstepu
 fmul ds:dword ptr[fp_16]			; d_sdivzstepu*fp_16
 mov edx,ds:dword ptr[_cacheblock]	; edx = &cacheblock
 shr edx,1							; half cacheblock start for 16 bit addressing (i.e. esi*2)
 fld ds:dword ptr[_d_tdivzstepu]	; d_tdivzstepu | d_sdivzstepu*fp_16
 fmul ds:dword ptr[fp_16]			; d_tdivzstepu*fp_16 | d_sdivzstepu*fp_16
 mov ebx,ds:dword ptr[pspans+esp]	; point to the first span descriptor
 fld ds:dword ptr[_d_zistepu]		; d_zistepu | d_tdivzstepu*fp_16 | d_sdivzstepu*fp_16
 fmul ds:dword ptr[fp_16]			; d_zistepu*fp_16 | d_tdivzstepu*fp_16 | d_sdivzstepu*fp_16
 mov ds:dword ptr[pbase],edx		; pbase = cacheblock
 fstp ds:dword ptr[zi16stepu]		; zi16stepu = d_zistepu*fp_16
 fstp ds:dword ptr[tdivz16stepu]	; tdivz16stepu = d_tdivzstepu*fp_16
 fstp ds:dword ptr[sdivz16stepu]	; sdivz16stepu = d_sdivzstepu*fp_16

LSpanLoop:	
;
; set up the initial s/z, t/z, and 1/z on the FP stack, and generate the
; initial s and t values
;
; FIXME: pipeline FILD?
 fild ds:dword ptr[espan_t_v+ebx]	; dv
 fild ds:dword ptr[espan_t_u+ebx]	; dv | du

 fld st(1)							; dv | du | dv
 fmul ds:dword ptr[_d_sdivzstepv]	; dv*d_sdivzstepv | du | dv
 fld st(1)							; du | dv*d_sdivzstepv | du | dv
 fmul ds:dword ptr[_d_sdivzstepu]	; du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
 fld st(2)							; du | du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
 fmul ds:dword ptr[_d_tdivzstepu]	; du*d_tdivzstepu | du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
 fxch st(1)							; du*d_sdivzstepu | du*d_tdivzstepu | dv*d_sdivzstepv | du | dv
 faddp st(2),st(0)					; du*d_tdivzstepu | du*d_sdivzstepu + dv*d_sdivzstepv | du | dv
 fxch st(1)							; du*d_sdivzstepu + dv*d_sdivzstepv | du*d_tdivzstepu | du | dv
 fld st(3)							; dv | du*d_sdivzstepu + dv*d_sdivzstepv | du*d_tdivzstepu | du | dv
 fmul ds:dword ptr[_d_tdivzstepv]	; dv*d_tdivzstepv | du*d_sdivzstepu + dv*d_sdivzstepv | du*d_tdivzstepu | du | dv
 fxch st(1)							; du*d_sdivzstepu + dv*d_sdivzstepv | dv*d_tdivzstepv | du*d_tdivzstepu | du | dv
 fadd ds:dword ptr[_d_sdivzorigin]	; sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu
									; stays in %st(2) at end
 fxch st(4)							; dv | dv*d_tdivzstepv | du*d_tdivzstepu | du |	s/z
 fmul ds:dword ptr[_d_zistepv]		; dv*d_zistepv | dv*d_tdivzstepv | du*d_tdivzstepu | du | s/z
 fxch st(1)							; dv*d_tdivzstepv |  dv*d_zistepv | du*d_tdivzstepu | du | s/z
 faddp st(2),st(0)					; dv*d_zistepv | dv*d_tdivzstepv + du*d_tdivzstepu | du | s/z
 fxch st(2)							; du | dv*d_tdivzstepv + du*d_tdivzstepu | dv*d_zistepv | s/z
 fmul ds:dword ptr[_d_zistepu]		; du*d_zistepu | dv*d_tdivzstepv + du*d_tdivzstepu | dv*d_zistepv | s/z
 fxch st(1)							; dv*d_tdivzstepv + du*d_tdivzstepu | du*d_zistepu | dv*d_zistepv | s/z
 fadd ds:dword ptr[_d_tdivzorigin]	; tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu; stays in %st(1) at end
 fxch st(2)							; dv*d_zistepv | du*d_zistepu | t/z | s/z
 faddp st(1),st(0)					; dv*d_zistepv + du*d_zistepu | t/z | s/z

 fld ds:dword ptr[fp_64k]			; fp_64k | dv*d_zistepv + du*d_zistepu | t/z | s/z
 fxch st(1)							; dv*d_zistepv + du*d_zistepu | fp_64k | t/z | s/z
 fadd ds:dword ptr[_d_ziorigin]		; zi = d_ziorigin + dv*d_zistepv + du*d_zistepu
									; stays in %st(0) at end
									; 1/z | fp_64k | t/z | s/z

;
; calculate and clamp s & t
;
 fdiv st(1),st(0)					; 1/z | z*64k | t/z | s/z

;
; point %edi to the first pixel in the span
;
 mov ecx,ds:dword ptr[_d_viewbuffer]		; ecx = &d_viewbuffer
 mov eax,ds:dword ptr[espan_t_v+ebx]		; eax = v
 mov ds:dword ptr[pspantemp],ebx			; preserve spans pointer

 mov edx,ds:dword ptr[_tadjust]				; edx = tadjust
 mov esi,ds:dword ptr[_sadjust]				; esi = sadjust
 mov edi,ds:dword ptr[_d_scantable+eax*4]	; edi = v * screenwidth
 add edi,ecx								; edi = v * screenwidth + d_viewbuffer
 mov ecx,ds:dword ptr[espan_t_u+ebx]		; ecx = u
 shl ecx,1
 add edi,ecx								; edi = v * screenwidth + d_viewbuffer + u
 mov ecx,ds:dword ptr[espan_t_count+ebx]	; ecx = count

;
; now start the FDIV for the end of the span
;
 cmp ecx,16									;
 ja LSetupNotLast1	

 dec ecx	
 jz LCleanup1								; if only one pixel, no need to start an FDIV
 mov ds:dword ptr[spancountminus1],ecx		; spancountminus1 = count - 1

; finish up the s and t calcs
 fxch st(1)							; z*64k | 1/z | t/z | s/z
 fld st(0)							; z*64k | z*64k | 1/z | t/z | s/z
 fmul st(0),st(4)					; s | z*64k | 1/z | t/z | s/z			NOTE:(s = z*64k * s/z)
 fxch st(1)							; z*64k | s | 1/z | t/z | s/z
 fmul st(0),st(3)					; t | s | 1/z | t/z | s/z				NOTE:(t = z*64k * t/z)
 fxch st(1)							; s | t | 1/z | t/z | s/z
 fistp ds:dword ptr[s]				; 1/z | t | t/z | s/z
 fistp ds:dword ptr[t]				; 1/z | t/z | s/z

 fild ds:dword ptr[spancountminus1]	; spancountminus1 | 1/z | t/z | s/z

;NOTE scm1 refers to spancountminus1

 fld ds:dword ptr[_d_tdivzstepu]	; C(d_tdivzstepu) | scm1
 fld ds:dword ptr[_d_zistepu]		; C(d_zistepu) | C(d_tdivzstepu) | scm1
 fmul st(0),st(2)					; C(d_zistepu)*scm1 | C(d_tdivzstepu) | scm1					
 fxch st(1)							; C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
 fmul st(0),st(2)					; C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
 fxch st(2)							; scm1 | C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1
 fmul ds:dword ptr[_d_sdivzstepu]	; C(d_sdivzstepu)*scm1 | C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1
 fxch st(1)							; C(d_zistepu)*scm1 | C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
 faddp st(3),st(0)					; C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
 fxch st(1)							; C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
 faddp st(3),st(0)					; C(d_sdivzstepu)*scm1
 faddp st(3),st(0)	

 fld ds:dword ptr[fp_64k]	
 fdiv st(0),st(1)					; this is what we've gone to all this trouble to overlap
 jmp LFDIVInFlight1	

LCleanup1:	
; finish up the s and t calcs
 fxch st(1)							; z*64k | 1/z | t/z | s/z

 fld st(0)							; z*64k | z*64k | 1/z | t/z | s/z
 fmul st(0),st(4)					; s | z*64k | 1/z | t/z | s/z
 fxch st(1)							; z*64k | s | 1/z | t/z | s/z
 fmul st(0),st(3)					; t | s | 1/z | t/z | s/z
 fxch st(1)							; s | t | 1/z | t/z | s/z
 fistp ds:dword ptr[s]				; 1/z | t | t/z | s/z
 fistp ds:dword ptr[t]				; 1/z | t/z | s/z
 jmp LFDIVInFlight1	

 align 4	
LSetupNotLast1:	
; finish up the s and t calcs
 fxch st(1)							; z*64k | 1/z | t/z | s/z

 fld st(0)							; z*64k | z*64k | 1/z | t/z | s/z
 fmul st(0),st(4)					; s | z*64k | 1/z | t/z | s/z
 fxch st(1)							; z*64k | s | 1/z | t/z | s/z
 fmul st(0),st(3)					; t | s | 1/z | t/z | s/z
 fxch st(1)							; s | t | 1/z | t/z | s/z
 fistp ds:dword ptr[s]				; 1/z | t | t/z | s/z
 fistp ds:dword ptr[t]				; 1/z | t/z | s/z

 fadd ds:dword ptr[zi16stepu]		; 1/z+zi16stepu | t/z | s/z
 fxch st(2)							; s/z | t/z | 1/z+zi16stepu
 fadd ds:dword ptr[sdivz16stepu]	; s/z+sdivz16stepu | t/z | 1/z+zi16stepu
 fxch st(2)							; 1/z+zi16stepu | t/z | s/z+sdivz16stepu
 fld ds:dword ptr[tdivz16stepu]		; tdivz16stepu | 1/z+zi16stepu | t/z | s/z+sdivz16stepu
 faddp st(2),st(0)					; 1/z+zi16stepu | t/z+tdivz16stepu | s/z+sdivz16stepu
 fld ds:dword ptr[fp_64k]			; fp_64k | 1/z+zi16stepu | t/z+tdivz16stepu | s/z+sdivz16stepu
 fdiv st(0),st(1)					; z = 1/1/z | 1/z+zi16stepu | t/z+tdivz16stepu | s/z+sdivz16stepu
									; this is what we've gone to all this trouble to overlap
LFDIVInFlight1:	
; AT THIS POINT:
; esi = sadjust
; edi = viewbuffer
; eax = v
; ebx = unused
; ecx = count - 1
; edx = tadjust

 add esi,ds:dword ptr[s]			; sadjust += s
 add edx,ds:dword ptr[t]			; tadjust += t
 mov ebx,ds:dword ptr[_bbextents]	; ebx = bbextents
 mov ebp,ds:dword ptr[_bbextentt]	; ebp = bbextentt
 cmp esi,ebx						; clamp sadjust
 ja LFogClampHighOrLow0	
LFogClampReentry0:	
 mov ds:dword ptr[s],esi			; s = sadjust
 mov ebx,ds:dword ptr[pbase]		; ebx = pbase
 shl esi,16							; sadjust << 16
 cmp edx,ebp						; clamp tadjust
 mov ds:dword ptr[sfracf],esi		; sfrac = sadjust << 16
 ja LFogClampHighOrLow1	
LFogClampReentry1:	
 mov ds:dword ptr[t],edx			; t = tadjust
 mov esi,ds:dword ptr[s]			; sfrac = scans->sfrac;
 shl edx,16							; tadjust << 16
 mov eax,ds:dword ptr[t]			; tfrac = scans->tfrac;
 sar esi,16							; sadjust >> 16
 mov ds:dword ptr[tfracf],edx		; tfrac = tadjust << 16

;
; calculate the texture starting address
;
 sar eax,16							; tfrac >> 16
 mov edx,ds:dword ptr[_cachewidth]	; edx = cachewidth
 imul eax,edx						; (tfrac >> 16) * cachewidth
 add esi,ebx						; pbase + (sadjust >> 16)
 add esi,eax						; psource = pbase + (sfrac >> 16) + ((tfrac >> 16) * cachewidth);

;
; determine whether last span or not
;
 cmp ecx,16							; count == 16?
 jna LLastSegment					; last segment of 16

;
; not the last segment; do full 16-wide segment
;
LNotLastSegment:	

;
; advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
; get there
;

; pick up after the FDIV that was left in flight previously

 fld st(0)							; duplicate it
 fmul st(0),st(4)					; s = s/z * z
 fxch st(1)							
 fmul st(0),st(3)					; t = t/z * z
 fxch st(1)	
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]	
 mov eax,ds:dword ptr[snext]	
 mov edx,ds:dword ptr[tnext]	

 mov bx,ds:word ptr[esi*2]			; get first source texel
 sub ecx,16							; count off this segments' pixels
 mov ebp,ds:dword ptr[_sadjust]
 mov ds:dword ptr[counttemp],ecx	; remember count of remaining pixels

 mov ecx,ds:dword ptr[_tadjust]	
 mov ds:word ptr[edi],bx			; store first dest pixel

 add ebp,eax						; sadjust += snext
 add ecx,edx						; tadjust += tnext

 mov eax,ds:dword ptr[_bbextents]	; set up for clamping
 mov edx,ds:dword ptr[_bbextentt]	; set up for clamping

 cmp ebp,4096						; clamp sadjust
 jl LFogClampLow2	
 cmp ebp,eax						; clamp sadjust
 ja LFogClampHigh2	
LFogClampReentry2:	

 cmp ecx,4096						; clamp tadjust
 jl LFogClampLow3	
 cmp ecx,edx						; clamp tadjust
 ja LFogClampHigh3	
LFogClampReentry3:	

 mov ds:dword ptr[snext],ebp		; snext = sadjust
 mov ds:dword ptr[tnext],ecx		; tnext = tadjust

 sub ebp,ds:dword ptr[s]			; sadjust -= s
 sub ecx,ds:dword ptr[t]			; tadjust -= t

;
; set up advancetable
;
 mov eax,ecx						; eax = t
 mov edx,ebp						; edx = s
 sar eax,20							; tstep >>= 16;
 jz LZero	
 sar edx,20							; sstep >>= 16;
 mov ebx,ds:dword ptr[_cachewidth]	; ebx = cachewidth
 imul eax,ebx						; eax = cachewidth * t
 jmp LSetUp1	

LZero:	
 sar edx,20							; sstep >>= 16;
 mov ebx,ds:dword ptr[_cachewidth]	; ebx = cachewidth

LSetUp1:	
 
 add eax,edx								; add in sstep (tstep >> 16) * cachewidth + (sstep >> 16);
 mov edx,ds:dword ptr[tfracf]				; edx = tracf
 mov ds:dword ptr[advancetable+4],eax		; advance base in t
 add eax,ebx								; ((tstep >> 16) + 1) * cachewidth + (sstep >> 16);
 shl ebp,12									; left-justify sstep fractional part
 mov ebx,ds:dword ptr[sfracf]	
 shl ecx,12									; left-justify tstep fractional part
 mov ds:dword ptr[advancetable],eax			; advance extra in t

 mov ds:dword ptr[tstep],ecx				
 add edx,ecx								; advance tfrac fractional part by tstep frac

 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; point to next source texel

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ax,ds:word ptr[esi*2]					; get the source texel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ds:word ptr[2+edi],ax					; write the destination pixel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[4+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[6+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac	
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[8+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[10+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[12+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[14+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t


;
; start FDIV for end of next segment in flight, so it can overlap
;
 mov ecx,ds:dword ptr[counttemp]	
 cmp ecx,16							; more than one segment after this?
 ja LSetupNotLast2					; yes

 dec ecx	
 jz LFDIVInFlight2					; if only one pixel, no need to start an FDIV
 mov ds:dword ptr[spancountminus1],ecx	
 fild ds:dword ptr[spancountminus1]	

 fld ds:dword ptr[_d_zistepu]		; C(d_zistepu) | spancountminus1
 fmul st(0),st(1)					; C(d_zistepu)*scm1 | scm1
 fld ds:dword ptr[_d_tdivzstepu]	; C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
 fmul st(0),st(2)					; C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
 fxch st(1)							; C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1 | scm1
 faddp st(3),st(0)					; C(d_tdivzstepu)*scm1 | scm1
 fxch st(1)							; scm1 | C(d_tdivzstepu)*scm1
 fmul ds:dword ptr[_d_sdivzstepu]	; C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
 fxch st(1)							; C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
 faddp st(3),st(0)					; C(d_sdivzstepu)*scm1
 fld ds:dword ptr[fp_64k]			; 64k | C(d_sdivzstepu)*scm1
 fxch st(1)							; C(d_sdivzstepu)*scm1 | 64k
 faddp st(4),st(0)					; 64k

 fdiv st(0),st(1)					; this is what we've gone to all this trouble to overlap
 jmp LFDIVInFlight2	

 align 4	
LSetupNotLast2:	
 fadd ds:dword ptr[zi16stepu]	
 fxch st(2)	
 fadd ds:dword ptr[sdivz16stepu]	
 fxch st(2)	
 fld ds:dword ptr[tdivz16stepu]	
 faddp st(2),st(0)	
 fld ds:dword ptr[fp_64k]	
 fdiv st(0),st(1)					; z = 1/1/z
									; this is what we've gone to all this trouble to overlap
LFDIVInFlight2:	
 mov ds:dword ptr[counttemp],ecx	

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[16+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[18+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[20+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[22+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[24+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[26+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 add edx,ds:dword ptr[tstep]				; advance tfrac fractional part by tstep frac
 sbb ecx,ecx								; turn tstep carry into -1 (0 if none)
 mov ds:word ptr[28+edi],ax					; write the destination pixel
 add ebx,ebp								; advance sfrac fractional part by sstep frac
 mov ax,ds:word ptr[esi*2]					; get the source texel
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	; advance base in t

 ;add edi,16									; pdest += 16 - should we double this for 16 bit?
 add edi,32									; pdest += 16 - should we double this for 16 bit?
 mov ds:dword ptr[tfracf],edx				; set tfracf (edx is adjusted tfracf)
 mov edx,ds:dword ptr[snext]				; edx = snext
 mov ds:dword ptr[sfracf],ebx				; set sfracf (ebx is adjusted sfracf)
 mov ebx,ds:dword ptr[tnext]				; ebx = tnext
 mov ds:dword ptr[s],edx					; s = snext
 mov ds:dword ptr[t],ebx					; t = tnext

 mov ecx,ds:dword ptr[counttemp]			; retrieve count

;
; determine whether last span or not
;
 cmp ecx,16							; are there multiple segments remaining?
 mov ds:word ptr[-2+edi],ax	
 ja LNotLastSegment					; yes

;
; last segment of scan
;
LLastSegment:	

;
; advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
; get there. The number of pixels left is variable, and we want to land on the
; last pixel, not step one past it, so we can't run into arithmetic problems
;
 test ecx,ecx	
 jz LNoSteps						; just draw the last pixel and we're done

; pick up after the FDIV that was left in flight previously


 fld st(0)							; duplicate it
 fmul st(0),st(4)					; s = s/z * z
 fxch st(1)	
 fmul st(0),st(3)					; t = t/z * z
 fxch st(1)	
 fistp ds:dword ptr[snext]	
 fistp ds:dword ptr[tnext]	

 mov ax,ds:word ptr[esi*2]			; load first texel in segment
 mov ebx,ds:dword ptr[_tadjust]	
 mov ds:word ptr[edi],ax			; store first pixel in segment
 mov eax,ds:dword ptr[_sadjust]	

 add eax,ds:dword ptr[snext]	
 add ebx,ds:dword ptr[tnext]	

 mov ebp,ds:dword ptr[_bbextents]	
 mov edx,ds:dword ptr[_bbextentt]	

 cmp eax,4096	
 jl LFogClampLow4	
 cmp eax,ebp	
 ja LFogClampHigh4	
LFogClampReentry4:	
 mov ds:dword ptr[snext],eax	

 cmp ebx,4096	
 jl LFogClampLow5	
 cmp ebx,edx	
 ja LFogClampHigh5	
LFogClampReentry5:	

 cmp ecx,1										; don't bother 
 je LOnlyOneStep								; if two pixels in segment, there's only one step,
;  of the segment length
 sub eax,ds:dword ptr[s]	
 sub ebx,ds:dword ptr[t]	

 add eax,eax									; convert to 15.17 format so multiply by 1.31
 add ebx,ebx									; reciprocal yields 16.48

 imul ds:dword ptr[reciprocal_table_16-8+ecx*4]	; sstep = (snext - s) / (spancount-1)
 mov ebp,edx	

 mov eax,ebx	
 imul ds:dword ptr[reciprocal_table_16-8+ecx*4]	; tstep = (tnext - t) / (spancount-1)
LSetEntryvec:	
;
; set up advancetable
;
 mov ebx,ds:dword ptr[entryvec_table_16+ecx*4]	
 mov eax,edx	
 mov ds:dword ptr[jumptemp],ebx		; entry point into code for RET later
 mov ecx,ebp	
 sar edx,16							; tstep >>= 16;
 mov ebx,ds:dword ptr[_cachewidth]	
 sar ecx,16							; sstep >>= 16;
 imul edx,ebx	

 add edx,ecx							; add in sstep
										; (tstep >> 16) * cachewidth + (sstep >> 16);
 mov ecx,ds:dword ptr[tfracf]	
 mov ds:dword ptr[advancetable+4],edx	; advance base in t
 add edx,ebx							; ((tstep >> 16) + 1) * cachewidth + (sstep >> 16);
 shl ebp,16								; left-justify sstep fractional part
 mov ebx,ds:dword ptr[sfracf]	
 shl eax,16								; left-justify tstep fractional part
 mov ds:dword ptr[advancetable],edx		; advance extra in t

 mov ds:dword ptr[tstep],eax	
 mov edx,ecx	
 add edx,eax	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	

 jmp dword ptr[jumptemp]				; jump to the number-of-pixels handler

;----------------------------------------

LNoSteps:	
 mov ax,ds:word ptr[esi*2]				; load first texel in segment
 sub edi,30								; adjust for hardwired offset
 jmp LEndSpan	


LOnlyOneStep:	
 sub eax,ds:dword ptr[s]	
 sub ebx,ds:dword ptr[t]	
 mov ebp,eax	
 mov edx,ebx	
 jmp LSetEntryvec	

;----------------------------------------

 public Entry2_16f, Entry3_16f, Entry4_16f, Entry5_16f	
 public Entry6_16f, Entry7_16f, Entry8_16f, Entry9_16f	
 public Entry10_16f, Entry11_16f, Entry12_16f, Entry13_16f
 public Entry14_16f, Entry15_16f, Entry16_16f	

Entry2_16f:	
 sub edi,28							; adjust for hardwired offsets
 mov ax,ds:word ptr[esi*2]	
 jmp LEntry2_16f	

;----------------------------------------

Entry3_16f:	
 sub edi,26							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 jmp LEntry3_16f	

;----------------------------------------

Entry4_16f:	
 sub edi,24							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry4_16f	

;----------------------------------------

Entry5_16f:	
 sub edi,22							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry5_16f	

;----------------------------------------

Entry6_16f:	
 sub edi,20							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry6_16f	

;----------------------------------------

Entry7_16f:	
 sub edi,18							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry7_16f	

;----------------------------------------

Entry8_16f:	
 sub edi,16							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry8_16f	

;----------------------------------------

Entry9_16f:	
 sub edi,14							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry9_16f	

;----------------------------------------

Entry10_16f:	
 sub edi,12							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry10_16f	

;----------------------------------------

Entry11_16f:	
 sub edi,10							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry11_16f	

;----------------------------------------

Entry12_16f:	
 sub edi,8							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry12_16f	

;----------------------------------------

Entry13_16f:	
 sub edi,6							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry13_16f	

;----------------------------------------

Entry14_16f:	
 sub edi,4							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry14_16f	

;----------------------------------------

Entry15_16f:	
 sub edi,2							; adjust for hardwired offsets
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
 jmp LEntry15_16f	

;----------------------------------------

Entry16_16f:	
 add edx,eax	
 mov ax,ds:word ptr[esi*2]	
 sbb ecx,ecx	
 add ebx,ebp	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	

 add edx,ds:dword ptr[tstep]	
 sbb ecx,ecx	
 mov ds:word ptr[2+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry15_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[4+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry14_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[6+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry13_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[8+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry12_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[10+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry11_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[12+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry10_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[14+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry9_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[16+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry8_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[18+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry7_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[20+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry6_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[22+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry5_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[24+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
 add edx,ds:dword ptr[tstep]	
LEntry4_16f:	
 sbb ecx,ecx	
 mov ds:word ptr[26+edi],ax	
 add ebx,ebp	
 mov ax,ds:word ptr[esi*2]	
 adc esi,ds:dword ptr[advancetable+4+ecx*4]	
LEntry3_16f:	
 mov ds:word ptr[28+edi],ax	
 mov ax,ds:word ptr[esi*2]	
LEntry2_16f:	

LEndSpan:	

;
; clear s/z, t/z, 1/z from FP stack
;
 fstp st(0)	
 fstp st(0)	
 fstp st(0)	

 mov ebx,ds:dword ptr[pspantemp]			; restore spans pointer
 mov ebx,ds:dword ptr[espan_t_pnext+ebx]	; point to next span
 test ebx,ebx								; any more spans?
 mov ds:word ptr[30+edi],ax	
 jnz LSpanLoop								; more spans

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	


;----------------------------------------------------------------------
; 8-bpp horizontal span z drawing codefor polygons, with no transparency.
;
; Assumes there is at least one span in pzspans, and that every span
; contains at least one pixel
;----------------------------------------------------------------------

	

; z-clamp on a non-negative gradient span
LFogClamp:	
 mov edx,040000000h	
 xor ebx,ebx	
 fstp st(0)	
 jmp LZDraw	

; z-clamp on a negative gradient span
LFogClampNeg:	
 mov edx,040000000h	
 xor ebx,ebx	
 fstp st(0)	
 jmp LZDrawNeg	


pzspans	equ		4+16

 public _D_DrawZSpans_f
_D_DrawZSpans_f:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

 fld ds:dword ptr[_d_zistepu]	
 mov eax,ds:dword ptr[_d_zistepu]	
 mov esi,ds:dword ptr[pzspans+esp]	
 test eax,eax	
 jz LFNegSpan	

 fmul ds:dword ptr[Float2ToThe31nd]	
 fistp ds:dword ptr[izistep]	; note: we are relying on FP exceptions being turned
; off here to avoid range problems
 mov ebx,ds:dword ptr[izistep]	; remains loaded for all spans

LFSpanLoop:	
; set up the initial 1/z value
 fild ds:dword ptr[espan_t_v+esi]	
 fild ds:dword ptr[espan_t_u+esi]	
 mov ecx,ds:dword ptr[espan_t_v+esi]	
 mov edi,ds:dword ptr[_d_pzbuffer]	
 fmul ds:dword ptr[_d_zistepu]	
 fxch st(1)	
 fmul ds:dword ptr[_d_zistepv]	
 fxch st(1)	
 fadd ds:dword ptr[_d_ziorigin]	
 imul ecx,ds:dword ptr[_d_zrowbytes]	
 faddp st(1),st(0)	

; clamp if z is nearer than 2 (1/z > 0.5)
 fcom ds:dword ptr[float_point5]	
 add edi,ecx	
 mov edx,ds:dword ptr[espan_t_u+esi]	
 add edx,edx	; word count
 mov ecx,ds:dword ptr[espan_t_count+esi]	
 add edi,edx	; pdest = &pdestspan[scans->u];
 push esi	; preserve spans pointer
 fnstsw ax	
 test ah,045h	
 jz LFogClamp	

 fmul ds:dword ptr[Float2ToThe31nd]	
 fistp ds:dword ptr[izi]	; note: we are relying on FP exceptions being turned
; off here to avoid problems when the span is closer
; than 1/(2**31)
 mov edx,ds:dword ptr[izi]	

; at this point:
; %ebx = izistep
; %ecx = count
; %edx = izi
; %edi = pdest

LZDraw:	

; do a single pixel up front, if necessary to dword align the destination
 test edi,2	
 jz LFMiddle	
 mov eax,edx	
 add edx,ebx	
 shr eax,16	
 dec ecx	
 mov ds:word ptr[edi],ax	
 add edi,2	

; do middle a pair of aligned dwords at a time
LFMiddle:	
 push ecx	
 shr ecx,1	; count / 2
 jz LFLast	; no aligned dwords to do
 shr ecx,1	; (count / 2) / 2
 jnc LFMiddleLoop	; even number of aligned dwords to do

 mov eax,edx	
 add edx,ebx	
 shr eax,16	
 mov esi,edx	
 add edx,ebx	
 and esi,0FFFF0000h	
 or eax,esi	
 mov ds:dword ptr[edi],eax	
 add edi,4	
 and ecx,ecx	
 jz LFLast	

LFMiddleLoop:	
 mov eax,edx	
 add edx,ebx	
 shr eax,16	
 mov esi,edx	
 add edx,ebx	
 and esi,0FFFF0000h	
 or eax,esi	
 mov ebp,edx	
 mov ds:dword ptr[edi],eax	
 add edx,ebx	
 shr ebp,16	
 mov esi,edx	
 add edx,ebx	
 and esi,0FFFF0000h	
 or ebp,esi	
 mov ds:dword ptr[4+edi],ebp	; FIXME: eliminate register contention
 add edi,8	

 dec ecx	
 jnz LFMiddleLoop	

LFLast:	
 pop ecx	; retrieve count
 pop esi	; retrieve span pointer

; do the last, unaligned pixel, if there is one
 and ecx,1	; is there an odd pixel left to do?
 jz LFSpanDone	; no
 shr edx,16	
 mov ds:word ptr[edi],dx	; do the final pixel's z

LFSpanDone:	
 mov esi,ds:dword ptr[espan_t_pnext+esi]	
 test esi,esi	
 jnz LFSpanLoop	

 jmp LFDone	

LFNegSpan:	
 fmul ds:dword ptr[FloatMinus2ToThe31nd]	
 fistp ds:dword ptr[izistep]	; note: we are relying on FP exceptions being turned
; off here to avoid range problems
 mov ebx,ds:dword ptr[izistep]	; remains loaded for all spans

LFNegSpanLoop:	
; set up the initial 1/z value
 fild ds:dword ptr[espan_t_v+esi]	
 fild ds:dword ptr[espan_t_u+esi]	
 mov ecx,ds:dword ptr[espan_t_v+esi]	
 mov edi,ds:dword ptr[_d_pzbuffer]	
 fmul ds:dword ptr[_d_zistepu]	
 fxch st(1)	
 fmul ds:dword ptr[_d_zistepv]	
 fxch st(1)	
 fadd ds:dword ptr[_d_ziorigin]	
 imul ecx,ds:dword ptr[_d_zrowbytes]	
 faddp st(1),st(0)	

; clamp if z is nearer than 2 (1/z > 0.5)
 fcom ds:dword ptr[float_point5]	
 add edi,ecx	
 mov edx,ds:dword ptr[espan_t_u+esi]	
 add edx,edx	; word count
 mov ecx,ds:dword ptr[espan_t_count+esi]	
 add edi,edx	; pdest = &pdestspan[scans->u];
 push esi	; preserve spans pointer
 fnstsw ax	
 test ah,045h	
 jz LFogClampNeg	

 fmul ds:dword ptr[Float2ToThe31nd]	
 fistp ds:dword ptr[izi]	; note: we are relying on FP exceptions being turned
; off here to avoid problems when the span is closer
; than 1/(2**31)
 mov edx,ds:dword ptr[izi]	

; at this point:
; %ebx = izistep
; %ecx = count
; %edx = izi
; %edi = pdest

LZDrawNeg:	

; do a single pixel up front, if necessary to dword align the destination
 test edi,2	
 jz LFNegMiddle	
 mov eax,edx	
 sub edx,ebx	
 shr eax,16	
 dec ecx	
 mov ds:word ptr[edi],ax	
 add edi,2	

; do middle a pair of aligned dwords at a time
LFNegMiddle:	
 push ecx	
 shr ecx,1	; count / 2
 jz LFNegLast	; no aligned dwords to do
 shr ecx,1	; (count / 2) / 2
 jnc LFNegMiddleLoop	; even number of aligned dwords to do

 mov eax,edx	
 sub edx,ebx	
 shr eax,16	
 mov esi,edx	
 sub edx,ebx	
 and esi,0FFFF0000h	
 or eax,esi	
 mov ds:dword ptr[edi],eax	
 add edi,4	
 and ecx,ecx	
 jz LFNegLast	

LFNegMiddleLoop:	
 mov eax,edx	
 sub edx,ebx	
 shr eax,16	
 mov esi,edx	
 sub edx,ebx	
 and esi,0FFFF0000h	
 or eax,esi	
 mov ebp,edx	
 mov ds:dword ptr[edi],eax	
 sub edx,ebx	
 shr ebp,16	
 mov esi,edx	
 sub edx,ebx	
 and esi,0FFFF0000h	
 or ebp,esi	
 mov ds:dword ptr[4+edi],ebp	; FIXME: eliminate register contention
 add edi,8	

 dec ecx	
 jnz LFNegMiddleLoop	

LFNegLast:	
 pop ecx	; retrieve count
 pop esi	; retrieve span pointer

; do the last, unaligned pixel, if there is one
 and ecx,1	; is there an odd pixel left to do?
 jz LFNegSpanDone	; no
 shr edx,16	
 mov ds:word ptr[edi],dx	; do the final pixel's z

LFNegSpanDone:	
 mov esi,ds:dword ptr[espan_t_pnext+esi]	
 test esi,esi	
 jnz LFNegSpanLoop	

LFDone:	
 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	

 public _D_Spans16Patch_f
_D_Spans16Patch_f:
 mov eax,[_alias_fogmap]
 ;mov ds:dword ptr[FogPatch-4],eax
 ;mov ds:dword ptr[FogPatch1-4],eax
 ;mov ds:dword ptr[FogPatch2-4],eax
 ;mov ds:dword ptr[FogPatch3-4],eax
 ;mov ds:dword ptr[FogPatch4-4],eax
 ;mov ds:dword ptr[FogPatch5-4],eax
 ;mov ds:dword ptr[FogPatch6-4],eax
 ;mov ds:dword ptr[FogPatch7-4],eax
 ;mov ds:dword ptr[FogPatch8-4],eax
 ;mov ds:dword ptr[FogPatch9-4],eax
 ;mov ds:dword ptr[FogPatch10-4],eax
 ;mov ds:dword ptr[FogPatch11-4],eax
 ;mov ds:dword ptr[FogPatch12-4],eax
 ;mov ds:dword ptr[FogPatch13-4],eax
 ;mov ds:dword ptr[FogPatch14-4],eax
 ;mov ds:dword ptr[FogPatch15-4],eax
 ;mov ds:dword ptr[FogPatch16-4],eax
 ;mov ds:dword ptr[FogPatch17-4],eax
 ;mov ds:dword ptr[FogPatch18-4],eax
 ;mov ds:dword ptr[FogPatch19-4],eax
 ;mov ds:dword ptr[FogPatch20-4],eax
 ;mov ds:dword ptr[FogPatch21-4],eax
 ;mov ds:dword ptr[FogPatch22-4],eax
 ;mov ds:dword ptr[FogPatch23-4],eax
 ;mov ds:dword ptr[FogPatch24-4],eax
 ;mov ds:dword ptr[FogPatch25-4],eax
 ;mov ds:dword ptr[FogPatch26-4],eax
 ;mov ds:dword ptr[FogPatch27-4],eax
 ;mov ds:dword ptr[FogPatch28-4],eax
 ;mov ds:dword ptr[FogPatch29-4],eax
 ;mov ds:dword ptr[FogPatch30-4],eax
 ;mov ds:dword ptr[FogPatch31-4],eax

 ;mov al,ds:byte ptr[_alias_fogdiv]
 ;mov ds:byte ptr[FogLevel0-1],al
 ;mov ds:byte ptr[FogLevel1-1],al
 ;mov ds:byte ptr[FogLevel2-1],al
 ;mov ds:byte ptr[FogLevel3-1],al
 ret

_TEXT ENDS
endif	;id386
 END
