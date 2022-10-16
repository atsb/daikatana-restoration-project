 .386P
 .model FLAT
;
; surf8.s
; x86 assembly-language 8 bpp surface block drawing code.
;

include qasm.inc

if	id386

_DATA SEGMENT	

sb_v dd 0	

_DATA ENDS
_TEXT SEGMENT	

 align 4	
 public _R_Surf8Start	
_R_Surf8Start:	

;----------------------------------------------------------------------
; Surface block drawer for mip level 0
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock8_mip0	
_R_DrawSurfaceBlock8_mip0:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

;		for (v=0 ; v<numvblocks ; v++)
;		{
 mov ebx,ds:dword ptr[_r_lightptr]	
 mov eax,ds:dword ptr[_r_numvblocks]	

 mov ds:dword ptr[sb_v],eax	
 mov edi,ds:dword ptr[_prowdestbase]	

 mov esi,ds:dword ptr[_pbasesource]	

Lv_loop_mip0:	

;			lightleft = lightptr[0];
;			lightright = lightptr[1];
;			lightdelta = (lightleft - lightright) & 0xFFFFF;
 mov eax,ds:dword ptr[ebx]	; lightleft
 mov edx,ds:dword ptr[4+ebx]	; lightright

 mov ebp,eax	
 mov ecx,ds:dword ptr[_r_lightwidth]	

 mov ds:dword ptr[_lightright],edx	
 sub ebp,edx	

 and ebp,0FFFFFh	
 lea ebx,ds:dword ptr[ebx+ecx*4]	

;			lightptr += lightwidth;
 mov ds:dword ptr[_r_lightptr],ebx	

;			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
;			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
;			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
;					0xF0000000;
 mov ecx,ds:dword ptr[4+ebx]	; lightptr[1]
 mov ebx,ds:dword ptr[ebx]	; lightptr[0]

 sub ebx,eax	
 sub ecx,edx	

 sar ecx,4	
 or ebp,0F0000000h	

 sar ebx,4	
 mov ds:dword ptr[_lightrightstep],ecx	

 sub ebx,ecx	
 and ebx,0FFFFFh	

 or ebx,0F0000000h	
 sub ecx,ecx	; high word must be 0 in loop for addressing

 mov ds:dword ptr[_lightdeltastep],ebx	
 sub ebx,ebx	; high word must be 0 in loop for addressing

Lblockloop8_mip0:	
 mov ds:dword ptr[_lightdelta],ebp	
 mov cl,ds:byte ptr[14+esi]				;cl = source pixel

 sar ebp,4								;divide by 16
 ;mov bh,dh								;bh = light value

 mov bl,ds:byte ptr[15+esi]				;bl = source pixel
 add edx,ebp							;increment to next light value

 ;mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch0:	
 mov bl,ds:byte ptr[13+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch1:	
 mov cl,ds:byte ptr[12+esi]				;cl = source pixel

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16								;rotate low word into high word
 ;mov ch,dh								;ch = light value

 add edx,ebp							;increment light value
 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch2:	
	
 mov bl,ds:byte ptr[11+esi]				;bl = source pixel
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap
LBPatch3:	

 mov cl,ds:byte ptr[10+esi]				;cl = source pixel
 mov ds:dword ptr[12+edi],eax			;write the word (4 pixels) in eax to the screen

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ;mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch4:	
 mov bl,ds:byte ptr[9+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch5:	
 mov cl,ds:byte ptr[8+esi]				;cl = source pixel

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16								;rotate low word into hight word
 ;mov ch,dh								;ch = light value

 add edx,ebp							;increment to next light value
 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch6:	

 mov bl,ds:byte ptr[7+esi]				;bl = source pixel
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch7:	

 mov cl,ds:byte ptr[6+esi]				;cl = source pixel
 mov ds:dword ptr[8+edi],eax			;write the word (4 pixels) in eax to the screen

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ;mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch8:	
 mov bl,ds:byte ptr[5+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch9:	
 mov cl,ds:byte ptr[4+esi]				;cl = source pixel

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16								;rotate low word into high word
 ;mov ch,dh								;ch = light value

 add edx,ebp							;increment to next light value
 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch10:	

 mov bl,ds:byte ptr[3+esi]				;bl = source pixel
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch11:	

 mov cl,ds:byte ptr[2+esi]				;cl = source pixel
 mov ds:dword ptr[4+edi],eax			;write the word (4 pixels) in eax to the screen

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ;mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch12:	
 mov bl,ds:byte ptr[1+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch13:	
 mov cl,ds:byte ptr[esi]				;cl = source pixel

 ;mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16								;rotate low word into high word
 ;mov ch,dh								;ch = light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch14:	
 mov edx,ds:dword ptr[_lightright]		;

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch15:	
 mov ebp,ds:dword ptr[_lightdelta]		;

 mov ds:dword ptr[edi],eax				;write the word (4 pixels) in eax to the screen

 add esi,ds:dword ptr[_sourcetstep]		;increment the source pixel
 add edi,ds:dword ptr[_surfrowbytes]	;increment the destination buffer
			
 add edx,ds:dword ptr[_lightrightstep]	;lightright += lightrightstep
 add ebp,ds:dword ptr[_lightdeltastep]	;lightleft += lightleftstep

 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip0	

;			if (pbasesource >= r_sourcemax)
;				pbasesource -= stepback;

 cmp esi,ds:dword ptr[_r_sourcemax]	
 jb LSkip_mip0	
 sub esi,ds:dword ptr[_r_stepback]	
LSkip_mip0:	

 mov ebx,ds:dword ptr[_r_lightptr]	
 dec ds:dword ptr[sb_v]	

 jnz Lv_loop_mip0	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	


;----------------------------------------------------------------------
; Surface block drawer for mip level 1
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock8_mip1	
_R_DrawSurfaceBlock8_mip1:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

;		for (v=0 ; v<numvblocks ; v++)
;		{
 mov ebx,ds:dword ptr[_r_lightptr]	
 mov eax,ds:dword ptr[_r_numvblocks]	

 mov ds:dword ptr[sb_v],eax	
 mov edi,ds:dword ptr[_prowdestbase]	

 mov esi,ds:dword ptr[_pbasesource]	

Lv_loop_mip1:	

;			lightleft = lightptr[0];
;			lightright = lightptr[1];
;			lightdelta = (lightleft - lightright) & 0xFFFFF;
 mov eax,ds:dword ptr[ebx]	; lightleft
 mov edx,ds:dword ptr[4+ebx]	; lightright

 mov ebp,eax	
 mov ecx,ds:dword ptr[_r_lightwidth]	

 mov ds:dword ptr[_lightright],edx	
 sub ebp,edx	

 and ebp,0FFFFFh	
 lea ebx,ds:dword ptr[ebx+ecx*4]	

;			lightptr += lightwidth;
 mov ds:dword ptr[_r_lightptr],ebx	

;			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
;			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
;			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
;					0xF0000000;
 mov ecx,ds:dword ptr[4+ebx]	; lightptr[1]
 mov ebx,ds:dword ptr[ebx]	; lightptr[0]

 sub ebx,eax	
 sub ecx,edx	

 sar ecx,3	
 or ebp,070000000h	

 sar ebx,3	
 mov ds:dword ptr[_lightrightstep],ecx	

 sub ebx,ecx	
 and ebx,0FFFFFh	

 or ebx,0F0000000h	
 sub ecx,ecx	; high word must be 0 in loop for addressing

 mov ds:dword ptr[_lightdeltastep],ebx	
 sub ebx,ebx	; high word must be 0 in loop for addressing

Lblockloop8_mip1:	
 mov ds:dword ptr[_lightdelta],ebp	
 mov cl,ds:byte ptr[6+esi]				;cl = source pixel

 sar ebp,3	
 mov bh,dh								;bh = light value

 mov bl,ds:byte ptr[7+esi]				;bl = source pixel
 add edx,ebp							;increment to next light value

 mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch22:	
 mov bl,ds:byte ptr[5+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch23:	
 mov cl,ds:byte ptr[4+esi]				;cl = source pixel

 mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16	
 mov ch,dh								;ch = light value

 add edx,ebp							;increment to next light value
 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch24:	

 mov bl,ds:byte ptr[3+esi]				;bl = source pixel
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch25:	

 mov cl,ds:byte ptr[2+esi]				;cl = source pixel
 mov ds:dword ptr[4+edi],eax			;write the word (4 pixels) in eax to the screen

 mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch26:	
 mov bl,ds:byte ptr[1+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch27:	
 mov cl,ds:byte ptr[esi]				;cl = source pixel

 mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16	
 mov ch,dh								;ch = light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch28:	
 mov edx,ds:dword ptr[_lightright]	

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch29:	
 mov ebp,ds:dword ptr[_lightdelta]	

 mov ds:dword ptr[edi],eax				;write the word (4 pixels) in eax to the screen
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 add edx,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 add ebp,eax	
 mov ds:dword ptr[_lightright],edx	

 jc Lblockloop8_mip1	

;			if (pbasesource >= r_sourcemax)
;				pbasesource -= stepback;

 cmp esi,ds:dword ptr[_r_sourcemax]	
 jb LSkip_mip1	
 sub esi,ds:dword ptr[_r_stepback]	
LSkip_mip1:	

 mov ebx,ds:dword ptr[_r_lightptr]	
 dec ds:dword ptr[sb_v]	

 jnz Lv_loop_mip1	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	


;----------------------------------------------------------------------
; Surface block drawer for mip level 2
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock8_mip2	
_R_DrawSurfaceBlock8_mip2:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

;		for (v=0 ; v<numvblocks ; v++)
;		{
 mov ebx,ds:dword ptr[_r_lightptr]	
 mov eax,ds:dword ptr[_r_numvblocks]	

 mov ds:dword ptr[sb_v],eax	
 mov edi,ds:dword ptr[_prowdestbase]	

 mov esi,ds:dword ptr[_pbasesource]	

Lv_loop_mip2:	

;			lightleft = lightptr[0];
;			lightright = lightptr[1];
;			lightdelta = (lightleft - lightright) & 0xFFFFF;
 mov eax,ds:dword ptr[ebx]	; lightleft
 mov edx,ds:dword ptr[4+ebx]	; lightright

 mov ebp,eax	
 mov ecx,ds:dword ptr[_r_lightwidth]	

 mov ds:dword ptr[_lightright],edx	
 sub ebp,edx	

 and ebp,0FFFFFh	
 lea ebx,ds:dword ptr[ebx+ecx*4]	

;			lightptr += lightwidth;
 mov ds:dword ptr[_r_lightptr],ebx	

;			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
;			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
;			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
;					0xF0000000;
 mov ecx,ds:dword ptr[4+ebx]	; lightptr[1]
 mov ebx,ds:dword ptr[ebx]	; lightptr[0]

 sub ebx,eax	
 sub ecx,edx	

 sar ecx,2	
 or ebp,030000000h	

 sar ebx,2	
 mov ds:dword ptr[_lightrightstep],ecx	

 sub ebx,ecx	

 and ebx,0FFFFFh	

 or ebx,0F0000000h	
 sub ecx,ecx	; high word must be 0 in loop for addressing

 mov ds:dword ptr[_lightdeltastep],ebx	
 sub ebx,ebx	; high word must be 0 in loop for addressing

Lblockloop8_mip2:	
 mov ds:dword ptr[_lightdelta],ebp	
 mov cl,ds:byte ptr[2+esi]				;cl = source pixel

 sar ebp,2	
 mov bh,dh								;bh = light value

 mov bl,ds:byte ptr[3+esi]				;bl = source pixel
 add edx,ebp							;increment to next light value

 mov ch,dh								;ch = light value
 add edx,ebp							;increment to next light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch18:	
 mov bl,ds:byte ptr[1+esi]				;bl = source pixel

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch19:	
 mov cl,ds:byte ptr[esi]				;cl = source pixel

 mov bh,dh								;bh = light value
 add edx,ebp							;increment to next light value

 ror eax,16	
 mov ch,dh								;ch = light value

 ;mov ah,ds:byte ptr[12345678h+ebx]		;ah = pixel in ebx from colormap
LBPatch20:	
 mov edx,ds:dword ptr[_lightright]	

 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch21:	
 mov ebp,ds:dword ptr[_lightdelta]	

 mov ds:dword ptr[edi],eax				;write the word (4 pixels) in eax to the screen
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 add edx,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 add ebp,eax	
 mov ds:dword ptr[_lightright],edx	

 jc Lblockloop8_mip2	

;			if (pbasesource >= r_sourcemax)
;				pbasesource -= stepback;

 cmp esi,ds:dword ptr[_r_sourcemax]	
 jb LSkip_mip2	
 sub esi,ds:dword ptr[_r_stepback]	
LSkip_mip2:	

 mov ebx,ds:dword ptr[_r_lightptr]	
 dec ds:dword ptr[sb_v]	

 jnz Lv_loop_mip2	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	


;----------------------------------------------------------------------
; Surface block drawer for mip level 3
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock8_mip3	
_R_DrawSurfaceBlock8_mip3:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

;		for (v=0 ; v<numvblocks ; v++)
;		{
 mov ebx,ds:dword ptr[_r_lightptr]				;r_lightptr is a pointer to the array of light values
 mov eax,ds:dword ptr[_r_numvblocks]			;r_numvblocks is the number of vertical lightmap blocks

 mov ds:dword ptr[sb_v],eax						;sb_v 
 mov edi,ds:dword ptr[_prowdestbase]			;prowdest base is a pointer to the screen

 mov esi,ds:dword ptr[_pbasesource]				;pbasesource is a pointer to the image data

Lv_loop_mip3:	

;			lightleft = lightptr[0];
;			lightright = lightptr[1];
;			lightdelta = (lightleft - lightright) & 0xFFFFF;
 mov eax,ds:dword ptr[ebx]						;lightleft now in eax
 mov edx,ds:dword ptr[4+ebx]					;lightright now in edx

 mov ebp,eax									;lightleft now inn epb
 mov ecx,ds:dword ptr[_r_lightwidth]			;r_lightwidth now in ecx.  r_lightwidth = 1/16th the width of the surface extents

 mov ds:dword ptr[_lightright],edx				;lightright
 sub ebp,edx									;lightleft - lightright (lightdelta is the result)

 and ebp,0FFFFFh								
 lea ebx,ds:dword ptr[ebx+ecx*4]				

 mov ds:dword ptr[_lightdelta],ebp				

 mov ds:dword ptr[_r_lightptr],ebx				;			lightptr += lightwidth

;			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
;			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
;			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
;					0xF0000000;
 mov ecx,ds:dword ptr[4+ebx]	; lightptr[1]
 mov ebx,ds:dword ptr[ebx]	; lightptr[0]

 sub ebx,eax	
 sub ecx,edx	

 sar ecx,1	

 sar ebx,1	
 mov ds:dword ptr[_lightrightstep],ecx	

 sub ebx,ecx	
 and ebx,0FFFFFh	

 sar ebp,1	
 or ebx,0F0000000h	

 mov ds:dword ptr[_lightdeltastep],ebx	
 sub ebx,ebx	; high word must be 0 in loop for addressing

 mov bl,ds:byte ptr[1+esi]				;bl = source pixel
 sub ecx,ecx	; high word must be 0 in loop for addressing

 mov bh,dh								;bh = light value
 mov cl,ds:byte ptr[esi]				;cl = source pixel

 add edx,ebp							;increment to next light value
 mov ch,dh								;ch = light value

 ;mov al,ds:byte ptr[12345678h+ebx]		;al = pixel in ecx from colormap 
LBPatch16:	
 mov edx,ds:dword ptr[_lightright]	

 mov ds:byte ptr[1+edi],al	
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch17:	

 mov ds:byte ptr[edi],al				;write the word (4 pixels) in eax to the screen
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 mov ebp,ds:dword ptr[_lightdelta]	
 mov cl,ds:byte ptr[esi]				;cl = source pixel

 add ebp,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 sar ebp,1	
 add edx,eax	

 mov bh,dh								;bh = light value
 mov bl,ds:byte ptr[1+esi]				;bl = source pixel

 add edx,ebp							;increment to next light value
 mov ch,dh								;ch = light value

 ;mov al,ds:byte ptr[12345678h+ebx]		;al = pixel in ecx from colormap 
LBPatch30:	
 mov edx,ds:dword ptr[_sourcetstep]	

 mov ds:byte ptr[1+edi],al	
 ;mov al,ds:byte ptr[12345678h+ecx]		;al = pixel in ecx from colormap 
LBPatch31:	

 mov ds:byte ptr[edi],al				;write the word (4 pixels) in eax to the screen
 mov ebp,ds:dword ptr[_surfrowbytes]	

 add esi,edx	
 add edi,ebp	

;			if (pbasesource >= r_sourcemax)
;				pbasesource -= stepback;

 cmp esi,ds:dword ptr[_r_sourcemax]	
 jb LSkip_mip3	
 sub esi,ds:dword ptr[_r_stepback]	
LSkip_mip3:	

 mov ebx,ds:dword ptr[_r_lightptr]	
 dec ds:dword ptr[sb_v]	

 jnz Lv_loop_mip3	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	


 public _R_Surf8End	
_R_Surf8End:	

;----------------------------------------------------------------------
; Code patching routines
;----------------------------------------------------------------------
_TEXT ENDS
_DATA SEGMENT	

 align 4	
LPatchTable8:	
 ;dd LBPatch0-4	
 ;dd LBPatch1-4	
 ;dd LBPatch2-4	
 ;dd LBPatch3-4	
 ;dd LBPatch4-4	
 ;dd LBPatch5-4	
 ;dd LBPatch6-4	
 ;dd LBPatch7-4	
 ;dd LBPatch8-4	
 ;dd LBPatch9-4	
 ;dd LBPatch10-4	
 ;dd LBPatch11-4	
 ;dd LBPatch12-4	
 ;dd LBPatch13-4	
 ;dd LBPatch14-4	
 ;dd LBPatch15-4	
 ;dd LBPatch16-4	
 ;dd LBPatch17-4	
 ;dd LBPatch18-4	
 ;dd LBPatch19-4	
 ;dd LBPatch20-4	
 ;dd LBPatch21-4	
 ;dd LBPatch22-4	
 ;dd LBPatch23-4	
 ;dd LBPatch24-4	
 ;dd LBPatch25-4	
 ;dd LBPatch26-4	
 ;dd LBPatch27-4	
 ;dd LBPatch28-4	
 ;dd LBPatch29-4	 
 ;dd LBPatch30-4	
 ;dd LBPatch31-4	

_DATA ENDS
_TEXT SEGMENT	

 align 4	
 public _R_Surf8Patch	
_R_Surf8Patch:	
 ;push ebx	

 ;mov eax,ds:dword ptr[_colormap]	
 ;mov ebx,offset LPatchTable8
 ;mov ecx,32	
LPatchLoop8:	
 ;mov edx,ds:dword ptr[ebx]	
 ;add ebx,4	
 ;mov ds:dword ptr[edx],eax
 ;dec ecx	
 ;jnz LPatchLoop8	

 ;pop ebx	

 ret	

_TEXT ENDS
endif	;id386

 END

