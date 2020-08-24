/*!
    @file
*/
#include "SerialCLI.h"


/*!
    @brief  Call this function in your loop().
*/
void SerialCLI::loop()
{
    bool complete_message = false;

    while (ser_.available() && !complete_message)
    {
        // !complete_message - this prevents the Serial_buffer_ being rewritten
        // by new data when 2 messages are sent with very little delay (index
        // is set to 0 when complete_message is received).
        // I need to process the data before I receive the next message.

        Serial_buffer_[Serial_buffer_index_] = tolower(ser_.read());

        if (Serial_buffer_[Serial_buffer_index_] == '\n' || Serial_buffer_[Serial_buffer_index_] == '\r')
        {
            //  CR/LF
            if (Serial_buffer_index_ != 0)
            {
                // ignore if it is the first character (to avoid problems with CR+LF/LF)

                Serial_buffer_[Serial_buffer_index_] = '\0';  // rewrite CR/LF
                complete_message = true;
                Serial_buffer_index_ = 0;
                ser_.println();

#if defined(DEBUG) && defined(DEBUG_Serial_CLI)
                Serial.println();
                Serial.print(">> ");
                Serial.println(Serial_buffer_);

                char *ptr = &Serial_buffer_[0];
                while (*ptr != '\0')
                {
                    Serial.print(byte(*ptr), HEX);
                    Serial.print(' ');
                    ptr++;
                }
                Serial.println();
#endif // DEBUG
            }
        }
        else
        {
            // Character playback - this needs to be before index++ and should
            // not happen when the character is CR/LF
            ser_.print(Serial_buffer_[Serial_buffer_index_]);

            if (Serial_buffer_index_ < kSerial_buffer_length_)
            {
                Serial_buffer_index_++;
            }
            else
            {
                ser_.println();
                ser_.print(F("Cmd too long: "));
                for (byte i = 0; i <= Serial_buffer_index_; i++)
                    ser_.print(Serial_buffer_[i]);

                delay(40);  // to receive the rest of the message
                while (ser_.available())
                    ser_.print(char(ser_.read()));

                ser_.println();

                Serial_buffer_index_ = 0;
                Serial_buffer_[0] = '\0';  // this is currently not necessary
            }
        }
    }


    if (complete_message)
    {
        DEBUG_println();
        DEBUG_println(F("Processing"));

        char *cmd_ptr = nullptr;
        bool cmd_found = false;

        for (byte i = 0; i < command_count_; i++)
        {
            if ((cmd_ptr = strstr(Serial_buffer_, commands_[i].text)) != nullptr)
            {
                print_error_((commands_[i].handler)(cmd_ptr));
                cmd_found = true;
                break;  // do not process any further commands
            }
        }

        if (!cmd_found)
        {
            cmd_not_found_();
        }

        prev_command_millis_ = millis();

        // Prompt will be missing after reboot, but this can't be easily fixed.
        ser_.println();
        ser_.print(prompt_);
    }
}
