/*
 * Copyright (C) 2026 TheCRatTime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char Peek(const char* text) {
  return (*text == '\0' ? '\0' : *(text+1));
}

static int ToUpper(const char* text) {
  if (*text >= 'a' && *text <= 'z') {
    return *text+('A'-'a');
  }

  return *text;
}

static void RealPutChar(const char text) {
  /* Needed escape characters. */
  if (text == '-') {
    fputs("\\-", stdout);
  } else if (text == '.') {
    fputs("\\&.", stdout);
  } else if (text == '\'') {
    fputs("'\\&", stdout);
  } else if (text == '\\'){
    fputs("\\e", stdout);
  } else {
    putchar(text);
  }
}

#define PutChar(chr) \
  RealPutChar(chr)

static const char* HeaderPrint(const char* text) {
  while(*text == '#') {
    text++;
  }

  /* Skipping space */
  if (Peek(text) == ' ') {
    text++;
  }

  fputs(".SH", stdout);

  while(*text != '\n' && *text != '\0') {
    PutChar((char)ToUpper(text++));
  }

  fputs("\n", stdout);

  return text;
}

static const char* PrintColor(const char* text,
                              const char* const first_ptr) {
  int star_count = 0;

  if (text != first_ptr) {
    if (*(text-1) == '\\') {
      return text;
    }
  }

  if (Peek(text) == ' ') {
    if (text != first_ptr) {
      if (*(text-1) != '\n') {
        putchar('\n');
      }
    }
    
    fputs(".br\n\\fB\\-\\-\\fR", stdout);
    return text;
  }
  
  while(*text == '*') {
    star_count++;
    text++;
  }

  switch(star_count) {
    case 1:
      fputs("\\fI", stdout);
      break;
    case 2:
    default:
      fputs("\\fB", stdout);
      break;
  }

  while(*text != '*' && *text != '\0') {
    PutChar(*(text++));
  }

  while(*text == '*') {
    text++;
  }

  fputs("\\fR", stdout);

  return --text;
}

static const char* PrintCode(const char* text,
                             const char* const first_ptr) {
  int quotes_count = 0;
  const char* saved_text = text;
  while(*text == '`') {
    quotes_count++;
    text++;
  }

  if (quotes_count >= 3) {
    if (saved_text != first_ptr) {
      if (*(saved_text-1) != '\n') {
        putchar('\n');
      }
    }
    
    fputs(".nf\n.RS 4", stdout);

    /* Skipping lang name */
    while(*text != '\n' && *text != '\0') {
      text++;
    }

    PutChar(*(text++));
    
  redo:
    while(*text != '`' && *text != '\0') {
      PutChar(*(text++));
    }

    if (*text == '\0') {
      goto end;
    }

    if (Peek(text) == '\0') {
      goto end;
    }

    if (Peek(text+1) == '\0') {
      goto end;
    }

    if (*text != '`' || Peek(text) != '`' || Peek(text+1) != '`') {
      goto redo;
    }

    text = text+3;
    fputs(".fi\n.RE\n.PP", stdout);
  end:;
  }
  else {
    while(*text != '`' && *text != '\0') {
      PutChar(*(text++));
    }

    text++;
  }

  text--;

  return text;
}

static const char* PrintBlock(const char* text,
                              const char* const first_ptr) {
  if (text != first_ptr) {
    if (*(text-1) != '\n') {
      fputs("\\-", stdout);
      return text;
    }
  }

  if (text != first_ptr) {
    if (*(text-1) != '\n') {
      putchar('\n');
    }
  }

  fputs(".br\n\\fB\\-\\-\\fR", stdout);

  return text;
}

static const char* PrintBlock2(const char* text,
                               const char* const first_ptr) {
  if (text != first_ptr) {
    if (*(text-1) != '\n') {
      return text;
    }
  }

  if (Peek(text) == ' ') {
    text++;
  }

  fputs(".RS 2\n", stdout);
  while(*text != '\n' && *text != '\0') {
    PutChar(*(text++));
  }

  fputs("\n.RE\n", stdout);

  return text;
}

static const char* NewlineCheck(const char* text,
                                const char* const first_ptr) {
  if (text != first_ptr) {
    if (*(text-1) != '\n') {
      putchar('\n');
      return text;
    }
  }

  fputs(".PP", stdout);

  if (Peek(text) != '\n') {
    putchar('\n');
  }
  
  return ++text;
}

static const char* ParseTH(const char* text) {
  int quotes = 0;
  const char* copy_text = text;
  
  if (strncmp(text, "TH ", 3)) {
    return text;
  }

  while(*copy_text != '\n' && *copy_text != '\0') {
    if (*copy_text == '"') {
      quotes++;
    }
    
    copy_text++;
  }

  /* 'TH "NAME" "SEC" "DOWN" "DOWN-LEFT"' - 8 quotes. */
  if (quotes != 8) {
    fputs(
          ".\\\" ==! md2man - TH Invalid. === \n"
          ".TH \"null\" \"1\" \"man\\-page\" \"null\"\n",
          stdout);
    return text;
  }

  fputs(".TH ", stdout);
  text += 3;

  while(*text != '\n' && *text != '\0') {
    PutChar(*(text++));
    
    while(*text != '"' && *text != '\n' && *text != '\0') {
      PutChar(*(text++));
    }

    PutChar(*(text++));
    
    if (*text != '\n' && *text != '\0') {
      text++;
      putchar(' ');
    }
  }
  
  return text;
}

static void PutComment(void) {
  fputs(
        ".\\\" === Automatically generated by md2man ===\n"
        ".\\\" --             Do not edit.            --\n"
        ".\\\" --     Copyright (C) TheCRatTime =)    --\n"
        ".\\\" --      Begin generated content:       --\n",
        stdout);
}

static const char* PrintLink(const char* text) {
  text++;

  fputs("\\fB", stdout);
  
  while(*text != ']' && *text != '\0') {
    PutChar(*(text++));
  }

  if (*text == '\0') {
  error:
    fputs("\\fR", stdout);
    return text;
  }
  
  if (Peek(text) == '\0' || Peek(text+1) == '\0') {
    goto error;
  }
  
  text += 2;

  fputs(" \\fR(\\fB", stdout);
  
  while(*text != ')' && *text != '\0') {
    PutChar(*(text++));
  }

  fputs("\\fR)", stdout);
  
  return text;
}

static const char* Backslash(const char* text) {
  if (Peek(text) != '*') {
    PutChar('\\');
    return text;
  }

  PutChar('*');
  
  return text;
}

static void PrettyPrint(const char* text) {
  const char* const first_text = text;
  
  PutComment();
  
  text = ParseTH(text);
  
  while(*text) {
    
    switch(*text) {
      case '#':
        text = HeaderPrint(text);
        break;
      case '*':
        text = PrintColor(text, first_text);
        break;
      case '`':
        text = PrintCode(text, first_text);
        break;
      case '-':
        text = PrintBlock(text, first_text);
        break;
      case '>':
        text = PrintBlock2(text, first_text);
        break;
      case '\n':
        text = NewlineCheck(text, first_text);
        break;
      case '[':
        text = PrintLink(text);
        break;
      case '\\':
        text = Backslash(text);
        break;
      default:
        PutChar(*text);
        break;
    }
    
    if (*text != '\0') {
      text++;
    }
  }

  if (*(text-1) != '\n') {
    putchar('\n');
  }
}

int main(int argc, char** argv) {
  FILE* file = NULL;
  char* buffer = NULL;
  size_t size = 0;
  
  if (argc < 2) {
    printf("Usage: %s FILE > OUTFILE\n", argv[0]);
    return 1;
  }

  file = fopen(argv[1], "rb");
  if (file == NULL) {
    perror(argv[0]);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  size = (size_t)ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer = (char*)malloc(size+1);

  if (buffer == NULL) {
    perror(argv[0]);
    fclose(file);
    return 1;
  }

  if (fread(buffer, 1, size, file) <= 0) {
    perror(argv[0]);
    free(buffer);
    fclose(file);
    return 1;
  }

  buffer[size] = '\0';
  fclose(file);

  PrettyPrint(buffer);

  free(buffer);
  return 0;
}
