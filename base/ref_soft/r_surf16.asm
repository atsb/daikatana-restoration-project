 .386P
 .model FLAT
;
; surf8.s
; x86 assembly-language 16 bpp surface block drawing code.
;

include qasm.inc

if	id386

_DATA SEGMENT	

sb_v dd 0	

_DATA ENDS
_TEXT SEGMENT	

 align 4	
 public _R_Surf16Start	
_R_Surf16Start:	

;----------------------------------------------------------------------
; Surface block drawer for mip level 0
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock16_mip0	
_R_DrawSurfaceBlock16_mip0:	
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

Lblockloop16_mip0:	
 mov ds:dword ptr[_lightdelta],ebp	

 sar ebp,4	

 xor eax,eax
 xor ebx,ebx
 xor ecx,ecx
 xor edx,edx

 mov al,ds:byte ptr[15+esi]					; al = source texel
 mov bl,ds:byte ptr[14+esi]					; bl = source texel
 mov ax,ds:word ptr[12345678h+eax*2]		; ax = 16 bit texel from lookup table
LBPatch0:	
 mov bx,ds:word ptr[12345678h+ebx*2]		; bx = 16 bit texel from lookup table
LBPatch1:	
 mov cl,ds:byte ptr[13+esi]					; cl = source texel
 mov dl,ds:byte ptr[12+esi]					; dl = source texel
 mov cx,ds:word ptr[12345678h+ecx*2]		; cx = 16 bit texel from lookup table
LBPatch2:	
 mov ds:word ptr[30+edi],ax					; write the pixel
 mov dx,ds:word ptr[12345678h+edx*2]		; dx = 16 bit texel from lookup table
LBPatch3:	
 xor eax,eax
 mov ds:word ptr[28+edi],bx					; write the pixel
 mov al,ds:byte ptr[11+esi]					; al = source texel
 xor ebx,ebx
 mov ds:word ptr[26+edi],cx					; write the pixel
 mov bl,ds:byte ptr[10+esi]					; bl = source texel
 xor ecx,ecx
 mov ds:word ptr[24+edi],dx					; write the pixel
 mov cl,ds:byte ptr[9+esi]					; cl = source texel
 xor edx,edx
 mov ax,ds:word ptr[12345678h+eax*2]		; ax = 16 bit texel from lookup table
LBPatch4:	
 mov dl,ds:byte ptr[8+esi]					; dl = source texel
 mov bx,ds:word ptr[12345678h+ebx*2]		; bx = 16 bit texel from lookup table
LBPatch5:	
 mov ds:word ptr[22+edi],ax					; write the pixel
 mov cx,ds:word ptr[12345678h+ecx*2]		; cx = 16 bit texel from lookup table
LBPatch6:	
 xor eax,eax
 mov ds:word ptr[20+edi],bx					; write the pixel
 mov dx,ds:word ptr[12345678h+edx*2]		; dx = 16 bit texel from lookup table
LBPatch7:	
 xor ebx,ebx
 mov al,ds:byte ptr[7+esi]					; al = source texel
 mov ds:word ptr[18+edi],cx					; write the pixel
 mov bl,ds:byte ptr[6+esi]					; bl = source texel
 xor ecx,ecx
 mov ds:word ptr[16+edi],dx					; write the pixel
 mov ax,ds:word ptr[12345678h+eax*2]		; ax = 16 bit texel from lookup table
LBPatch8:	
 xor edx,edx
 mov cl,ds:byte ptr[5+esi]					; cl = source texel
 mov bx,ds:word ptr[12345678h+ebx*2]		; bx = 16 bit texel from lookup table
LBPatch9:	
 mov dl,ds:byte ptr[4+esi]					; dl = source texel
 mov cx,ds:word ptr[12345678h+ecx*2]		; cx = 16 bit texel from lookup table
LBPatch10:	
 mov ds:word ptr[14+edi],ax					; write the pixel
 mov dx,ds:word ptr[12345678h+edx*2]		; dx = 16 bit texel from lookup table
LBPatch11:	
 xor eax,eax
 mov ds:word ptr[12+edi],bx					; write the pixel
 mov al,ds:byte ptr[3+esi]					; al = source texel
 xor ebx,ebx
 mov ds:word ptr[10+edi],cx					; write the pixel
 mov bl,ds:byte ptr[2+esi]					; bl = source texel
 xor ecx,ecx
 mov ds:word ptr[8+edi],dx					; write the pixel
 mov cl,ds:byte ptr[1+esi]					; cl = source texel
 xor edx,edx
 mov ax,ds:word ptr[12345678h+eax*2]		; ax = 16 bit texel from lookup table
LBPatch12:	
 mov dl,ds:byte ptr[0+esi]					; dl = source texel
 mov bx,ds:word ptr[12345678h+ebx*2]		; bx = 16 bit texel from lookup table
LBPatch13:	
 mov ds:word ptr[6+edi],ax					; write the pixel
 mov cx,ds:word ptr[12345678h+ecx*2]		; cx = 16 bit texel from lookup table
LBPatch14:	
 mov ds:word ptr[4+edi],bx					; write the pixel
 mov dx,ds:word ptr[12345678h+edx*2]		; dx = 16 bit texel from lookup table
LBPatch15:	
 mov ds:word ptr[2+edi],cx					; write the pixel
 mov eax,ds:dword ptr[_lightright]		
 mov ds:word ptr[0+edi],dx					; write the pixel

 mov ebp,ds:dword ptr[_lightdelta]	

 add edi,ds:dword ptr[_surfrowbytes]	
 add esi,ds:dword ptr[_sourcetstep]	
 add edi,ds:dword ptr[_surfrowbytes]	

 add eax,ds:dword ptr[_lightrightstep]	
 add ebp,ds:dword ptr[_lightdeltastep]	

 mov ds:dword ptr[_lightright],eax	
 jc Lblockloop16_mip0	

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
 public _R_DrawSurfaceBlock16_mip1	
_R_DrawSurfaceBlock16_mip1:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	
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

Lblockloop16_mip1:	
 mov ds:dword ptr[_lightdelta],ebp	
 mov cx,ds:word ptr[6+esi]	

 sar ebp,3	
 mov bh,dh	

 mov bx,ds:word ptr[7+esi]	
 add edx,ebp	

 mov ch,dh	
 add edx,ebp	

 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch22:	
 mov bl,ds:byte ptr[5+esi]	

 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch23:	
 mov cl,ds:byte ptr[4+esi]	

 mov bh,dh	
 add edx,ebp	

 ror eax,16	
 mov ch,dh	

 add edx,ebp	
 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch24:	

 mov bl,ds:byte ptr[3+esi]	
 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch25:	

 mov cl,ds:byte ptr[2+esi]	
 ;mov ds:dword ptr[4+edi],eax	

 mov bh,dh	
 add edx,ebp	

 mov ch,dh	
 add edx,ebp	

 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch26:	
 mov bl,ds:byte ptr[1+esi]	

 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch27:	
 mov cl,ds:byte ptr[esi]	

 mov bh,dh	
 add edx,ebp	

 ror eax,16	
 mov ch,dh	

 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch28:	
 mov edx,ds:dword ptr[_lightright]	

 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch29:	
 mov ebp,ds:dword ptr[_lightdelta]	

 ;mov ds:dword ptr[edi],eax	
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 add edx,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 add ebp,eax	
 mov ds:dword ptr[_lightright],edx	

 jc Lblockloop16_mip1	

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


 public _R_Surf16End	
_R_Surf16End:	

;----------------------------------------------------------------------
; Surface block drawer for mip level 2
;----------------------------------------------------------------------

 align 4	
 public _R_DrawSurfaceBlock16_mip2	
_R_DrawSurfaceBlock16_mip2:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	
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

Lblockloop16_mip2:	
 mov ds:dword ptr[_lightdelta],ebp	
 mov cl,ds:byte ptr[2+esi]	

 sar ebp,2	
 mov bh,dh	

 mov bl,ds:byte ptr[3+esi]	
 add edx,ebp	

 mov ch,dh	
 add edx,ebp	

 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch18:	
 mov bl,ds:byte ptr[1+esi]	

 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch19:	
 mov cl,ds:byte ptr[esi]	

 mov bh,dh	
 add edx,ebp	

 ror eax,16	
 mov ch,dh	

 mov ah,ds:byte ptr[12345678h+ebx]	
LBPatch20:	
 mov edx,ds:dword ptr[_lightright]	

 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch21:	
 mov ebp,ds:dword ptr[_lightdelta]	

 ;mov ds:dword ptr[edi],eax	
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 add edx,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 add ebp,eax	
 mov ds:dword ptr[_lightright],edx	

 jc Lblockloop16_mip2	

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
 public _R_DrawSurfaceBlock16_mip3	
_R_DrawSurfaceBlock16_mip3:	
 push ebp	; preserve caller's stack frame
 push edi	
 push esi	; preserve register variables
 push ebx	

 pop ebx	; restore register variables
 pop esi	
 pop edi	
 pop ebp	; restore the caller's stack frame
 ret	
;		for (v=0 ; v<numvblocks ; v++)
;		{
 mov ebx,ds:dword ptr[_r_lightptr]	
 mov eax,ds:dword ptr[_r_numvblocks]	

 mov ds:dword ptr[sb_v],eax	
 mov edi,ds:dword ptr[_prowdestbase]	

 mov esi,ds:dword ptr[_pbasesource]	

Lv_loop_mip3:	

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

 mov ds:dword ptr[_lightdelta],ebp	
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

 sar ecx,1	

 sar ebx,1	
 mov ds:dword ptr[_lightrightstep],ecx	

 sub ebx,ecx	
 and ebx,0FFFFFh	

 sar ebp,1	
 or ebx,0F0000000h	

 mov ds:dword ptr[_lightdeltastep],ebx	
 sub ebx,ebx	; high word must be 0 in loop for addressing

 mov bl,ds:byte ptr[1+esi]	
 sub ecx,ecx	; high word must be 0 in loop for addressing

 mov bh,dh	
 mov cl,ds:byte ptr[esi]	

 add edx,ebp	
 mov ch,dh	

 mov al,ds:byte ptr[12345678h+ebx]	
LBPatch16:	
 mov edx,ds:dword ptr[_lightright]	

 ;mov ds:byte ptr[1+edi],al	
 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch17:	

 ;mov ds:byte ptr[edi],al	
 mov eax,ds:dword ptr[_sourcetstep]	

 add esi,eax	
 mov eax,ds:dword ptr[_surfrowbytes]	

 add edi,eax	
 mov eax,ds:dword ptr[_lightdeltastep]	

 mov ebp,ds:dword ptr[_lightdelta]	
 mov cl,ds:byte ptr[esi]	

 add ebp,eax	
 mov eax,ds:dword ptr[_lightrightstep]	

 sar ebp,1	
 add edx,eax	

 mov bh,dh	
 mov bl,ds:byte ptr[1+esi]	

 add edx,ebp	
 mov ch,dh	

 mov al,ds:byte ptr[12345678h+ebx]	
LBPatch30:	
 mov edx,ds:dword ptr[_sourcetstep]	

 ;mov ds:byte ptr[1+edi],al	
 mov al,ds:byte ptr[12345678h+ecx]	
LBPatch31:	

 ;mov ds:byte ptr[edi],al	
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

;----------------------------------------------------------------------
; Code patching routines
;----------------------------------------------------------------------
_TEXT ENDS
_DATA SEGMENT	

 align 4	
LPatchTable16:	
 dd LBPatch0-4	
 dd LBPatch1-4	
 dd LBPatch2-4	
 dd LBPatch3-4	
 dd LBPatch4-4	
 dd LBPatch5-4	
 dd LBPatch6-4	
 dd LBPatch7-4	
 dd LBPatch8-4	
 dd LBPatch9-4	
 dd LBPatch10-4	
 dd LBPatch11-4	
 dd LBPatch12-4	
 dd LBPatch13-4	
 dd LBPatch14-4	
 dd LBPatch15-4	
 dd LBPatch16-4	
 dd LBPatch17-4	
 dd LBPatch18-4	
 dd LBPatch19-4	
 dd LBPatch20-4	
 dd LBPatch21-4	
 dd LBPatch22-4	
 dd LBPatch23-4	
 dd LBPatch24-4	
 dd LBPatch25-4	
 dd LBPatch26-4	
 dd LBPatch27-4	
 dd LBPatch28-4	
 dd LBPatch29-4	 
 dd LBPatch30-4	
 dd LBPatch31-4	

 externdef _d_8to16table:dword

_DATA ENDS
_TEXT SEGMENT	

 align 4	
 public _R_Surf16Patch	
_R_Surf16Patch:	
 push ebx	

 mov eax,ds:dword ptr[_colormap]	
 mov eax,ds:dword ptr[_d_8to16table]	
 mov ebx,offset LPatchTable16
 mov ecx,32	
LPatchLoop16:	
 mov edx,ds:dword ptr[ebx]	
 add ebx,4	
 mov ds:dword ptr[edx],eax
 dec ecx	
 jnz LPatchLoop16	

 pop ebx	

 ret	

_TEXT ENDS
endif	;id386

 END

