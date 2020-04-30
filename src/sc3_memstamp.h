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

/** \file sc3_memstamp.h \ingroup sc3
 * A data container to create memory items of the same size.
 * Allocations are bundled so it's fast for small memory sizes.
 * The items created will remain valid until the container is destroyed.
 * 
 * The memstamp container stores any number of fixed-size items within a new
 * stamp. If needed, memory is reallocated internally.
 *
 * We use standard `int` types for indexing.
 *
 * In the setup phase, we set the size of the element and stamp,
 * a number of items in stamp, an initzero property, and some more.
 *
 * An memstamp container can only be refd if it is setup.
 * Otherwise, the usual ref-, unref- and destroy semantics hold.
 */

#ifndef SC3_MEMSTAMP_H
#define SC3_MEMSTAMP_H

#include <sc3_error.h>

/** The memstamp container is an opaque struct. */
typedef struct sc3_mstamp sc3_mstamp_t;

#ifdef __cplusplus
extern              "C"
{
#if 0
}
#endif
#endif

/** Query whether a memory stamp container is not NULL and internally
 * consistent.
 * The container may be valid in both its setup and usage phases.
 * \param [in] mst      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True iff pointer is not NULL and container consistent.
 */
int                 sc3_mstamp_is_valid (const sc3_mstamp_t * mst,
                                         char *reason);

/** Query whether a memory stamp is not NULL, consistent and not setup.
 * This means that the memory stamp is not (yet) in its usage phase.
 * \param [in] mst      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True iff pointer not NULL, memory stamp consistent,
 *                      not setup.
 */
int                 sc3_mstamp_is_new (const sc3_mstamp_t * mst,
                                       char *reason);

/** Query whether a memory stamp is not NULL, internally consistent and setup.
 * This means that the memory stamp is in its usage phase.
 * We provide allocation of fixed-size memory items
 * without allocating new memory in every request.
 * Instead we block the allocations in what we call a stamp of multiple items.
 * Even if no allocations are done, the container's internal memory
 * must be freed eventually by \ref sc3_mstamp_destroy.
 * \param [in] mst      Any pointer.
 * \param [out] reason  If not NULL, existing string of length SC3_BUFSIZE
 *                      is set to "" if answer is yes or reason if no.
 * \return              True iff pointer not NULL, memory stamp consistent and
 *                      setup.
 */
int                 sc3_mstamp_is_setup (const sc3_mstamp_t * mst,
                                         char *reason);

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif /* !SC3_MEMSTAMP_H */