#ifndef TINY_SHELL_H
#define TINY_SHELL_H


#include <inttypes.h>



#define SHELL_MAX_BUFFER_LENGTH    80
#define SHELL_MAX_COMMANDS         32
#define SHELL_MAX_ARGS             8
#define SHELL_EOL_CHARACTER       '\r'
#define SHELL_BACKSPACE_CHARACTER '\x08'
#define SHELL_ESCAPE_CHARACTER    '\\'
#define SHELL_QUOTE_CHARACTER     '"'


class TinyShell 
{
public: 
  typedef void (TPromptCallback)(void);  
  typedef void (TDefaultCmdCallback)(char *pc_Cmd);
  typedef void (TCmdErrorCallback)(char *pc_Cmd, int rc);
  typedef int (TCmdCallback)(int argc, char *argv[]);
  enum ERc {RcOK = 0, RcError = -1};

private:
  TPromptCallback     *mp_PromptCallback;
  TDefaultCmdCallback *mp_DefaultCmdCallback;
  TCmdErrorCallback   *mp_CmdErrorCallback;

  struct 
  {
    const char *pc_Cmd;
    TCmdCallback *p_Callback;
  } ma_Commands[SHELL_MAX_COMMANDS];

  uint16_t mu16_NumberOfCommands;

  char mac_Buffer[SHELL_MAX_BUFFER_LENGTH];
  uint16_t mu16_BufferPos;

public:
  TinyShell(void);

  // for configuration
  void setPromptCallback(TPromptCallback *p_PromptCallback);
  void setCommandNotFoundCallback(TDefaultCmdCallback *p_DefaultCmdCallback);
  void setCommandErrorCallback(TCmdErrorCallback *p_CmdErrorCallback);
  ERc addCommandCallback(const char *pc_Cmd, TCmdCallback *p_CmdCallback);

  // for operation
  void begin(void);
  void end(void);
  void putChar(const char c_Char);
  void reset(const bool b_DisplayPrompt = false);

private:
  void _execCmd(void);
//  void _defaultCmdCallback(char *pc_Cmd);
};

#endif
