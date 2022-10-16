#ifndef	_VIEWTHING_H
#define	_VIEWTHING_H


void	vt_Stop (userEntity_t *self);
void	vt_Start (userEntity_t *self);
void	vt_Forward(userEntity_t *self);
void	vt_Backward(userEntity_t *self);
void	vt_Delay (userEntity_t *self);
void	vt_ViewFrame (userEntity_t *self);
void	vt_ViewSequence (userEntity_t *self);
void	vt_ListSequences (userEntity_t *self);
void	vt_Rotate (userEntity_t *self);
void	vt_ViewModel (userEntity_t *self);
void	vt_ViewModelOnly(userEntity_t *ent);
void	vt_ScaleModel (userEntity_t *ent);
void	vt_SetTranslucent (userEntity_t *self);

#endif