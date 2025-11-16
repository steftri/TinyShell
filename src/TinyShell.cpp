#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdio.h>
#endif


#include <ctype.h>
#include <string.h>

#include "TinyShell.h"



static const uint8_t SHELL_MAX_ARGS = 8;
static const char    SHELL_EOL_CHARACTER = '\r';
static const char    SHELL_BACKSPACE_CHARACTER = '\x08';
static const char    SHELL_ESCAPE_CHARACTER = '\\';
static const char    SHELL_QUOTE_CHARACTER = '"';



/**
 * @brief Constructor for the TinyShell class.
 * 
 * Initializes the TinyShell object with default values for the number of commands
 * and the buffer position.
 */
TinyShell::TinyShell(void)
  : mu16_NumberOfCommands{0}
  , mu16_BufferPos{0}
#ifdef ARDUINO
  , mp_Stream{nullptr}
#endif
{
}



#ifdef ARDUINO
/**
 * @brief Constructor for the TinyShell class with a specified output stream.
 * 
 * Initializes the TinyShell object with a given output stream for command
 * prompt display. This constructor is intended for use on Arduino platforms.
 * 
 * @param p_Stream A pointer to a Stream object (e.g., Serial) for output.
 */
TinyShell::TinyShell(Stream *p_Stream)
  : mu16_NumberOfCommands{0}
  , mu16_BufferPos{0}
  , mp_Stream{p_Stream}
{
}
#endif



/**
 * @brief Initializes the TinyShell instance and performs a reset.
 * 
 * This function is used to start the TinyShell instance. It calls the
 * reset function to perform an initial reset of the shell.
 */
void TinyShell::begin(void)
{
  reset(true);
}



/**
 * @brief Terminates the TinyShell instance and performs any necessary cleanup.
 * 
 * This function is intended to be called when the TinyShell instance is no longer
 * needed. Override this function to implement specific cleanup logic if required.
 */
void TinyShell::end(void)
{
}


#ifdef ARDUINO
/**
 * @brief Sets the output stream for the TinyShell instance.
 * 
 * This function allows the user to specify a different output stream for the
 * TinyShell instance. It is intended for use on Arduino platforms.
 * 
 * @param p_Stream A pointer to a Stream object (e.g., Serial) for output.
 */
void TinyShell::setStream(Stream *p_Stream)
{
  mp_Stream = p_Stream;
}
#endif



/**
 * @brief Main loop function for the TinyShell class.
 * 
 * This function continuously reads characters from the associated stream
 * (if available) and processes them. It mirrors back the entered characters
 * to the stream, ensuring that a newline sequence is sent when a carriage
 * return ('\r') is encountered. Additionally, it passes the received character
 * to the `putChar` method for further handling.
 * 
 * @note This function assumes that `mp_Stream` is a valid pointer to a stream
 *       object that supports `available()`, `read()`, `write()`, and `println()` methods.
 */
void TinyShell::loop(void)
{
#ifdef ARDUINO
  if(mp_Stream)
  {
    while(mp_Stream->available())
    {
      char c = mp_Stream->read();

      // mirror back entered characters
      if(c == SHELL_EOL_CHARACTER)
      {
        // if return, also send a complete newline sequence  
        mp_Stream->println(); 
      } 
      else
      {
        mp_Stream->write(c);
      }

      // handover the received character from the serial device
      putChar(c);
    }
  }
  #endif
}



/**
 * @brief Prints the command prompt to the appropriate output stream.
 * 
 * This function outputs a command prompt symbol ("> ") to indicate that the
 * shell is ready to accept user input. The output stream depends on the
 * platform:
 * - On Arduino, the prompt is printed to the Serial interface.
 * - On other platforms, the prompt is printed to the standard output (stdout) using printf.
 * 
 * This method is intended to be overridden by derived classes to customize
 * the shell prompt display.
 */
void TinyShell::printPrompt()
{
#ifdef ARDUINO
  if(mp_Stream != nullptr)
  {
    mp_Stream->print(F("> "));
  }
#else
  printf("> ");
#endif
}


/**
 * @brief Prints a message indicating that a command was not found.
 * 
 * This function outputs an error message when an unrecognized command is entered.
 * The output format depends on whether the code is running on an Arduino platform
 * or a standard system. On Arduino, the message is sent to the Serial interface.
 * On other platforms, it is printed to the standard output using printf.
 * 
 * @param pc_Cmd A pointer to a null-terminated string representing the command
 *               that was not recognized.
 * 
 * This method is intended to be overridden by derived classes to customize
 * the error message for unknown commands.
 */
void TinyShell::printCommandNotFound(const char *pc_Cmd)
{
#ifdef ARDUINO
  if(mp_Stream != nullptr)
  {
    mp_Stream->print(F("Command '"));
    mp_Stream->print(pc_Cmd);
    mp_Stream->println(F("' not found"));
  }
#else
  printf("Command '%s' not found\n", pc_Cmd);
#endif  
}


/**
 * @brief Prints an error message indicating that a command has failed, along with its return code.
 * 
 * This function outputs an error message to the appropriate output stream depending on the platform.
 * On Arduino, it uses the `Serial` interface, while on other platforms, it uses `printf`.
 * 
 * @param pc_Cmd The name of the command that failed.
 * @param rc The return code associated with the failure.
 *
 * This method is intended to be overridden by derived classes to customize
 * the error message for command failures.
 */
void TinyShell::printCommandError(const char *pc_Cmd, const int rc)
{
#ifdef ARDUINO
  if(mp_Stream != nullptr)
  {
    mp_Stream->print(pc_Cmd);
    mp_Stream->print(F(" failed returncode "));
    mp_Stream->println(rc);
  }
#else
  printf("%s failed returncode %i\n", pc_Cmd, rc);
#endif
}


/**
 * @brief Adds a new command to the TinyShell command list.
 * 
 * @param pc_CmdName A pointer to a null-terminated string representing the name of the command.
 * @param p_Command A pointer to a TinyShellCommand object representing the command implementation.
 * @return TinyShell::ERc Returns ERc::OK if the command was successfully added, or ERc::Error if the maximum number of commands (SHELL_MAX_COMMANDS) has been reached.
 * 
 * @note The function does not check for duplicate command names. It is the caller's responsibility to ensure unique command names.
 */
TinyShell::ERc TinyShell::addCommand(const char *pc_CmdName, TinyShellCommand *p_Command)
{
  if(mu16_NumberOfCommands>=TINYSHELL_MAX_COMMANDS)
    return ERc::Error;

  ma_Commands[mu16_NumberOfCommands].pc_CmdName = pc_CmdName;
  ma_Commands[mu16_NumberOfCommands].p_Command = p_Command;
  mu16_NumberOfCommands++;

  return ERc::OK;
}





/**
 * @brief Resets the TinyShell state, clearing the input buffer and optionally displaying the prompt.
 * 
 * @param b_DisplayPrompt If true, the shell prompt will be displayed after resetting.
 */
void TinyShell::reset(const bool b_DisplayPrompt)
{
  mu16_BufferPos = 0;
  if(b_DisplayPrompt)
    printPrompt();
}



/**
 * @brief Processes a character input for the TinyShell.
 *
 * This function handles character input by performing actions such as executing
 * a command when the end-of-line character is received, handling backspace to
 * remove the last character in the buffer, or appending characters to the buffer
 * if it has not reached its maximum length.
 *
 * @param c_Char The character to be processed.
 *
 * Behavior:
 * - If `c_Char` is the end-of-line character (`SHELL_EOL_CHARACTER`), the command
 *   in the buffer is executed, and the buffer position is reset.
 * - If `c_Char` is the backspace character (`SHELL_BACKSPACE_CHARACTER`), the buffer
 *   position is decremented if it is greater than 0, effectively removing the last
 *   character.
 * - If the buffer has not reached its maximum length (`SHELL_MAX_BUFFER_LENGTH`),
 *   the character is appended to the buffer, and the buffer position is incremented.
 */
void TinyShell::putChar(const char c_Char)
{
  if(c_Char==SHELL_EOL_CHARACTER)
  {
    _execCmd();
    mu16_BufferPos = 0;
  }
  else if(c_Char==SHELL_BACKSPACE_CHARACTER)
  {
    if(mu16_BufferPos>0)
      mu16_BufferPos--;
  }   
  else if(mu16_BufferPos<TINYSHELL_MAX_BUFFER_LENGTH)
  {
    mac_Buffer[mu16_BufferPos++] = c_Char;
  }
}



/**
 * @brief Executes a command entered into the shell by parsing the input buffer,
 *        matching it against available commands, and invoking the corresponding
 *        command handler.
 * 
 * This function processes the input buffer `mac_Buffer` to extract arguments,
 * handles escape characters and quoted strings, and matches the parsed command
 * with registered commands. If a match is found, the corresponding command
 * handler is executed. If no match is found, a "command not found" message is
 * displayed. The function also handles edge cases such as empty input or
 * exceeding the maximum number of arguments.
 * 
 * @details
 * - The input buffer is parsed into words (arguments) while respecting escape
 *   characters (`\`) and quoted strings (`"`).
 * - Arguments are stored in the `argv` array, and the number of arguments is
 *   tracked by `u16_Args`.
 * - If no command is found in the input, the function simply returns after
 *   displaying the shell prompt.
 * - If a matching command is found, its handler is executed, and any error
 *   returned by the handler is displayed.
 * - If no matching command is found, a default "command not found" message is
 *   displayed.
 * 
 * @note
 * - The function ensures safety by initializing unused `argv` entries to point
 *   to a predefined error string (`<err>`).
 * - The maximum number of arguments is defined by `SHELL_MAX_ARGS`.
 * - The maximum buffer length is defined by `SHELL_MAX_BUFFER_LENGTH`.
 * 
 * @param None
 * 
 * @return void
 */
void TinyShell::_execCmd(void)
{
  char ac_Buffer[TINYSHELL_MAX_BUFFER_LENGTH+1];   // +1 because of trailing \0
  uint16_t u16_BufferPos = 0;
  bool b_ArgFound = false;
  char *argv[SHELL_MAX_ARGS];
  uint16_t u16_Args = 0;
  bool b_Escaped = false;
  bool b_Quoted = false;
  char ac_Err[]="<err>";
  int rc = 0;  
  
  // first, parse the string - split it into words, but allow escape character (\) 
  // and quotes ("")
  for(auto i=0U; i<mu16_BufferPos; i++)
  {
    if(mac_Buffer[i]==SHELL_ESCAPE_CHARACTER && !b_Escaped)
    {
      b_Escaped=true;
    }
    else if(mac_Buffer[i]==SHELL_QUOTE_CHARACTER && !b_Escaped)
    {
      b_Quoted = !b_Quoted;
      if(!b_ArgFound && u16_Args<SHELL_MAX_ARGS)
      {
        argv[u16_Args++]=&ac_Buffer[u16_BufferPos];
        b_ArgFound = true;
      }
    }
    else
    { 
      if(isspace(mac_Buffer[i]) && !(b_Escaped||b_Quoted))
      {
        if(b_ArgFound) 
        {
          ac_Buffer[u16_BufferPos++] = 0;
          b_ArgFound = false;
        }
      }
      else
      {
        b_Escaped = false;
        if(!b_ArgFound && u16_Args<SHELL_MAX_ARGS)
        {
          argv[u16_Args++]=&ac_Buffer[u16_BufferPos];
          b_ArgFound = true;
        }
        ac_Buffer[u16_BufferPos++] = mac_Buffer[i];
      }
    }
  }
  ac_Buffer[u16_BufferPos++]=0;
  for(auto i=u16_Args; i<SHELL_MAX_ARGS; i++)
    argv[i]=ac_Err;  // for safety, set pointer to defined string instead of pointing anywhere 

  // if no command found, quit
  if(u16_Args == 0)
  {
    printPrompt();
    return;
  }

  // second, match the command with available ones
  for(auto i=0U; i<mu16_NumberOfCommands; i++)
  {
    if(!strcmp(ma_Commands[i].pc_CmdName, argv[0]))
    {
      if(ma_Commands[i].p_Command)
      {
        rc=ma_Commands[i].p_Command->exec(u16_Args, argv);
        if(rc)
          printCommandError(argv[0], rc);
      }  
      printPrompt();
      return;
    }
  }

  // if no matching command found, call default command callback
  printCommandNotFound(argv[0]);
  printPrompt();

  return;
}

