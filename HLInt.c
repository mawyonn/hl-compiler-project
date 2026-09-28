#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to trim leading and trailing whitespace (including \r and \n)
void trim(char *str)
{
    char *end;
    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
    {
        return;
    }
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';

    // Shift if needed (or we can just modify in place if str moved)
    // A simpler way:
    // memmove(...)
}

// Safer trim function that returns a pointer to the trimmed string
char *trim_string(char *str)
{
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;

    char *end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '\r' || *end == '\n'))
    {
        *end = '\0';
        end--;
    }
    return str;
}

int isReservedWord(const char *str)
{
    return (strcmp(str, "integer") == 0 ||
            strcmp(str, "double") == 0 ||
            strcmp(str, "output") == 0 ||
            strcmp(str, "if") == 0);
}

int isSymbol(const char *str)
{
    return (strcmp(str, ":") == 0 ||
            strcmp(str, ";") == 0 ||
            strcmp(str, ":=") == 0 ||
            strcmp(str, "<<") == 0 ||
            strcmp(str, "+") == 0 ||
            strcmp(str, "-") == 0 ||
            strcmp(str, "<") == 0 ||
            strcmp(str, ">") == 0 ||
            strcmp(str, "==") == 0 ||
            strcmp(str, "!=") == 0 ||
            strcmp(str, "(") == 0 ||
            strcmp(str, ")") == 0 ||
            strcmp(str, "\"") == 0);
}

int validateSyntax(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return 0;

    char raw_line[256];
    int hasError = 0;

    while (fgets(raw_line, sizeof(raw_line), file))
    {
        char *ptr = trim_string(raw_line);

        // Skip empty lines
        if (strlen(ptr) == 0)
            continue;

        // Check if statements: if(<condition>) - these do NOT require a semicolon
        if (strncmp(ptr, "if(", 3) == 0 || strncmp(ptr, "If(", 3) == 0 ||
            strncmp(ptr, "if (", 4) == 0 || strncmp(ptr, "If (", 4) == 0)
        {
            if (strchr(ptr, ')') == NULL)
            {
                hasError = 1;
            }
            continue;
        }

        // For all other statements, they must end with a semicolon
        int len = strlen(ptr);
        if (ptr[len - 1] != ';')
        {
            hasError = 1;
            continue;
        }

        // Check declarations: variable: type;
        char temp_line[256];
        strcpy(temp_line, ptr);
        temp_line[len - 1] = '\0'; // strip ';'

        char *colon = strchr(temp_line, ':');
        char *assignment = strstr(temp_line, ":=");

        if (colon != NULL && assignment == NULL)
        {
            *colon = '\0';
            char *dType = trim_string(colon + 1);

            if (strcmp(dType, "integer") != 0 && strcmp(dType, "double") != 0)
            {
                hasError = 1;
            }
            continue;
        }

        // Check assignments: variable := value;
        if (assignment != NULL)
        {
            continue;
        }

        // Check output statements: output<<...;
        if (strncmp(ptr, "output<<", 8) == 0)
        {
            continue;
        }

        // If it ended with ';' but matched nothing valid
        hasError = 1;
    }

    fclose(file);
    return !hasError;
}

int main(int argc, char *argv[])
{
    char filename[100];

    if (argc > 1)
    {
        strcpy(filename, argv[1]);
    }
    else
    {
        printf("Enter source code filename (e.g., PROG1.HL): ");
        scanf("%99s", filename);
    }

    FILE *inFile = fopen(filename, "r");
    if (!inFile)
    {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }

    FILE *noSpaceFile = fopen("NOSPACES.TXT", "w");
    FILE *resSymFile = fopen("RES_SYM.TXT", "w");

    if (!noSpaceFile || !resSymFile)
    {
        printf("Error: Could not create output files.\n");
        fclose(inFile);
        return 1;
    }

    char ch;
    // Step 1: Remove spaces and write to NOSPACES.TXT
    while ((ch = fgetc(inFile)) != EOF)
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fputc(ch, noSpaceFile);
        }
    }
    fclose(noSpaceFile);

    // Rewind input file for Step 2
    rewind(inFile);

    // Step 2: Extract reserved words and symbols to RES_SYM.TXT
    char buffer[100];
    int bufIndex = 0;

    while ((ch = fgetc(inFile)) != EOF)
    {
        if (isalnum(ch) || ch == '.')
        {
            buffer[bufIndex++] = ch;
        }
        else
        {
            if (bufIndex > 0)
            {
                buffer[bufIndex] = '\0';
                if (isReservedWord(buffer))
                {
                    fprintf(resSymFile, "%s\n", buffer);
                }
                bufIndex = 0;
            }

            char sym[3] = {ch, '\0', '\0'};
            if (ch == ':' || ch == '<' || ch == '=' || ch == '!')
            {
                char nextCh = fgetc(inFile);
                if (nextCh == '=' || (ch == '<' && nextCh == '<'))
                {
                    sym[1] = nextCh;
                }
                else
                {
                    ungetc(nextCh, inFile);
                }
            }

            if (isSymbol(sym))
            {
                fprintf(resSymFile, "%s\n", sym);
            }
        }
    }
    if (bufIndex > 0)
    {
        buffer[bufIndex] = '\0';
        if (isReservedWord(buffer))
        {
            fprintf(resSymFile, "%s\n", buffer);
        }
    }

    fclose(inFile);
    fclose(resSymFile);

    // Step 3: Syntax Verification
    if (validateSyntax(filename))
    {
        printf("NO ERROR(S) FOUND\n");
    }
    else
    {
        printf("ERROR\n");
    }

    return 0;
}