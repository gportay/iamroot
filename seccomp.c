/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <seccomp.h>

#include "iamroot.h"

/**
 * Query the library version information
 *
 * This function returns a pointer to a populated scmp_version struct, the
 * caller does not need to free the structure when finished.
 *
 */
const struct scmp_version *seccomp_version(void)
{
	fprintf(stderr, "%s()\n", __func__);

	return __set_errno(ENOSYS, NULL);
}

/**
 * Query the library's level of API support
 *
 * This function returns an API level value indicating the current supported
 * functionality.  It is important to note that this level of support is
 * determined at runtime and therefore can change based on the running kernel
 * and system configuration (e.g. any previously loaded seccomp filters).  This
 * function can be called multiple times, but it only queries the system the
 * first time it is called, the API level is cached and used in subsequent
 * calls.
 *
 * The current API levels are described below:
 *  0 : reserved
 *  1 : base level
 *  2 : support for the SCMP_FLTATR_CTL_TSYNC filter attribute
 *      uses the seccomp(2) syscall instead of the prctl(2) syscall
 *  3 : support for the SCMP_FLTATR_CTL_LOG filter attribute
 *      support for the SCMP_ACT_LOG action
 *      support for the SCMP_ACT_KILL_PROCESS action
 *  4 : support for the SCMP_FLTATR_CTL_SSB filter attrbute
 *  5 : support for the SCMP_ACT_NOTIFY action and notify APIs
 *  6 : support the simultaneous use of SCMP_FLTATR_CTL_TSYNC and notify APIs
 *
 */
unsigned int seccomp_api_get(void)
{
	fprintf(stderr, "%s()\n", __func__);

	return __set_errno(ENOSYS, 0);
}

/**
 * Set the library's level of API support
 *
 * This function forcibly sets the API level of the library at runtime.  Valid
 * API levels are discussed in the description of the seccomp_api_get()
 * function.  General use of this function is strongly discouraged.
 *
 */
int seccomp_api_set(unsigned int level)
{
	fprintf(stderr, "%s(level: %u)\n", __func__, level);

	return __set_errno(ENOSYS, -1);
}

/**
 * Initialize the filter state
 * @param def_action the default filter action
 *
 * This function initializes the internal seccomp filter state and should
 * be called before any other functions in this library to ensure the filter
 * state is initialized.  Returns a filter context on success, NULL on failure.
 *
 */
scmp_filter_ctx seccomp_init(uint32_t def_action)
{
	fprintf(stderr, "%s(def_action: %u)\n", __func__, def_action);

	return __set_errno(ENOSYS, NULL);
}

/**
 * Reset the filter state
 * @param ctx the filter context
 * @param def_action the default filter action
 *
 * This function resets the given seccomp filter state and ensures the
 * filter state is reinitialized.  This function does not reset any seccomp
 * filters already loaded into the kernel.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_reset(scmp_filter_ctx ctx, uint32_t def_action)
{
	fprintf(stderr, "%s(ctx: %p, def_action: %u)\n", __func__, ctx,
		def_action);

	return __set_errno(ENOSYS, -1);
}

/**
 * Destroys the filter state and releases any resources
 * @param ctx the filter context
 *
 * This functions destroys the given seccomp filter state and releases any
 * resources, including memory, associated with the filter state.  This
 * function does not reset any seccomp filters already loaded into the kernel.
 * The filter context can no longer be used after calling this function.
 *
 */
void seccomp_release(scmp_filter_ctx ctx)
{
	fprintf(stderr, "%s(ctx: %p)\n", __func__, ctx);
}

/**
 * Merge two filters
 * @param ctx_dst the destination filter context
 * @param ctx_src the source filter context
 *
 * This function merges two filter contexts into a single filter context and
 * destroys the second filter context.  The two filter contexts must have the
 * same attribute values and not contain any of the same architectures; if they
 * do, the merge operation will fail.  On success, the source filter context
 * will be destroyed and should no longer be used; it is not necessary to
 * call seccomp_release() on the source filter context.  Returns zero on
 * success, negative values on failure.
 *
 */
int seccomp_merge(scmp_filter_ctx ctx_dst, scmp_filter_ctx ctx_src)
{
	fprintf(stderr, "%s(ctx_dst: %p, ctx_src: %p)\n", __func__, ctx_dst,
		ctx_src);

	return __set_errno(ENOSYS, -1);
}

/**
 * Resolve the architecture name to a architecture token
 * @param arch_name the architecture name
 *
 * This function resolves the given architecture name to a token suitable for
 * use with libseccomp, returns zero on failure.
 *
 */
uint32_t seccomp_arch_resolve_name(const char *arch_name)
{
	fprintf(stderr, "%s(arch_name: '%s')\n", __func__, arch_name);

	return __set_errno(ENOSYS, 0);
}

/**
 * Return the native architecture token
 *
 * This function returns the native architecture token value, e.g. SCMP_ARCH_*.
 *
 */
uint32_t seccomp_arch_native(void)
{
	fprintf(stderr, "%s()\n", __func__);

	return __set_errno(ENOSYS, 0);
}

/**
 * Check to see if an existing architecture is present in the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function tests to see if a given architecture is included in the filter
 * context.  If the architecture token is SCMP_ARCH_NATIVE then the native
 * architecture will be assumed.  Returns zero if the architecture exists in
 * the filter, -EEXIST if it is not present, and other negative values on
 * failure.
 *
 */
int seccomp_arch_exist(const scmp_filter_ctx ctx, uint32_t arch_token)
{
	fprintf(stderr, "%s(ctx: %p, arch_token: %u)\n", __func__, ctx,
		arch_token);

	return __set_errno(ENOSYS, -1);
}

/**
 * Adds an architecture to the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function adds a new architecture to the given seccomp filter context.
 * Any new rules added after this function successfully returns will be added
 * to this architecture but existing rules will not be added to this
 * architecture.  If the architecture token is SCMP_ARCH_NATIVE then the native
 * architecture will be assumed.  Returns zero on success, -EEXIST if
 * specified architecture is already present, other negative values on failure.
 *
 */
int seccomp_arch_add(scmp_filter_ctx ctx, uint32_t arch_token)
{
	fprintf(stderr, "%s(ctx: %p, arch_token: %u)\n", __func__, ctx,
		arch_token);

	return __set_errno(ENOSYS, -1);
}

/**
 * Removes an architecture from the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function removes an architecture from the given seccomp filter context.
 * If the architecture token is SCMP_ARCH_NATIVE then the native architecture
 * will be assumed.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_arch_remove(scmp_filter_ctx ctx, uint32_t arch_token)
{
	fprintf(stderr, "%s(ctx: %p, arch_token: %u)\n", __func__, ctx,
		arch_token);

	return __set_errno(ENOSYS, -1);
}

/**
 * Loads the filter into the kernel
 * @param ctx the filter context
 *
 * This function loads the given seccomp filter context into the kernel.  If
 * the filter was loaded correctly, the kernel will be enforcing the filter
 * when this function returns.  Returns zero on success, negative values on
 * error.
 *
 */
int seccomp_load(const scmp_filter_ctx ctx)
{
	fprintf(stderr, "%s(ctx: %p)\n", __func__, ctx);

	return __set_errno(ENOSYS, -1);
}

/**
 * Get the value of a filter attribute
 * @param ctx the filter context
 * @param attr the filter attribute name
 * @param value the filter attribute value
 *
 * This function fetches the value of the given attribute name and returns it
 * via @value.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_attr_get(const scmp_filter_ctx ctx,
		     enum scmp_filter_attr attr, uint32_t *value)
{
	fprintf(stderr, "%s(ctx: %p, attr: %i, value: %p)\n", __func__, ctx,
		attr, value);

	return __set_errno(ENOSYS, -1);
}

/**
 * Set the value of a filter attribute
 * @param ctx the filter context
 * @param attr the filter attribute name
 * @param value the filter attribute value
 *
 * This function sets the value of the given attribute.  Returns zero on
 * success, negative values on failure.
 *
 */
int seccomp_attr_set(scmp_filter_ctx ctx,
		     enum scmp_filter_attr attr, uint32_t value)
{
	fprintf(stderr, "%s(ctx: %p, attr: %i, value: %u)\n", __func__, ctx,
		attr, value);

	return __set_errno(ENOSYS, -1);
}

/**
 * Resolve a syscall number to a name
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 * @param num the syscall number
 *
 * Resolve the given syscall number to the syscall name for the given
 * architecture; it is up to the caller to free the returned string.  Returns
 * the syscall name on success, NULL on failure.
 *
 */
char *seccomp_syscall_resolve_num_arch(uint32_t arch_token, int num)
{
	fprintf(stderr, "%s(arch_token: %u, num: %i)\n", __func__, arch_token,
		num);

	return __set_errno(ENOSYS, NULL);
}

/**
 * Resolve a syscall name to a number
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 * @param name the syscall name
 *
 * Resolve the given syscall name to the syscall number for the given
 * architecture.  Returns the syscall number on success, including negative
 * pseudo syscall numbers (e.g. __PNR_*); returns __NR_SCMP_ERROR on failure.
 *
 */
int seccomp_syscall_resolve_name_arch(uint32_t arch_token, const char *name)
{
	fprintf(stderr, "%s(arch_token: %u, name: '%s')\n", __func__,
		arch_token, name);

	return __set_errno(ENOSYS, -1);
}

/**
 * Resolve a syscall name to a number and perform any rewriting necessary
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 * @param name the syscall name
 *
 * Resolve the given syscall name to the syscall number for the given
 * architecture and do any necessary syscall rewriting needed by the
 * architecture.  Returns the syscall number on success, including negative
 * pseudo syscall numbers (e.g. __PNR_*); returns __NR_SCMP_ERROR on failure.
 *
 */
int seccomp_syscall_resolve_name_rewrite(uint32_t arch_token, const char *name)
{
	fprintf(stderr, "%s(arch_token: %u, name: '%s')\n", __func__,
		arch_token, name);

	return __set_errno(ENOSYS, -1);
}

/**
 * Resolve a syscall name to a number
 * @param name the syscall name
 *
 * Resolve the given syscall name to the syscall number.  Returns the syscall
 * number on success, including negative pseudo syscall numbers (e.g. __PNR_*);
 * returns __NR_SCMP_ERROR on failure.
 *
 */
int seccomp_syscall_resolve_name(const char *name)
{
	fprintf(stderr, "%s(name: '%s')\n", __func__, name);

	return __set_errno(ENOSYS, -1);
}

/**
 * Set the priority of a given syscall
 * @param ctx the filter context
 * @param syscall the syscall number
 * @param priority priority value, higher value == higher priority
 *
 * This function sets the priority of the given syscall; this value is used
 * when generating the seccomp filter code such that higher priority syscalls
 * will incur less filter code overhead than the lower priority syscalls in the
 * filter.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_syscall_priority(scmp_filter_ctx ctx,
			     int syscall, uint8_t priority)
{
	fprintf(stderr, "%s(ctx: %p, syscall: %i, priority: %u)\n", __func__,
		ctx, syscall, priority);

	return __set_errno(ENOSYS, -1);
}

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of argument filters in the argument filter chain
 * @param ... scmp_arg_cmp structs (use of SCMP_ARG_CMP() recommended)
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule needs to be adjusted due to architecture specifics it
 * will be adjusted without notification.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_rule_add(scmp_filter_ctx ctx,
		     uint32_t action, int syscall, unsigned int arg_cnt, ...)
{
	fprintf(stderr, "%s(ctx: %p, action: %u, syscall: %i, arg_cnt: %u, ...)\n",
		__func__, ctx, action, syscall, arg_cnt);

	return __set_errno(ENOSYS, -1);
}


/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of elements in the arg_array parameter
 * @param arg_array array of scmp_arg_cmp structs
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule needs to be adjusted due to architecture specifics it
 * will be adjusted without notification.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_rule_add_array(scmp_filter_ctx ctx,
			   uint32_t action, int syscall, unsigned int arg_cnt,
			   const struct scmp_arg_cmp *arg_array)
{
	fprintf(stderr, "%s(ctx: %p, action: %u, syscall: %i, arg_cnt: %u, arg_array: %p)\n",
		__func__, ctx, action, syscall, arg_cnt, arg_array);

	return __set_errno(ENOSYS, -1);
}

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of argument filters in the argument filter chain
 * @param ... scmp_arg_cmp structs (use of SCMP_ARG_CMP() recommended)
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule can not be represented on the architecture the
 * function will fail.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_rule_add_exact(scmp_filter_ctx ctx, uint32_t action,
			   int syscall, unsigned int arg_cnt, ...)
{
	fprintf(stderr, "%s(ctx: %p, action: %u, syscall: %i, arg_cnt: %u, ...)\n",
		__func__, ctx, action, syscall, arg_cnt);

	return __set_errno(ENOSYS, -1);
}

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt  the number of elements in the arg_array parameter
 * @param arg_array array of scmp_arg_cmp structs
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule can not be represented on the architecture the
 * function will fail.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_rule_add_exact_array(scmp_filter_ctx ctx,
				 uint32_t action, int syscall,
				 unsigned int arg_cnt,
				 const struct scmp_arg_cmp *arg_array)
{
	fprintf(stderr, "%s(ctx: %p, action: %u, syscall: %i, arg_cnt: %u, arg_array: %p)\n",
		__func__, ctx, action, syscall, arg_cnt, arg_array);

	return __set_errno(ENOSYS, -1);
}

/**
 * Allocate a pair of notification request/response structures
 * @param req the request location
 * @param resp the response location
 *
 * This function allocates a pair of request/response structure by computing
 * the correct sized based on the currently running kernel. It returns zero on
 * success, and negative values on failure.
 *
 */
int seccomp_notify_alloc(struct seccomp_notif **req,
			 struct seccomp_notif_resp **resp)
{
	fprintf(stderr, "%s(req: %p, resp: %p)\n", __func__, req, resp);

	return __set_errno(ENOSYS, -1);
}

/**
 * Free a pair of notification request/response structures.
 * @param req the request location
 * @param resp the response location
 */
void seccomp_notify_free(struct seccomp_notif *req,
			 struct seccomp_notif_resp *resp)
{
	fprintf(stderr, "%s(req: %p, resp: %p)\n", __func__, req, resp);
}

/**
 * Receive a notification from a seccomp notification fd
 * @param fd the notification fd
 * @param req the request buffer to save into
 *
 * Blocks waiting for a notification on this fd. This function is thread safe
 * (synchronization is performed in the kernel). Returns zero on success,
 * negative values on error.
 *
 */
int seccomp_notify_receive(int fd, struct seccomp_notif *req)
{
	fprintf(stderr, "%s(fd: %i, req: %p)\n", __func__, fd, req);

	return __set_errno(ENOSYS, -1);
}

/**
 * Send a notification response to a seccomp notification fd
 * @param fd the notification fd
 * @param resp the response buffer to use
 *
 * Sends a notification response on this fd. This function is thread safe
 * (synchronization is performed in the kernel). Returns zero on success,
 * negative values on error.
 *
 */
int seccomp_notify_respond(int fd, struct seccomp_notif_resp *resp)
{
	fprintf(stderr, "%s(fd: %i, req: %p)\n", __func__, fd, resp);

	return __set_errno(ENOSYS, -1);
}

/**
 * Check if a notification id is still valid
 * @param fd the notification fd
 * @param id the id to test
 *
 * Checks to see if a notification id is still valid. Returns 0 on success, and
 * negative values on failure.
 *
 */
int seccomp_notify_id_valid(int fd, uint64_t id)
{
	fprintf(stderr, "%s(fd: %i, id: %lu)\n", __func__, fd,
		(unsigned long)id);

	return __set_errno(ENOSYS, -1);
}

/**
 * Return the notification fd from a filter that has already been loaded
 * @param ctx the filter context
 *
 * This returns the listener fd that was generated when the seccomp policy was
 * loaded. This is only valid after seccomp_load() with a filter that makes
 * use of SCMP_ACT_NOTIFY.
 *
 */
int seccomp_notify_fd(const scmp_filter_ctx ctx)
{
	fprintf(stderr, "%s(ctx: %p)\n", __func__, ctx);

	return __set_errno(ENOSYS, -1);
}

/**
 * Generate seccomp Pseudo Filter Code (PFC) and export it to a file
 * @param ctx the filter context
 * @param fd the destination fd
 *
 * This function generates seccomp Pseudo Filter Code (PFC) and writes it to
 * the given fd.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_export_pfc(const scmp_filter_ctx ctx, int fd)
{
	fprintf(stderr, "%s(ctx: %p, fd: %i)\n", __func__, ctx, fd);

	return __set_errno(ENOSYS, -1);
}

/**
 * Generate seccomp Berkley Packet Filter (BPF) code and export it to a file
 * @param ctx the filter context
 * @param fd the destination fd
 *
 * This function generates seccomp Berkley Packer Filter (BPF) code and writes
 * it to the given fd.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_export_bpf(const scmp_filter_ctx ctx, int fd)
{
	fprintf(stderr, "%s(ctx: %p, fd: %i)\n", __func__, ctx, fd);

	return __set_errno(ENOSYS, -1);
}
