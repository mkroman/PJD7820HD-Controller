#ifndef __PJD7820HD_COMMANDS_H
#define __PJD7820HD_COMMANDS_H

// THIS FILE IS AUTO-GENERATED! DO NOT EDIT!

typedef struct pjd7820hd_command {
  int cmd;             /** The command id */
  const char* command; /** The actual command as a character byte array. */
  size_t command_len; /** The length of the command, in bytes. */
} pjd7820hd_command_t;

typedef enum {
{%- for command in commands %}
  {{command[0]}} = {{loop.index-1}},
{%- endfor %}
} pjd7820hd_command_e;

static const pjd7820hd_command_t pjd7820hd_commands[] = {
{%- for command in commands %}
  {
    .cmd = {{command[0]}},
    .command = (const char*)"{{ c_stringify_hex_str(command[1]) }}",
    .command_len = {{ len(command[1].split()) }}
  },
{%- endfor %}
};

#endif
