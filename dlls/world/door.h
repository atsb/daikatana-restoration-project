extern	void	Sub_CalcMove (userEntity_t *self, Vector &tdest, float tspeed, void (*func) (serverState_t *server, userEntity_t *self));
extern	void	Sub_CalcMoveDone (userEntity_t *self);
extern	void	Sub_CalcAngleMoveDone (userEntity_t *self);
extern	void	Sub_CalcAngleMove (userEntity_t *self, Vector &destangle, float tspeed, void (*func) (serverState_t *server, userEntity_t *self));