/*!
    @file
*/

#ifndef SERIALCLI_H
#define SERIALCLI_H

#include "Arduino.h"

// For DEBUG:
#include "Settings.h"
#include "Constants.h"

/*!
    @brief  A command line interface provided on a Stream object.
            The actual commands are defined externally, this object can be used
            to create any CLI you need.
*/
class SerialCLI
{
public:
    //! An error returned by CLI commands.
    typedef byte error_t;

    //! A CLI command.
    struct command_t
    {
        //! string to be matched
        const char* text;
        //! command handler to be called when text is matched
        error_t (*handler)(char* cmdptr);
    };

    //! call this function in your loop()
    void loop();

    /*!
        @brief  Constructor.
        @param ser  Reference to the Stream object the CLI should be provided on.
        @param commands Pointer to the first element of an array of commands.
                        The commands are processed from the start of the array
                        to the end, the first command thus has highest
                        priority.
                        Once a command is found, no further commands are
                        processed.
        @param command_count    Number of commands in the `commands` array.
                                You should be able to use something like
                                `(sizeof(commands) / sizeof(SerialCLI::command_t))`.
        @param print_error  Reference to a function that prints the string
                            representation of a given error code.
        @param cmd_not_found    Reference to a function that should be called
                                if no valid command is found in the user input.
        @param prompt   Pointer to a string that should be used as a prompt.
    */
    SerialCLI(Stream& ser, const command_t* commands, const byte command_count,
              void (&print_error)(error_t),
              void (&cmd_not_found)(), const char* prompt
             ) : ser_(ser), commands_(commands), command_count_(command_count),
                 print_error_(print_error), cmd_not_found_(cmd_not_found),
                 prompt_(prompt) { }

    //! Get the millis() of last command execution.
    unsigned long prev_command_millis() const { return prev_command_millis_; };


protected:
    Stream& ser_;
    const command_t* commands_;
    const byte command_count_;
    void (&print_error_)(error_t);
    void (&cmd_not_found_)();
    const char* prompt_;
    bool print_prompt_ = true;

    //! maximum command length
    static constexpr byte kSerial_buffer_length_ = 12;
    char Serial_buffer_[kSerial_buffer_length_ + 1]; // +1 for termination
    byte Serial_buffer_index_;

    unsigned long prev_command_millis_ = 0;

};

#endif
