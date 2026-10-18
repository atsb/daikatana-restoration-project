// input.h -- external (non-keyboard) input devices

struct SDL_Window;

void IN_Init (void);
void IN_Shutdown (void);
void IN_Commands (void);
// opportunity for devices to stick commands on the script buffer
void IN_Frame (void);
void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd
void IN_Activate (qboolean active);

// SDL3 event/input bridge.  The core input implementation is platform agnostic.
void IN_PumpEvents (void);
void IN_ActivateMouse (void);
void IN_DeactivateMouse (void);
void IN_MenuMouseSetup (void);
void IN_MenuMouseRelease (void);
void IN_SetSDLWindow (SDL_Window *window);
qboolean IN_HandleSetCursor (void);
