#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getline.h"
#include "ggets.h"

#ifndef SIZE_MAX
    #define SIZE_MAX ((size_t) -1)
#endif

#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#else
    typedef enum { false, true } bool;
#endif

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof *(a))


static const char hexDigits[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};


/** escape_string
  *
  * PARAMETERS:
  *     IN s : The string to escape.
  *
  * RETURNS:
  *     Returns an allocated copy of `s` with non-printable characters escaped.
  *
  *     Returns NULL on failure.
  *
  *     The caller is responsible for freeing the returned string with `free()`.
  */
static char*
escape_string(const char* s)
{
    char* escaped = NULL;
    char* out;
    char c;
    size_t n;

    assert(s != NULL);

    n  = strlen(s);

    /* Assume the worst case where every character needs to be escaped as a
     * hex sequence (e.g. `\xFF`).
     */
    if (n >= SIZE_MAX / 4)
    {
        return NULL;
    }
    escaped = malloc(n * 4 + 1 /* NUL */);
    if (escaped == NULL)
    {
        return NULL;
    }

    out = escaped;
    while ((c = *s++) != '\0')
    {
        switch (c)
        {
            case '\a':
                *out++ = '\\';
                *out++ = 'a';
                break;
            case '\b':
                *out++ = '\\';
                *out++ = 'b';
                break;
            case '\f':
                *out++ = '\\';
                *out++ = 'f';
                break;
            case '\n':
                *out++ = '\\';
                *out++ = 'n';
                break;
            case '\r':
                *out++ = '\\';
                *out++ = 'r';
                break;
            case '\t':
                *out++ = '\\';
                *out++ = 't';
                break;
            case '\v':
                *out++ = '\\';
                *out++ = 'v';
                break;
            case '\\':
                *out++ = '\\';
                *out++ = '\\';
                break;
            case '\"':
                *out++ = '\\';
                *out++ = '\"';
                break;
            default:
            {
                unsigned char uc = (unsigned char) c;
                if (isprint(uc))
                {
                    *out++ = c;
                }
                else
                {
                    *out++ = '\\';
                    *out++ = 'x';
                    *out++ = hexDigits[uc >> 4];
                    *out++ = hexDigits[uc & 0xF];
                }
                break;
            }
        }
    }
    *out = '\0';

    return escaped;
}



static bool
expect(bool condition, const char* expectedReason, int sourceLineNumber)
{
    if (condition)
    {
        return true;
    }

    fprintf(stderr,
            "FAIL (line: %d)\n"
            "  expected: %s\n",
            sourceLineNumber,
            expectedReason);
    return false;
}

#define EXPECT_LINE(condition, sourceLineNumber) \
    expect(condition, #condition, sourceLineNumber)

#define EXPECT(condition) \
    EXPECT_LINE(condition, __LINE__)

#define EXPECT_VAL_LINE(actual, expected, fmt, sourceLineNumber) \
    (((actual) == (expected)) \
     ? true \
     : (fprintf(stderr, \
                "FAIL (line: %d)\n" \
                "  expected: " fmt "\n" \
                "    actual: " fmt "\n", \
                sourceLineNumber, \
                expected, \
                actual), \
        false))

#define EXPECT_VAL(actual, expected, fmt) \
    EXPECT_VAL_LINE(actual, expected, fmt, __LINE__)

static bool
expect_str(const char* actual, const char* expected, int sourceLineNumber)
{
    char* escapedActual = NULL;
    char* escapedExpected = NULL;
    if (strcmp(actual, expected) == 0)
    {
        return true;
    }

    escapedActual = escape_string(actual);
    escapedExpected = escape_string(expected);

    fprintf(stderr,
            "FAIL (line: %d)\n"
            "  expected: \"%s\"\n"
            "    actual: \"%s\"\n",
            sourceLineNumber,
            (escapedExpected != NULL) ? escapedExpected : expected,
            (escapedActual != NULL) ? escapedActual : actual);

    free(escapedActual);
    free(escapedExpected);
    return false;
}

#define EXPECT_STR(actual, expected) \
    expect_str(actual, expected, __LINE__)


static bool
expect_getline(char** buffer, size_t* bufferSize, FILE* fp,
               const char* expectedString, int sourceLineNumber)
{
    ssize_t bytesRead;
    bool success = true;

    assert(buffer != NULL);
    assert(bufferSize != NULL);
    assert(fp != NULL);

    bytesRead = getline(buffer, bufferSize, fp);
    success &= EXPECT_LINE(bytesRead >= 0, sourceLineNumber);
    success &= EXPECT_VAL_LINE((unsigned long) bytesRead,
                               (unsigned long) strlen(expectedString),
                               "%lu",
                               sourceLineNumber);
    success &= expect_str(*buffer, expectedString, sourceLineNumber);
    success &= EXPECT_LINE(*bufferSize > bytesRead, sourceLineNumber);
    return success;
}

#define EXPECT_GETLINE(line, len, fp, expectedString) \
    expect_getline(line, len, fp, expectedString, __LINE__)


static bool
expect_fggets(FILE* fp, const char* expectedString, int sourceLineNumber)
{
    bool success = true;
    int error;
    char* line = NULL;

    assert(fp != NULL);

    error = fggets(&line, fp);
    success &= EXPECT_VAL_LINE(error, 0, "%d", sourceLineNumber);
    success &= expect_str(line, expectedString, sourceLineNumber);

    free(line);
    return success;
}

#define EXPECT_FGGETS(fp, expectedString) \
    expect_fggets(fp, expectedString, __LINE__)


typedef struct
{
    FILE* fp;

    char* line;
    size_t len;
} TestContext;


/** init_test_context
  *
  *     Initializes a `TestContext`.
  *
  * PARAMETERS:
  *     OUT context : The `TestContext` to initialize.
  *
  * RETURNS:
  *     Returns true on successful initialization, false on failure.
  *
  *     A successfully  initialized `TestContext` must be released with
  *     `free_test_context` when no longer needed.
  */
static bool
init_test_context(TestContext* context)
{
    TestContext emptyContext = { 0 };

    assert(context != NULL);

    *context = emptyContext;

    /* We need to create a separate temporary file per test because the
     * standard library doesn't provide any means for truncating a file.
     */
    context->fp = tmpfile();
    if (context->fp == NULL)
    {
        fprintf(stderr, "Failed to create temporary file.\n");
        return false;
    }
    return true;
}


/** free_test_context
  *
  *     Releases an initialized `TestContext`.
  *
  * PARAMETERS:
  *     IN context : The `TestContext` to free.
  */
static void
free_test_context(TestContext* context)
{
    assert(context != NULL);
    free(context->line);
    if (context->fp != NULL)
    {
        fclose(context->fp);
    }
}


typedef bool (*TestCallback)(TestContext*);


static bool
test_escape_string(TestContext* context)
{
    bool success = true;
    size_t i;

    struct
    {
        const char* original;
        const char* expected;
    } cases[] =
    {
        {
            "The five boxing wizards jump quickly.",
            "The five boxing wizards jump quickly."
        },
        {
            "The five boxing wizards jump quickly.\n",
            "The five boxing wizards jump quickly.\\n",
        },
        {
            "The five boxing wizards jump quickly.\r\n",
            "The five boxing wizards jump quickly.\\r\\n",
        },
        {
            "\\",
            "\\\\",
        },
        {
            "\"Hello!\"",
            "\\\"Hello!\\\"",
        },
        {
            "\x01\x02",
            "\\x01\\x02",
        },
    };

    for (i = 0; i < ARRAY_LENGTH(cases); i++)
    {
        char* escaped = escape_string(cases[i].original);
        if (escaped == NULL)
        {
            fprintf(stderr, "Failed to allocate memory.\n");
            return false;
        }

        success &= EXPECT_STR(escaped, cases[i].expected);
        free(escaped);
    }

    return success;
}


static bool
test_getline_empty_file(TestContext* context)
{
    bool success = true;
    ssize_t bytesRead = getline(&(context->line), &(context->len),
                                context->fp);
    success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
    success &= EXPECT_STR(context->line, "");
    success &= EXPECT_VAL((unsigned long) context->len, 1UL, "%lu");
    return success;
}


static bool
test_getline_single_terminated_line(TestContext* context)
{
    bool success = true;
    const char* expectedString = "The five boxing wizards jump quickly.\n";

    fprintf(context->fp, "%s", expectedString);
    fflush(context->fp);
    rewind(context->fp);

    success &= EXPECT_GETLINE(&(context->line), &(context->len), context->fp,
                              expectedString);

    {
        ssize_t bytesRead = getline(&(context->line), &(context->len),
                                    context->fp);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
    }

    return success;
}


static bool
test_getline_multiple_terminated_lines(TestContext* context)
{
    bool success = true;

    const char* expectedStrings[] =
    {
        "The five boxing wizards jump quickly.\n",
        "Pack my box with five dozen liquor jugs.\n",
        "The quick brown fox jumps over the dog.\n",
    };

    size_t i;
    for (i = 0; i < ARRAY_LENGTH(expectedStrings); i++)
    {
        fprintf(context->fp, "%s", expectedStrings[i]);
    }
    fflush(context->fp);
    rewind(context->fp);

    for (i = 0; i < ARRAY_LENGTH(expectedStrings); i++)
    {
        success &= EXPECT_GETLINE(&(context->line), &(context->len),
                                  context->fp,
                                  expectedStrings[i]);
    }

    {
        ssize_t bytesRead = getline(&(context->line), &(context->len),
                                    context->fp);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
    }

    return success;
}


/** random_printable_char
  *
  * RETURNS:
  *     Returns a random char is satisfies `isprint`.
  */
static char
random_printable_char()
{
    /* This is a *terrible* way to generate random numbers in the range of
     * [0, 0xFF] for a number of reasons, but we don't really care about bias.
     */
    while (true)
    {
        int i = rand() & 0xFF;
        if (isprint(i))
        {
            return (char) (unsigned char) i;
        }
    }
}


static bool
test_getline_file_with_long_lines(TestContext* context)
{
    bool success = true;
    const size_t maxLineSize = 1 << 10;
    size_t lineSize;
    for (lineSize = 2; success && lineSize < maxLineSize; lineSize++)
    {
        ssize_t bytesRead;
        size_t i;
        char* expectedString = NULL;
        FILE* tempFile = tmpfile();
        if (tempFile == NULL)
        {
            fprintf(stderr, "Failed to create temporary file.\n");
            success = false;
            goto loopEnd;
        }

        expectedString = malloc(lineSize);
        if (expectedString == NULL)
        {
            fprintf(stderr, "Failed to allocate %lu bytes.\n",
                    (unsigned long) lineSize);
            success = false;
            goto loopEnd;
        }

        for (i = 0; i + 2 < lineSize; i++)
        {
            expectedString[i] = random_printable_char();
        }
        expectedString[i++] = '\n';
        expectedString[i] = '\0';
        assert(i < lineSize);

        fprintf(tempFile, "%s", expectedString);
        fflush(tempFile);
        rewind(tempFile);

        success &= EXPECT_GETLINE(&(context->line), &(context->len),
                                  tempFile,
                                  expectedString);

        bytesRead = getline(&(context->line), &(context->len), tempFile);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");

    loopEnd:
        free(expectedString);
        if (tempFile != NULL)
        {
            fclose(tempFile);
        }
    }
    return success;
}

static bool
test_getline_file_without_newline(TestContext* context)
{
    bool success = true;
    const char* expectedString = "The five boxing wizards jump quickly.";

    fprintf(context->fp, "%s", expectedString);
    fflush(context->fp);
    rewind(context->fp);

    success &= EXPECT_GETLINE(&(context->line), &(context->len), context->fp,
                              expectedString);

    {
        ssize_t bytesRead = getline(&(context->line), &(context->len),
                                    context->fp);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
    }

    return success;
}

static bool
test_getline_writes_into_existing_buffer(TestContext* context)
{
    bool success = true;
    const char* expectedString = "The five boxing wizards jump quickly.\n";

    size_t bufferSize = 64;
    const char* oldBuffer;
    char* buffer = malloc(bufferSize);
    if (buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate memory.\n");
        return false;
    }
    oldBuffer = buffer;

    fprintf(context->fp, "%s", expectedString);
    fflush(context->fp);
    rewind(context->fp);

    success &= EXPECT_GETLINE(&buffer, &bufferSize, context->fp,
                              expectedString);
    success &= EXPECT(buffer == oldBuffer);
    success &= EXPECT_VAL((unsigned long) bufferSize, 64UL, "%lu");

    {
        ssize_t bytesRead = getline(&buffer, &bufferSize, context->fp);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
        success &= EXPECT(buffer == oldBuffer);
        success &= EXPECT_VAL((unsigned long) bufferSize, 64UL, "%lu");
    }

    free(buffer);

    return success;
}

static bool
test_getline_grows_existing_buffer(TestContext* context)
{
    bool success = true;
    const char* expectedString = "The five boxing wizards jump quickly.\n";

    size_t bufferSize = 4;
    const char* oldBuffer;
    char* buffer = malloc(bufferSize);
    if (buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate memory.\n");
        return false;
    }
    oldBuffer = buffer;

    fprintf(context->fp, "%s", expectedString);
    fflush(context->fp);
    rewind(context->fp);

    success &= EXPECT_GETLINE(&buffer, &bufferSize, context->fp,
                              expectedString);
    success &= EXPECT(buffer != oldBuffer);

    {
        ssize_t bytesRead = getline(&buffer, &bufferSize, context->fp);
        success &= EXPECT_VAL((long) bytesRead, -1L, "%ld");
    }

    free(buffer);

    return success;
}


static bool
test_getdelim_binary_data(TestContext* context)
{
    bool success = true;
    ssize_t bytesRead;

    char expectedBuffer[257];
    size_t i;
    for (i = 0; i < sizeof expectedBuffer; i++)
    {
        expectedBuffer[i] = i & 0xFF;
    }
    assert(expectedBuffer[sizeof expectedBuffer - 1] == '\0');

    if (fwrite(expectedBuffer, sizeof expectedBuffer, 1, context->fp) != 1)
    {
        fprintf(stderr, "Failed to write to temporary file.\n");
        return false;
    }

    fflush(context->fp);
    rewind(context->fp);

    bytesRead = getdelim(&(context->line), &(context->len), 9, context->fp);
    success &= EXPECT(bytesRead >= 0);
    success &= EXPECT_VAL((long) bytesRead, 10L, "%ld");
    success &= EXPECT(context->len > bytesRead);
    success &= memcmp(context->line, expectedBuffer, bytesRead) == 0;
    success &= context->line[bytesRead] == '\0';

    bytesRead = getdelim(&(context->line), &(context->len), '\0', context->fp);
    success &= EXPECT(bytesRead >= 0);
    success &= EXPECT_VAL((long) bytesRead, 247L, "%ld");
    success &= EXPECT(context->len > bytesRead);
    success &= memcmp(context->line, &expectedBuffer[10], bytesRead) == 0;
    success &= context->line[bytesRead] == '\0';

    return success;
}

static bool
test_fggets_single_line(TestContext* context, bool newlineTerminated)
{
    bool success = true;
    const char* expectedString = "The five boxing wizards jump quickly.";

    fprintf(context->fp, "%s%s",
            expectedString,
            newlineTerminated ? "\n" : "");
    fflush(context->fp);
    rewind(context->fp);

    success &= EXPECT_FGGETS(context->fp, expectedString);

    {
        char* line = NULL;
        int result = fggets(&line, context->fp);
        success &= EXPECT(result != 0);
        success &= EXPECT(line == NULL);
    }

    return success;
}


static bool test_fggets_single_terminated_line(TestContext* context)
{
    return test_fggets_single_line(context, true);
}


static bool
test_fggets_multiple_terminated_lines(TestContext* context)
{
    bool success = true;
    const char* expectedStrings[] =
    {
        "The five boxing wizards jump quickly.",
        "Pack my box with five dozen liquor jugs.",
        "The quick brown fox jumps over the dog.",
    };

    size_t i;
    for (i = 0; i < ARRAY_LENGTH(expectedStrings); i++)
    {
        fprintf(context->fp, "%s\n", expectedStrings[i]);
    }

    fflush(context->fp);
    rewind(context->fp);

    for (i = 0; i < ARRAY_LENGTH(expectedStrings); i++)
    {
        success &= EXPECT_FGGETS(context->fp, expectedStrings[i]);
    }

    {
        char* line = NULL;
        int result = fggets(&line, context->fp);
        success &= EXPECT(result != 0);
        success &= EXPECT(line == NULL);
    }

    return success;
}


static bool
test_fggets_file_without_newline(TestContext* context)
{
    return test_fggets_single_line(context, false);
}


int
main(void)
{
    bool success = true;

    size_t i;

    #define ADD_TEST(testCallback) { #testCallback, testCallback }
    struct
    {
        const char* description;
        TestCallback callback;
    } tests[] =
    {
        ADD_TEST(test_escape_string),

        ADD_TEST(test_getline_empty_file),
        ADD_TEST(test_getline_single_terminated_line),
        ADD_TEST(test_getline_multiple_terminated_lines),
        ADD_TEST(test_getline_file_with_long_lines),
        ADD_TEST(test_getline_file_without_newline),
        ADD_TEST(test_getline_writes_into_existing_buffer),
        ADD_TEST(test_getline_grows_existing_buffer),

        ADD_TEST(test_getdelim_binary_data),

        ADD_TEST(test_fggets_single_terminated_line),
        ADD_TEST(test_fggets_multiple_terminated_lines),
        ADD_TEST(test_fggets_file_without_newline),
    };
    #undef ADD_TEST

    for (i = 0; i < ARRAY_LENGTH(tests); i++)
    {
        TestContext context;
        if (!init_test_context(&context))
        {
            success = false;
            break;
        }
        printf("%s...\n", tests[i].description);
        success &= tests[i].callback(&context);
        free_test_context(&context);
    }

    if (success)
    {
        printf("All tests succeeded.\n");
    }
    else
    {
        printf("Tests failed.\n");
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
