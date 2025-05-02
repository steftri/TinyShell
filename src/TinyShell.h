#ifndef TINY_SHELL_H
#define TINY_SHELL_H


#include <inttypes.h>



static const uint8_t SHELL_MAX_BUFFER_LENGTH = 80;
static const uint8_t SHELL_MAX_COMMANDS = 32;


/**
 * @class TinyShellCommand
 * @brief Abstract base class for defining commands in the TinyShell framework.
 *
 * This class provides an interface for implementing custom shell commands.
 * Derived classes must override the `exec` method to define the behavior
 * of the command when executed.
 */
class TinyShellCommand
{
public:
  /**
   * @brief Pure virtual function to execute a command with the given arguments.
   * 
   * This function must be implemented by derived classes to define the behavior
   * of executing a command. It takes the number of arguments and an array of 
   * argument strings as input.
   * 
   * @param argc The number of arguments passed to the command.
   * @param argv An array of C-style strings representing the arguments.
   * @return An integer representing the result or status of the command execution.
   */
  virtual int exec(int argc, char *argv[]) = 0;
};


/**
 * @class TinyShell
 * @brief A lightweight shell interface for managing and executing commands.
 * 
 * The TinyShell class provides a simple framework for adding, managing, and executing
 * commands in an embedded or constrained environment. It includes functionality for
 * handling input, maintaining a command buffer, and invoking registered commands.
 * 
 * @note This class is designed to be extended or customized for specific use cases.
 */
class TinyShell 
{
public: 
  /**
   * @enum ERc
   * @brief Represents return codes for indicating the success or failure of operations.
   * 
   * @var ERc::OK
   * Indicates that the operation completed successfully.
   * 
   * @var ERc::Error
   * Indicates that the operation encountered an error.
   */
  enum class ERc 
  {
    OK = 0, 
    Error = -1
  };

private:
  struct 
  {
    const char *pc_CmdName;
    TinyShellCommand *p_Command;
  } ma_Commands[SHELL_MAX_COMMANDS];

  uint16_t mu16_NumberOfCommands;

  char mac_Buffer[SHELL_MAX_BUFFER_LENGTH];
  uint16_t mu16_BufferPos;

public:
  TinyShell(void);

  // for configuration
  virtual void printPrompt(void);
  virtual void printCommandNotFound(const char *pc_Cmd);
  virtual void printCommandError(const char *pc_Cmd, const int rc);
  ERc addCommand(const char *pc_CmdName, TinyShellCommand *p_Command);

  // for operation
  void begin(void);
  void end(void);
  void putChar(const char c_Char);
  void reset(const bool b_DisplayPrompt = false);

private:
  void _execCmd(void);
};

#endif
