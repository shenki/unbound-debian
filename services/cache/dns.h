/*
 * services/cache/dns.h - Cache services for DNS using msg and rrset caches.
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains the DNS cache.
 */

#ifndef SERVICES_CACHE_DNS_H
#define SERVICES_CACHE_DNS_H
#include "util/storage/lruhash.h"
#include "util/data/msgreply.h"
struct module_env;
struct query_info;
struct reply_info;
struct regional;
struct delegpt;

/**
 * Region allocated message reply
 */
struct dns_msg {
	/** query info */
	struct query_info qinfo;
	/** reply info - ptr to packed repinfo structure */
	struct reply_info *rep;
};

/**
 * Allocate a dns_msg with malloc/alloc structure and store in dns cache.
 *
 * @param env: environment, with alloc structure and dns cache.
 * @param qinf: query info, the query for which answer is stored.
 * 	this is allocated in a region, and will be copied to malloc area
 * 	before insertion.
 * @param rep: reply in dns_msg from dns_alloc_msg for example.
 * 	this is allocated in a region, and will be copied to malloc area
 * 	before insertion.
 * @param is_referral: If true, then the given message to be stored is a
 *      referral. The cache implementation may use this as a hint.
 *      It will store only the RRsets, not the message.
 * @return 0 on alloc error (out of memory).
 */
int dns_cache_store(struct module_env* env, struct query_info* qinf,
        struct reply_info* rep, int is_referral); 

/**
 * Store message in the cache. Stores in message cache and rrset cache.
 * Both qinfo and rep should be malloced and are put in the cache.
 * They should not be used after this call, as they are then in shared cache.
 * Does not return errors, they are logged and only lead to less cache.
 *
 * @param env: module environment with the DNS cache.
 * @param qinfo: query info
 * @param hash: hash over qinfo.
 * @param rep: reply info, together with qinfo makes up the message.
 *	Adjusts the reply info TTLs to absolute time.
 */
void dns_cache_store_msg(struct module_env* env, struct query_info* qinfo,
	hashvalue_t hash, struct reply_info* rep);

/**
 * Find a delegation from the cache.
 * @param env: module environment with the DNS cache.
 * @param qname: query name.
 * @param qnamelen: length of qname.
 * @param qtype: query type.
 * @param qclass: query class.
 * @param region: where to allocate result delegation.
 * @param msg: if not NULL, delegation message is returned here, synthesized
 *	from the cache.
 * @param timenow: the time now, for checking if TTL on cache entries is OK.
 * @return new delegation or NULL on error or if not found in cache.
 */
struct delegpt* dns_cache_find_delegation(struct module_env* env, 
	uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass, 
	struct regional* region, struct dns_msg** msg, uint32_t timenow);

/** 
 * Find cached message 
 * @param env: module environment with the DNS cache.
 * @param qname: query name.
 * @param qnamelen: length of qname.
 * @param qtype: query type.
 * @param qclass: query class.
 * @param region: where to allocate result.
 * @param scratch: where to allocate temporary data.
 * @return new response message (alloced in region, rrsets do not have IDs).
 * 	or NULL on error or if not found in cache.
 *	TTLs are made relative to the current time.
 */
struct dns_msg* dns_cache_lookup(struct module_env* env,
	uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass,
	struct regional* region, struct regional* scratch);

/** 
 * find and add A and AAAA records for missing nameservers in delegpt 
 * @param env: module environment with rrset cache
 * @param qclass: which class to look in.
 * @param region: where to store new dp info.
 * @param dp: delegation point to fill missing entries.
 * @return false on alloc failure.
 */
int cache_fill_missing(struct module_env* env, uint16_t qclass, 
	struct regional* region, struct delegpt* dp);

#endif /* SERVICES_CACHE_DNS_H */
