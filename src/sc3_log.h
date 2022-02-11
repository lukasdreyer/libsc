/*
  This file is part of the SC Library, version 3.
  The SC Library provides support for parallel scientific applications.

  Copyright (C) 2019 individual authors

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/** \file sc3_log.h
 * Provide a general mechanism for logging messages.
 *
 * We define the logger object, which remembers options such as the
 * minimum log level to print, which file to print to, and communicator.
 * The communicator is used to determine the rank of a process, such
 * that we can either print a message on all ranks or on the root only.
 * There is always a static logger object available printing to stderr
 * and using \ref SC3_MPI_COMM_WORLD as communicator.
 *
 * There is a compile-time minimum log level, \ref SC3_LOG_LEVEL,
 * below which all messages are ignored (at compile time when the
 * compiler optimizes for constant comparisons).  This level is
 * preset to \ref SC3_LOG_DEBUG with --enable-debug and \ref
 * SC3_LOG_INFO otherwise.  You may redefine this on configure,
 * however, it is safer to rely on the log level of the logger object.
 *
 * When a logger is set to a given log level, it will only log message
 * greater equal than the maximum of that level and \ref SC3_LOG_LEVEL.
 */

#ifndef SC3_LOG_H
#define SC3_LOG_H

#include <sc3_alloc.h>
#include <sc3_mpi.h>

#ifdef __cplusplus
extern              "C"
{
#if 0
}
#endif
#endif

/** Macro for error checking without hope for clean recovery.
 * If an error is encountered in calling \a f, we output its message
 * using \ref sc3_log_new_static () and then call MPI_Abort.
 */
#define SC3X(f) do {                                                    \
  sc3_error_t *_e = (f);                                                \
  if (_e != NULL) {                                                     \
    sc3_error_t *_e2 = sc3_error_new_stack                              \
      (&_e, __FILE__, __LINE__, #f);                                    \
    sc3_log_error_abort (NULL, SC3_LOG_LOCAL, 0, _e2);                  \
  }} while (0)

/** Opaque object to encapsulate options to the logging mechanism. */
typedef struct sc3_log sc3_log_t;

/** We may log per root MPI rank or for each MPI process. */
typedef enum sc3_log_role
{
  SC3_LOG_LOCAL,      /**< Log on all processes. */
  SC3_LOG_GLOBAL,     /**< Log only on root process. */
  SC3_LOG_ROLE_LAST   /**< Guard value is invalid. */
}
sc3_log_role_t;

/** Log level or priority.  Used to ignore messages of low priority. */
typedef enum sc3_log_level
{
  SC3_LOG_NOISE,      /**< Anything at all and all sorts of nonsense. */
  SC3_LOG_DEBUG,      /**< Information mainly useful for debugging.
                           Too much to be acceptable for production. */
  SC3_LOG_INFO,       /**< Detailed, but still acceptable for production. */
  SC3_LOG_STATISTICS, /**< Major diagnostics and statistical summaries. */
  SC3_LOG_PRODUCTION, /**< Sparse flow logging for toplevel functions. */
  SC3_LOG_ESSENTIAL,  /**< A few lines per program: version, options. */
  SC3_LOG_ERROR,      /**< Errors by misusage, internal bugs, I/O. */
  SC3_LOG_SILENT,     /**< This log level prints nothing at all. */
  SC3_LOG_LEVEL_LAST  /**< Guard value is invalid. */
}
sc3_log_level_t;

/** Prototype for the user-selectable log output function.
 * This function does not need to decide whether to log based on role or level.
 * This is done before calling this function, or not calling it depending.
 * This function is just responsible for formatting and writing the message.
 */
typedef void        (*sc3_log_function_t) (void *user, const char *msg,
                                           sc3_log_role_t role, int rank,
                                           sc3_log_level_t level,
                                           int indent, FILE * outfile);

#ifndef SC3_LOG_LEVEL
#ifdef SC_ENABLE_DEBUG
#define SC3_LOG_LEVEL SC3_LOG_DEBUG
#else
/** Minimum log level set at compile time without --enable-debug.
 * This becomes \ref SC3_LOG_DEBUG with --enable-debug.
 */
#define SC3_LOG_LEVEL SC3_LOG_INFO
#endif
#endif

/** Log function that prints the incoming message without formatting.
 * \param [in,out] user    This function ignores any user-defined context.
 * \param [in] msg     This function adds a newline to the end of the message.
 * \param [in] role    Ignored.
 * \param [in] rank    Ignored.
 * \param [in] level   Ignored.
 * \param [in] indent  Ignored.
 * \param [in,out] outfile      File printed to.  If NULL use stderr.
 */
void                sc3_log_function_bare
  (void *user, const char *msg,
   sc3_log_role_t role, int rank, sc3_log_level_t level,
   int indent, FILE * outfile);

/** Type suitable as user data for \ref sc3_log_function_prefix. */
typedef struct sc3_log_puser
{
  const char         *prefix;     /**< Short string used as log prefix. */
  int                 prefix_newline;     /**< Boolean: prefix each line of
                                               multi-line strings anew. */
}
sc3_log_puser_t;

/** Log function that adds rank information and indent spacing.
 * It does not check whether to log or not, that happens beforehand.
 * The parameters passed in are for formatting, not selection.
 *
 * \param [in] user     Pointer to \ref sc3_log_puser_t for prefix.
 * \param [in] msg      Nul-terminated string, or NULL, to be logged.
 * \param [in] role     Indicate local process or global (root) logging.
 * \param [in] rank     Number to report as MPI rank.
 * \param [in] level    Indicate log level.  Ignored by this function.
 * \param [in] indent   Non-negative number of spaces to prepend.
 * \param [in,out] outfile  File open for writing, or NULL for stderr.
 */
void                sc3_log_function_prefix
  (void *user, const char *msg,
   sc3_log_role_t role, int rank, sc3_log_level_t level,
   int indent, FILE * outfile);

/** Log function that adds rank information and indent spacing.
 * It does not check whether to log or not, that happens beforehand.
 * The parameters passed in are for formatting, not for decision.
 * Using prefix "sc3" and prefixing multi-line messages per line.
 *
 * \param [in] user     Ignored.
 * \param [in] msg      Nul-terminated string, or NULL, to be logged.
 * \param [in] role     Indicate local process or global (root) logging.
 * \param [in] rank     Number to report as MPI rank.
 * \param [in] level    Indicate log level.  Ignored by this function.
 * \param [in] indent   Non-negative number of spaces to prepend.
 * \param [in,out] outfile  File open for writing, or NULL for stderr.
 */
void                sc3_log_function_default
  (void *user, const char *msg,
   sc3_log_role_t role, int rank, sc3_log_level_t level,
   int indent, FILE * outfile);

/** Check whether a logger is not NULL and internally consistent.
 * The logger may be valid in both its setup and usage phases.
 * \param [in] log      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True if pointer is not NULL and logger consistent.
 */
int                 sc3_log_is_valid (const sc3_log_t * log, char *reason);

/** Check whether a logger is not NULL and not (yet) setup for usage.
 * \param [in] log      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True if logger is valid but not setup.
 */
int                 sc3_log_is_new (const sc3_log_t * log, char *reason);

/** Check whether a logger is not NULL and setup for usage.
 * \param [in] log      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True if logger is both valid and setup.
 */
int                 sc3_log_is_setup (const sc3_log_t * log, char *reason);

/** Create new logging object in its setup phase with default settings.
 * \param [in] lator    Either NULL or an allocator that is setup.
 *                      If NULL, use \ref sc3_allocator_new_static.
 * \param [in,out] logp Pointer not NULL.  Its value is the new logger
 *                      object on output, or NULL if an error occurred.
 * \return              NULL on success and error object otherwise.
 */
sc3_error_t        *sc3_log_new (sc3_allocator_t * lator, sc3_log_t ** logp);

/** Set the minimum log level that this log object may print.
 * Note that levels below \ref SC3_LOG_LEVEL are never printed.
 * Default: \ref SC3_LOG_LEVEL.
 * \param [in,out] log  Logger must be valid and not setup.
 * \param [in] level    Valid log level of \ref sc3_log_level_t.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_set_level (sc3_log_t * log,
                                       sc3_log_level_t level);

/** Set the MPI communicator to use for querying the rank.
 * Default: SC3_MPI_COMM_WORLD.
 * \param [in,out] log  Logger must be valid and not setup.
 * \param [in] mpicomm  Valid MPI communicator on this process.
 *                      The calling process must be a rank in this
 *                      communicator.  We do not MPI_Comm_dup it, which
 *                      means that it must not cease to exist while this
 *                      logger is in use.  Otherwise the log calls may crash
 *                      or output messages about an invalid communicator.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_set_comm (sc3_log_t * log,
                                      sc3_MPI_Comm_t mpicomm);

/** Set file to write logging output to.
 * Function may be called multiple times to override previous setting.
 * Defaults: stderr, 0.
 * \param [in,out] log  Logger object must not yet be setup.
 * \param [in] file     File to use for logging.  Open for writing.
 *                      Must not be closed while logger is alive.
 * \param [in] call_fclose  If true, we call fclose on \a file
 *                      when the logger object ist destroyed.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_set_file (sc3_log_t * log,
                                      FILE * file, int call_fclose);

/** Set function that effectively formats and outputs the log message.
 * Default: \ref sc3_log_function_default.
 * \param [in,out] log  Logger must not yet be setup.
 * \param [in] func     Non-NULL pointer of type \ref sc3_log_function_t.
 * \param [in] user     Pointer is passed through to log function \a func.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_set_function (sc3_log_t * log,
                                          sc3_log_function_t func,
                                          void *user);

/** Setup a logger object after setting its parameters.
 * \param [in,out] log  Valid log object.  Not yet setup.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_setup (sc3_log_t * log);

/** Add one reference to a logger object.
 * \param [in,out] log  Log object must be setup.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_ref (sc3_log_t * log);

/** Remove one reference to a logger object.
 * If the reference count reaches zero, logger is destroyed.
 * \param [in,out] logp Log object must be setup.
 *                      Value becomes NULL if destroyed.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_unref (sc3_log_t ** logp);

/** Destroy a logger object known to have one reference.
 * It is not forseen to query the amount of references of an object.
 * Use of this function is only possible if the logger object passed in
 * has one reference, provably.
 * \param [in,out] logp Log object must be setup and have one reference.
 *                      Value becomes NULL on output.
 * \return              NULL on success, error object otherwise.
 */
sc3_error_t        *sc3_log_destroy (sc3_log_t ** logp);

/** Return a predefined static logger that uses MPI_COMM_WORLD.
 * It prints to stderr and uses formatting with the prefix "sc3."
 * It can be treated as a dynamic logger regarding refing and destroy.
 * \return          Valid and setup logger object prints to stderr.
 */
sc3_log_t          *sc3_log_new_static (void);

/** Log a message depending on selection criteria.
 * Format by the settings of the logger and the log function registered.
 * If parameters passed in are illegal output appropriate error message.
 * If the logger is NULL, output appropriate error message to stderr.
 * \param [in] log       If NULL, write a message to stderr.
 *                       Otherwise, logger must be setup and will be queried
 *                       for log level and format options, etc.
 * \param [in] role      See \ref sc3_log_role_t for legal values.
 * \param [in] level     See \ref sc3_log_level_t for legal values.
 *                       Log if this log level is greater equal both the
 *                       minimum level of the logger and \ref SC3_LOG_LEVEL.
 * \param [in] indent    Number of spaces to prepend to message.
 * \param [in] msg       If NULL, print "NULL message," otherwise \a msg.
 */
void                sc3_log (sc3_log_t * log,
                             sc3_log_role_t role, sc3_log_level_t level,
                             int indent, const char *msg);

/** Log a message depending on selection criteria.
 * Format by the settings of the logger and the log function registered.
 * If parameters passed in are illegal output appropriate error message.
 * If the logger is NULL, output appropriate error message to stderr.
 * \param [in] log       If NULL, write a message to stderr.
 *                       Otherwise, logger must be setup and will be queried
 *                       for log level and format options, etc.
 * \param [in] role      See \ref sc3_log_role_t for legal values.
 * \param [in] level     See \ref sc3_log_level_t for legal values.
 *                       Log if this log level is greater equal both the
 *                       minimum level of the logger and \ref SC3_LOG_LEVEL.
 * \param [in] indent    Number of spaces to prepend to message.
 * \param [in] fmt       If NULL, print "NULL message," otherwise see printf (3).
 */
void                sc3_logf (sc3_log_t * log,
                              sc3_log_role_t role, sc3_log_level_t level,
                              int indent, const char *fmt, ...)
  __attribute__ ((format (printf, 5, 6)));

/** Log a message depending on selection criteria.
 * Format by the settings of the logger and the log function registered.
 * If parameters passed in are illegal output appropriate error message.
 * If the logger is NULL, output appropriate error message to stderr.
 * \param [in] log       If NULL, write a message to stderr.
 *                       Otherwise, logger must be setup and will be queried
 *                       for log level and format options, etc.
 * \param [in] role      See \ref sc3_log_role_t for legal values.
 * \param [in] level     See \ref sc3_log_level_t for legal values.
 *                       Log if this log level is greater equal both the
 *                       minimum level of the logger and \ref SC3_LOG_LEVEL.
 * \param [in] indent    Number of spaces to prepend to message.
 * \param [in] fmt       If NULL, print "NULL message," otherwise see printf (3).
 * \param [in] ap        Parameter list macro as with stdarg (3).
 */
void                sc3_logv (sc3_log_t * log,
                              sc3_log_role_t role, sc3_log_level_t level,
                              int indent, const char *fmt, va_list ap);

/*** convenience functions to log on any process ***/

/** Log a message to stderr with level \ref SC3_LOG_NOISE. */
#define SC3_NOISEC(s) SC3_NOISEF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_NOISE.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_NOISEF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_DEBUG. */
#define SC3_DEBUGC(s) SC3_DEBUGF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_DEBUG.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_DEBUGF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_INFO. */
#define SC3_INFOC(s) SC3_INFOF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_INFO.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_INFOF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_STATISTICS. */
#define SC3_STATISTICSC(s) SC3_STATISTICSF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_STATISTICS.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_STATISTICSF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_PRODUCTION. */
#define SC3_PRODUCTIONC(s) SC3_PRODUCTIONF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_PRODUCTION.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_PRODUCTIONF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_ESSENTIAL. */
#define SC3_ESSENTIALC(s) SC3_ESSENTIALF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_ESSENTIAL.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_ESSENTIALF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message to stderr with level \ref SC3_LOG_ERROR. */
#define SC3_ERRORC(s) SC3_ERRORF("%s", s)

/** Log a message to stderr with level \ref SC3_LOG_ERROR.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_ERRORF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/*** convenience functions to log on the root process ***/

/** Log a message on root with level \ref SC3_LOG_NOISE. */
#define SC3_GLOBAL_NOISEC(s) SC3_GLOBAL_NOISEF("%s", s)

/** Log a message on root with level \ref SC3_LOG_NOISE.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_NOISEF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_DEBUG. */
#define SC3_GLOBAL_DEBUGC(s) SC3_GLOBAL_DEBUGF("%s", s)

/** Log a message on root with level \ref SC3_LOG_DEBUG.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_DEBUGF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_INFO. */
#define SC3_GLOBAL_INFOC(s) SC3_GLOBAL_INFOF("%s", s)

/** Log a message on root with level \ref SC3_LOG_INFO.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_INFOF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_STATISTICS. */
#define SC3_GLOBAL_STATISTICSC(s) SC3_GLOBAL_STATISTICSF("%s", s)

/** Log a message on root with level \ref SC3_LOG_STATISTICS.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_STATISTICSF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_PRODUCTION. */
#define SC3_GLOBAL_PRODUCTIONC(s) SC3_GLOBAL_PRODUCTIONF("%s", s)

/** Log a message on root with level \ref SC3_LOG_PRODUCTION.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_PRODUCTIONF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_ESSENTIAL. */
#define SC3_GLOBAL_ESSENTIALC(s) SC3_GLOBAL_ESSENTIALF("%s", s)

/** Log a message on root with level \ref SC3_LOG_ESSENTIAL.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_ESSENTIALF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Log a message on root with level \ref SC3_LOG_ERROR. */
#define SC3_GLOBAL_ERRORC(s) SC3_GLOBAL_ERRORF("%s", s)

/** Log a message on root with level \ref SC3_LOG_ERROR.
* \param [in] fmt    Format string as with printf (3). */
void                SC3_GLOBAL_ERRORF (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

/** Examine an error: print its message and unref if not NULL.
 * This function can be called directly with the return value of an
 * error-object returning function.  Assumes ownership of the error.
 * \param [in] log      Logger must be setup, or NULL for
 *                      using \ref sc3_log_new_static ().
 *                      Using log level \ref SC3_LOG_ERROR.
 * \param [in] role     Valid \ref sc3_log_role_t.  When invalid,
 *                      return negative even with a NULL error.
 * \param [in] indent   Indentation passed to the log function.
 * \param [in] e        If NULL, do nothing and return 0.  Otherwise,
 *                      log error's message and return negative.
 *                      The error object is unrefd without checking.
 * \return              0 on success, negative value on error.
 */
int                 sc3_log_error_check (sc3_log_t * log,
                                         sc3_log_role_t role,
                                         int indent,
                                         sc3_error_t * e);

/** Examine an error: print its message, unref and abort if not NULL.
 * This function can be called directly with the return value of an
 * error-object returning function.  Assumes ownership of the error.
 * \param [in] log      Logger must be setup, or NULL for
 *                      using \ref sc3_log_new_static ().
 *                      Using log level \ref SC3_LOG_ERROR.
 * \param [in] role     Valid \ref sc3_log_role_t.  When invalid,
 *                      abort even with a NULL error.
 * \param [in] indent   Indentation passed to the log function.
 * \param [in] e        If NULL, do nothing.  Otherwise, log error's
 *                      message, unref error and abort program.
 */
void                sc3_log_error_abort (sc3_log_t * log,
                                         sc3_log_role_t role,
                                         int indent,
                                         sc3_error_t * e);

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif /* !SC3_LOG_H */
